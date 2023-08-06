#ifndef GLOBAL_SETTINGS_CONFIGURATION_H
#define GLOBAL_SETTINGS_CONFIGURATION_H

#include "configuration.h"

#include "ns3/attack-vector-configuration.h"

#include <vector>

namespace ns3
{

/*
 * Stores the configuration of the global settings. Provides default values for optional properties.
 *
 * Class consists of simple getters and setters.
 */

class GlobalSettingsConfiguration : public Configuration
{
  public:
    GlobalSettingsConfiguration(std::vector<AttackVectorConfiguration> attackVectors =
                                    std::vector<AttackVectorConfiguration>());
    void PrintConfiguration(std::string spacer = "") override;
    // prints configured values and default values where no explicit ones are provided in the config.

    std::string GetGlobalPcapPrefix();
    double GetBurstDuration();
    double GetTargetSwitchDuration();
    std::vector<AttackVectorConfiguration> GetAttackVectors();
    double GetSimDuration();
    std::string GetASConnectionNetworkAddress();
    std::string GetASConnectionNetworkMask();

    void SetGlobalPcapPrefix(std::string prefix);
    void SetBurstDuration(double burstDuration);
    void SetTargetSwitchDuration(double targetSwitchDuration);
    void SetSimDuration(double simDuration);
    void SetASConnectionNetworkAddress(std::string address);
    void SetASConnectionNetworkMask(std::string mask);

    void ResolveGlobalAndPerVectorAttackSettings();
    // attack vector specific settings are given precedence over global settings

  private:
    // capture settings
    std::string m_globalPcapPrefix = "";

    // attack settings
    double m_burstDuration = 60.0;
    double m_targetSwitchDuration = 0.0;
    std::vector<AttackVectorConfiguration> m_attackVectors;

    // scheduling settings
    double m_simDuration = 300.0;

    // autonomous systems connection settings.
    std::string m_asConnectionNetworkAddress = "20.1.1.0";
    std::string m_asConnectionNetworkMask = "255.255.255.0";
};

} // namespace ns3

#endif /* GLOBAL_SETTINGS_CONFIGURATION_H */