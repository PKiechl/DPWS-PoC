#include "randomized-partial-mesh-central-network.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RandomizedPartialMeshCentralNetwork");

RandomizedPartialMeshCentralNetwork::RandomizedPartialMeshCentralNetwork(
    ns3::CentralNetworkConfiguration config)
{
    m_config = config;
    m_numNodes = m_config.GetNodes().size();
    m_numConnectionsForMinimalTopology = m_numNodes - 1;

    // calculate number of additional connects based on configured degree of redundancy
    // degree 0 -> remain at minimal topology, 0 new connections
    // degree 1 -> same number of connections as if full-mesh
    // degree > 1 -> more connections than if full-mesh
    int numConnectionsForFullMesh = ((m_numNodes * (m_numNodes - 1)) / 2); // N*(N-1)/2
    int numAdditionalConnectionsIfFullMesh =
        numConnectionsForFullMesh - m_numConnectionsForMinimalTopology;
    // N*(N-1)/2 minus the already created connections when building minimal topology
    m_numAdditionalConnections =
        std::floor(numAdditionalConnectionsIfFullMesh * m_config.GetDegreeOfRedundancy());
    // using floor instead of ceil, to avoid automatic conversion to full mesh with 3 nodes when
    // not exactly using degree 0

    // instantiate randomizer -> https://stackoverflow.com/a/7560564
    // set range to cover all node indices in m_node, such that each node index is drawable
    std::mt19937 gen(m_config.GetTopologySeed());
    std::uniform_int_distribution<> distr(0, m_numNodes - 1);
    m_generator = gen;
    m_distribution = distr;

    PopulateNodeIdToIndexMap();
    PopulateRandomizedMinimalTopologyConnectionsVector();
    if (m_numAdditionalConnections != 0)
    {
        PopulateRandomizedAdditionalConnectionsVector();
    }
    BuildTopology();
}

void
RandomizedPartialMeshCentralNetwork::BuildTopology()
{
    // Create Nodes
    NodeContainer nodes;

    if (m_numNodes == 0)
    {
        /*
         * 0 -> fatal because operations like GetNodeById etc. would all have to implement special
         *      cases to support no nodes being present. also makes no sense semantically.
         */
        NS_FATAL_ERROR(
            "Attempt to construct RandomizedPartialMeshCentralNetwork with 0 nodes. Aborting.");
        return;
    }

    if (m_numNodes == 1)
    {
        /*
         * 1 -> fatal because this class relies on point-to-point mechanics, which requires 2 nodes
         * (at least)
         */
        NS_FATAL_ERROR(
            "Cannot construct RandomizedPartialMeshCentralNetwork with 1 node. Underlying "
            "Point-to-Point mechanics require at least 2 nodes. If you just want one "
            "central node for your scenario, then instantiate it directly instead of "
            "using this class. (or try to build in support here for just 1 node)");
    }

    // central network is always on rank 0, no matter if multithreading or not
    nodes.Create(m_numNodes, 0);

    InternetStackHelper stack;
    stack.Install(nodes);

    // set up collections for topology elements (both the minimal, and the additional,
    // superfluous connections
    std::vector<NetDeviceContainer> deviceVector;
    std::vector<Ipv4InterfaceContainer> interfaceVector;

    // STAGE 1:
    // Establish minimal partial mesh, i.e., create a network with m_numNodes-1 point-to-point
    // connections based on random-draws resulting in a random topology where each node is reachable
    // from each other node.
    StringValue dataRate(m_config.GetBandwidth());
    StringValue delay(m_config.GetDelay());

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", dataRate);
    p2p.SetChannelAttribute("Delay", delay);

    // set up for address assignments
    Ipv4AddressHelper address;
    Ipv4Address addressBase = m_config.GetNetworkAddress().c_str();
    Ipv4Mask addressMask = m_config.GetNetworkMask().c_str();
    address.SetBase(addressBase, addressMask);

    for (int i = 0; i < m_numConnectionsForMinimalTopology; i++)
    {
        // iterate through the pairs of random draws in the minimal topology and create actual
        // connection
        std::pair<int, int> currentPair = m_randomizedMinimalTopologyConnections[i];
        int firstIndex = currentPair.first;
        int secondIndex = currentPair.second;

        NetDeviceContainer devices = p2p.Install(nodes.Get(firstIndex), nodes.Get(secondIndex));
        Ipv4InterfaceContainer interfaces = address.Assign(devices);
        address.NewNetwork();
        // calling NewNetwork to increment the 3rd position of the address, resulting in the
        // following pattern:
        //
        // Node X<->Node Y:    Node X Ipv4: nnn.mmm.uuu.1, Node Y Ipv4: nnn.mmm.uuu.2
        // Node X<->Node Z:    Node X Ipv4: nnn.mmm.uuu+1.1, Node Z Ipv4: nnn.mmm.uuu+1.2
        // ...
        // this was required, in the p2p implementations of the autonomous system
        // contrib/distributed-pulse-wave-simulator/wrapper/autonomous-systems/implementations/p2p-autonomous-system.cc
        // as otherwise there were severe issues with TTL on most packets, presumably resulting from
        // some routing related issue.
        // Just to be on the safe side, I do the same here (although I have not actually been able
        // to identify any routing issues without in this central network, but still, best to be
        // safe.
        deviceVector.push_back(devices);
        interfaceVector.push_back(interfaces);

        // also update the tracking vector that tracks relationship of nodes' Ids for a given
        // pair in the deviceVector
        m_deviceToNodeIndexTrackingVector.push_back(std::pair<int, int>(firstIndex, secondIndex));
    }

    NS_LOG_DEBUG("minimal topology with " + std::to_string(m_numNodes) + " nodes and " +
                 std::to_string(m_numConnectionsForMinimalTopology) + " connections:");

    // StAGE 2:
    // add the additional, connections to the topology, which based on the degree of connectedness
    // configured can "upgrade" the minimal topology all the way to a full mesh.
    if (m_numAdditionalConnections != 0)
    {
        for (int i = 0; i < m_numAdditionalConnections; i++)
        {
            // same procedure as with minimal topology
            std::pair<int, int> currentPair = m_randomizedAdditionalTopologyConnections[i];
            int firstIndex = currentPair.first;
            int secondIndex = currentPair.second;

            NetDeviceContainer devices = p2p.Install(nodes.Get(firstIndex), nodes.Get(secondIndex));
            Ipv4InterfaceContainer interfaces = address.Assign(devices);
            deviceVector.push_back(devices);
            interfaceVector.push_back(interfaces);

            // also update the tracking vector that tracks relationship of nodes' Ids for a given
            // pair in the deviceVector
            m_deviceToNodeIndexTrackingVector.push_back(
                std::pair<int, int>(firstIndex, secondIndex));
        }

        NS_LOG_DEBUG("Final topology with " + std::to_string(m_numNodes) + " nodes and " +
                     std::to_string(m_numAdditionalConnections) +
                     " additional connections beyond the minimal topology.");
    }

    //     thanks to continuously merging device containers, address assignment is simple despite
    //     the many p2p connections. Addresses will be consumed on each node for each interface, but
    //     given that the addresses here are considered transparent (won't show in any pcap) that is
    //     fine

    // write to class members
    m_nodes = nodes;
    m_deviceVector = deviceVector;
    m_interfaceVector = interfaceVector;
}

