#ifndef ADDRESS_PROVIDER_H
#define ADDRESS_PROVIDER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

/*
 * Strictly speaking, this class is not required, as in its current state it only wraps an
 * Ipv4AddressHelper instance and has no real logic on its own
 *
 * The PURPOSE of the class is to have a way to draw addresses from the same subnet for all the
 * different AS <-> Central Network connections, thus not "consuming" a separate subnet for each
 * such connection.
 *
 * Historically, this class functioned differently and had its own logic, but the approach with the
 * Ipv4AddressHelper proved superior and also simpler.
 *
 * Still, I have decided to keep the class in here, in case that future needs will require a more
 * intricate approach.
 */

namespace ns3
{

// inherit from SimpleRefCounter for access to ns3 Smart Pointer Class Ptr<>
class AddressProvider : public SimpleRefCount<AddressProvider>
{
  public:
    AddressProvider(Ipv4Address address, Ipv4Mask mask);
    Ipv4AddressHelper* GetAddressHelper();
    // cannot use ns-3 built in Ptr<>, apparently SimpleRefCount not part of that class hierarchy

  private:
    Ipv4AddressHelper m_addressHelper;
};

} // namespace ns3

#endif /* ADDRESS_PROVIDER_H */