/** \file DHCPClient.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: DHCPClient.c
 *
 * $Log: DHCPClient.cpp,v $
 * Revision 1.1.2.6  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.5  2005/05/26 09:21:40  jla
 * Added overloaded start method
 *
 * Revision 1.1.2.4  2005/05/20 14:27:32  jla
 * *** empty log message ***
 *
 * Revision 1.1.2.3  2005/05/14 10:23:57  jla
 * Added get method for hostname
 *
 * Revision 1.1.2.2  2005/05/12 14:02:43  jla
 * Added dorenew member
 *
 * Revision 1.1.2.1  2005/05/12 07:23:36  jla
 * Added members for renew/rebind times and removed cout-s
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#include "DHCPClient.h"


DHCPClient* DHCPClient::mpinstance=0;

DHCPClient::DHCPClient()
 : menabled(false),
   mgotlease(false),
   mdorenew(false),
   mpipstackconfigurator(0)
{
}

DHCPClient::~DHCPClient()
{
  disable();          //Disable and release before shutting down
}


void DHCPClient::enable()
{
  menabled = true;
}

/**If the DHCP client is enabled, and has leased an IP address, the IP address
 * is released, before disabling.
 */
void DHCPClient::disable()
{
  if (menabled)
  {
    menabled = false;
    if (mgotlease)
    {
      release();
    }
  }
}

const string& DHCPClient::getHostname()
{
  return mhostname;
}

void DHCPClient::setHostname(string &hostname)
{
  mhostname = hostname;
}

bool DHCPClient::isEnabled()
{
  return menabled;
}

bool DHCPClient::gotLease()
{
  return mgotlease;
}

/**When the DHCP client gets a new lease from the DHCP server, this method
 * must be called, so the IPConfiguration object can be updated. This is
 * done via the IPStackConfigurator object.
 */
void DHCPClient::gotNewLease()
{
  mpipstackconfigurator->newDHCPConfiguration(
    mipaddress,
    msubnetmask,
    mdefaultgateway,
    mprimarydns,
    msecondarydns);
}

/**Call this method with a valid pointer before starting and enabling the
 * DHCP client.
 */
void DHCPClient::setIPStackConfPtr(IPStackConfigurator *p)
{
  mpipstackconfigurator = p;
}

/**The task priority and stack size is defined in DHCPClient.h
 */
void DHCPClient::start()
{
  Thread::start(DHCPCLITASKPRIO, "DHCP Client task", DHCPCLITASKSTACKSIZE);
}
