#ifndef RANDOMIZED_PARTIAL_MESH_CENTRAL_NETWORK_H
#define RANDOMIZED_PARTIAL_MESH_CENTRAL_NETWORK_H

#include "ns3/central-network-configuration.h"
#include "ns3/central-network.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

#include <random>

/*
 * This class constructs a central network with a randomized topology. The minimal topology is a
 * partial mesh with the minimal number of connections, such that all nodes are reachable from each
 * other node
 * => N nodes, thus would have N-1 connections
 *
 * The configured "degree of redundancy" then adds additional random connections to that minimal
 * topology, potentially upgrading it all the way to a full mesh.
 * Note though, that when adding the redundant additional connections, there are no checks to ensure
 * they are "new" connections, i.e., connections between nodes that don't have a DIRECT connection
 * between them yet.
 * In other words, this central network allows for duplicate connections.
 *
 * The number of additional connections added follows the following rules regarding the degree of
 * redundancy:
 *      degree 0 => remain at minimal topology
 *      degree 1 => topology will have same number of connections as are found in full mesh, but
 *                  without guarantee to become a full-mesh, due to the randomization laid out above
 *      degree > 1 => more connections that found in full mesh
 *
 * => be careful with that degree, especially when scaling up the number of nodes, as in a full-mesh
 * the number of connections grows by the square (N*(N-1)/2)
 *
 * The topology is realised with point-to-point channels.
 */

namespace ns3
{

class RandomizedPartialMeshCentralNetwork : public CentralNetwork
{
  public:
    Ptr<Node> GetNodeById(std::string id) override;
    // return ns3 node based on associated nodeId from configuration
    void EnablePcap(std::string prefix) override;
    // enables pcap for all INTERNAL point-to-point connections. NOTE: this does not include
    // connections that are from outside the central network, e.g., from an AS to one of the nodes
    // in m_nodes. Those connections are (in the example of the AS) managed by the AS, thus pcap
    // outputting has to be enabled through said AS

    RandomizedPartialMeshCentralNetwork(CentralNetworkConfiguration config);
    void PrintTopology();
    // print the connections formed during buildTopology, so user can check what random topology
    // ultimately resulted from the configuration input

  private:
    void BuildTopology() override;
    // construct topology. always build minimal topology. And, if configured, extends minimal
    // topology with additional connections all the way up to a full mesh

    void PopulateNodeIdToIndexMap();
    // create associations from nodeIds of the configuration input to actual ns3 nodes in m_nodes

    int DrawRandomNodeIndex();
    // draw a random index, thus effectively drawing a random node form m_nodes

    void PopulateRandomizedMinimalTopologyConnectionsVector();
    // fill m_randomizedMinimalTopologyConnections by performing pairs of random draws on the
    // indices of m_nodes, resulting in pairs of indices representing a connection between those
    // nodes.
    // Logic is present to ensure that each node is reachable from each other node whilst using the
    // minimal number of connections to do so (also means, no duplicate connections)

    void PopulateRandomizedAdditionalConnectionsVector();
    // calculate the additional connections to be formed on top of the minimal topology. In this
    // case, duplicate connections are allowed, all draws can stem from same pool, since the
    // minimal topology already guarantees that all nodes are reachable. However, logic is present
    // to ensure that no self-to-self connection os formed on a given node.

  protected:
    /* randomization */
    std::mt19937 m_generator;
    std::uniform_int_distribution<> m_distribution;

    /* connections represented as pairs */
    std::vector<std::pair<int, int>> m_randomizedMinimalTopologyConnections;
    // stores those connections that are drawn when forming the initial minimal partial mesh.
    int m_numConnectionsForMinimalTopology;
    // number of connections required to establish minimal topology
    std::vector<std::pair<int, int>> m_randomizedAdditionalTopologyConnections;
    // stores the additional connects
    int m_numAdditionalConnections;
    // number of additionalConnections on top of the minimal topology. determined by number of nodes
    // and the configured degree of connectedness

    std::set<std::pair<int, int>> m_biDirectionalConnectionSet;
    // track connections between pairs of nodes
    std::unordered_map<std::string, int> m_nodeIdToContainerIndexMap;
    // associate nodeIds from config with nodeContainer indices. required to get correct node for
    // attachment of AS (or other constructs)
    std::vector<NetDeviceContainer> m_deviceVector;
    // contains the NetDevices for each established p2p connection during topology construction
    std::vector<Ipv4InterfaceContainer> m_interfaceVector;
    // contains the Interfaces for each established p2p connection during topology construction
    
    std::vector<std::pair<int, int>> m_deviceToNodeIndexTrackingVector;
    // at a given index i in this vector that is a pair of integers (x, y).
    // at the same index i in m_deviceVector (and for that matter also in  m_interfaceVector)
    // the pair of nodes identified by the indices x and y in m_nodes was used
    // during topology construction.
    // Thanks to this knowledge, it is then possible to essentially map any given NetDevice to
    // a specific node in m_nodes and therefore to a specific nodeId as configuerd in the config
    // file by the user.
    // This is then ultimately needed to enable semantically useful labelling of the pcap output
    // files
};

} // namespace ns3

#endif /* RANDOMIZED_PARTIAL_MESH_CENTRAL_NETWORK_H */