void
RandomizedPartialMeshCentralNetwork::EnablePcap(std::string prefix)
{
    // ns3 automatically adds a pair of numbers that are somewhat cryptic and seem to have to do
    // with the indices of interfaces or NetDevices. There appears to be no way to remove them, so
    // I'm adding an additional spacer to make the actually semantically useful part of the filename
    // more clearly distinguishable from the appended stuff.
    std::string semanticSpacer = "____";

    // create reverse map of m_NodeIdToContainerIndexMap to enable lookup by value instead of key
    std::unordered_map<int, std::string> reverseMap;
    for (std::pair<std::string, int> kv : m_nodeIdToContainerIndexMap)
    {
        reverseMap[kv.second] = kv.first;
    }

    // can seemingly use EnablePcap method of any channel instance for this
    PointToPointHelper anyChannel;

    // iterate through each pair of devices and set the corresponding nodeId and the nodeId it
    // connects to as part of the filename. Devices are created in pairs when using point-to-point
    // channels and the pair is bidirectional. E.g., at indices 0 and 1 of the merged device
    // container a pair of devices that belong to the same channel is found.
    // Each device "belongs" to a node, so the label then at index 0 and 1 is:
    // index 0 -> "{nodeId of node that that owns device 0}-to-{nodeId of node that owns device 1}"
    // index 1 -> "{nodeId of node that that owns device 1}-to-{nodeId of node that owns device 0}"
    for (int i = 0; i < m_deviceVector.size(); i++)
    {
        // get the device container
        NetDeviceContainer devices = m_deviceVector[i];
        // get the pair of m_node related indices at same index i
        auto [first, second] = m_deviceToNodeIndexTrackingVector[i];
        // get their respective nodeIds
        std::string firstId = reverseMap[first];
        std::string secondId = reverseMap[second];

        // enable pcap for first device
        std::string midSection = "__" + firstId + "-to-" + secondId;
        anyChannel.EnablePcap(prefix + midSection + semanticSpacer, devices.Get(0), false);

        // enable pcap for second device, swapping the positions of the ids accordingly
        midSection = "__" + secondId + "-to-" + firstId;
        anyChannel.EnablePcap(prefix + midSection + semanticSpacer, devices.Get(1), false);
    }

    // NOTE: in principle, the output at both devices in a given channel is ~identical, so it is
    // tempting to argue, that only one file per device-pair should be produced, thus saving
    // performance and storage space. However, the two pcap files have differed ever so slightly in
    // two ways based on my observation (and based on what logic dictates):
    // 1. the last few packets of one file may not be present in the other
    // 2. the time-stamps are ever so slightly different
    // both of these factors have to do with the per-channel delay. If the user configures a very
    // large delay, these differences MAY become large enough that using the file of the peer node
    // may falsify some analytics.
}

