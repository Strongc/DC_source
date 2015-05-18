//
// WEBAPI.C - WEB API functions
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//


#include "sock.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#include "rtipext.h"
#endif
#include "servers.h"

#if (INCLUDE_WEB)
#include "web.h"
#include "webapi.h"
#include "vfile.h"
#include "base64.h"

#ifdef RTKERNEL32
    static const char * OTCR = "\r\n"
        "RTIP-32/WEB Server 4.00 "
        "Copyright (c) 2001,2002 On Time Informatik GmbH\r\n"
        "Copyright (c) 1993,2002 EBS, Inc.\r\n";
#endif

#if defined(RTKERNEL32)
#if defined(EVALVER)
    #include <evalchk.h>
#else
    #define RTEVALCHECK(n, m)
#endif
#endif

// ********************************************************************
// DEBUG AIDS
// ********************************************************************
#define DEBUG_AUTHENTICATION 0		// turn on to debug basic authentication
									// NOTE: DEBUG_DECODE must be turned
									//       on in base64.c
									// NOTE: decode code must be turned
									//       on in tools.c and rtip.h;
									//       it is usually only turned on
									//       for POP3
#define DEBUG_WEB 0

// ********************************************************************
#define INIT_WEB_CONTEXT(ctx) (ctx)->web_socket = INVALID_SOCKET;

//****************************************************************
#if (CFG_SPAWN_WEB || POLLOS)
int          _http_server_daemon(PFBYTE ip_bind_addr);
int          _web_server_daemon(RTIP_BOOLEAN secure_server, PFBYTE ip_bind_addr);
RTIP_BOOLEAN web_process_spawn (int socket_no, PROT_CTXT_OR_INDEX proto_index, RTIP_BOOLEAN secure_server);
RTIP_BOOLEAN http_process_spawn(int socket_no, PROT_CTXT_OR_INDEX proto_cntxt);
#if (INCLUDE_HTTPS_SRV)
RTIP_BOOLEAN https_process_spawn(int socket_no, PROT_CTXT_OR_INDEX proto_cntxt);
#endif
#endif

#if (!CFG_SPAWN_WEB)
int _http_server_process_one(PWEB_CONTEXT ctx, unsigned int secure);
int _web_server_daemon(RTIP_BOOLEAN secure_server, PFBYTE ip_bind_addr);
#endif

#if (POLLOS)
void pollos_http_main(void);
#if (INCLUDE_HTTPS_SRV)
void pollos_https_main(void);
#endif
void pollos_web_main(RTIP_BOOLEAN secure_server);
#endif
RTIP_BOOLEAN check_security_request(PFBYTE ip_addr);
void    parse_mime_header(PIO_CONTEXT io_context, struct http_mime_ptrs *mime_ptrs);

//****************************************************************
extern struct server_context KS_FAR http_server_context;
#if (INCLUDE_HTTPS_SRV)
extern struct server_context KS_FAR https_server_context;
#endif

KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *web_string;

#if (INCLUDE_HTTPS_SRV)
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *https_string;
#endif

#if (!INCLUDE_MALLOC_CONTEXT)
#if (CFG_SPAWN_WEB)
extern struct _web_context KS_FAR httpcntxt[CFG_WEB_MAX_SPAWN];
#else
extern struct _web_context KS_FAR httpcntxt[1];
#endif
#endif

#if (INCLUDE_HTTPS_SRV)
#if (!INCLUDE_MALLOC_CONTEXT)
#if (CFG_SPAWN_WEB)
extern struct _web_context KS_FAR httpscntxt[CFG_WEB_MAX_SPAWN];
#else
extern struct _web_context KS_FAR httpscntxt[1];
#endif
#endif
#endif

#if (POLLOS)
extern POLLOS_TASK pollos_web_handle;
#endif

#if (INCLUDE_WEB_SECURITY)
	extern browser KS_FAR browser_ip_table[CFG_MAX_BROWSERS];
#endif

#if (INCLUDE_WEB_AUTHENTICATION)
extern struct web_auth_entry KS_FAR *web_basic_auth;
extern int KS_FAR num_web_basic_auth;
#endif

#if (INCLUDE_WEB_EXTRA_MIME)
extern PFMIME_FOR_PAGES web_mime_fields;
#endif

//****************************************************************
// SERVER DAEMON
//****************************************************************

#if (!NACT_OS)
// ********************************************************************
// INITIALIZATION
// ********************************************************************
// init_web_srv() - initialize WEB server
//   
//   Initialize WEB server at system startup
//
//   Returns nothing
//
void init_web_srv(void)
{
#if (defined(_MSC_VER))
extern void web_dummy(void);
	// dummy routine needed to compensate for a bug in linker; it avoids
	// the data in this file being unresolved
	web_dummy();
#endif

	http_server_context.master_socket = INVALID_SOCKET;

	#if (INCLUDE_WEB_SECURITY)
		tc_memset(browser_ip_table, 0, 
		          sizeof(browser) * CFG_MAX_BROWSERS);
	#endif

	#if (INCLUDE_WEB_AUTHENTICATION)
		web_basic_auth = 0;
		num_web_basic_auth = 0;
	#endif

	#if (INCLUDE_WEB_EXTRA_MIME)
		web_mime_fields = (PFMIME_FOR_PAGES)0;
	#endif
}

#if (INCLUDE_MALLOC_CONTEXT)
//****************************************************************
PWEB_CONTEXT web_alloc_context(RTIP_BOOLEAN secure_server)
{
PWEB_CONTEXT cntxt;

    cntxt = (PWEB_CONTEXT)ks_malloc(1, sizeof(struct _web_context),
    									WEB_SRV_MALLOC);
	if (!cntxt)
		return(0);
	INIT_WEB_CONTEXT(cntxt)

#if (INCLUDE_HTTPS_SRV)
    if (secure_server)
    {
        https_server_context.root_prot_context =
    		os_list_add_rear_off(https_server_context.root_prot_context,
    	                         (POS_LIST)cntxt, ZERO_OFFSET);
    }
    else
#endif
    {
    	http_server_context.root_prot_context =
    		os_list_add_rear_off(http_server_context.root_prot_context,
    	                         (POS_LIST)cntxt, ZERO_OFFSET);
    }
	return(cntxt);
}

