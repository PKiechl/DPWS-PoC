#include "config-file-reader.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ConfigFileReader");

// ####### HELPERS FROM RYML #######################################################################
/*
 * NOTE: file_get_contents template functions directly from the author (https://github.com/biojppm)
 * of the rapidyaml library
 *
 * Functions taken from https://github.com/biojppm/rapidyaml/blob/master/samples/quickstart.cpp
 * starting from line 4325 (mater branch, repo state July 5th, 2023)
 */

C4_SUPPRESS_WARNING_MSVC_WITH_PUSH(4996) // fopen: this function or variable may be unsafe

/** load a file from disk and return a newly created CharContainer */
template <class CharContainer>
size_t
file_get_contents(const char* filename, CharContainer* v)
{
    ::FILE* fp = ::fopen(filename, "rb");
    C4_CHECK_MSG(fp != nullptr, "could not open file");
    ::fseek(fp, 0, SEEK_END);
    long sz = ::ftell(fp);
    v->resize(static_cast<typename CharContainer::size_type>(sz));
    if (sz)
    {
        ::rewind(fp);
        size_t ret = ::fread(&(*v)[0], 1, v->size(), fp);
        C4_CHECK(ret == (size_t)sz);
    }
    ::fclose(fp);
    return v->size();
}

template <class CharContainer>
CharContainer
file_get_contents(const char* filename)
{
    CharContainer cc;
    file_get_contents(filename, &cc);
    return cc;
}

// ####### READER ##################################################################################
ConfigFileReader::ConfigFileReader(std::string fileName)
{
    NS_LOG_DEBUG("configFile name: " + fileName);

    std::string contents = file_get_contents<std::string>(fileName.c_str());
    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(contents));

    m_fileName = fileName;
    m_rawTree = tree;

    ParseConfiguration();
}

ConfigFileReader::ConfigFileReader(std::string fileName, Ptr<NodeLookupMapper> lookupMapper)
{
    NS_LOG_DEBUG("configFile name: " + fileName + ". Using mapper for easier node lookups.");

    std::string contents = file_get_contents<std::string>(fileName.c_str());
    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(contents));

    m_fileName = fileName;
    m_rawTree = tree;
    m_lookupMapper = lookupMapper;

    ParseConfiguration();
}

CentralNetworkConfiguration
ConfigFileReader::GetCentralNetworkConfiguration()
{
    return m_centralNetwork;
}

GlobalSettingsConfiguration
ConfigFileReader::GetGlobalSettingsConfiguration()
{
    return m_globalSettings;
}

std::vector<ServerNodeConfiguration>
ConfigFileReader::GetTargetServerNodeConfigurations()
{
    return m_targetServerNodes;
}

std::vector<ServerNodeConfiguration>
ConfigFileReader::GetNonTargetServerNodeConfigurations()
{
    return m_nonTargetServerNodes;
}

std::vector<AttackerNodeConfiguration>
ConfigFileReader::GetAttackerNodeConfigurations()
{
    return m_attackerNodes;
}

std::vector<BenignNodeConfiguration>
ConfigFileReader::GetBenignNodeConfigurations()
{
    return m_benignNodes;
}

std::vector<AutonomousSystemConfiguration>
ConfigFileReader::GetAutonomousSystemConfigurations()
{
    return m_autonomousSystems;
}

void
ConfigFileReader::ParseConfiguration()
{
    NS_LOG_DEBUG("\nParsing configuration...");
    ParseGlobalSettingsConfiguration();
    ParseCentralNetworkConfiguration();
    ParseAutonomousSystemsConfiguration();
    ParseTargetServerNodesConfiguration();
    ParseNonTargetServerNodesConfiguration();
    ParseAttackerNodesConfiguration();
    ParseBenignNodesConfiguration();
    UpdateASNodeCounterOnAS();
}

