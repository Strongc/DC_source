/** \file IPConfiguration.cpp
*
* Written by: Jesper Larsen, jla@iotech.dk
* Company: IO Technologies A/S, Egaa, Denmark
*
* Project: Grundfos MPC DHCP / IP configuration
* Projectno.: 5072
* File: IPConfiguration.c
*
* $Log: IPConfiguration.cpp,v $
* Revision 1.1.2.3  2005/05/27 13:23:51  jla
* Finished documentation
*
* Revision 1.1.2.2  2005/05/20 14:29:45  jla
* Now a singleton
*
* Revision 1.1.2.1  2005/05/12 14:05:23  jla
* Now also setting hostname when DHCP is disabled
*
* Revision 1.1  2005/04/12 14:13:09  jla
* Scratch version
*
*/

#include "IPConfiguration.h"
#include "DHCPClient.h"

IPConfiguration* IPConfiguration::mpinstance = 0;

IPConfiguration::IPConfiguration()
: mdhcpenabled(false)

{
}

IPConfiguration::~IPConfiguration()
{
}

IPConfiguration* IPConfiguration::getInstance()
{
  if (mpinstance == 0)
  {
    mpinstance = new IPConfiguration;
  }

  return mpinstance;
}

/**If DHCP is enabled, it is not necessary to specify ipaddress,
* subnetmask and default gateway. Specifying hostname is optional.
* I DHCP is enabled, ipaddress, subnetmask and default gateway must
* be specified but the hostname is not used.
*
* This method will call the Subject notify method.
*/
void IPConfiguration::setNewIPConfig(bool dhcpenabled,
                                     IPAddress ipaddress,
                                     IPAddress subnetmask,
                                     IPAddress defaultgateway,
                                     IPAddress primarydns,
                                     IPAddress secondarydns,
                                     string &hostname)
{
  if (mdhcpenabled = dhcpenabled)
  {
    setHostname(hostname);
  }
  else
  {
    setIPAddress(ipaddress);
    setSubnetMask(subnetmask);
    setDefaultGateway(defaultgateway);
    setPrimaryDNS(primarydns);
    setSecondaryDNS(secondarydns);

    setHostname(hostname);
  }

  notify();
}

void IPConfiguration::setNewIPConfig(bool dhcpenabled,
                                     IPAddress ipaddress,
                                     IPAddress subnetmask,
                                     IPAddress defaultgateway,
                                     string &hostname)
{

    IPAddress primarydns = IPAddress(0,0,0,0);
    IPAddress secondarydns = IPAddress(0,0,0,0);    

    IPConfiguration::setNewIPConfig( dhcpenabled,
                                      ipaddress,
                                      subnetmask,
                                      defaultgateway,
				      primarydns,
				      secondarydns,
                                      hostname);
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setDHCPEnabled(bool enabled)
{
  mdhcpenabled = enabled;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setIPAddress(IPAddress ipadr)
{
  mipaddress = ipadr;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setSubnetMask(IPAddress mask)
{
  msubnetmask = mask;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setDefaultGateway(IPAddress ipadr)
{
  mdefaultgateway = ipadr;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setPrimaryDNS(IPAddress ipadr)
{
  mprimarydns = ipadr;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setSecondaryDNS(IPAddress ipadr)
{
  msecondarydns = ipadr;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setHostname(string &hostname)
{
  if(hostname.length() >= DHCPMAXHOSTNAMELENGTH)
  {
    mhostname = hostname.substr(0, DHCPMAXHOSTNAMELENGTH - 1);
  }
  else
  {
    mhostname = hostname;
  }
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setActualIPAddress(IPAddress ipadr)
{
  mactipaddress = ipadr;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setActualSubnetMask(IPAddress mask)
{
  mactsubnetmask = mask;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setActualDefaultGateway(IPAddress ipadr)
{
  mactdefaultgateway = ipadr;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setActualPrimaryDNS(IPAddress ipadr)
{
  mactprimarydns = ipadr;
}

/**
* \note notify must be called afterwards.
*/
void IPConfiguration::setActualSecondaryDNS(IPAddress ipadr)
{
  mactsecondarydns = ipadr;
}

void IPConfiguration::setMacAddress(string &macAddress)
{
  mmacaddress = macAddress;
}

bool IPConfiguration::getDHCPisEnabled()
{
  return mdhcpenabled;
}

IPAddress IPConfiguration::getIPAddress()
{
  return mipaddress;
}

IPAddress IPConfiguration::getSubnetMask()
{
  return msubnetmask;
}

IPAddress IPConfiguration::getDefaultGateway()
{
  return mdefaultgateway;
}

IPAddress IPConfiguration::getPrimaryDNS()
{
  return mprimarydns;
}

IPAddress IPConfiguration::getSecondaryDNS()
{
  return msecondarydns;
}
void IPConfiguration::getHostname(string &hostname)
{
  hostname = mhostname;
}

IPAddress IPConfiguration::getActualIPAddress()
{
  return mactipaddress;
}

IPAddress IPConfiguration::getActualSubnetMask()
{
  return mactsubnetmask;
}

IPAddress IPConfiguration::getActualDefaultGateway()
{
  return mactdefaultgateway;
}

IPAddress IPConfiguration::getActualPrimaryDNS()
{
  return mactprimarydns;
}

IPAddress IPConfiguration::getActualSecondaryDNS()
{
  return mactsecondarydns;
}

void IPConfiguration::getMacAddress(string &macaddress)
{
  macaddress = mmacaddress;
}

