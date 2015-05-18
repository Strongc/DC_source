/** \file IPUtils.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPUtils.cpp
 *
 * $Log: IPUtils.cpp,v $
 * Revision 1.1.2.4  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.3  2005/05/26 09:26:50  jla
 * All methods static. Added convertPostValue method
 *
 * Revision 1.1.2.2  2005/05/20 14:32:41  jla
 * Minor changes
 *
 * Revision 1.1.2.1  2005/05/12 12:17:32  jla
 * *** empty log message ***
 *
 */
#include "IPUtils.h"
#include <rtipapi.h>
#include <stdlib.h>
#include <ctype.h>

/**The the host with the IP address specified is pinged with 32 bytes of data.
 * The timeout is 500 ms.
 *
 * \note The host may or may not reply to the ping request. (Ping may be disabled.)
 * \param pingaddr IP address of host to ping.
 * \return True if ping was successful
 */
bool IPUtils::ping(IPAddress pingaddr)
{
	ROUTE_INFO routeinfo;
	unsigned char addr[4];
	long elapsed;

	pingaddr.getIPAddress(addr);

  if (xn_ping(0xAA55, 32, 0, addr, 0, &routeinfo, 500, &elapsed) == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/**This method was implemented instead of a lookInArp method, as RTIP does not
 * facilitate looking in the ARP cache, only adding and deleting.
 *
 * \param testaddr IP address to delete from ARP cache
 * \return True if IP address was in the ARP cache
 */
bool IPUtils::delFromArp(IPAddress testaddr)
{
	unsigned char addr[4];

	testaddr.getIPAddress(addr);

	if (xn_arp_del(addr)==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**Special characters in form post data on webpages are represented by their
 * hex values and spaces are replaced by '+'. This method converts back to
 * the original values.
 *
 * An example: \n
 * The string "A + B" is sent like this to the webserver: "A+%2B+B".\n
 * This method make the conversion back to "A + B"
 *
 * \param valstring A reference to the string to be converted.
 */
void IPUtils::convertPostValue(string& valstring)
{
	int x;
	string svalue;
	char cvalue[2];

		//Replace '+' with ' ' (space):
	x = valstring.find("+");
	while (x<string::npos)
	{
		valstring.replace(x, 1, " ");
		x = valstring.find("+", x+1);
	}

		//Replace "%xx" with character with value xx
	x = valstring.find("%");
	while (x<string::npos)
	{
		char *end;

		svalue = valstring.substr(x+1, 2);
		cvalue[0] = strtol(svalue.c_str(), &end, 16);
		cvalue[1] = 0;

		valstring.erase(x, 3); 						//delete "%xx"
		valstring.insert(x, string(cvalue));        //insert char

		x = valstring.find("%", x + 1);
	}
}

/**
 *
 * \return The errorcode directly from RTIP xn_getlasterror
 */
int IPUtils::getLastError()
{
	return xn_getlasterror();
}

/**
 * 
 *   A hostname may only consist of:
 *   A to Z ; upper case characters
 *   a to z ; lower case characters
 *   0 to 9 ; numeric characters 0 to 9
 *   -      ; dash
 *
 *   The rules say:
 *   A host name (label) can start or end with a letter or a number 
 *   A host name (label) MUST NOT start or end with a '-' (dash) 
 *   A host name (label) MUST NOT consist of all numeric values 
 *   A host name (label) can be up to 63 characters 
 *
 * \return true if valid, false if not
 */
bool IPUtils::isHostnameValid(string& hostname)
{
  if (hostname.length() == 0)
    return true;
  
  if (hostname.length() > 63)
    return false;

  if (hostname.at(0) == '-' || hostname.at(hostname.length() - 1) == '-' )
    return false;

  bool allDigits = true;

  for (int i=0; i<hostname.length(); i++)
  {
    if (isalpha(hostname[i]) || hostname[i] == '-')
    {
      allDigits = false;
    }
    else if (! isdigit(hostname[i]))
    {
      return false;
    }
  }

  if (allDigits)
    return false;

  return true;
}
