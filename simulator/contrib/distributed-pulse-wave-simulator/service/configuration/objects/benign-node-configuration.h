#ifndef BENIGN_NODE_CONFIGURATION_H
#define BENIGN_NODE_CONFIGURATION_H

#include "node-configuration.h"

namespace ns3
{

/*
 * Each configured benign node results in an instance of this class. The class stores the necessary
 * information about each benign node and provides default values for optional properties.
 *
 * The class consists of simple getters and setters.
 */

class BenignNodeConfiguration : public NodeConfiguration
{
  public:
    BenignNodeConfiguration() = default;
    BenignNodeConfiguration(std::string nodeId, std::string ownerAS, std::string peer);
    void PrintConfiguration(std::string spacer = "") override;
    // prints configured values and default values where no explicit ones are provided in the
    // config.

    std::string GetPeer();
    std::string GetOwnerAS();
    int GetMaxReadingTime();

    void SetMaxReadingTime(int seconds);

  private:
    std::string m_peer;
    // node id of the (non)Target Server the benign client connects to
    std::string m_ownerAS;
    int m_maxReadingTime = 120;
    // 10000 is default max reading time implemented in http-client by
    // https://github.com/saulodamata/ns-3-http-traffic-generator,
    // This value is backed up by a scientific paper (An HTTP Web Traffic Model Based on the Top One
    // Million Visited Web Pages" by Rastin Pries et. al (Table II).
    // However, for the shorter simulations we aim to conduct (at least in the proof-of-concept phase),
    // a pause of 2+ hours is not useful, hence setting a lower default max reading time here
};
} // namespace ns3

#endif /* BENIGN_NODE_CONFIGURATION_H */
