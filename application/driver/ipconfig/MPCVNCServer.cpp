/** \file MPCVNCServer.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: MPCVNCServer.cpp
 *
 * $Log: MPCVNCServer.cpp,v $
 * Revision 1.1.2.2  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.1  2005/05/26 09:31:53  jla
 * Added a class to encapsulate the VNC server
 *
 */


#include "MPCVNCServer.h"

#include "RTIP.H"     /* RTIP API */
#include "SOCKET.H"   /* BSD socket interface */

#include "GUI.h"
#include "GUI_X.h"
#include "GUI_VNC.h"


MPCVNCServer* MPCVNCServer::mpinstance=0;

static GUI_VNC_CONTEXT _Context;
static int s;						//!<Socket id used in _ServerTask and terminate

static void _ServerTask(void);


MPCVNCServer::MPCVNCServer()
 : 	mkill(false)
{
}

MPCVNCServer::~MPCVNCServer()
{
}

MPCVNCServer* MPCVNCServer::getInstance()
{
	if (mpinstance == 0)
	{
		mpinstance = new MPCVNCServer;
	}

	return mpinstance;
}

/**This method must be called only once when booting the system.
 * The display has to be initialized and running before doing so.
 *
 * \param layerindex Layerindex passed to GUI_VNC_AttachToLayer.
 * \param serverindex Serverindex copied to _Context.ServerIndex.
 */
void MPCVNCServer::init(int layerindex, int serverindex)
{
	GUI_VNC_AttachToLayer(&_Context, layerindex);
	_Context.ServerIndex = serverindex;
}

/**Task priority and stack size are defined in MPCVNCServer.h
 */
void MPCVNCServer::start()
{
	mkill = false;
	Thread::start(VNCSERVTASKPRIO, "MPCVNCServer task", VNCSERVTASKSTACKSIZE);
}

/**The Thread::terminate has to be overloaded, because it is necessary to
 * close the socket used by the VNC server before terminating the thread.
 *
 */
void MPCVNCServer::terminate()
{
	mkill = true;
	xn_abort(s, TRUE);
	delay(100);
	if (isRunning())
	{
	  Thread::terminate();				//Make sure thread is killed, though xn_abort ougth to do it
										//(_ServerTask returns when IP socket binding is aborted)
	}
}

/**The run method calls the _ServerTask copied from GUI_VNC_X_StartServer.c.
 * _ServerTask returns if the socket it is listening on is closed, and then
 * we can terminate the thread, if we want to.
 */
void MPCVNCServer::run()
{
	while (!mkill)
	{
		_ServerTask();
	}

	Thread::terminate();
}


/*********************************************************************
*
*       _Send
*
* Function description
*   This function is called indirectly by the server; it's address is passed to the actual
*   server code as function pointer. It is needed because the server is independent
*   of the TCP/IP stack implementation, so details for the TCP/IP stack can be placed here.
* Notes:
*   (1) This implementation is for EBS's RTIP stack.
*   (2) It switches between blocking and non-blocking mode; This is due to a weekness
*       of the RTIP stack: It tries to fill up the TCP-window before sending the
*       package; but since the socket normally blocks, no more data is send to the socket
*       until a time-out expires of about 200 ms. This would slow down communication
*       dramatically and can be avoided with the "trick" below; as long as there is enough
*       space in the TCP-window, we use non-blocking mode.
*/
static int _Send(const U8* buf, int len, void* pConnectionInfo) {
  unsigned long r;                             /* Needs to be unsigned long, because ioctlsocket takes a pointer to it */
  SOCKET socket;
  unsigned long zero = 0;

  socket = (SOCKET)pConnectionInfo;
  ioctlsocket (socket, FIONWRITE, &r);
  if (r > len) {
    unsigned long one  = 1;
    ioctlsocket (socket, FIONBIO, &one); /* nonblocking */
  }
  r = send(socket, (const char*)buf, len, 0);
  ioctlsocket (socket, FIONBIO, &zero); /* blocking */
  return r;
}

/*********************************************************************
*
*       _Recv
*
* Function description
*   This function is called indirectly by the server; it's address is passed to the actual
*   server code as function pointer. It is needed because the server is independent
*   of the TCP/IP stack implementation, so details for the TCP/IP stack can be placed here.
* Notes:
*   (1) This implementation is for EBS's RTIP stack, but it should work without modification
*       on most TCP/IP stacks.
*/
static int _Recv(U8* buf, int len, void* pConnectionInfo) {
  return recv((SOCKET)pConnectionInfo, (char*)buf, len, 0);
}

/*********************************************************************
*
*       _ServerTask
*
* Function description
*   This routine is the actual server task.
*   It executes some one-time init code, then runs in an ednless loop.
*   It therefor does not terminate.
*   In the endless loop it
*     - Waits for a conection from a client
*     - Runs the server code
*     - Closes the connection
*/
static void _ServerTask(void) {
  //int s, sock;		//JLA, s made global
  int sock;
  short Port;
  struct sockaddr_in addr_in;
  const int one = 1;

  /* Prepare socket (one time setup) */
  Port = 5900 + _Context.ServerIndex;                     /* Default port for VNC is is 590x, where x is the 0-based layer index */
  addr_in.sin_family      = AF_INET;
  addr_in.sin_port        = htons(Port);
  addr_in.sin_addr.s_addr = INADDR_ANY;
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    return;                                               /* Error ... We are done with this task */
  }
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *) &one, sizeof(one)) < 0) {
    closesocket(s);
    return;                                               /* Error ... We are done with this task */
  }
  if (bind(s, (struct sockaddr *)&addr_in, sizeof(addr_in))) {
    closesocket(s);
    return;                                               /* Error ... We are done with this task */
  }
  if (listen(s, 1)) {
    closesocket(s);
    return;                                               /* Error ... We are done with this task */
  }

  /* Endless loop. We run thru it once for every client connecting to us */
  while (1) {
    struct sockaddr addr;
    int addrlen;
		int rc;

    /* Wait for an incoming connection */
    addrlen = sizeof(addr);
    if ((sock = accept(s, &addr, &addrlen)) < 0) {
      return;                                             /* Error ... We are done with this task */
    }

    /* Disable Nagle's algorithm - improves performance (optional) */
    {
      const int zero = 0;
      rc = setsockopt(sock, SOL_SOCKET, SO_NAGLE, (const char *) &zero, sizeof(zero));
      if (rc)
      {
        OS_DebugSendString("MPCVNCServer: Failed to set SO_NAGLE"); /*OS*/ /* output to embOSView terminal window */
      }
    }

	/* -----------------14-09-2005 15:49-----------------
   * set socket receive timeout to avoid a socket
   * connection to hang in the VNC process
	 * --------------------------------------------------*/
	{
		struct timeval timeout;

    timeout.tv_sec = 20;
		timeout.tv_usec = 0;

    if (rc == 0)
    {
      rc = setsockopt(sock, SOL_SOCKET, SO_RCV_TIMEO, (const char *)&timeout, sizeof(timeout));
      if (rc)
      {
        OS_DebugSendString("MPCVNCServer: Failed to set SO_RCV_TIMEO"); /*OS*/ /* output to embOSView terminal window */
      }
    }
	}

    /* Run the actual server */
    if (rc == 0)
    {
      GUI_VNC_Process(&_Context, _Send, _Recv, (void*)sock);
    }

    /* Close the connection */
    closesocket(sock);
  }
}
