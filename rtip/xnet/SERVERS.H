/*                                                                      */
/* SERVER.H - include file for starting servers (WEB for example)       */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __SERVER_H__
#define __SERVER_H__ 1


/* *********************************************************************   */
/* ******                CONFIGURATION                            ******   */
/* *********************************************************************   */
#if (RUN_TIME_CONFIG)
typedef struct cfg_srv_data
{
    int srv_read_tmo;
} CFG_SRV_DATA;
extern CFG_SRV_DATA KS_FAR cfg_srv_data;

#define CFG_SRV_READ_TMO        cfg_srv_data.srv_read_tmo
#else
#define CFG_SRV_READ_TMO        _CFG_SRV_READ_TMO
#endif

#if (INCLUDE_WEB || INCLUDE_FTP_SRV || INCLUDE_TELNET_SRV)

#if (!INCLUDE_MALLOC_CONTEXT)
#define PROT_CTXT_OR_INDEX int
#else
#define PROT_CTXT_OR_INDEX PFVOID
#endif

/* ********************************************************************   */
/* parameter passed to server processing routines in tasks.c              */
typedef struct server_context
{
    word      server_port;      /* port number */
    SOCKET    master_socket;    /* master socket */
    int       option_off;       /* socket option to turn off */
    int       option_on;        /* socket option to turn on */
    int       backlog;          /* backlog size for listen() */
    PFBYTE    bind_addr;        /* IP address to bind to */
    RTIP_BOOLEAN   (*server_prot)(SOCKET socket_no, 
                                  PROT_CTXT_OR_INDEX proto_index1);
    char      task_name[10];    /* name of task */
    int       task_class;       /* use this for spawning */
    /* map of available contexts we can allocate from   */
    int       n_contexts_available;
#if (INCLUDE_MALLOC_CONTEXT)
    int       cntxt_size;
    POS_LIST  root_prot_context;
#else
    char      cntxt_avail_map[CFG_MAX_CONTEXT];
#endif
    #define SECURE_FLAG 0x01
    byte      server_flags;
#if (INCLUDE_HTTPS_SRV)        /* __st__ 2002.06.10 */
    /*  Don't need to declare the structure
     *  of the following pointer, as this
     *  will be allocated by the appropriate
     *  SSL function
     */
    void        *sctx;          /* must be cast as SSL_CTX */
    RTIP_BOOLEAN SSLinitialized;
#endif
} server_context;
typedef struct server_context KS_FAR *PSERVER_CONTEXT;
#endif /* INCLUDE_WEB || INCLUDE_FTP_SRV || INCLUDE_TELNET_SRV */

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
#if (CFG_SPAWN_WEB || CFG_SPAWN_FTP || CFG_SPAWN_TELNET)
int         server_daemon(PSERVER_CONTEXT server_context);
int         process_one_task(PSERVER_CONTEXT server_context);
#endif
#if (INCLUDE_WEB || INCLUDE_FTP_SRV || INCLUDE_TELNET_SRV)
int         server_process_init(PSERVER_CONTEXT server_context);
int         server_accept_one(PSERVER_CONTEXT server_context, SOCKET *csock);
#endif      /* WEB, FTP SRV or TELNET_SRV */

#endif      /* SERVER_H */


