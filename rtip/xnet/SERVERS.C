/* SERVERS.C - RTIP's service routines for SERVERs (WEB, TELNET etc)    */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */

#include "sock.h"

#if (INCLUDE_RTIP)
#include "rtip.h"
#include "rtipext.h"
#endif
#include "servers.h"

#if (INCLUDE_WEB || INCLUDE_FTP_SRV || INCLUDE_TELNET_SRV)

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_WEB           0
#define DEBUG_CONTEXT       0
#define DEBUG_SHOW_PROGRESS 0

/* ********************************************************************   */
#if (DEBUG_WEB)
int web_accept_count = 0;
int web_count = 0;
#endif

/* ********************************************************************   */
/* FUNCTION DECLARATIONS                                                  */
void tc_spawn_(void);
void release_task(void);
#if (INCLUDE_WEB || INCLUDE_FTP_SRV || INCLUDE_TELNET_SRV)
#if (!USE_DB_L0)
void display_server_status(PSERVER_CONTEXT server_context, SOCKET csock);
#endif
#endif

/* ********************************************************************   */
/* EXTERNS                                                                */
#if (INCLUDE_HTTPS_SRV)
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *https_string;
#endif


#if (CFG_SPAWN_WEB || SPAWN_FTP || CFG_SPAWN_TELNET)
/* ********************************************************************
 *  SERVER DAEMON/SPAWN TASK STUFF
 * ********************************************************************
 *    The following routines provide a general purpose server daemon
 *    which spawns a task to process incoming requests.  When a request
 *    comes in the task is spawned through os_spawn_task which passes
 *    data containing information needed to process the request to
 *    the spawned task through the system user structure.
 *    Then a task is spawned to handle the request.  The spawned task
 *    retrieves the arguments from the user structure and calls
 *    the appropriate routine based upon info in 
 *    the server context (WEB/FTP/TELNET) to handle the request.
 *    The routine it calls depends upon which protocol the
 *    request is for (the information is in the server context structure).
 *
 *                                poll_os_cycle
 *                                      |
 *             server_daemon      pollos_telnet_main
 *                      \           /      \
 *                     process_one_task  pollos_telnet_requests
 *                           |               |
 *                       os_spawn_task       |
 *                           |               |
 *                       tc_spawn_           |
 *                           |               |
 *                       telnet_server      /
 *                           |             /
 *                       process_telnet_requests
 *
 * ********************************************************************/

/* ********************************************************************
 * server_daemon - server daemon for WEB, FTP and TELNET servers 
 *
 *   General purpose server which spawns a task to process an incoming
 *   request.  The parameter server_context specifies the port number
 *   the server binds to and the server routine to call once an accept
 *   has completed. It is also is used to keep track of server specific
 *   context blocks (specific to the protocol)
 *   The servers which use this
 *   routine are:
 *      WEB (WWW_PORT)         - Web server
 *      HTTPS (HTTPS_PORT)     - HTTPS server   
 *      FTP (FTP_CONTROL_PORT) - FTP server
 *      Telnet (TELNET_PORT)   - Telnet server
 *
 *   Returns 0 if successful or -1 if an error occurs
 * ********************************************************************/
int server_daemon(PSERVER_CONTEXT server_context)   /*__fn__*/
{
int ret_val;

#    if (INCLUDE_ORTFS)
        if (!pc_memory_init())
            return(set_errno(ERTFSINITFAIL));
#    endif

    /* call socket(), bind(), listen()   */
    if (server_process_init(server_context) < 0)
        return(-1);

    DEBUG_LOG("server_daemon: Waiting For a connection", LEVEL_3, 
         NOVAR, 0, 0);

    /* Daemon server main loop;                                         */
    /* multitasking kernels will loop forever; POLLOS will execute once */
    for (;;)     /* In POLLOS this is not infinite - see return below */
    {
        ret_val = process_one_task(server_context);
        if (ret_val == -1)
        {
            /* only returns if there is an error   */
            net_closesocket(server_context->master_socket);
            return(ret_val);
        }
    }

}

