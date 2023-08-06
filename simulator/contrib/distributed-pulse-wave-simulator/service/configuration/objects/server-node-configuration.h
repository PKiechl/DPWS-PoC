#ifndef SERVER_NODE_CONFIGURATION_H
#define SERVER_NODE_CONFIGURATION_H

#include "node-configuration.h"

namespace ns3
{

/*
 * Each configured server node (target/non-target) results in an instance of this class. The class
 * stores the necessary information and provides default values for optional properties.
 *
 * The class consists of simple getters and setters
 */

class ServerNodeConfiguration : public NodeConfiguration
{
  public:
    ServerNodeConfiguration() = default;
    ServerNodeConfiguration(std::string nodeId, std::string ownerAS);
    void PrintConfiguration(std::string spacer="") override;
    // prints configured values and default values where no explicit ones are provided in the config.

    std::string GetOwnerAS();
    int GetHttpServerPort();

    void SetHttpServerPort(int port);

  private:
    std::string m_ownerAS;
    int m_httpServerPort = 80;
    // 80 -> default port for worldwide http, as per the IANA
    // https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml
};

} // namespace ns3

#endif /* SERVER_NODE_CONFIGURATION_H */