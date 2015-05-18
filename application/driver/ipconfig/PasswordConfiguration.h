/** \file PasswordConfiguration.h
 * 
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 * 
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: PasswordConfiguration.h
 * 
 * $Log: PasswordConfiguration.h,v $
 * Revision 1.1.2.1  2005/07/06 10:19:03  jla
 * class PasswordConfiguration added.
 *
 */

#ifndef _PASSWORDCONFIGURATION_H_
#define _PASSWORDCONFIGURATION_H_

class PasswordConfiguration;

#include <string>
#include <iostream>
#include "SubjectIO.h"
#include "MPCWebServer.h"

using namespace std;

/**\brief This class contains basic password configuration.
 * 
 * This class is added to allow the password to be observed, in order
 * to be able to update datapoints in the MPC software. An observer
 * must be a subclass of ObserverIO and attach to PasswordConfiguration by
 * calling PasswordConfiguration::attach.
 * 
 * If the username or password has to be changed, this must done via 
 * the MPCWebserver class.
 */
class PasswordConfiguration : public SubjectIO
{
	friend class MPCWebServer;						//!<Grant access to setUsername and setPassword
	
public:
	virtual ~PasswordConfiguration();
	
	static PasswordConfiguration* getInstance();	//!<Get pointer to singleton instance.
	
	void getUsername(string &);						//!<Get username
	void getPassword(string &);						//!<Get password
	
protected:
	PasswordConfiguration();						//!<Contructor

	void setUsername(const string &);				//!<Set username
	void setPassword(const string &);				//!<Set password
	
private:
	string musername;
	string mpassword;
	
	static PasswordConfiguration* mpinstance;
};

#endif //_PASSWORDCONFIGURATION_H_
