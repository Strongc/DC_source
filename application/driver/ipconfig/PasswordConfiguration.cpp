/** \file PasswordConfiguration.cpp
 * 
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 * 
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: PasswordConfiguration.cpp
 * 
 * $Log: PasswordConfiguration.cpp,v $
 * Revision 1.1.2.1  2005/07/06 10:19:03  jla
 * class PasswordConfiguration added.
 *
 */
 
#include "PasswordConfiguration.h"

PasswordConfiguration* PasswordConfiguration::mpinstance = 0;

PasswordConfiguration::PasswordConfiguration()
{
}

PasswordConfiguration::~PasswordConfiguration()
{
}

PasswordConfiguration* PasswordConfiguration::getInstance()
{
	if (mpinstance == 0)
	{
		mpinstance = new PasswordConfiguration;
	}
	
	return mpinstance;
}

/**
 * This method will call the SubjectIO notify method.
 */
void PasswordConfiguration::setUsername(const string &s)
{
	musername = s;
	
	notify();
}

/**
 * This method will call the SubjectIO notify method.
 */
void PasswordConfiguration::setPassword(const string &s)
{
	mpassword = s;
	
	notify();
}

void PasswordConfiguration::getUsername(string &s)
{
	s = musername;
}

void PasswordConfiguration::getPassword(string &s)
{
	s = mpassword;
}