void
ConfigFileReader::ParseGlobalSettingsConfiguration()
{
    // check if global settings configured
    ryml::ConstNodeRef root = m_rawTree;
    if (!root.has_child("global_settings"))
    {
        NS_FATAL_ERROR("Configuration does not contain any 'global_settings' key.");
        return;
    }

    root = root["global_settings"];
    NS_LOG_DEBUG("\nParsing global settings configuration...");

    std::vector<AttackVectorConfiguration> attackVectors;
    if (root.has_child("attack"))
    {
        if (root["attack"].has_child("attack_vectors"))
        {
            for (ryml::ConstNodeRef av : root["attack"]["attack_vectors"].children())
            {
                if (av.has_child("type"))
                {
                    AttackVectorConfiguration aVConfig(ToString(av["type"].val()));
                    // parse optionals of attack vector
                    if (av.has_child("burst_duration_s"))
                    {
                        aVConfig.SetBurstDuration(
                            std::stod(ToString(av["burst_duration_s"].val())));
                    }
                    if (av.has_child("target_switch_duration_s"))
                    {
                        aVConfig.SetTargetSwitchDuration(
                            std::stod(ToString(av["target_switch_duration_s"].val())));
                    }
                    if (av.has_child("data_rate"))
                    {
                        aVConfig.SetDataRate(ToString(av["data_rate"].val()));
                    }
                    if (av.has_child("packet_size"))
                    {
                        aVConfig.SetPacketSize(std::stoi(ToString(av["packet_size"].val())));
                    }
                    if (av.has_child("destination_port"))
                    {
                        aVConfig.SetDestinationPort(std::stoi(ToString(av["destination_port"].val())));
                    }
                    if (av.has_child("source_port"))
                    {
                        aVConfig.SetSourcePort(std::stoi(ToString(av["source_port"].val())));
                    }

                    attackVectors.push_back(aVConfig);
                }
                else
                {
                    NS_FATAL_ERROR("Encountered Attack Vector without 'type' field. Fatal.");
                }
            }
        }
        else
        {
            NS_LOG_WARN("No 'attack_vectors' configured in 'attack' config.");
        }
    }
    else
    {
        NS_LOG_DEBUG("No attack settings configured in 'global_settings'.");
    }

    GlobalSettingsConfiguration config(attackVectors);

    // parsing optional values
    if (root.has_child("capture"))
    {
        if (root["capture"].has_child("pcap_prefix"))
        {
            config.SetGlobalPcapPrefix(ToString(root["capture"]["pcap_prefix"].val()));
        }
        else
        {
            NS_LOG_DEBUG("No global pcap prefix configured, using default: " +
                         config.GetGlobalPcapPrefix());
        }
    }
    else
    {
        NS_LOG_DEBUG("No capture settings configured in 'global_settings'.");
    }

    if (root.has_child("attack"))
    {
        ryml::ConstNodeRef tempRoot = root["attack"];
        if (tempRoot.has_child("burst_duration_s"))
        {
            config.SetBurstDuration(std::stod(ToString(tempRoot["burst_duration_s"].val())));
        }
        else
        {
            NS_LOG_DEBUG("No burst duration configured, using default (s): " +
                         std::to_string(config.GetBurstDuration()));
        }

        if (tempRoot.has_child("target_switch_duration_s"))
        {
            config.SetTargetSwitchDuration(
                std::stod(ToString(tempRoot["target_switch_duration_s"].val())));
        }
        else
        {
            NS_LOG_DEBUG("No target switch duration configured, using default (s): " +
                         std::to_string(config.GetTargetSwitchDuration()));
        }
    }

    // update individual attack vectors with global 'attack' settings if not already provided on a
    // per-vector basis
    config.ResolveGlobalAndPerVectorAttackSettings();

    if (root.has_child("scheduling"))
    {
        if (root["scheduling"].has_child("simulation_duration_s"))
        {
            config.SetSimDuration(
                std::stod(ToString(root["scheduling"]["simulation_duration_s"].val())));
        }
        else
        {
            NS_LOG_DEBUG("No simulation duration configured, using default (s) " +
                         std::to_string(config.GetSimDuration()));
        }
    }
    else
    {
        NS_LOG_DEBUG("No scheduling settings configured in 'global_settings'");
    }

    if (root.has_child("autonomous_systems_connections"))
    {
        ryml::ConstNodeRef subRoot = root["autonomous_systems_connections"];
        if (subRoot.has_child("network_address"))
        {
            config.SetASConnectionNetworkAddress(ToString(subRoot["network_address"].val()));
        }
        else
        {
            NS_LOG_DEBUG("No network address base set for AS connections. Using default " +
                         config.GetASConnectionNetworkAddress());
        }
        if (subRoot.has_child("network_mask"))
        {
            config.SetASConnectionNetworkMask(ToString(subRoot["network_mask"].val()));
        }
        else
        {
            NS_LOG_DEBUG("No network mask set for AS connections. Using default " +
                         config.GetASConnectionNetworkMask());
        }
    }
    else
    {
        NS_LOG_DEBUG("No global Autonomous System Connection Settings configured in "
                     "'autonomous_systems_connections'");
    }

    m_globalSettings = config;
}

