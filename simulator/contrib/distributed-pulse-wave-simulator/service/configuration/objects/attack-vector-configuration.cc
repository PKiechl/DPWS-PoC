#include "attack-vector-configuration.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AttackVectorConfiguration");

AttackVectorConfiguration::AttackVectorConfiguration(std::string attackVector)
{
    m_attackVectorType = stringToAttackVectorEnum(attackVector);
}

double
AttackVectorConfiguration::GetBurstDuration()
{
    return m_burstDuration;
}

double
AttackVectorConfiguration::GetTargetSwitchDuration()
{
    return m_targetSwitchDuration;
}

int
AttackVectorConfiguration::GetPacketSize()
{
    return m_packetSize;
}

std::string
AttackVectorConfiguration::GetDataRate()
{
    return m_dataRate;
}

AttackVector
AttackVectorConfiguration::GetAttackVectorType()
{
    return m_attackVectorType;
}

int
AttackVectorConfiguration::GetSourcePort()
{
    return m_sourcePort;
}

int
AttackVectorConfiguration::GetDestinationPort()
{
    return m_destinationPort;
}

double
AttackVectorConfiguration::GetMaxDataRateFluctuation()
{
    return m_maxDataRateFluctuation;
}

void
AttackVectorConfiguration::SetBurstDuration(double burstDuration)
{
    m_burstDuration = burstDuration;
}

void
AttackVectorConfiguration::SetTargetSwitchDuration(double targetSwitchDuration)
{
    m_targetSwitchDuration = targetSwitchDuration;
}

void
AttackVectorConfiguration::SetPacketSize(int packetSize)
{
    m_packetSize = packetSize;
}

void
AttackVectorConfiguration::SetDataRate(std::string dataRate)
{
    m_dataRate = dataRate;
}

void
AttackVectorConfiguration::SetSourcePort(int port)
{
    if (port < -2 || port > 65535)
    {
        // https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
        // max port is 65535
        NS_FATAL_ERROR("Invalid source port number specified on attack vector: "
                       << port
                       << ", set -1 for randomization, -2 for ignore and a value between 0 and "
                          "65535 for a fixed port number.");
    }
    m_sourcePort = port;
}

void
AttackVectorConfiguration::SetDestinationPort(int port)
{
    if (port < -2 || port > 65535)
    {
        // https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
        // max port is 65535
        NS_FATAL_ERROR("Invalid destination port number specified on attack vector: "
                       << port
                       << ", set -1 for randomization, -2 for ignore and a value between 0 and "
                          "65535 for a fixed port number.");
    }
    m_destinationPort = port;
}

void
AttackVectorConfiguration::SetMaxDataRateFluctuation(double amount)
{
    m_maxDataRateFluctuation = amount;
}

void
AttackVectorConfiguration::PrintConfiguration(std::string spacer)
{
    std::cout << spacer << "- Attack Vector (type) " << attackVectorEnumToString(m_attackVectorType)
              << std::endl;
    if (m_burstDuration != -1.0)
    {
        std::cout << spacer << "  Burst Duration Override (s): " << std::to_string(m_burstDuration)
                  << std::endl;
    }
    if (m_targetSwitchDuration != -1.0)
    {
        std::cout << spacer << "  Target Switch Duration Override (s): "
                  << std::to_string(m_targetSwitchDuration) << std::endl;
    }
    if (m_packetSize != -1)
    {
        std::cout << spacer << "  Packet Size Override: " << std::to_string(m_packetSize)
                  << std::endl;
    }
    if (m_dataRate != "")
    {
        std::cout << spacer << "  Data Rate Override: " << m_dataRate << std::endl;
    }
    if (m_maxDataRateFluctuation != -1.0)
    {
        std::cout << spacer << "  Data Rate Fluctuation Override: " << m_maxDataRateFluctuation
                  << std::endl;
    }
    if (m_destinationPort != -2)
    {
        std::cout << spacer << "  Destination Port Override (-1 means randomized): "
                  << m_destinationPort << std::endl;
    }
    if (m_sourcePort != -2)
    {
        std::cout << spacer << "  Source Port Override (-1 means randomized): " << m_sourcePort
                  << std::endl;
    }
}