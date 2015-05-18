/** \file IPAddress.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPAddress.cpp
 *
 * $Log: IPAddress.cpp,v $
 * Revision 1.2.2.6  2005/06/10 08:53:41  jla
 * Added getIPAddress method that returns a 32 bit integer
 *
 * Revision 1.2.2.5  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.2.2.4  2005/05/14 10:24:36  jla
 * Added == and != operators
 *
 * Revision 1.2.2.3  2005/05/09 11:59:19  jla
 * Changed member to four byte array
 *
 * Revision 1.2.2.2  2005/05/09 09:39:57  jla
 * Changes byte order in getIPAddress(unsigned char *)
 *
 * Revision 1.2.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#include "IPAddress.h"

/**The IP address in initialized to 0.0.0.0
 */
IPAddress::IPAddress()
{
	int i;
	for (i=0 ; i<IPLEN ; i++)
	{
		mipadr[i] = 0;
	}
}

IPAddress::~IPAddress()
{
}

/**The object will be initialized with the value set by fromString.
 * \note The return value from fromString is ignored, parsing of the string
 * could fail without further notice.
 */
IPAddress::IPAddress(string &s)					//!<Initialize with string
{
	fromString(s);
}

/**Example:
 * \code
 * unsigned char ip[4] = {192,168,1,1};
 * IPAddress ipobj(ip);
 * \endcode
 *
 * \param field Pointer to an unsigned char array 4 elements long.
 */
IPAddress::IPAddress(unsigned char *field)
{
	int i;
	for (i=0 ; i<IPLEN ; i++)
	{
		mipadr[i] = field[i];
	}
}

IPAddress::IPAddress(unsigned char field3,
			 		 unsigned char field2,
					 unsigned char field1,
					 unsigned char field0)
{
	mipadr[0] = field3;
	mipadr[1] = field2;
	mipadr[2] = field1;
	mipadr[3] = field0;
}

IPAddress::IPAddress(unsigned int ip)
{
	int i;
	for (i=0 ; i<IPLEN ; i++)
	{
		mipadr[IPLEN-1-i] = ip & 0x000000FF;
		ip >>= 8;
	}

}

/**The most significant byte is put in ipaddrarray[0]
 *
 * \param ipaddrarray Pointer to unsigned char array, at least 4 elements long.
 */
void IPAddress::getIPAddress(unsigned char *ipaddrarray)
{
	int i;
	for (i=0 ; i<IPLEN ; i++)
	{
		ipaddrarray[i] = mipadr[i];
	}
}

/**
 * \return IP address, eg. 1.0.168.192
 */
unsigned int IPAddress::getIPAddress()
{
	unsigned int retval=0;

  retval |= mipadr[0] << 0;
  retval |= mipadr[1] << 8;
  retval |= mipadr[2] << 16;
  retval |= mipadr[3] << 24;
	return retval;
}

/**
 * \return IP address, eg. 192.168.0.1
 */
unsigned int IPAddress::getIPAddressMpc()
{
	unsigned int retval=0;

  retval |= mipadr[3] << 0;
  retval |= mipadr[2] << 8;
  retval |= mipadr[1] << 16;
  retval |= mipadr[0] << 24;
	return retval;
}

/**The IP address is converted to a string without leading zeros.
 * \param s Reference to string, where IP address is put.
 */
void IPAddress::toString(string &s)
{
	char buffer[20];
	sprintf(buffer, "%u.%u.%u.%u",
			mipadr[0],	mipadr[1], mipadr[2], mipadr[3]);

	s = buffer;
}

/**The conversion from a string to an IP address will fail if the
 * string contains other characters than '0','1','2','3','4','5','6','7','8','9' and '.'.
 *
 * \param s String to be converted.
 * \return True if conversion succeded, false if it failed.
 */
bool IPAddress::fromString(string s)
{
	bool retval = false;

	unsigned char ipadr[4];
	int i, end, temp;
	string field;

	if (s.find_first_not_of("0123456789.") != string::npos)	//Check for illegal chars
	{
		return retval;
	}
	for (i=0 ; i<IPLEN ; i++)
	{
		if ((end = s.find(".")) == string::npos && i<IPLEN-1)
			return retval;

		field = s.substr(0, end);
		s.erase(0,end+1);

		temp = atoi(field.c_str());
		if (temp<0 || temp > 255)
			return retval;

		ipadr[i] = temp;
	}

	*this = IPAddress(ipadr);

    return true;
}

bool IPAddress::operator==(const IPAddress &right) const
{
	return (	this->mipadr[0] == right.mipadr[0]
			&&	this->mipadr[1] == right.mipadr[1]
			&&	this->mipadr[2] == right.mipadr[2]
			&&	this->mipadr[3] == right.mipadr[3]);

}

ostream &operator<<(ostream &output, const IPAddress &input)
{
	output << 	static_cast<unsigned int>( input.mipadr[0]) << '.' <<
				static_cast<unsigned int>( input.mipadr[1]) << '.' <<
				static_cast<unsigned int>( input.mipadr[2]) << '.' <<
				static_cast<unsigned int>( input.mipadr[3]);

    return output;
}
