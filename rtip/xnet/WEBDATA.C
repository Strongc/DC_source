//
// WEBDATA.C - WEB Data
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//
//  Module description:
//  	This module contains all the global data used by WEB.
//

#include "sock.h"

#if (INCLUDE_WEB)
#include "web.h"
#include "webapi.h"
#include "servers.h"

#if (INCLUDE_RUN_TIME_CONFIG)
// ********************************************************************
CFG_WEB_DATA KS_FAR cfg_web_data = 
{
	_CFG_WEB_KEEPALIVE_TMO,
	_CFG_WEB_KEEPALIVE_MAX,
	_CFG_WEB_LISTEN_BACKLOG,
	_CFG_WEB_TIMEOUT,
#if (CONFIG_PARAM_DONE)
	_CFG_MAX_AUTH_COOKIE,
	_CFG_MAX_BROWSERS,
#endif
#if INCLUDE_WEB_CWD
	_CFG_WEB_ROOT_DIR,
#endif
	_CFG_MOD_DATE
};
#endif

#if (defined(_MSC_VER))
// ********************************************************************
// dummy routine needed to compensate for a bug in linker; it avoids
// the data in this file being unresolved
void web_dummy(void)
{
}
#endif

//****************************************************************
#if (INCLUDE_RTIP)
// used to register WEB init fnc
INIT_FNCS KS_FAR web_fnc;		
#endif

//****************************************************************
// webapi.c
//****************************************************************

#if (INCLUDE_WEB_EXTRA_MIME)
// pointer to extra mime fields supplied by user to be sent in response
// to a GET command for specified web pages;
// pointer to user area set up by http_mime_fields()
PFMIME_FOR_PAGES web_mime_fields;
#endif

// context information for WEB server (includes master socket etc)
struct server_context KS_FAR http_server_context;

#if (INCLUDE_HTTPS_SRV)
// context information for HTTPS server (includes master socket etc)
struct server_context KS_FAR https_server_context;
#endif


#if (!CFG_SPAWN_WEB)
// control socket for WEB server (needs to be global so can kill task)
int KS_FAR web_csocket;
#endif

#if (CFG_SPAWN_WEB && !INCLUDE_MALLOC_CONTEXT)
struct _web_context KS_FAR httpcntxt[CFG_WEB_MAX_SPAWN];
#endif
#if (!CFG_SPAWN_WEB)
struct _web_context KS_FAR httpcntxt[1];
#endif

#if (INCLUDE_HTTPS_SRV)
#if (CFG_SPAWN_WEB && !INCLUDE_MALLOC_CONTEXT)
struct _web_context KS_FAR httpscntxt[CFG_WEB_MAX_SPAWN];
#endif
#if (!CFG_SPAWN_WEB)
struct _web_context KS_FAR httpscntxt[1];
#endif
#endif

#if (POLLOS)
// handle to task to run by poll_os_cycle
POLLOS_TASK pollos_web_handle;
#endif

#if (INCLUDE_WEB_SECURITY)
	// list of browsers IP addresses which are allowed to access web pages
	browser KS_FAR browser_ip_table[CFG_MAX_BROWSERS];
#endif

#if (INCLUDE_WEB_AUTHENTICATION)
//****************************************************************
// BASIC AUTHENTICATION
//****************************************************************
// pointer to basic authentication info;
// setup by http_set_auth()
struct web_auth_entry KS_FAR *web_basic_auth;

// number of entries in web_basic_auth
int KS_FAR num_web_basic_auth;

#endif
#endif // INCLUDE_WEB

