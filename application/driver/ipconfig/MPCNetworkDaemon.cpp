/** \file MPCNetworkDaemon.cpp
*
* Written by: Jesper Larsen, jla@iotech.dk
* Company: IO Technologies A/S, Egaa, Denmark
*
* Project: Grundfos MPC DHCP / IP configuration
* Projectno.: 5072
* File: MPCNetworkDaemon.cpp
*
* $Log: MPCNetworkDaemon.cpp,v $
* Revision 1.1.2.4  2005/06/15 09:23:28  jla
* Added checking whether object has been in interrupthandler, to avoid calling new from interrupthandler.
*
* Revision 1.1.2.3  2005/05/27 13:23:51  jla
* Finished documentation
*
* Revision 1.1.2.2  2005/05/26 09:27:31  jla
* Implemented the class
*
* Revision 1.1.2.1  2005/05/09 13:51:54  jla
* Added som skeletoncode
*
* Revision 1.1  2005/04/12 14:13:09  jla
* Scratch version
*
*/

#include "MPCNetworkDaemon.h"

#include "RTIPConfigurationInterface.h"
#include "RTIPDHCPClient.h"
#include "IPStackConfigurator.h"
#include "MPCWebServer.h"
#include "MPCVNCServer.h"
#include "IPConfigErrorHandler.h"

#include <rtipapi.h>
#include <Ethernet_hwc.h>

extern "C"
{
#include <bget.h>
}

extern "C"
{
#include "MPCNetworkDaemon_C.h"
}

#define TRACE(x) x

extern P_INIT_FNCS registered_fncs;				//!<We need to access an RTIP datastructure.

MPCNetworkDaemon* MPCNetworkDaemon::mpinstance = 0;

TRACE(char scount[64];)

MPCNetworkDaemon::MPCNetworkDaemon()
:	minterruptcounter(0),
minterruptlimit(MPCNDDEFAULTINTERRUPTLIMIT),
mreenabledelay(MPCNDDEFAULTREENABLEDELAY),
mnidisabled(false),
mfaultlessduration(MPCNDFAULTCLEARTIME)
{
}

MPCNetworkDaemon::~MPCNetworkDaemon()
{
}

MPCNetworkDaemon* MPCNetworkDaemon::getInstance()
{
  if (MPCNetworkDaemon::mpinstance == 0)
  {
    MPCNetworkDaemon::mpinstance = new MPCNetworkDaemon;
  }

  return MPCNetworkDaemon::mpinstance;
}

/**
* \param limit The new maximum limit of interrupts per second.
*/
void MPCNetworkDaemon::setNoofInterruptsLimit(unsigned int limit)
{
  minterruptlimit = limit;
}

/**
* \return The current maximum limit of interrupts per second.
*/
unsigned int MPCNetworkDaemon::getNoofInterruptsLimit()
{
  return minterruptlimit;
}

/**
* \param delay The new delay for reenabling the network interface.
*/
void MPCNetworkDaemon::setReenableDelay(unsigned int delay)
{
  mreenabledelay = delay;
}

/**
* \return The current delay for reenabling the network interface.
*/
unsigned int MPCNetworkDaemon::getReenableDelay()
{
  return mreenabledelay;
}

/**
* \param minterruptcounter is reset.
*/
void MPCNetworkDaemon::resetInterruptCounter()
{
  minterruptcounter = 0;
}

/**The task priority and stack size is defined in MPCNetworkDaemon.h
*/
void MPCNetworkDaemon::start()
{
  Thread::start(NETDAEMONTASKPRIO, "MPCNetworkDaemon task", NETDAEMONTASKSTACKSIZE);
}

/**
* Call this method to manually enable the network interface
*/
void MPCNetworkDaemon::enableNetwork()
{
  if (mnidisabled)
  {
    enableNetworkInterface();
  }
}

/**
* Call this method to manually disable the network interface
* \note The network interface will be automatically re-enabled after
*       the re-enable delay.
*/
void MPCNetworkDaemon::disableNetwork()
{
  if (!mnidisabled)
  {
    disableNetworkInterface();
  }
}


