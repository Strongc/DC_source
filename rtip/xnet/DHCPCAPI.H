/*
   DHCPAPI.H  - DHCP header file

   EBS - RTIP

   Copyright Peter Van Oudenaren , 1996
   All rights reserved.
   This code may not be redistributed in source or linkable object form
   without the consent of its author.

   Module description:
    This file contains constants, structures, and function prototypes
    for use in DHCP applications.
*/

#ifndef __DHCPAPI_H__
#define __DHCPAPI_H__

#include "xnconf.h"

#define INCLUDE_TFTP_CISCO 1

/********** CONSTANTS FOR DHCP OPTIONS *********                            */
/* NOTE: values for the 'id' field of the struct DHCP_param structure       */
#define PAD                     0
#define END                   255
#define SUBNET_MASK             1
#define TIME_OFFSET             2
#define ROUTER_OPTION           3
#define TIME_SERVER_OP          4
#define NAME_SERVER_OP          5
#define DNS_OP                  6
#define LOG_SERVER_OP           7
#define COOKIE_SERVER_OP        8
#define LPR_SERVER_OPTION       9
#define IMPRESS_SERVER_OP      10
#define RLS_OP                 11
#define HOST_NAME_OP           12
#define BFS_OP                 13
#define MERIT_DUMP             14
#define DOMAIN_NAME            15
#define SWAP_SERVER            16
#define ROOT_PATH              17
#define EXTENSIONS_PATH        18
#define IP_FORWARDING          19
#define NLSR                   20
#define POLICY_FILTER          21
#define MDRS                   22
#define DEFAULT_IP_TTL         23
#define PATH_MTU_AT            24
#define PATH_MTU_PLATEAU       25
#define INTERFACE_MTU          26
#define ALL_SUBNETS_LOCAL      27
#define BROADCAST_ADDRESS      28
#define MASK_DISCOVERY         29
#define MASK_SUPPLIER          30
#define ROUTER_DISCOVERY       31
#define RSA_OP                 32
#define STATIC_ROUTE_OP        33
#define TRAILER_ENCAP_OP       34
#define ARP_CT_OP              35
#define EE_OP                  36
#define TCP_DEFAULT_TTL        37
#define TCP_KA_INTERVAL        38
#define TCP_KA_GARBAGE         39
#define NISD                   40
#define NIS_OP                 41
#define NTPS_OP                42
#define VENDOR_SPECIFIC        43
#define NET_OVER_TCP_NS        44
#define NET_OVER_TCP_DDS       45
#define NET_OVER_TCP_NT        46
#define NET_OVER_TCP_SCOPE     47
#define XWIN_SFS               48
#define XWIN_SDM               49
#define REQ_IP                 50
#define IP_LEASE               51
#define OP_OVERLOAD            52
#define DHCP_MSG_TYPE          53
#define SERVER_ID              54
#define PARAM_REQ_LST          55
#define MESSAGE                56
#define MAX_DHCP_MSG_SIZE      57
#define RENEWAL_TIME           58
#define REBINDING_TIME         59
#define CLASS_ID               60
#define CLIENT_ID              61
#if (INCLUDE_TFTP_CISCO)
#define TFTP_ADDR_CISCO       150       /* Cisco option to be TFTP address */
#endif

/* *********************************************************************      */
/********** DHCP ARRAY SIZE CONSTANTS *********                               */
#define DHCP_PKT_OP_SIZE      312
#define DHCP_PKT_SNAME_SIZE    64
#define DHCP_PKT_FILE_SIZE    128
#define DHCP_PKT_CHADDR_SIZE   16

/* struct DHCP_packet     */
typedef struct DHCP_packet
{
    byte  op;
    byte  htype;
    byte  hlen;
    byte  hops;
    dword xid;
    word  secs;
    word  flags;
    dword ciaddr;                           /* address client knows it wants */
    dword yiaddr;
    dword siaddr;
    dword giaddr;
    byte  chaddr[DHCP_PKT_CHADDR_SIZE];     /* client ether address */
    byte  sname[DHCP_PKT_SNAME_SIZE];
    byte  file[DHCP_PKT_FILE_SIZE];
    byte  options[DHCP_PKT_OP_SIZE];

    word  options_index;     /* internal */
    byte  overload_type;     /* internal */
    byte  byte_even;         /* pad to make structure even # of bytes */
} DHCP_packet;
typedef struct DHCP_packet KS_FAR * PFDHCP_packet;

