/*                                                                      */
/* TNCDATA.C - Data file for Telnet Client                              */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1998                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#include "xnconf.h"
#include "rtip.h"
#include "tncapi.h"

/* ********************************************************************   */
/* CONFIGURATION DATA                                                     */
/* ********************************************************************   */
#if (INCLUDE_RUN_TIME_CONFIG)
CFG_TNC_DATA KS_FAR cfg_tnc_data = 
{
    _CFG_TNC_WRITE_TMO,
};
#endif
