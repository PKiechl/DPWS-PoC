#include "attacker-node-configuration.h"

using namespace ns3;

AttackerNodeConfiguration::AttackerNodeConfiguration(std::string nodeId, std::string ownerAS) : NodeConfiguration(nodeId)
{
    m_ownerAS = ownerAS;
}

std::string
AttackerNodeConfiguration::GetOwnerAS()
{
    return m_ownerAS;
}

std::string
AttackerNodeConfiguration::GetDataRate()
{
    return m_dataRate;
}

int
AttackerNodeConfiguration::GetPacketSize()
{
    return m_packetSize;
}

int
AttackerNodeConfiguration::GetSourcePort()
{
    return m_sourcePort;
}

int
AttackerNodeConfiguration::GetDestinationPort()
{
    return m_destinationPort;
}

double
AttackerNodeConfiguration::GetMaxDataRateFluctuation()
{
    return m_maxDataRateFluctuation;
}

void
AttackerNodeConfiguration::SetDataRate(std::string dataRate)
{
    m_dataRate = dataRate;
}

void
AttackerNodeConfiguration::SetPacketSize(int packetSize)
{
    m_packetSize = packetSize;
}

void
AttackerNodeConfiguration::SetSourcePort(int port)
{
    if (port < -1 || port > 65535)
    {
        // https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
        // max port is 65535
        NS_FATAL_ERROR("Invalid source port number specified on attacker node: "
                       << port
                       << ", set -1 for randomization or a value between 0 and "
                          "65535 for a fixed port number.");
    }
    m_sourcePort = port;
}

void
AttackerNodeConfiguration::SetDestinationPort(int port)
{
    if (port < -1 || port > 65535)
    {
        // https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
        // max port is 65535
        NS_FATAL_ERROR("Invalid source port number specified on attacker node: "
                       << port
                       << ", set -1 for randomization or a value between 0 and "
                          "65535 for a fixed port number.");
    }
    m_destinationPort = port;
}

void
AttackerNodeConfiguration::SetMaxDataRateFluctuation(double amount)
{
    m_maxDataRateFluctuation = amount;
}

void
AttackerNodeConfiguration::PrintConfiguration(std::string spacer)
{
    NodeConfiguration::PrintConfiguration(spacer);
    std::cout << spacer << "  Owner AS (id): " << m_ownerAS << std::endl;
    std::cout << spacer << "  Data Rate : " << m_dataRate << std::endl;
    std::cout << spacer << "  Max. Data Rate Fluctuation (factor) : " << m_maxDataRateFluctuation << std::endl;
    std::cout << spacer << "  Packet Size (Kb): " << m_packetSize << std::endl;
    std::cout << spacer << "  Destination Port (-1 means random): " << m_destinationPort << std::endl;
    std::cout << spacer << "  Source Port (-1 means random): " << m_sourcePort << std::endl;
}