void web_free_context(PWEB_CONTEXT cntxt)
{
#if (INCLUDE_HTTPS_SRV)
    if (cntxt->server_flags & SECURE_FLAG)
    {
        https_server_context.root_prot_context =
		    os_list_remove_off(https_server_context.root_prot_context,
						       (POS_LIST)cntxt,
		                       ZERO_OFFSET);
	    FREE_WITH_LIMIT(cntxt, https_server_context.cntxt_size,
					    total_server_threads)
    }
    else
#endif
    {
        http_server_context.root_prot_context =
		    os_list_remove_off(http_server_context.root_prot_context,
						       (POS_LIST)cntxt,
		                       ZERO_OFFSET);
	    FREE_WITH_LIMIT(cntxt, http_server_context.cntxt_size,
					    total_server_threads)
    }
}
#endif


#if (INCLUDE_HTTPS_SRV)
//****************************************************************
// http_server_daemon() - WEB server daemon
//
// Summary:
//   #include "webapi.h"
//
//   int http_server_daemon()
//
// Description:
//   
//   WEB server daemon to process incoming WEB requests.
//
//   If CFG_SPAWN_WEB is set and running a multitasking kernel (i.e.
//   not POLLOS), a task (_tc_spawn in tasks.c) is spawned to process 
//   each connection.  The spawned task will call http_server to process 
//   the request.  This routine will return only if an error is
//   detected.
// 
//   If CFG_SPAWN_WEB is not set and running a multitasking kernel (i.e.
//   not POLLOS), this routine will loop forever processing WEB 
//   requests.  It will return only if an error is detected.
//
//   If POLLOS, this routine will perform the necessary setup to allow
//   the RTIP state machine (see xn_pollos_cycle) to process incoming
//   WEB requests.
//
//   See WEB Manual for more details.
//
//   Returns 0 if successful or -1 if an error occurs.

int https_server_daemon(void)
{
byte ip_addr[IP_ALEN];

	tc_mv4(ip_addr, ip_nulladdr, IP_ALEN);
    return (_web_server_daemon(TRUE, ip_addr));
}

int https_server_daemon_interface(int iface_no)
{
byte ip_bind_addr[IP_ALEN];
PIFACE pi;

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
	if (!pi)
		return(-1);

    return (_web_server_daemon(TRUE, pi->addr.my_ip_addr));
}
#endif

//****************************************************************
// http_server_daemon()  - WEB server daemon
//
// Summary:
//   #include "webapi.h"
//
//   int http_server_daemon()
//
// Description:
//
//   WEB server daemon to process incoming WEB requests.
//
//   If CFG_SPAWN_WEB is set and running a multitasking kernel (i.e.
//   not POLLOS), a task (_tc_spawn in tasks.c) is spawned to process
//   each connection.  The spawned task will call http_server to process
//   the request.  This routine will return only if an error is
//   detected.
//
//   If CFG_SPAWN_WEB is not set and running a multitasking kernel (i.e.
//   not POLLOS), this routine will loop forever processing WEB
//   requests.  It will return only if an error is detected.
//
//   If POLLOS, this routine will perform the necessary setup to allow
//   the RTIP state machine (see xn_pollos_cycle) to process incoming
//   WEB requests.
//
//   See WEB Manual for more details.
//
//   Returns 0 if successful or -1 if an error occurs.

int http_server_daemon(void)
{
byte ip_addr[IP_ALEN];

	tc_mv4(ip_addr, ip_nulladdr, IP_ALEN);
    return (_web_server_daemon(FALSE, ip_addr));
}

int http_server_daemon_interface(int iface_no)
{
PIFACE pi;

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
	if (!pi)
		return(-1);

    return (_web_server_daemon(FALSE, pi->addr.my_ip_addr));
}

//****************************************************************
// _web_server_daemon() - WEB server daemon
//
// Summary:
//   #include "webapi.h"
//
//   int http_server_daemon()
//
// Description:
//
//   WEB server daemon to process incoming WEB requests.
//
//   If CFG_SPAWN_WEB is set and running a multitasking kernel (i.e.
//   not POLLOS), a task (_tc_spawn in tasks.c) is spawned to process
//   each connection.  The spawned task will call http_server to process
//   the request.  This routine will return only if an error is
//   detected.
//
//   If CFG_SPAWN_WEB is not set and running a multitasking kernel (i.e.
//   not POLLOS), this routine will loop forever processing WEB
//   requests.  It will return only if an error is detected.
//
//   If POLLOS, this routine will perform the necessary setup to allow
//   the RTIP state machine (see xn_pollos_cycle) to process incoming
//   WEB requests.
//
//   See WEB Manual for more details.
//
//   Returns 0 if successful or -1 if an error occurs.

