/*                                                                      */
/* TNSAPI.H - Defines & prototypes for Telnet server                    */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1998                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __TNSAPI_H__
#define __TNSAPI_H__

/* *********************************************************************   */
/* ******                CONFIGURATION                            ******   */
/* *********************************************************************   */
#if (INCLUDE_RUN_TIME_CONFIG)
typedef struct cfg_tns_data
{
    int tns_listen_backlog;
    int tns_read_tmo;
    int tns_write_tmo;
} CFG_TNS_DATA;

extern CFG_TNS_DATA KS_FAR cfg_tns_data;

#define CFG_TNS_LISTEN_BACKLOG  cfg_tns_data.tns_listen_backlog
#define CFG_TNS_READ_TMO        cfg_tns_data.tns_read_tmo
#define CFG_TNS_WRITE_TMO       cfg_tns_data.tns_write_tmo
#else
#define CFG_TNS_LISTEN_BACKLOG  _CFG_TNS_LISTEN_BACKLOG 
#define CFG_TNS_READ_TMO        _CFG_TNS_READ_TMO
#define CFG_TNS_WRITE_TMO       _CFG_TNS_WRITE_TMO
#endif


/* ********************************************************************   */
/* Telnet options from rfc 855-860                                        */
#define TNOPBINARY      0
#define TNOPECHO        1
#define TNOPSUPGA       3
#define TNOPSTATUS      5
#define TNOPTIMINGMARK  6
#define MAXOPT          7

typedef struct _telnet_context
{
#if (INCLUDE_MALLOC_CONTEXT)
    OS_LIST    list1;           /* list of telnet contexts - needed to */
                                /* kill all sessions   */
#endif
    int  tns_socket;
    int  tns_options[MAXOPT];
    char tns_buf[80];
    int  tns_bp;
#if (USE_TELNET_CONSOLE)
    int  tns_rp;
#endif
} telnet_context;

typedef struct _telnet_context KS_FAR *PTELNET_CONTEXT;

/* *********************************************************************   */
/* Telnet options from rfc 855-860                                         */
#define TNOPBINARY      0
#define TNOPECHO        1
#define TNOPSUPGA       3
#define TNOPSTATUS      5
#define TNOPTIMINGMARK  6
#define MAXOPT          7

/* *********************************************************************   */
#ifdef __cplusplus
extern "C" {
#endif

/* API functions   */
void init_telnet_srv(void);
int  telnet_server_daemon(void);
int  telnet_server_daemon_interface(int iface_no);
int  telnet_kill_server_daemon(void);
int  telnet_server_init(void);
int  telnet_server_process_one(void);

#ifdef __cplusplus
}
#endif

#endif      /* __TNSAPI_H__ */

