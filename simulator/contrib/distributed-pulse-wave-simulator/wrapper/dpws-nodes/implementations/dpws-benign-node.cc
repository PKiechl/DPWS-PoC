#include "dpws-benign-node.h"

#include "ns3/http-client-server-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DPWSBenignNode");

DPWSBenignNode::DPWSBenignNode(ns3::BenignNodeConfiguration config,
                               Ptr<ns3::Node> ns3Node,
                               ns3::Ipv4Address address,
                               std::pair<Ipv4Address, int> peerServerInfo)
{
    m_config = config;
    m_ns3Node = ns3Node;
    m_assignedAddress = address;
    m_peerServerInfo = peerServerInfo;
}

std::string
DPWSBenignNode::GetNodeId()
{
    return m_config.GetNodeId();
}

void
DPWSBenignNode::StartApplications(double start, double stop)
{
    m_applications.Start(Seconds(start));
    m_applications.Stop(Seconds(stop));
}

void
DPWSBenignNode::CreateApplications()
{
    auto [peerAddress, peerPort] = m_peerServerInfo;
    HttpClientHelper httpClient(peerAddress, peerPort);
    httpClient.SetAttribute("MaxReadingTime", UintegerValue(m_config.GetMaxReadingTime()));
    m_applications.Add(httpClient.Install(m_ns3Node));
}