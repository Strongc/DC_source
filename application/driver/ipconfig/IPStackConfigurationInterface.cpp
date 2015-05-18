/** \file IPStackConfigurationInterface.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPStackConfigurationInterface.cpp
 *
 * $Log: IPStackConfigurationInterface.cpp,v $
 * Revision 1.1.2.3  2005/05/20 14:30:17  jla
 * Added interface number
 *
 * Revision 1.1.2.2  2005/05/09 09:20:31  jla
 * Changed to a singleton,
 *
 * Revision 1.1.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#include "IPStackConfigurationInterface.h"

IPStackConfigurationInterface* IPStackConfigurationInterface::mpinstance = 0;

IPStackConfigurationInterface::IPStackConfigurationInterface()
 : minitialized(false),
   mopen(false)
{
}

IPStackConfigurationInterface::~IPStackConfigurationInterface()
{
}

/**
 * \return The interface number.
 */
int IPStackConfigurationInterface::getInterfaceNo()
{
	return minterfaceno;
}

/**
 * \return True if IP stack has been initialized
 */
bool IPStackConfigurationInterface::isInitialized()
{
	return minitialized;
}
