/** \file PasswordSetting.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: PasswordSetting.cpp
 *
 * $Log: PasswordSetting.cpp,v $
 * Revision 1.1.2.2  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.1  2005/05/26 09:31:20  jla
 * Added a class for handling the webpage password.
 *
 */

#include "PasswordSetting.h"


PasswordSetting* PasswordSetting::mpinstance=0;

PasswordSetting::PasswordSetting()
{
}

PasswordSetting::~PasswordSetting()
{
}

PasswordSetting* PasswordSetting::getInstance()
{
	if (mpinstance==0)
	{
		mpinstance = new PasswordSetting;
	}

	return mpinstance;
}


void PasswordSetting::webifGetUserName(std::ostream& res)
{
  res << MPCWebServer::getInstance()->getUsername();
}

void PasswordSetting::webifGetTextMessage(std::ostream& res)
{
  res << mtextresponse;
  mtextresponse = ""; // delete old response message
}

/**This method handles the data submitted from the password setting webpage.
 * The new password has to be entered two times. If the two entries are equal,
 * the new and existing password is sent to MPCWebServer, that handles the
 * actual setting of the password.
 * 
 * After doing so, a response is sent to the webbrowser, instructing it to
 * reload the page, in order to display a response to the user.
 * 
 * \param existingpw Pointer to existing password.
 * \param newpw Pointer to new password.
 * \param repnewpw Pointer to repeated new password.
 * 
 * \return Reference to string containing HTML redirect response for webbrowser.
 */
string& PasswordSetting::webifPostNewConfig(const char* existingpw, const char* newpw, const char* repnewpw)
{
	string existpassword, newpassword, repeatnewpassword;
	bool proceed = true;

	if (strlen(existingpw) > 0)
	{
		existpassword = existingpw;
	}
	else
	{
		proceed = false;
		mtextresponse = "Please enter existing password.";
	}

	if (proceed)
  {
  	if ( strlen(newpw) > 0)
  	{
  		newpassword = newpw;
  	}
  	else
  	{
  		proceed = false;
  		mtextresponse = "Please enter new password.";
  	}
  }

	if (proceed)
  {
  	if (strlen(repnewpw) > 0)
  	{
  		repeatnewpassword = repnewpw;
  	}
  	else
  	{
  		proceed = false;
  		mtextresponse = "Please repeat new password.";
  	}
  }

	if (proceed)
	{
		if (repeatnewpassword != newpassword)
		{
			proceed = false;
			mtextresponse = "New and repeated new password differ.";
		}
		else
		{
			if (MPCWebServer::getInstance()->setPassword(existpassword, newpassword))
			{
				mtextresponse = "Password changed successfully.";
			}
			else
			{
				mtextresponse = MPCWebServer::getInstance()->getErrorString();
			}
		}
	}

	mhttpresponse = "/pwconfig.html";

	return mhttpresponse;
}
