#ifndef CSMA_AUTONOMOUS_SYSTEM_H
#define CSMA_AUTONOMOUS_SYSTEM_H

#include "ns3/autonomous-system-configuration.h"
#include "ns3/autonomous-system.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

/*
 * NOTE: csma channels have been found in my testing to have a severe negative impact on traffic,
 * with a majority of the packets ultimately never showing up in any pcap output. use at your own
 * risk.
 *
 * NOTE: also does not implement anything related to MPI (multithreading)
 *
 * provides an implementation of an AS that bundles all internal nodes (including the gateway node)
 * into a single CSMA channel. This is fundamentally a pragmatic AS model that does not try to
 * recreate some specific AS as it might be found in the real world.
 *
 * The model consists of:
 *  - AS gateway, through which all traffic in/out of the AS is directed
 *  - an essentially flat internal topology, without any nested subnets
 *  - CSMA channel to depict the notion of shared resources within an AS
 */

namespace ns3
{
class CsmaAutonomousSystem : public AutonomousSystem
{
  public:
    CsmaAutonomousSystem(AutonomousSystemConfiguration config);
    void EnablePcap(std::string prefix) override;

  private:
    void BuildTopology() override;
    std::pair<Ptr<Node>, Ipv4Address> GetAndClaimNextAvailableNodeInfo(std::string nodeId) override;
    Ptr<Node> GetGatewayNode() override;

  protected:
    // used for internal topology
    CsmaHelper m_csmaLink;
    NetDeviceContainer m_csmaDevices;
    Ipv4InterfaceContainer m_csmaInterfaces;
};
} // namespace ns3

#endif /* CSMA_AUTONOMOUS_SYSTEM_H */