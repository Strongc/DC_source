/** \file RTIPConfigurationInterface.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: RTIPConfigurationInterface.cpp
 *
 * $Log: RTIPConfigurationInterface.cpp,v $
 * Revision 1.2.2.7  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.2.2.6  2005/05/20 14:33:53  jla
 * Added errorhandling and moved interfaceno to IPStackConfigurationInterface
 *
 * Revision 1.2.2.5  2005/05/12 14:07:06  jla
 * todo
 *
 * Revision 1.2.2.4  2005/05/09 13:15:35  jla
 * Cleaned up
 *
 * Revision 1.2.2.3  2005/05/09 09:44:56  jla
 * Now using 4-byte arrays for local variables with ip address
 *
 * Revision 1.2.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#include "RTIPConfigurationInterface.h"

#include "Thread.h"
#include "IPConfigErrorHandler.h"


extern "C"
{
	#include <smc91c9x.h>
}

#include <rtipapi.h>
#include <socket.h>
#include <Ethernet_hwc.h>

#include "GPIO.h"

RTIPConfigurationInterface::RTIPConfigurationInterface()
{
}

RTIPConfigurationInterface::~RTIPConfigurationInterface()
{
}

IPStackConfigurationInterface* RTIPConfigurationInterface::getInstance()
{
	if (mpinstance == 0)
		mpinstance = new RTIPConfigurationInterface;

	return mpinstance;
}

/**
 * \return True if RTIP and ethernet hardware was initialized successfully.
 */
bool RTIPConfigurationInterface::init()
{
	bool retval=false;

	HW_EthernetInit(); 		// Init the hardware for the ethernet

	/*----------------- NOW WAIT FOR AT LEAST 50mS -------------------*/
	Thread::delay(100);
	/*--------------------------- FINISH WAITING ----------------------*/

	if (minitialized)
	{
		if (xn_rtip_restart() == 0)
		{
			retval = true;
		}
	}
	else
	{
		if (xn_rtip_init() == 0)
		{
			retval = true;
			minitialized = true;
	    }
	}

    if (!retval)
    {
		handleError(0);
    }

    return retval;
}

/**
 * This method also enables the ethernet HW interrupt.
 *
 * \return True if the interface was opened successfully.
 */
bool RTIPConfigurationInterface::open()
{
	bool retval = false;

	if (minitialized || !mopen)
	{
		if (xn_bind_smc91c9x(0) == 0)			//append interface to device table
		{
			minterfaceno =
				xn_interface_open_config(SMC91C9X_DEVICE, 0, (IOADDRESS)ETHERNET_BASE, 5, 0);

			if (minterfaceno != -1)
			{
	    		HW_EnEthernetIRQ();
	    		retval = true;
	    		mopen = true;
	    	}
		}
	}

	if (!retval)
	{

		if (!minitialized)
		{
			string es = "RTIP interface was not initialized before opening.";

			IPConfigErrorHandler::getInstance()->setErrorNo(IPCRUNTIMEERROR);
			IPConfigErrorHandler::getInstance()->setDetailedErrorText(es);
		}
		else if (mopen)
		{
			string es = "RTIP interface was already open before opening.";

			IPConfigErrorHandler::getInstance()->setErrorNo(IPCRUNTIMEERROR);
			IPConfigErrorHandler::getInstance()->setDetailedErrorText(es);
		}
		else		//RTIP error
		{
			handleError(0);
		}
	}

	return retval;
}

/**
 * This method also disables the ethernet HW interrupt.
 *
 * \return True if the interface was closed successfully.
 */
bool RTIPConfigurationInterface::close()
{
	bool retval = false;

	if (mopen)
	{
		if (xn_interface_close(minterfaceno) == 0)
		{
			HW_DisEthernetIRQ();
			mopen = false;
			retval = true;
		}
	}

	if (!retval)
	{
		if (!mopen)
		{
			string es = "RTIP interface was not open before closing";

			IPConfigErrorHandler::getInstance()->setErrorNo(IPCRUNTIMEERROR);
			IPConfigErrorHandler::getInstance()->setDetailedErrorText(es);
		}
		else		//RTIP error
		{
			handleError(0);
		}
	}

	return retval;
}

