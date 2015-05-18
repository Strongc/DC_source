/*                                                                      */
/* OS.H - defines for layer which interfaces with the porting layer     */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*                                                                      */
/*  This module consists of all the macros which interface to the       */
/*      operating systems porting layer.                                */

#ifndef __OS__
#define __OS__ 1

#include "xnconf.h"
#include "osport.h"
#if (INCLUDE_ERTFS)
#ifndef BOOLEAN
typedef int BOOLEAN;
#endif
#include "pcconf.h"
#endif

/* Declarations that are common to rtfs/rtip   */

#if (INCLUDE_RTIP)
#if (INCLUDE_PPP)
/*#include "ppp.h"   */
#endif

/* ********************************************************************   */
#define KS_INF RTIP_INF

/* ********************************************************************   */
/* macro to access OS_LIST in entry which is at offset instead of         */
/* at the beginning of the structures (the offset are 0 for the first,    */
/* 1 for the second, etc)                                                 */
/* NOTE: offsets are 0, 1, 2 etc                                          */
#define POS_ENTRY_OFF(entry, offset) ((POS_LIST)entry + offset)

/* ********************************************************************   */
/* SIGNALS                                                                */
/* Define all signals bound to an interface (iface) - signal index
    0 = IP     - signalled when data packet sent to IP exchange
    1 = RARP   - signalled when rarp response has been received (and processed)
    2 = OUTPUT - signalled when output is complete
    3 = INTERRUPT - signalled when interrupt needs processing (processing
                    done at task layer instead of interrupt layer)
    4 = PPP    - signalled when PPP connection has become established (xn_lcp_open) */
#define KS_IFACE_SIGNAL_IP          0
#define KS_IFACE_SIGNAL_RARP        1
#define KS_IFACE_SIGNAL_INTERRUPT   2
#define KS_IFACE_SIGNAL_PPP         3

#define NUM_SIG_PER_IFACE           (3+(INCLUDE_TASK_ISRS || INCLUDE_PPP))


/* Define all signals bound to a port (socket) - signal index
    0  = READ   - signalled when data available for TCP. Also signaled when
                  UDP packet sent to input exchange 
    1  = WRITE  - signalled when data sent by TCP has been acked 
    2  = SENT   - if requested, ARP signals the port when a packet which 
                  was queued for address resolution is sent
    3+x = SELECT - If the port is in a socket select group on of the port's
                  select signal will be used to signal completion 
                  NOTE: x ranges from 0 to CFG_NUM_SELECT_P_SOCK-1
    3+CFG_NUM_SELECT_P_SOCK  = 
         DCU    - signalled when DCU is freed 
                  NOTE: only used if INCLUDE_NO_DCU_BLOCK
*/
/* NOTE: KS_PORT_SIGNAL_SELECT needs to be last   */
#define KS_PORT_SIGNAL_READ     0
#define KS_PORT_SIGNAL_WRITE    1
#define KS_PORT_SIGNAL_PING     1
#define KS_PORT_SIGNAL_SENT     2
#define KS_PORT_SIGNAL_SELECT   3
#define KS_PORT_SIGNAL_DCU      (3+CFG_NUM_SELECT_P_SOCK)
#define NUM_SIG_PER_PORT        (3+INCLUDE_NO_DCU_BLOCK+CFG_NUM_SELECT_P_SOCK)

/* ********************************************************************   */
#if (!INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
extern KS_RTIPSIG portsig[TOTAL_PORTS][NUM_SIG_PER_PORT];
#endif

extern KS_RTIPSEM udpsem;
extern KS_RTIPSEM syslogsem;
extern KS_RTIPSEM tcpsem;
extern KS_RTIPSEM criticalsem;

#if (INCLUDE_WEB_BROWSER && defined(PEGRTIP))
/* TBDTBD - Move this later   */
#define MAX_GUI_TASKS         4     /* three demo tasks + PegTask */
#define NUM_PEG_EVENTS (MAX_GUI_TASKS + 1)
#define PEG_EV_MSGFREE 0
#define PEG_EV_CONSOLE MAX_GUI_TASKS

