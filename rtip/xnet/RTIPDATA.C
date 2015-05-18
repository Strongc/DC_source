/* RTIPDATA.C - RTIP GLOBAL DATA                                        */
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

#undef DIAG_SECTION_KERNEL
#define DIAG_SECTION_KERNEL DIAG_SECTION_OS


#include "rtip.h"
#include "sock.h"
#include "terminal.h"

#if (INCLUDE_RTIP)
#if (INCLUDE_MTOKEN)
    #include "rtiptr.h"
#endif

#if (DO_FILE)
    #include <stdio.h>
#endif

#if (CONSOLE_DEVICE == CONDEV_TELNET)       /* - telnet server */
#include "telnet.h"
#endif

#endif      /* INCLUDE_RTIP */

/* ********************************************************************   */
/* TABLES                                                                 */
/* ********************************************************************   */
#if (INCLUDE_RTIP)
#if (INCLUDE_RUN_TIME_CONFIG)
PIFACE          ifaces;                         /* interface structures */
#else
IFACE  KS_FAR   ifaces[CFG_NIFACES];            /* interface structures */
#endif

#if (INCLUDE_ROUTING_TABLE)
#if (INCLUDE_RUN_TIME_CONFIG)
PROUTE          rt_table;
#else
ROUTE  KS_FAR   rt_table[CFG_RTSIZE];           /* routing table */
#endif
POS_LIST        root_rt_default;                /* default gateways */
#else
byte KS_FAR default_gateway_address[IP_ALEN];
#endif

/* ARP CACHE   */
#if (INCLUDE_ARP && INCLUDE_RUN_TIME_CONFIG)
ARPCACHE KS_FAR *tc_arpcache;
#elif (INCLUDE_ARP)
ARPCACHE KS_FAR tc_arpcache[CFG_ARPCLEN];
#endif

/* IP to ethernet translation table;                                    */
/* used for ethernet packets where translation is known by application, */
/* therefore, ARP is not necessary                                      */
#if (INCLUDE_ETH_BUT_NOARP && INCLUDE_RUN_TIME_CONFIG)
IP2ETH KS_FAR *ip2eth_table;
#elif (INCLUDE_ETH_BUT_NOARP)
IP2ETH KS_FAR ip2eth_table[CFG_NUM_IP2ETH_ADDR];
#endif

#if (INCLUDE_RARP_SRV)
    /* Table containing IP address an ether address of clients.           */
    /* The rarp server function will look up the IP address based on the  */
    /* RARP requestors etehernet address. The IP address will be returned */
    /* to the client.                                                     */
    /*                                                                    */
    struct rarp_table KS_FAR rt[RT_TABLE_SIZE] =
    {
        { {192,42,172,10 }, { 0x00,0x00, 0xc0, 0x6d, 0xb8, 0x17 } },  /* rachel */
        { {192,42,172,11 }, { 0x02,0x60, 0x8c, 0xad, 0x44, 0xf9 } },  /* scott */
        { {0,0,0,0 } , { 0x0,0x0, 0x0, 0x0, 0x0, 0x0 } }               /* Null term */
    };
#endif

#endif /* (INCLUDE_RTIP) */

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* RTIP API DATA                                                          */
/* ********************************************************************   */
/* See the RTFS/RTIP common section at the end for more                   */

#if (INCLUDE_RARP)
/* depth of calls to xn_rarp; used for reentrancy test   */
int KS_FAR rarp_depth;
#endif
#if (INCLUDE_BOOTP)
/* depth of calls to xn_bootp; used for reentrancy test   */
int KS_FAR bootp_depth;
#endif

/* default interface for xn_interface_mcast if -1 is passed as the   */
/* interface (the value is set by xn_interface_opt)                  */
int KS_FAR default_mcast_iface;

/* initialization functions   */
P_INIT_FNCS registered_fncs = 0;

#if (INCLUDE_ERROR_STRING_NUMBER)
char errno_buf[128];
#endif

/* Pointer to device table   */
PDEVTABLE devices_ptr;
#if (!BUILD_NEW_BINARY && !INCLUDE_RUN_TIME_CONFIG)
EDEVTABLE KS_FAR devices[CFG_NUM_DEVICES];
#endif

