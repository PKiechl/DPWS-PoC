#ifndef ATTACK_VECTOR_CONFIGURATION_H
#define ATTACK_VECTOR_CONFIGURATION_H

#include "configuration.h"

namespace ns3
{

// enums and strings in c++...wow. I'm tempted to ditch this and just use a map...
enum AttackVector : int
{
    udp_flooding,
    icmp_flooding,
    tcp_syn_flooding
};

/*
 * Each configured attack vector results in an instance of this class. The class stores the necessary
 * information about each vector and provides default values for those properties that are optional,
 * thus can be left out of the configuration yaml.
 *
 * This configuration class is special, because most of its properties (data rate, packet size,
 * burst duration and target switching duration) can be configured at different places in the
 * configuration file.
 *      -> data rate and packet size can also be set on a per-attacker-node-level
 *      -> burst duration and target switching duration can be set globally
 * for both groups of properties, the values that are set on the individual attack vector trump
 * the settings on the attacker node and the global config.
 * In that sense, the individual attack vector is the most specific "unit of configuration" out of
 * attacker node, attack vector and global attack settings.
 * To see how these settings are resolved, c.f.
 *      -> DPWSAttackerNode::ResolveDataRate, respectively DPWSAttackerNode::ResolvePacketSize
 *      -> GlobalSettingsConfiguration::ResolveGlobalAndPerVectorAttackSettings
 *
 * In terms of logic, the class consists of simple getters and setters, as well as a few conversion
 * functions to deal with string<->enum conversions.
 */

//--------------------------------------------------------------------------------------------------
class AttackVectorConfiguration : public Configuration
{
  public:
    AttackVectorConfiguration(std::string attackVector);
    void PrintConfiguration(std::string spacer = "") override;
    // implements printing of configured values. Only prints members that have non-default values,
    // thus not guaranteed to print entire configuration, unlike in all other configuration classes.
    //
    // This is due to attack-vector vs. attack-node specific settings not being resolved during
    // parsing, but rather during the instantiation process on the DPWSAttackerNode.
    // This is the only PrintConfiguration() method that operates this way and does not print all
    // properties.

    double GetBurstDuration();
    double GetTargetSwitchDuration();
    int GetPacketSize();
    int GetSourcePort();
    int GetDestinationPort();
    double GetMaxDataRateFluctuation();
    std::string GetDataRate();
    AttackVector GetAttackVectorType();

    void SetBurstDuration(double burstDuration);
    void SetTargetSwitchDuration(double targetSwitchDuration);
    void SetPacketSize(int packetSize);
    void SetSourcePort(int port);
    void SetDestinationPort(int port);
    void SetMaxDataRateFluctuation(double amount);
    void SetDataRate(std::string dataRate);

    static AttackVector stringToAttackVectorEnum(std::string enumString)
    {
        if (enumString == "udp_flooding")
        {
            return AttackVector::udp_flooding;
        }
        if (enumString == "icmp_flooding")
        {
            return AttackVector::icmp_flooding;
        }
        if (enumString == "tcp_syn_flooding")
        {
            return AttackVector::tcp_syn_flooding;
        }
        NS_FATAL_ERROR("Unknown Attack vector: " + enumString +
                       ". Double check your configuration file.");
    }

    static std::string attackVectorEnumToString(AttackVector vector)
    {
        switch (vector)
        {
        case AttackVector::udp_flooding:
            return "udp_flooding";
        case AttackVector::icmp_flooding:
            return "icmp_flooding";
        case AttackVector::tcp_syn_flooding:
            return "tcp_syn_flooding";
        default:
            return "unknown attack vector";
        }
    }

  private:
    double m_burstDuration = -1.0;
    double m_targetSwitchDuration = -1.0;
    int m_packetSize = -1;
    std::string m_dataRate = "";
    // the default values of the above members are chosen such that it is easily detectable whether
    // values have been set or not (e.g., you COULD configure burstDuration = 0). This is needed
    // to resolve attack vector specific vs. global/per-node configuration. see top of file for
    // more context.
    AttackVector m_attackVectorType;

    int m_destinationPort = -2;
    int m_sourcePort = -2;
    // -1 indicates that they will be randomized for appropriate attack vectors (TCP SYN and UDP flooding)
    // -2 indicates that no value is set and thus any values in defined on a per-attacker node level
    // should not be overwritten
    double m_maxDataRateFluctuation = -1.0;
    // allowed data rate fluctuation factor. Fluctuation can happen in either direction
    // -1 means not set and any values defined on a per-attacker node level should not be overwritten
};

} // namespace ns3

#endif /* ATTACK_VECTOR_CONFIGURATION_H */