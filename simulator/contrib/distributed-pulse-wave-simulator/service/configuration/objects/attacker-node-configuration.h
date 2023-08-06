#ifndef ATTACKER_NODE_CONFIGURATION_H
#define ATTACKER_NODE_CONFIGURATION_H

#include "node-configuration.h"

namespace ns3
{

/*
 * Each configured attacker node results in an instance of this class. The class stores the necessary
 * information about each attacker node and provides default values for optional properties.
 *
 * The class consists of simple getters and setters.
 */

class AttackerNodeConfiguration : public NodeConfiguration
{
  public:
    AttackerNodeConfiguration() = default;
    AttackerNodeConfiguration(std::string nodeId, std::string ownerAS);
    void PrintConfiguration(std::string spacer = "") override;
    // prints configured values and default values where no explicit ones are provided in the config.

    std::string GetOwnerAS();
    std::string GetDataRate();
    int GetPacketSize();
    int GetSourcePort();
    int GetDestinationPort();
    double GetMaxDataRateFluctuation();

    void SetDataRate(std::string dataRate);
    void SetPacketSize(int packetSize);
    void SetSourcePort(int port);
    void SetDestinationPort(int port);
    void SetMaxDataRateFluctuation(double amount);

  private:
    std::string m_dataRate = "1Mbps";
    int m_packetSize = 156;
    // the above default values are chosen mostly arbitrarily, but should provide reasonable outcomes
    // if left as is.
    int m_destinationPort = -1;
    int m_sourcePort = -1;
    // -1 indicates that they will be randomized for appropriate attack vectors (TCP SYN and UDP flooding)
    std::string m_ownerAS;
    double m_maxDataRateFluctuation = 0.2;
    // allowed data rate fluctuation factor. Fluctuation can happen in either direction
};
} // namespace ns3

#endif /* ATTACKER_NODE_CONFIGURATION_H */