/* ********************************************************************   */
/* IP DATA                                                                */
/* ********************************************************************   */
word  KS_FAR ipsequence;                /* ip packet numbers start here */
                                        /* and increment   */
dword KS_FAR ip_forwarding;             /* 1=can forward; 2=can't forward */
                                        /* can be modified by SNMP   */

/* ********************************************************************   */
/* SOCKET DATA                                                            */
/* ********************************************************************   */

/* global data returned by inet_ntoa                             */
/* NOTE: user must extract the data from ntoa_str before calling */
/*       inet_ntoa again                                         */
char KS_FAR ntoa_str[20];

/* ********************************************************************   */
/* DNS                                                                    */
/* ********************************************************************   */

#if (INCLUDE_DNS)

/* h_table is the user host table. Names which have known IP addresses    */
/* can be added to this table using API calls. This will make the service */
/* of these domain names much faster.  This host table is the first place */
/* that gethostbyname() will look for names. The host_cache is the second */
/* place, and the name server is the third.                               */
struct  hostentext KS_FAR h_table[CFG_MAX_HT_ENT];

/* host_cache is the internal host cache.                                  */
/* If caching is disabled, every time that gethostbyname()/gethostbyaddr() */
/* is called, it adds an entry to this cache and returns a pointer to the  */
/* cached entry.                                                           */
/* If caching is enabled, gethostbyname()/gethostbyaddr() will look for    */
/* entry in cache and return it if found.  If not found, it performs       */
/* DNS lookup and adds an entry to cache.  If free entry is not found,     */
/* the entry with the smallest ttl is used                                 */
struct  hostentext  KS_FAR host_cache[CFG_MAX_HC_ENT];

/* host_cache[host_cache_index] is the current cached name entry. When   */
/* gethostbyname() and gethostbyaddr() finds a host, it will cache the   */
/* name at the index+1; xn_add_host_cache_entry also adds entries to     */
/* the host_cache                                                        */
int     host_cache_index;                    /* index to host cache */

/* the server_ip_table is a list of IP addresses.  Both gethostbyname and   */
/* gethostbyaddr loop through this list looking for name servers.           */
/* addresses are in network byte order                                      */
dword KS_FAR server_ip_table[CFG_MAX_DNS_SRV];  /* 32-bit IP addresses */


#endif
int  KS_FAR cnt_dns;    /* number of valid DNS servers */
char KS_FAR my_domain_name[CFG_DNS_NAME_LEN];

/* ********************************************************************   */
/* OS DATA                                                                */
/* ********************************************************************   */

#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
POS_LIST root_dcu;                          /* head of the free DCU list */
                                            /* Only used at startup   */
int KS_FAR dcu_size_array[CFG_NUM_FREELISTS]; /* Table of sizes of DCUs */
                                              /* managed by root_dcu_array[]   */
POS_LIST root_dcu_array[CFG_NUM_FREELISTS];/* head of the free DCU list */
                                           /* Used to manage free list by   */
                                           /* one list per dcu size         */
#if (INCLUDE_NO_DCU_BLOCK)
POS_LIST ports_blocked_for_dcu;
#endif

/* lowest number of free packet ever   */
int KS_FAR lowest_free_packets_array[CFG_NUM_FREELISTS];

/* lowest number of free packets now   */
int KS_FAR current_free_packets_array[CFG_NUM_FREELISTS];

/* higest number of free packet ever   */
int KS_FAR highest_free_packets_array[CFG_NUM_FREELISTS];

/* pointers to DCUs and packets   */
EPACKET KS_FAR *dcu_pool_ptr;
#endif      /* !INCLUDE_MALLOC_DCU_AS_NEEDED */

#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
int KS_FAR current_alloc_packets;
int KS_FAR highest_alloc_packets;
#endif  /* INCLUDE_MALLOC_DCU_AS_NEEDED */

/* ********************************************************************   */
/* Pools of memory that are threaded onto freelist by os_memory_init().
   they may be replaced by calling an allocator function instead of
   declaring them externally. The memory doesnt have to be contiguous

   Note: Some architectures limit an array size (intel real mode for example)
   We may want to allocate > a segment's worth so to do so we allocate two
   arrays. More could be added if needed
*/

