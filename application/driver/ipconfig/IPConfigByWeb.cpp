/** \file IPConfigByWeb.cpp
*
* Written by: Jesper Larsen, jla@iotech.dk
* Company: IO Technologies A/S, Egaa, Denmark
*
* Project: Grundfos MPC DHCP / IP configuration
* Projectno.: 5072
* File: IPConfigByWeb.cpp
*
* $Log: IPConfigByWeb.cpp,v $
* Revision 1.1.2.9  2007/03/02 15:37:00  xfk (Jesper Freksen, jf@cim-electronics.com)
* Added support for mac address
*
* Revision 1.1.2.8  2005/06/10 12:41:01  jla
* Fixed errormessage
*
* Revision 1.1.2.7  2005/05/27 13:23:51  jla
* Finished documentation
*
* Revision 1.1.2.6  2005/05/26 09:24:03  jla
* Added overloaded start method and added conversion of hostname characters
*
* Revision 1.1.2.5  2005/05/20 14:28:40  jla
* Added errorhandling. testIPinUse now works.
*
* Revision 1.1.2.4  2005/05/14 10:22:09  jla
* Changed handling of all empty fields
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

#define TRACE(x)

#include "IPConfigByWeb.h"
#include "MPCWebServer.h"
#include "DHCPClient.h"
#include "RTIPDHCPClient.h"

IPConfigByWeb* IPConfigByWeb::mpinstance = 0;

IPConfigByWeb::IPConfigByWeb()
: mdhcpenabled(false),
mpipconfiguration(0),
mwaitingforconfig(false),
mupdateipconfig(false)
{
}

IPConfigByWeb::~IPConfigByWeb()
{
  if (mpipconfiguration)
  {
    mpipconfiguration->detach(this);
  }
}

IPConfigByWeb* IPConfigByWeb::getInstance()
{
  if (IPConfigByWeb::mpinstance == 0)
    IPConfigByWeb::mpinstance = new IPConfigByWeb;

  return IPConfigByWeb::mpinstance;
}

/**
* This method will also attach observer (IPConfigByWeb) to subject
* (IPConfiguration)
*/
void IPConfigByWeb::setIPConfigurationPtr(IPConfiguration *pipconf)
{
  if (mpipconfiguration)
  {
    mpipconfiguration->detach(this);
  }

  mpipconfiguration = pipconf;
  mpipconfiguration->attach(this);
}

/**
* If we are waiting for a configuration from a DHCP server, the text response
* for the webpage is changed accordingly.
*
* If the run method is about to update the IPConfiguration,
* calls to update are ignored.
*/
void IPConfigByWeb::update(SubjectIO *subject)
{
  if (subject == mpipconfiguration && !mupdateipconfig)
  {
    msemaphore.enter();

    mdhcpenabled = mpipconfiguration->getDHCPisEnabled();
    mpipconfiguration->getHostname(mhostname);
    mpipconfiguration->getMacAddress(mmacaddress);

    mipaddress = mpipconfiguration->getIPAddress();
    msubnetmask = mpipconfiguration->getSubnetMask();
    mdefaultgateway = mpipconfiguration->getDefaultGateway();
    mprimarydns = mpipconfiguration->getPrimaryDNS();
    msecondarydns = mpipconfiguration->getSecondaryDNS();

    mactipaddress = mpipconfiguration->getActualIPAddress();
    mactsubnetmask = mpipconfiguration->getActualSubnetMask();
    mactdefaultgateway = mpipconfiguration->getActualDefaultGateway();
    mactprimarydns = mpipconfiguration->getActualPrimaryDNS();
    mactsecondarydns = mpipconfiguration->getActualSecondaryDNS();

    if (mwaitingforconfig)
    {
      mwaitingforconfig = false;
      if (mdhcpenabled)
        mtextresponse = "Got new IP configuration from DHCP server.";
      else
        mtextresponse = "IP configuration has been updated.";      
    }

    msemaphore.exit();
  }
}
/**The task priority and stack size is defined in IPConfigByWeb.h
*/
void IPConfigByWeb::start()
{
  Thread::start(WEBCONFIGTASKPRIO, "IPConfigByWeb task", WEBCONFIGTASKSTACKSIZE);
}

