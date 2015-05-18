/* POPDATA.C - POP GLOBAL DATA                                          */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains all the global data for POP                */
/*      which is not constant.                                          */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

#include "rtipapi.h"
#include "popapi.h"

#if (INCLUDE_RUN_TIME_CONFIG)
/* *********************************************************************   */
/* ******                CONFIGURATION                            ******   */
/* *********************************************************************   */
struct cfg_pop_data cfg_pop_data =
{
    _CFG_POP_TIMEOUT
};
#endif

