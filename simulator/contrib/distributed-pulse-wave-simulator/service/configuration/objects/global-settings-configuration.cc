#include "global-settings-configuration.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("GlobalSettingsConfiguration");

GlobalSettingsConfiguration::GlobalSettingsConfiguration(
    std::vector<AttackVectorConfiguration> attackVectors)
{
    m_attackVectors = attackVectors;
}

std::string
GlobalSettingsConfiguration::GetGlobalPcapPrefix()
{
    return m_globalPcapPrefix;
}

double
GlobalSettingsConfiguration::GetBurstDuration()
{
    return m_burstDuration;
}

double
GlobalSettingsConfiguration::GetTargetSwitchDuration()
{
    return m_targetSwitchDuration;
}

std::vector<AttackVectorConfiguration>
GlobalSettingsConfiguration::GetAttackVectors()
{
    return m_attackVectors;
}

double
GlobalSettingsConfiguration::GetSimDuration()
{
    return m_simDuration;
}

std::string
GlobalSettingsConfiguration::GetASConnectionNetworkAddress()
{
    return m_asConnectionNetworkAddress;
}

std::string
GlobalSettingsConfiguration::GetASConnectionNetworkMask()
{
    return m_asConnectionNetworkMask;
}

void
GlobalSettingsConfiguration::SetGlobalPcapPrefix(std::string prefix)
{
    m_globalPcapPrefix = prefix;
}

void
GlobalSettingsConfiguration::SetBurstDuration(double burstDuration)
{
    m_burstDuration = burstDuration;
}

void
GlobalSettingsConfiguration::SetTargetSwitchDuration(double targetSwitchDuration)
{
    m_targetSwitchDuration = targetSwitchDuration;
}

void
GlobalSettingsConfiguration::SetSimDuration(double simDuration)
{
    m_simDuration = simDuration;
}

void
GlobalSettingsConfiguration::SetASConnectionNetworkAddress(std::string address)
{
    m_asConnectionNetworkAddress = address;
}

void
GlobalSettingsConfiguration::SetASConnectionNetworkMask(std::string mask)
{
    m_asConnectionNetworkMask = mask;
}

void
GlobalSettingsConfiguration::ResolveGlobalAndPerVectorAttackSettings()
{
    // look at global settings (no matter if default or explicitly provided in config) and fill in
    // the gaps in the individual attack vector configs (i.e., if not set explicitly on per-vector
    // basis) then set now to "complete" their config. Note: data-rate and packet-size are settable
    // on a per-attacker-node-basis, thus that comparison happens when the individual attacker nodes
    // are created
    for (AttackVectorConfiguration& av : m_attackVectors)
    {
        if (av.GetBurstDuration() == -1.0)
        {
            av.SetBurstDuration(m_burstDuration);
        }
        if (av.GetTargetSwitchDuration() == -1.0)
        {
            av.SetTargetSwitchDuration(m_targetSwitchDuration);
        }
    }
}

void
GlobalSettingsConfiguration::PrintConfiguration(std::string spacer)
{
    std::cout << spacer << "-----------------------------" << std::endl;
    std::cout << spacer << "Global Settings Configuration" << std::endl;
    std::cout << spacer << "-----------------------------" << std::endl;
    std::cout << spacer << "Capture:" << std::endl;
    std::cout << spacer << "\tGlobal Pcap Prefix: " << m_globalPcapPrefix << std::endl;
    std::cout << spacer << "Attack:" << std::endl;
    std::cout << spacer << "\tBurst Duration (s): " << m_burstDuration << std::endl;
    std::cout << spacer << "\tTarget Switch Duration (s): " << m_targetSwitchDuration << std::endl;
    std::cout << spacer << "\tAttack Vectors:" << std::endl;
    for (AttackVectorConfiguration attackVector : m_attackVectors)
    {
        attackVector.PrintConfiguration(spacer + "\t\t");
    }
    std::cout << spacer << "Scheduling:" << std::endl;
    std::cout << spacer << "\tTotal Simulation Duration (s): " << m_simDuration << std::endl;
    std::cout << spacer << "Global AS Connection Settings:" << std::endl;
    std::cout << spacer << "\tAS Connection Address Base: " << m_asConnectionNetworkAddress
              << std::endl;
    std::cout << spacer << "\tAS Connection Address Mask: " << m_asConnectionNetworkMask
              << std::endl;
    std::cout << spacer << "-----------------------------" << std::endl;
}