/* ********************************************************************       */
/* processes one connection request; called from server daemon and for POLLOS */
/* from poll_os_cycle                                                         */
/* results: 0  - ok or error                                                  */
/*          -1 - fatal error, end server                                      */
/*          -2 - POLLOS ONLY; no request has arrived or daemon is not running */
int process_one_task(PSERVER_CONTEXT server_context)
{
SOCKET c_socket;
#if (!INCLUDE_MALLOC_CONTEXT)
int i;
#else
PFBYTE i;
#endif

    /* if daemon is not running, return   */
    if (server_context->master_socket == INVALID_SOCKET)
        return(-2);
    
    /* wait forever until a request has arrived   */
    if (!do_read_select(server_context->master_socket, RTIP_INF))
    {
        DEBUG_ERROR("select returned error", NOVAR, 0, 0);
    }

#if (DEBUG_SHOW_PROGRESS)
    DEBUG_ERROR("Waiting For a connection", NOVAR, 0, 0)
#endif

    /* MULTASKING: put this task to sleep until a connection is requested by   */
    /*             a client.                                                   */
    /* POLLOS: Get the connection; will return right away since did select()   */
    /*         above                                                           */
    if (server_accept_one(server_context, &c_socket) < 0)
        return(-1);

    /* get a context so can process the request                  */
    /* MULTITASKING: will spawn a task to process request        */
    /* POLLOS: will call processing routine directly             */
    /* Find the index of an available server instance context    */
#if (!INCLUDE_MALLOC_CONTEXT)
    for (i=0; i < server_context->n_contexts_available; i++)
    {
        if (server_context->cntxt_avail_map[i])
            break;
    }

    if (i >= server_context->n_contexts_available)
#else
    MALLOC_WITH_LIMIT(i, server_context->cntxt_size, 
                      total_server_threads, SRV_MALLOC)
    if (!i)
#endif
    {
        xn_abort(c_socket, TRUE);

        /* no context is available; go wait for next request;            */
        /* hopefully a context will be available                         */
        /* return success even though failed; returning failure will     */
        /* kill the master socket which we don't want to do              */
        return(0);
    }

#if (DEBUG_CONTEXT)
    DEBUG_ERROR("servers.c: malloc server context ", DINT1, i, 0);
#endif

#if (!INCLUDE_MALLOC_CONTEXT)
    /* found one, set it's state to in use (i.e. not available)   */
    server_context->cntxt_avail_map[i] = 0;  
#else
    server_context->root_prot_context = 
        os_list_add_rear_off(server_context->root_prot_context,
                             (POS_LIST)i, ZERO_OFFSET);
#endif

    if (!os_spawn_task(server_context->task_class, tc_spawn_,
                       (PFVOID)(long)c_socket, (PFVOID)(long)server_context, 
                       (PFVOID)(long)i, 0))
    {
        DEBUG_ERROR("spawn_task - spawn task failed ", NOVAR, 0, 0);
        /* Set the context to the available state   */
#if (!INCLUDE_MALLOC_CONTEXT)
        server_context->cntxt_avail_map[i] = 1;
#else
#        if (DEBUG_CONTEXT)
            DEBUG_ERROR("servers.c : free context ", DINT1, i, 0);
#        endif
        FREE_WITH_LIMIT(i, server_context->cntxt_size, 
                            total_server_threads)
#endif
        xn_abort(c_socket, TRUE);
        ks_sleep(ks_ticks_p_sec()); /* delay before accepting another */
                                    /* connection   */
    }
    return(0);
}

/*****************************************************************   */
/* SPAWNED SERVER TASK                                               */
/*****************************************************************   */

/*****************************************************************             */
/* tc_spawn_() - task spawned to handle server (web/ftp/telnet) requests       */
/*                                                                             */
/*   This task is spawned to handle server requests; spawn_array contains      */
/*   information to process the request; the status of entries in              */
/*   spawn_array which are ready to be processed are set to SPAWN_IN_PROGRESS; */
/*   when the task has finished processing the request, it sets the            */
/*   status to SPAWN_DONE                                                      */

