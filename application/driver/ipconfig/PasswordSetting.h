/** \file PasswordSetting.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: PasswordSetting.h
 *
 * $Log: PasswordSetting.h,v $
 * Revision 1.1.2.2  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.1  2005/05/26 09:31:20  jla
 * Added a class for handling the webpage password.
 *
 */

#ifndef __PASSWORD_SETTING_H__
#define __PASSWORD_SETTING_H__

#include <string>

#include "MPCWebServer.h"

using namespace std;

/**\brief Handles setting of webpage password.
 *
 * This class contains the password setting functionality.
 */
class PasswordSetting
{
public:
	virtual ~PasswordSetting();
	static PasswordSetting* getInstance();			//!<Get pointer to singleton instance

  // methods called by the WebIfHandler
  void webifGetUserName(std::ostream& res);
  void webifGetTextMessage(std::ostream& res);
  string& webifPostNewConfig(const char* existingpw, const char* newpw, const char* repnewpw);
  
protected:
	PasswordSetting();

private:
	string mhttpresponse;
	string mtextresponse;

	static PasswordSetting* mpinstance;
};

#endif //_PASSWORDSETTING_H_
