/*                                                                      */
/* DHCPDATA.C - DHCP GLOBAL DATA                                        */
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
/*      which is not constant.                                          */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DHCP

#include "sock.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#endif

#if (INCLUDE_DHCP_CLI)
#include "dhcp.h"
#endif

#if (INCLUDE_DHCP_SRV)
#include "dhcpsapi.h"
#endif


/* ********************************************************************   */
/* CONFIGURATION DATA                                                     */
/* ********************************************************************   */
#if (INCLUDE_DHCP_SRV && INCLUDE_RUN_TIME_CONFIG)
CFG_DHCPS_DATA KS_FAR cfg_dhcps_data = 
{
    _CFG_DHCPS_LEASE_TIME_DIFF,
};
#endif

#if (INCLUDE_DHCP_CLI && INCLUDE_RUN_TIME_CONFIG)
CFG_DHCPC_DATA KS_FAR cfg_dhcpc_data = 
{
    _CFG_DHCP_RETRIES,
    _CFG_DHCP_TIMEOUT,
};
#endif

#if (INCLUDE_DHCP_CLI)
/* ********************************************************************   */
/* DHCP API                                                               */
/* ********************************************************************   */
DHCP_callback_fp KS_FAR choose_dhcp_offer;

/* timer information   */
EBS_TIMER KS_FAR dhcp_timer_info;       

/* used to register DHCP init fnc   */
INIT_FNCS KS_FAR dhcp_fnc;      

#endif /* INCLUDE_DHCP_CLI */
