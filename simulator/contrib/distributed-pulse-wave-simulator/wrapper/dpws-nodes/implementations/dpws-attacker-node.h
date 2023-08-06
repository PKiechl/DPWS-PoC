#ifndef DPWS_ATTACKER_NODE_H
#define DPWS_ATTACKER_NODE_H

#include "ns3/attack-schedule-helper.h"
#include "ns3/attacker-node-configuration.h"
#include "ns3/core-module.h"
#include "ns3/dpws-node.h"

namespace ns3
{

/*
 * DPWSNode implementation for malicious clients that perform coordinated pulse wave DDoS attacks
 * against a list of targets.
 *
 * This DPWSNode type makes use of the OnOffRetargetApplication (one instance for each attack
 * vector) to model the pulse waves.
 * Relies on an external scheduler (AttackScheduleHelper) for the creation of the timings for
 * on/off state switches as well as remote-changes.
 *
 */

class DPWSAttackerNode : public DPWSNode
{
  public:
    std::string GetNodeId() override;

    DPWSAttackerNode(
        AttackerNodeConfiguration config,
        Ptr<Node> ns3Node,
        Ipv4Address address,
        std::vector<Ipv4Address> targetList,
        Ptr<AttackScheduleHelper> attackScheduler);

    void StartApplications(double stop);

    void ScheduleDynamicTargetChange(Ptr<OnOffRetargetApplication> app,
                                     Ptr<AttackScheduleHelper> scheduleHelper,
                                     std::vector<Ipv4Address> targetList,
                                     int vectorApplicationIndex,
                                     int targetIndex);
    // schedule the next remote change for a given application (denoted by vectorApplicationIndex).
    // This method is then continuously called recursively to schedule future remote changes for each
    // attack vector (i.e., for each OnOffRetargetApplication instance)

    void CreateApplications();
    // enact creation of applications. Required to be public, such that the creating instance (e.g,
    // the Autonomous System within which the node resides) can control if the application actually
    // gets created (relevant when multithreading)

  private:
    std::string GetSocketFactoryString(AttackVector vector);
    // get attack vector specific socket

    int ResolvePacketSize(int vectorValue);
    int ResolveSourcePort(int vectorValue);
    int ResolveDestinationPort(int vectorValue);
    std::string ResolveDataRate(std::string vectorValue);
    // resolve precedence of attack-vector-specific vs. attacker-node specific configuration of
    // packet size, data-rate and ports with the per-vector value enjoying higher precedence


    std::string CreateConstantRandomVariableString(double value);
    // create factory string for the random variable used to control the on/off timings. Note, that
    // despite the "random", the created constantRandomVariable always produced the exact same value

    InetSocketAddress GetRemoteAtTarget(AttackVector vector, int targetIndex=0);
    // get the target connection information, taking into account the type of attack vector, which
    // has an impact on (in this case) which port to target (if any at all)

  protected:
    AttackerNodeConfiguration m_config;
    std::vector<Ipv4Address> m_targetList;
    Ptr<AttackScheduleHelper> m_scheduleHelper;
};

} // namespace ns3

#endif /* DPWS_ATTACKER_NODE_H */