void tc_spawn_(void)
{
SOCKET c_socket;
PSERVER_CONTEXT server_context;
#if (!INCLUDE_MALLOC_CONTEXT)
int i_ctx;
#else
PFVOID i_ctx;
#endif
PSYSTEM_USER puser;

    puser = get_system_user();
    c_socket = (int)(long)puser->udata0;
    server_context = (PSERVER_CONTEXT) puser->udata1;
#if (!INCLUDE_MALLOC_CONTEXT)
    i_ctx = (int)(long)puser->udata2;
#else
    i_ctx = puser->udata2;
#endif

    /* call the server process function                              */
    /* for example: ftp_process_spawn for FTP server;                */
    /* processing routine will returns TRUE if done with session or  */
    /* FALSE if not done                                             */
    if (server_context->server_prot(c_socket, i_ctx))
    {
#if (!INCLUDE_MALLOC_CONTEXT)
        /* Set the context to the available state   */
        server_context->cntxt_avail_map[i_ctx] = 1;
#else
        /* context will be freed before server_prot() returns   */
#endif
    }
}
#endif   /* end of if CFG_SPAWN_WEB or SPAWN_FTP or CFG_SPAWN_TELNET */


/* ********************************************************************   */
/* SERVER PROCESS (USED WHEN NOT SPAWNING TASKS TO PROCESS REQUESTS)      */
/* ********************************************************************   */

/* ********************************************************************   */
/* server_process_init - server process for WEB, FTP and TELNET servers;  */
/*   General purpose server sets up to processes incoming requests.       */
/*   The parameter server_port specifies the port number                  */
/*   the server binds to.  It is also used to determine which task        */
/*   to spawn to process the request.  The servers which use this         */
/*   routine are:                                                         */
/*      WEB (WWW_PORT)         - Web server                               */
/*      FTP (FTP_CONTROL_PORT) - FTP server                               */
/*      Telnet (TELNET_PORT)   - Telnet server                            */
/*                                                                        */
/*   The parameter backlog is the backlog to use when calling listen()    */
/*                                                                        */
/*   Returns 0 if successful or -1 if an error occurs                     */

