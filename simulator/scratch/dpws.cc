#include "ns3/config-file-reader.h"
#include "ns3/core-module.h"
#include "ns3/mpi-interface.h"
#include "ns3/p2p-autonomous-system.h"
#include "ns3/randomized-partial-mesh-central-network.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DPWS");

// helpers

void
LogDPWSMessage(std::string message)
{
    // running in build-profile 'optimized' turns of the runtime logging framework. this is used
    // instead (and only in this file) to still be able to provide progress messages even when using
    // said 'optimized' build-profile
    if (MpiInterface::GetSystemId() == 0)
    {
        // only log on rank 0
        std::cout << message << std::endl;
    }
}

std::string
GetTimeString(std::time_t* time)
{
    // https://stackoverflow.com/a/16358111
    std::ostringstream oss;
    auto tm = *std::localtime(time);
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void
ScheduleProgressLog(int simulationTimeInterval, std::time_t startTime, int totalSimTime)
{
    // calculate elapsed real time in seconds
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds =
        now - std::chrono::system_clock::from_time_t(startTime);

    ns3::Time simulationNow = ns3::Simulator::Now();
    std::string log = "DPWS:\t\tsimulation progress: elapsed simulation time (s): (" +
                      std::to_string(simulationNow.GetSeconds()) + " of " +
                      std::to_string(totalSimTime) +
                      "), elapsed real-time (s): " + std::to_string(elapsedSeconds.count());
    LogDPWSMessage(log);

    // schedule next progress log
    Simulator::Schedule(Seconds(simulationTimeInterval),
                        &ScheduleProgressLog,
                        simulationTimeInterval,
                        startTime,
                        totalSimTime);
}

void
PrintIpv4ListToConsole(std::string listName, std::vector<Ipv4Address> list)
{
    std::stringstream stream;
    for (auto address : list)
    {
        address.Print(stream);
        stream << " ";
    }
    LogDPWSMessage("DPWS: " + listName + stream.str());
}

void
PrintASTopologyConnections(std::vector<AutonomousSystemConfiguration> aSConfigVector) {
    std::cout << "Autonomous Systems to Central Network Topology Connections:" << std::endl;
    std::cout << "\t";
    for (auto a : aSConfigVector) {
        std::cout << " (" << a.GetId() << ", " << a.GetAttachmentNodeId() << ")";
    }
    std::cout << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
}

// main script
int
main(int argc, char* argv[])
{
    // ### START TIME MEASUREMENT #################################################################
    auto nowScriptStart = std::chrono::system_clock::now();
    std::time_t clockScriptStart = std::chrono::system_clock::to_time_t(nowScriptStart);
    // simulation time resolution
    ns3::Time::SetResolution(Time::NS);

    // ### MULTI-THREADING #########################################################################
    // NOTE: using the other simulator implementation leads to memory issues
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::NullMessageSimulatorImpl"));
    // set up parallelization (https://www.nsnam.org/docs/models/html/distributed.html)
    MpiInterface::Enable(&argc, &argv);

    // ### COMMANDLINE ARGS ########################################################################
    CommandLine cmd;

    std::string configFileName = "";
    cmd.AddValue("configFile",
                 "Specify your configuration file (including file extension)",
                 configFileName);

    bool printConfiguration = false;
    cmd.AddValue("printConfiguration",
                 "Specify whether the configuration should be logged to console",
                 printConfiguration);

    bool printTopology = true;
    cmd.AddValue(
        "printTopology",
        "Specify whether the randomized central network topology should be logged to the console",
        printTopology);

    int progressLogInterval = 15;
    cmd.AddValue("progressLogInterval",
                 "Specify the interval in seconds with which progress in simulation time is logged "
                 "to the console (full numbers only)",
                 progressLogInterval);

    cmd.Parse(argc, argv);

    if (configFileName == "" || configFileName.empty())
    {
        // filename is required
        NS_FATAL_ERROR(
            "Attempted to run simulation without specifying a configuration file. Terminating");
    }
    // run with e.g.:
    // ./ns3 run dpws --command-template="mpiexec -np 2 %s" -- --configFile="sample_config.yaml"
    // NOTE: Yes, the "double" -- is intended

    // ### READ CONFIGURATION ######################################################################
    NodeLookupMapper nodeLookupMapper;
    ConfigFileReader config(configFileName, &nodeLookupMapper);

    if (MpiInterface::GetSystemId() == 0 && printConfiguration)
    {
        config.PrintConfiguration();
    }
    NS_LOG_INFO("DPWS: configuration parsing done");

    // ### LOGGING #################################################################################
    // Add the components for which you want to have logs appear in the console output. component
    // needs to be log defined, look for NS_LOG_COMPONENT_DEFINE in the corresponding .cc file
    // Only log on rank 0 when multithreading, simulation will still move in lock-step, so you
    // don't get e.g., any different timing logs for the individual ranks on the different logical
    // cores.
    if (MpiInterface::GetSystemId() == 0)
    {
        LogComponentEnable("DPWS", LOG_LEVEL_INFO); // log component for this particular file
    }

    // ### INSTANTIATE CENTRAL NETWORK #############################################################
    RandomizedPartialMeshCentralNetwork cN(config.GetCentralNetworkConfiguration());
    if (MpiInterface::GetSystemId() == 0)
    {
        // only enable tracing if instance is system with rank 0: if not multithreading, rank is
        // always 0, if multithreading, then the central network shall always be handled by rank 0.
        cN.EnablePcap(config.GetGlobalSettingsConfiguration().GetGlobalPcapPrefix());
    }
    NS_LOG_INFO("DPWS: instantiating central network done");

    // ### INSTANTIATE AUTONOMOUS SYSTEMS ##########################################################
    std::vector<AutonomousSystemConfiguration> aSConfigVector =
        config.GetAutonomousSystemConfigurations();
    std::vector<P2pAutonomousSystem> aSVector;
    int asVectorIndex = 0;

    // AddressProvider as a vehicle for one common Ipv4AddressHelper to enable all AS<->CN
    // connections to get addresses from the same subnet, to avoid consuming more address-space
    // than needed
    AddressProvider provider(
        Ipv4Address(
            config.GetGlobalSettingsConfiguration().GetASConnectionNetworkAddress().c_str()),
        Ipv4Mask(config.GetGlobalSettingsConfiguration().GetASConnectionNetworkMask().c_str()));

    // Determine how to distribute ASs onto available logical cores.
    // If #AS + 1 (for central network) > # granted logical cores, then some AS, will simply not
    // produce any traffic.
    // To avoid that scenario, if that check above evaluates to true, multiple AS will receive
    // the same rank, so the same logical core might have to handle multiple ASs.
    // This is a pretty rudimentary way of using multithreading on my part, future work may look
    // into refining this, by e.g., looking at what AS might be the most load-intensive and
    // distributing them in a more optimal manner.

    // -> STEP 1: determine if multithreading is enabled at all.
    bool mpiMultiThreading = MpiInterface::GetSize() > 1;
    std::vector<uint32_t> rankToAsIndex;

    // -> STEP 2: if multithreading, assign MPI rank to AS
    if (mpiMultiThreading)
    {
        int numLogicalCores = MpiInterface::GetSize();
        int numAS = aSConfigVector.size();
        int totalTasks = numAS + 1;
        // plus 1 for the CN, which ideally runs on its own logical core

        LogDPWSMessage("DPWS: MPI: MPI with " + std::to_string(numLogicalCores) +
                       " logical cores.");

        if (totalTasks <= numLogicalCores)
        {
            for (int i = 0; i < aSConfigVector.size(); i++)
            {
                rankToAsIndex.push_back(i + 1);
                // rank 0 reserved for central network
            }
        }
        else
        {
            NS_LOG_INFO("DPWS: MPI: not enough logical cores for the total number of tasks ("
                        << totalTasks
                        << ") (total number of tasks := #ASs + 1 for Central Network).");
            // iterate through list of ASs (symbolized by aSConfigVector) and naively assign rank
            // using modulo
            for (int i = 0; i < aSConfigVector.size(); i++)
            {
                rankToAsIndex.push_back((i + 1) % numLogicalCores);
                // retain the +1 when calculating the modulo. This way, rank 0 will be the first one
                // with multiple tasks and, if there is only one task more than available ranks,
                // the only rank with multiple tasks. This is likely desirable, given that the
                // only task special to rank 0 is traffic capturing, which is likely less costly
                // than actually generating traffic, as is the case within the ASs
            }
        }
        NS_LOG_INFO("DPWS: MPI: Task to rank assignment:");
        NS_LOG_INFO("\tCentral Network: rank 0");
        for (int i = 0; i < aSConfigVector.size(); i++)
        {
            NS_LOG_INFO("\tAS '" << aSConfigVector[i].GetId() << "': rank " << rankToAsIndex[i]);
        }
    }
    else
    {
        // if not multithreading, all done on same rank (rank 0)
        LogDPWSMessage("DPWS: MPI: Not running with MPI.");
        for (auto _ : aSConfigVector)
        {
            rankToAsIndex.push_back(0);
        }
    }

    int asIndex = 0;
    for (auto asConfig : aSConfigVector)
    {
        // create AS and assign system rank (MPI)
        P2pAutonomousSystem aS(asConfig, rankToAsIndex[asIndex]);
        asIndex += 1;

        // add to map for easier access of AS in vector when adding nodes to owner AS
        nodeLookupMapper.AddAsToAsIndexEntry(asConfig.GetId(), asVectorIndex);
        asVectorIndex += 1;
        // set up the connection to the target central network nodes
        Ptr<Node> targetNode = cN.GetNodeById(asConfig.GetAttachmentNodeId());
        if (targetNode)
        {
            aS.ConnectToNode(targetNode, &provider);
        }
        else
        {
            NS_FATAL_ERROR("Attempt to connect AS (" + asConfig.GetId() +
                           ") to central network node with id: " + asConfig.GetAttachmentNodeId() +
                           ", but no node with that ID found on the central network. Double check "
                           "your configuration");
        }
        aS.EnablePcap(config.GetGlobalSettingsConfiguration().GetGlobalPcapPrefix());
        aSVector.push_back(aS);
    }
    NS_LOG_INFO("DPWS: instantiating autonomous systems done");

    // ### SETUP TARGET SERVERS ####################################################################
    std::vector<ServerNodeConfiguration> tSConfigVector =
        config.GetTargetServerNodeConfigurations();

    for (auto sConfig : tSConfigVector)
    {
        aSVector[nodeLookupMapper.GetAsIndexByAsId(sConfig.GetOwnerAS())].CreateTargetServerNode(
            sConfig);
    }
    NS_LOG_INFO("DPWS: creating target server nodes done");

    // ### SETUP NON-TARGET SERVERS ################################################################
    std::vector<ServerNodeConfiguration> nTSConfigVector =
        config.GetNonTargetServerNodeConfigurations();

    for (auto sConfig : nTSConfigVector)
    {
        aSVector[nodeLookupMapper.GetAsIndexByAsId(sConfig.GetOwnerAS())].CreateNonTargetServerNode(
            sConfig);
    }
    NS_LOG_INFO("DPWS: creating non-target server nodes done");

    // ### SETUP BENIGN CLIENTS ####################################################################
    std::vector<BenignNodeConfiguration> bCConfigVector = config.GetBenignNodeConfigurations();

    for (auto bConfig : bCConfigVector)
    {
        // retrieve server connection information for configured peer
        std::pair<Ipv4Address, int> peerServerInfo =
            aSVector[nodeLookupMapper.GetAsIndexByNodeId(bConfig.GetPeer())]
                .GetHttpConnectionInfoByNodeId(bConfig.GetPeer());

        aSVector[nodeLookupMapper.GetAsIndexByAsId(bConfig.GetOwnerAS())].CreateBenignClientNode(
            bConfig,
            peerServerInfo);
    }
    NS_LOG_INFO("DPWS: creating benign nodes done");

    // ### SETUP ATTACKER NODES ####################################################################
    std::vector<AttackerNodeConfiguration> aNConfigVector = config.GetAttackerNodeConfigurations();
    // create targetList
    std::vector<Ipv4Address> targetList;
    for (auto tNConfig : config.GetTargetServerNodeConfigurations())
    {
        targetList.push_back(
            aSVector[nodeLookupMapper.GetAsIndexByNodeId(tNConfig.GetNodeId())].GetIpv4ByNodeId(
                tNConfig.GetNodeId()));
    }
    // create schedule helper (all attacker nodes share same scheduling)
    AttackScheduleHelper scheduleHelper(config.GetGlobalSettingsConfiguration().GetAttackVectors(),
                                        targetList.size());

    for (auto aConfig : aNConfigVector)
    {
        aSVector[nodeLookupMapper.GetAsIndexByAsId(aConfig.GetOwnerAS())].CreateAttackerNode(
            aConfig,
            targetList,
            &scheduleHelper);
    }
    // create attacker ivp4 list to debug to console
    std::vector<Ipv4Address> attackerList;
    attackerList.reserve(aNConfigVector.size());
    for (auto aConfig : aNConfigVector)
    {
        attackerList.push_back(aSVector[nodeLookupMapper.GetAsIndexByNodeId(aConfig.GetNodeId())]
                                   .GetIpv4ByNodeId(aConfig.GetNodeId(), DPWSNodeType::attacker));
    }
    NS_LOG_INFO("DPWS: creating attacker nodes done");

    // ### START APPLICATIONS ######################################################################
    for (auto AS : aSVector)
    {
        AS.StartApplications(0.0, config.GetGlobalSettingsConfiguration().GetSimDuration());
    }
    NS_LOG_INFO("DPWS: issuing start commands to all applications done");

    // ### POPULATE ROUTING TABLES #################################################################
    // build global view of topology and create routing tables automatically. ecmp not enabled.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    NS_LOG_INFO("DPWS: global routing tables calculation done");

    // ### START SIMULATION ########################################################################
    // schedule the stop before running
    Simulator::Stop(Seconds(config.GetGlobalSettingsConfiguration().GetSimDuration()));

    // set up some timing logs
    auto nowSimStart = std::chrono::system_clock::now();
    std::time_t clockSimStart = std::chrono::system_clock::to_time_t(nowSimStart);

    // start progress logs scheduling chain
    LogDPWSMessage("DPWS: Started simulation run. Please be patient.");
    Simulator::Schedule(Seconds(progressLogInterval),
                        &ScheduleProgressLog,
                        progressLogInterval,
                        clockSimStart,
                        config.GetGlobalSettingsConfiguration().GetSimDuration());

    // start simulation
    Simulator::Run();

    // calculate total running time of the simulation
    auto nowEnd = std::chrono::system_clock::now();
    std::time_t clockEnd = std::chrono::system_clock::to_time_t(nowEnd);
    std::chrono::duration<double> elapsedSimSeconds = nowEnd - nowSimStart;
    // calculate total running time of the entire script
    std::chrono::duration<double> elapsedScriptSeconds = nowEnd - nowScriptStart;
    // print time stamps
    LogDPWSMessage("DPWS: started script at: " + GetTimeString(&clockScriptStart));
    LogDPWSMessage("DPWS: started simulation at: " + GetTimeString(&clockSimStart));
    LogDPWSMessage("DPWS: finished simulation (and script) at: " + GetTimeString(&clockEnd));
    LogDPWSMessage("DPWS: total running time of the simulation (s): " +
                   std::to_string(elapsedSimSeconds.count()));
    LogDPWSMessage("DPWS: total running time of the entire script (s): " +
                   std::to_string(elapsedScriptSeconds.count()));
    // print auxiliary information that should make processing of resulting pcaps easier
    PrintIpv4ListToConsole("Target IP Addresses: ", targetList);
    PrintIpv4ListToConsole("Attacker IP Addresses: ", attackerList);

    if (printTopology && MpiInterface::GetSystemId() == 0)
    {
        cN.PrintTopology();
        PrintASTopologyConnections(aSConfigVector);
    }

    // first sleep for a few seconds to ensure the system has enough time to finish up before starting
    // the teardown. without this you MAY encounter an error related to MPI shared memory access:
    // "A system call failed during shared memory initialization that should not have"
    sleep(5);
    // terminate simulator, resolve multithreading
    Simulator::Destroy();
    MpiInterface::Disable();
}