#if (INCLUDE_MALLOC_DCU_INIT)
PFBYTE packet_pool0;
PFBYTE packet_pool1;
PFBYTE packet_pool2;
PFBYTE packet_pool3;
PFBYTE packet_pool4;
PFBYTE packet_pool5;
#endif


#if (!INCLUDE_MALLOC_DCU_INIT && !INCLUDE_MALLOC_DCU_AS_NEEDED)
/* DCU structures   */
EPACKET KS_FAR tc_dcu_pool[CFG_NDCUS];

/* DATA AREA FOR DCUs   */
#if (CFG_NUM_PACKETS0)
word KS_HUGE packet_pool0[CFG_NUM_PACKETS0]
                         [(CFG_PACKET_SIZE0+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS1)
word KS_HUGE packet_pool1[CFG_NUM_PACKETS1]
                         [(CFG_PACKET_SIZE1+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS2)
word KS_HUGE packet_pool2[CFG_NUM_PACKETS2]
                         [(CFG_PACKET_SIZE2+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS3)
word KS_HUGE packet_pool3[CFG_NUM_PACKETS3]
                         [(CFG_PACKET_SIZE3+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS4)
word KS_HUGE packet_pool4[CFG_NUM_PACKETS4]
                         [(CFG_PACKET_SIZE4+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS5)
word KS_HUGE packet_pool5[CFG_NUM_PACKETS5]
                         [(CFG_PACKET_SIZE5+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#endif /* !INCLUDE_MALLOC_DCU_INIT && !INCLUDE_MALLOC_DCU_AS_NEEDED */

/* total number of packets and dcus allocated   */
int KS_FAR ndcus_alloced;

/* ********************************************************************   */
/* POOLS OF PORTS                                                         */

/* array of allocated port pointers   */
#if (INCLUDE_MALLOC_PORTS || INCLUDE_RUN_TIME_CONFIG)
/* array of allocated port pointers   */
PANYPORT KS_FAR *alloced_ports;
#else
PANYPORT KS_FAR alloced_ports[TOTAL_PORTS];
#endif

#if (!INCLUDE_MALLOC_PORTS)
#if (INCLUDE_UDP || INCLUDE_RAW)
    POS_LIST root_udp_port;         /* head of the free UDP port list */
#if (INCLUDE_RUN_TIME_CONFIG)
    PUDPPORT tc_udpp_pool;           /* UDP Ports pointer */
#else
    UDPPORT KS_FAR tc_udpp_pool[CFG_NUDPPORTS];   /* UDP Ports */
#endif
#endif      /* INCLUDE_UDP || INCLUDE_RAW */

#if (INCLUDE_TCP)
    POS_LIST root_tcp_port;         /* head of the free TCP port list */
#if (INCLUDE_RUN_TIME_CONFIG)
    PTCPPORT tc_tcpp_pool;                      /* TCP ports pointer */
#else
    TCPPORT KS_FAR tc_tcpp_pool[CFG_NTCPPORTS]; /* TCP ports */
#endif
#endif      /* INCLUDE_TCP */

#endif      /* !INCLUDE_MALLOC_PORTS */

/* ********************************************************************   */
#if (INCLUDE_SYSLOG && INCLUDE_BGET)
    char KS_FAR syslog_mempool[CFG_SYSLOG_BGET_CORE_SIZE+BGET_OVERHEAD];
#endif  /* (INCLUDE_SYSLOG && INCLUDE_BGET) */

#if (INCLUDE_SSL && INCLUDE_BGET)
    char KS_FAR ssl_mempool[CFG_SSL_BGET_CORE_SIZE+BGET_OVERHEAD];
#endif  /* (INCLUDE_SSL && INCLUDE_BGET) */

int KS_FAR lowest_free_packets;     /* lowest number of free packet ever */
int KS_FAR current_free_packets;    /* lowest number of free packets now */

/* If this value is one or more the device layer ks_invoke_input() accepts   */
/* UDP broadcasts. Otherwise it throws them away. Each port that wishes      */
/* to receive UDP broadcasts increments this variable and decrements         */
/* it when it no longer wants them                                           */
int KS_FAR allow_udp_broadcasts;

#endif /* INCLUDE_RTIP */


#if (KS_NUM_INTS > 0) /*OS*/
/* ********************************************************************   */
/* OSINT DATA                                                             */
/* ********************************************************************   */
/* When interrupt N (n = 0-KS_NUM_INTS) comes through we will call
    rtip_isr_strategy[N](rtip_args[N]);
*/
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS)
/* If doing the strategy from task level we leave the option to      */
/* call a routine from the interrupt which may have to mask off      */
/* interrupts or do some early processing (this will usually be null */
RTIPINTFN_POINTER rtip_isr_interrupt[KS_NUM_INTS];
PIFACE KS_FAR rtip_irq_iface[KS_NUM_INTS];
#endif

RTIPINTFN_POINTER rtip_isr_strategy[KS_NUM_INTS];
int KS_FAR rtip_args[KS_NUM_INTS];

#endif /*OS*/

/* depth of currents calls to interrupt service   */
int KS_FAR in_irq;

#if (INCLUDE_RTIP)
#if (INCLUDE_MODEM)
/* ********************************************************************   */
/* MODEM                                                                  */
/* ********************************************************************   */
/* buffer used to pass input chars from modem to RTIP                     */
PFCHAR rawdata;

/* default script used to dial a modem where login is required   */
struct al_command KS_FAR default_script_login[] =
{
    {AL_PRINT,      "AL: Logging in . . .",    0},
    {AL_SEND,       "ATM1L1",                  0},      /* setup string goes here */
    {AL_WAIT,       "OK",                      5},
    {AL_BRKERR,     "error1",                  0},
    {AL_SEND,       "ATDT5555555",             0},      /* phone number goes here */
    {AL_WAITLIST,   "CONNECT",                 240},
    {AL_WAITLIST,   "BUSY",                    0},
    {AL_BRKFND,     "error5",                  0},
    {AL_WAITLIST,   "NO DIAL",                 0},
    {AL_BRKFND,     "error6",                  0},
    {AL_BRKERR,     "error4",                  0},
    {AL_WAITLIST,   "ogin:",                   40},
    {AL_WAITLIST,   "sername:",                0},
    {AL_BRKERR,     "error2",                  0},
    {AL_SEND,       "johndoe",                 0},      /* username goes here */
    {AL_WAIT,       "ssword",                  10},
    {AL_BRKERR,     "error3",                  0},
    {AL_SEND,       "xxx",                     0},      /* password goes here */
    {AL_END,        "",                        0},
    {AL_LABEL,      "error1",                  0},
    {AL_PRINT,      "AL: Modem not responding",0},
    {AL_ENDERR,     "",                        0},
    {AL_LABEL,      "error2",                  0},
    {AL_PRINT,      "AL: Login failed.",       0},
    {AL_ENDERR,     "",                        0},
    {AL_LABEL,      "error3",                  0},
    {AL_PRINT,      "AL: No passwd prompt",    0},
    {AL_ENDERR,     "",                        0},
    {AL_LABEL,      "error4",                  0},
    {AL_PRINT,      "AL: No response to dial", 0},
    {AL_ENDERR,     "",                        0},
    {AL_LABEL,      "error5",                  0},
    {AL_PRINT,      "AL: Busy",                0},
    {AL_ENDERR,     "",                        0},
    {AL_LABEL,      "error6",                  0},
    {AL_PRINT,      "AL: NO DIAL TONE",        0},
    {AL_ENDERR,     "",                        0},
};

/* Pointer to structure; this is done this way due to avoid some compilers   */
/* giving you an error if you extern an array without its size               */
struct al_command KS_FAR *default_script_login_ptr = default_script_login;

/* default script used to dial a modem with no login required   */
struct al_command KS_FAR default_script[] =
{
    {AL_PRINT,      "AL: Logging in . . .",    0},
    {AL_SEND,       "ATM1L1",                  0},      /* setup string goes here */
    {AL_WAIT,       "OK",                      5},
    {AL_BRKERR,     "error1",                  0},
    {AL_SEND,       "ATDT5555555",             0},      /* phone number goes here */
    {AL_WAITLIST,   "CONNECT",                 60},
    {AL_WAITLIST,   "BUSY",                    0},
    {AL_BRKFND,     "error3",                  0},
    {AL_WAITLIST,   "NO DIAL",                 0},
    {AL_BRKFND,     "error4",                  0},
    {AL_BRKERR,     "error2",                  0},
    {AL_END,        "",                        0},
    {AL_LABEL,      "error1",                  0},
    {AL_PRINT,      "AL: Modem not responding",0},
    {AL_ENDERR,     "",                        0},
    {AL_LABEL,      "error2",                  0},
    {AL_PRINT,      "AL: Modem not connected", 0},
    {AL_ENDERR,     "",                        0},
    {AL_LABEL,      "error3",                  0},
    {AL_PRINT,      "AL: Busy",                0},
    {AL_ENDERR,     "",                        0},
    {AL_LABEL,      "error4",                  0},
    {AL_PRINT,      "AL: NO DIAL TONE",        0},
    {AL_ENDERR,     "",                        0},
};

/* Pointer to structure; this is done this way due to avoid some compilers   */
/* giving you an error if you extern an array without its size               */
struct al_command KS_FAR *default_script_ptr = default_script;

#endif      /* INCLUDE_MODEM */
#endif      /* INCLUDE_RTIP */

/* ********************************************************************   */
/* OS PORT DATA                                                           */
/* ********************************************************************   */

#if (INCLUDE_RTIP)
/* used to pass task number to tasks spawned   */
int KS_FAR static_iface_no = 0;
int KS_FAR static_interrupt_no = 0;

/* Kernel independent  declaration of semaphores and signals.
   The types are defined in osport.h */
#if (!INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
KS_RTIPSIG portsig[TOTAL_PORTS][NUM_SIG_PER_PORT];
#endif

#if (INCLUDE_ERTFS_PRO)
dword ertfssignexthandle = 0;
KS_RTIPSIG ertfssigs[ERTFS_PRO_NSIGS];
#endif

#if (defined(PEGRTIP))

#ifdef __cplusplus
extern "C" {
#endif

RTIP_BOOLEAN hand_kb_to_peg = FALSE;
RTIP_BOOLEAN    hand_timer_to_peg = FALSE;

KS_RTIPSIG ks_peg_sig[NUM_PEG_EVENTS];
KS_RTIPSEM ks_peg_sem[NUM_PEG_SEM];
#ifdef __cplusplus
}
#endif
#endif

/* SEMAPHORES                                                 */
/* NOTE: iface semaphores are attached to interface structure */
KS_RTIPSEM criticalsem;
KS_RTIPSEM tcpsem;
KS_RTIPSEM udpsem;
KS_RTIPSEM syslogsem;
KS_RTIPSEM tablesem;
KS_RTIPSEM memfilesem;
#if (INCLUDE_ERTFS_PRO)
dword ertfssemnexthandle = 0;
KS_RTIPSEM ertfssems[ERTFS_PRO_NSEMS];
#endif

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* RS232 DATA                                                             */
/* ********************************************************************   */

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
    RS232_IF_INFO KS_FAR rs232_if_info_arry[CFG_NUM_RS232];
#endif

#if (INCLUDE_PPP)
/* *********************************************************************   */
/* PPP                                                                     */
/* *********************************************************************   */
PPP_XMIT_ESCAPE_FNC ppp_escape_routine;
PPP_GIVE_STRING_FNC ppp_give_string_routine;
#endif

/* ********************************************************************   */
/* TCP                                                                    */
/* ********************************************************************   */

#if (INCLUDE_TCP)
/* TCP LISTS which include:                                               */
/* offset LISTEN_LIST: Head of list of listener TCP ports                 */
/* offset SOCKET_LIST: Head of list of allocated but not active TCP ports */
/* offset ACTIVE_LIST: Head of list of active TCP ports                   */
/* offset MASTER_LIST: Head of list of master sockets that had a problem  */
/*                     (probably allocating a new socket) during          */
/*                     tcp_interpret; i.e. a SYNC came in and there       */
/*                     was room in the backlog but listen() failed        */
POS_LIST root_tcp_lists[NUM_TCP_LISTS];

word KS_FAR tcp_port_number;     /* port number to use for new sockets */
RTIP_BOOLEAN KS_FAR ka_send_garbage;     /* if set, keepalive will send a garbage */
                                 /* byte   */
long KS_FAR default_tcp_options; /* default options when allocating a */
                                 /* TCP socket   */
#if (INCLUDE_TCP_OUT_OF_ORDER)
RTIP_BOOLEAN KS_FAR free_ooo_lists;  /* flag for alloc_packet to tell */
                                 /* timeout routine to free all out of   */
                                 /* order packet                         */
#endif
#if (INCLUDE_TCP_PORT_CACHE)
PTCPPORT KS_FAR cached_tcp_port; /* This value stores the last port */
                                /* we found in tcp_interpret.   */
#endif

#if (INCLUDE_OOO_QUE_LIMIT)
int KS_FAR num_ooo_que;          /* number of DCUs on the out of order list */
#endif
#endif      /* INCLUDE_TCP */

/* ********************************************************************   */
/* UDP                                                                    */
/* ********************************************************************   */
#if (INCLUDE_UDP)
/* list of heads of UDP socket lists where:                               */
/* offset ACTIVE_LIST: Head of list of active UDP ports                   */
/* offset SOCKET_LIST: Head of list of allocated but not active UDP ports */
POS_LIST root_udp_lists[NUM_UDP_LISTS];

word KS_FAR udp_port_number;    /* port number used by socket to assign */
                                /* unique port numbers to sockets   */
#if (INCLUDE_UDP_PORT_CACHE)
PUDPPORT KS_FAR cached_udp_port; /* This value stores the last port */
                                /* we found in udp_interpret.        */
                                /* We only save it if port number is */
                                /* unique                            */
#endif
#endif      /* INCLUDE_UDP */

#if (INCLUDE_UDP || INCLUDE_RAW)
long KS_FAR default_udp_options;      /* default options when allocating a */
                                     /* UDP socket   */
#endif

#if (INCLUDE_RAW)
/* ********************************************************************   */
/* RAW                                                                    */
/* ********************************************************************   */
/* list of heads of TCP socket lists where:                               */
/* offset ACTIVE_LIST: Head of list of active UDP ports                   */
/* offset SOCKET_LIST: Head of list of allocated but not active UDP ports */
POS_LIST root_raw_lists[NUM_UDP_LISTS];
#endif


/* ********************************************************************   */
/* ARP                                                                    */
/* ********************************************************************   */
dword KS_FAR arpc_res_tmeout;

/* ********************************************************************   */
/* DEVICE INFORMATION                                                     */
/* ********************************************************************   */

/* size of device table; setup at initialization   */
int KS_FAR total_devices;

/* total number of valid entries in device table   */
int KS_FAR valid_device_entries;

/* ********************************************************************   */
/* DRIVERS                                                                */
/* ********************************************************************   */

#if (!BUILD_NEW_BINARY)
#define DECLARING_DATA 1

#if (INCLUDE_PKT)
#include "packet.c"
#endif

#if (INCLUDE_NE2000)
#include "ne2000.c"
#endif

#if (GPL_DRIVERS)
#include "3c509.c"
#include "3c589.c"
#include "xircom.c"
#include "eepro595.c"
#endif

#if (INCLUDE_ED )
#include "ifed.c"
/*#include "ifedshse.c"   */
#endif

#if (INCLUDE_SMC91C9X)
#include "smc91c9x.c"
#endif

#if(INCLUDE_CS89X0)
/* replaced by lan8900.c for other platforms     */
#include "cs89x0.c"
#endif

#if (INCLUDE_RTLANCE)
#include "rtlance.c"
#endif

#if (INCLUDE_LANCE_ISA)
#include "lanceisa.c"
#endif

#if (INCLUDE_I82559)
#include "i82559.c"
#endif

#if (INCLUDE_LAN89X0)
#include "lan8900.c"
#endif

#if (INCLUDE_TC90X)
#include "3c90x.c"
#endif

#if (INCLUDE_R8139)
#include "r8139.c"
#endif

#if (INCLUDE_N83815)
#include "83815.c"
#endif

/* not included becuase it is not an embedded system   */
/* the data for this driver isn't an issue             */
/* #include "winether.c"                               */

#if (INCLUDE_LOOP)
#include "loop.c"
#endif

#if (INCLUDE_PRISM)
#include "prismapi.c"
#endif

#if (INCLUDE_DAVICOM)
#include "dm9102a.c"
#endif

#if (INCLUDE_SLIP)
#include "slip.c"
#endif
#undef DECLARING_DATA
#endif  /* !BUILD_NEW_BINARY */

#if (INCLUDE_MTOKEN)
    MTOKDEVICE KS_FAR mdevicesoftc[CFG_NUM_MTOKEN];
    MTOK_ADAPTER KS_FAR madaptersoftc[CFG_NUM_MTOKEN];
#endif

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_ED || USE_PCVID_OUTPUT || INCLUDE_LANCE || INCLUDE_PCMCIA || INCLUDE_ERTFS)

#endif

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* FRAGMENT.C                                                             */
/* ********************************************************************   */

#if (INCLUDE_FRAG)
#if (INCLUDE_RUN_TIME_CONFIG)
IP_FRAGLIST KS_FAR *frag_table;
#else
IP_FRAGLIST KS_FAR frag_table[CFG_FRAG_TABLE_SIZE];
#endif
dword KS_FAR max_frag_size;
#endif

#endif /* (INCLUDE_RTIP) */

/* ********************************************************************   */
/* TOOLS.C                                                                */
/* ********************************************************************   */


/* ********************************************************************   */
/* DEBUG data                                                             */
/* ********************************************************************   */
#if (!USE_DB_L0)
    word    KS_FAR say_port = SAY_COMM_PORT;
    int     KS_FAR write_to_file;
    RTIP_BOOLEAN KS_FAR say_init_done = FALSE;

    RTIP_BOOLEAN KS_FAR print_debug_log = TRUE; /*OS*/ /* was FALSE; */
#    if (DO_FILE)
        FILE    *say_fp = 0;
#    endif

    /* Select COMM portBase and interrupt number     */
    IOADDRESS sayportBase;   /* I/O address for COM */
    word     KS_FAR saycommIRQ;    /* IRQ number for COM */
#endif


/* ********************************************************************   */
/* OSPORT.C                                                               */
/* ********************************************************************   */

/* set when RTIP has been initialized by xn_rtip_init   */
RTIP_BOOLEAN KS_FAR rtip_initialized = FALSE;

/* set when RTIP has been exited by xn_rtip_exit   */
RTIP_BOOLEAN KS_FAR rtip_exited = FALSE;

/* set when resources initialized by xn_rtip_init (by RTIP) or by   */
/* pc_kernel_init (by ERTFS);                                       */
/* possible values are INIT_NOT_DONE, INIT_IN_PROG and INIT_DONE    */
int KS_FAR resource_initialized = INIT_NOT_DONE;

/* set when task templates and stack pool manager initialized by the   */
/* first call to ks_bind_task_index() or os_spawn_task()               */
RTIP_BOOLEAN KS_FAR kernel_initialized = FALSE;

/* ********************************************************************   */
/* POLLOS.C                                                               */
/* ********************************************************************   */
#if (INCLUDE_RIP)
/* ********************************************************************   */
/* RIP                                                                    */
/* ********************************************************************   */
byte rip_buffer[RIP_MAX_PKTLEN] = {0};
byte rip_version_running = 0;
int  rip_udp_socket = -1;
int  rip_secs_to_update = 5;
int  rip_update_flag = 0;
#endif /* INCLUDE_RIP */

#if (INCLUDE_SNTP)
/* ********************************************************************   */
/* SNTP                                                                   */
/* ********************************************************************   */
#endif

/* ********************************************************************   */
/* TASK                                                                   */
/* ********************************************************************   */
/* Head of running timer chain.
 * The list of running timers is sorted in increasing order of expiration;
 * i.e., the first timer to expire is always at the head of the list.
 */
PTIMER ebs_one_sec_timers;  /* multiple seconds timers */
PTIMER ebs_timers;          /* every time timer task runs */

int KS_FAR timer_freq;  /* frequency timer task should run; set to CFG_TIMER_FREQ */
                        /* if there are any TCP sockets or 1 sec if no TCP sockets   */
                        /* are allocated                                             */
                        /* UNITS: msecs                                              */

/* the following data needs to be global for POLLOS only since the   */
/* POLLOS version of tc_timer_main is called every interval versus   */
/* looping forever while sleeping every loop                         */
#if (INCLUDE_MEASURE_PERFORMANCE)
/* ********************************************************************   */
/* MEASURE PERFORMANCE PERFORMANCE                                        */
/* ********************************************************************   */
RTIP_BOOLEAN do_sock_arp_cache = FALSE;
RTIP_BOOLEAN do_sock_udp_cache = FALSE;
RTIP_BOOLEAN do_sock_tcp_cache = FALSE;
#endif

/* ********************************************************************   */
/* TASK MANAGEMENT                                                        */
/* ********************************************************************   */
/* Task management data. These structures are used exclusively by the     */
/* porting layer (osport.h) to manage tasks                               */
/* Task properties (stack size and priority by class)                     */
EBS_TASK_CLASS KS_FAR class_templates[TASKCLASS_EXTERN_APPTASK+1];

/* Task context block. One per task                                   */
/* NOTE: 0 is reserved for application task when calling os_exit_task */
/*#if (USE_RTIP_TASKING_PACKAGE)                                      */
EBS_TASK_CONTEXT KS_FAR task_contexts[CFG_NUM_TASK_CONTROL_BLOCKS];
/*#endif   */

/*  Flag that tells the timer task to call ks_kernel_timer_callback() on   */
/*  the next tick because the kernel layer needs to process a task delete  */
/*  operation                                                              */
int KS_FAR ks_data_run_timer;

#if (KS_DECLARE_STACK)
KS_STACK_TYPE normal_1_stacks_array[CFG_N_STACKS_NORMAL_1][SIZESTACK_NORMAL];
#if (CFG_N_STACKS_NORMAL_2)
KS_STACK_TYPE normal_2_stacks_array[CFG_N_STACKS_NORMAL_2][SIZESTACK_NORMAL];
#endif
#if (CFG_N_STACKS_BIG)
KS_STACK_TYPE big_stacks_array[CFG_N_STACKS_BIG][SIZESTACK_BIG];
#endif
#if (CFG_N_STACKS_HUGE)
KS_STACK_TYPE huge_stacks_array[CFG_N_STACKS_HUGE][SIZESTACK_HUGE];
#endif
struct  stack_manager KS_FAR sm_array[CFG_N_STACKS_TOTAL];
#endif  /* KS_DECLARE_STACK */

/* ********************************************************************   */
/* SERVERS                                                                */
/* ********************************************************************   */
#if (INCLUDE_BGET && INCLUDE_MALLOC)
//char KS_FAR context_core[CFG_CONTEXT_CORE];
int KS_FAR context_core[CFG_CONTEXT_CORE];
#endif

/* ********************************************************************   */
/* TERMINAL                                                               */
/* ********************************************************************   */
RTIP_BOOLEAN KS_FAR terminal_is_open;
#if (CONSOLE_DEVICE == CONDEV_TELNET)       /* - telnet server */
PTELNET_CONTEXT telnet_console_context;
#endif


/* ********************************************************************   */
/* CALLBACKS                                                              */
/* ********************************************************************   */
PRTIP_CALLBACKS rtip_callbacks = 0;

#if (INCLUDE_SNMP)
/* ********************************************************************   */
/* MIB variables                                                          */
/* ********************************************************************   */

#if (INCLUDE_TRAPS)
/* function to call to send trap; set up when SNMP initialized   */
SEND_TRAP_FNC snmp_trap_fnc = 0;
#endif

#if (!USE_OFFSET)
/* data structure containing all the mib variables   */
struct mib_variables KS_FAR mib_vars;
#endif

/* variables used to calculate MIB variables   */
dword KS_FAR sys_up_time;
#endif /* INCLUDE_SNMP */
