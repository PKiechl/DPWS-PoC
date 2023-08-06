#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "ns3/core-module.h"

namespace ns3
{

/*
 * Base class for all configuration classes. Enforces that they all provide an implementation
 * of PrintConfiguration.
 */

class Configuration
{
  public:
    virtual void PrintConfiguration(std::string spacer="");
    virtual ~Configuration();
};

} // namespace ns3

#endif /* CONFIGURATION_H */