int _web_server_daemon(RTIP_BOOLEAN secure_server, PFBYTE ip_bind_addr)
{
#if (!INCLUDE_MALLOC_CONTEXT)
int i;
#if (CFG_SPAWN_WEB)
int num_loop = CFG_WEB_MAX_SPAWN;
#else
int num_loop = 1;
#endif
#endif
#if (!CFG_SPAWN_WEB)
PWEB_CONTEXT cntxt;
#endif
PSERVER_CONTEXT web_server_context;

#if (RTKERNEL32)
    RTEVALCHECK(6, 0x67afde12);
#endif

#if (INCLUDE_HTTPS_SRV)
    if (secure_server)
    {
        /* begin initialization of secure server context    */
        web_server_context                = &https_server_context;
        web_server_context->server_port   = HTTPS_PORT;
        tc_strcpy(web_server_context->task_name, https_string);
        web_server_context->task_class    = TASKCLASS_HTTPS_SERVER;
        web_server_context->server_flags |= SECURE_FLAG;
        web_server_context->SSLinitialized= FALSE;
        web_server_context->SSLmode       = ( /* ENABLE_RTSSLv2 | */ ENABLE_RTSSLv3 /* | ENABLE_RTTLSv1 */ );
        web_server_context->SSLverify     = RTSSL_VERIFY_NONE;
        web_server_context->option_off    = SO_NAGLE;
#if (CFG_SPAWN_WEB || POLLOS)
    	web_server_context->server_prot   = https_process_spawn;
#endif
#if (POLLOS)
	    pollos_web_handle.func            = pollos_https_main;
#endif
    }
    else
    {
#else
	ARGSUSED_INT(secure_server)
#endif

        /* begin initialization of web server context       */
    	web_server_context                = &http_server_context;
    	web_server_context->server_port   = WWW_PORT;
    	tc_strcpy(web_server_context->task_name, web_string);
    	web_server_context->task_class    = TASKCLASS_WEB_SERVER;
    	web_server_context->option_off    = 0;
#if (CFG_SPAWN_WEB || POLLOS)
    	web_server_context->server_prot   = http_process_spawn;
#endif
#if (POLLOS)
        pollos_web_handle.func            = pollos_http_main;
#endif
#if (INCLUDE_HTTPS_SRV)
    }
#endif

#if (USE_PKT_API)
	web_server_context->option_on     = SO_TCP_NO_COPY;
#else
	web_server_context->option_on     = 0;
#endif
	web_server_context->bind_addr     = ip_bind_addr;  
	web_server_context->backlog       = CFG_WEB_LISTEN_BACKLOG;

#if (!INCLUDE_MALLOC_CONTEXT)
	/* Save how many blocks are availabe (this is a temporary solution) */
	web_server_context->n_contexts_available = num_loop;
#else
	web_server_context->root_prot_context = (POS_LIST)0;
	web_server_context->cntxt_size = sizeof(struct _web_context);
#endif

	// initialize all sockets
	web_server_context->master_socket = INVALID_SOCKET;
#if (!INCLUDE_MALLOC_CONTEXT)
	for (i = 0; i < num_loop; i++)
	{
#if (INCLUDE_HTTPS_SRV)
	    if (secure_server)
		    INIT_WEB_CONTEXT(&httpscntxt[i])
		else
#endif
		    INIT_WEB_CONTEXT(&httpcntxt[i])
		/* have a map of available contexts we can allocate from */
		web_server_context->cntxt_avail_map[i] = 1;
	}
#endif

#if (POLLOS)
	// schedule pollos_web_handle to run (by poll_os_cycle)
	pollos_task_add((PPOLLOS_TASK)&pollos_web_handle);
#endif

#if (CFG_SPAWN_WEB)
	// call server deamon in servers.c
	return(server_daemon(web_server_context));

#else		
	// DONT SPAWN WEB
	// set-up for WEB connections then loop forever processing requests;
	// returns only if there is an error
	if (server_process_init(web_server_context) == 0)
	{
		#if (INCLUDE_MALLOC_CONTEXT)
			cntxt = web_alloc_context(secure_server);
			if (!cntxt)
				return(-1);
		#else
		    #if (INCLUDE_HTTPS_SRV)
		    if (secure_server)
		        cntxt = &(httpscntxt[0]);
		    else
		    #endif
			    cntxt = &(httpcntxt[0]);
		#endif

#if (POLLOS)
		_http_server_process_one(cntxt);
		web_free_context(cntxt);
		return(0);
#else
		for ( ; ; )
		{
			if (server_accept_one(web_server_context,
         	                      (PFINT)&(cntxt->web_socket)) != 0)
				break;
		    http_server(cntxt->web_socket, cntxt);
		}

#if (INCLUDE_MALLOC_CONTEXT)
		web_free_context(cntxt);
#endif

		// only returns if there is an error
    	net_closesocket(web_server_context->master_socket);
		return(-1);
#endif
	}
#endif		// CFG_SPAWN_WEB
 return(-1);
}

#if (CFG_SPAWN_WEB || POLLOS)
//****************************************************************
RTIP_BOOLEAN http_process_spawn(int socket_no, PROT_CTXT_OR_INDEX proto_cntxt)
{
RTIP_BOOLEAN secure_server = FALSE;

    return (web_process_spawn(socket_no, proto_cntxt, secure_server));
}

#if (INCLUDE_HTTPS_SRV)
RTIP_BOOLEAN https_process_spawn(int socket_no, PROT_CTXT_OR_INDEX proto_cntxt)
{
RTIP_BOOLEAN secure_server = TRUE;

    return (web_process_spawn(socket_no, proto_cntxt, secure_server));
}
#endif

//****************************************************************
// web_process_spawn() - process request
//                     - TOP level called by spawned task via
//                       wrapper function http_process_spawn
//                       or https_process_spawn if INCLUDE_HTTPS_SRV
//
//   Returns TRUE if done with session or FALSE if not done
//

RTIP_BOOLEAN web_process_spawn(int socket_no, PROT_CTXT_OR_INDEX proto_index, 
							   RTIP_BOOLEAN secure_server)
{
#if (INCLUDE_MALLOC_CONTEXT || !INCLUDE_HTTPS_SRV)
	ARGSUSED_INT(secure_server)
#endif

#if (!INCLUDE_MALLOC_CONTEXT)
#if (INCLUDE_HTTPS_SRV)
    if (secure_server)
        http_server(socket_no, (PWEB_CONTEXT)&(httpscntxt[proto_index]));
    else
#endif
	    http_server(socket_no, (PWEB_CONTEXT)&(httpcntxt[proto_index]));
#else
	INIT_WEB_CONTEXT((PWEB_CONTEXT)proto_index)

	http_server(socket_no, (PWEB_CONTEXT)proto_index);

	// free context malloced in servers.c
	web_free_context((PWEB_CONTEXT)proto_index);
#endif

#if (DEBUG_WEB)
	web_count--;
	DEBUG_ERROR("tc_spawn_ - done - call ks_exit task - task = ", 
		EBS_INT1, i, 0);
#endif

	// always done
	return(TRUE);
}
#endif

#if (!CFG_SPAWN_WEB)
// ********************************************************************
// http_server_init() - initialize WEB server daemon
//
// Summary:
//
//   #include "webapi.h"
//
//   int http_server_init(void)
//
// Description:
//   Initializes WEB server daemon to process incoming WEB requests.
//
//   The routine http_server_process_one() may be called to process the
//   web requests after calling this routine.  The daemon can be 
//   stopped by calling http_kill_server_daemon().
//
//   See WEB Manual for more details.
//
// Returns:
//   Returns 0 if successful or -1 if an error occurs

