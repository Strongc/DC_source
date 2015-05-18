/* RTIPEXT.H - RTIP EXTERNAL GLOBAL DATA DECLARATIONS                   */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains all the extern global data declarations    */
/*      for RTIP.                                                       */

#ifndef __RTIPEXT__
#define __RTIPEXT__ 1


#include "rtip.h"
#if (INCLUDE_TERMINAL)
#include "terminal.h"
#endif
#if (INCLUDE_RTIP)
#include "sock.h"

#if (INCLUDE_MTOKEN)
    #include "rtiptr.h"
#endif

#if (DO_FILE)
    #include <stdio.h>
#endif

#if (INCLUDE_VFS)
#include "vfile.h"
#if (INCLUDE_MFS)
    #include "memfile.h"
#endif
#endif

#if (INCLUDE_SSL)
#include "vsslapi.h"
#endif

#if (CONSOLE_DEVICE == CONDEV_TELNET)       /* - telnet server */
#include "telnet.h"
#endif

#endif      /* INCLUDE_RTIP */

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************   */
/* TABLES                                                                 */
/* ********************************************************************   */
#if (INCLUDE_RTIP)
#if (INCLUDE_RUN_TIME_CONFIG)
extern PIFACE        ifaces;                        /* interface structures */
#else
extern IFACE  KS_FAR ifaces[CFG_NIFACES];           /* interface structures */
#endif
#if (INCLUDE_ROUTING_TABLE)
#if (INCLUDE_RUN_TIME_CONFIG)
extern PROUTE rt_table;
#else
extern ROUTE KS_FAR rt_table[CFG_RTSIZE];           /* routing table */
#endif
extern POS_LIST     root_rt_default;                /* default gateways */
#else
extern byte KS_FAR default_gateway_address[IP_ALEN];
#endif

/* ARP CACHE   */
#if (INCLUDE_ARP && INCLUDE_RUN_TIME_CONFIG)
extern ARPCACHE KS_FAR *tc_arpcache;
#elif (INCLUDE_ARP)
extern ARPCACHE KS_FAR tc_arpcache[CFG_ARPCLEN];
#endif

/* IP to ethernet translation table;                                    */
/* used for ethernet packets where translation is known by application, */
/* therefore, ARP is not necessary                                      */
#if (INCLUDE_ETH_BUT_NOARP && INCLUDE_RUN_TIME_CONFIG)
extern IP2ETH KS_FAR *ip2eth_table;
#elif (INCLUDE_ETH_BUT_NOARP)
extern IP2ETH KS_FAR ip2eth_table[CFG_NUM_IP2ETH_ADDR];
#endif

#if (INCLUDE_RARP_SRV)
    /* Table containing IP address an ether address of clients.           */
    /* The rarp server function will look up the IP address based on the  */
    /* RARP requestors etehernet address. The IP address will be returned */
    /* to the client.                                                     */
    /*                                                                    */
    extern struct rarp_table KS_FAR rt[RT_TABLE_SIZE];
#endif

#endif /* (INCLUDE_RTIP) */

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* RTIP API DATA                                                          */
/* ********************************************************************   */
/* See the RTFS/RTIP common section at the end for more                   */

#if (INCLUDE_RARP)
/* depth of calls to xn_rarp; used for reentrancy test   */
extern int  KS_FAR rarp_depth;
#endif
#if (INCLUDE_BOOTP)
/* depth of calls to xn_bootp; used for reentrancy test   */
extern int  KS_FAR bootp_depth;
#endif

/* default interface for xn_interface_mcast if -1 is passed as the   */
/* interface (the value is set by xn_interface_opt)                  */
extern int KS_FAR default_mcast_iface;

/* initialization functions   */
extern P_INIT_FNCS registered_fncs;

#if (INCLUDE_ERROR_STRING_NUMBER)
extern char errno_buf[128];
#endif

