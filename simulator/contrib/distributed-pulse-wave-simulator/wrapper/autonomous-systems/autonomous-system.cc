#include "autonomous-system.h"

#include "ns3/mpi-interface.h"

using namespace ns3;

void
AutonomousSystem::ConnectToNode(Ptr<ns3::Node> targetNode, std::string addressBase)
{
    // create a point-to-point connection between targetNode and Gateway
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(m_config.GetAttachmentConnectionBandwidth()));
    p2p.SetChannelAttribute("Delay", StringValue(m_config.GetAttachmentConnectionDelay()));

    NetDeviceContainer devices = p2p.Install(GetGatewayNode(), targetNode);
    Ipv4AddressHelper address;
    address.SetBase(addressBase.c_str(), "255.255.255.0");
    // just assuming that this mask is fine for networks that are essentially transparent given
    // that with only two addresses are used from this base, /24 netmask is certainly fine
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // write back to class members
    m_connectionLink = p2p;
    m_connectionDevices = devices;
    m_connectionInterfaces = interfaces;
}

void
AutonomousSystem::ConnectToNode(Ptr<ns3::Node> targetNode, Ptr<AddressProvider> addressProvider)
{
    // create a point-to-point connection between targetNode and Gateway
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(m_config.GetAttachmentConnectionBandwidth()));
    p2p.SetChannelAttribute("Delay", StringValue(m_config.GetAttachmentConnectionDelay()));

    NetDeviceContainer devices = p2p.Install(GetGatewayNode(), targetNode);
    Ipv4AddressHelper* address = addressProvider->GetAddressHelper();
    Ipv4InterfaceContainer interfaces = address->Assign(devices);
    address->NewNetwork();
    // reuse Ipv4AddressHelper instance provided through argument

    // write back to class members
    m_connectionLink = p2p;
    m_connectionDevices = devices;
    m_connectionInterfaces = interfaces;
}

void
AutonomousSystem::StartApplications(double start, double stop)
{
    // only start applications if actually installed (applications only installed in process
    // (mpi rank) that is responsible for AS
    // if multithreading is not enabled, rank will always be 0, and so will
    // m_processInstanceId.
    if (MpiInterface::GetSystemId() == m_assignedMpiRank)
    {
        // server nodes start regularly
        for (DPWSServerNode sN : m_targetServerNodes)
        {
            sN.StartApplications(start, stop);
        }
        for (DPWSServerNode sN : m_nonTargetServerNodes)
        {
            sN.StartApplications(start, stop);
        }
        for (DPWSBenignNode bN : m_benignClientNodes)
        {
            bN.StartApplications(start, stop);
        }
        for (DPWSAttackerNode aN : m_attackerNodes)
        {
            aN.StartApplications(stop);
        }
    }
}

void
AutonomousSystem::CreateTargetServerNode(ServerNodeConfiguration config)
{
    // instantiate new server node and pass it its ns3 node to operate with
    auto [nextAvailableNode, address] = GetAndClaimNextAvailableNodeInfo(config.GetNodeId());
    DPWSServerNode sN(config, nextAvailableNode, address);

    // when multithreading, only install applications if in mpi rank that is responsible for
    // this particular AS. When not multithreading, both sides of the check are guaranteed to be 0.
    if (MpiInterface::GetSystemId() == m_assignedMpiRank)
    {
        sN.CreateApplications();
    }

    m_targetServerNodes.push_back(sN);
    // update map with httpServer connection data
    m_nodeIdToHttpServerConnectionMap[config.GetNodeId()] = sN.GetHttpConnectionInfo();
}

void
AutonomousSystem::CreateNonTargetServerNode(ServerNodeConfiguration config)
{
    // instantiate new server node and pass it its ns3 node to operate with
    auto [nextAvailableNode, address] = GetAndClaimNextAvailableNodeInfo(config.GetNodeId());
    DPWSServerNode sN(config, nextAvailableNode, address);

    // when multithreading, only install applications if in mpi rank that is responsible for
    // this particular AS. When not multithreading, both sides of the check are guaranteed to be 0.
    if (MpiInterface::GetSystemId() == m_assignedMpiRank)
    {
        sN.CreateApplications();
    }

    m_nonTargetServerNodes.push_back(sN);
    // update map with httpServer connection data
    m_nodeIdToHttpServerConnectionMap[config.GetNodeId()] = sN.GetHttpConnectionInfo();
}

