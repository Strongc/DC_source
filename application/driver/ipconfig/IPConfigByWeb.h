/** \file IPConfigByWeb.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPConfigByWeb.h
 *
 * $Log: IPConfigByWeb.h,v $
 * Revision 1.1.2.6  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.5  2005/05/26 09:24:03  jla
 * Added overloaded start method and added conversion of hostname characters
 *
 * Revision 1.1.2.4  2005/05/20 14:28:40  jla
 * Added errorhandling. testIPinUse now works.
 *
 * Revision 1.1.2.3  2005/05/12 14:02:11  jla
 * Added text response CGI and cleaned up post method
 *
 * Revision 1.1.2.2  2005/05/09 13:16:06  jla
 * Added some thing for response to user
 *
 * Revision 1.1.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 */

#ifndef _IPCONFIGBYWEB_H_
#define _IPCONFIGBYWEB_H_

#include <string>
#include "IPAddress.h"
#include "IPConfiguration.h"
#include "IPUtils.h"
#include "Thread.h"
#include "Semaphore.h"
#include "IPConfigErrorHandler.h"

using namespace std;

#define WEBCONFIGRUNDELAY_MS  1000      //!<Delay for run method
#define WEBCONFIGTASKPRIO   10        //!<Priority of IPConfigByWeb task
#define WEBCONFIGTASKSTACKSIZE  (2*1024)    //!<Size of stack for IPConfigByWeb task


/**\brief Handles the webpage for IP Configuration.
 *
 * It uses the oberver pattern to observe the current IP configuration, so
 * the information on the webpage can be updated, if it is changed from another
 * point in the system.
 *
 * (The webpage is not updated until it is refreshed or reloaded.)
 *
 * CGI functions:
 * - get_dhcp_enabled.fn
 * - get_hostname.fn
 * - get_ip_address.fn
 * - get_subnet_mask.fn
 * - get_default_gateway.fn
 *
 * Post functions:
 * - ipconfig_submit.fn
 *
 */
class IPConfigByWeb : public ObserverIO, public Thread
{
public:
  virtual ~IPConfigByWeb();

  static IPConfigByWeb *getInstance();  //!<Get pointer to the singleton instance

  void setIPConfigurationPtr(IPConfiguration *);  //!<Set pointer to IPConfiguration subject.

  bool testIPInUse(IPAddress);    //!<Test if IP address is in use already on the network

  void start();           //!<Overloads the Thread::start method
  virtual void update(SubjectIO *); //!<Called by subject when IPConfiguration is changed
  
  // methods called by the WebIfHandler
  void webifGetDHCPEnabled(std::ostream& res);
  void webifGetHostName(std::ostream& res);
  void webifGetIPAddress(bool actual, std::ostream& res);
  void webifGetSubnetMask(bool actual, std::ostream& res);
  void webifGetDefaultGateway(bool actual, std::ostream& res);
  void webifGetPrimaryDNS(bool actual, std::ostream& res);
  void webifGetSecondaryDNS(bool actual, std::ostream& res);
  void webifGetTextMessage(std::ostream& res);
  string& webifPostNewConfig( bool dhcpEnabled, 
                              const char* hostname,
                              const char* ipaddress,
                              const char* subnetmask,
                              const char* defaultgateway,
                              const char* primarydns,
                              const char* secondarydns);
  
protected:
  IPConfigByWeb();
  virtual void run();     //!<Updates IPConfiguration

private:
  IPAddress mipaddress;
  IPAddress msubnetmask;
  IPAddress mdefaultgateway;
  IPAddress mprimarydns;
  IPAddress msecondarydns;
  string mhostname;
  string mmacaddress;
  bool mdhcpenabled;

  IPAddress mactipaddress;
  IPAddress mactsubnetmask;
  IPAddress mactdefaultgateway;
  IPAddress mactprimarydns;
  IPAddress mactsecondarydns;

  bool mwaitingforconfig;     //!<True when waiting for config (static or DHCP)
  bool mupdateipconfig;       //!<True when run method should update ip configuration.

  string mhttpresponse;       //!<Response returned to webbrowser
  string mtextresponse;       //!<Text response to user

  Semaphore msemaphore;       //!<Protects against multiple access to members

  IPConfiguration *mpipconfiguration;

  static IPConfigByWeb *mpinstance; //!>Points to the one and only instance.
};

#endif //_IPCONFIGBYWEB_H_
