#ifndef NODE_CONFIGURATION_H
#define NODE_CONFIGURATION_H

#include "configuration.h"

namespace ns3
{

/*
 * Base class for all node-type configuration classes. Provides implementation for GetNodeId().
 */

class NodeConfiguration : public Configuration
{
  public:
    NodeConfiguration() = default;
    NodeConfiguration(std::string nodeId);
    void PrintConfiguration(std::string spacer="") override;

    std::string GetNodeId();

  private:
    std::string m_nodeId;
};

} // namespace ns3

#endif /* NODE_CONFIGURATION_H */