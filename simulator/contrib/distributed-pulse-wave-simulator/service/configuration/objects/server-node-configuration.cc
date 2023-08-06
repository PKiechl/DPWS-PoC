#include "server-node-configuration.h"

using namespace ns3;

ServerNodeConfiguration::ServerNodeConfiguration(std::string nodeId, std::string ownerAS)
    : NodeConfiguration(nodeId)
{
    m_ownerAS = ownerAS;
}

std::string
ServerNodeConfiguration::GetOwnerAS()
{
    return m_ownerAS;
}

int
ServerNodeConfiguration::GetHttpServerPort()
{
    return m_httpServerPort;
}

void
ServerNodeConfiguration::SetHttpServerPort(int port)
{
    m_httpServerPort = port;
}

void
ServerNodeConfiguration::PrintConfiguration(std::string spacer)
{
    NodeConfiguration::PrintConfiguration(spacer);
    std::cout << spacer << "  Owner AS (id): " << m_ownerAS << std::endl;
    std::cout << spacer << "  HTTP server port: " << m_httpServerPort << std::endl;
}