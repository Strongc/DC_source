/** \file DHCPCLient.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: DHCPClient.h
 *
 * $Log: DHCPClient.h,v $
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

#ifndef _DHCPCLIENT_H_
#define _DHCPCLIENT_H_

class DHCPClient;

#include <string>
#include "Thread.h"
#include "IPAddress.h"
#include "IPStackConfigurator.h"

using namespace std;

#define DHCPMAXHOSTNAMELENGTH 64        //!<Maximum length of hostname
#define DHCPLOOPTIME_MS     1000      //!<Delaytime in loop
#define DHCPMINFAULTDUR     6       //!<Number of loop run throughs before setting fault status

#define DHCPCLITASKPRIO     10        //!<Priority of DHCP client task
#define DHCPCLITASKSTACKSIZE  (2*1024)    //!<Size of stack for DHCP client task


/**\brief Baseclass for implementing a DHCP Client
 *
 * Inherit from this class to implement af DHCP client for a specific IP stack.
 * This is a singleton class, subclasses need to implement a getInstance method.
 */
class DHCPClient : public Thread
{
public:
  virtual ~DHCPClient();

  virtual void enable();            //!<Enables the DHCP client
  virtual void disable();           //!<Disables the DHCP client

  virtual void renew()=0;           //!<Renew IP address lease
  virtual void release()=0;         //!<Release IP address

  const string& getHostname();        //!<Return client hostname
  void setHostname(string &);         //!<Set client hostname

  bool isEnabled();             //!<Test if DHCP client is enabled
  bool gotLease();              //!<Test if DHCP client has received a lease

  void start();               //!<Start the DHCP client thread

  void setIPStackConfPtr(IPStackConfigurator *);  //!<Set pointer to IPStackConfigurator object


protected:
  DHCPClient();

  void gotNewLease();       //!<Call when DHCP client got a new lease

  virtual void run()=0;     //!<Implement in a concrete subclass

  bool menabled;          //!<True when DHCP client is enabled.
  bool mgotlease;         //!<True when an IP address has been obtained.
  bool mdorenew;          //!<True when a renew is requested.

  unsigned int mleasetime;    //!<Lease time received from DHCP server, in seconds
  unsigned int mleasegone;    //!<Time since lease was aquired/renewed, in seconds
  unsigned int mrenewtime;    //!<Time for renew lease, in seconds
  unsigned int mrebindtime;   //!<Time for rebind lease, in seconds

  IPAddress mipaddress;     //!<Current ip address in host byte order
  IPAddress msubnetmask;      //!<Current subnetmask in host byte order
  IPAddress mdefaultgateway;    //!<Current def. gateway in host byte order
  IPAddress mprimarydns;     //!<Current def. DNS in host byte order
  IPAddress msecondarydns;     //!<Current def. DNS in host byte order

  string mhostname;       //!<Client hostname
  static DHCPClient* mpinstance;  //!<Pointer to the one and only instance

  IPStackConfigurator *mpipstackconfigurator; //!<Pointer to IPStackConfigurator, needed in gotNewLease

private:

};

#endif //_DHCPCLIENT_H_

