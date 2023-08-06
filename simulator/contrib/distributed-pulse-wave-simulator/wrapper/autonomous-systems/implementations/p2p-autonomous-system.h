#ifndef P2P_AUTONOMOUS_SYSTEM_H
#define P2P_AUTONOMOUS_SYSTEM_H

#include "ns3/autonomous-system-configuration.h"
#include "ns3/autonomous-system.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

/*
 * provides an implementation of an AS that uses point-to-point connections to model the internal
 * topology of the AS. This is fundamentally a pragmatic AS model that does not try to
 * recreate some specific AS as it might be found in the real world.
 * This implementation can make use of MPI, if given an mpiRank in during construction.
 *
 * The model consists of:
 *  - AS gateway, through which all traffic in/out of the AS is directed
 *  - point-to-point connections rom gateway to each other node in AS, each using their own subnet
 *  (using Ipv4AddressHelper::NewNetwork()) to prevent issues with RTT that were encountered when
 *  merging all NetDeviceContainers and performing a single Ipv4AddressHelper::Assign() thereon.
 */

namespace ns3
{
class P2pAutonomousSystem : public AutonomousSystem
{
  public:
    P2pAutonomousSystem(AutonomousSystemConfiguration config, uint32_t mpiRank);
    void EnablePcap(std::string prefix) override;

  private:
    void BuildTopology() override;
    std::pair<Ptr<Node>, Ipv4Address> GetAndClaimNextAvailableNodeInfo(std::string nodeId) override;
    Ptr<Node> GetGatewayNode() override;

  protected:
    // used for internal topology
    std::vector<NetDeviceContainer> m_deviceVector;
    std::vector<Ipv4InterfaceContainer> m_interfaceVector;
};
} // namespace ns3

#endif /* P2P_AUTONOMOUS_SYSTEM_H */