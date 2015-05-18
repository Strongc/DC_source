/** \file RTIPDHCPClient.cpp
*
* Written by: Jesper Larsen, jla@iotech.dk
* Company: IO Technologies A/S, Egaa, Denmark
*
* Project: Grundfos MPC DHCP / IP configuration
* Projectno.: 5072
* File: RTIPDHCPClient.cpp
*
*/

#include "RTIPDHCPClient.h"

#define TRACE(x)


/**
*
*/
RTIPDHCPClient::RTIPDHCPClient()
: 	mprtipconfif(0),
mcomdisabled(false),
mfaultduration(0)
{
#if (INCLUDE_PPP && INCLUDE_CHAP)
  mcallbacks.cb_chap_get_random_value_fnc = 0;
#endif
  mcallbacks.cb_wr_screen_string_fnc = 0;
  mcallbacks.cb_wr_interrupt_string_fnc = 0;
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
  mcallbacks.cb_raw_mode_in_char_fnc = 0;
  mcallbacks.cb_rs232_connection_lost_fnc = 0;
#endif
  mcallbacks.cb_error_report_fnc = 0;
#if (INCLUDE_TELNET_SRV)
  mcallbacks.cb_telnet_init_fnc = 0;
  mcallbacks.cb_telnet_inchar_fnc = 0;
#endif
#if (INCLUDE_FTP_SRV)
  mcallbacks.cb_ftp_check_user_name_fnc = 0;
  mcallbacks.cb_ftp_check_password_fnc = 0;
#endif
#if (INCLUDE_DHCP_CLI)
  mcallbacks.cb_dhcp_new_ip_fnc = 0;
  mcallbacks.cb_dhcp_no_ip_fnc = 0;
#endif

  mcallbacks.cb_dhcp_new_ip_fnc = RTIPDHCPClient::dhcpNewIPCallback;

  xn_register_callbacks(&mcallbacks);
}

RTIPDHCPClient::~RTIPDHCPClient()
{
}

DHCPClient* RTIPDHCPClient::getInstance()
{
  if (DHCPClient::mpinstance == 0)
    DHCPClient::mpinstance = new RTIPDHCPClient;

  return DHCPClient::mpinstance;
}


void RTIPDHCPClient::renew()
{
  msemaphore.enter();
  mdorenew = true;
  msemaphore.exit();
}

/**The IP address will be 0.0.0.0 after performing a release.
*
* \note If communication is disabled, when calling release,
* 		 a release message will not be sent to the DHCP server.
*/
void RTIPDHCPClient::release()
{
  msemaphore.enter();

  IPAddress nulladdr(0,0,0,0);

  if (!mcomdisabled && mlinkstatus)
  {
    if (xn_dhcp_release(&msession))
    {
      handleError(0);
    }
  }

  mgotlease = false;
  mleasetime = 0;
  mleasegone = 0;
  mrenewtime = 0;
  mrebindtime = 0;



  mipaddress = nulladdr;
  msubnetmask = nulladdr;
  mdefaultgateway = nulladdr;
  mprimarydns = nulladdr;
  msecondarydns = nulladdr;

  gotNewLease();

  msemaphore.exit();
}

void RTIPDHCPClient::enableCommunication()
{
  msemaphore.enter();
  mcomdisabled = false;
  msemaphore.exit();
}

void RTIPDHCPClient::disableCommunication()
{
  msemaphore.enter();
  mcomdisabled = true;
  msemaphore.exit();
}