void
ConfigFileReader::ParseCentralNetworkConfiguration()
{
    // check if central network configured
    ryml::ConstNodeRef root = m_rawTree;
    if (!root.has_child("central_network"))
    {
        /* there is a case to be made for this to not throw a fatal error, e.g., when setting up a
         * scenario where there is no central network or when you want to manually set up a central
         * network in the main script still, probably better to just not use this config reader and
         * build a different one then, so I'm leaving it as is for now
         */
        NS_FATAL_ERROR("Configuration does not contain any 'central_network' key.");
        return;
    }

    root = root["central_network"];
    NS_LOG_DEBUG("\nParsing central network configuration...");

    std::vector<NodeConfiguration> centralNetworkNodes;
    for (ryml::ConstNodeRef node : root["nodes"].children())
    {
        if (node.has_child("id"))
        {
            NodeConfiguration n(ToString(node["id"].val()));
            centralNetworkNodes.push_back(n);
            // add to validation vector
            m_validCentralNetworkNodeIds.emplace(n.GetNodeId());
        }
        else
        {
            NS_LOG_WARN("NodeConfiguration without 'id' detected. Ignoring node.");
        }
    }

    CentralNetworkConfiguration config(centralNetworkNodes);
    // parsing optional values
    if (root.has_child("topology_seed"))
    {
        config.SetTopologySeed(std::stoi(ToString(root["topology_seed"].val())));
    }
    else
    {
        NS_LOG_DEBUG("No topology seed configured, using default value: " +
                     std::to_string(config.GetTopologySeed()));
    }

    if (root.has_child("network_address"))
    {
        config.SetNetworkAddress(ToString(root["network_address"].val()));
    }
    else
    {
        NS_LOG_DEBUG("No network address configured, using default value: " +
                     config.GetNetworkAddress());
    }

    if (root.has_child("network_mask"))
    {
        config.SetNetworkMask(ToString(root["network_mask"].val()));
    }
    else
    {
        NS_LOG_DEBUG("No network mask configured, using default value: " + config.GetNetworkMask());
    }

    if (root.has_child("bandwidth"))
    {
        config.SetBandwidth(ToString(root["bandwidth"].val()));
    }
    else
    {
        NS_LOG_DEBUG("No bandwidth configured, using default value: " + config.GetBandwidth());
    }

    if (root.has_child("delay"))
    {
        config.SetDelay(ToString(root["delay"].val()));
    }
    else
    {
        NS_LOG_DEBUG("No delay configured, using default value: " + config.GetDelay());
    }

    if (root.has_child("degree_of_redundancy"))
    {
        double degree = std::stod(ToString(root["degree_of_redundancy"].val()));
        if (degree < 0.0)
        {
            NS_LOG_WARN("Detected invalid value for 'degree_of_redundancy'. Specify a value "
                        "positive value. Ignoring value and remaining at the default degree "
                        "of redundancy of " +
                        std::to_string(config.GetDegreeOfRedundancy()));
        }
        else
        {
            config.SetDegreeOfRedundancy(degree);
        }
    }
    else
    {
        NS_LOG_DEBUG("No degree of redundancy configured, using default value: " +
                     std::to_string(config.GetDegreeOfRedundancy()));
    }

    m_centralNetwork = config;
}