#define DHCP_PKT_EXTRA  4   /* size of internal fields in packet */

#if (INCLUDE_DHCP_CLI)  /* exclude rest of file */
/* *********************************************************************      */
/* ******                CONFIGURATION                            ******      */
/* *********************************************************************      */
#if (INCLUDE_RUN_TIME_CONFIG)
typedef struct cfg_dhcpc_data
{
    int dhcp_retries;
    int dhcp_timeout;
} CFG_DHCPC_DATA;

#define CFG_DHCP_OFFERS     _CFG_DHCP_OFFERS
#define CFG_DHCP_RETRIES    cfg_dhcpc_data.dhcp_retries
#define CFG_DHCP_TIMEOUT    cfg_dhcpc_data.dhcp_timeout

extern CFG_DHCPC_DATA KS_FAR cfg_dhcpc_data;

#else
#define CFG_DHCP_OFFERS     _CFG_DHCP_OFFERS
#define CFG_DHCP_RETRIES    _CFG_DHCP_RETRIES
#define CFG_DHCP_TIMEOUT    _CFG_DHCP_TIMEOUT
#endif

/* ************************************************************************         */
/********** CONSTANTS FOR 'packet_style' in DHCP_session *********                  */
#define DHCP_RFC        1
#define DHCP_MICROSOFT  2

/********** CONSTANT for 'prio' field of DHCP_param *********    */
#define DHCP_MUST_HAVE  100

/* parameter to request from server     */
typedef struct DHCP_param
{
    byte id;    /* option (SUBNET_MASK for example) */
    byte prio;  /* 0-99, 100=must have */
} DHCP_param;

typedef struct DHCP_param KS_FAR * PFDHCP_param;

/* *********************************************************************      */
/* parameter for client to send to server                                     */
typedef struct DHCP_cparam
{
    byte id;        /* option (HOST_NAME_OP for example) */
    byte len;       /* length of option */
    byte fill[2];   /* alignment */
    void *cpdata;   /* point to option - if string must be null terminated 
                                         for example, domain name */
    void *next;     /* point to next parameter entry */
} DHCP_cparam;

typedef struct DHCP_cparam KS_FAR * PFDHCP_cparam;

/* *********************************************************************      */
/* STRUCT DHCP_conf                                                           */
typedef struct DHCP_conf
{
    PFDHCP_param plist; /* list of parameters to request from server */
                        /* set to NULL if:                                   */
                        /* a) you have no parameters to request,             */
                        /* b) you are requesting only the RTIP standard      */
                        /* parameters, and have set request_std_params to    */
                        /* TRUE.                                             */
    int          plist_entries;
    PFDHCP_cparam cplist; /* list of parameters to send to server         */
                        /* set to NULL if:                                      */
                        /* a) you have no parameters to send                    */
    int          cplist_entries;
    RTIP_BOOLEAN request_std_params; /* set to TRUE to request RTIP standard */
                                     /* configuration parameters in addition      */
                                     /* to those in plist (if any). Any RTIP      */
                                     /* standard parameters returned by the       */
                                     /* server can be accessed through the        */
                                     /* 'std_params' field of DHCP_session.       */
    RTIP_BOOLEAN apply_std_params;   /* if set to TRUE, any RTIP standard */
                                     /* parameters returned by the server      */
                                     /* will be applied.                       */
    dword        lease_time;         /* desired ip lease time (units: seconds) */
    byte         client_ip_addr[4];  /* fill in if known, otherwise you will */
                                     /* be leased one from server     */
    byte         req_ip_addr[4];
    PFBYTE       vendor_specific;
    int          vs_size;
} DHCP_conf;
typedef struct DHCP_conf KS_FAR * PFDHCP_conf;

