/** \file IPStackConfigurator.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPStackConfigurator.h
 *
 * $Log: IPStackConfigurator.h,v $
 * Revision 1.4.2.3  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.4.2.2  2005/05/20 14:31:22  jla
 * Now a singleton. Added refreshIPConfig method.
 *
 * Revision 1.4.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.3  2005/04/12 14:13:09  jla
 * Scratch version
 *
 * Revision 1.2  2005/04/06 12:07:31  jla
 * A little more (CVS/Doxygen test)
 *
 * Revision 1.1  2005/04/06 12:04:49  jla
 * Scratch version
 *
 */

#ifndef _IPSTACKCONFIGURATOR_H_
#define _IPSTACKCONFIGURATOR_H_

class IPStackConfigurator;

#include "ObserverIO.h"
#include "IPAddress.h"
#include "IPConfiguration.h"
#include "IPStackConfigurationInterface.h"
#include "DHCPClient.h"

using namespace std;

/**\brief Abstract baseclass for implementing an IP stack configurator.
 *
 * This is a mediator class, handling a DHCP Client and configuration of
 * an IP Stack.
 *
 * An IPConfiguration object is observed and the configuration of the
 * IP stack is automatically updated, if the IPConfiguration object
 * changes its state.
 */
class IPStackConfigurator : public ObserverIO
{
public:
  virtual ~IPStackConfigurator();

  static IPStackConfigurator* getInstance();    //!<Get pointer to the singleton instance

  virtual void update(SubjectIO *subject);         //!<Update method for the observer pattern
  void newDHCPConfiguration(IPAddress ipaddress,
                IPAddress subnetmask,
                IPAddress defaultgateway,
                IPAddress primarydns,
                IPAddress secondarydns);  //!<Set new configuration from DHCP

  void refreshIPConfig();               //!<Refresh IP address, subnetmask and default gateway

  void setObjectPtrs( IPConfiguration *pipconfiguration,
            DHCPClient *pdhcpclient,
            IPStackConfigurationInterface *pipstackconfif); //!<Set pointers to objects

protected:
  IPStackConfigurator();

private:
  bool mupdating;                   //!<True when IP configuration is being updated
  IPConfiguration *mpipconfiguration;
  DHCPClient *mpdhcpclient;
  IPStackConfigurationInterface *mpipstackconfif;

  static IPStackConfigurator* mpinstance;
};

#endif //_IPSTACKCONFIGURATOR_H_