void
ConfigFileReader::ParseTargetServerNodesConfiguration()
{
    // check if target server nodes configured
    ryml::ConstNodeRef root = m_rawTree;
    if (!root.has_child("target_server_nodes"))
    {
        NS_LOG_DEBUG("Configuration does not contain any 'target_server_nodes' key.");
        return;
    }

    root = root["target_server_nodes"];
    NS_LOG_DEBUG("\nParsing target server nodes' configuration...");

    std::vector<ServerNodeConfiguration> serverNodes;
    for (ryml::ConstNodeRef sNode : root.children())
    {
        if (sNode.has_child("id") && sNode.has_child("owner_as"))
        {
            std::string asId = ToString(sNode["owner_as"].val());
            if (m_validAutonomousSystemIds.count(asId) == 0)
            {
                NS_FATAL_ERROR("Detected Target Server Node configuration with invalid 'owner_as': "
                               << asId << ". No AS found in configuration with that id.");
            }
            ServerNodeConfiguration sN(ToString(sNode["id"].val()), asId);
            // update AS node counter
            AddOrIncrementASNodeCounter(sN.GetOwnerAS());
            // update mapper if present
            if (m_lookupMapper)
            {
                m_lookupMapper->AddNodeToAsEntry(sN.GetNodeId(), sN.GetOwnerAS());
            }
            // parse optionals
            if (sNode.has_child("http_server_port"))
            {
                sN.SetHttpServerPort(std::stoi(ToString(sNode["http_server_port"].val())));
            }
            serverNodes.push_back(sN);
            // also update validation list
            m_validServerNodeIds.emplace(sN.GetNodeId());
        }
        else
        {
            NS_LOG_WARN("ServerNodeConfiguration with faulty config detected. Requires 'id' and "
                        "'owner_as'. Ignoring node.");
        }
    }

    m_targetServerNodes = serverNodes;
}

void
ConfigFileReader::ParseNonTargetServerNodesConfiguration()
{
    // check if non-target server nodes configured
    ryml::ConstNodeRef root = m_rawTree;
    if (!root.has_child("non_target_server_nodes"))
    {
        NS_LOG_DEBUG("Configuration does not contain any 'non_target_server_nodes' key.");
        return;
    }

    root = root["non_target_server_nodes"];
    NS_LOG_DEBUG("\nParsing non-target server nodes' configuration...");
    std::vector<ServerNodeConfiguration> serverNodes;
    for (ryml::ConstNodeRef nSNode : root.children())
    {
        if (nSNode.has_child("id") && nSNode.has_child("owner_as"))
        {
            std::string asId = ToString(nSNode["owner_as"].val());
            if (m_validAutonomousSystemIds.count(asId) == 0)
            {
                NS_FATAL_ERROR(
                    "Detected Non-Target Server Node configuration with invalid 'owner_as': "
                    << asId << ". No AS found in configuration with that id.");
            }
            ServerNodeConfiguration nSN(ToString(nSNode["id"].val()), asId);
            // update AS node counter
            AddOrIncrementASNodeCounter(nSN.GetOwnerAS());
            // update mapper if present
            if (m_lookupMapper)
            {
                m_lookupMapper->AddNodeToAsEntry(nSN.GetNodeId(), nSN.GetOwnerAS());
            }
            // parse optionals
            if (nSNode.has_child("http_server_port"))
            {
                nSN.SetHttpServerPort(std::stoi(ToString(nSNode["http_server_port"].val())));
            }
            serverNodes.push_back(nSN);
            // also update validation list
            m_validServerNodeIds.emplace(nSN.GetNodeId());
        }
        else
        {
            NS_LOG_WARN("ServerNodeConfiguration with fault config detected. Requires 'id' and "
                        "'owner_as'. Ignoring node.");
        }
    }

    m_nonTargetServerNodes = serverNodes;
}