/* Pointer to device table   */
extern PDEVTABLE devices_ptr;
#if (!BUILD_NEW_BINARY && !INCLUDE_RUN_TIME_CONFIG)
extern EDEVTABLE KS_FAR devices[CFG_NUM_DEVICES];
#endif

/* ********************************************************************   */
/* IP DATA                                                                */
/* ********************************************************************   */
extern word  KS_FAR ipsequence;         /* ip packet numbers start here */
                                        /* and increment   */
extern dword KS_FAR ip_forwarding;      /* 1=can forward; 2=can't forward */
                                        /* can be modified by SNMP   */

/* ********************************************************************   */
/* SOCKET DATA                                                            */
/* ********************************************************************   */

/* global data returned by inet_ntoa                             */
/* NOTE: user must extract the data from ntoa_str before calling */
/*       inet_ntoa again                                         */
extern char KS_FAR ntoa_str[20];

/* ********************************************************************   */
/* DNS                                                                    */
/* ********************************************************************   */

#if (INCLUDE_DNS)

/* h_table is the user host table. Names which have known IP addresses    */
/* can be added to this table using API calls. This will make the service */
/* of these domain names much faster.  This host table is the first place */
/* that gethostbyname() will look for names. The host_cache is the second */
/* place, and the name server is the third.                               */
extern struct   hostentext KS_FAR h_table[CFG_MAX_HT_ENT];

/* host_cache is the internal host cache.                                  */
/* If caching is disabled, every time that gethostbyname()/gethostbyaddr() */
/* is called, it adds an entry to this cache and returns a pointer to the  */
/* cached entry.                                                           */
/* If caching is enabled, gethostbyname()/gethostbyaddr() will look for    */
/* entry in cache and return it if found.  If not found, it performs       */
/* DNS lookup and adds an entry to cache.  If free entry is not found,     */
/* the entry with the smallest ttl is used                                 */
extern struct   hostentext  KS_FAR host_cache[CFG_MAX_HC_ENT];

/* host_cache[host_cache_index] is the current cached name entry. When   */
/* gethostbyname() and gethostbyaddr() finds a host, it will cache the   */
/* name at the index+1; xn_add_host_cache_entry also adds entries to     */
/* the host_cache                                                        */
extern int host_cache_index;                     /* index to host cache */

/* the server_ip_table is a list of IP addresses.  Both gethostbyname and   */
/* gethostbyaddr loop through this list looking for name servers.           */
/* addresses are in network byte order                                      */
extern dword KS_FAR server_ip_table[CFG_MAX_DNS_SRV];   /* 32-bit IP addresses */


#endif
extern int  KS_FAR cnt_dns; /* number of valid DNS servers */
extern char KS_FAR my_domain_name[CFG_DNS_NAME_LEN];

/* ********************************************************************   */
/* OS DATA                                                                */
/* ********************************************************************   */

#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
extern POS_LIST root_dcu;                           /* head of the free DCU list */
                                            /* Only used at startup   */
extern int KS_FAR dcu_size_array[CFG_NUM_FREELISTS]; /* Table of sizes of DCUs */
                                              /* managed by root_dcu_array[]   */
extern POS_LIST root_dcu_array[CFG_NUM_FREELISTS];/* head of the free DCU list */
                                           /* Used to manage free list by   */
                                           /* one list per dcu size         */
#if (INCLUDE_NO_DCU_BLOCK)
extern POS_LIST ports_blocked_for_dcu;
#endif
/* lowest number of free packet ever   */
extern int KS_FAR lowest_free_packets_array[CFG_NUM_FREELISTS];

/* lowest number of free packets now   */
extern int KS_FAR current_free_packets_array[CFG_NUM_FREELISTS];

/* higest number of free packet ever   */
extern int KS_FAR highest_free_packets_array[CFG_NUM_FREELISTS];