#define PEG_RES_MSG     0
#define PEG_RES_TMR     1
#define PEG_RES_SCREEN  2
#define NUM_PEG_SEM 3

#ifdef __cplusplus
extern "C" {
#endif
extern KS_RTIPSIG ks_peg_sig[NUM_PEG_EVENTS];
extern KS_RTIPSEM ks_peg_sem[NUM_PEG_SEM];
#ifdef __cplusplus
}
#endif
#endif

/* ********************************************************************   */
/* This mapping scheme reduces the number of entry points                 */

/* Signal and exchange encoding 
    BIT NUMBERS         PURPOSE
    -----------         -------
    0-3 (0x000x)        Which signal or semaphore to use
    4-7 (0x00x0)        Which exchange to use 
    8   (0x0100)        Object is an iface structure
    9   (0x0200)        Object is a port structure
*/

/* defines for the above encoding scheme   */
#define OS_HNDL_TO_SIGNAL(X) (word) (X & 0x0f)        /* Signal index */
#define OS_HNDL_TO_EXCH(X)   (word) (((X & 0xf0)>>4)) /* exchange index */

#define OS_OB_IFACE 0x0100                   /* Encodings */
#define OS_OB_PORT  0x0200

/* Define all signals bound to an interface
    0 = IP exchange, 1 = ping exchange, 2 = rarp complete sig. 
    3 = output complete signal. (3 is optionally used by drivers) 
    4 = interrupt needs processing, 5 = PPP open done */
#define IF_SIG_IP           (OS_OB_IFACE|KS_IFACE_SIGNAL_IP)
#define IF_SIG_RARP         (OS_OB_IFACE|KS_IFACE_SIGNAL_RARP)
#define IF_SIG_INTERRUPT    (OS_OB_IFACE|KS_IFACE_SIGNAL_INTERRUPT)
#define IF_SIG_PPP          (OS_OB_IFACE|KS_IFACE_SIGNAL_PPP)

/* Define all exchanges bound to an interface
    0 (0x00) = IP
    All exchanges have a signal (of the same name) associated with them */
#define IF_EX_IP    (0x00|IF_SIG_IP)

/* Define all signals bound to a port   */
#define PO_SIG_READ   (OS_OB_PORT|KS_PORT_SIGNAL_READ)
#define PO_SIG_WRITE  (OS_OB_PORT|KS_PORT_SIGNAL_WRITE)
#define PO_SIG_SENT   (OS_OB_PORT|KS_PORT_SIGNAL_SENT)
#define PO_SIG_SELECT (OS_OB_PORT|KS_PORT_SIGNAL_SELECT)
#define PO_SIG_PING   (OS_OB_PORT|KS_PORT_SIGNAL_PING)
#define PO_SIG_DCU    (OS_OB_PORT|KS_PORT_SIGNAL_DCU)

/* Port exchanges: one for udp input and one for PING (they are
   the same signal and exchange);
    All exchanges have a signal associated with them */
#define PO_EX_UDP  (0x00|PO_SIG_READ)
#define PO_EX_PING (0x00|PO_SIG_PING)

/* ********************************************************************   */
/* SEMAPHORES                                                             */
/* ********************************************************************   */