void
ConfigFileReader::ParseAttackerNodesConfiguration()
{
    // check if any attacker nodes configured
    ryml::ConstNodeRef root = m_rawTree;
    if (!root.has_child("attacker_nodes"))
    {
        NS_LOG_WARN("Configuration does not contain any 'attacker_nodes' key.");
        return;
    }

    root = root["attacker_nodes"];
    NS_LOG_DEBUG("\nParsing attacker node configurations...");

    std::vector<AttackerNodeConfiguration> attackerNodes;
    for (ryml::ConstNodeRef aNode : root.children())
    {
        if (aNode.has_child("id") && aNode.has_child("owner_as"))
        {
            std::string asId = ToString(aNode["owner_as"].val());
            if (m_validAutonomousSystemIds.count(asId) == 0)
            {
                NS_FATAL_ERROR("Detected Attacker Node configuration with invalid 'owner_as': "
                               << asId << ". No AS found in configuration with that id.");
            }
            AttackerNodeConfiguration aN(ToString(aNode["id"].val()), asId);
            // update AS node counter
            AddOrIncrementASNodeCounter(aN.GetOwnerAS());
            // update Mapper if present
            if (m_lookupMapper)
            {
                m_lookupMapper->AddNodeToAsEntry(aN.GetNodeId(), aN.GetOwnerAS());
            }
            // parse optionals
            if (aNode.has_child("data_rate"))
            {
                aN.SetDataRate(ToString(aNode["data_rate"].val()));
            }
            if (aNode.has_child("packet_size"))
            {
                aN.SetPacketSize(std::stoi(ToString(aNode["packet_size"].val())));
            }
            if (aNode.has_child("destination_port"))
            {
                aN.SetDestinationPort(std::stoi(ToString(aNode["destination_port"].val())));
            }
            if (aNode.has_child("source_port"))
            {
                aN.SetSourcePort(std::stoi(ToString(aNode["source_port"].val())));
            }
            if (aNode.has_child("max_data_rate_fluctuation"))
            {
                aN.SetMaxDataRateFluctuation(std::stod(ToString(aNode["max_data_rate_fluctuation"].val())));
            }
            attackerNodes.push_back(aN);
        }
        else
        {
            NS_LOG_WARN("AttackerNodeConfiguration with faulty config detected. Requires 'id' and "
                        "'owner_as'. Ignoring node.");
        }
    }

    m_attackerNodes = attackerNodes;
}

void
ConfigFileReader::ParseBenignNodesConfiguration()
{
    // check if any benign nodes configured
    ryml::ConstNodeRef root = m_rawTree;
    if (!root.has_child("benign_client_nodes"))
    {
        NS_LOG_WARN("Configuration does not contain any 'benign_client_nodes' key.");
        return;
    }

    root = root["benign_client_nodes"];
    NS_LOG_DEBUG("\nParsing benign client nodes configurations...");

    std::vector<BenignNodeConfiguration> benignNodes;
    for (ryml::ConstNodeRef bNode : root.children())
    {
        if (bNode.has_child("id") && bNode.has_child("owner_as") && bNode.has_child("peer"))
        {
            std::string asId = ToString(bNode["owner_as"].val());
            if (m_validAutonomousSystemIds.count(asId) == 0)
            {
                NS_FATAL_ERROR("Detected Benign Node configuration with invalid 'owner_as': "
                               << asId << ". No AS found in configuration with that id.");
            }
            std::string peerId = ToString(bNode["peer"].val());
            if (m_validServerNodeIds.count(peerId) == 0)
            {
                NS_FATAL_ERROR(
                    "Detected Benign Node configuration with invalid 'peer': "
                    << peerId
                    << ". No target/non-target server node in configuration with that id.");
            }
            BenignNodeConfiguration bN(ToString(bNode["id"].val()), asId, peerId);
            // update AS node counter
            AddOrIncrementASNodeCounter(bN.GetOwnerAS());
            // update mapper if present
            if (m_lookupMapper)
            {
                m_lookupMapper->AddNodeToAsEntry(bN.GetNodeId(), bN.GetOwnerAS());
            }
            // parse optionals
            if (bNode.has_child("max_reading_time"))
            {
                bN.SetMaxReadingTime(std::stoi(ToString(bNode["max_reading_time"].val())));
            }
            else
            {
                NS_LOG_DEBUG("No 'max_reading_time' configured, using default value: " +
                             std::to_string(bN.GetMaxReadingTime()));
            }
            benignNodes.push_back(bN);
        }
        else
        {
            NS_LOG_WARN("BenignNodeConfiguration with faulty config detected. Requires 'id', "
                        "'owner_as' and 'peer'. Ignoring node.");
        }
    }

    m_benignNodes = benignNodes;
}

