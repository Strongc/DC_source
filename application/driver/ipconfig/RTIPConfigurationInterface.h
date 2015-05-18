/** \file RTIPConfigurationInterface.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: RTIPConfigurationInterface.h
 *
 * $Log: RTIPConfigurationInterface.h,v $
 * Revision 1.2.2.6  2005/05/27 13:23:52  jla
 * Finished documentation
 *
 * Revision 1.2.2.5  2005/05/20 14:33:53  jla
 * Added errorhandling and moved interfaceno to IPStackConfigurationInterface
 *
 * Revision 1.2.2.4  2005/05/12 07:26:26  jla
 * Added headerfile for ethernet hardwareconfig
 *
 * Revision 1.2.2.3  2005/05/09 13:51:38  jla
 * Added a todo
 *
 * Revision 1.2.2.2  2005/05/09 09:20:21  jla
 * Changed to a singleton,
 *
 * Revision 1.2.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#ifndef _RTIPCONFIGURATIONINTERFACE_H_
#define _RTIPCONFIGURATIONINTERFACE_H_

#include <iostream>
using namespace std;

#include "IPStackConfigurationInterface.h"

/**\brief Handles configuration of the RTIP IP stack.
 *
 */
class RTIPConfigurationInterface : public IPStackConfigurationInterface
{
public:
	virtual ~RTIPConfigurationInterface();

	static IPStackConfigurationInterface* getInstance();	//!<Get pointer to singleton instance

    virtual bool init();						//!<Initialize RTIP and ethernet hardware
    virtual bool open();						//!<Open RTIP interface
    virtual bool close();						//!<Close RTIP interface
    virtual bool exit();						//!<Shutdown the RTIP stack

	virtual bool getLinkStatus();				//!<Get current link status

    virtual void setIPAddress(IPAddress);		//!<Set IP address
    virtual void setSubnetMask(IPAddress);		//!<Set subnet mask
    virtual void setDefaultGateway(IPAddress);	//!<Set default gateway
    virtual void setDNSServers(IPAddress primaryIpadr, IPAddress secondaryIpadr);	//!<Set DNS servers

protected:
	RTIPConfigurationInterface();

private:
	void handleError(char *);						//!<Handles RTIP errors

	IPAddress mipaddress;
	IPAddress msubnetmask;
	IPAddress mdefaultgateway;
  IPAddress mprimarydns;
  IPAddress msecondarydns;
};

#endif //_RTIPCONFIGURATIONINTERFACE_H_