int http_server_init(void)
{
    /* begin initialization of web server context       */
	http_server_context.server_port   = WWW_PORT;
	tc_strcpy(http_server_context.task_name, web_string);

	http_server_context.backlog       = CFG_WEB_LISTEN_BACKLOG;
#if (CFG_SPAWN_WEB || POLLOS)
	http_server_context.server_prot   = http_process_spawn;
#endif
#if (INCLUDE_MALLOC_CONTEXT)
	http_server_context.cntxt_size = sizeof(struct _web_context);
#endif

	return(0);
}

#if (INCLUDE_HTTPS_SRV)
// ********************************************************************
// https_server_init() - initialize HTTPS server daemon
//
// Summary:
//
//   #include "webapi.h"
//
//   int https_server_init(void)
//
// Description:
//   Initializes HTTPS server daemon to process incoming HTTPS requests.
//
//   The routine http_server_process_one() may be called to process the
//   web requests after calling this routine.  The daemon can be
//   stopped by calling http_kill_server_daemon().
//
//   See WEB Manual for more details.
//
// Returns:
//   Returns 0 if successful or -1 if an error occurs
//
int https_server_init(void)
{
    /* begin initialization of secure server context    */
    https_server_context.server_port   = HTTPS_PORT;
    tc_strcpy(https_server_context.task_name, https_string);
    https_server_context.SSLinitialized= FALSE;
    https_server_context.server_flags |= SECURE_FLAG;
    https_server_context.SSLmode       = ( /* ENABLE_RTSSLv2 | */ ENABLE_RTSSLv3 /* | ENABLE_RTTLSv1 */ );
    https_server_context.SSLverify     = RTSSL_VERIFY_NONE;

	https_server_context.backlog       = CFG_WEB_LISTEN_BACKLOG;
#if (CFG_SPAWN_WEB || POLLOS)
	https_server_context.server_prot   = https_process_spawn;
#endif
#if (INCLUDE_MALLOC_CONTEXT)
	https_server_context.cntxt_size    = sizeof(struct _web_context);
#endif

	return(0);
}
#endif  // INCLUDE_HTTPS_SRV

// ********************************************************************
// _http_server_process_one() - process one WEB server request
//
// Summary:
//
//   #include "webapi.h"
//
//   int _http_server_process_one(PWEB_CONTEXT ctx)
//
// Description:
//   Processes one WEB server request.  If a request is not pending,
//   http_server_process_one returns immediately.
//
//   The WEB server daemon must be initialized by calling http_server_init()
//   prior to calling this routine.  The daemon can be stopped by calling
//   http_kill_server_daemon().
//
//   See WEB Manual for more details.
//
// Returns:
//   Returns 0 if successful (no request pending or one request 
//   processed successfully) or -1 if an error occurs

int _http_server_process_one(PWEB_CONTEXT ctx, unsigned int secure)
{
PSERVER_CONTEXT web_server_context;
int ret_val = 0;

#if (INCLUDE_HTTPS_SRV)
    if (secure)
        web_server_context = &https_server_context;
    else
#endif
        web_server_context = &http_server_context;

	// poll to see if a request is pending
	if (do_read_select(web_server_context->master_socket, 0))
	{
		ret_val = server_accept_one(web_server_context,
                                    (PFINT)&(ctx->web_socket));
		if (ret_val == 0)
		    http_server(ctx->web_socket, ctx);
	}
	return(ret_val);
}

int http_server_process_one(void)
{
PWEB_CONTEXT ctx;
unsigned int secure = 0;

#if (INCLUDE_MALLOC_CONTEXT)
	ctx = (PWEB_CONTEXT)http_server_context.root_prot_context;
#else
	ctx = &httpcntxt[0];
#endif
    return(_http_server_process_one(ctx, secure));
}

#if (INCLUDE_HTTPS_SRV)
int https_server_process_one(void)
{
PWEB_CONTEXT ctx;
unsigned int secure = 1;

#if (INCLUDE_MALLOC_CONTEXT)
	ctx = (PWEB_CONTEXT)https_server_context.root_prot_context;
#else
	ctx = &httpscntxt[0];
#endif
    return(_http_server_process_one(ctx, secure));
}
#endif
#endif		// !CFG_SPAWN_WEB


#if (INCLUDE_HTTPS_SRV)
//****************************************************************
//   web_kill_server_daemon is a helper function
//   to help leave the api consistant
int web_kill_server_daemon(RTIP_BOOLEAN secure_server);

//****************************************************************
// https_kill_server_daemon() - HTTPS kill server daemon
//
// Summary:
//
//   #include "webapi.h"
//
//   int https_kill_server_daemon(void)
//
// Description:
//   Kills the HTTPS server daemon started by http_server_daemon().
//
//   See WEB Manual for more details.
//
// Returns:
//   Returns 0 if successful or -1 if an error occurs

int https_kill_server_daemon(void)
{
RTIP_BOOLEAN https = TRUE;
    return (web_kill_server_daemon(https));
}

//****************************************************************
// http_kill_server_daemon() - WEB kill server daemon
//
// Summary:
//
//   #include "webapi.h"
//
//   int http_kill_server_daemon(void)
//
// Description:
//   Kills the WEB server daemon started by http_server_daemon().
//
//   See WEB Manual for more details.
//
// Returns:
//   Returns 0 if successful or -1 if an error occurs

int http_kill_server_daemon(void)
{
RTIP_BOOLEAN https = FALSE;
    return (web_kill_server_daemon(https));
}

int web_kill_server_daemon(RTIP_BOOLEAN secure_server)
#else
//****************************************************************
// http_kill_server_daemon() - WEB kill server daemon
//
// Summary:
//
//   #include "webapi.h"
//
//   int http_kill_server_daemon(void) 
//
// Description:
//   Kills the WEB server daemon started by tlenet_server_daemon().
//
//   See WEB Manual for more details.
//
// Returns:
//   Returns 0 if successful or -1 if an error occurs