void
AutonomousSystem::CreateBenignClientNode(BenignNodeConfiguration config,
                                         std::pair<Ipv4Address, int> serverConnectionInfo)
{
    // instantiate new benign node and pass it its ns3 node to operate with
    auto [nextAvailableNode, address] = GetAndClaimNextAvailableNodeInfo(config.GetNodeId());
    DPWSBenignNode bN(config, nextAvailableNode, address, serverConnectionInfo);

    // when multithreading, only install applications if in mpi rank that is responsible for
    // this particular AS. When not multithreading, both sides of the check are guaranteed to be 0.
    if (MpiInterface::GetSystemId() == m_assignedMpiRank)
    {
        bN.CreateApplications();
    }

    m_benignClientNodes.push_back(bN);
}

void
AutonomousSystem::CreateAttackerNode(AttackerNodeConfiguration config,
                                     std::vector<Ipv4Address> targetList,
                                     Ptr<AttackScheduleHelper> attackScheduler)
{
    // instantiate new attacker node and pass it its ns3 node to operate with
    auto [nextAvailableNode, address] = GetAndClaimNextAvailableNodeInfo(config.GetNodeId());
    DPWSAttackerNode aN(config, nextAvailableNode, address, targetList, attackScheduler);

    // when multithreading, only install applications if in mpi rank that is responsible for
    // this particular AS. When not multithreading, both sides of the check are guaranteed to be 0.
    if (MpiInterface::GetSystemId() == m_assignedMpiRank)
    {
        aN.CreateApplications();
    }

    m_attackerNodes.push_back(aN);
}

std::pair<Ipv4Address, int>
AutonomousSystem::GetHttpConnectionInfoByNodeId(std::string nodeId)
{
    if (m_nodeIdToHttpServerConnectionMap.count(nodeId) != 0)
    {
        return m_nodeIdToHttpServerConnectionMap[nodeId];
    }
    else
    {
        NS_FATAL_ERROR("Attempted to access HttpServerConnectionData for unknown nodeId " + nodeId);
    }
}

Ipv4Address
AutonomousSystem::GetIpv4ByNodeId(std::string nodeId, DPWSNodeType type)
{
    // this extraordinarily hideous and should be improved in a refinement step down the line
    switch (type)
    {
    case DPWSNodeType::benign:
        for (auto b : m_benignClientNodes)
        {
            if (b.GetNodeId() == nodeId)
            {
                return b.GetAssignedIpv4Address();
            }
        }
        NS_FATAL_ERROR("Attempted to access Ipv4Address for unknown nodeId " + nodeId);
    case DPWSNodeType::attacker:
        for (auto a : m_attackerNodes)
        {
            if (a.GetNodeId() == nodeId)
            {
                return a.GetAssignedIpv4Address();
            }
        }
        NS_FATAL_ERROR("Attempted to access Ipv4Address for unknown nodeId " + nodeId);
    case DPWSNodeType::target:
        for (auto t : m_targetServerNodes)
        {
            if (t.GetNodeId() == nodeId)
            {
                return t.GetAssignedIpv4Address();
            }
        }
        NS_FATAL_ERROR("Attempted to access Ipv4Address for unknown nodeId " + nodeId);
    case DPWSNodeType::non_target:
        for (auto nt : m_nonTargetServerNodes)
        {
            if (nt.GetNodeId() == nodeId)
            {
                return nt.GetAssignedIpv4Address();
            }
        }
        NS_FATAL_ERROR("Attempted to access Ipv4Address for unknown nodeId " + nodeId);
    default:
        NS_FATAL_ERROR("Attempted to access Ipv4Address for unknown node type");
    }
}