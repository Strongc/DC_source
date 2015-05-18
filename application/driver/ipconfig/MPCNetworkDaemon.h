/** \file MPCNetworkDaemon.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: MPCNetworkDaemon.h
 *
 * $Log: MPCNetworkDaemon.h,v $
 * Revision 1.2.2.4  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.2.2.3  2005/05/26 09:27:31  jla
 * Implemented the class
 *
 * Revision 1.2.2.2  2005/05/09 13:51:54  jla
 * Added som skeletoncode
 *
 * Revision 1.2.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#ifndef _MPCNETWORKDAEMON_H_
#define _MPCNETWORKDAEMON_H_

#include "Thread.h"

#define	MPCNDDEFAULTINTERRUPTLIMIT		500			//!<Default max. no. of interrupts per second
#define	MPCNDDEFAULTREENABLEDELAY		15			//!<Default delay before networkinterface is reenabled, in seconds
#define MPCNDFAULTCLEARTIME				30			//!<Time after reenabling network to clearing flag, in seconds

#define NETDAEMONTASKPRIO     130        //!<Priority of Network Daemon task
#define NETDAEMONTASKSTACKSIZE	(4*1024)			//!<Size of stack for Network Daemon task

/**\brief Handles disabling and enabling of the network interface.
 *
 * This class watches the interrupt handler for the network interface.
 * If the interrupt handler is called too many times per second, the interrupt
 * is simply disabled. The interrupt is automatically reenabled after a certain
 * time. The maximum allowable number of interrupts per second and reenabling
 * delay is runtime configurable.
 *
 */
class MPCNetworkDaemon : public Thread
{
public:
	virtual ~MPCNetworkDaemon();

	static MPCNetworkDaemon* getInstance();				//!<Get pointer to singleton instance.

	void setNoofInterruptsLimit(unsigned int);			//!<Set max. no. of interrupts
	unsigned int getNoofInterruptsLimit();				//!<Get max. no. of interrupts

	void setReenableDelay(unsigned int);				//!<Set delay for reenabling
	unsigned int getReenableDelay();					//!<Get delay for reenabling

  void resetInterruptCounter();        //!< Reset interruptcounter, used for when resuming task

	void enableNetwork();								//!<Manually enable the network interface
	void disableNetwork();								//!<Manually disable the network interface

	static bool interrupt();							//!<To be called from ethernet IRQ handler

	void start();										//!<Start the thread.

protected:
	MPCNetworkDaemon();
	virtual void run();									//!<Contains most of the functionality

private:
	volatile unsigned int minterruptcounter;			//!<Counts number of interrupts
	unsigned int minterruptlimit;						//!<Max. no. of interrupts per second
	unsigned int mreenabledelay;						//!<Time before automatically reenabling the N.I.
	unsigned int mtimetoreenable;						//!<Countdown to reenable
	bool mdosdetected;
	bool mnidisabled;									//!<True when Network Interface is disabled
	int mfaultlessduration;								//!<Counter for time after reenabling network, used to clear fault flag

	static MPCNetworkDaemon* mpinstance;

	void enableNetworkInterface();						//!<Enable the IRQ for the network interface
	void disableNetworkInterface();						//!<Disable the IRQ for the network interface
};

#endif //_MPCNETWORKDAEMON_H_