/**Instead of updating the IPConfiguration directly from the postSubmitIPConfiguration
* method, it is done here after a small delay. This makes it possible to return
* a response to the webbrowser before the IP address is changed.
* The response instructs the webbrowser  to reload the IP Configuration page
* (ipconfig.html) from the new IP address.
*/
void IPConfigByWeb::run()
{
  while(1)
  {
    if (mupdateipconfig)
    {
      delay(WEBCONFIGRUNDELAY_MS);		//Wait here to make sure httpresponse has reached browserclient

      msemaphore.enter();
      if (mpipconfiguration)
      {
        mupdateipconfig = false;

        mpipconfiguration->setNewIPConfig(mdhcpenabled,
          mipaddress,
          msubnetmask,
          mdefaultgateway,
          mprimarydns,
          msecondarydns,
          mhostname);

        if (mdhcpenabled)
        {
    	    RTIPDHCPClient::getInstance()->renew();
        }

        TRACE(OS_DebugSendString("Updated IP config."););
      }
      else
      {
        string es("Null pointer in IPConfigByWeb::run method.");

        IPConfigErrorHandler::getInstance()->setErrorNo(IPCRUNTIMEERROR);
        IPConfigErrorHandler::getInstance()->setDetailedErrorText(es);

        TRACE(OS_DebugSendString("Unable to update IP config."););
      }

      msemaphore.exit();
    }
    else
    {
      delay(WEBCONFIGRUNDELAY_MS);
    }
  }
}

/**Testing whether the IP address is in use already is done by looking in the
* ARP cache and pinging the IP address.
*
* First the ARP cache is searched for the IP address. If the IP address is found
* here, it is assumed to be present on the network. If the IP address is not
* found in the ARP cache, it is pinged. If the ping is answered, the IP address
* is already in use.
*
* It is not possible to first ping, and then looking in the ARP cache, as RTIP
* will put an entry for the IP address into the ARP cache, regardless of it
* being on the network or not.
*
* \note This method is not 100% fault proof, as a node on the network may ignore
* PING requests.
*
* \param testip IP address to test.
* \return True if address is in use, false if not in use.
*/
bool IPConfigByWeb::testIPInUse(IPAddress testip)
{
  bool retval = false;

  if (testip != mipaddress)				//Same as current IP address?
  {
    if (IPUtils::delFromArp(testip))
    {
      retval = true;
    }
    else
    {
      if (IPUtils::ping(testip))
      {
        retval = true;
      }
    }
  }

  return retval;
}

void IPConfigByWeb::webifGetDHCPEnabled(std::ostream& res)
{
  if (mdhcpenabled)
    res << "Enabled";
  else
    res << "Disabled";
}

void IPConfigByWeb::webifGetHostName(std::ostream& res)
{
  res << mhostname;
}

void IPConfigByWeb::webifGetIPAddress(bool actual, std::ostream& res)
{
  if (actual)
    res << mactipaddress;
  else  
    res << mipaddress;
}

void IPConfigByWeb::webifGetSubnetMask(bool actual, std::ostream& res)
{
  if (actual)
    res << mactsubnetmask;
  else  
    res << msubnetmask;
}

void IPConfigByWeb::webifGetDefaultGateway(bool actual, std::ostream& res)
{
  if (actual)
    res << mactdefaultgateway;
  else  
    res << mdefaultgateway;
}

void IPConfigByWeb::webifGetPrimaryDNS(bool actual, std::ostream& res)
{
  if (actual)
    res << mactprimarydns;
  else  
    res << mprimarydns;
}

void IPConfigByWeb::webifGetSecondaryDNS(bool actual, std::ostream& res)
{
  if (actual)
    res << mactsecondarydns;
  else  
    res << msecondarydns;
}

void IPConfigByWeb::webifGetTextMessage(std::ostream& res)
{
  res << mtextresponse;
  
  // delete old response message
  mtextresponse = "";
}

