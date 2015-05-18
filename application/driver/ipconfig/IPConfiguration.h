/** \file IPConfiguration.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPConfiguration.h
 *
 * $Log: IPConfiguration.h,v $
 * Revision 1.1.2.3  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.2  2005/05/20 14:29:45  jla
 * Now a singleton
 *
 * Revision 1.1.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#ifndef _IPCONFIGURATION_H_
#define _IPCONFIGURATION_H_

#include <string>
#include <iostream>
#include "SubjectIO.h"
#include "IPAddress.h"

using namespace std;

/**\brief This class contains a basic IP configuration.
 *
 * Two configurations are contained in the IPConfiguration class:
 * The actual configuration and "manual" configuration. The two
 * configurations will only be different when using DCHP. \n
 * Maintaining the actual configuration is the client's
 * responsibility.
 *
 * This is a Subject class in an observer patten.
 * Clients must inherit from the Observer class and implement the update
 * method. The update method is called when IPConfiguration changes its state.
 *
 */
class IPConfiguration : public SubjectIO
{
public:
  virtual ~IPConfiguration();

  static IPConfiguration* getInstance();  //!<Get pointer to singleton instance.

  void setNewIPConfig(bool dhcpenabled,
            IPAddress ipaddress,
            IPAddress subnetmask,
            IPAddress defaultgateway,
            IPAddress primarydns,
            IPAddress secondarydns,
            string &hostname);  //!<Set new IP configuration

  void setNewIPConfig(bool dhcpenabled,
            IPAddress ipaddress,
            IPAddress subnetmask,
            IPAddress defaultgateway,
            string &hostname);  //!<Set new IP configuration
  
  void setDHCPEnabled(bool enabled);    //!<Set DHCP enabled or disabled
  //void setDHCPFaultStatus(bool fault);  //!<Set DHCP fault status
  //void setNIAutoDisabled(bool disabled);  //!<Set flag indicating Network Auto disabled
  void setIPAddress(IPAddress );      //!<Set IP address
  void setSubnetMask(IPAddress );     //!<Set Subnetmask
  void setDefaultGateway(IPAddress );   //!<Set default gateway
  void setPrimaryDNS(IPAddress );   //!<Set primary DNS
  void setSecondaryDNS(IPAddress );   //!<Set secondary DNS
  void setHostname(string &);       //!<Set hostname

  void setActualIPAddress(IPAddress );  //!<Set Actual IP address
  void setActualSubnetMask(IPAddress ); //!<Set Actual Subnetmask
  void setActualDefaultGateway(IPAddress ); //!<Set Actual default gateway
  void setActualPrimaryDNS(IPAddress ); //!<Set Actual primary DNS
  void setActualSecondaryDNS(IPAddress ); //!<Set Actual secondary DNS
  void setMacAddress(string &);

  bool getDHCPisEnabled();        //!<Get DHCP enabled or disabled
  IPAddress getIPAddress();       //!<Get IP address
  IPAddress getSubnetMask();        //!<Get subnet mask
  IPAddress getDefaultGateway();      //!<Get default gateway
  IPAddress getPrimaryDNS();  //!<Get primary DNS
  IPAddress getSecondaryDNS();  //!<Get secondary DNS
  void getHostname(string &);       //!<Get hostname
  void getMacAddress(string &);       //!<Get mac address
  
  IPAddress getActualIPAddress();     //!<Get Actual IP address
  IPAddress getActualSubnetMask();    //!<Get Actual subnet mask
  IPAddress getActualDefaultGateway();  //!<Get Actual default gateway
  IPAddress getActualPrimaryDNS();  //!<Get Actual primary DNS
  IPAddress getActualSecondaryDNS();  //!<Get Actual secondary DNS

protected:
  IPConfiguration();            //!<Constructor

private:
  IPAddress mipaddress;
  IPAddress msubnetmask;
  IPAddress mdefaultgateway;
  IPAddress mprimarydns;
  IPAddress msecondarydns;
  IPAddress mactipaddress;
  IPAddress mactsubnetmask;
  IPAddress mactdefaultgateway;
  IPAddress mactprimarydns;
  IPAddress mactsecondarydns;
  string mhostname;
  string mmacaddress;
  bool mdhcpenabled;

  static IPConfiguration* mpinstance;
};

#endif //_IPCONFIGURATION_H_