int http_kill_server_daemon(void) 
#endif  // INCLUDE_HTTPS_SRV
{
#if (!INCLUDE_MALLOC_CONTEXT)
#if (CFG_SPAWN_WEB)
int num_loop = CFG_WEB_MAX_SPAWN;
#else
int num_loop = 1;
#endif
int i;
#else
PWEB_CONTEXT cntxt;
PWEB_CONTEXT ncntxt;
#endif
PSERVER_CONTEXT web_server_context;

#if (INCLUDE_HTTPS_SRV)
    if (secure_server)
        web_server_context = &https_server_context;
    else
#endif
        web_server_context = &http_server_context;


	if (web_server_context->master_socket != INVALID_SOCKET)
	{
		xn_abort(web_server_context->master_socket, TRUE);
		web_server_context->master_socket = INVALID_SOCKET;
	}

#if (!INCLUDE_MALLOC_CONTEXT)
#if (INCLUDE_HTTPS_SRV)
    if (secure_server)
    {
        for (i = 0; i < num_loop; i++)
    	{
    		if (httpscntxt[i].web_socket != INVALID_SOCKET)
    		{
    			xn_abort(httpscntxt[i].web_socket, TRUE);
    			httpscntxt[i].web_socket = INVALID_SOCKET;
    		}
    	}
    }
    else
#endif
    {
    	for (i = 0; i < num_loop; i++)
    	{
    		if (httpcntxt[i].web_socket != INVALID_SOCKET)
    		{
    			xn_abort(httpcntxt[i].web_socket, TRUE);
    			httpcntxt[i].web_socket = INVALID_SOCKET;
    		}
    	}
    }
#else
    cntxt = (PWEB_CONTEXT)web_server_context->root_prot_context;
    while (cntxt)
	{
		// get next context in case cntxt is freed when end session
		// by aborting the socket
		ncntxt = (PWEB_CONTEXT)
			os_list_next_entry_off(web_server_context->root_prot_context,
			                       (POS_LIST)cntxt, ZERO_OFFSET);
		if (cntxt->web_socket != INVALID_SOCKET)
		{
			xn_abort(cntxt->web_socket, TRUE);
			cntxt->web_socket = INVALID_SOCKET;
		}
		cntxt = ncntxt;
	}
#endif
	return(0);
}

#endif		// !NACT_OS

//****************************************************************
// PROCESS WEB REQUEST 
//****************************************************************

//****************************************************************
// http_server() - process a WEB request
//
// Summary:
//   #include "webapi.h"
//
//   http_server(int socket)
//
// Description:
//   Performs the following:
//     Allocates memory for I/O.
//     Identifies what kind of request has been received.
//     Passes control to the appropriate handler.
//
//   Returns nothing

void http_server(SOCKET socket, PWEB_CONTEXT web_context)	/*__fn__*/
{
PFCHAR args;
PFCHAR buffer;
PIO_CONTEXT pio_context;
struct http_mime_ptrs mime_ptrs;		
#if (INCLUDE_WEB_SECURITY)
int sin_len;
struct sockaddr_in sin;
#endif
int line_len;
long tmeout;
#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
int persist_ctr = 0;
#endif

#if (INCLUDE_WEB_SECURITY)
    sin_len = sizeof(struct sockaddr_in);
    if ( getpeername(socket, (PSOCKADDR)&sin, (PFINT)&sin_len)) 
    {
        DEBUG_ERROR("http_server: getpeername failed, socket, errno = ",
			EBS_INT2, socket, xn_getlasterror());
        xn_abort(socket, TRUE);     /* send reset and free resources */
        return;
    }

    if ( !check_security_request((PFBYTE)&(sin.sin_addr)) )
    {
        DEBUG_ERROR("REQUEST NOT IN BROWSER TABLE: ", IPADDR, 
            &(sin.sin_addr), 0);
        xn_abort(socket, TRUE);     /* send reset and free resources */
        return;
    }
#endif

	pio_context = &(web_context->io_context);
	pio_context->mime_in_ptrs = (PFVOID)&mime_ptrs;

	web_context->web_socket = pio_context->sock = socket;

	// set the data socket to non-blocking
	if (!set_non_blocking_mode(socket))
	{
		DEBUG_ERROR("http_server: set_non_blocking_mode failed",
			NOVAR, 0, 0);
	}

	if (xn_line_init(pio_context, 
                     LINE_INPUT | LINE_OUTPUT_QUE | LINE_OUTPUT_SEND))
	{
		tmeout = CFG_WEB_TIMEOUT;	// time out to wait for brower to
									// send command

    	// Now get the request from the client.
		// For WEB 1.1, handles all commands for a persistent connection
    	while ((line_len=xn_line_get(pio_context, &buffer, tmeout, 
                                     GET_LINE)) > 0) 
		{
	       	DEBUG_LOG("HTTP: {", LEVEL_3, STR2, buffer, (PFCHAR)"}");

			// save the major and minor numbers in spawn array for
			// access by other routines
			get_version(buffer, line_len, &(pio_context->major_number), 
                        &(pio_context->minor_number));

			// parse the header fields in the request (mime_ptrs is set
			// up to point to specific fields read)
			parse_mime_header(pio_context, &mime_ptrs);

#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
			persist_ctr++;
			if (persist_ctr >= CFG_WEB_KEEPALIVE_MAX)
			{
				mime_ptrs.persistent = FALSE;
			}
#endif

    	   	switch (get_command(buffer, &args)) 
	   		{
         	case GET_CMD:
				DEBUG_LOG("http_server - process GET command", LEVEL_3, 
					NOVAR, 0, 0);
           		process_get_command(pio_context, &mime_ptrs, args);
            	break;

         	case 
			
			POST_CMD:
				DEBUG_LOG("http_server - process POST command", LEVEL_3, 
					NOVAR, 0, 0);
            	process_post_command(pio_context, &mime_ptrs, args);
            	break;

#if (INCLUDE_WEB_PUT)
			case PUT_CMD:
				DEBUG_LOG("http_server - process PUT command", LEVEL_3,
					NOVAR, 0, 0);
				process_put_command(pio_context, args);
				break;
#endif // INCLUDE_WEB_PUT

			default:
				DEBUG_ERROR("http_server - illegal command = ", NOVAR, 0, 0);
            	http_send_response(pio_context, NOT_IMPLEMENTED_501);
            	break;
       		}	// end of switch
#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
			persist_ctr++;
			if (mime_ptrs.persistent == FALSE)
			{
				break;	// break out of while since persistent connection done
			}
			else
			{
				// go wait for another request from browser
				tmeout = CFG_WEB_KEEPALIVE_TMO;	// time out to wait for brower to
												// send next command before
												// closing
			}
			
#else
			break;		// break out of while
#endif
		}		// end of while

		xn_line_done(pio_context);
	}		// end of if xn_line_init


	DEBUG_LOG("http_server - close the socket", LEVEL_3, NOVAR, 0, 0);
    net_closesocket(socket);
}

