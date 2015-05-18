/* CONFDATA.C - RTIP GLOBAL DATA                                        */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains all the global data for RTIP               */
/*      which is not constant.                                          */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

#include "sock.h"
#include "rtip.h"
#include "rtipapi.h"

/* ********************************************************************   */
/* CONFIGURATION DATA                                                     */
/* ********************************************************************   */
#if (INCLUDE_MALLOC_CONTEXT)

/* NOTE: -1 means no limit   */
CFG_LIMITS_DATA KS_FAR cfg_limits_data = 
{
#if (INCLUDE_MALLOC_PORT_LIMIT)
    -1,                     /* total_sockets */
#endif
    -1,                     /* total_server_threads */
#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
    -1,                     /* total_dcus */
#endif
};

/* actual number allocated   */
CFG_LIMITS_DATA KS_FAR num_alloced_data = 
{
#if (INCLUDE_MALLOC_PORT_LIMIT)
    0,                      /* total_sockets */
#endif
    0,                      /* total_server_threads */
#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
    0,                      /* total_dcus */
#endif
};

#endif      /* INCLUDE_RUN_TIME_CONFIG */

/* ********************************************************************   */
CFG_IP_DATA KS_FAR cfg_ip_data = 
{
    _CFG_IP_TTL,
#if (INCLUDE_RUN_TIME_CONFIG)
    _CFG_IP_FRAG_TTL,
    _CFG_MAX_FRAG,
    _CFG_MAX_FRAG_ICMP,
    _CFG_FRAG_TABLE_SIZE,
    _CFG_RTSIZE
#endif
};

#if (INCLUDE_RUN_TIME_CONFIG)
CFG_IFACE_DATA cfg_iface_data =
{
    _CFG_NIFACES,
    _CFG_NUM_DEVICES,
    _CFG_MCLISTSIZE
};
#endif

#if (INCLUDE_RUN_TIME_CONFIG)
CFG_PACKET_DATA KS_FAR cfg_packet_data =
{
    _CFG_MAX_PACKETSIZE,
    _CFG_NUM_PACKETS0, _CFG_PACKET_SIZE0,
    _CFG_NUM_PACKETS1, _CFG_PACKET_SIZE1,
    _CFG_NUM_PACKETS2, _CFG_PACKET_SIZE2,
    _CFG_NUM_PACKETS3, _CFG_PACKET_SIZE3,
    _CFG_NUM_PACKETS4, _CFG_PACKET_SIZE4,
    _CFG_NUM_PACKETS5, _CFG_PACKET_SIZE5,
    _CFG_PKT_LOWWATER, _CFG_PKT_LOWWATER_OOO,
    _CFG_PKT_QUENCH         
};

CFG_PROTOCOL_DATA KS_FAR cfg_protocol_data =
{
    _CFG_NTCPPORTS,
    _CFG_NUDPPORTS,
    _CFG_ARPCLEN,
    _CFG_NUM_IP2ETH_ADDR,
    _CFG_UDP_PORT_MIN,
    _CFG_UDP_PORT_MAX,
    _CFG_TCP_PORT_MIN,
    _CFG_TCP_PORT_MAX,
    _CFG_TIMER_FREQ,
    _CFG_IGMP_MAX_DELAY,
};
#endif /* INCLUDE_RUN_TIME_CONFIG */

#if (INCLUDE_TCP)
CFG_TCP_DATA KS_FAR cfg_tcp_data =
{
    _CFG_KA_INTERVAL,
    _CFG_KA_RETRY,
    _CFG_KA_TMO,
#if (INCLUDE_RUN_TIME_CONFIG)
    _CFG_TIMER_FREQ,
    _CFG_TMO_PROC,
    _CFG_MAX_DELAY_ACK,
    _CFG_MAXRTO,
    _CFG_MINRTO,
    _CFG_RETRANS_TMO,
    _CFG_REPORT_TMO,
    _CFG_WIN_PROBE_MIN,
    _CFG_WIN_PROBE_MAX,
    _CFG_TWAITTIME,
    _CFG_LASTTIME,
    _CFG_TCP_MAX_CONN,
    _CFG_NO_MSS_VAL,
    _CFG_MSS_GATEWAY,
    _CFG_NUM_OOO_QUE,
    _CFG_TCP_SEND_WAIT_ACK
#endif /* INCLUDE_RUN_TIME_CONFIG */
};
#endif /* INCLUDE_TCP */

#if (INCLUDE_RUN_TIME_CONFIG)
#if (INCLUDE_BOOTP || INCLUDE_RARP)
CFG_RARP_BOOTP_DATA KS_FAR cfg_rarp_bootp_data =
{
    _CFG_RARP_TRIES,    _CFG_RARP_TMO,
    _CFG_BOOTP_RETRIES, _CFG_BOOTP_RTSIZE 
};
#endif

#if (INCLUDE_DNS)
/* ********************************************************************   */
struct cfg_dns_data KS_FAR cfg_dns_data =
{
    _CFG_MIN_DNS_DELAY,
    _CFG_MAX_DNS_DELAY,
    _CFG_DNS_RETRIES,
};
#endif

#if (INCLUDE_PPP_VANJC || INCLUDE_CSLIP)
/* ********************************************************************   */
struct cfg_slhc_data KS_FAR cfg_slhc_data =
{
    _CFG_CSLIP_SLOTS
};
#endif

/* ********************************************************************   */
struct cfg_tasks_data cfg_tasks_data =
{
    _PRIOTASK_NORMAL,
    _PRIOTASK_HI,
    _PRIOTASK_HIGHEST,
    _SIZESTACK_NORMAL,
    _SIZESTACK_BIG,
    _SIZESTACK_HUGE
};
#endif  /* INCLUDE_RUN_TIME_CONFIG */
