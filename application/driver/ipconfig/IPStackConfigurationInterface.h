/** \file IPStackConfigurationInterface.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPStackConfigurationInterface.h
 *
 * $Log: IPStackConfigurationInterface.h,v $
 * Revision 1.1.2.4  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.3  2005/05/20 14:30:17  jla
 * Added interface number
 *
 * Revision 1.1.2.2  2005/05/09 09:20:21  jla
 * Changed to a singleton,
 *
 * Revision 1.1.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 *
 */

#ifndef _IPSTACKCONFIGURATOIONINTERFACE_H_
#define _IPSTACKCONFIGURATOIONINTERFACE_H_

#include "IPAddress.h"

/**\brief Abstract baseclass for implementing an IP stack configuration class.
 *
 * Inherit from this class to implement a class for configuring a
 * specific IP stack.
 *
 * This class is a singleton, subclasses need to implement a getInstance method.
 *
 */
class IPStackConfigurationInterface
{
public:
	virtual ~IPStackConfigurationInterface();

    virtual bool init()=0;                      //!<Initializes ip stack and network interface
    virtual bool open()=0;                      //!<Opens the network interface
    virtual bool close()=0;                     //!<Closes the network interface
    virtual bool exit()=0;						//!<"Un-init" method

	virtual bool getLinkStatus()=0;				//!<Get current link status

	virtual void setIPAddress(IPAddress )=0;		//!<Sets the IP address
	virtual void setSubnetMask(IPAddress )=0;		//!<Sets the submetmask
	virtual void setDefaultGateway(IPAddress )=0;	//!<Sets the default gateway
  virtual void setDNSServers(IPAddress primaryIpadr, IPAddress secondaryIpadr) =0;	//!<Sets DNS servers

	int getInterfaceNo();						//!<Get network interface number

	bool isInitialized();						//!<Has IP stack been initialized?

protected:
	IPStackConfigurationInterface();
    bool minitialized;							//!<True when ip stack is initialized.
    bool mopen;									//!<True when ip stack is open.

	int minterfaceno;							//!<Identifies the network interface

    static IPStackConfigurationInterface* mpinstance;
};

#endif //_IPSTACKCONFIGURATOR_H_
