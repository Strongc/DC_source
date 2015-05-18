/*                                                                      */
/* DEBUGAPI.H - ERROR LOGGING functions                                 */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */


#ifndef __RTIP_DEBUGAPI__
#define __RTIP_DEBUGAPI__ 1

#include "xnconf.h"

/* ********************************************************************   */
/* definitions for TYPE parameter                                         */
#define NOVAR       0       /* write string only, i.e.val1 and val2 are invalid */
#define EBS_INT1    1       /* val1 is an integer */
#define EBS_INT2    2       /* val1 and val 2 are an integers */
#define LINT1       3       /* val1 is a long signed integer */
#define LINT2       4       /* val1 and val 2 are long signed integers */
#define DINT1       5       /* val1 is a long unsigned integer */
#define DINT2       6       /* val1 and val 2 are long unsigned integers */
#define STR1        7       /* val1 is a string */
#define STR2        8       /* val1 and val 2 are strings */
#define PTR1        9       /* val1 is a pointer */
#define PTR2        10      /* val1 & val2 are pointers */
#define IPADDR      11      /* val1 is an IP address of type PFBYTE */
#define ARPC_ENTRY  12      /* val1 is an arp cache entry */
#define ETHERADDR   13      /* val1 is an ethernet address */
#define RT_ENTRY    14      /* val1 is a routing table entry */
#define RT          15      /* val1, val2 not used (ROUTING TABLE) */
#define PKT         16      /* val1 is an ethernet packet of type PFBYTE */
#define PORTS_TCP   17      /* write port lists; val1 and val2 are invalid */
#define PORTS_UDP   18      /* write port lists; val1 and val2 are invalid */
#define PORTS_RAW   19      /* write port lists; val1 and val2 are invalid */
#define PORTS_PING  20      /* write port lists; val1 and val2 are invalid */
#define PORTS_ALL   21      /* write port lists; val1 and val2 are invalid */

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
#ifdef __cplusplus
extern "C" {
#endif

void SOCKAPI_ xn_debug_error(PFCCHAR string, int type, dword val1, dword val2);

#ifdef __cplusplus
}
#endif

#endif  /* __RTIP_DEBUGAPI__  */