/**
 * \return True if RTIP was shutdown successfully.
 */
bool RTIPConfigurationInterface::exit()
{
	bool retval = false;

	if (xn_rtip_exit() == 0)
	{
		retval = true;
	}

	if (!retval)
	{
		handleError(0);
	}

	return retval;
}

/**Get status for connected to network or not
 *
 * \return True if connected to network, false if not
 */
bool RTIPConfigurationInterface::getLinkStatus()
{
    OS_DI();

    bool rc = (smc91c9x_getLinkStatus() == 1);
    OS_RestoreI();

    return rc;
	
}


/**
 * \param ipadr New IP address.
 *
 * \note This method will copy both IP address and subnetmask to RTIP.
 */
void RTIPConfigurationInterface::setIPAddress(IPAddress ipadr)
{
	unsigned char ipaddress[4], subnetmask[4];
	mipaddress = ipadr;

	mipaddress.getIPAddress(ipaddress);
	msubnetmask.getIPAddress(subnetmask);

	if (xn_set_ip(minterfaceno, ipaddress, subnetmask) == -1)
	{
		handleError(0);
	}
}

/**
 * \param mask New subnet mask.
 *
 * \note This method will copy both IP address and subnetmask to RTIP.
 */
void RTIPConfigurationInterface::setSubnetMask(IPAddress mask)
{
	unsigned char ipaddress[4], subnetmask[4];
	msubnetmask = mask;

	mipaddress.getIPAddress(ipaddress);
	msubnetmask.getIPAddress(subnetmask);

	if (xn_set_ip(minterfaceno, ipaddress, subnetmask) == -1)
	{
		handleError(0);
	}
}

/**
 * Adds a default gateway to the route table.
 *
 * \param ipadr IP address of new default gateway.
 *
 */
void RTIPConfigurationInterface::setDefaultGateway(IPAddress ipadr)
{
	unsigned char ipadrbytes[4];

	//Edit route table.

    xn_rt_del((PFBYTE)RT_DEFAULT, ip_ffaddr);

	mdefaultgateway = IPAddress(0,0,0,0);

	ipadr.getIPAddress(ipadrbytes);
    if (xn_rt_add((PFBYTE)RT_DEFAULT, ip_ffaddr, ipadrbytes, RT_USEIFACEMETRIC, minterfaceno, RT_INF) == -1)
	{
		handleError(0);
	}
    else
	{
		mdefaultgateway = ipadr;
	}
}

/**
 * Adds a primary and a secondary DNS server.
 *
 * \param primaryIpadr IP address of new primary DNS.
 * \param secondaryIpadr IP address of new secondary DNS.
 *
 */
void RTIPConfigurationInterface::setDNSServers(IPAddress primaryIpadr, IPAddress secondaryIpadr)
{
  unsigned char ipadrbytes[4];

  mprimarydns = IPAddress(0,0,0,0);
  msecondarydns = IPAddress(0,0,0,0);

  // Clear list of current DNS servers
  xn_set_server_list(0, 0);

  primaryIpadr.getIPAddress(ipadrbytes);

  if (xn_add_dns_server((PFBYTE)ipadrbytes) == -1)
  {
    handleError(0);
  }
  else
  {
    mprimarydns = primaryIpadr;

    secondaryIpadr.getIPAddress(ipadrbytes);

    if (xn_add_dns_server((PFBYTE)ipadrbytes) == -1)
    {
      handleError(0);
    }
    else
    {
      msecondarydns = secondaryIpadr;
    }
  }

}

/**
 * \param es Pointer to error description. If this is 0, get error string from RTIP
 */
void RTIPConfigurationInterface::handleError(char* es)
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

	IPConfigErrorHandler::getInstance()->setErrorNo(IPCRTIPERROR);
	IPConfigErrorHandler::getInstance()->setDetailedErrorText(s);
}

