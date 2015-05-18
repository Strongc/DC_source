/** \file MPCIPConfig.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: MPCIPConfig.cpp
 *
 * $Log: MPCIPConfig.cpp,v $
 * Revision 1.1.2.2  2005/06/27 07:59:38  jla
 * Added documentation
 *
 */

#include "MPCIPConfig.h"

MPCIPConfig::MPCIPConfig()
{
}

MPCIPConfig::~MPCIPConfig()
{
}

/**Objects are created by calling all getInstance methods.
 */
void MPCIPConfig::createObjects()
{
	IPConfiguration::getInstance();
	IPStackConfigurator::getInstance();
	RTIPDHCPClient::getInstance();
	RTIPConfigurationInterface::getInstance();

	MPCWebServer::getInstance();
	MPCVNCServer::getInstance();
	MPCNetworkDaemon::getInstance();

	PasswordSetting::getInstance();
	IPConfigByWeb::getInstance();
}

/**Sets all class relations and initializes IP stack.
 */
void MPCIPConfig::init()
{

	IPStackConfigurator::getInstance()->setObjectPtrs(
		IPConfiguration::getInstance(),
		RTIPDHCPClient::getInstance(),
		RTIPConfigurationInterface::getInstance());

	RTIPDHCPClient::getInstance()->setIPStackConfPtr(
		IPStackConfigurator::getInstance());

	static_cast<RTIPDHCPClient*>(RTIPDHCPClient::getInstance())->
		setRTIPConfigurationInterfacePtr(
			static_cast<RTIPConfigurationInterface*>(RTIPConfigurationInterface::getInstance()));

	IPConfigByWeb::getInstance()->setIPConfigurationPtr(
		IPConfiguration::getInstance());


	//string hostname("MPC");

	RTIPConfigurationInterface::getInstance()->init();
   	RTIPConfigurationInterface::getInstance()->open();

//   	IPConfiguration::getInstance()->setNewIPConfig(false,
//												   IPAddress(192,168,0,102),
//												   IPAddress(255,255,255,0),
//												   IPAddress(192,168,0,1),
//												   hostname);
}

/**Threads in the following classes are started:
 * - RTIPDHCPClient
 * - IPConfigByWeb
 * - MPCNetworkDaemon
 * - MPCWebServer
 * - MPCVNCServer
 */
void MPCIPConfig::startThreads()
{
	RTIPDHCPClient::getInstance()->start();
	IPConfigByWeb::getInstance()->start();
	MPCNetworkDaemon::getInstance()->start();

	MPCWebServer::getInstance()->enableAuthentication();
	MPCWebServer::getInstance()->start(0);

	MPCVNCServer::getInstance()->init(0,0);
	MPCVNCServer::getInstance()->start();
}