/* called by rest of rtip   */
#if (!INCLUDE_TRK_PKTS )
#define OS_CLAIM_UDP(X)              KS_SEMAPHORE_GET(udpsem);
#define OS_RELEASE_UDP()             KS_SEMAPHORE_GIVE(udpsem);
#define OS_CLAIM_SYSLOG(X)           KS_SEMAPHORE_GET(syslogsem);
#define OS_RELEASE_SYSLOG()          KS_SEMAPHORE_GIVE(syslogsem);
#define OS_CLAIM_TCP(X)              KS_SEMAPHORE_GET(tcpsem);
#define OS_RELEASE_TCP()             KS_SEMAPHORE_GIVE(tcpsem);
#define OS_CLAIM_IFACE(PI,X)         KS_SEMAPHORE_GET(PI->ctrl.ifsem);
#define OS_RELEASE_IFACE(PI)         KS_SEMAPHORE_GIVE(PI->ctrl.ifsem);
#define OS_ENTER_CRITICAL(X)         KS_SEMAPHORE_GET(criticalsem);
#define OS_EXIT_CRITICAL()           KS_SEMAPHORE_GIVE(criticalsem);
#define OS_CLAIM_MEMFILE(X)          KS_SEMAPHORE_GET(memfilesem);
#define OS_RELEASE_MEMFILE()         KS_SEMAPHORE_GIVE(memfilesem);

#else
/* parameters for sem_no   */
#define UDP_CLAIM       1
#define TCP_CLAIM       2
#define CRIT_CLAIM      3
#define MEMFILE_CLAIM   4
#define SYSLOG_CLAIM    5
#define IFACE_CLAIM     6   /* PUT IFACE LAST SINCE MIGHT BE MORE THAN ONE */

void track_sem_pend(int who, int sem_no, int offset);
void track_sem_claim(int who, int sem_no, int offset);
void track_sem_rel(int sem_no, int offset);
#define OS_CLAIM_UDP(X)              {track_sem_pend(X, UDP_CLAIM, 0);     \
                                      KS_SEMAPHORE_GET(udpsem);            \
                                      track_sem_claim(X, UDP_CLAIM, 0);}
#define OS_RELEASE_UDP()             {track_sem_rel(UDP_CLAIM, 0);            \
                                      KS_SEMAPHORE_GIVE(udpsem);}
#define OS_CLAIM_SYSLOG(X)           {track_sem_pend(X, SYSLOG_CLAIM, 0);     \
                                      KS_SEMAPHORE_GET(syslogsem);         \
                                      track_sem_claim(X, SYSLOG_CLAIM, 0);}
#define OS_RELEASE_SYSLOG()          {track_sem_rel(SYSLOG_CLAIM, 0);         \
                                      KS_SEMAPHORE_GIVE(syslogsem);}
#define OS_CLAIM_TCP(X)              {track_sem_pend(X, TCP_CLAIM, 0);        \
                                      KS_SEMAPHORE_GET(tcpsem);            \
                                      track_sem_claim(X, TCP_CLAIM, 0);}
#define OS_RELEASE_TCP()             {track_sem_rel(TCP_CLAIM, 0);            \
                                      KS_SEMAPHORE_GIVE(tcpsem);}

#define OS_CLAIM_IFACE(PI,X)         {track_sem_pend(X, IFACE_CLAIM, PI->ctrl.index);      \
                                      KS_SEMAPHORE_GET(PI->ctrl.ifsem);    \
                                      track_sem_claim(X, IFACE_CLAIM, PI->ctrl.index);}
#define OS_RELEASE_IFACE(PI)         {track_sem_rel(IFACE_CLAIM, PI->ctrl.index);          \
                                      KS_SEMAPHORE_GIVE(PI->ctrl.ifsem);}
#define OS_ENTER_CRITICAL(X)         {track_sem_pend(X, CRIT_CLAIM, 0);       \
                                      KS_SEMAPHORE_GET(criticalsem);       \
                                      track_sem_claim(X, CRIT_CLAIM, 0);}
#define OS_EXIT_CRITICAL()           {track_sem_rel(CRIT_CLAIM, 0);           \
                                      KS_SEMAPHORE_GIVE(criticalsem);}
#define OS_CLAIM_MEMFILE(X)              {track_sem_pend(X, MEMFILE_CLAIM, 0);        \
                                      KS_SEMAPHORE_GET(memfilesem);            \
                                      track_sem_claim(X, MEMFILE_CLAIM, 0);}
#define OS_RELEASE_MEMFILE()             {track_sem_rel(MEMFILE_CLAIM, 0);            \
                                      KS_SEMAPHORE_GIVE(memfilesem);}
