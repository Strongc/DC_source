/** \file MPCIPConfig.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: MPCIPConfig.h
 *
 * $Log: MPCIPConfig.h,v $
 * Revision 1.1.2.2  2005/06/27 07:59:38  jla
 * Added documentation
 *
 */

#ifndef _MPCIPCONFIG_H_
#define _MPCIPCONFIG_H_

#include "IPConfiguration.h"
#include "IPStackConfigurator.h"
#include "RTIPConfigurationInterface.h"
#include "RTIPDHCPClient.h"
#include "IPConfigByWeb.h"
#include "PasswordSetting.h"
#include "MPCWebServer.h"
#include "MPCNetworkDaemon.h"
#include "MPCVNCServer.h"

/**\brief Creates objects and initializes the IP configuration software.
 *
 * Call the three static methods from main() to get the IP configuration
 * software up and running.
 */
class MPCIPConfig
{
public:
	static void createObjects();		//!<Creates all objects used by IP configuration software.
	static void init();					//!<Initialize IP configuration software.
	static void startThreads();			//!<Starts all threads used by IP configuration software.

	virtual ~MPCIPConfig();

private:
	MPCIPConfig();
};

#endif //_MPCIPCONFIG_H_
