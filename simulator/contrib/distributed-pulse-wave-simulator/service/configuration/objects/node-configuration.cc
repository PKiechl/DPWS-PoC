#include "node-configuration.h"

using namespace ns3;

NodeConfiguration::NodeConfiguration(std::string nodeId)
{
    m_nodeId = nodeId;
}

std::string
NodeConfiguration::GetNodeId()
{
    return m_nodeId;
}

void
NodeConfiguration::PrintConfiguration(std::string spacer)
{
    Configuration::PrintConfiguration();
    std::cout << spacer << "- Node ID: " << m_nodeId << std::endl;
}