int server_process_init(PSERVER_CONTEXT server_context) /*__fn__*/
{
SOCKET msock;
struct sockaddr_in port; /* port to use. default or result of port command */
int option;
//XPX: byte inanyaddr[IP_ALEN] = {0, 0, 0, 0};

#    if (INCLUDE_ORTFS)
        if (!pc_memory_init())
            return(set_errno(ERTFSINITFAIL));
#    endif

    /* Allocate a master socket for the server; if we cannot then bail   */
    msock = socket(AF_INET, SOCK_STREAM, 0);

    /* If we cannot allocate the socket, bail.   */
    if (msock < 0)
    {
        DEBUG_ERROR("server_process_init: Could not allocate a socket in the HTTP server",
                    NOVAR, 0, 0);
        return(-1);
    }

#if (INCLUDE_HTTPS_SRV)

    /* make sure we are only initializing the   */
    /* secure information when it is needed     */
    if (server_context->server_flags & SECURE_FLAG)
    {
        if (!server_context->SSLinitialized)
        {
            /* we add the secure connection initialization here because       */
            /* both spawn and don't spawn ultimately call server_process_init */
            /* and we need the socket to be available to obtain the           */
            /* ssl structures needed to be set up           __st__ 2002.05.06 */

            server_context->sctx = vssl_srvinit();
            if (!server_context->sctx)
            {
                net_closesocket(msock);
                return(-1);
            }
            server_context->SSLinitialized = TRUE;
            DEBUG_ERROR("Initialization of secure server COMPLETE.", NOVAR, 0, 0);  /* not an error */
        }
    }

#endif

#if (INCLUDE_RTIP)
    option = 1;
    if ( setsockopt(msock, SOL_SOCKET, SO_REUSESOCK, 
                    (PFCCHAR)&option, sizeof(int)) ) 
    {
        DEBUG_ERROR("server daemon: could not set socket option: SO_REUSESOCK",
                  NOVAR, 0, 0);
        net_closesocket(msock);
        return(-1);     /* internal error if fails */
    }

    option = 1;
    if ( setsockopt(msock, SOL_SOCKET, SO_FREE_WITH_INPUT, 
                    (PFCCHAR)&option, sizeof(int)) ) 
    {
        DEBUG_ERROR("server daemon: could not set socket option: SO_FREE_WITH_INPUT",
                  NOVAR, 0, 0);
        net_closesocket(msock);
        return(-1);     /* internal error if fails */
    }

#endif

#if (INCLUDE_TELNET_SRV || INCLUDE_WEB)
    if (server_context->option_off)
    {
        option = 0;     /* turn option off */
        if ( setsockopt(msock, SOL_SOCKET, 
                        server_context->option_off, (PFCCHAR)&option, 
                        sizeof(int)) ) 
        {
            DEBUG_ERROR("server_process_init: could not set socket option",
                      NOVAR, 0, 0);
            net_closesocket(msock);
            return(-1);     /* internal error if fails */
        }
    }
#endif

#if (USE_PKT_API)
    /* set socket to NO COPY MODE   */
    if (server_context->option_on)
    {
        option = 1;  
        if ( setsockopt(msock, SOL_SOCKET, 
                        server_context->option_on, 
                        (PFCCHAR)&option, sizeof(int)) )
        {
            DEBUG_ERROR("server_process_init: set NO COPY mode failed", 
                NOVAR, 0, 0);
        }
    }
#endif

    /* let address be reused (i.e. for bind)                           */
    /* this is needed in case killed the daemon and restarted it, i.e. */
    /* there could be sockets in TWAIT state from previous connection  */
    /* which have not been returned to free list yet                   */
    option = 1;     
    if ( setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, 
                    (PFCCHAR)&option, sizeof(int)) )
    {
        DEBUG_ERROR("server_process_init: setsockopt failed - REUSEADDR", NOVAR,
            0, 0);
    }

    /* Bind it to the well-known port and INANYADDR   */
    port.sin_family = AF_INET;
    port.sin_port = hs2net((word)server_context->server_port);
    tc_mv4(&port.sin_addr, server_context->bind_addr, IP_ALEN);

    if (bind(msock, (PSOCKADDR)&port, sizeof(port)) < 0)
    {
        /* Close the socket to release the socket resource.   */
        net_closesocket(msock);
        DEBUG_ERROR("server_process_init: could not bind socket",
                    NOVAR, 0, 0);
        return(-1);
    }

    /* Listen for any connection attempts and allocate a backlog of    */
    /* slave ports ifor other HTTP requests.                           */
    if (listen(msock, server_context->backlog) == -1)
    {
        net_closesocket(msock);
        DEBUG_ERROR("server_process_init: could not listen on the socket",
              NOVAR, 0, 0);
        return(-1);
    }

    DEBUG_LOG("ftp server_process_init: Waiting For a connection", LEVEL_3, 
         NOVAR, 0, 0);

    server_context->master_socket = msock;

    return(0);
}

/* ********************************************************************    */
/* server_accept_one - server process for WEB, FTP and TELNET servers;     */
/*   General purpose server which accepts one incoming request             */
/*   (i.e. connection for FTP)                                             */
/*   The parameter server_port specifies the port number the server binds  */
/*   to.  The servers which use this routine are:                          */
/*      WEB (WWW_PORT)         - Web server                                */
/*      FTP (FTP_CONTROL_PORT) - FTP server                                */
/*      Telnet (TELNET_PORT)   - Telnet server                             */
/*                                                                         */
/*   Returns 0 if successful or -1 if an error occurs                      */

