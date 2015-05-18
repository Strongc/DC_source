/** \file IPConfigDoc.h
 * 
 * \brief Documentation
 * 
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 * 
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPConfigDoc.h
 * 
 * $Log: IPConfigDoc.h,v $
 * Revision 1.1.2.5  2005/06/27 07:59:57  jla
 * Updated documentation
 *
 * Revision 1.1.2.4  2005/05/30 07:08:06  jla
 * Some cosmetics
 *
 * Revision 1.1.2.3  2005/05/30 06:37:37  jla
 * Changed version to V1.00.00
 *
 * Revision 1.1.2.2  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.1  2005/05/20 14:26:01  jla
 * A headerfile with documentation for Doxygen
 *
 */

#ifndef _IPCONFIGDOC_H_
#define _IPCONFIGDOC_H_

/**\mainpage
 * 
 * This is the documentation for IP software for the Grundfos MPC. 
 *
 * \author Jesper Larsen, 
 * 			IO Technologies A/S		\n
 * 			Gåseagervej 6			\n
 * 			DK-8250 Egå				\n
 * 			Phone: +45 87 43 80 74	\n
 * 			E-mail: jla@iotech.dk	\n
 * 
 * \version 1.01.00
 *
 *
 * The IP software consist of the following parts:
 * 
 *  - Central class containing IP configuration (IPConfiguration)
 * 
 *  - A class that handles use of static or dynamic IP address (IPStackConfigurator)
 * 
 *  - Abstract and concrete classes for IP stack configuration 
 * 	   (IPStackConfigurationInterface and RTIPConfigurationInterface)
 * 
 *  - Abstract and concrete classes for DHCP client (DHCPClient and RTIPDHCPClient)
 * 
 *  - Class for network configuration via webpage (IPConfigByWeb)
 *  
 *  - Wrapper class for webserver that also handles password for webpage access  
 *    (MPCWebServer)
 * 
 *  - Wrapper class for VNC server (MPCVNCServer)
 * 
 *  - A class that handles disabling and enabling of the network interface 
 *    (MPCNetworkDaemon)
 * 
 *  - A class that handles changing of password via a webpage (PasswordSetting)
 * 
 *  - A class that handles errors in the IP software (IPConfigErrorHandler)
 * 
 *  - And finally a utility class (IPUtils), observer pattern (ObserverIO and 
 *    SubjectIO), RTOS wrappers (Semaphore and Thread) and ip address class (IPAddress)
 * 
 * 
 * \section usage Using the IP software
 *
 * It is necessary to instatiate some object and set some pointers when booting.
 * This can be done in the following way (example):\n
 * 
	\code	
	
	void startIPSoftware()
	{
	   	string hostname("MPC");

		// ************* Setup pointers *******************
		IPStackConfigurator::getInstance()->
			setObjectPtrs(IPConfiguration::getInstance(), 
						  RTIPDHCPClient::getInstance(), 
						  RTIPConfigurationInterface::getInstance());
						  
		RTIPDHCPClient::getInstance()->
			setIPStackConfPtr(IPStackConfigurator::getInstance());
			
		static_cast<RTIPDHCPClient*>(RTIPDHCPClient::getInstance())->
			setRTIPConfigurationInterfacePtr(
				static_cast<RTIPConfigurationInterface*>(
					RTIPConfigurationInterface::getInstance()));
	
		IPConfigByWeb::getInstance()->
			setIPConfigurationPtr(IPConfiguration::getInstance());
		
		
		// ******* Initialize and start threads *************

		RTIPConfigurationInterface::getInstance()->init();
		RTIPConfigurationInterface::getInstance()->open();

		IPConfiguration::getInstance()->setNewIPConfig(false,
													  IPAddress(192,168,0,103),	
													  IPAddress(255,255,255,0),
													  IPAddress(192,168,1,20),
													  hostname);

		RTIPDHCPClient::getInstance()->start();
		IPConfigByWeb::getInstance()->start();
		MPCNetworkDaemon::getInstance()->start();

		MPCWebServer::getInstance()->enableAuthentication();
		MPCWebServer::getInstance()->start(iotechtest->mpipstackif->getInterfaceNo());

		MPCVNCServer::getInstance()->init(0,0);
		MPCVNCServer::getInstance()->start();
	}
	
	\endcode
 *
 * All this in contained by the MPCIPConfig class, so it is only necessary to 
 * call three methods:
 * 
   \code
   
   MPCIPConfig::createObjects();
   MPCIPConfig::init();
   MPCIPConfig::startThreads();
   
   \endcode
 * 
 * 
 * \section notice Notices
 * 
 * \subsection dhcp DHCP enabled when no DHCP server is available.
 * If the DHCP client is enabled, and no DHCP server is available, the IP address
 * will be set to 0.0.0.0. This is done by the RTIP DHCP software, and as the
 * IP address is invalid, it is not possible to access the MPC via the ethernet.
 * 
 * If the DHCP client has to be disabled (and an IP address entered manually), 
 * this must be done locally on the MPC, or through some other interface, e.g.
 * the GENI bus.
 * 
 * \subsection dhcplinking Redirecting browser when enabling DHCP client
 * When the DHCP client is enabled, the browser client cannot be directed
 * directly to the new IP address, as the MPC IP configuration software has no
 * way of knowing the IP address. In stead the browser client is redirected to
 * the hostname, if it has been supplied. 
 * 
 * However, for this to work, the hostname has to be linked with the IP address
 * given by the DHCP server. This is usually done by a DNS server.
 * 
 * \subsection extension Configuring IP software from other places
 * If you want to configure the IP address etc. from other places than the 
 * webpage, you should make a class that is a subclass of ObserverIO and
 * make that class observe IPConfiguration. You can then change the IP address,
 * subnet mask etc. in IPConfiguration. Just make sure that SubjectIO::notify is
 * called afterwards, so the IP configuration can be distributed in the IP 
 * software. 
 * 
 * To get started, you might want to take at look at the IPConfigByWeb class.
 * 
 * \subsection changedfiles Changed files
 * The following files have been changed or added during the development of 
 * the IP software:
 * - rtip\\config
 *   - HW_ETH.C: Added call to MPCNetworkDaemonInterrupt_C
 *   - XNCONF.H: Set CFG_SPAWN_WEB to 0
 * 
 * - rtip\\xnet
 *   - RTIPAPI.C: Added a poolregistered variable, to make sure that memory 
 *                pool is only registered once
 *   - WEBCONF.H: Changed INCLUDE_WEB_PUT to 0, as authentication is not 
 *                supported on PUT commands
 *   - RTIPAPI.H: Added #ifndef / #define / #endif structure to the defines of
 *                some error numbers, because they conflicted with defines in 
 *                errorno.h
 *  
 * - rtip\\webpage
 *   - HTTPGETF.C: Added CGI functions for IP configuration page and password 
 *                 setting page.
 *   - HTTPGETF.H: Added prototypes for CGI functions.
 *   - HTTPPSTF.C: Added post functions for IP configuration page and password 
 *                 setting page. 
 *   - HTTPPSTF.H: Added prototypes for post functions.
 *   - index.html: Added link to IP configuration page and changed static IP 
 *                 address for java applet to CGI function.
 *   - ipconfig.html: Webpage for IP configuration.
 *   - make_c_files.bat: Added ipconfig.html and pwsetting.html
 *   - pwsetting.html: Webpage for password setting.
 *   - webpages.c: Added ipconfig.html and pwsetting.html
 * 
 * - emWin\\GUI\\VNC
 *   - GUI_VNC_X_StartServer.c: This file is not needed anymore, as the contents
 *                              have been copied to MPCVNCServer.cpp.
 * 
 * - driver\\ipconfig
 *   - This directory contains the IP software.
 * 
 * \section design Design / UML diagrams
 * 
 * \subsection classes Class diagrams
 *
 * Class diagram showing the most important classes in the IP software:
 * \n
 * \n
 * \n
 * <IMG SRC="graphics/class_diagram.gif" ALT="Class diagram">
 * 
 * \subsection sequence Sequence diagrams
 * \n
 * \n
 * \n
 * <IMG SRC="graphics/Seq_enable_dhcp.gif" ALT="Sequence diagram">
 * \n
 * \n
 * <IMG SRC="graphics/Seq_disable_dhcp.gif" ALT="Sequence diagram">
 * 
 */



#endif //_IPCONFIGDOC_H_
