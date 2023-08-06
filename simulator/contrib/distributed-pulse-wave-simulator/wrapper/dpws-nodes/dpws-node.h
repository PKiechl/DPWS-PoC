#ifndef DPWS_NODE_H
#define DPWS_NODE_H

#include "ns3/application-container.h"
#include "ns3/internet-module.h"
#include "ns3/core-module.h"
#include "ns3/node.h"

namespace ns3
{

/*
 * Base class for all DPWSNode types. DPWSNode nomenclature to avoid confusion with the 'Node' type
 * that comes with Ns-3.
 */

class DPWSNode
{
  public:
    virtual ~DPWSNode(){};
    virtual std::string GetNodeId() = 0;
    Ptr<Node> GetNs3Node();
    Ipv4Address GetAssignedIpv4Address();

  private:

  protected:
    Ptr<Node> m_ns3Node;
    Ipv4Address m_assignedAddress;
    ApplicationContainer m_applications;
};

} // namespace ns3

#endif /* DPWS_NODE_H */