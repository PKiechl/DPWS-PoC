#ifndef AUTONOMOUS_SYSTEM_H
#define AUTONOMOUS_SYSTEM_H

#include "ns3/address-provider.h"
#include "ns3/attack-schedule-helper.h"
#include "ns3/attacker-node-configuration.h"
#include "ns3/autonomous-system-configuration.h"
#include "ns3/benign-node-configuration.h"
#include "ns3/core-module.h"
#include "ns3/dpws-attacker-node.h"
#include "ns3/dpws-benign-node.h"
#include "ns3/dpws-server-node.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/point-to-point-module.h"
#include "ns3/server-node-configuration.h"

namespace ns3
{

enum DPWSNodeType : int
{
    benign,
    target,
    non_target,
    attacker
};

/*
 * This class is the base class of the autonomous system class hierarchy, defining the signature
 * of a number of virtual methods as well as already providing some implementations for methods that
 * are not dependent on the internal topology of a given AS implementation.
 *
 * The idea is to have a basis for implementing (potentially) different types of AS, which differ
 * in terms of how they model their internal structure (what channel, topology, address assignment,
 * ...)
 */

class AutonomousSystem
{
  public:
    // start all applications
    void StartApplications(double start, double stop);

    void ConnectToNode(Ptr<Node> targetNode, std::string addressBase);
    // AS establishes a connection from AS Gateway to specified node, thus linking the AS to the
    // overarching topology. The supplied address is used to set up the interfaces for that
    // connection.

    void ConnectToNode(Ptr<Node> targetNode, Ptr<AddressProvider> addressIncrementor);
    // alternate version where an incrementor is used to provide access to an already existing
    // Ipv4Helper instance

    void CreateAttackerNode(AttackerNodeConfiguration config,
                            std::vector<Ipv4Address> targetList,
                            Ptr<AttackScheduleHelper> attackScheduler);
    void CreateBenignClientNode(BenignNodeConfiguration config,
                                std::pair<Ipv4Address, int> serverConnectionInfo);
    void CreateNonTargetServerNode(ServerNodeConfiguration config);
    void CreateTargetServerNode(ServerNodeConfiguration config);
    // each create DPWSNode of given type on the next available Ns3Node in m_nodes

    std::pair<Ipv4Address, int> GetHttpConnectionInfoByNodeId(std::string nodeId);
    // returns http connection data of target/non-target server associated with a given nodeId

    Ipv4Address GetIpv4ByNodeId(std::string nodeId, DPWSNodeType type=DPWSNodeType::target);
    // returns the Ipv4 address assigned to a given node

    virtual ~AutonomousSystem(){};

    virtual void EnablePcap(std::string prefix) = 0;
    // AS manages connection and interface to central network, hence EnablePcap has to be available
    // here too.
    // has to be done by subclass, because enabling is done on device containers, which might be
    // used differently based on how the internal topology is modeled (i.e., what channel etc. is
    // used)

  private:
    virtual void BuildTopology() = 0;
    // creates internal topology -> creates Ns3-Nodes, channels, interfaces, ...
    // Implementation thus left to subclass to allow for different types of AS that use different
    // internal topologies

    virtual std::pair<Ptr<Node>, Ipv4Address> GetAndClaimNextAvailableNodeInfo(
        std::string nodeId) = 0;
    // returns Ns3-Node and assigned address of the next free (not yet bound to a DPWSNode) node and
    // marks it as claimed
    // Implementation left to subclass, since topology-dependent.

    virtual Ptr<Node> GetGatewayNode() = 0;
    // return the node which serves as a gateway, i.e. is used to set up a connection to a node
    // outside of the AS, and through which all traffic that traverses the boundary between AS and
    // external topologies travels.
    // Implementation left to subclass, since topology-dependent.

  protected:
    uint32_t m_assignedMpiRank = 0; // used for mpi multithreading to identify in which instance
                                    // (rank) this particular AS is supposed to be ran
    AutonomousSystemConfiguration m_config;
    NodeContainer m_nodes;
    int m_numNodes;
    int m_firstUnclaimedNodeIndex = 1; // 1st node (index 0) always reserved for gateway
    std::unordered_map<std::string, int> m_nodeIdToContainerIndexMap;
    std::unordered_map<std::string, std::pair<Ipv4Address, int>>
        m_nodeIdToHttpServerConnectionMap; // shared by both target and non-target servers, since
                                           // benign nodes can communicate with both
    std::vector<DPWSServerNode> m_nonTargetServerNodes;
    std::vector<DPWSServerNode> m_targetServerNodes;
    std::vector<DPWSBenignNode> m_benignClientNodes;
    std::vector<DPWSAttackerNode> m_attackerNodes;

    // for connection to external topologies (i.e. some external node) through gateway
    PointToPointHelper m_connectionLink;
    NetDeviceContainer m_connectionDevices;
    Ipv4InterfaceContainer m_connectionInterfaces;
};

} // namespace ns3

#endif /* AUTONOMOUS_SYSTEM_H */