/*                                                                      */
/* RTIPSTUB.H - RTIP functions need for standalone (i.e. non-RTIP)      */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __RTIPSTUB_H__
#define __RTIPSTUB_H__

/* ********************************************************************   */
#if (INCLUDE_RTIP)
 #include "rtipapi.h"

/* ********************************************************************   */
#else /* !INCLUDE_RTIP */
 #include <string.h>
 #include <stdlib.h>
 #include <stdio.h>

/* ********************************************************************   */
/* GENERAL TYPES                                                          */
/* ********************************************************************   */
# if(BCP0||BCP0NET)
  #define KS_FAR far
# else
  #define KS_FAR
# endif
 
 #define IP_ALEN 4

 #define KS_CONSTANT const
 #define RTIP_INF    0xffff

 typedef unsigned char  byte;
 typedef unsigned short word;
 typedef unsigned long  dword;

 typedef byte  KS_FAR * PFBYTE;
 typedef int   KS_FAR * PFINT;
 typedef word  KS_FAR * PFWORD;
 typedef long  KS_FAR * PFLONG;
 typedef dword KS_FAR * PFDWORD;
 typedef char  KS_FAR * PFCHAR;
 typedef void  KS_FAR * PFVOID;
 typedef const char  KS_FAR * PFCCHAR;
 typedef const byte  KS_FAR * PFCBYTE;
 typedef struct timeval KS_FAR *PCTIMEVAL;

/* ********************************************************************   */
/* PORTING                                                                */
/* ********************************************************************   */
#define OS_CLAIM_MEMFILE(X) 
#define OS_RELEASE_MEMFILE()

/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
 KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ip_nulladdr[IP_ALEN];

/* ********************************************************************   */
/* STRING LIBRARY                                                         */
/* ********************************************************************   */
 #define tc_strlen           strlen
 #define tc_movebytes           memcpy
 #define tc_memset              memset
 #define tc_comparen(A,B,C)      (memcmp((A),(B),(C))==0)
 #define tc_cmp4(A,B,C)         (tc_comparen((A),(B),(C))==0)
 #define tc_memmove             memmove
 #define tc_mv4                 tc_movebytes
 #define tc_strncmp             strncmp
 #define tc_strchr              strchr
 #define tc_strstr              strstr
 #define tc_stristr             stristr
 #define tc_strrchr             strrchr
 #define tc_strcpy              strcpy
 #define tc_strncpy             strncpy
 #define tc_strcat              strcat
 #define tc_strncat             strncat
 #define tc_atoi                atoi
 #define tc_atol                atol
 #define tc_strcmp              strcmp 
 #define tc_sprintf             sprintf
 #define tc_strspn              strspn
 #define tc_strcspn             strcspn 
#if (defined(__MET__))
 #define tc_itoa                _itoa
 #define tc_ltoa                _ltoa
 #define tc_ultoa               _ultoa
 #define tc_strnicmp            _strnicmp
 #define tc_stricmp             _stricmp
#elif (!defined(VXWORKS))
 #define tc_itoa                itoa
 #define tc_ltoa                ltoa
 #define tc_ultoa               ultoa
 #define tc_strnicmp            strnicmp
 #define tc_stricmp             stricmp
#endif

void debug_error(char *str, int type, void * val1, void * val2);
/* definitions for TYPE parameter   */
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

#define DEBUG_LOG(A,B,C,D,E)
#define DEBUG_ERROR(A,B,C,D) debug_error(A, B, (void *)C, (void *)D);
#define DEBUG_ASSERT(Z,A,B,C,D)
#define SHOW_PROGRESS(A)
#define SHOW_OUTPUT(A)
#define SELECT_TCP_EMPTY 0
#define SOCKAPI_
#define MAX_PACKETSIZE 1514

/* ********************************************************************   */
/* ERRNOS                                                                 */
/* ********************************************************************   */
#define ETOOMANYSERVERS 1 
#define ENOPKTS         1 

