#include "benign-node-configuration.h"

using namespace ns3;

BenignNodeConfiguration::BenignNodeConfiguration(std::string nodeId,
                                                 std::string ownerAS,
                                                 std::string peer)
    : NodeConfiguration(nodeId)
{
    m_ownerAS = ownerAS;
    m_peer = peer;
}

std::string
BenignNodeConfiguration::GetPeer() {
    return m_peer;
}

std::string
BenignNodeConfiguration::GetOwnerAS()
{
    return m_ownerAS;
}

int
BenignNodeConfiguration::GetMaxReadingTime()
{
    return m_maxReadingTime;
}

void
BenignNodeConfiguration::SetMaxReadingTime(int seconds)
{
    m_maxReadingTime = seconds;
}

void BenignNodeConfiguration::PrintConfiguration(std::string spacer)
{
    NodeConfiguration::PrintConfiguration(spacer);
    std::cout << spacer << "  Owner AS (id): " << m_ownerAS << std::endl;
    std::cout << spacer << "  Peer (id): " << m_peer << std::endl;
    std::cout << spacer << "  Max Reading Time Limit (s): " << m_maxReadingTime << std::endl;
}