#if (POLLOS)
#if (INCLUDE_HTTPS_SRV)
void pollos_https_main(void)
{
RTIP_BOOLEAN secure_server = TRUE;
    pollos_web_main(secure_server);
}
#endif
void pollos_http_main(void)
{
RTIP_BOOLEAN secure_server = FALSE;
    pollos_web_main(secure_server);
}
// ********************************************************************
// pollos_web_main - task run by poll_os_cycle
//
//   Processes all WEB requests; this task was scheduled to run by
//   the HTTP daemon.  The routine poll_os_cycle will run this
//   routine.
//
//   Returns nothing
//

void pollos_web_main(RTIP_BOOLEAN secure_server)
{
PROT_CTXT_OR_INDEX i;
PSERVER_CONTEXT web_server_context;

#if (INCLUDE_HTTPS_SRV)
    if (secure_server)
        web_server_context = &https_server_context;
    else
#endif
        web_server_context = &http_server_context;

	if (web_server_context->master_socket != INVALID_SOCKET)
	{
		#if (CFG_SPAWN_WEB)
			// start up any WEB sessions remote hosts connected to
			// NOTE: if process a session and the session is
			//       done, _tc_spawn will set the cntxt_avail_map
			//       entry back to available
			while (process_one_task(web_server_context) != -2)
				;
		#else
			#if (INCLUDE_MALLOC_CONTEXT)
				i = web_alloc_context(secure_server);
				if (!i)
					return;
			#else
				i = 0;
			#endif

			pollos_process_one(web_server_context, i);

			#if (INCLUDE_MALLOC_CONTEXT)
				web_free_context(i);
			#endif
		#endif
	}
}
#endif

#if (INCLUDE_WEB_SECURITY)
// ********************************************************************
// http_set_browser_list() - Defines the global list of acceptable browsers
//
// Summary:
//   #include "webapi.h"
//
//   int http_set_browser_list(browser_info, num_elements)
//		 dword browser_info[]  -	List of browser IP addresses/maske
//		 int num_elements	   - 	Number of elemenets to copy
//
//
// Description:
//	 This function copies the contents of the browser_info parameter 
//   into the global browser list.  This enables a simple security
//   feature for the WEB server by limiting the browsers which may
//   access the server.  If this function is calledr with IP
//   address, only requests from browsers which are in browser_info
//   will be serviced.  A reset will be sent to all browsers not
//   in the table.
//
//   Security testing will be disabled if this function is never
//   called, INCLUDE_WEB_SECURITY is set to 0,  or this function
//   is called with num_elements set to 0
//
//   The parameter browser_info need not be preserved after this 
//   function is called.
//
//   For more details see RTIP Reference Manual.
//
// Returns:
//	 -1 if RTIP not initialized or num_elements was larger than 
//   CFG_MAX_BROWSERS, 0 on success
//
//	 Upon error, xn_getlasterror() may be called to retrieve error code set by
//   this function.  For error values returned by this function see
//   RTIP Reference's Manual.
//

int http_set_browser_list(PFBROWSER browser_info, int num_elements)
{
int i;

#if (INCLUDE_RTIP && INCLUDE_ERROR_CHECKING)
	if (!rtip_initialized)
      return(set_errno(ENOTINITIALIZED));
#endif

	for (i=0; i < CFG_MAX_BROWSERS; i++)
	{
		if (i < num_elements)
		{
			tc_mv4(browser_ip_table[i].ip_addr, browser_info[i].ip_addr,
				   IP_ALEN);
			tc_mv4(browser_ip_table[i].ip_mask, browser_info[i].ip_mask,
				   IP_ALEN);
		}
		else
		{
			// set entry to invalid
			tc_mv4(browser_ip_table[i].ip_addr, ip_nulladdr,
				   IP_ALEN);
		}
	}

	// if an attempt was made to set too many browsers
	if (num_elements > CFG_MAX_BROWSERS)	
		return(set_errno(ETOOMANYSERVERS));

	return(0);
}
 
//****************************************************************
// check_security_request() - authenticate the http request
//
//   Authenticates the web request by checking the source IP address
//   in the request (the parameter ip_addr) against the table
//   of legal IP browser addresses (browser_ip_table).  The brower
//   table contains ip address, mask pairs.
//
//   Returns TRUE if the requestors IP address has permission or
//   FALSE if the IP address does not have permission.
//

RTIP_BOOLEAN check_security_request(PFBYTE ip_addr)
{
int i, j;
RTIP_BOOLEAN match;
byte mask_byte;

	for (i=0; i < CFG_MAX_BROWSERS; i++)
	{
		// check if at end of table; if table is empty then no
		// security checking should be done
		if (tc_cmp4(browser_ip_table[i].ip_addr, ip_nulladdr, IP_ALEN))
		{
			// if table empty, all addresses are ok
			if (i == 0)
				return(TRUE);
			return(FALSE);
		}

		// check current entry in table for a match
		match = TRUE;
		for (j=0; j < IP_ALEN; j++)
		{
			mask_byte = browser_ip_table[i].ip_mask[j];
			if ( (browser_ip_table[i].ip_addr[j] & mask_byte) != 
			     (ip_addr[j] & mask_byte) )
			{
				match = FALSE;
				break;
			}
		}

		// return success if this entry in the table matches
		if (match)
			return(TRUE);
	}
		
	return(FALSE);
}
#endif

#if (INCLUDE_WEB_AUTHENTICATION)
//****************************************************************
// BASIC AUTHENTICATION
//****************************************************************