/*
* This method primarily handles the data submitted from the IP configuration
* webpage and returns a HTML response to the webbrowser. The Update of the
* IP configuration itself is handled by the run method.
*
* \param dhcpenabled true if dhcp is enabled.
* \param hostname new hostname 
* \param ipaddress new IP Address if DHCP disabled
* \param subnetmask new subnet mask if DHCP disabled
* \param defaultgateway new default gateway if DHCP disabled
* \param primarydns new primary DNS if DHCP disabled
* \param secondarydns new secondary DNS if DHCP disabled
*
* \return Reference to string containing HTML response to be sent to browser.
*/
string& IPConfigByWeb::webifPostNewConfig(bool dhcpenabled, 
                                          const char* hostname,
                                          const char* ipaddress,
                                          const char* subnetmask,
                                          const char* defaultgateway,
                                          const char* primarydns,
                                          const char* secondarydns)
{
  string temp;
  string refreshipaddr;
  bool error = false;
  bool doupdate = false;							//Dont update if all fields are empty
  IPAddress newipaddress = mipaddress;		//Use existing IP addr. if none specified
  IPAddress newsubnetmask = msubnetmask;
  IPAddress newdefaultgateway = mdefaultgateway;
  IPAddress newprimarydns = mprimarydns;
  IPAddress newsecondarydns = msecondarydns;

  msemaphore.enter();

  if (dhcpenabled)
  {
    mdhcpenabled = true;

    //Return response to wait and then auto-update page
    mtextresponse = "Waiting for configuration from DHCP server...";

    string hostnameStr = hostname;

    if (IPUtils::isHostnameValid(hostnameStr) )
    {
      mhostname = hostname;
    }
    else
    {
      error = true;
      mtextresponse = "The entered <b>Host name</b> is not valid: " + hostnameStr + '.';
    }
    
    if(!error)
    {
      mwaitingforconfig = true;
      mupdateipconfig = true;
    }
  }
  else
  {
    mdhcpenabled = false;
    string hostnameStr = hostname;

    if (IPUtils::isHostnameValid(hostnameStr) )
    {
      mhostname = hostname;

      doupdate = true;
    }
    else
    {
       error = true;
       mtextresponse = "The entered <b>Host name</b> is not valid: " + hostnameStr + '.';
    }
    
    temp = ipaddress;
    if (!error && temp.length() > 0)
    {
      doupdate = true;
      if (!newipaddress.fromString(temp))
      {
        error = true;
        mtextresponse = "The entered <b>IP address</b> is not valid: " + temp + '.';
      }
      else if (testIPInUse(newipaddress))
      {
        string newip;

        error = true;

        newipaddress.toString(newip);
        mtextresponse = "This IP address (<b>";
        mtextresponse += newip;
        mtextresponse += "</b>) is already in use on the network.";

        IPConfigErrorHandler::getInstance()->setErrorNo(IPCIPADDRESSINUSE);
        IPConfigErrorHandler::getInstance()->setDetailedErrorText(newip);
      }
    }

    temp = subnetmask;
    if (!error && temp.length() > 0)
    {
      doupdate = true;
      if (!newsubnetmask.fromString(temp))
      {
        error = true;
        mtextresponse = "The entered <b>Subnet mask</b> is not valid: " + temp + '.';
      }
    }

    temp = defaultgateway;
    if (!error && temp.length() > 0)
    {
      doupdate = true;
      if (!newdefaultgateway.fromString(temp))
      {
        error = true;
        mtextresponse = "The entered <b>Default gateway</b> is not valid: " + temp + '.';
      }
    }

    // primary DNS
    if (!error && primarydns)
    {
      temp = primarydns;
      if (!error && temp.length() > 0)
      {
        doupdate = true;
        if (!newprimarydns.fromString(temp))
        {
          error = true;
          mtextresponse = "The entered <b>Primary DNS server</b> is not valid: " + temp + '.';
        }
      }
    }

    // secondary DNS
    temp = secondarydns;
    if (!error && temp.length() > 0)
    {
      doupdate = true;
      if (!newsecondarydns.fromString(temp))
      {
        error = true;
        mtextresponse = "The entered <b>Secondary DNS server</b> is not valid: " + temp + '.';
      }
    }

    if (!error && doupdate)
    {
      mipaddress = newipaddress;
      msubnetmask = newsubnetmask;
      mdefaultgateway = newdefaultgateway;
      mprimarydns = newprimarydns;
      msecondarydns = newsecondarydns;
      mtextresponse = "Please wait, updating IP configuration...";

      mipaddress.toString(refreshipaddr);

      mwaitingforconfig = true;
      mupdateipconfig = true;
    }
  }
  msemaphore.exit();

  int endwait = 0;
  while(endwait < 30)
  {
    msemaphore.enter();
    if(!(mwaitingforconfig))
    {
      endwait = 100;
      msemaphore.exit();
      break;
    }
    msemaphore.exit();
    ++endwait;
    OS_Delay(1000);
  }

  msemaphore.enter();
  mactipaddress.toString(refreshipaddr);
  
  mhttpresponse =  "<meta http-equiv=\"Refresh\" content=\"2;URL=http://";
  mhttpresponse += refreshipaddr;
  mhttpresponse += "/ipconfig.html\" />";
  
  msemaphore.exit();

  return mhttpresponse;
}

