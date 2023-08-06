#ifndef DPWS_BENIGN_NODE_H
#define DPWS_BENIGN_NODE_H

#include "ns3/benign-node-configuration.h"
#include "ns3/core-module.h"
#include "ns3/dpws-node.h"

namespace ns3
{

/*
 * DPWSNode implementation for benign clients that have the purpose of adding background traffic
 * to the simulation.
 *
 * This DPWSNode type installs an http-client and requires connection information for a
 * DPWSServerNode with which it then starts communicating.
 *
 * The http-client implementation is part of the traffic model implementation by Saulo Da Mata:
 * https://github.com/saulodamata/ns-3-http-traffic-generator
 *
 */

class DPWSBenignNode : public DPWSNode
{
  public:
    std::string GetNodeId() override;

    DPWSBenignNode(BenignNodeConfiguration config,
                   Ptr<Node> ns3Node,
                   Ipv4Address address,
                   std::pair<Ipv4Address, int> peerServerInfo);


    void StartApplications(double start, double stop);

    void CreateApplications();
    // enact creation of applications. Required to be public, such that the creating instance (e.g,
    // the Autonomous System within which the node resides) can control if the application actually
    // gets created (relevant when multithreading)

  protected:
    BenignNodeConfiguration m_config;
    std::pair<Ipv4Address, int> m_peerServerInfo;
};

} // namespace ns3

#endif /* DPWS_BENIGN_NODE_H */