/**The run method loops once per second, and counts how many interrupts was
* generated the last second. If the number of interrupts is above the specified
* limit, the network interface is disabled. The network interface is enabled
* again after the specified method.
*
* The default interrupt limit and reenabling delay are defined in
* MPCNetworkDaemon.h, but they may also be set with the methods
* setNoofInterruptsLimit and setReenableDelay.
*
*/
void MPCNetworkDaemon::run()
{

  delay(5000);				//Allow network interface some time to start up
  minterruptcounter=0;
  int delay = 1000;

  while(1)
  {

    //TRACE(sprintf(scount, "Interrupts last second: %u.\n", intcount););
    //TRACE(OS_DebugSendString(scount););

    if(mdosdetected)
    {
/*
    disableNetworkInterface();
	if (mfaultlessduration == MPCNDFAULTCLEARTIME)
        {
          IPConfigErrorHandler::getInstance()->setErrorNo(IPCNETWORKLOAD);
          mfaultlessduration = 0;
        }
        */
        TRACE(sprintf(scount, "Too many network interrupts last second: %u.\n", minterruptcounter););
        TRACE(OS_DebugSendString(scount););
//        TRACE(OS_DebugSendString("NI disabled\n"););

	Thread::delay(100);
        minterruptcounter /= 3;
        if(minterruptcounter < 2)
        {
            mdosdetected = false;
	    minterruptcounter = 0;
            delay = 1000;
        }
	else
	{
	    delay = 200;
	}
	    
        if(!mnidisabled)    
	    HW_EnEthernetIRQ(); 
    }
    else
    {
        minterruptcounter=0;
    }

    if (mnidisabled)
    {
      --mtimetoreenable;
      if (mtimetoreenable == 0)
      {
        enableNetworkInterface();

        TRACE(OS_DebugSendString("NI enabled\n"););
      }
    }
    else							//Network interface enabled
    {
      if ( mfaultlessduration < MPCNDFAULTCLEARTIME)
      {
        ++mfaultlessduration;
        if (mfaultlessduration == MPCNDFAULTCLEARTIME)
        {
          IPConfigErrorHandler::getInstance()->setErrorNo(0);
        }
      }
      else
      {
        mfaultlessduration = MPCNDFAULTCLEARTIME;
      }
    }

    Thread::delay(delay);			//Delay 1 second
  }

}

/**After initializing and opening the IP stack, the IP configuration is refreshed
* (IP address etc.) and the DHCPClient, webserver and VNC server are restarted.
*/
void MPCNetworkDaemon::enableNetworkInterface()
{
  registered_fncs = 0;                    //Necessary due to bug in RTIP, that creates an endless loop
  //(registered_fncs is a global var. in RTIP)

  RTIPConfigurationInterface::getInstance()->exit();
  RTIPConfigurationInterface::getInstance()->init();
  RTIPConfigurationInterface::getInstance()->open();
  IPStackConfigurator::getInstance()->refreshIPConfig();
  static_cast<RTIPDHCPClient*>(RTIPDHCPClient::getInstance())->enableCommunication();

  MPCWebServer::getInstance()->start(
    RTIPConfigurationInterface::getInstance()->getInterfaceNo());
  MPCVNCServer::getInstance()->start();

  mnidisabled = false;
}

/**Before closing the IP stack and disabling the ethernet interrupt, the DHCP client
* webserver and VNC server are stopped.
*
* The IP stack is not shutdown completely until enableNetworkInterface is called,
* as this seemed to work better than shutting it down here.
*
* \note A call to xn_abort_tcp here might be useful.
*/
void MPCNetworkDaemon::disableNetworkInterface()
{
  registered_fncs = 0;                    //Necessary due to bug in RTIP, that creates an endless loop
  //(registered_fncs is a global var. in RTIP)

  RTIPConfigurationInterface::getInstance()->exit();

  MPCVNCServer::getInstance()->terminate();
  MPCWebServer::getInstance()->stop();
  static_cast<RTIPDHCPClient*>(RTIPDHCPClient::getInstance())->disableCommunication();
  delay(2000);							//Give RTIP a chance to finish transmissions
  RTIPConfigurationInterface::getInstance()->close();
  delay(2000);							//Give RTIP a chance to finish transmissions
  


  RTIPConfigurationInterface::getInstance()->exit();
  HW_DisEthernetIRQ();
    

  mnidisabled = true;
}

/**Called from MPCNetworkDaemonInterrupt_C. Increments interrupt counter and
* returns interrupt status.
*
* \return True if interrupt is enabled, false if not.
*/
bool MPCNetworkDaemon::interrupt()
{
  if (MPCNetworkDaemon::mpinstance		//Only call if already instantiated, to avoid calling new from interrupt
    && RTIPConfigurationInterface::getInstance()->isInitialized())	//Only call if IP stack (and NIC) has been initialized
  {
    MPCNetworkDaemon* me = MPCNetworkDaemon::getInstance();
    ++(me->minterruptcounter);
    
    if(me->minterruptcounter > me->minterruptlimit)
    {
      HW_DisEthernetIRQ();
      me->mtimetoreenable = me->mreenabledelay;
      me->mdosdetected = true;
    } 
    return !me->mnidisabled;
  }
  return true;
}


extern "C"
{

  /**Calling this function lets the MPCNetworkDaemon count the number of
  * interrupts per second. The interrupt handler should use the return value
  * to evaluate whether to process the interrupt or not
  *
  * \return 1 if interrupt is enabled, 0 if not.
  */
  int MPCNetworkDaemonInterrupt_C(void)
  {
    if (MPCNetworkDaemon::interrupt())
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }

}