#endif /* INCLUDE_TRK_PKTS || POLLOS */

/* ********************************************************************   */
/* SIGNALS                                                                */
/* ********************************************************************   */

#define IFSIG(PI, SIG_NO) (PI)->ctrl.ifsig[(SIG_NO)]

/* map iface signals to kernel specific   */
#define OS_IFACE_SIGNAL_BIND(SIGNAL, PI)                                    \
        KS_SIGNAL_BIND(IFSIG((PI), OS_HNDL_TO_SIGNAL(SIGNAL)))
#define OS_IFACE_SIGNAL_SET(SIGNAL, PI)                                     \
        KS_SIGNAL_SET(IFSIG((PI), OS_HNDL_TO_SIGNAL(SIGNAL)))
#define OS_IFACE_SIGNAL_CLEAR(SIGNAL, PI)                                   \
        KS_SIGNAL_CLEAR(IFSIG((PI), OS_HNDL_TO_SIGNAL(SIGNAL)))
#define OS_IFACE_SIGNAL_TEST(SIGNAL, PI, WAIT_COUNT)                        \
        KS_SIGNAL_GET(IFSIG((PI), OS_HNDL_TO_SIGNAL(SIGNAL)), WAIT_COUNT)

/* map port signals to kernel specific   */
#if (!INCLUDE_MALLOC_PORTS && !INCLUDE_RUN_TIME_CONFIG)
#define OS_PORT_SIGNAL_BIND(SIGNAL, PO)             \
        KS_SIGNAL_BIND(portsig[((PANYPORT) PO)->ctrl.index][OS_HNDL_TO_SIGNAL(SIGNAL)])
#define OS_PORT_SIGNAL_SET(SIGNAL, PO)              \
        KS_SIGNAL_SET(portsig[((PANYPORT) PO)->ctrl.index][OS_HNDL_TO_SIGNAL(SIGNAL)])
#define OS_PORT_SIGNAL_CLEAR(SIGNAL, PO)            \
        KS_SIGNAL_CLEAR(portsig[((PANYPORT) PO)->ctrl.index][OS_HNDL_TO_SIGNAL(SIGNAL)])
#define OS_PORT_SIGNAL_TEST(SIGNAL, PO, WAIT_COUNT) \
        KS_SIGNAL_GET(portsig[((PANYPORT) PO)->ctrl.index][OS_HNDL_TO_SIGNAL(SIGNAL)], WAIT_COUNT)
#else
/* SPR: tbd: when this works do it this way for all   */
#define OS_PORT_SIGNAL_BIND(SIGNAL, PO)             \
        KS_SIGNAL_BIND(((PANYPORT) PO)->portsig[OS_HNDL_TO_SIGNAL(SIGNAL)])
#define OS_PORT_SIGNAL_SET(SIGNAL, PO)              \
        KS_SIGNAL_SET(((PANYPORT) PO)->portsig[OS_HNDL_TO_SIGNAL(SIGNAL)])
#define OS_PORT_SIGNAL_CLEAR(SIGNAL, PO)            \
        KS_SIGNAL_CLEAR(((PANYPORT) PO)->portsig[OS_HNDL_TO_SIGNAL(SIGNAL)])
#define OS_PORT_SIGNAL_TEST(SIGNAL, PO, WAIT_COUNT) \
        KS_SIGNAL_GET(((PANYPORT) PO)->portsig[OS_HNDL_TO_SIGNAL(SIGNAL)], WAIT_COUNT)
#endif

/* ********************************************************************   */
/* called by rest of rtip                                                 */
#define OS_TEST_RARP_SIGNAL(PI,WAIT_COUNT)  \
    OS_IFACE_SIGNAL_TEST((IF_SIG_RARP),  PI, WAIT_COUNT)
#define OS_BIND_RARP_SIGNAL(PI)             \
    OS_IFACE_SIGNAL_BIND((IF_SIG_RARP),  PI) 
