/** \file MPCVNCServer.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: MPCVNCServer.h
 *
 * $Log: MPCVNCServer.h,v $
 * Revision 1.1.2.2  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.1  2005/05/26 09:31:53  jla
 * Added a class to encapsulate the VNC server
 *
 */

#ifndef _MPCVNCSERVER_H_
#define _MPCVNCSERVER_H_

#include "Thread.h"


#define	VNCSERVTASKPRIO			50				//!<Priority of VNC server task
#define VNCSERVTASKSTACKSIZE	(16*1024)		//!<Size of stack for VNC server task


/**\brief Encapsulates the VNC Server
 *
 * Implementation is based on existing C source (GUI_VNC_X_StartServer.c).
 * It'a a singleton class.
 */
class MPCVNCServer : public Thread
{
public:
	virtual ~MPCVNCServer();

	static MPCVNCServer* getInstance();				//!<Get pointer to the singleton instance

	void init(int layerindex, int serverindex);		//!<Initializes the VNC server
	void start();									//!<Starts the VNC server
	void terminate();								//!<Overloads the Thread terminate method

protected:
	MPCVNCServer();
	virtual void run();								//!<VNC server thread

private:
	bool mkill;										//!<True when thread has to be terminated
	static MPCVNCServer* mpinstance;
};

#endif //_MPCVNCSERVER_H_