int
RandomizedPartialMeshCentralNetwork::DrawRandomNodeIndex()
{
    return m_distribution(m_generator);
}

void
RandomizedPartialMeshCentralNetwork::PopulateRandomizedAdditionalConnectionsVector()
{
    for (int i = 0; i < m_numAdditionalConnections; i++)
    {
        int firstIndex = DrawRandomNodeIndex();
        int secondIndex = DrawRandomNodeIndex();
        while (firstIndex == secondIndex)
        {
            // prevent self-to-self connection
            secondIndex = DrawRandomNodeIndex();
        }
        m_randomizedAdditionalTopologyConnections.push_back(
            std::pair<int, int>(firstIndex, secondIndex));
    }
}

void
RandomizedPartialMeshCentralNetwork::PopulateRandomizedMinimalTopologyConnectionsVector()
{
    // using sets for easy checking if index already drawn
    std::set<int> alreadyDrawn;
    std::set<int> notYetDrawn;
    for (int i = 0; i < m_numNodes; i++)
    {
        notYetDrawn.emplace(i);
    }

    // draw one random index from each set (not directly, because set's don't really support this)
    // this means that with each connection that is formed, one new node is connected to a random
    // node that is already in the topology.
    // The first connection that is drawn from alreadyDrawn set is accepted no matter what, since
    // that set is initially empty
    for (int i = 0; i < m_numConnectionsForMinimalTopology; i++)
    {
        int firstIndex;
        int secondIndex;
        bool isInTargetSet = false;

        // draw for firstIndex from alreadyDrawn set
        while (!isInTargetSet)
        {
            firstIndex = DrawRandomNodeIndex();
            if (alreadyDrawn.empty())
            {
                // this set is initially empty, so just accept first draw as is. on top of that move
                // the draw to the alreadyDraw set to ensure the logic holds for future iterations
                // and that the same number cannot be drawn again when performing the draw for the
                // secondIndex in this particular iteration.
                isInTargetSet = true;
                notYetDrawn.erase(firstIndex);
                alreadyDrawn.emplace(firstIndex);
            }
            else
            {
                if (alreadyDrawn.count(firstIndex) != 0)
                {
                    isInTargetSet = true;
                }
            }
        }

        isInTargetSet = false;
        // draw for secondIndex from notYetDrawn set
        while (!isInTargetSet)
        {
            secondIndex = DrawRandomNodeIndex();
            if (notYetDrawn.count(secondIndex) != 0)
            {
                isInTargetSet = true;
            }
        }
        // move draw to alreadyDrawn set
        notYetDrawn.erase(secondIndex);
        alreadyDrawn.emplace(secondIndex);

        m_randomizedMinimalTopologyConnections.push_back(
            std::pair<int, int>(firstIndex, secondIndex));
    }
}

void
RandomizedPartialMeshCentralNetwork::PrintTopology()
{
    // for each pair of nodes that have a direct connection, get the corresponding nodeIds from the
    // configuration and print each connection

    // create reverse map of m_NodeIdToContainerIndexMap to enable lookup by value instead of key
    std::unordered_map<int, std::string> reverseMap;
    for (std::pair<std::string, int> kv : m_nodeIdToContainerIndexMap)
    {
        reverseMap[kv.second] = kv.first;
    }

    // traverse set of connections and output the connection as a pair of nodeIds via reverseMap
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "Central Network Topology:" << std::endl;
    std::cout << "\tMinimal Topology Connections: (" << m_numConnectionsForMinimalTopology << ")"
              << std::endl;
    std::cout << "\t\t";
    for (std::pair<int, int> nodeIndices : m_randomizedMinimalTopologyConnections)
    {
        std::cout << " (" << reverseMap[nodeIndices.first] << ", "
                  << reverseMap[nodeIndices.second] << ")";
    }
    std::cout << std::endl;
    std::cout << "Additional Redundant Connections: (" << m_numAdditionalConnections << ")"
              << std::endl;
    std::cout << "\t\t";
    for (std::pair<int, int> nodeIndices : m_randomizedAdditionalTopologyConnections)
    {
        std::cout << " (" << reverseMap[nodeIndices.first] << ", "
                  << reverseMap[nodeIndices.second] << ")";
    }
    std::cout << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
}

void
RandomizedPartialMeshCentralNetwork::PopulateNodeIdToIndexMap()
{
    std::vector<NodeConfiguration> nodes = m_config.GetNodes();
    //    in this central network, the nodes are all instantiated at once and are treated equally,
    //    hence can simply mapped by their order as found in config. The key is to have each node be
    //    associated with a given nodeId, which this achieves.
    int index = 0;
    for (NodeConfiguration node : nodes)
    {
        m_nodeIdToContainerIndexMap[node.GetNodeId()] = index;
        index++;
    }
}

Ptr<Node>
RandomizedPartialMeshCentralNetwork::GetNodeById(std::string id)
{
    int index = m_nodeIdToContainerIndexMap.at(id);
    return m_nodes.Get(index);
}