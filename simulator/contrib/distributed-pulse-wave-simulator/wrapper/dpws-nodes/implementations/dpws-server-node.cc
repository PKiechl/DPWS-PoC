#include "dpws-server-node.h"

#include "ns3/http-client-server-helper.h"

using namespace ns3;

DPWSServerNode::DPWSServerNode(ServerNodeConfiguration config,
                               Ptr<Node> ns3Node,
                               Ipv4Address address)
{
    m_config = config;
    m_ns3Node = ns3Node;
    m_assignedAddress = address;
}

std::string
DPWSServerNode::GetNodeId()
{
    return m_config.GetNodeId();
}

std::tuple<Ipv4Address, int>
DPWSServerNode::GetHttpConnectionInfo()
{
    return
    {
        m_assignedAddress, m_config.GetHttpServerPort()
    };
}

void
DPWSServerNode::CreateApplications()
{
    // DPWSServerNode is used for both targetServers and nonTargetServers. Both only require an
    // http-server to be installed at current
    HttpServerHelper httpServer(m_config.GetHttpServerPort());
    m_applications.Add(httpServer.Install(m_ns3Node));
}

void
DPWSServerNode::StartApplications(double start, double stop)
{
    m_applications.Start(Seconds(start));
    m_applications.Stop(Seconds(stop));
}