#define OS_CLEAR_RARP_SIGNAL(PI)            \
    OS_IFACE_SIGNAL_CLEAR((IF_SIG_RARP),  PI) 
#define OS_SET_RARP_SIGNAL(PI)              \
    OS_IFACE_SIGNAL_SET((IF_SIG_RARP),  PI) 

#define OS_TEST_INTERRUPT_SIGNAL(PI, WAIT_COUNT)    \
    OS_IFACE_SIGNAL_TEST((IF_SIG_INTERRUPT),  PI, WAIT_COUNT)
#define OS_BIND_INTERRUPT_SIGNAL(PI)                \
    OS_IFACE_SIGNAL_BIND((IF_SIG_INTERRUPT),  PI) 
#define OS_CLEAR_INTERRUPT_SIGNAL(PI)               \
    OS_IFACE_SIGNAL_CLEAR((IF_SIG_INTERRUPT),  PI) 
#define OS_SET_INTERRUPT_SIGNAL(PI)             \
    OS_IFACE_SIGNAL_SET((IF_SIG_INTERRUPT),  PI) 

#define OS_TEST_IP_SIGNAL(PI, WAIT_COUNT)   \
    OS_IFACE_SIGNAL_TEST((IF_SIG_IP),  PI, WAIT_COUNT)
#define OS_BIND_IP_SIGNAL(PI)               \
    OS_IFACE_SIGNAL_BIND((IF_SIG_IP),  PI) 
#define OS_CLEAR_IP_SIGNAL(PI)              \
    OS_IFACE_SIGNAL_CLEAR((IF_SIG_IP),  PI) 
#define OS_SET_IP_SIGNAL(PI)                \
    OS_IFACE_SIGNAL_SET((IF_SIG_IP),  PI) 

#if (INCLUDE_PPP)
#define OS_TEST_PPP_SIGNAL(PI, WAIT_COUNT)  \
    OS_IFACE_SIGNAL_TEST((IF_SIG_PPP),  PI, WAIT_COUNT)
#define OS_BIND_PPP_SIGNAL(PI)  \
    OS_IFACE_SIGNAL_BIND((IF_SIG_PPP),  PI)
#define OS_CLEAR_PPP_SIGNAL(PI) \
    OS_IFACE_SIGNAL_CLEAR((IF_SIG_PPP),  PI)
#define OS_SET_PPP_SIGNAL(PI)   \
    OS_IFACE_SIGNAL_SET((IF_SIG_PPP),  PI)
#endif

#define OS_TEST_SENT_SIGNAL(PORT, WAIT_COUNT)   \
    OS_PORT_SIGNAL_TEST((PO_SIG_SENT),  PORT, WAIT_COUNT)
#define OS_BIND_SENT_SIGNAL(PORT)   \
    OS_PORT_SIGNAL_BIND((PO_SIG_SENT),  PORT)
#define OS_CLEAR_SENT_SIGNAL(PORT)  \
    OS_PORT_SIGNAL_CLEAR((PO_SIG_SENT),  PORT)

#define OS_TEST_WRITE_SIGNAL(PORT, WAIT_COUNT)  \
    OS_PORT_SIGNAL_TEST((PO_SIG_WRITE),  PORT, WAIT_COUNT)
#define OS_BIND_WRITE_SIGNAL(PORT)              \
    OS_PORT_SIGNAL_BIND((PO_SIG_WRITE),  PORT)
#define OS_CLEAR_WRITE_SIGNAL(PORT)             \
    OS_PORT_SIGNAL_CLEAR((PO_SIG_WRITE),  PORT)

#define OS_TEST_READ_SIGNAL(PORT, WAIT_COUNT)   \
    OS_PORT_SIGNAL_TEST((PO_SIG_READ),  PORT, WAIT_COUNT)
#define OS_BIND_READ_SIGNAL(PORT)               \
    OS_PORT_SIGNAL_BIND((PO_SIG_READ),  PORT)
