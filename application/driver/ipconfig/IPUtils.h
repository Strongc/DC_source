/** \file IPUtils.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPUtils.h
 *
 * $Log: IPUtils.h,v $
 * Revision 1.1.2.4  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.3  2005/05/26 09:26:50  jla
 * All methods static. Added convertPostValue method
 *
 * Revision 1.1.2.2  2005/05/12 14:06:36  jla
 * *** empty log message ***
 *
 * Revision 1.1.2.1  2005/05/12 12:17:32  jla
 * *** empty log message ***
 *
 */

#ifndef _IPUTILS_H_
#define _IPUTILS_H_

#include "IPAddress.h"


/**\brief Some utilities implemented for RTIP
 *
 */
class IPUtils
{
public:
	static bool ping(IPAddress);			//!<Ping an IP address
	static bool delFromArp(IPAddress);		//!<Delete an entry in the ARP cache
	static void convertPostValue(string&);	//!<Converts HTML post form data
	static int getLastError();				//!<Get last errorcode from RTIP
  static bool isHostnameValid(string&); //return true if hostname is valid
};

#endif //_IPUTILS_H_
