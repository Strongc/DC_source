/*                                                                      */
/* DYMCONF.H - Dynamic Memory Configuration FIle                        */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __DYMCONF__
#define __DYMCONF__

#include "ostype.h"
#include "vfconf.h"

/* ************************************************************************         */
#define INCLUDE_BGET    1 /* Use allocator routines in bget.c */

/* *********************************************************************   */
#define INCLUDE_MALLOC 0

#if (INCLUDE_MFS || INCLUDE_BOOTP || INCLUDE_PPPOE || INCLUDE_TRK_PKTS)
#undef INCLUDE_MALLOC
#define INCLUDE_MALLOC 1

#elif (BUILD_NEW_BINARY || INCLUDE_RUN_TIME_CONFIG || INCLUDE_MALLOC_PORTS || INCLUDE_MALLOC_CONTEXT || INCLUDE_MALLOC_DCU_INIT || INCLUDE_MALLOC_DCU_AS_NEEDED)
#undef INCLUDE_MALLOC
#define INCLUDE_MALLOC 1

#elif (INCLUDE_FTP_SRV || INCLUDE_WEB || INCLUDE_TELNET_SRV)
#if (INCLUDE_MALLOC_CONTEXT)
#undef INCLUDE_MALLOC
#define INCLUDE_MALLOC 1
#endif
#endif

#endif /* __DYMCONF__ */

