/** \file Semaphore.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: Semaphore.cpp
 *
 * $Log: Semaphore.cpp,v $
 * Revision 1.1.2.3  2005/05/27 13:23:52  jla
 * Finished documentation
 *
 * Revision 1.1.2.2  2005/05/20 14:35:20  jla
 * Minor changes
 *
 * Revision 1.1.2.1  2005/05/12 07:23:02  jla
 * Added semaphore class for resource protection
 *
 */

#include "Semaphore.h"

Semaphore::Semaphore()
{
	OS_CREATERSEMA(&msemaphore);
}

Semaphore::~Semaphore()
{
}

/**
 * Lock resource or wait for it to become free.
 */
void Semaphore::enter()
{
	OS_Use(&msemaphore);
}

/**
 */
void Semaphore::exit()
{
	OS_Unuse(&msemaphore);
}

