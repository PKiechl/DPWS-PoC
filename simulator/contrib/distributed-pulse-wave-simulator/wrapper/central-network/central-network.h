#ifndef CENTRAL_NETWORK_H
#define CENTRAL_NETWORK_H

#include "ns3/core-module.h"
#include "ns3/node.h"
#include "ns3/node-container.h"

namespace ns3
{

/*
 * Abstract base class for central networks.
 */

class CentralNetwork
{
  public:
    virtual ~CentralNetwork() {};
    virtual Ptr<Node> GetNodeById(std::string id) = 0;
    virtual void EnablePcap(std::string prefix) = 0;

  private:
    virtual void BuildTopology() = 0;

  protected:
    CentralNetworkConfiguration m_config;
    NodeContainer m_nodes;
    int m_numNodes;

};

} // namespace ns3

#endif /* CENTRAL_NETWORK_H */