/* pointers to DCUs and packets   */
extern EPACKET KS_FAR *dcu_pool_ptr;
#endif      /* !INCLUDE_MALLOC_DCU_AS_NEEDED */

#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
extern int KS_FAR current_alloc_packets;
extern int KS_FAR highest_alloc_packets;
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
extern PFBYTE packet_pool0;
extern PFBYTE packet_pool1;
extern PFBYTE packet_pool2;
extern PFBYTE packet_pool3;
extern PFBYTE packet_pool4;
extern PFBYTE packet_pool5;
#endif


#if (!INCLUDE_MALLOC_DCU_INIT && !INCLUDE_MALLOC_DCU_AS_NEEDED)
/* DCU structures   */
extern EPACKET KS_FAR tc_dcu_pool[CFG_NDCUS];

/* DATA AREA FOR DCUs   */
#if (CFG_NUM_PACKETS0)
extern word KS_HUGE packet_pool0[CFG_NUM_PACKETS0]
                         [(CFG_PACKET_SIZE0+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS1)
extern word KS_HUGE packet_pool1[CFG_NUM_PACKETS1]
                         [(CFG_PACKET_SIZE1+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS2)
extern word KS_HUGE packet_pool2[CFG_NUM_PACKETS2]
                         [(CFG_PACKET_SIZE2+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS3)
extern word KS_HUGE packet_pool3[CFG_NUM_PACKETS3]
                         [(CFG_PACKET_SIZE3+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS4)
extern word KS_HUGE packet_pool4[CFG_NUM_PACKETS4]
                         [(CFG_PACKET_SIZE4+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#if (CFG_NUM_PACKETS5)
extern word KS_HUGE packet_pool5[CFG_NUM_PACKETS5]
                         [(CFG_PACKET_SIZE5+CFG_PACKET_ADJ+PKT_GUARD_SIZE)/2];
#endif

#endif /* !INCLUDE_MALLOC_DCU_INIT && !INCLUDE_MALLOC_DCU_AS_NEEDED */

#if (INCLUDE_MALLOC_DCU_INIT)
    /* total number of packets to allocate (most of these will be attached    */
    /* to DCUs); initialized to CFG_NPACKETS but you should change this value */
    /* before calling xn_rtip_init in order to allocate a different           */
    /* number of packets                                                      */

    /* total number of DCUs to allocate;                            */
    /* initialized to CFG_NDCUS but you should change this value    */
    /* before calling xn_rtip_init in order to allocate a different */
    /* number of DCUs                                               */
    extern int KS_FAR total_ndcus;
#endif /* INCLUDE_MALLOC_DCU_INIT */

/* total number of packets and dcus allocated   */
extern int KS_FAR ndcus_alloced;

/* ********************************************************************   */
/* POOLS OF PORTS                                                         */

#if (INCLUDE_MALLOC_PORTS || INCLUDE_RUN_TIME_CONFIG)
/* array of allocated port pointers   */
extern PANYPORT KS_FAR *alloced_ports;
#else
extern PANYPORT KS_FAR alloced_ports[TOTAL_PORTS];
#endif

#if (!INCLUDE_MALLOC_PORTS)
#if (INCLUDE_UDP || INCLUDE_RAW)
    extern POS_LIST root_udp_port;          /* head of the free UDP port list */
#if (INCLUDE_RUN_TIME_CONFIG)
    extern PUDPPORT tc_udpp_pool;        /* UDP Ports pointer */
#else
    extern UDPPORT KS_FAR tc_udpp_pool[CFG_NUDPPORTS];   /* UDP Ports */
#endif
#endif      /* INCLUDE_UDP || INCLUDE_RAW */

#if (INCLUDE_TCP)
    extern POS_LIST root_tcp_port;          /* head of the free TCP port list */
#if (INCLUDE_RUN_TIME_CONFIG)
    extern PTCPPORT tc_tcpp_pool;                       /* TCP ports pointer */
#else
    extern TCPPORT KS_FAR tc_tcpp_pool[CFG_NTCPPORTS]; /* TCP ports */
#endif
#endif      /* INCLUDE_TCP */

#endif      /* !INCLUDE_MALLOC_PORTS */

/* ********************************************************************   */
#if (INCLUDE_SYSLOG && INCLUDE_BGET)
extern char KS_FAR syslog_mempool[CFG_SYSLOG_BGET_CORE_SIZE+BGET_OVERHEAD];
#endif  /* (INCLUDE_SYSLOG && INCLUDE_BGET) */

#if (INCLUDE_SSL && INCLUDE_BGET)
extern char KS_FAR ssl_mempool[CFG_SSL_BGET_CORE_SIZE+BGET_OVERHEAD];
#endif  /* (INCLUDE_SSL && INCLUDE_BGET) */

extern int KS_FAR lowest_free_packets;      /* lowest number of free packet ever */
extern int KS_FAR current_free_packets; /* lowest number of free packets now */

/* If this value is one or more the device layer ks_invoke_input() accepts   */
/* UDP broadcasts. Otherwise it throws them away. Each port that wishes      */
/* to receive UDP broadcasts increments this variable and decrements         */
/* it when it no longer wants them                                           */
extern int KS_FAR allow_udp_broadcasts;

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
extern RTIPINTFN_POINTER rtip_isr_interrupt[KS_NUM_INTS];
extern PIFACE KS_FAR rtip_irq_iface[KS_NUM_INTS];
#endif
extern RTIPINTFN_POINTER rtip_isr_strategy[KS_NUM_INTS];
extern int KS_FAR rtip_args[KS_NUM_INTS];
#endif /*OS*/



/* depth of currents calls to interrupt service   */
extern int KS_FAR in_irq;

#if (INCLUDE_RTIP)
#if (INCLUDE_MODEM)
/* ********************************************************************   */
/* MODEM                                                                  */
/* ********************************************************************   */
/* buffer used to pass input chars from modem to RTIP                     */
extern PFCHAR rawdata;

/* default script used to dial a modem where login is required   */
extern struct al_command KS_FAR default_script_login[];

/* Pointer to structure; this is done this way due to avoid some compilers   */
/* giving you an error if you extern an array without its size               */
extern struct al_command KS_FAR *default_script_login_ptr;

/* default script used to dial a modem with no login required   */
extern struct al_command KS_FAR default_script[];

/* Pointer to structure; this is done this way due to avoid some compilers   */
/* giving you an error if you extern an array without its size               */
extern struct al_command KS_FAR *default_script_ptr;

#endif      /* INCLUDE_MODEM */
#endif      /* INCLUDE_RTIP */

/* ********************************************************************   */
/* OS PORT DATA                                                           */
/* ********************************************************************   */

#if (INCLUDE_RTIP)
/* used to pass task number to tasks spawned   */
extern int KS_FAR static_iface_no;
extern int KS_FAR static_interrupt_no;

/* Kernel independent  declaration of semaphores and signals.
   The types are defined in os_port.h */
#if (!INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
extern KS_RTIPSIG portsig[TOTAL_PORTS][NUM_SIG_PER_PORT];
#endif

#if (defined(PEGRTIP))
extern RTIP_BOOLEAN hand_kb_to_peg;
extern RTIP_BOOLEAN hand_timer_to_peg;

extern KS_RTIPSIG ks_peg_sig[NUM_PEG_EVENTS];
extern KS_RTIPSEM ks_peg_sem[NUM_PEG_SEM];
#endif

/* SEMAPHORES   */
extern KS_RTIPSEM criticalsem;
extern KS_RTIPSEM tcpsem;
extern KS_RTIPSEM udpsem;
extern KS_RTIPSEM syslogsem;
extern KS_RTIPSEM tablesem;
extern KS_RTIPSEM memfilesem;

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* RS232 DATA                                                             */
/* ********************************************************************   */

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
    extern RS232_IF_INFO KS_FAR rs232_if_info_arry[CFG_NUM_RS232];
#endif

#if (INCLUDE_PPP)
/* *********************************************************************   */
/* PPP                                                                     */
/* *********************************************************************   */
extern PPP_XMIT_ESCAPE_FNC ppp_escape_routine;
extern PPP_GIVE_STRING_FNC ppp_give_string_routine;
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
extern POS_LIST root_tcp_lists[NUM_TCP_LISTS];

extern word KS_FAR tcp_port_number;      /* port number to use for new sockets */
extern RTIP_BOOLEAN KS_FAR ka_send_garbage;  /* if set, keepalive will send a garbage */
                                 /* byte   */
extern long KS_FAR default_tcp_options; /* default options when allocating a */
                                 /* TCP socket   */
#if (INCLUDE_TCP_OUT_OF_ORDER)
extern RTIP_BOOLEAN KS_FAR free_ooo_lists;   /* flag for alloc_packet to tell */
                                 /* timeout routine to free all out of   */
                                 /* order packet                         */
#endif
#if (INCLUDE_TCP_PORT_CACHE)
extern PTCPPORT KS_FAR cached_tcp_port; /* This value stores the last port */
                                /* we found in tcp_interpret.   */
#endif


#if (INCLUDE_OOO_QUE_LIMIT)
extern int KS_FAR num_ooo_que;           /* number of DCUs on the out of order list */
#endif
#endif      /* INCLUDE_TCP */

/* ********************************************************************   */
/* UDP                                                                    */
/* ********************************************************************   */
#if (INCLUDE_UDP)
/* list of heads of UDP socket lists where:                               */
/* offset ACTIVE_LIST: Head of list of active UDP ports                   */
/* offset SOCKET_LIST: Head of list of allocated but not active UDP ports */
extern POS_LIST root_udp_lists[NUM_UDP_LISTS];

extern word KS_FAR udp_port_number;     /* port number used by socket to assign */
                                /* unique port numbers to sockets   */
#if (INCLUDE_UDP_PORT_CACHE)
extern PUDPPORT KS_FAR cached_udp_port; /* This value stores the last port */
                                /* we found in udp_interpret.        */
                                /* We only save it if port number is */
                                /* unique                            */
#endif
#endif      /* INCLUDE_UDP */

#if (INCLUDE_UDP || INCLUDE_RAW)
extern long KS_FAR default_udp_options;      /* default options when allocating a */
                                     /* UDP socket   */
#endif

#if (INCLUDE_RAW)
/* ********************************************************************   */
/* RAW                                                                    */
/* ********************************************************************   */
/* list of heads of TCP socket lists where:                               */
/* offset ACTIVE_LIST: Head of list of active UDP ports                   */
/* offset SOCKET_LIST: Head of list of allocated but not active UDP ports */
extern POS_LIST root_raw_lists[NUM_UDP_LISTS];
#endif


/* ********************************************************************   */
/* ARP                                                                    */
/* ********************************************************************   */
extern dword KS_FAR arpc_res_tmeout;

/* ********************************************************************   */
/* DEVICE INFORMATION                                                     */
/* ********************************************************************   */

/* size of device table; setup at initialization   */
extern int KS_FAR total_devices;

/* total number of valid entries in device table   */
extern int KS_FAR valid_device_entries;

/* ********************************************************************   */
/* DRIVERS                                                                */
/* ********************************************************************   */

#if (INCLUDE_MTOKEN)
    extern MTOKDEVICE KS_FAR mdevicesoftc[CFG_NUM_MTOKEN];
    extern MTOK_ADAPTER KS_FAR madaptersoftc[CFG_NUM_MTOKEN];
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
extern IP_FRAGLIST KS_FAR *frag_table;
#else
extern IP_FRAGLIST KS_FAR frag_table[CFG_FRAG_TABLE_SIZE];
#endif
extern dword KS_FAR max_frag_size;
#endif

#endif /* (INCLUDE_RTIP) */

/* ********************************************************************   */
/* TOOLS.C                                                                */
/* ********************************************************************   */


/* ********************************************************************   */
/* DEBUG data                                                             */
/* ********************************************************************   */
#if (!USE_DB_L0)
    extern word    KS_FAR say_port;
    extern int     KS_FAR write_to_file;
    extern RTIP_BOOLEAN KS_FAR say_init_done;

    extern RTIP_BOOLEAN KS_FAR print_debug_log;
#    if (DO_FILE)
        extern FILE    *say_fp;
#    endif

    /* Select COMM portBase and interrupt number   */
    extern IOADDRESS sayportBase;   /* I/O address for COM */
    extern word     KS_FAR saycommIRQ;    /* IRQ number for COM */

#endif


/* ********************************************************************   */
/* OSPORT.C                                                               */
/* ********************************************************************   */

/* set when RTIP has been initialized by xn_rtip_init   */
extern RTIP_BOOLEAN KS_FAR rtip_initialized;

/* set when RTIP has been exited by xn_rtip_exit   */
extern RTIP_BOOLEAN KS_FAR rtip_exited;

/* set when resources initialized by xn_rtip_init (by RTIP) or by   */
/* pc_kernel_init (by ERTFS);                                       */
/* possible values are INIT_NOT_DONE, INIT_IN_PROG and INIT_DONE    */
extern int KS_FAR resource_initialized;

/* set when task templates and stack pool manager initialized by the   */
/* first call to ks_bind_task_index() or os_spawn_task()               */
extern RTIP_BOOLEAN KS_FAR kernel_initialized;

/* ********************************************************************   */
/* POLLOS.C                                                               */
/* ********************************************************************   */
#if (INCLUDE_RIP)
/* ********************************************************************   */
/* RIP                                                                    */
/* ********************************************************************   */
extern byte rip_buffer[RIP_MAX_PKTLEN];
extern byte rip_version_running;
extern int  rip_udp_socket;
extern int  rip_secs_to_update;
extern int  rip_update_flag;
#endif /* INCLUDE_RIP */


/* ********************************************************************   */
/* TASK                                                                   */
/* ********************************************************************   */
/* Head of running timer chain.
 * The list of running timers is sorted in increasing order of expiration;
 * i.e., the first timer to expire is always at the head of the list.
 */
extern PTIMER ebs_one_sec_timers;   /* multiple seconds timers */
extern PTIMER ebs_timers;           /* every time timer task runs */

extern int KS_FAR timer_freq;   /* frequency timer task should run; set to CFG_TIMER_FREQ */
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
extern RTIP_BOOLEAN do_sock_arp_cache;
extern RTIP_BOOLEAN do_sock_udp_cache;
extern RTIP_BOOLEAN do_sock_tcp_cache;
#endif

/* ********************************************************************   */
/* TASK MANAGEMENT                                                        */
/* ********************************************************************   */
/* Task management data. These structures are used exclusively by the     */
/* porting layer (osport.h) to manage tasks                               */
/* Task properties (stack size and priority by class)                     */
extern EBS_TASK_CLASS KS_FAR class_templates[TASKCLASS_EXTERN_APPTASK+1];

/*#if (USE_RTIP_TASKING_PACKAGE)    */
/* Task context block. One per task */
extern EBS_TASK_CONTEXT KS_FAR task_contexts[CFG_NUM_TASK_CONTROL_BLOCKS];
/*#endif   */

/*  Flag that tells the timer task to call ks_kernel_timer_callback() on   */
/*  the next tick because the kernel layer needs to process a task delete  */
/*  operation                                                              */
extern int KS_FAR ks_data_run_timer;

#if (KS_DECLARE_STACK)
extern KS_STACK_TYPE normal_1_stacks_array[CFG_N_STACKS_NORMAL_1][SIZESTACK_NORMAL];
#if (CFG_N_STACKS_NORMAL_2)
extern KS_STACK_TYPE normal_2_stacks_array[CFG_N_STACKS_NORMAL_2][SIZESTACK_NORMAL];
#endif
#if (CFG_N_STACKS_BIG)
extern KS_STACK_TYPE big_stacks_array[CFG_N_STACKS_BIG][SIZESTACK_BIG];
#endif
#if (CFG_N_STACKS_HUGE)
extern KS_STACK_TYPE huge_stacks_array[CFG_N_STACKS_HUGE][SIZESTACK_HUGE];
#endif
extern struct  stack_manager KS_FAR sm_array[CFG_N_STACKS_TOTAL];
#endif  /* KS_DECLARE_STACK */

/* ********************************************************************   */
/* SERVERS                                                                */
/* ********************************************************************   */
#if (INCLUDE_BGET && INCLUDE_MALLOC)
extern char KS_FAR context_core[CFG_CONTEXT_CORE];
#endif

/* ********************************************************************   */
/* TERMINAL                                                               */
/* ********************************************************************   */
extern RTIP_BOOLEAN KS_FAR terminal_is_open;
#if (CONSOLE_DEVICE == CONDEV_TELNET)       /* - telnet server */
extern PTELNET_CONTEXT telnet_console_context;
#endif


/* ********************************************************************   */
/* CALLBACKS                                                              */
/* ********************************************************************   */
extern PRTIP_CALLBACKS rtip_callbacks;

#if (INCLUDE_SNMP)
/* ********************************************************************   */
/* MIB variables                                                          */
/* ********************************************************************   */

#if (INCLUDE_TRAPS)
extern SEND_TRAP_FNC snmp_trap_fnc;
#endif

#if (!USE_OFFSET)
/* data structure containing all the mib variables   */
extern struct mib_variables KS_FAR mib_vars;
#endif

/* variables used to calculate MIB variables   */
extern dword KS_FAR sys_up_time;
#endif /* INCLUDE_SNMP */


/* ********************************************************************   */
/* ********************************************************************   */
/* RTIPCONS.C - RTIP CONSTANT GLOBAL DATA                                 */
/* ********************************************************************   */
/* ********************************************************************   */

#if (INCLUDE_RTIP)
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR broadaddr[ETH_ALEN];
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR nulladdr[ETH_ALEN] ;
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ip_ffaddr[IP_ALEN] ;
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ip_nulladdr[IP_ALEN];
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ip_lbmask[IP_ALEN] ;
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
/* all hosts group multicast IP address   */
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ip_igmp_all_hosts[IP_ALEN];
#if (INCLUDE_IGMP_V2)
/* all routers group multicast IP address   */
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ip_igmp_all_routers[IP_ALEN];
#endif
#endif

#if (INCLUDE_BOOTP)
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR bootp_VM_RFC1048[5];
#endif

/* ********************************************************************   */
/* DEVICES                                                                */
/* ********************************************************************   */

KS_EXTERN_GLOBAL_CONSTANT EDEVTABLE default_device_entry;

/* ********************************************************************   */
/* OSPORT DATA                                                            */
/* ********************************************************************   */

#if (INCLUDE_802_2)
/* ********************************************************************   */
/* 802.2 DATA                                                             */
/* ********************************************************************   */
KS_EXTERN_GLOBAL_CONSTANT byte llc_snap_data[LLC_SNAP_DATA_LEN];
#endif

/* ********************************************************************   */
/* TCP                                                                    */
/* ********************************************************************   */
#if (INCLUDE_TCP)

/* Array which contains tcp flags which should be sent in output packet     */
/* for each state                                                           */
/* NOTE: when enter FW2 no need to send pkt in response to other sides      */
/*       ack, therefore, for this case msg will not be sent; but while      */
/*       in FW2 can still be getting input pkts, therefore, need to ACK/ack */
/*       them                                                               */
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR tcp_flags_state[14];
#endif

/* ********************************************************************   */
/* UDP                                                                    */
/* ********************************************************************   */

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_LOOP)
/* ********************************************************************   */
/* LOOP, SLIP, CSLIP                                                      */
/* ********************************************************************   */
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR phony_en_addr[ETH_ALEN];
#endif

/* ********************************************************************   */
/* DRIVERS                                                                */
/* ********************************************************************   */

/* ********************************************************************   */
/* SPRINTF, DEBUG data                                                    */
/* ********************************************************************   */

KS_EXTERN_GLOBAL_CONSTANT char KS_FAR tc_hmap[17];

/* ********************************************************************   */
/* API data                                                               */
/* ********************************************************************   */
#if (INCLUDE_MODEM)
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *modem_off_line;
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *modem_ath0;
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *modem_ok;
#endif

#if (INCLUDE_API_STR)
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_rarp_name[8];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_bootp_name[9];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_ping_name[8];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_set_ip_name[10];
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_attach_name[10];
#endif
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_rt_add_name[10];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_rt_del_name[10];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_abort_name[9];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_pkt_alloc_name[13];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_pkt_data_max_name[16];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_data_pointer_name[16];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_pkt_data_size_name[17];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_pkt_recv_name[29];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_pkt_send_name[27];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_interface_open_name[18];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_interface_opt_name[17];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_pkt_free_name[12];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_tcp_is_connect_name[18];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_tcp_is_read_name[15];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_tcp_is_write_name[16];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_getlasterror_name[16];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_geterror_string_name[19];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_arp_send_name[12];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_arp_add_name[11];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_arp_del_name[11];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR socket_name[7];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR listen_name[7];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR connect_name[8];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR bind_name[5];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR accept_name[7];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR getsockopt_name[11];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR setsockopt_name[11];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR ioctlsocket_name[12];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR select_name[7];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR shutdown_name[9];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR closesocket_name[12];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR recv_name[14];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR send_name[12];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR getpeername_name[12];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR getsockname_name[12];
#endif

/* ********************************************************************   */
/* SOCKET DATA                                                            */
/* ********************************************************************   */

#if (INCLUDE_DB)
/* database for getservbyname() and getservbyport()   */
KS_EXTERN_GLOBAL_CONSTANT struct servent KS_FAR servs[NUM_SERVENT];

/* database for getprotobynumber() and getprotobyname()   */
KS_EXTERN_GLOBAL_CONSTANT struct protoent KS_FAR protos[NUM_PROTOENT];
#endif

#if (INCLUDE_POP3 || INCLUDE_SMTP || INCLUDE_WEB)
/* ********************************************************************   */
/* POP or SMTP                                                            */
/* ********************************************************************   */
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *mime_term_field;

KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *base64_alphabet;
#endif

/* ********************************************************************   */
/* TOOLS.C                                                                */
/* ********************************************************************   */
#if (INCLUDE_ERRNO_STR)

KS_EXTERN_GLOBAL_CONSTANT ERRNO_STRINGS KS_FAR error_strings[];

/* Pointer to structure; this is done this way due to avoid some compilers   */
/* giving you an error if you extern an array without its size               */
KS_EXTERN_GLOBAL_CONSTANT ERRNO_STRINGS KS_FAR *error_strings_ptr;
#endif

extern PFCCHAR bad_errno_string;

#endif      /* INCLUDE_RTIP */

/* ********************************************************************   */
/* TASKS                                                                  */
/* ********************************************************************   */
/* Class names. These are provided to assign names to tasks as they are
  spawned */
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR class_names[TASKCLASS_EXTERN_APPTASK+1][16];

#ifdef __cplusplus
}
#endif

#endif  /* __RTIPEXT__ */

