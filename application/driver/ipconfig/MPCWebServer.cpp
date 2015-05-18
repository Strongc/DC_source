/** \file MPCWebServer.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: MPCWebServer.cpp
 *
 * $Log: MPCWebServer.cpp,v $
 * Revision 1.1.2.4  2005/06/15 09:15:34  jla
 * Added resetPassword method
 *
 * Revision 1.1.2.3  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.2  2005/05/26 09:28:14  jla
 * Implemented the class
 *
 * Revision 1.1.2.1  2005/05/14 11:02:25  jla
 * Added a class for the webserver
 *
 */

#include "MPCWebServer.h"
#include "AppTypeDefs.h"

#define TRACE(x)

MPCWebServer* MPCWebServer::mpinstance=0;

MPCWebServer::MPCWebServer()
  :	mrealm(PRODUCT_NAME),
	mfilesysteminit(false),	mkill(false),
	mauthenticationenabled(false),
	musername(DEFAULTUSERNAME),
	mpassword(DEFAULTPASSWORD)
{
	musrpwd = DEFAULTUSERNAME;
	musrpwd += ":";
	musrpwd += DEFAULTPASSWORD;
}

MPCWebServer::~MPCWebServer()
{
}

MPCWebServer* MPCWebServer::getInstance()
{
	if (MPCWebServer::mpinstance == 0)
	{
		MPCWebServer::mpinstance = new MPCWebServer;
	}

	return MPCWebServer::mpinstance;
}

/**Start the webserver.
 * Task priority and stack size are defined in MPCWebServer.h
 *
 * \param interfaceno Not used
 */
void MPCWebServer::start(int interfaceno)
{
	mkill = false;

	if (!mfilesysteminit)
	{
		mfilesysteminit = true;
		http_file_init();
	}

	if (mauthenticationenabled)
	{
		enableAuthentication();			//Call this method to deploy new password
	}

	Thread::start(WEBSERVTASKPRIO, "MPCWebServer Task", WEBSERVTASKSTACKSIZE);
}

/**Stops the webserver and terminates the thread.
 */
void MPCWebServer::stop()
{
	mkill = true;
	http_kill_server_daemon();

	while(isRunning())				//Wait for thread to be terminated
	{
		delay(50);
	}
}


/**
 * Maximum length of username is
 * \link MPCWebServer.h MPCUSRPWDSIZE \endlink.
 * This method also resets password to
 * \link MPCWebServer.h DEFAULTPASSWORD \endlink.
 *
 * If an error occurs, you can get an error description with
 * getErrorString()
 *
 * \param username The new username.
 * \return True if successful, false if error.
 */
bool MPCWebServer::setUsername(const string& username)
{
	if (username.length() > MPCUSRPWDSIZE)
	{
		//throw(string("Username is too long. (More than 16 letters.)"));
		merrorstring = "Username is too long. (More than 16 letters.)";
		return false;
	}

	musername = username;
	mpassword = DEFAULTPASSWORD;

	PasswordConfiguration::getInstance()->setUsername(musername);
	PasswordConfiguration::getInstance()->setPassword(mpassword);

	if (mauthenticationenabled)
	{
		enableAuthentication();			//Call this method to deploy new username
	}

	return true;
}

/**
 * \return Reference to string containing username.
 */
const string& MPCWebServer::getUsername()
{
	return musername;
}

/**
 * Maximum length of password is
 * \link MPCWebServer.h MPCUSRPWDSIZE \endlink.
 *
 * If an error occurs, you can get an error description with
 * getErrorString()
 *
 * \param existingpw The existing password must be specified to change it.
 * \param newpw The new password.
 * \throws string A string containing an errordescription may be thrown.
 * \return True if successful, false if error.
 */
bool MPCWebServer::setPassword(string& existingpw, string& newpw)
{
	if (existingpw != mpassword)
	{
		//throw (string("Wrong existing password."));
		merrorstring = "Wrong existing password.";
		return false;
	}

	if (newpw.length() > MPCUSRPWDSIZE)
	{
		//throw(string("Password is too long. (More than 16 letters.)"));
		merrorstring = "Password is too long. (More than 16 letters.)";
		return false;
	}

	mpassword = newpw;

	PasswordConfiguration::getInstance()->setPassword(mpassword);

	if (mauthenticationenabled)
	{
		enableAuthentication();			//Call this method to deploy new password
	}

	return true;
}

/**
 * Maximum length of password is
 * \link MPCWebServer.h MPCUSRPWDSIZE \endlink.
 *
 * \return True if successful, false if error.
 */
bool MPCWebServer::setPassword(string& newpw)
{

  if (newpw.length() > MPCUSRPWDSIZE)
	{
		return false;
	}

	mpassword = newpw;

	PasswordConfiguration::getInstance()->setPassword(mpassword);

	if (mauthenticationenabled)
	{
		enableAuthentication();			//Call this method to deploy new password
	}

	return true;
}


/**Resets password to default password. The default password is
 * \link MPCWebServer.h DEFAULTPASSWORD \endlink.
 */
void MPCWebServer::resetPassword()
{
	mpassword = DEFAULTPASSWORD;

	PasswordConfiguration::getInstance()->setPassword(mpassword);

	if (mauthenticationenabled)
	{
		enableAuthentication();			//Call this method to deploy new password
	}
}

/**
 * \return Reference to string containing error description.
 */
const string& MPCWebServer::getErrorString()
{
	return merrorstring;
}

/**
 * This method must be called to enable
 * authentication.
 */
void MPCWebServer::enableAuthentication()
{
	int i;

	musrpwd = musername + ":" + mpassword;

	for (i=0 ; i < WEBPAGES_NO_OF_PROTECTED_FILES; i++)
	{
		mauthinfo[i].filename = const_cast<char*>(WEBPAGES_PROTECTED_FILES[i]);//const_cast<char*>(mfilename[i].c_str());
		mauthinfo[i].realm = const_cast<char*>(mrealm.c_str());
		mauthinfo[i].user_pwd = const_cast<char*>(musrpwd.c_str());
		mauthinfo[i].encoded_user_pwd = mencusrpwd;
		mauthinfo[i].err401_file = 0;					//RTIP expects this to be set to something valid or 0:-/
														//although it is doc'ed as internal use, RTIP only reads it,
														//which makes the webserver go haywire if the heap is not inited to 0
														//JLA IO Technologies, 2005-10-21
	}

	http_set_auth(mauthinfo, WEBPAGES_NO_OF_PROTECTED_FILES);

	mauthenticationenabled = true;
}

/**
 *
 */
void MPCWebServer::disableAuthentication()
{
	http_set_auth(0, 0);

	mauthenticationenabled = false;
}

/**The run method keeps calling the RTIP http_server_daemon function
 * until stop method is called. (http_server_daemon only returns if
 * error occurs, or if http_kill_server_daemon is called.)
 *
 */
void MPCWebServer::run()
{
	while (!mkill)
	{
		if (http_server_daemon() == -1)
  	{
 	   	TRACE(OS_DebugSendString("HTTP server failed.\n"););
 	   	delay(1000);
  	}
  	else
  	{
 	    TRACE(OS_DebugSendString("HTTP server finished.\n"););
  	}
	}

	terminate();
}

