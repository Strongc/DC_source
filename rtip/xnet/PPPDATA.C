/* PPPDATA.C - PPP GLOBAL DATA                                          */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains all the global data for PPP                */
/*      which is not constant.                                          */
/*                                                                      */
/*  Revision History:                                                   */
/*  11/98      initial coding                                           */
/*                                                                      */
#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP

#include "sock.h"
#include "rtip.h"

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* ************************ PPP ***************************************   */
/* ********************************************************************   */
#if (INCLUDE_PPP)

/* *********************************************************************   */
/* ******                CONFIGURATION                            ******   */
/* *********************************************************************   */
#if (INCLUDE_RUN_TIME_CONFIG)
struct cfg_ppp_data cfg_ppp_data = 
{
    _CFG_LCP_MRU_HI,
    _CFG_LCP_MRU_LO,
    _CFG_LCP_REQ_TRY,
    _CFG_LCP_NAK_TRY,
    _CFG_LCP_TERM_TRY,
    _CFG_LCP_TIMEOUT,
    _CFG_PAP_REQ_TRY,
    _CFG_PAP_FAIL_MAX,
    _CFG_PAP_TIMEOUT,
    _CFG_IPCP_REQ_TRY,
    _CFG_IPCP_NAK_TRY,
    _CFG_IPCP_TERM_TRY,
    _CFG_IPCP_TIMEOUT,
    _CFG_CHAP_REQ_TRY,
    _CFG_CHAP_TIMEOUT,
};
#endif

/* ********************************************************************   */
/* PPPFSM                                                                 */
/* ********************************************************************   */

/* ********************************************************************   */
/* PPPLCP                                                                 */
/* ********************************************************************   */

/* ********************************************************************   */
/* PPPIPCP                                                                */
/* ********************************************************************   */

/* ********************************************************************   */
/* PPPPAP                                                                 */
/* ********************************************************************   */

/* ********************************************************************   */
/* PPPUSER                                                                */
/* ********************************************************************   */
#if (!PAP_FILE)
    char KS_FAR pap_users[CFG_PAP_USERS][CFG_USER_REC_LEN];
#endif

#if (INCLUDE_CHAP)
/* ********************************************************************   */
/* PPPCHAP                                                                */
/* ********************************************************************   */

char KS_FAR local_host[CFG_CHAP_NAMELEN+1];

struct _chap_secrets KS_FAR chap_machine_secrets[CFG_CHAP_SECRETS];
#endif

/* ********************************************************************   */
/* CONTROL BLOCKS                                                         */
/* ********************************************************************   */
struct ppp_s  KS_FAR ppp_cb[CFG_NUM_RS232];     /* PPP control block */
struct lcp_s  KS_FAR lcp_cb[CFG_NUM_RS232];     /* LCP control block */
struct ipcp_s KS_FAR ipcp_cb[CFG_NUM_RS232];        /* IPCP control block */
struct pap_s  KS_FAR pap_cb[CFG_NUM_RS232];     /* PAP control block */
#if (INCLUDE_CHAP)
struct chap_s KS_FAR chap_cb[CFG_NUM_RS232];        /* CHAP control block */
#endif

#endif /*INCLUDE_PPP */
#endif /*INCLUDE_RTIP */


