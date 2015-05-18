/*                                                                      */
/* DHCPCONS.C - DHCP CONSTANT GLOBAL DATA                               */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains all the global data for DHCP               */
/*      which is never modified.                                        */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DHCP

#include "sock.h"
#if (INCLUDE_DHCP_CLI)
#include "dhcp.h"

/* ********************************************************************   */
/* DHCPAPI                                                                */
/* ********************************************************************   */
KS_GLOBAL_CONSTANT DHCP_conf KS_FAR dhcp_default_conf =
{
    (PFDHCP_param)(0),  /* plist */
    0,                  /* plist_entries */
    (PFDHCP_cparam)(0),  /* cplist */
    0,                  /* cplist_entries */
    TRUE,               /* request_std_params */
    TRUE,               /* apply_std_params */
    900ul, /*86400ul,           // lease_time -- 1 day=60sec*60min*24hrs=86400ul */
                        /* 900=15 minutes   */
    {0,0,0,0},          /* client_ip_addr */
    {0,0,0,0},          /* req_ip_addr */
    (PFBYTE)(0),        /* vendor_specific */
    0                   /* vs_size */
};

/* ********************************************************************   */
/* DHCPPROC                                                               */
/* ********************************************************************   */
KS_GLOBAL_CONSTANT DHCP_param KS_FAR std_param_lst[DHCP_STD_PARAMS] = 
{
    {ROUTER_OPTION, 50},
    {MDRS, 50},
    {DEFAULT_IP_TTL, 50},
    {SUBNET_MASK, 50},
    {PATH_MTU_AT, 50},
    {STATIC_ROUTE_OP, 50},
    {ARP_CT_OP, 50},
};

#endif

