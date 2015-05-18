/** \file MPCWebServer.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: MPCWebServer.h
 *
 * $Log: MPCWebServer.h,v $
 * Revision x.x.x.5  2005/09/11 fka
 * PC stub...
 *
  * $Log: MPCWebServer.h,v $
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

#ifndef _MPCWEBSERVER_H_
#define _MPCWEBSERVER_H_

class MPCWebServer;


using namespace std;

/**\brief Encapsulates the RTIP webserver
 *
 * The webserver may be started, and authentication may be set/changed.
 * It's a singleton class.
 *
 * The authentication is applied to all pages used by the server (see MPCWebServer.h).
 * User/password pairs are the same for all pages.
 */
class MPCWebServer
{
public:
  virtual ~MPCWebServer() {};
	static MPCWebServer* getInstance();					//!<Returns pointer to singleton instance
  void resetPassword() {};               //!<Reset password to default

protected:
  MPCWebServer() {};                   //!<Protectec constructor, due to singleton pattern
 private:
  static MPCWebServer* mpinstance;
};


MPCWebServer* MPCWebServer::getInstance()
{
	if (MPCWebServer::mpinstance == 0)
	{
		MPCWebServer::mpinstance = new MPCWebServer;
	}

	return MPCWebServer::mpinstance;
}

MPCWebServer* MPCWebServer::mpinstance=0;


#endif //_MPCWEBSERVER_H_
