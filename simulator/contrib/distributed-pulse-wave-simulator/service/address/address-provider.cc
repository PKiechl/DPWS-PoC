#include "address-provider.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AddressProvider");

AddressProvider::AddressProvider(Ipv4Address address, Ipv4Mask mask)
{
    m_addressHelper = Ipv4AddressHelper(address, mask);
}

Ipv4AddressHelper*
AddressProvider::GetAddressHelper()
{
    return &m_addressHelper;
}
