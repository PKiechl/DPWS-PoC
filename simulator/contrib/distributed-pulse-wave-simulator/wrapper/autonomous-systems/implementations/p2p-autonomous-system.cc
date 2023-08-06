#include "p2p-autonomous-system.h"

#include "ns3/mpi-interface.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("P2pAutonomousSystem");

P2pAutonomousSystem::P2pAutonomousSystem(ns3::AutonomousSystemConfiguration config,
                                         uint32_t mpiRank)
{
    m_config = config;
    m_assignedMpiRank = mpiRank;
    BuildTopology();
}

void
P2pAutonomousSystem::BuildTopology()
{
    // Create Nodes
    NodeContainer nodes;
    m_numNodes = m_config.GetNumNodes();

    if (m_numNodes == 0)
    {
        // Operations such as setting up node functionalities will fail, hence best to just throw
        // fatal
        NS_FATAL_ERROR("Attempt to construct P2pAutonomousSystem with 0 nodes. Aborting.");
        return;
    }

    if (m_numNodes == 1)
    {
        // 1 -> would mean only gateway node, otherwise empty AS. functionalities would fail.
        NS_FATAL_ERROR("Attempt to construct P2pAutonomousSystem with no actual nodes, just with "
                       "the gateway node. Aborting.");
        return;
    }

    // m_numNodes already includes the +1 for the AS gateway
    // create nodes specifically for the assigned mpi rank if using multithreading. If not using
    // multithreading, m_assignedMpiRank will equal zero.
    nodes.Create(m_numNodes, m_assignedMpiRank);

    InternetStackHelper stack;
    stack.Install(nodes);

    // set up collections for the individual p2p connections
    std::vector<NetDeviceContainer> deviceVector;
    std::vector<Ipv4InterfaceContainer> interfaceVector;

    Ipv4AddressHelper address;
    Ipv4Address addressBase = m_config.GetNetworkAddress().c_str();
    Ipv4Mask addressMask = m_config.GetNetworkMask().c_str();
    address.SetBase(addressBase, addressMask);

    // initialize helper to crate p2p connections
    StringValue dataRate(m_config.GetBandwidth());
    StringValue delay(m_config.GetDelay());

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", dataRate);
    p2p.SetChannelAttribute("Delay", delay);

    // create p2p connection from gateway node to each other node within AS
    for (int i = 1; i < m_numNodes; i++)
    // excluding 0, since that is the index of the gateway node
    {
        NetDeviceContainer devices = p2p.Install(nodes.Get(0), nodes.Get(i));
        Ipv4InterfaceContainer interfaces = address.Assign(devices);
        address.NewNetwork();
        // have to call NewNetwork (thus essentially "incrementing" the 3rd position of the address
        // resulting in the following pattern:
        //
        // Gateway<->Node 1:    Gateway Ipv4: nnn.mmm.uuu.1, Node 1 Ipv4: nnn.mmm.uuu.2
        // Gateway<->Node 2:    Gateway Ipv4: nnn.mmm.uuu+1.1, Node 1 Ipv4: nnn.mmm.uuu+1.2
        // ...
        //
        // this is required, as otherwise most packets run into severe issues with TTL and will not
        // reqch their destination. Why exactly that is I cannot say, but not having all the
        // individual point-to-point connections in the same /24 subnet appears to do the trick.
        deviceVector.push_back(devices);
        interfaceVector.push_back(interfaces);
    }

    // write to class members
    m_nodes = nodes;
    m_deviceVector = deviceVector;
    m_interfaceVector = interfaceVector;
}

std::pair<Ptr<Node>, Ipv4Address>
P2pAutonomousSystem::GetAndClaimNextAvailableNodeInfo(std::string nodeId)
{
    int index = m_firstUnclaimedNodeIndex;
    m_firstUnclaimedNodeIndex += 1;

    m_nodeIdToContainerIndexMap[nodeId] = index;
    // for the interface, have to subtract 1, to offset the initial index in the node container,
    // that that is reserved for the gateway node.
    return std::pair<Ptr<Node>, Ipv4Address>(m_nodes.Get(index),
                                             m_interfaceVector[index - 1].GetAddress(1));
}

Ptr<Node>
P2pAutonomousSystem::GetGatewayNode()
{
    return m_nodes.Get(0);
}

void
P2pAutonomousSystem::EnablePcap(std::string prefix)
{
    // only enable pcap tracing if is within rank 0 (rank 0 is always responsible for
    // the central network, no matter if multithreading is enabled or not
    if (MpiInterface::GetSystemId() == 0)
    {
        // enable pcap on the targetNode
        std::string midSection = "__" + m_config.GetAttachmentNodeId() + "-to-" + m_config.GetId();
        // ns3 automatically adds a pair of numbers that are somewhat cryptic and seem to have to do
        // with the indices of interfaces or NetDevices. There appears to be no way to remove them,
        // so I'm adding an additional spacer to make the actually semantically useful part of the
        // filename more clearly distinguishable from the appended stuff.
        std::string semanticSpacer = "____";
        m_connectionLink.EnablePcap(prefix + midSection + semanticSpacer,
                                    m_connectionDevices.Get(1),
                                    false);
    }
}