void
ConfigFileReader::ParseAutonomousSystemsConfiguration()
{
    // check if autonomous systems configured
    ryml::ConstNodeRef root = m_rawTree;
    if (!root.has_child("autonomous_systems"))
    {
        /* here there is no choice but to throw a fatal error, since the entire config and the way
         * the nodes are instantiated in the scenario is centered around the presence of AS.
         */
        NS_FATAL_ERROR("Configuration does not contain any 'autonomous_systems' key.");
        return;
    }

    root = root["autonomous_systems"];
    NS_LOG_DEBUG("\nParsing autonomous systems configuration");

    std::vector<AutonomousSystemConfiguration> autonomousSystems;
    for (ryml::ConstNodeRef AS : root.children())
    {
        if (AS.has_child("id") && AS.has_child("network_address"))
        {
            AutonomousSystemConfiguration ASC(ToString(AS["id"].val()),
                                              ToString(AS["network_address"].val()));
            // parse optionals
            if (AS.has_child("network_mask"))
            {
                ASC.SetNetworkMask(ToString(AS["network_mask"].val()));
            }
            if (AS.has_child("bandwidth"))
            {
                ASC.SetBandwidth(ToString(AS["bandwidth"].val()));
            }
            if (AS.has_child("delay"))
            {
                ASC.SetDelay(ToString(AS["delay"].val()));
            }
            // parse connection of AS gateway to some central network node
            bool hasAttachmentNodeIdSpecified = false;
            if (AS.has_child("attachment"))
            {
                ryml::ConstNodeRef ASAttachment = AS["attachment"];
                if (ASAttachment.has_child("central_network_attachment_node"))
                {
                    hasAttachmentNodeIdSpecified = true;
                    std::string nodeId =
                        ToString(ASAttachment["central_network_attachment_node"].val());
                    if (m_validCentralNetworkNodeIds.count(nodeId) == 0)
                    {
                        NS_FATAL_ERROR("Found AS configuration that references "
                                       "'central_network_attachment_node': "
                                       << nodeId
                                       << ". No such central network node found in the "
                                          "configuration file. The faulty AS in question: "
                                       << ASC.GetId());
                    }
                    ASC.SetAttachmentNodeId(nodeId);
                }
                if (ASAttachment.has_child("bandwidth"))
                {
                    ASC.SetAttachmentConnectionBandwidth(ToString(ASAttachment["bandwidth"].val()));
                }
                if (ASAttachment.has_child("delay"))
                {
                    ASC.SetAttachmentConnectionDelay(ToString(ASAttachment["delay"].val()));
                }
            }
            else
            {
                NS_LOG_WARN("Found AutonomousSystemConfiguration without 'attachment' key.");
            }
            if (!hasAttachmentNodeIdSpecified)
            {
                NS_LOG_WARN("Found AutonomousSystemConfiguration that does not specify a "
                            "'central_network_attachment_node' key. Unless you plan to manually "
                            "connect the AS in the main script, this will lead to an isolated AS "
                            "that is not connected to the main topology.");
            }

            autonomousSystems.push_back(ASC);
            // add to validation vector
            m_validAutonomousSystemIds.emplace(ASC.GetId());
        }
        else
        {
            // fatal because cannot instantiate and some nodes might have this AS as owner_as.
            NS_FATAL_ERROR("Detected faulty AutonomousSystemConfiguration. Requires 'id' and "
                           "'network_address'. Terminating Program.");
            return;
        }
    }

    m_autonomousSystems = autonomousSystems;
}

