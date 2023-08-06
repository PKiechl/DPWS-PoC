#include "dpws-node.h"

using namespace ns3;

Ptr<Node>
DPWSNode::GetNs3Node()
{
    return m_ns3Node;
}

Ipv4Address
DPWSNode::GetAssignedIpv4Address()
{
    return m_assignedAddress;
}