int server_accept_one(PSERVER_CONTEXT server_context, SOCKET *csock)    /*__fn__*/
{
#if (INCLUDE_HTTPS_SRV)
PTCPPORT sport;
int blocking;
#endif
struct sockaddr_in port; /* port to use. default or result of port command */
int sin_len;

    /* Wait for a request from client i.e.                       */
    /* put this task to sleep until a connection is requested by */
    /* a client.                                                 */
    port.sin_family = AF_INET;
    sin_len = sizeof(port);
    if ( (*csock = accept(server_context->master_socket, (PSOCKADDR)&port, &sin_len)) < 0)
    {
#        if (INCLUDE_ERRNO_STR)
            DEBUG_ERROR("server_accept_one: Accept failed - errno = ",
                    STR1, xn_geterror_string(GETLASTERROR()), 0);
#        else
            DEBUG_ERROR("server_accept_one: Accept failed - errno = ",
                    EBS_INT1, GETLASTERROR(), 0);
#        endif
        return(-1);
    }

#    if (!USE_DB_L0)
        display_server_status(server_context, *csock);
#    endif      /* USE_DB_L0 */

#if (INCLUDE_HTTPS_SRV)
    if (server_context->server_flags & SECURE_FLAG)
    {
        sport = (PTCPPORT)api_sock_to_port(*csock);
        if (!sport)
        {
            net_closesocket(*csock);
            return (-1);
        }

        if (!do_read_select(*csock, CFG_SRV_READ_TMO))
        {
            DEBUG_ERROR("server_accept_one: read select failed.", NOVAR, 0, 0);
            net_closesocket(*csock);
            return (-1);
        }

        if (!(blocking = net_is_blocking(*csock)))
            net_set_blocking(*csock);

        /* must use secure context from server_context because the TCPPORT                        */
        /* doesn't know about the secure context until net_set_socket_secure()                    */
        /* will return -1 if vssl_api has not been initialized yet or vssl_saccept was not set up */
        if ( vssl_saccept(*csock,server_context->sctx,(void **)&sport->ssl) < 0 )
        {
            DEBUG_ERROR("server_accept_one: secure accept failed.", NOVAR, 0, 0);
            net_closesocket(*csock);
            return(-1);
        }

        if (!blocking)
            net_set_non_blocking(*csock);

        /* set socket to SECURE SOCKET MODE   */
        if (!net_set_socket_secure(*csock, server_context->sctx))
        {
            DEBUG_ERROR("server_accept_one: net_set_socket_secure failed.", NOVAR, 0, 0);
            net_closesocket(*csock);
            return (-1);
        }
    }
#endif

#if (DEBUG_WEB)
    web_accept_count++;
    DEBUG_ERROR("server_daemon: Connection established - socket, #accepts = ", 
        EBS_INT2, csock, web_accept_count);
#endif

    return(0);
}

#endif  /* INCLUDE_WEB || INCLUDE_FTP_SRV || INCLUDE_TELNET_SRV */

#if (INCLUDE_WEB || INCLUDE_FTP_SRV || INCLUDE_TELNET_SRV)
#if (!USE_DB_L0)
/* ********************************************************************   */
/* DEBUG UTILITY                                                          */
/* ********************************************************************   */
void display_server_status(PSERVER_CONTEXT server_context, SOCKET csock)
{
int sin_len;
struct sockaddr_in sin;
char str[30];

    tc_strcpy(str, server_context->task_name);
    tc_strcat(str, " Connection established");
#if (DEBUG_SHOW_PROGRESS)
    DEBUG_ERROR(str, NOVAR, 0, 0);
#endif

    sin_len = sizeof(struct sockaddr_in);
    getpeername(csock, (PSOCKADDR)&sin, (PFINT)&sin_len);

    tc_strcpy(str, server_context->task_name);
    tc_strcat(str, "REQUEST FROM ");

#if (!CFG_UNIX_COMPLIANT_SOCK_STRUCTS)
    DEBUG_ERROR(str, IPADDR, &(sin.sin_addr), 0);
#else
    DEBUG_ERROR(str, IPADDR, &(sin.sin_addr.s_addr), 0);
#endif
}
#endif      /* USE_DB_L0 */
#endif      /* INCLUDE_WEB || INCLUDE_FTP || INCLUDE_TELNET */


