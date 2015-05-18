/** \file MPCNetworkDaemon_C.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: MPCNetworkDaemon_C.h
 *
 * $Log: MPCNetworkDaemon_C.h,v $
 * Revision 1.1.2.2  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.1  2005/05/20 14:25:35  jla
 * Added interface to interrupt handler
 *
 * \brief C interface to the MPCNetworkDaemon.
 * 
 * The only thing defined here is a function to call from the ethernet interrupthandler.
 */

#ifndef _MPCNETWORKDAEMON_C_H_
#define _MPCNETWORKDAEMON_C_H_


int MPCNetworkDaemonInterrupt_C(void);		//!<Call this function from ethernet interrupthandler

#endif //_MPCNETWORKDAEMON_H_
