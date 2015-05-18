/*                                                                        */
/* RIP.H  - RIP header file                                               */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains header file for RIP                          */

#ifndef __RIP_INCLUDED__
#define __RIP_INCLUDED__

#include "rtip.h"
#include "socket.h"

/* *********************************************************************   */
#define DEBUG_RIP          0    /* turn on debug messages */

#define RIP_MAX_PKTLEN  512     /* max allowed RIP message length in bytes */
#define RIP_MAX_ENTRIES 25      /* max allowed route entries per RIP message */
#define RIP_PORT        520     /* the RIP UDP port */
#define RIP_AF_AUTH     0xFFFF  /* family used to indicate authentication */
#define RIP_TTL         180     /* rt table entries time out after 180 sec */
#define RIP_GARBAGE_TTL 120     /* expired rt table entries are purged from */
                                /*  memory after this many seconds   */
#define RIP_REQUEST  1
#define RIP_RESPONSE 2

/* *********************************************************************   */
/* format of RIP packet on the wire - a packet consists of a header        */
/* (struct rip_hdr) followed by multiple (at least 1) sections of          */
/* data (struct rip_data)                                                  */
typedef struct rip_hdr                                      
{
    byte        rp_command;     /* type of RIP packet this is (RIP_REQUEST  */
                                /*  or RIP_RESPONSE)   */
    byte        rp_version;     /* RIP version: 1 or 2 */
    word        rp_res;         /* must be 0 */
} RIPHDR;
typedef RIPHDR KS_FAR *PRIPHDR;

/* This struct is used to frame both RIP version 1 and 2 messages    */
/*  (for version 1, the rp_next, rp_mask, and rp_tag are ignored)    */

typedef struct rip_data
{
    word        rp_family;           /* Address type (AF_INET) */
    word        rp_tag;              /* Route Tag (version 2 only) */
    byte        rp_addr[IP_ALEN];    /* destination address */
    byte        rp_mask[IP_ALEN];    /* subnet mask (version 2 only) */
    byte        rp_next[IP_ALEN];    /* next hop address (version 2 only) */
    dword       rp_metric;           /* hops (or cost) estimated to destination */
} RIPDATA;
typedef RIPDATA KS_FAR *PRIPDATA;

#if (INCLUDE_RIPV2)
/* *********************************************************************   */
typedef struct rip_auth
{
    word        rp_family;          /* Always 0xFFFF for authentication */
    word        rp_type;            /* Authentication type: */

#define RIP_PASSWORD_AUTH   2       /* Simple plaintext password */

    byte        rp_auth[16];        /* Authentication data */
} RIPAUTH;
typedef RIPAUTH KS_FAR *PRIPAUTH;
#endif /* RIPV2 */

/* *********************************************************************   */
/* Context structure for a single RIP command                              */
typedef struct rip_command
{
    PRIPHDR            msg;
    int                msglen;
    struct sockaddr_in src;
    int                srclen;
} RIP_COMMAND;
typedef RIP_COMMAND KS_FAR *PRIP_COMMAND;


/* **********************************************************    */
/* RIP API                                                       */

void     rip_daemon_start(byte version);
void     rip_format_header(PRIPHDR msg, byte command, byte version);
void     rip_format_route_entry(PRIPHDR msg, int index, word family, 
                 word tag, PFBYTE addr, PFBYTE mask, PFBYTE gw, dword metric);
byte     rip_get_version(void);
PRIPDATA rip_rte(PRIPHDR msg, int index);
int      rip_send_request(PRIPHDR msg, word len, PFBYTE dst_ip, word port, 
                 short timeout);
void     rip_set_version(byte version);


/* **********************************************************    */
/* OTHER RIP FUNCTIONS                                           */

void     _rip_broadcast_request_all(byte version);
void     _rip_daemon(void);
int      _rip_daemon_cycle(void);
void     rip_fake_response(PRIPHDR msg, word len, PFBYTE src_ip, word port);
void     _rip_init_data(void);
int      _rip_init_socket(void);
void     _rip_process_response(PRIP_COMMAND prip);
RTIP_BOOLEAN  _rip_receive_okay(byte version);
RTIP_BOOLEAN  _rip_send_okay(byte version);
void     _rip_send_request_all(PIFACE pi, byte version);

#if (!INCLUDE_RIP_LISTEN_ONLY)
void     _rip_broadcast_rt_table(int update, byte version);
void     _rip_process_request(PRIP_COMMAND prip);
void     _rip_send_rt_table(PCSOCKADDR dest, int size, int update, byte version);
int      _rip_send_response(PRIPHDR msg, word rt_entries, PFBYTE dst_ip);
void     _rip_timer(void);
void     _rip_trigger_update(void);
#endif

#if (INCLUDE_RIPV1)
void     _rip_get_mask(PFBYTE mask, PFBYTE addr);
#endif 

#if (INCLUDE_RIPV2)
RTIP_BOOLEAN  _rip_process_authentication(PRIP_COMMAND prip, PRIPAUTH rpauth);
#endif


/* **********************************************************    */
/* RIP MACROS                                                    */

/* size in bytes of a rip packet with X route entries   */
#define rip_pkt_size(X)             (sizeof(RIPHDR)+sizeof(RIPDATA)*X)

#define _rip_daemon_is_running()    (rip_get_version() > 0)

#endif

