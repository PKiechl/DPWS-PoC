#ifndef CENTRAL_NETWORK_CONFIGURATION_H
#define CENTRAL_NETWORK_CONFIGURATION_H

#include "configuration.h"
#include "node-configuration.h"

#include <vector>

namespace ns3
{

/*
 * Stores the configuration for the central network, providing default values for optional properties.
 *
 * In terms of logic, the class consists of simple getters and setters.
 */

class CentralNetworkConfiguration : public Configuration
{
  public:
    CentralNetworkConfiguration(
        std::vector<NodeConfiguration> nodes = std::vector<NodeConfiguration>());
    void PrintConfiguration(std::string spacer = "") override;
    // prints configured values and default values where no explicit ones are provided in the config.

    int GetTopologySeed();
    std::string GetNetworkAddress();
    std::string GetNetworkMask();
    std::string GetBandwidth();
    std::string GetDelay();
    std::vector<NodeConfiguration> GetNodes();
    NodeConfiguration GetNode(int index);
    double GetDegreeOfRedundancy();

    void SetTopologySeed(int seed);
    void SetNetworkAddress(std::string networkAddress);
    void SetNetworkMask(std::string networkMask);
    void SetBandwidth(std::string bandwidth);
    void SetDelay(std::string delay);
    void SetDegreeOfRedundancy(double degree);

  private:
    // seed for topology randomization. arbitrary default value set here. Seed is required, such
    // that all parallelized instances (when using MPI) actually end up with the same topology
    int m_topologySeed = 47;

    std::string m_networkAddress = "10.1.1.0";
    std::string m_networkMask = "255.255.255.0";
    // address and netmask for CN internal connections. addresses are transparent, as they don't
    // appear in any pcap output

    std::string m_bandwidth = "1000Gbps";
    std::string m_delay = "5ms";
    std::vector<NodeConfiguration> m_nodes;
    double m_degreeOfRedundancy = 0.25;
    // degree of redundancy determines the amount of redundant connections (connections that go
    // beyond what is needed to create a minimal topology where each node is reachable from each
    // other node) Do not go overboard with this number unless you know what you are doing, this is
    // can start very quickly creating a very big number of connections, all of which will have pcap
    // output associated with it. For information on how this degree functions, refer to the intro
    // text on the file
    // contrib/distributed-pulse-wave-simulator/wrapper/central-network/implementations/randomized-partial-mesh-central-network.h
    // NOTE: if you modify the scenario.cc to instead use the full-mesh implementation of the
    // central network, this value will be ignored.
};

} // namespace ns3

#endif /* CENTRAL_NETWORK_CONFIGURATION_H */