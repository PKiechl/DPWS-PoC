#ifndef CONFIG_FILE_READER_H
#define CONFIG_FILE_READER_H

// using header only version of rapidyaml
#define RYML_SINGLE_HDR_DEFINE_NOW
#include "../../external/rapidyaml-0.5.0.hpp"

#include "ns3/attacker-node-configuration.h"
#include "ns3/autonomous-system-configuration.h"
#include "ns3/benign-node-configuration.h"
#include "ns3/central-network-configuration.h"
#include "ns3/core-module.h"
#include "ns3/global-settings-configuration.h"
#include "ns3/node-lookup-mapper.h"
#include "ns3/server-node-configuration.h"

#include <vector>

/*
 * Reads configuration from a yaml file and creates corresponding configuration class instances.
 * Makes use of the rapidyaml library for parsing (https://github.com/biojppm/rapidyaml)
 */

namespace ns3
{
class ConfigFileReader
{
  public:
    ConfigFileReader(std::string fileName, Ptr<NodeLookupMapper> lookupMapper);
    // alternative version of the config reader, instantiates a NodeLookupMapper that will create
    // a number of maps that make finding nodes / ASs easier in the main script

    ConfigFileReader(std::string fileName);

    void PrintConfiguration();
    // print entire configuration by calling the corresponding print functions on all configuration
    // class instances. Intended to help double-checking what actually was configured, given that
    // many configurable properties are optional due to default values being present

    CentralNetworkConfiguration GetCentralNetworkConfiguration();
    GlobalSettingsConfiguration GetGlobalSettingsConfiguration();
    std::vector<ServerNodeConfiguration> GetTargetServerNodeConfigurations();
    std::vector<ServerNodeConfiguration> GetNonTargetServerNodeConfigurations();
    std::vector<AttackerNodeConfiguration> GetAttackerNodeConfigurations();
    std::vector<BenignNodeConfiguration> GetBenignNodeConfigurations();
    std::vector<AutonomousSystemConfiguration> GetAutonomousSystemConfigurations();
    // the above batch of methods expose the parsed out configuration class instances

  private:
    std::string m_fileName;
    ryml::Tree m_rawTree;
    // store the root tree of the parsed configuration file in ryml format
    Ptr<NodeLookupMapper> m_lookupMapper = nullptr;
    // if instantiated, will be used to create a number of maps for easier lookups in the main
    // script
    std::unordered_map<std::string, int> m_autonomousSystemsNodeCounters;
    // tracks for each AS id the number of nodes (including the AS gateway)

    CentralNetworkConfiguration m_centralNetwork;
    GlobalSettingsConfiguration m_globalSettings;
    std::vector<ServerNodeConfiguration> m_targetServerNodes;
    std::vector<ServerNodeConfiguration> m_nonTargetServerNodes;
    std::vector<AttackerNodeConfiguration> m_attackerNodes;
    std::vector<BenignNodeConfiguration> m_benignNodes;
    std::vector<AutonomousSystemConfiguration> m_autonomousSystems;
    // the above batch of members store the parsed configuration class instances

    std::set<std::string> m_validCentralNetworkNodeIds;
    // contains list of IXP node ids -> valid attachment node ids for all ASs
    std::set<std::string> m_validAutonomousSystemIds;
    // contains list of AS ids -> valid ownerAS for all node types
    std::set<std::string> m_validServerNodeIds;
    // contains list of target/non-target server node ides -> valid peer IDS for benign nodes

    std::string ToString(ryml::csubstr raw);
    // conversion of csubstr that rapidyaml operates with to std::string
    void AddOrIncrementASNodeCounter(std::string key);
    // modify state of m_autonomousSystemsNodeCounters
    void UpdateASNodeCounterOnAS();
    // traverse m_autonomousSystems and add the node counter to each AS config. Is done in the end
    // to simplify validation of valid ownerAS on NodeConfigurations (validation basically requires
    // AS config to be parsed before the nodes, which means the counter has to be set after the
    // fact)

    void ParseConfiguration();
    void ParseGlobalSettingsConfiguration();
    void ParseCentralNetworkConfiguration();
    void ParseTargetServerNodesConfiguration();
    void ParseNonTargetServerNodesConfiguration();
    void ParseAttackerNodesConfiguration();
    void ParseBenignNodesConfiguration();
    void ParseAutonomousSystemsConfiguration();
    // the above batch of methods are responsible for reading the corresponding subtrees of
    // m_rawTree and instantiating the corresponding configuration class instances (i.e., actually
    // parsing the configuration)
};

} // namespace ns3

#endif /* CONFIG_FILE_READER_H */