//****************************************************************
// http_set_auth() - setup basic authentication information
//
// Summary:
//   #include "webapi.h"
//
//   int http_set_auth(struct web_auth_entry *web_auth, int num_entries)
//       struct web_auth_entry *web_auth - structure which contains
//                                         the authentication information
//       int num_entries                 - number of entries in web_auth
//
//   Saves pointer to basic authentication information, web_auth, in
//   the global variable; also saves the number of entries.
//
//   NOTE: the information in web_auth is not copied to internal data
//         i.e. a pointer to web_auth is saved, therefore, the information 
//         in web_auth should not be modified after this routine
//
//   Returns 0 upon success or -1 upon error
int http_set_auth(struct web_auth_entry *web_auth, int num_entries)
{
int i;
base64_encode_context  context;
PBASE64_ENCODE_CONTEXT pContext = &context;

	// save auththentication info globally
	web_basic_auth = web_auth;
	num_web_basic_auth = num_entries;

	// encode the username:password for all the entries
	for (i=0; i<num_web_basic_auth; i++)
	{
		base64_init_encode_context(pContext, 0);
		web_basic_auth[i].encoded_len = 
			base64_encode(pContext, (PFBYTE)web_basic_auth[i].user_pwd,
                          (PFBYTE)web_basic_auth[i].encoded_user_pwd, 
                          tc_strlen(web_basic_auth[i].user_pwd));
		web_basic_auth[i].encoded_len += 
			base64_encode_finish(pContext, 
                                 (PFBYTE)(web_basic_auth[i].encoded_user_pwd+
                                          web_basic_auth[i].encoded_len));
	}

	return(0);
}

/*****************************************************************        */
/* return_error_401() - authentication has failed return error 401        */
/*                                                                        */
/*   If a error 401 web page has been specified then return it, else      */
/*   return a blank page.                                                 */
/*                                                                        */
/*   Returns TRUE if an error 401 page has been specified                 */
/*   FALSE if a blank page is to be returned.                             */
/*                                                                        */
RTIP_BOOLEAN return_error_401(PIO_CONTEXT io_context, char *filename, int nPageNdx )
	{
	if( web_basic_auth[nPageNdx].err401_file )
		{
        io_context->auth_mime_header = UNAUTHORIZED_401;
        io_context->auth_realm = web_basic_auth[nPageNdx].realm;
        // replace the filename with my UNAUTHORIZED_401 page.
        tc_strcpy( filename, web_basic_auth[nPageNdx].err401_file );
        return(TRUE);
        }
    else
    	{
		struct http_mime_header mhdr;

        /* browser needs to authenticate;                          */
        /* initialize mime info - i.e. clear all information which */
        /* will be constructed; this information will be used      */
        /* to send the response back to the browser                */
        init_mime_header(&mhdr);  

        /* get the string to return with the correct version number   */
        /* NOTE: assumes OK header is send                            */
        /* NOTE: HTTP 1.0 says to send 403 here but HTTP 1.1          */
        /*       says to send 401                                     */
        HTTP_GET_RETURN_STRING(mhdr.return_hdr, 
                               UNAUTHORIZED_401); 

        mhdr.auth_realm = web_basic_auth[nPageNdx].realm;

        /* first send the MIME header   */
        send_mime_header(io_context, &mhdr);

        /* force the last send since queued data   */
        io_context->length_out = 0;
        xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);

        return(FALSE);
        }
	}

//****************************************************************
// check_authenticate_request() - authenticate the http request
//
//   Authenticates the web request by checking 
//
//   Returns TRUE if the requested file does not need to be authenticated
//   FALSE if the request needs to be authenticated
//
RTIP_BOOLEAN check_authenticate_request(PIO_CONTEXT io_context, char *filename,
                                   struct http_mime_ptrs *mime_ptrs)
{
int i;

	for (i=0; i < num_web_basic_auth; i++)
	{
		// check if requesting a web page which must be authenticated
        if (tc_stristr(filename, web_basic_auth[i].filename))
		{
			// check if request has an authentication info
			if (mime_ptrs->auth_basic_cookie_len)
			{
				// mime header contains authentication field, check
				// if it is correct
				if ( tc_comparen(web_basic_auth[i].encoded_user_pwd,
                                 mime_ptrs->auth_basic_cookie,
                                 web_basic_auth[i].encoded_len) &&
                     (mime_ptrs->auth_basic_cookie_len == 
                      web_basic_auth[i].encoded_len) )
				{
					return(TRUE);		// authentication is correct
				}
				else
				{
#if (DEBUG_AUTHENTICATION)
					DEBUG_ERROR("bad autherization", PKT, 
						web_basic_auth[i].encoded_user_pwd, web_basic_auth[i].encoded_len);
					DEBUG_ERROR("bad autherization", PKT, 
						mime_ptrs->auth_basic_cookie, web_basic_auth[i].encoded_len);
//					decode_base64_msg(mime_ptrs->auth_basic_cookie, 
//                                      web_basic_auth[i].encoded_len);
					DEBUG_ERROR("len from browser, len from api = ", EBS_INT2,
					    mime_ptrs->auth_basic_cookie_len, web_basic_auth[i].encoded_len);
#endif

					return return_error_401( io_context, filename, i );
				}
			}
			return return_error_401( io_context, filename, i );
		}
	}
	return(TRUE);
}
#endif 	// INCLUDE_WEB_AUTHENTICATION


// ********************************************************************
// http_send_response() - Sends a response to the remote host.
//
// Summary:
//   #include "webapi.h"
//
//   http_send_response(int sock, int error)
//
// Description:
//   Performs the following:
//     Sends text over HTTP connection notifying remote client that
//       an error has occurred or that request was satisfied.
//

void http_send_response(PIO_CONTEXT io_context, int error) 
{
    DEBUG_ERROR("HTTP: ", STR1, (PFCHAR)&http_return_strings[error][0], 0);

	// get the string with the correct version number
	HTTP_GET_RETURN_STRING((PFCHAR)io_context->pb_out, error); 

	io_context->offset_out = tc_strlen((PFCHAR)io_context->pb_out);
    xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_SEND);
}


//****************************************************************
// FIND_STRING_VALUE
//****************************************************************
// This function will look in buffer for a string of the form:
// match=xx with the following before the string:
// &,\0,\n
// This will ensure that Hits does not match ResetHits, for
// example.