#define OS_CLEAR_READ_SIGNAL(PORT)              \
    OS_PORT_SIGNAL_CLEAR((PO_SIG_READ),  PORT)

#define OS_TEST_SELECT_SIGNAL(PORT, i, WAIT_COUNT)  \
    OS_PORT_SIGNAL_TEST((PO_SIG_SELECT+i),  PORT, WAIT_COUNT)
#define OS_BIND_SELECT_SIGNAL(PORT, i)              \
    OS_PORT_SIGNAL_BIND((PO_SIG_SELECT+i),  PORT)
#define OS_CLEAR_SELECT_SIGNAL(PORT, i)         \
    OS_PORT_SIGNAL_CLEAR((PO_SIG_SELECT+i),  PORT)

#if (INCLUDE_NO_DCU_BLOCK)
#define OS_TEST_DCU_SIGNAL(PORT, WAIT_COUNT)    \
    OS_PORT_SIGNAL_TEST((PO_SIG_DCU),  PORT, WAIT_COUNT)
#define OS_BIND_DCU_SIGNAL(PORT)                \
    OS_PORT_SIGNAL_BIND((PO_SIG_DCU),  PORT)
#define OS_CLEAR_DCU_SIGNAL(PORT)               \
    OS_PORT_SIGNAL_CLEAR((PO_SIG_DCU),  PORT)
#define OS_SET_DCU_SIGNAL(PORT)                 \
    OS_PORT_SIGNAL_SET((PO_SIG_DCU),  PORT)
#endif  /* INCLUDE_NO_DCU_BLOCK */

/* ********************************************************************   */
/* EXCHANGES                                                              */
/* ********************************************************************   */

#define OS_CLEAR_IP_EXCHG(PI)   \
    os_exchange_clear(IF_EX_IP, (PFVOID) PI)
#define OS_SNDX_IP_EXCHG(PI, MSG)   \
    os_exchange_send(IF_EX_IP, (PFVOID) PI, MSG)

/* Note: these procedures are functions in os_.c not macros...
**  void os_clear_udpapp_exchg(PUDPPORT port) 
**  void os_sndx_udpapp_exchg(PUDPPORT port, DCU msg)
**  DCU os_rcvx_udpapp_exchg(PUDPPORT pport, word wait_count)
*/

#define OS_BIND_UDPAPP_EXCHG(PORT)  \
    os_exchange_bind(PO_EX_UDP, (PFVOID) PORT)

#define OS_CLEAR_PING_EXCHG(PORT)   \
    os_exchange_clear(PO_EX_PING, (PFVOID) PORT)
#define OS_BIND_PING_EXCHG(PORT)    \
    os_exchange_bind(PO_EX_PING, (PFVOID) PORT)
#define OS_SNDX_PING_EXCHG(PORT, MSG)   \
    os_exchange_send(PO_EX_PING, (PFVOID) PORT, MSG)
#define OS_RCVX_PING_EXCHG(PORT, WAIT_COUNT)    \
    os_exchange_rcv(PO_EX_PING, (PFVOID) PORT, WAIT_COUNT)
#endif /* INCLUDE_RTIP */

extern KS_RTIPSEM tablesem;
extern KS_RTIPSEM memfilesem;

#if (!INCLUDE_TRK_PKTS )
#define OS_CLAIM_TABLE(X)            KS_SEMAPHORE_GET(tablesem);
#define OS_RELEASE_TABLE()           KS_SEMAPHORE_GIVE(tablesem);
#else
#define TABLE_CLAIM 0
#define OS_CLAIM_TABLE(X)            {track_sem_pend(X, TABLE_CLAIM, 0);    \
                                      KS_SEMAPHORE_GET(tablesem);            \
                                      track_sem_claim(X, TABLE_CLAIM, 0);}
#define OS_RELEASE_TABLE()           {track_sem_rel(TABLE_CLAIM, 0);        \
                                      KS_SEMAPHORE_GIVE(tablesem);}
#endif /* INCLUDE_TRK_PKTS */

#endif /* __OS__ */