/**This method is called when the RTIP DHCP client receives an IP
* configuration from the DHCP server.
*
* It sets IP address, subnet mask and default gateway according to
* offer from DHCP server.
*/
void RTIPDHCPClient::dhcpNewIPCallback(int ifaceno)
{
  IFACE_INFO ifinfo;

  RTIPDHCPClient *me =
    static_cast<RTIPDHCPClient*>(RTIPDHCPClient::getInstance());

  if ( me->minterfaceno != ifaceno)
  {
    return;
  }
  if (!me->menabled)
  {
    me->mpipstackconfigurator->refreshIPConfig();	//Due to timing, rtip might set the ip address, when we dont want it
    //this will reset to previous configuration
    return;
  }

  xn_interface_info(me->minterfaceno, &ifinfo);

  IPAddress ipaddress(ifinfo.my_ip_address);
  me->mipaddress = ipaddress;

  IPAddress subnetmask(ifinfo.ip_mask);
  me->msubnetmask = subnetmask;

  //Default gateway from dhcp server?:
  if(me->msession.params.router_option && me->msession.params.router_option_len > 0)
  {
    IPAddress defaultgateway(reinterpret_cast<unsigned char*>(me->msession.params.router_option));	//Use first gateway
    me->mdefaultgateway = defaultgateway;
  }
  else
  {
    me->mdefaultgateway = IPAddress(0,0,0,0);
  }

  if(me->msession.params.dns_server && me->msession.params.dns_server_len > 0)
  {
    IPAddress primarydns(reinterpret_cast<unsigned char*>(me->msession.params.dns_server));
    me->mprimarydns = primarydns;

    if(me->msession.params.dns_server_len > 4)
    {
      IPAddress secondarydns(reinterpret_cast<unsigned char*>(me->msession.params.dns_server)+4);
      me->msecondarydns = secondarydns;
    }
    else
    {
      me->msecondarydns = IPAddress(0,0,0,0);
    }
  }
  else
  {
    me->mprimarydns = IPAddress(0,0,0,0);
    me->msecondarydns = IPAddress(0,0,0,0);
  }


  me->mgotlease = true;
  me->gotNewLease();
}

/**The run method contains most of the functionality of the DHCP client.
* Renew and rebind of the lease will be done when certain amounts of the lease time
* has passed. However, if communication is disabled, when renew or rebind
* should be done, it will be postponed to a later time (half the remaining lease time).
* If the leasetime expires before the DHCP client was able to renew the lease,
* the IP address will be released.
*/
void RTIPDHCPClient::run()
{
  while(1)
  {
    Thread::delay(DHCPLOOPTIME_MS);

    if ( mprtipconfif)
    {
      minterfaceno = mprtipconfif->getInterfaceNo();
      mlinkstatus = mprtipconfif->getLinkStatus();
    }

    if (menabled)
    {
      TRACE(OS_DebugSendString("DHCP, enabled:\n"););

      msemaphore.enter();							//Lock resource or wait

      if (mgotlease)								//Extend lease time here
      {
        TRACE(OS_DebugSendString(" Got lease.\n"););
        mleasegone++;

        if (mleasegone == mleasetime)			//Lease expired!
        {
          release();
          handleError("Lease ran out, now released.");
        }
        else if (mleasegone == mrebindtime		//We need to rebind now
          || mdorenew)
        {
          mdorenew = false;

          initConfig();

          if(mcomdisabled || xn_dhcp(minterfaceno, &msession, &mconfig))
          {
            TRACE(OS_DebugSendString("  Failed rebinding. \n"););

            if (mleasetime-mrebindtime  >= 120)				//must have at least 60 sec. before rebind
            {
              mrenewtime += (mleasetime-mrebindtime)>>1;	//Try again after 1/2 interval
            }
            else											//Too little time, give up
            {
              handleError("Giving up trying to rebind lease.");
            }
          }
          else
          {
            TRACE(OS_DebugSendString("  Got rebinded. \n"););

            handleServerParams();
          }
        }
        else if (mleasegone == mrenewtime)	//We need to renew now
        {
          if (mcomdisabled || xn_extend_dhcp_lease(&msession, mleasetime))
          {
            TRACE(OS_DebugSendString("  Failed renewing. \n"););

            if (mleasetime-mrenewtime  >= 120)				//must have at least 60 sec. before renew
            {
              mrenewtime += (mleasetime-mrenewtime)>>1;	//Try again after 1/2 interval
            }
            else											//Too little time, give up
            {
              handleError("Giving up trying to renew lease.");
            }
          }
          else
          {
            TRACE(OS_DebugSendString("  Renewed lease.\n"););

            handleServerParams();
          }
        }

        if (mfaultduration)
        {
          mfaultduration = 0;
          if (mpipstackconfigurator)
          {
            IPConfigErrorHandler::getInstance()->setErrorNo(0);		//Delete error
          }
        }

        if (!mlinkstatus)							//Restart if network disconnected
        {
          release();
        }
      }
      else
      {
        if (mfaultduration < DHCPMINFAULTDUR)
        {
          mfaultduration++;
          if (mfaultduration == DHCPMINFAULTDUR)
          {
            if (mpipstackconfigurator)
            {
              handleError("Missing DHCP server");
            }
          }
        }

        if (!mcomdisabled && mlinkstatus)
        {
          TRACE(OS_DebugSendString(" Trying to get lease.\n"););

          initConfig();

          if(xn_dhcp(minterfaceno, &msession, &mconfig))			//This takes about 10 seconds, if there is no DHCP server available
          {
            TRACE(OS_DebugSendString("  Failed getting lease. \n"););
          }
          else
          {
            TRACE(OS_DebugSendString("  Got new lease. \n"););

            handleServerParams();
          }
        }
      }

      msemaphore.exit();
    }
    else
    {
      TRACE(OS_DebugSendString(" Disabled.\n"););

      if (mfaultduration)												//Delete error if DHCP client is disabled
      {
        mfaultduration = 0;
        if (mpipstackconfigurator)
        {
          IPConfigErrorHandler::getInstance()->setErrorNo(0);		//Delete error
        }
      }

    }
  }
}

