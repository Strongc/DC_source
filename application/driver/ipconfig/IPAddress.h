/** \file IPAddress.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPAddress.h
 *
 * $Log: IPAddress.h,v $
 * Revision 1.4.2.6  2005/06/10 08:53:41  jla
 * Added getIPAddress method that returns a 32 bit integer
 *
 * Revision 1.4.2.5  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.4.2.4  2005/05/14 10:24:36  jla
 * Added == and != operators
 *
 * Revision 1.4.2.3  2005/05/09 11:59:19  jla
 * Changed member to four byte array
 *
 * Revision 1.4.2.2  2005/05/09 10:04:26  jla
 * Cosmetics
 *
 * Revision 1.4.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.3  2005/04/12 14:13:09  jla
 * Scratch version
 *
 * Revision 1.2  2005/04/06 12:15:47  jla
 * Scratch version II
 *
 *
 */

#ifndef _IPADDRESS_H_
#define _IPADDRESS_H_

#include <iostream>
#include <stdio.h>
using namespace std;

#define IPLEN			4					//!<Number of bytes in an IP address

/**\brief Wrapper class for an IP address
 *
 */
class IPAddress
{
	friend ostream &operator<<(ostream&, const IPAddress &);	//!<Enables IPAddress object to used directly in streams.

public:
	IPAddress();
    IPAddress(string &s);					//!<Initialize with string
	IPAddress(unsigned char *);				//!<Initialize with pointer to 4 8-bit values
	IPAddress(unsigned char field3,
			  unsigned char field2,
			  unsigned char field1,
			  unsigned char field0);		//!<Initialize with 4 8-bit values
	IPAddress(unsigned int);				//!<Initialize with 32-bit value

	virtual ~IPAddress();

	void getIPAddress(unsigned char *);		//!<Copy IP address to 4-byte array
  unsigned int getIPAddress();      //!<Returns 32 bit value containing IP address. Eg. 1.0.168.192
  unsigned int getIPAddressMpc();   //!<Returns 32 bit value containing IP address. Eg. 192.168.0.1

	bool fromString(string );				//!<Convert IP address from a string
	void toString(string &);				//!<Convert IP address to a string

	bool operator==(const IPAddress &right) const;	//!<Equality operator
	bool operator!=(const IPAddress &right) const	//!<Inequality operator
		{return !(*this == right);}

private:
	unsigned char mipadr[4];			//!<The IP address, MSB in mipadr[0] and LSB in mipadr[3]
};

#endif //_IPADDRESS_H_