/* *********************************************************************      */
/* STRUCT DHCP_std_params;                                                    */
/* the pointers are setup to point to the option information; if the          */
/* option is not available the entry is set to 0;                             */
/* the types for the options are based upon the types in the message          */
/* received (i.e. 1 byte, 2 bytes or 4 bytes); they are converted             */
/* to the internal RTIP type if they differ                                   */
typedef struct DHCP_std_params /* RTIP standard configuration params */
{
    /* Capitalized names in comments correspond to constants for these
       options. */
    dword *  subnet_mask;       /* SUBNET_MASK */
    dword *  dns_server;        /* DNS */
    int      dns_server_len;    /* length of dns */
    byte *   domain_name;       /* DOMAIN_NAME */
    int      domain_name_len;   /* length of domain name */
    byte *   be_a_router;       /* ROUTER_OPTION */
    word *   mdrs;              /* MDRS (maximum datagram reassembly size) */
    byte *   default_ip_ttl;    /* DEFAULT_IP_TTL */
    word *   mtu;               /* PATH_MTU_AT */
    dword *  static_route;      /* STATIC_ROUTE_OP */
    int      static_route_len;  /* length of static route information */
    dword *  router_option;     /* ROUTER_OPTION */
    int      router_option_len; /* length of router option information */
    dword *  arp_cache_timeout; /* ARP_CT_OP */
    byte *   tcp_default_ttl;   /* TCP_DEFAULT_TTL */
    dword *  tcp_ka_interval;   /* TCP_KA_INTERVAL */
    byte *   tcp_ka_garbage;    /* TCP_KA_GARBAGE */
#if (INCLUDE_TFTP_CISCO)
    dword *  tftp_srv_ip;       /* TFTP_ADDR_CISCO */
#endif
} DHCP_std_params /* RTIP standard configuration params */;

typedef struct DHCP_std_params KS_FAR * PFDHCP_std_params;

/* *********************************************************************      */
/* STRUCT DHCP_session                                                        */
typedef struct DHCP_session
{
    struct DHCP_packet     outmsg;
    struct DHCP_packet     replies[CFG_DHCP_OFFERS];
    byte                   client_ip[IP_ALEN];
    dword                  lease_time;      /* units: seconds */
    byte                   server_ip[IP_ALEN]; 
    struct DHCP_std_params params;
    /* internal     */
    byte                   packet_style;
    int                    iface_no;
#if (INCLUDE_DHCP_RENEW_NO_BLOCK)
    int                    dhcpc_sock;
    long                   timeout_ticks;
    int                    retries;
    dword                  start_time;
    int                    rindex;
#endif
} DHCP_session;
typedef struct DHCP_session KS_FAR * PFDHCP_session;

typedef int (*DHCP_callback_fp)(PFDHCP_packet,int,PFDHCP_conf);

/* *********************************************************************      */
/********** DHCP CLIENT API FUNCTIONS (DHCPAPI.C) *********                   */

#ifdef __cplusplus
extern "C" {
#endif

/* basic API functions     */
void init_dhcp(void);
void xn_init_dhcp_conf(PFDHCP_conf conf);
int  xn_dhcp(int iface_no,PFDHCP_session session,PFDHCP_conf conf);
int  xn_extend_dhcp_lease(PFDHCP_session session,dword lease_time);
int  xn_dhcp_release(PFDHCP_session session);
int  xn_dhcp_decline(PFDHCP_session session);

/* API functions for advanced use of DHCP:     */
void xn_set_dhcp_callback(DHCP_callback_fp fp);
int  xn_get_dhcp_op(PFDHCP_session session,byte op_id,byte bufsize,PFVOID vpdest);
void xn_set_dhcp_conf_op(PFDHCP_conf conf,PFDHCP_cparam cpdata);

#ifdef __cplusplus
}
#endif

#endif  /* INCLUDE_DHCP_CLI */
#endif  /* __DHCPAPI_H__ */