void RTIPDHCPClient::setRTIPConfigurationInterfacePtr(RTIPConfigurationInterface* p)
{
  mprtipconfif = p;
}

/**This method extracts renew and rebind times from the offer from the DHCP
* server.
*/
void RTIPDHCPClient::handleServerParams()
{
  unsigned char buf[4];

  mgotlease = true;
  mleasetime = msession.lease_time;
  mleasegone = 0;


  if (xn_get_dhcp_op(&msession, RENEWAL_TIME, 4, buf))
  {
    TRACE(OS_DebugSendString(" ERROR getting renewal time, using default. \n"););
    mrenewtime = mleasetime>>1;			//renew = lease * 0.5
  }
  else
  {
    TRACE(OS_DebugSendString(" Got renewal time:"););

    ((char*)(&mrenewtime))[0] = buf[3];			//Convert to little-endian
    ((char*)(&mrenewtime))[1] = buf[2];			//(ntohl not available)
    ((char*)(&mrenewtime))[2] = buf[1];
    ((char*)(&mrenewtime))[3] = buf[0];
  }

  if (xn_get_dhcp_op(&msession, REBINDING_TIME, 4, buf))
  {
    TRACE(OS_DebugSendString(" ERROR getting rebinding time, using default. \n"););
    mrebindtime = (mleasetime*7)>>3;			//rebind = lease * 0.875
  }
  else
  {
    TRACE(OS_DebugSendString(" Got rebinding time:"););

    ((char*)(&mrebindtime))[0] = buf[3];
    ((char*)(&mrebindtime))[1] = buf[2];
    ((char*)(&mrebindtime))[2] = buf[1];
    ((char*)(&mrebindtime))[3] = buf[0];
  }

}

/**Initialize configuration data sent to DHCP server, so that hostname
* is sent to DHCP server and renew and rebind times are requested from DHCP
* server.
*/
void RTIPDHCPClient::initConfig()
{
  if (mhostname.length() > DHCPMAXHOSTNAMELENGTH)
  {
    mhostname = mhostname.substr(0,DHCPMAXHOSTNAMELENGTH);
  }

  strcpy(mhostnameoption, mhostname.c_str());

  mcparam[0].id = HOST_NAME_OP;					//Setup extra parameter (hostname)
  mcparam[0].len = mhostname.length();
  mcparam[0].cpdata = mhostnameoption;

  mcoption[0].id = RENEWAL_TIME;
  mcoption[0].prio = 50;
  mcoption[1].id = REBINDING_TIME;
  mcoption[1].prio = 50;
  mcoption[2].id = SUBNET_MASK;
  mcoption[2].prio = 60;
  mcoption[3].id = ROUTER_OPTION;
  mcoption[3].prio = 60;
  mcoption[4].id = DNS_OP;
  mcoption[4].prio = 60;

  xn_init_dhcp_conf(&mconfig);

  xn_set_dhcp_conf_op(&mconfig, mcparam);

  mconfig.lease_time = 86400; // Set desired ip lease time: 24h

  mconfig.plist = mcoption;
  mconfig.plist_entries = 5;
}

/**
* \param es Pointer to error description. If this is 0, get error string from RTIP
*/
void RTIPDHCPClient::handleError(char* es)
{
  string s;

  if (es)
  {
    s=es;
  }
  else
  {
    int rtiperror = xn_getlasterror();
    s = xn_geterror_string(rtiperror);
  }

  IPConfigErrorHandler::getInstance()->setErrorNo(IPCDHCPERROR);
  IPConfigErrorHandler::getInstance()->setDetailedErrorText(s);
}