std::string
ConfigFileReader::ToString(ryml::csubstr raw)
{
    // https://github.com/biojppm/rapidyaml/issues/275
    std::string converted(raw.str, raw.len);
    return converted;
}

void
ConfigFileReader::AddOrIncrementASNodeCounter(std::string key)
{
    if (m_autonomousSystemsNodeCounters.count(key) != 0)
    {
        // increment
        int value = m_autonomousSystemsNodeCounters[key];
        m_autonomousSystemsNodeCounters[key] = value + 1;
    }
    else
    {
        // add -> initialize at 2 (1 AS gateway, 1 current node to be counted)
        m_autonomousSystemsNodeCounters[key] = 2;
    }
}


void
ConfigFileReader::UpdateASNodeCounterOnAS()
{
    // traverse list of AS configurations and update the counter for each. use reference to prevent
    // copying loop argument and thus not actually modifying the AS config
    for (AutonomousSystemConfiguration& AS : m_autonomousSystems)
    {
        // set node counter
        if (m_autonomousSystemsNodeCounters.count(AS.GetId()) != 0)
        {
            AS.SetNumNodes(m_autonomousSystemsNodeCounters.at(AS.GetId()));
        }
    }
}

void
ConfigFileReader::PrintConfiguration()
{
    std::cout << std::endl
              << "###################################################################" << std::endl;
    std::cout << "##################### CONFIG PRINTOUT #############################" << std::endl;
    std::cout << "###################################################################" << std::endl;

    m_globalSettings.PrintConfiguration();
    m_centralNetwork.PrintConfiguration();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "Autonomous Systems Configuration" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    std::cout << "Autonomous Systems List:" << std::endl;
    for (AutonomousSystemConfiguration ac : m_autonomousSystems)
    {
        ac.PrintConfiguration("\t");
    }
    std::cout << "--------------------------------" << std::endl;

    std::cout << "--------------------------------" << std::endl;
    std::cout << "Target Server Node Configuration" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    std::cout << "Target Server Node List:" << std::endl;
    for (ServerNodeConfiguration sc : m_targetServerNodes)
    {
        sc.PrintConfiguration("\t");
    }
    std::cout << "--------------------------------" << std::endl;

    std::cout << "------------------------------------" << std::endl;
    std::cout << "Non-Target Server Node Configuration" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Non-Target Server Node List:" << std::endl;
    for (ServerNodeConfiguration sc : m_nonTargetServerNodes)
    {
        sc.PrintConfiguration("\t");
    }
    std::cout << "------------------------------------" << std::endl;

    std::cout << "---------------------------" << std::endl;
    std::cout << "Attacker Node Configuration" << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << "Attacker Node List:" << std::endl;
    for (AttackerNodeConfiguration ac : m_attackerNodes)
    {
        ac.PrintConfiguration("\t");
    }
    std::cout << "------------------------------------" << std::endl;

    std::cout << "-------------------------" << std::endl;
    std::cout << "Benign Node Configuration" << std::endl;
    std::cout << "-------------------------" << std::endl;
    std::cout << "Benign Node List:" << std::endl;
    for (BenignNodeConfiguration bc : m_benignNodes)
    {
        bc.PrintConfiguration("\t");
    }
    std::cout << "------------------------------------" << std::endl;

    std::cout << "###################################################################" << std::endl;
}
