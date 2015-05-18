/** \file RTIPDHCPClient.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: RTIPDHCPClient.h
 *
*/

#ifndef _RTIPDHCPCLIENT_H_
#define _RTIPDHCPCLIENT_H_


#include "DHCPCLient.h"
#include "RTIPConfigurationInterface.h"
#include "Semaphore.h"
#include "IPConfigErrorHandler.h"

#include <rtipapi.h>
#include <socket.h>

/**\brief DHCP Client for RTIP.
 *
 * This class contains the implementation of a DHCP client for the
 * RTIP IP stack. It is a singleton class.
 *
 * A hostname may be specified, this will be sent to the DHCP server.
 * The DHCP server may then send the hostname to a DNS server, thereby
 * making it possible to access the host that runs the DHCP client via it's hostname.
 */
class RTIPDHCPClient : public DHCPClient
{
public:
	virtual ~RTIPDHCPClient();

	static DHCPClient* getInstance();		//!<Get a pointer to the singleton instance

	virtual void run();
	virtual void renew();
	virtual void release();					//!<RTIP implementation of release method.

	void disableCommunication();			//!<Disable network communication with DHCP server
	void enableCommunication();				//!<Enable network communication with DHCP server

	void setRTIPConfigurationInterfacePtr(RTIPConfigurationInterface*);	//!<Pointer needed to get interface number

	static void dhcpNewIPCallback(int);		//!<Callback for RTIP

protected:
	RTIPDHCPClient();

private:
	void handleServerParams();				//!<Handles parameters returned by DHCP server
	void initConfig();						//!<Initializes configuration structure sent to DHCP server
	void handleError(char *);				//!<Handles errors from RTIP

	int minterfaceno;						//!<Number of network interface that the DHCP client operates on
	struct DHCP_session msession;			//!<RTIP data for this DHCP session
	struct DHCP_conf mconfig;				//!>RTIP configuration to be sent to DHCP server
	struct DHCP_cparam mcparam[1];				//!<Extra parameters for configuration
	struct DHCP_param mcoption[5];			//!<Custom option for reqesting renew and rebind times

	char mhostnameoption[DHCPMAXHOSTNAMELENGTH+3];

	bool mcomdisabled;						//!<True when communication/network is disabled
	bool mlinkstatus;                       //!<Network connection status
	int mfaultduration;						//!<Counts time for fault status (no contact to DHCP server)

	RTIPConfigurationInterface *mprtipconfif;	//!<Pointer to RTIPConfigurationInterface  object
	Semaphore msemaphore;					//!<Protects the access to the RTIP functions

	RTIP_CALLBACKS mcallbacks;				//!<Structure containing callbacks for RTIP (Only used for DHCP callback)
};

#endif //_RTIPDHCPCLIENT_H_
