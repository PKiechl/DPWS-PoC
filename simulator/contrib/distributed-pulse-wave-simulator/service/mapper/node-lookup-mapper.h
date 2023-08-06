#ifndef NODE_LOOKUP_MAPPER_H
#define NODE_LOOKUP_MAPPER_H

#include "ns3/core-module.h"

namespace ns3
{

/*
 * The purpose of this class is to have a central location for tracking information about relationships
 * between ASs and Nodes, thus enabling a more convenient way to look up / find specific instances
 * based on ids (rather than having to iterate through collections in the main script (use_case.cc).
 *
 * This helper class provides information about relationships:
 *      NodeId to AsId -> enables convenient lookup of AsId through NodeId (i.e., identify the AS
 *          to which a given node belongs)
 *      NodeId to AsIndex -> retrieve index of AS in AS vector through NodeId (i.e, get the index
 *          of the "owner AS" for a given nodeId)
 *      AsId to AsIndex -> retrieve index of AS in AS vector through AsId (i.e., get the index of
 *          the given AS)
 *
 *      -> AS vector is part of the main script (use_case.cc)
 */

// inherit from SimpleRefCounter for access to ns3 Smart Pointer Class Ptr<>
class NodeLookupMapper : public SimpleRefCount<NodeLookupMapper>
{
  public:
    void AddNodeToAsEntry(std::string nodeId, std::string asId);
    void AddAsToAsIndexEntry(std::string asId, int index);
    // add entries to corresponding map

    std::string GetAsIdByNodeId(std::string nodeId);
    int GetAsIndexByAsId(std::string asId);
    int GetAsIndexByNodeId(std::string nodeId);
    // retrieve information by Node-/AsId

    void PrintMaps(std::string spacer = "");
    // print all maps for debugging purposes

  private:
    void PrintNodeToAsMap(std::string spacer = "");
    void PrintAsToIndexMap(std::string spacer = "");

  protected:
    std::unordered_map<std::string, std::string>
        m_nodeIdToAsId; // maps nodeId to ownerAsId for convenient lookup
    std::unordered_map<std::string, int>
        m_asIdToAsVectorIndex; // map asId to index of some corresponding vector
};

} // namespace ns3

#endif /* NODE_LOOKUP_MAPPER_H */