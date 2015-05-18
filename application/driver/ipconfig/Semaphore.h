/** \file Semaphore.h
 * 
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 * 
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: Semaphore.h
 * 
 * $Log: Semaphore.h,v $
 * Revision 1.1.2.3  2005/05/27 13:23:52  jla
 * Finished documentation
 *
 * Revision 1.1.2.2  2005/05/12 13:56:58  jla
 * Fixed return types
 *
 * Revision 1.1.2.1  2005/05/12 07:23:02  jla
 * Added semaphore class for resource protection
 *
 */


#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <RTOS.H>

/**\brief Encapsulates embOS semaphore
 * 
 * Used for resourceprotection
 */
class Semaphore
{
public:
	Semaphore();
	virtual ~Semaphore();
	
	void enter();				//!<Lock resource
	void exit();				//!<Unlock resource
	
private:
	OS_RSEMA msemaphore;
};

#endif //_SEMAPHORE_H_
