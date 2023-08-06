#include "full-mesh-central-network.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FullMeshCentralNetwork");

FullMeshCentralNetwork::FullMeshCentralNetwork(ns3::CentralNetworkConfiguration config)
{
    m_config = config;
    FullMeshCentralNetwork::BuildTopology();
    PopulateNodeIdToIndexMap();
}

void
FullMeshCentralNetwork::BuildTopology()
{
    // Create Nodes
    NodeContainer nodes;
    m_numNodes = m_config.GetNodes().size();

    if (m_numNodes == 0 || m_numNodes == 1)
    {
        /*
         * 0 -> fatal because operations like GetNodeById etc. would all have to implement special
         *      cases to support no nodes being present. also makes no sense semantically.
         */
        NS_FATAL_ERROR("Attempt to construct FullMeshCentralNetwork with 0 nodes. Aborting.");
        return;
    }

    if (m_numNodes == 1)
    {
        /*
         * 1 -> fatal because this class relies on point-to-point mechanics, which requires 2 nodes
         * (at least)
         */
        NS_FATAL_ERROR("Cannot construct FullMeshCentralNetwork with 1 node. Underlying "
                       "Point-to-Point mechanics require at least 2 nodes. If you just want one "
                       "central node for your scenario, then instantiate it directly instead of "
                       "using this class. (or try to build in support here for just 1 node)");
    }

    nodes.Create(m_numNodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    // Establish point-to-point channels between each pair of nodes to get a full-mesh (without
    // self-to-self)
    std::vector<PointToPointHelper> linkVector;
    std::vector<NetDeviceContainer> deviceVector;

    StringValue dataRate(m_config.GetBandwidth());
    StringValue delay(m_config.GetDelay());

    for (int i = 0; i < m_numNodes; i++)
    {
        for (int j = i + 1; j < m_numNodes; j++)
        {
            PointToPointHelper p2p;
            p2p.SetDeviceAttribute("DataRate", dataRate);
            p2p.SetChannelAttribute("Delay", delay);
            NetDeviceContainer devices = p2p.Install(nodes.Get(i), nodes.Get(j));
            deviceVector.push_back(devices);
            linkVector.push_back(p2p);
        }
    }

    // address assignment on a per-device-container basis
    Ipv4AddressHelper address;
    Ipv4Address addressBase = m_config.GetNetworkAddress().c_str();
    Ipv4Mask addressMask = m_config.GetNetworkMask().c_str();
    address.SetBase(addressBase, addressMask);

    std::vector<Ipv4InterfaceContainer> interfaceVector;
    for (int i = 0; i < deviceVector.size(); i++)
    {
        Ipv4InterfaceContainer interface = address.Assign(deviceVector[i]);
        interfaceVector.push_back(interface);
    }

    // store to private members
    m_nodes = nodes;
    m_linkVector = linkVector;
    m_deviceVector = deviceVector;
    m_interfaceVector = interfaceVector;
}

void
FullMeshCentralNetwork::PopulateNodeIdToIndexMap()
{
    std::vector<NodeConfiguration> nodes = m_config.GetNodes();
    /*
     * in this central network, the nodes are all instantiated at once and are treated equally,
     * hence can simply mapped by their order as found in config
     */
    int index = 0;
    for (NodeConfiguration node : nodes) {
        m_NodeIdToContainerIndexMap[node.GetNodeId()] = index;
        index++;
    }
}

Ptr<Node>
FullMeshCentralNetwork::GetNodeById(std::string id)
{
    int index = m_NodeIdToContainerIndexMap.at(id);
    return m_nodes.Get(index);
}

void
FullMeshCentralNetwork::EnablePcap(std::string prefix)
{
    PointToPointHelper any;
    any.EnablePcap(prefix + "__central-network-internal__", m_nodes, false);
}

int
FullMeshCentralNetwork::GetNumNodes()
{
    return m_numNodes;
}
