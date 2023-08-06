#include "node-lookup-mapper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NodeLookupMapper");

void
NodeLookupMapper::AddNodeToAsEntry(std::string nodeId, std::string asId)
{
    if (m_nodeIdToAsId.count(nodeId) != 0)
    {
        NS_LOG_WARN("NodeId=>AsId map already contains key " + nodeId +
                    ". Ignoring duplicate entry. You should double-check your configuration.");
        return;
    }
    m_nodeIdToAsId[nodeId] = asId;
}

void
NodeLookupMapper::AddAsToAsIndexEntry(std::string asId, int index)
{
    if (m_asIdToAsVectorIndex.count(asId) != 0)
    {
        NS_LOG_WARN("AsId=>AsVectorIndex map already contains key " + asId +
                    ". Ignoring duplicate entry. You should double-check your configuration.");
        return;
    }
    m_asIdToAsVectorIndex[asId] = index;
}

std::string
NodeLookupMapper::GetAsIdByNodeId(std::string nodeId)
{
    return m_nodeIdToAsId.at(nodeId);
}

int
NodeLookupMapper::GetAsIndexByAsId(std::string asId)
{
    return m_asIdToAsVectorIndex.at(asId);
}

int
NodeLookupMapper::GetAsIndexByNodeId(std::string nodeId)
{
    return m_asIdToAsVectorIndex.at(m_nodeIdToAsId.at(nodeId));
}

void
NodeLookupMapper::PrintMaps(std::string spacer)
{
    PrintNodeToAsMap(spacer);
    PrintAsToIndexMap(spacer);
}

void
NodeLookupMapper::PrintNodeToAsMap(std::string spacer)
{
    std::cout << spacer << "-----------------------------" << std::endl;
    std::cout << spacer << "Printing {Node id => As Id} Map" << std::endl;
    for (auto [key, value] : m_nodeIdToAsId) {
        std::cout << spacer << key << " => " << value << std::endl;
    }
    std::cout << spacer << "-----------------------------" << std::endl;
}

void
NodeLookupMapper::PrintAsToIndexMap(std::string spacer)
{
    std::cout << spacer << "-------------------------------------" << std::endl;
    std::cout << spacer << "Printing {As id => As Vector} Index Map" << std::endl;
    for (auto [key, value] : m_asIdToAsVectorIndex) {
        std::cout << spacer << key << " => " << value << std::endl;
    }
    std::cout << spacer << "-------------------------------------" << std::endl;
}