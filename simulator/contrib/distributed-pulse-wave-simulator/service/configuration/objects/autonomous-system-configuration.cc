#include "autonomous-system-configuration.h"

using namespace ns3;

AutonomousSystemConfiguration::AutonomousSystemConfiguration(std::string id, std::string networkAddress)
{
    m_id = id;
    m_networkAddress = networkAddress;
}

int
AutonomousSystemConfiguration::GetNumNodes()
{
    return m_numNodes;
}

std::string
AutonomousSystemConfiguration::GetId()
{
    return m_id;
}

std::string
AutonomousSystemConfiguration::GetNetworkAddress()
{
    return m_networkAddress;
}

std::string
AutonomousSystemConfiguration::GetNetworkMask()
{
    return m_networkMask;
}

std::string
AutonomousSystemConfiguration::GetBandwidth()
{
    return m_bandwidth;
}

std::string
AutonomousSystemConfiguration::GetDelay()
{
    return m_delay;
}

std::string
AutonomousSystemConfiguration::GetAttachmentNodeId()
{
    return m_attachmentNodeId;
}

std::string
AutonomousSystemConfiguration::GetAttachmentConnectionBandwidth()
{
    return m_attachmentConnectionBandwidth;
}

std::string
AutonomousSystemConfiguration::GetAttachmentConnectionDelay()
{
    return m_attachmentConnectionDelay;
}

void
AutonomousSystemConfiguration::SetNumNodes(int numNodes)
{
    m_numNodes = numNodes;
}

void
AutonomousSystemConfiguration::SetNetworkMask(std::string networkMask)
{
    m_networkMask = networkMask;
}

void
AutonomousSystemConfiguration::SetBandwidth(std::string bandwidth)
{
    m_bandwidth = bandwidth;
}

void
AutonomousSystemConfiguration::SetDelay(std::string delay)
{
    m_delay = delay;
}

void
AutonomousSystemConfiguration::SetAttachmentNodeId(std::string attachmentNodeId)
{
    m_attachmentNodeId = attachmentNodeId;
}

void
AutonomousSystemConfiguration::SetAttachmentConnectionBandwidth(std::string attachmentConnectionBandwidth)
{
    m_attachmentConnectionBandwidth = attachmentConnectionBandwidth;
}

void
AutonomousSystemConfiguration::SetAttachmentConnectionDelay(std::string attachmentConnectionDelay)
{
    m_attachmentConnectionDelay = attachmentConnectionDelay;
}

void
AutonomousSystemConfiguration::PrintConfiguration(std::string spacer)
{
    std::cout << spacer << "- AS Id: " << m_id << std::endl;
    std::cout << spacer << "  AS Network Address: " << m_networkAddress << std::endl;
    std::cout << spacer << "  AS Network Mask: " << m_networkMask << std::endl;
    std::cout << spacer << "  AS Bandwidth: " << m_bandwidth << std::endl;
    std::cout << spacer << "  AS Delay: " << m_delay << std::endl;
    std::cout << spacer << "  Number of contained nodes (including gateway): " << m_numNodes << std::endl;
    std::cout << spacer << "  Attachment: " << std::endl;
    std::cout << spacer << "  \tAS attaches to Node (Id): " << m_attachmentNodeId << std::endl;
    std::cout << spacer << "  \tConnection Bandwidth: " << m_attachmentConnectionBandwidth << std::endl;
    std::cout << spacer << "  \tConnection Delay: " << m_attachmentConnectionDelay << std::endl;
}