/* ********************************************************************   */
/* ROUTINE DECLARATIONS for rtipstub.c                                    */
/* ********************************************************************   */
unsigned int tc_hatoi(PFCHAR s);
void         xn_abort(int socket, RTIP_BOOLEAN send_reset);
int          set_errno(int errno);

/* ********************************************************************   */
/* TIME DIFFERENCES                                                       */
/* ********************************************************************   */
/* Used by FSMs for detecting timeouts                                    */
/* ks_get_ticks()   : number of ticks since system startup                */
/* ks_msec_p_tick() : number of milliseconds per tick                     */
/* TBD: why RTKMSP3, RTKBCP3 - doesn't that imply rtip?                   */
#if (INCLUDE_WINSOCK || INCLUDE_BSDSOCK )
  #define ks_get_ticks()     GetTickCount()
  #define ks_msec_p_tick()   1
  #define ks_ticks_p_sec()   1000
  #define ks_sleep(x)        Sleep(x*ks_msec_p_tick())
  #define os_spawn_task(CLASS, FNC, PARM1, PARM2, PARM3, PARM4) FALSE
# elif (defined(VXWORKS))
  #include <tickLib.h>
  #include <time.h>
  #define ks_get_ticks()     tickGet()
  #define ks_msec_p_tick()   ( 1000 / CLOCKS_PER_SEC )
  #define ks_ticks_p_sec()   CLOCKS_PER_SEC
  #define ks_sleep(x)        taskDelay(( x * CLOCKS_PER_SEC ) / 1000 )
# elif (defined(MT92101))
  /* From Mqxpegc.c   */
  extern "C" unsigned long Mqxpegc_get_msec(void);

  #define ks_get_ticks()     Mqxpegc_get_msec()
  #define ks_msec_p_tick()   (1)
  #define ks_ticks_p_msec()  (1000)
  #define ks_sleep(X)         
# else
  /*#error: must define ks_get_ticks() and ks_msec_p_tick()   */
# endif

#if (PEG_NUM_COLORS == 2)
#define DARKGRAY BLACK
#define LIGHTGRAY WHITE
#endif

/* *******************************************************************   */
/* ********************************************************************  */
/* ARGSUSED (keep compiler happy macros)                                 */
/* *******************************************************************   */
/* ********************************************************************  */

/* the ARGSUSED_PVOID and ARGSUSED_INT macros are used throughout        */
/* the code where parameters are passed which are never used             */
/* (many times due to turning off pieces of code via INCLUDE_xxx);       */
/* to keep the compiler from compaining about an unreferenced parameter; */
#define ARGSUSED_NOTHING 1      /* ARGSUSED_ macros go away */
#define ARGSUSED_ASSIGN  2      /* ARGSUSED_ macros do assignments */
#define ARGSUSED_FUNC    3      /* ARGSUSED_ macros call function */

#define KS_ARGSUSED ARGSUSED_ASSIGN     /* you may want to modify this */

/* DO NOT CHANGE: change ARGSUSED instead   */
#if (KS_ARGSUSED == ARGSUSED_NOTHING)
#define ARGSUSED_PVOID(p)   
#define ARGSUSED_INT(i)     
#elif (KS_ARGSUSED == ARGSUSED_ASSIGN)
#define ARGSUSED_PVOID(x)  x=x;
#define ARGSUSED_INT(x)    x=x;
#elif (KS_ARGSUSED == ARGSUSED_FUNC)
#define ARGSUSED_PVOID(x)  argsused_pvoid((PFVOID)x);
#define ARGSUSED_INT(x)    argsused_int((int)x);
#else
#error: KS_ARGUSED must be defined in osenv.h
#endif

/* ********************************************************************   */
/* MALLOC BUFFER                                                          */
/* ********************************************************************   */

/* who parameter to ks_malloc   */
#define NETUTIL_MALLOC  1
#define VF_MALLOC       2
PFBYTE ks_malloc(int num_elements, int size, int who);
void   ks_free(PFBYTE ptr, int num_elements, int size);
#endif      /* INCLUDE_RTIP */


#endif      /* defined __RTIPSTUB_H__ */

