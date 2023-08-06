#ifndef FULL_MESH_CENTRAL_NETWORK_H
#define FULL_MESH_CENTRAL_NETWORK_H

/*
 * implements a fully connected mesh of nodes, which is a very optimistic assumption to go by, but
 * may be appropriate for some scenarios.
 * Therefore only use this if you are ok with the assumption that within the central network
 * everything is reachable in 1 hop.
 *
 * Note that unlike the RandomizedPartialMeshCentralNetwork, this does not have any logic that
 * applies nodeId's to pcap output, as it was not really seen as useful for the main use case.
 * So IF you intend to use this central network implementation, you may have to consider updating
 * the pcap labelling process in EnablePcap().
 */

#include "ns3/central-network-configuration.h"
#include "ns3/central-network.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3
{

class FullMeshCentralNetwork : public CentralNetwork
{
  public:
    Ptr<Node> GetNodeById(std::string id) override;
    void EnablePcap(std::string prefix) override;

    FullMeshCentralNetwork(CentralNetworkConfiguration config);
    int GetNumNodes();

  private:
    void BuildTopology() override;
    void PopulateNodeIdToIndexMap();

  protected:
    std::vector<PointToPointHelper> m_linkVector;
    std::vector<NetDeviceContainer> m_deviceVector;
    std::vector<Ipv4InterfaceContainer> m_interfaceVector;
    std::unordered_map<std::string, int> m_NodeIdToContainerIndexMap;
};

} // namespace ns3

#endif /* FULL_MESH_CENTRAL_NETWORK_H */