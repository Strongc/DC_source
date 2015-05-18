//
// WEBCONF.H - WEB Configuration File
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//

#ifndef __WEBCONF_H__
#define __WEBCONF_H__ 1

#include "ostype.h"

// ********************************************************************
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING */
/* Set to 1 to include code for web "PUT" operations, used in web
   publishing over HTTP. This is zero by default, because there is
   currently no authentication for this function. */
#define INCLUDE_WEB_PUT 0		//JLA , was 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING */
// set to 1 to enable authentication of WEB requests from browsers; a
// list of acceptable IP/masks may be set up by calling http_set_browser_list;
// if http_set_browser_list is not called no security checking will be
// done
#define INCLUDE_WEB_SECURITY 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING */
// set to 1 to enable Basic authentication (from RFC 1945/2068)
#define INCLUDE_WEB_AUTHENTICATION 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING */
// set to 1 to include code to allow mime fields specified by the
// user to be in the mime header sent with a web page in response
// to a GET command, the mime fields are supplied by api http_mime_fields()
// web_page_mime_fields
#define INCLUDE_WEB_EXTRA_MIME 1

#if (INCLUDE_RTFILES)
#define INCLUDE_WEB_CWD 	1
#else
#define INCLUDE_WEB_CWD 	0
#endif
#define _CFG_WEB_ROOT_DIR 	"C:\\"

// By setting this to 1, the we will include the date in the mime header
// of files uploaded (if it does not contain any server-side includes;
// the browser will send If-Modified-Since on subsequent uploads of the
// same file; if the file has not been modified since date specified
// in If-Modified-Since field of mime header in the requests, a
// 304: Not Modifed message will be returned to the browser
#define _CFG_MOD_DATE 1

// ********************************************************************
// HTTP 1.1 support
// ********************************************************************
// set to 1 to include support for HTTP/1.1 (RFC 2068)
#define INCLUDE_WEB_11 1

#define INCLUDE_WEB_ETAG 0	// NO SUPPORTED

// set to 1 to enable Persistent Connections (from RFC 2068)
// This is a HTTP 1.1 feature
// NOTE: INCLUDE_WEB_11 must also be set
#define INCLUDE_WEB_PERSIST_CONNECT 1

// Maximum time in a persistent connection before closing the socket
// Units: seconds
// NOTE: INCLUDE_WEB_11 and INCLUDE_WEB_PERSIST_CONNECT must be set
//       for this constant to be used
// NOTE: if this is too large, display from POST command will be slow
//       since many browsers wait for FIN before displaying the
//       data
#define _CFG_WEB_KEEPALIVE_TMO 1

// Maximum number of requests which will be processed in a persistent
// connection before closing the socket
// NOTE: INCLUDE_WEB_11 and INCLUDE_WEB_PERSIST_CONNECT must be set
//       for this constant to be used
#define _CFG_WEB_KEEPALIVE_MAX 5

// ********************************************************************
// maximum cookie (username:password 64-bit encoded) used for Basic
// authentication; valid only if INCLUDE_WEB_AUTHENTICATION is 1
#define _CFG_MAX_AUTH_COOKIE 40

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING */
// size of browser security table
#define _CFG_MAX_BROWSERS 5

// amount of time to wait for a response from Web browser, i.e. to wait
// send data or to wait for command after accepting connection
// from browser
// units: seconds
#define _CFG_WEB_TIMEOUT 20

// ********************************************************************
/* backlog values for servers when doing a listen */
#define _CFG_WEB_LISTEN_BACKLOG 10

//****************************************************************
// FILE COMPRESSION
//****************************************************************

// include code to compress files (virtual and disk files)
#define INCLUDE_WEB_COMPRESS 0			/* NOT SUPPORTED YET */

#endif		// __WEBCONF_H__

