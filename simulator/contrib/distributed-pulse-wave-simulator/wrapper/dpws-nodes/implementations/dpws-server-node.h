#ifndef DPWS_SERVER_NODE_H
#define DPWS_SERVER_NODE_H

#include "ns3/core-module.h"
#include "ns3/dpws-node.h"
#include "ns3/server-node-configuration.h"

namespace ns3
{

/*
 * DPWSNode implementation for server nodes (which includes both target- and non-target
 * server nodes).
 *
 * This DPWSNode type installs an http-server and provides a method that exposes the connection
 * information for said server.
 * The http-server implementation is part of the traffic model implementation by Saulo Da Mata:
 * https://github.com/saulodamata/ns-3-http-traffic-generator
 */

class DPWSServerNode : public DPWSNode
{
  public:
    std::string GetNodeId() override;

    DPWSServerNode(ServerNodeConfiguration config, Ptr<Node> ns3Node, Ipv4Address address);
    void StartApplications(double start, double stop);
    std::tuple<Ipv4Address, int> GetHttpConnectionInfo();

    void CreateApplications();
    // enact creation of applications. Required to be public, such that the creating instance (e.g,
    // the Autonomous System within which the node resides) can control if the application actually
    // gets created (relevant when multithreading)

  protected:
    ServerNodeConfiguration m_config;
};

} // namespace ns3

#endif /* DPWS_SERVER_NODE_H */