PFCHAR http_find_string_value(PFCHAR buffer, PFCHAR match, int got) 
{
PFCHAR val;
RTIP_BOOLEAN begin_string_ok;

     // Is there a match at all?
     if ((val=tc_strstr(buffer, match)) == 0) 
	 	return 0;

     do 
	 {
        // yes, some sort of match was found.
        // Now find out if it has and = sign after and
        // one of the three chars before.

		begin_string_ok = FALSE;

        // Is the string at the beginning of the buffer ok?
        if (val == buffer) 
			begin_string_ok = TRUE;
        else if (check_char(val-1)) 
			begin_string_ok = TRUE;

        if (begin_string_ok) 
		{
        	// Now check to see if the end of the string has an = sign on it.
			// If it does, return pointer to the char following the = sign
           	if (*(val+tc_strlen(match)) == '=') 
				return val+1+tc_strlen(match);
        }

        // Match not found, look at the next part of the string.
        if ((val) && ((val-buffer) < got)) 
			val++;
     } while ( (val=tc_strstr(val, match)) !=0 );

     return 0;

}

#if (INCLUDE_WEB_EXTRA_MIME)
//****************************************************************
// EXTRA MIME FIELDS
//****************************************************************
//****************************************************************
// http_mime_fields() - Mime fields to be sent to browser
//
// Summary:
//   #include "webapi.h"
//
//   void http_mime_fields(web_fields)
//      PFMIME_FOR_PAGES web_fields - pointer to mime field information
//
// Description:
//   
//   Passes extra mime fields to be sent to browser in response to 
//   GET requests for specified files.
//
//   NOTE: list of web pages/mime fields needs to be null terminated,
//         i.e. web page for last entry should be 0
//
//   One possible use of this command is to send caching information
//   to browser.
//
//   See WEB Manual for more details.
//
//   Returns nothing

void http_mime_fields(PFMIME_FOR_PAGES web_fields)
{
	// save pointer to user structure
	web_mime_fields = web_fields;
}

#endif

#if (INCLUDE_WEB_PUSH)
//****************************************************************
// SERVER PUSH
//****************************************************************
// ********************************************************************
// Server Push Package
//
// Server push is a method of using MIME (RFC 822, RFC 1521, tbd - add more)
// An internet mail header protocol that has been extended to HTTP,
// to send multiple chunks of data that may append to or replace data that
// was sent previously.  A single TCP connection is kept open the whole time,
// and the server has complete control over what data and when it sends it.

// This is the string that is used to specify the boundary.  It should
// be extremely unlikely to appear in the text of the files.;
// This is a random string
// tbd - implement method for user to change this string dynamically
#define BOUNDARYSTR 	"0plm(Micro-Web:Server-Push:Boundary-String)1qaz"

// This is the header that makes this whole thing into server push.  This
// should not be changed, except in the event that "x-mixed-replace" is
// upgraded from experimental status, in which case the "x-" would be removed.
#define CONTENT_TYPE_HEADER \
	"multipart/x-mixed-replace;boundary=" BOUNDARYSTR 

// These defines construct the actual boundaries.  This should not be changed.
#define BOUNDARY 		"\n--" BOUNDARYSTR "\n"
#define ENDSTRING 		"\n--" BOUNDARYSTR "--\n"
#define CTSTRING 		"Content-type: " CONTENTTYPE "\n\n"

// This should be called before any data is sent.  This send the first
// mime header, specifying server push.
int http_begin_push(PIO_CONTEXT io_context)
{
struct http_mime_header mpmixedhdr;

	// Initialize the mime header with no information
	init_mime_header(&mpmixedhdr);

	// Put the content type into the header.  This makes it server push.
	tc_strcpy(mpmixedhdr.content_type, CONTENT_TYPE_HEADER);

	// send the mime header, returning error if the connection has closed.
	return send_mime_header(io_context, &mpmixedhdr);
}

int http_push_data(PIO_CONTEXT io_context, PFCHAR data, int length, char *content_type)
{
struct http_mime_header parthdr;
int retval;
#if (INCLUDE_RTIP)
PANYPORT pport;
#endif

#if (INCLUDE_RTIP)
	// if the remote side closed, server push should stop
	// NOTE: on NETSCAPE if you hit stop, if it is in the middle of downloading
	//       a file it will send a reset but if a server push is in progress
	//       it will send a FIN; it is legal to continue to send data after
	//       receipt of a FIN (and NETSCAPE will continue acking it) 
	//       but we want to stop in this case
	pport = sock_to_port(io_context->sock);
	if ( ((PTCPPORT)(pport))->state == TCP_S_CWAIT )
	{
		DEBUG_ERROR("http_push_data - state is CWAIT", NOVAR, 0, 0);
		return(-1);
	}
#endif

	// Queue the boundary, telling them we are sending a sub-document
	tc_strcpy(io_context->buffer_out, (PFCHAR)BOUNDARY);
	io_context->length_out = tc_strlen(BOUNDARY), 
	retval = xn_line_put((PIO_CONTEXT)io_context, 
                         CFG_WEB_TIMEOUT, PUT_QUE);
	if (retval)
	{
		DEBUG_ERROR("http_push_data - send boundary out", NOVAR, 0, 0);
		return retval;
	}

	// This is the "sub-header" used for each chunk that is sent in the
	// server push (it is a regular mime header, with the proper content type)
	init_mime_header(&parthdr);

	// This puts the right content type in the header.
	tc_strcpy(parthdr.content_type, content_type);

	// send the header
	retval = send_mime_header(io_context, &parthdr);
	if (retval)
	{
		DEBUG_ERROR("http_push_data - state mime header timed out", NOVAR, 0, 0);
		return retval;
	}

	// Send the actual chunk of data, and return the result status.
	tc_strcpy(io_context->buffer_out, (PFCHAR)data);
	io_context->length_out = length;
	return xn_line_put((PIO_CONTEXT)io_context, CFG_WEB_TIMEOUT, 
                       PUT_QUE | PUT_SEND);
}

// send a final boundary with an extra "--\r\n" at the end, telling them we
// have sent the last chunk of data, and have no more to send.
// THis is the last thing you do before closing the connection.
int http_end_push(PIO_CONTEXT io_context)
{
	tc_strcpy(io_context->buffer_out, (PFCHAR)ENDSTRING);
	io_context->length_out = tc_strlen(ENDSTRING);
	return xn_line_put((PIO_CONTEXT)io_context, CFG_WEB_TIMEOUT,
                       PUT_QUE | PUT_SEND);
}
#endif // INCLUDE_WEB_PUSH
#endif // INCLUDE_WEB

