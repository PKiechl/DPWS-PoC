#include "csma-autonomous-system.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CsmaAutonomousSystem");

CsmaAutonomousSystem::CsmaAutonomousSystem(ns3::AutonomousSystemConfiguration config)
{
    m_config = config;
    CsmaAutonomousSystem::BuildTopology();
}

void
CsmaAutonomousSystem::BuildTopology()
{
    // Create Nodes
    NodeContainer nodes;
    m_numNodes = m_config.GetNumNodes();

    if (m_numNodes == 0)
    {
        // Operations such as setting up node functionalities will fail, hence best to just throw
        // fatal
        NS_FATAL_ERROR("Attempt to construct CsmaAutonomousSystem with 0 nodes. Aborting.");
        return;
    }

    // m_numNodes already includes the +1 for the AS gateway
    nodes.Create(m_numNodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    // set up csma channel that is used internally in the AS (includes all AS nodes as well as
    // Gateway)
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue(m_config.GetBandwidth()));
    csma.SetChannelAttribute("Delay", StringValue(m_config.GetDelay()));

    NetDeviceContainer devices;
    devices = csma.Install(nodes);

    Ipv4AddressHelper address;
    Ipv4Address addressBase = m_config.GetNetworkAddress().c_str();
    Ipv4Mask addressMask = m_config.GetNetworkMask().c_str();
    address.SetBase(addressBase, addressMask);

    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // store to private members
    m_nodes = nodes;
    m_csmaLink = csma;
    m_csmaDevices = devices;
    m_csmaInterfaces = interfaces;
}

Ptr<Node>
CsmaAutonomousSystem::GetGatewayNode()
{
    return m_nodes.Get(0);
}

void
CsmaAutonomousSystem::EnablePcap(std::string prefix)
{
    // enable pcap on the targetNode
    std::string midSection = "__" + m_config.GetId() + "-to-" + m_config.GetAttachmentNodeId();
    // ns3 automatically adds a pair of numbers that are somewhat cryptic and seem to have to do
    // with the indices of interfaces or NetDevices. There appears to be no way to remove them, so
    // I'm adding an additional spacer to make the actually semantically useful part of the filename
    // more clearly distinguishable from the appended stuff.
    std::string semanticSpacer = "____";
    m_connectionLink.EnablePcap(prefix + midSection + semanticSpacer, m_connectionDevices.Get(1), false);
}

std::pair<Ptr<Node>, Ipv4Address>
CsmaAutonomousSystem::GetAndClaimNextAvailableNodeInfo(std::string nodeId)
{
    // map available node index to DPWSNode's id, return corresponding Ns3 node and address,
    // and increment index counter
    int index = m_firstUnclaimedNodeIndex;
    m_firstUnclaimedNodeIndex += 1;

    m_nodeIdToContainerIndexMap[nodeId] = index;
    return {m_nodes.Get(index), m_csmaInterfaces.GetAddress(index)};
}