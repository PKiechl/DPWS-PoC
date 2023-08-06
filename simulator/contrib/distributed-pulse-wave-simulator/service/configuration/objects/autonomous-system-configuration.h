#ifndef AUTONOMOUS_SYSTEM_CONFIGURATION_H
#define AUTONOMOUS_SYSTEM_CONFIGURATION_H

#include "node-configuration.h"

namespace ns3
{

/*
 * Each configured AS results in an instance of this class. The class stores the necessary
 * information about each AS and provides default values for optional properties.
 *
 * In terms of logic, the class consists of simple getters and setters.
 */

class AutonomousSystemConfiguration : public Configuration
{
  public:
    AutonomousSystemConfiguration() = default;
    AutonomousSystemConfiguration(std::string id, std::string networkAddress);
    void PrintConfiguration(std::string spacer = "") override;
    // prints configured values and default values where no explicit ones are provided in the config.

    int GetNumNodes();
    std::string GetId();
    std::string GetNetworkAddress();
    std::string GetNetworkMask();
    std::string GetBandwidth();
    std::string GetDelay();
    std::string GetAttachmentNodeId();
    std::string GetAttachmentConnectionBandwidth();
    std::string GetAttachmentConnectionDelay();

    void SetNumNodes(int numNodes);
    void SetNetworkMask(std::string networkMask);
    void SetBandwidth(std::string bandwidth);
    void SetDelay(std::string delay);
    void SetAttachmentNodeId(std::string attachmentNodeId);
    void SetAttachmentConnectionBandwidth(std::string attachmentConnectionBandwidth);
    void SetAttachmentConnectionDelay(std::string attachmentConnectionDelay);

  private:
    int m_numNodes = 0; // number of inserted nodes (attackers, benign, servers) + 1 for gateway, 0
                        // if no other nodes (no need for gateway then)
    std::string m_id;
    std::string m_networkAddress; // address base of the AS network that contains all of
                                  // the AS nodes
    std::string m_networkMask = "255.255.255.0"; // network mask of the AS network
    std::string m_bandwidth =
        "1000Gbps"; // bandwidth of the individual channels within the AS network
    std::string m_delay = "2ms"; // delay within the AS network

    // Attachment node properties, i.e. specs for connection from AS gateway to a central network
    // (IXP) node
    std::string
        m_attachmentNodeId; // id of a 'central_network' node. if left empty you can still set the
                            // connection up manually the class that actually implements the AS
    std::string m_attachmentConnectionBandwidth = "1000Gbps";
    std::string m_attachmentConnectionDelay = "5ms";
};
} // namespace ns3

#endif /* AUTONOMOUS_SYSTEM_CONFIGURATION_H */