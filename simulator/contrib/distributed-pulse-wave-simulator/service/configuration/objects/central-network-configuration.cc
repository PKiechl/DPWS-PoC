#include "central-network-configuration.h"

using namespace ns3;

CentralNetworkConfiguration::CentralNetworkConfiguration(std::vector<NodeConfiguration> nodes)
{
    m_nodes = nodes;
}

int
CentralNetworkConfiguration::GetTopologySeed()
{
    return m_topologySeed;
}

std::string
CentralNetworkConfiguration::GetNetworkAddress()
{
    return m_networkAddress;
}

std::string
CentralNetworkConfiguration::GetNetworkMask()
{
    return m_networkMask;
}

std::string
CentralNetworkConfiguration::GetBandwidth()
{
    return m_bandwidth;
}

std::string
CentralNetworkConfiguration::GetDelay()
{
    return m_delay;
}

std::vector<NodeConfiguration>
CentralNetworkConfiguration::GetNodes()
{
    return m_nodes;
}

NodeConfiguration
CentralNetworkConfiguration::GetNode(int index)
{
    return m_nodes[index];
}

double
CentralNetworkConfiguration::GetDegreeOfRedundancy()
{
    return m_degreeOfRedundancy;
}

void
CentralNetworkConfiguration::SetTopologySeed(int seed)
{
    m_topologySeed = seed;
}

void
CentralNetworkConfiguration::SetNetworkAddress(std::string networkAddress)
{
    m_networkAddress = networkAddress;
}

void
CentralNetworkConfiguration::SetNetworkMask(std::string networkMask)
{
    m_networkMask = networkMask;
}

void
CentralNetworkConfiguration::SetBandwidth(std::string bandwidth)
{
    m_bandwidth = bandwidth;
}

void
CentralNetworkConfiguration::SetDelay(std::string delay)
{
    m_delay = delay;
}

void
CentralNetworkConfiguration::SetDegreeOfRedundancy(double degree)
{
    m_degreeOfRedundancy = degree;
}

void
CentralNetworkConfiguration::PrintConfiguration(std::string spacer)
{
    std::cout << spacer << "-----------------------------" << std::endl;
    std::cout << spacer << "Central Network Configuration" << std::endl;
    std::cout << spacer << "-----------------------------" << std::endl;
    std::cout << spacer << "Network Address: " << m_networkAddress << std::endl;
    std::cout << spacer << "Network Mask: " << m_networkMask << std::endl;
    std::cout << spacer << "Bandwidth: " << m_bandwidth << std::endl;
    std::cout << spacer << "Delay: " << m_delay << std::endl;
    std::cout << spacer << "Degree of Connectedness: " << m_degreeOfRedundancy << std::endl;
    std::cout << spacer << "Topology Randomization Seed: " << m_topologySeed << std::endl;
    std::cout << spacer << "Node List:" << std::endl;
    for (NodeConfiguration n : m_nodes)
    {
        n.PrintConfiguration(spacer + "\t");
    }
    std::cout << spacer << "-----------------------------" << std::endl;
}