
/* XNCONF.H  - configuration file

   EBS - RTIP

   Copyright Peter Van Oudenaren , 1993
   All rights reserved.
   This code may not be redistributed in source or linkable object form
   without the consent of its author.

    Module description:
        This module contains constants that may be change to the modify
        the build characteristics of RTIP.

        NOTE: constants which start with _CFG are runtime configurable;
              (see confdata.c)
              constants which start with CFG are NOT runtime configurable
*/

#ifndef __XNCONF__
#define __XNCONF__ 1

/* ********************************************************************   */
#define RTIP_VERSION            41  /* RTIP version 4.1 */

/* *******************************************************************     */
/* BINARY VERSION CONFIGURATION SECTION                                    */
/* *******************************************************************     */
#define BUILD_NEW_BINARY        0

#if (BUILD_NEW_BINARY)
#define INCLUDE_RUN_TIME_CONFIG     1   /* DO NOT CHANGE */
#define INCLUDE_MALLOC_PORTS        1   /* DO NOT CHANGE */
#define INCLUDE_MALLOC_PORT_LIMIT   0
#define INCLUDE_MALLOC_CONTEXT      1
#else
#define INCLUDE_RUN_TIME_CONFIG     0
#define INCLUDE_MALLOC_PORTS        0  /* 0 - 1 is NI */
#define INCLUDE_MALLOC_PORT_LIMIT   0
#define INCLUDE_MALLOC_CONTEXT      0
#endif

/* won't get alloced unless INCLUDE_BGET is on and is needed by RTIP for   */
/* (FTP, TNS or WEB etc)                                                   */
#define CFG_CONTEXT_CORE        0x80000    // FKA was 2000

/* Set to attach semaphores and signals to interface structure   */
/* instead of in global variables                                */

/* specify whether to dynamically allocate packets at init time or   */
/* to declare them as global arrays where:                           */
/*   1=malloc packets at initialization time                         */
/*   0=declare packets as global array                               */
#if (defined (RTKBCPP) )
#define INCLUDE_MALLOC_DCU_INIT         1
#define INCLUDE_MALLOC_DCU_AS_NEEDED    0   /* DO NOT CHANGE */

#elif (BUILD_NEW_BINARY || INCLUDE_RUN_TIME_CONFIG)
#define INCLUDE_MALLOC_DCU_INIT         1
#define INCLUDE_MALLOC_DCU_AS_NEEDED    0   /* WARNING: might not beable to turn on */
                                            /* since alloc packet from ISR   */
#else
#define INCLUDE_MALLOC_DCU_INIT         0
#define INCLUDE_MALLOC_DCU_AS_NEEDED    0   /* DO NOT CHANGE */
#endif

/* configuration parameters which are not run-time configurable   */
#define CONFIG_PARAM_DONE 0     /* DO NOT CHANGE */

/* *******************************************************************       */
/* START OF ADD-ON CONFIGURATION SECTION                                     */
/* *******************************************************************       */
#undef INCLUDE_RTIP
#undef INCLUDE_ERTFS

#define INCLUDE_RTIP        1 //TEST JSM / KHS
#define INCLUDE_PPP         0
#define INCLUDE_PPPOE       0
#define INCLUDE_SNMP        0
#define INCLUDE_SNMPV2      0
#define INCLUDE_SNMPV3      0
#define INCLUDE_FTP_CLI     0
#define INCLUDE_FTP_SRV     0
#define INCLUDE_TFTP_CLI    1
#define INCLUDE_TFTP_SRV    0
#define INCLUDE_TELNET_CLI  0
#define INCLUDE_TELNET_SRV  0
#define INCLUDE_DHCP_CLI    1 //TEST JSM / KHS
#define INCLUDE_DHCP_SRV    0
#define INCLUDE_SMTP        0
#define INCLUDE_POP3        0
#define INCLUDE_IMAP        0
#define INCLUDE_WEB         1
/* Include support for WEB PUSH                            */
/* NOTE: Netscape support WEB PUSH but not all browsers do */
#define INCLUDE_WEB_PUSH    0
#define INCLUDE_WEB_BROWSER 0

#define INCLUDE_NAT   0
#define INCLUDE_SNTP  1 //TEST JSM / KHS

#define INCLUDE_SSL         0
#define INCLUDE_CRYPT       0       /* used by SNMP V3 - tbd */

#define INCLUDE_NFS_SRV     0
#define INCLUDE_NFS_CLI     0
#define INCLUDE_SMB_SRV     0

#define INCLUDE_ORTFS       0       /* old version of RTFS */
#define INCLUDE_ERTFS       0
#define INCLUDE_RTFILES     0
#define INCLUDE_DOS_FS      0       /* set for dos file system and dortfdosem */
#define INCLUDE_WIN32_FS    0       /* DO NOT CHANGE */
#define INCLUDE_ERTFS_PRO   0       /* If INCLUDE_ERTFS_PRO is set INCLUDE_ERTFS should not be set */
#define ERTFS_PRO_NSEMS     25      /* reduce according to the ERTFS configuration guide */
#define ERTFS_PRO_NSIGS     4       /* reduce according to the ERTFS configuration guide */

/* *******************************************************************       */
/* END OF ADD-ON CONFIGURATION SECTION                                       */
/* *******************************************************************       */

/* Set to 1 to include NAT (Network Address Translation) where IP address   */
/* and port number are translated                                           */
/* NOTE: INCLUDE_ROUTER and INCLUDE_NAT must also be set                    */
#define INCLUDE_NATP 1


/* ************************************************************************    */
/* *********************          SSL               ***********************    */
/* ************************************************************************    */
/* This is the only supported secure socket layer by EBS as of yet.            */
#define INCLUDE_EBS_SSL     (1 && INCLUDE_SSL)

/* *******************************************************************       */
/* SSL CONFIGURATION FOR ADD-ONS SECTION                                     */
/* *******************************************************************       */
/* set to 1 to enable the secure web server                                  */
#define INCLUDE_HTTPS_SRV   (0 && INCLUDE_SSL && INCLUDE_WEB)

/* not yet implemented   */
#define INCLUDE_HTTPS_CLI   (0 && INCLUDE_SSL && INCLUDE_WEB_BROWSER)

/* set to 1 to enable the secure ftp client   */
#define INCLUDE_FTPS_CLI    (0 && INCLUDE_SSL && INCLUDE_FTP_CLI)

/* not yet implemented   */
#define INCLUDE_FTPS_SRV    (0 && INCLUDE_SSL && INCLUDE_FTP_SRV)

#if (INCLUDE_ERTFS_PRO)
/* *******************************************************************          */
/*          ERTFS                                                               */
/* *******************************************************************          */
#define INCLUDE_ERTFS_STARTUP       1   /* Call pc_ertfs_init from demos all versions */
#define INCLUDE_ERTFS_EXIT_TASK     1   /* Call exit task - all versions */
#define INCLUDE_ERTFS_API           1   /* Use the api from applications - all versions */
#define INCLUDE_ERTFS_TIMER         0   /* Call RTFS one second Timer (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_CONF          0   /* Include Conf file (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_USER_STRUCT   0   /* Include user structure in RTIP context block (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_SIGNALS       0   /* Provide RTFS signals. (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_TYPES         0   /* include pcdisk.h (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_INTERRUPTS    0   /* Provide RTFS interrupts. (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_CALLBACKS     0   /* Provide RTFS APPS layer critical error and output  (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_MEM_MAP       0   /* Provide RTFS phys to virtual mapping (older NON-RTFS_SA releaes) */
#endif

#if (INCLUDE_ERTFS)
/* *******************************************************************       */
/*          ERTFS                                                            */
/* *******************************************************************       */
#define INCLUDE_ERTFS_STARTUP       1   /* Call pc_ertfs_init from demos all versions */
#define INCLUDE_ERTFS_EXIT_TASK     1   /* Call exit task - all versions */
#define INCLUDE_ERTFS_API           1   /* Use the api from applications - all versions */
#define INCLUDE_ERTFS_TIMER         1   /* Call RTFS one second Timer (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_CONF          0   /* Include Conf file (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_USER_STRUCT   0   /* Include user structure in RTIP context block (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_SIGNALS       0   /* Provide RTFS signals. (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_TYPES         0   /* include pcdisk.h (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_INTERRUPTS    0   /* Provide RTFS interrupts. (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_CALLBACKS     0   /* Provide RTFS APPS layer critical error and output  (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_MEM_MAP       0   /* Provide RTFS phys to virtual mapping (older NON-RTFS_SA releaes) */
#endif

#if (!(INCLUDE_ERTFS_PRO || INCLUDE_ERTFS))
#define INCLUDE_ERTFS_STARTUP       0   /* Call pc_ertfs_init from demos all versions */
#define INCLUDE_ERTFS_EXIT_TASK     0   /* Call exit task - all versions */
#define INCLUDE_ERTFS_API           0   /* Use the api from applications - all versions */
#define INCLUDE_ERTFS_TIMER         0   /* Call RTFS one second Timer (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_CONF          0   /* Include Conf file (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_USER_STRUCT   0   /* Include user structure in RTIP context block (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_SIGNALS       0   /* Provide RTFS signals. (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_TYPES         0   /* include pcdisk.h (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_INTERRUPTS    0   /* Provide RTFS interrupts. (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_CALLBACKS     0   /* Provide RTFS APPS layer critical error and output  (older NON-RTFS_SA releaes) */
#define INCLUDE_ERTFS_MEM_MAP       0   /* Provide RTFS phys to virtual mapping (older NON-RTFS_SA releaes) */
#endif


/* *******************************************************************       */
/* Set/Modify defines based upon above configuration values                  */
/* *******************************************************************       */

#if (INCLUDE_WIN32_FS)
#undef INCLUDE_DOS_FS
#define INCLUDE_DOS_FS      1
#endif

#if (INCLUDE_ERTFS_API || INCLUDE_DOS_FS || INCLUDE_RTFILES)
#define INCLUDE_FILESYSTEM 1
#else
#define INCLUDE_FILESYSTEM 0    /* set to 1 if you are using your own file system */
                                /* which is integrated with the virtual file   */
                                /* system                                      */
#endif

#if (INCLUDE_RTIP)
/* *********************************************************************   */
#define INCLUDE_IPV4 1      /* DO NOT CHANGE */
#define INCLUDE_IPV6 0      /* NOT SUPPORTED YET */


/* ************************************************************************       */
/* *******************     LARGE/SMALL CONFIGURATION     ******************       */
/* ************************************************************************       */

/* Set this define to 1 to turn on all the options.               */
/* Set this define to 0 for a small configuration where           */
/* certain features such as RARP, BOOTP, MODEM, IGMP, RAW socket, */
/* all device drivers except NE2000 etc. are turned off           */
#if (BUILD_NEW_BINARY)
#define LARGE_CONFIG  1
#else
#define LARGE_CONFIG  0
#endif

/* Set to 0 to get really small configuration; no UDP, no DNS   */
#if (LARGE_CONFIG)
#define SMALL_CONFIG  0     /* DO NOT CHANGE */
#else
#define SMALL_CONFIG  1
#endif

/* ************************************************************************       */
/* *******************        INCLUDE Protocols      **********************       */
/* ************************************************************************       */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                      */
/* Set to 1 if you will use tcp. If you don't (udp/raw only) set to zero       */
/* to save code space and resources                                            */
#define INCLUDE_TCP 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                      */
/* Set to 1 if you will use udp. If you don't (tcp/raw only) set to zero       */
/* to save code space and resources                                            */
#define INCLUDE_UDP 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                               */
/* Set to 1 if you will use raw sockets. If you don't set to zero       */
/* to save code space and resources;
   NOTE: if this is set, INCLUDE_UDP must also be set */
#define INCLUDE_RAW 0 // Test ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                        */
/* Set to 1 if you will include calls to xn_ping(). If not set this to zero      */
/* to save code space and resources                                              */
#define INCLUDE_PING 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 if you do need ICMP support (except ping which is controlled
   by INCLUDE_PING. If not set this to zero to save code space and resources */
#define INCLUDE_ICMP 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 if you do need IGMP support (host side only, i.e. RTIP
   is not a multicast router
   NOTE: set to 0 if INCLUDE_IGMP_V2 is set to 1 */
#define INCLUDE_IGMP 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 if you need IGMP Version 2 support (host side only)
   NOTE: if set to 1, set INCLUDE_IGMP to 0 */
#define INCLUDE_IGMP_V2 0 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                     */
/* Set to 1 to include a rarp server 0 otherwise. The rarp server is in       */
/* arp.c It must be generalized                                               */
#define INCLUDE_RARP_SRV 0 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                   */
/* Set to 1 if you will call xn_rarp(). Set to zero to save code space      */
#define INCLUDE_RARP 0 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                    */
/* Set to 1 if you will call xn_bootp(). Set to zero to save code space      */
#define INCLUDE_BOOTP 0 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                              */
/* Set to 1 to include support for 802.2, i.e. LLC and SNAP headers    */
/* Support for 802.2 entails processing of 802.2 input packets and if
   the socket option (see setsockopt) or interface option
   (see xn_interfacea_opt) is set, sending 802 */
#define INCLUDE_802_2 0

/* Set to 1 to include support for 802.11   */
#define INCLUDE_802_11 0

/* Set to 1 to include support for Token Ring   */
#define INCLUDE_TOKEN 0

/* Set to 1 to include support for callback due to events needed   */
/* for WINSOCK implementation                                      */
#define INCLUDE_POSTMESSAGE 0


#if (INCLUDE_FTPS_CLI)

/* NOTE:     If both INCLUDE_FTPS_CLI_IMPLICIT   */
/*           and INCLUDE_FTPS_CLI_EXPLICIT are   */
/*           enabled, you must choose the mode   */
/*           to run in before connecting         */

/* SSL Implicit Encryption using port FTPS_CONTROL_PORT (defined in ftpcli.c)   */
#define INCLUDE_FTPS_CLI_IMPLICIT 1
/* SSL Explicit Encryption using port FTP_CONTROL_PORT (defined in ftpcli.c) and AUTH SSL   */
#define INCLUDE_FTPS_CLI_EXPLICIT 1
#endif

#if (INCLUDE_FTPS_SRV)
#define INCLUDE_FTPS_CLI_IMPLICIT 0
#define INCLUDE_FTPS_CLI_EXPLICIT 0
#endif


/* ************************************************************************             */
/* *********************         MISC               ***********************             */
/* ************************************************************************             */
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                               */
/* Set to 1 if you will need fragmentation support. Set to zero to save code space      */
#define INCLUDE_FRAG 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 if you need select (and xn_tcp_is_connect, xn_tcp_is_read or
   xn_tcp_is_write) */
#define INCLUDE_SELECT 1

/* Set to 1 to gather statistics about interfaces and ports     */
/* Statistics are stored in the interface structure and are useful for
   debugging applications. This may be turned of for a production system if
   the statistics won't be used. */
/* NOTE: the statistics are used by SNMP also, therefore, if SNMP is on
         do not turn this off */
#if (INCLUDE_SNMP || INCLUDE_WEB)
#define INCLUDE_KEEP_STATS 1
#else
#define INCLUDE_KEEP_STATS 0
#endif

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include DNS functions        */
#define INCLUDE_DNS 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                         */
/* Set to 1 to include DNS cache, i.e. cache results of lookup    */
#define INCLUDE_DNS_CACHE 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include database functions, i.e. getprotobynumber(),
   getprotobyname(), getservbyname() and getservbyport() */
#define INCLUDE_DB 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                      */
/* Set to 1 to include packet API, i.e. xn_pkt_send() etc      */
#define INCLUDE_PKT_API 1  /* tbd - LARGE_CONFIG */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to enable ARP code; ARP is only needed for ethernet
   interfaces;
   NOTE: if using SNMP, INCLUDE_ARP must be set */
/* NOTE: if this is set, INCLUDE_ETH_BUT_NOARP must be off   */
#define INCLUDE_ARP 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                           */
/* Set to 1 to include an IP to ETHERNET translation table; this is */
/* used for ETHERNET packet where the translation is known by the   */
/* application, therefore, it is not necessary to ARP; the table    */
/* must be set up by calls to xn_arp_add()                          */
/* NOTE: if this is set, INCLUDE_ARP must be off                    */
/* NOTE: if set RTIP will still respond to ARP requests             */
#define INCLUDE_ETH_BUT_NOARP 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                    */
/* Set 1 to include callbacks to application to allow POLLOS */
/* to yield to kernel when it has nothing to do; the         */
/* callback routines are cb_pollos_yield_to_rtos() and       */
/* cb_pollos_signal_to_rtos()                                */
/* This define is useful if running POLLOS as a task in a    */
/* multitasking kernel                                       */
#define INCLUDE_POLLOS_COOPERATIVE 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to enable routing table code; routing table code allows
   determination of which interface a packet is sent out on based upon
   the IP address sending to; This should only be turned of if there
   is only one interface, i.e. CFG_NIFACES is 1;
   NOTE: if using SNMP, INCLUDE_ROUTING_TABLE must be set */
#define INCLUDE_ROUTING_TABLE 1 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to enable a time-to-live timer in routing table entries; if
   this is set, the ttl value is set by a parameter in xn_rt_add.  If
   this is set to 0, routing table entries do not expire and are only
   removed by calls to xn_rt_del */
#define INCLUDE_RT_TTL   1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to enable code which locks a routing table entry while a
   TCP socket which uses the entry is connected. Turning this off save
   a small amount of code but if a routing table entry is deleted which
   is being used by a TCP connection, sends will fail */
#define INCLUDE_RT_LOCK  1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to build static routes when default gateway is chosen,
   NOTE: this enables ICMP redirects to effect sending to a better gateway */
#define INCLUDE_BUILD_STATIC_ROUTES 1 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to enable routing of packets, i.e. forwarding packets not destined
   for our IP address; the routing table is used to determine where to
   forward the packet
   NOTE: INCLUDE_ROUTING_TABLE must also be set to 1
   NOTE: RTIP provides a very simple router, we are not a "router" */
#define INCLUDE_ROUTER 0 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include code to act as a PROXY ARP; this is valid only
   if INCLUDE_ROUTER is set to 1 */
#define INCLUDE_PROXY_ARP 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                    */
/* Set these to enable RIP, version 1 or versions 1 and 2    */
#define INCLUDE_RIPV1         0
#define INCLUDE_RIPV2         0
#define INCLUDE_RIP_LISTEN_ONLY   0

/* ************************************************************************    */
/* *********************      ERRNO/API             ***********************    */
/* ************************************************************************    */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to enable strings associated with the API calls (used for debugging
   purposes).  The strings can be used for debugging purposes to track
   calls to the API layer, i.e. every api routine calls the macro
   RTIP_API_ENTER passing a unique parameter.  The macro which is defined in
   rtip.h calls the diagnostic routing DEBUG_LOG to print the name of the
   api.  (See the Users/Porting Manual for more details on DEBUG_LOG).
   The strings are defined in rtipcons.c */
#define INCLUDE_API_STR 0 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include various error checking code such as parameter checking
   and if system has been initialized.  This should be set to 1 for
   development but can be set to 0 to save space for production */
#define INCLUDE_ERROR_CHECKING 0 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to enable the routine geterror_string() which converts an
   errno value to a string.  A large array of data strings describing
   the errno values will be included if this is set.  The strings are
   define in rtipcons.c */
#define INCLUDE_ERRNO_STR 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* set to non-zero if you want NON-thread-safe unknown errno's to be printed as
   numbers within the xn_geterror_string() returned string.
   NOTE that, though the returned string is NOT threaqd-safe, is constructed in such
   a way that you'll always hit a sentinel (NUL-byte) at the end of the valid buffer.
   I.e. your software will NOT crash; only receive 'patched', i.e. incorrect unknown
   errno numbers when multiple tasks access xn_geterror_string() with unknown errno's
   at the same time.
 */
#define INCLUDE_ERROR_STRING_NUMBER    1

/* ************************************************************************    */
/* *********************     TCP ALGORITHMS         ***********************    */
/* ************************************************************************    */

/* when send is in blocking mode, send will wait until all bytes are
   acked (versus waiting until all are queued) */
#define _CFG_TCP_SEND_WAIT_ACK 1

/* Set to 1 to enable FAST RETRANSMIT in TCP; fast retransmit will
   retransmit packets if too many duplicate acks are received */
#define INCLUDE_FAST_RETRANS 1

/* Set to 1 to enable Congestion avoidance and SLOW START in TCP   */
#define INCLUDE_CWND_SLOW 1

/* Set to 1 to enable round-trip time calculations for TCP; if this
   is not calculated CFG_MINRTO is used to determine intervals
   to retransmit data */
#define INCLUDE_TCP_RTT 1

/* Set to 1 to enable code to do out of order processing, i.e. if set
   out of order packets are keep and processed when they are no longer
   out of order; if turned off, out of order packets are dropped */
#define INCLUDE_TCP_OUT_OF_ORDER 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                      */
/* Set to 1 to include code to support NO COPY mode for TCP    */
#define INCLUDE_TCP_NO_COPY     1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                   */
/* Set to 1 to include code to support COPY mode for TCP    */
#define INCLUDE_TCP_COPY        1

/* Set to 1 to include code to perform fast processing of TCP input
   packets; this code will perform delayed ack process but none of
   the other TCP algorithms such as congestion fast retransmit,
   out of order processing, etc.  This code not handle packets for
   sockets in COPY mode or packets which have RESET, SYNC or FIN bits set */
#define INCLUDE_FAST_TCP 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                   */
/* Use set sockopt(SO_LINGER=off, linger value >0) to set maximum amount of */
/* time to wait until remote closes                                         */
#define INCLUDE_TCP_REMOTE_CLOSE 1

/* ************************************************************************    */
/* *********************       OPTIONS              ***********************    */
/* ************************************************************************    */
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                      */
/* Set to 1 to include support for IP options                                  */
#define INCLUDE_IP_OPTIONS 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                  */
/* Set to 1 to include code to support timestamp option    */
#define INCLUDE_TCP_TIMESTAMP 1

/* ************************************************************************    */
/* *********************     PERFORMANCE            ***********************    */
/* ************************************************************************    */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include code in UDP that will speed up matching incoming
   packets with ports. It will speed up only if the port number of the socket
   in the previous packet is the same as the port number in the current packet
   and there is only one udp socket bound to that port number */
#define INCLUDE_UDP_PORT_CACHE 0 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include code in UDP that will speed up transmission by
   cacheing routing table information in udp sockets.
   Recommended setting 1 */
#define INCLUDE_UDP_IFACE_CACHE 1 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* This configuration flag is meaningfull only if INCLUDE_UDP_IFACE_CACHE
   is 1. If INCLUDE_SYNCHRONIZE_UDP_IFACE_CACHE is 1 then code is included to
   make sure that if a routing table entry changes then all routes cached
   at the socket level will be cleared. This feature may be disabled to
   save code space if you are sure that routes will not dynamically change
   Recommended setting 1 */
#define INCLUDE_SYNCHRONIZE_UDP_IFACE_CACHE 1 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include code that will speed up transmission by
   cacheing arp table information in tcp and udp sockets.
   Recommended setting 1 */
#define INCLUDE_SOCK_ARP_CACHE 1 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set this value to 1 if you are using UDP_PORT_CACHE and you can guarantee
   that your application will never bind the same socket number to more than
   one udp port. If this value is set to one some code is excluded but
   udp port cacheing is unreliable if more than one socket has the same
   port number.
   The recommended value is zero unless you are in a deeply embedded system
   and are very clear on the usage of this flag */
#define GUARANTEE_UNIQUE_UDP_PORT_NUMBERS 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include code in TCP that will speed up matching incoming
   packets with ports. This is a straight forewared algortithm and should
   be left on */
#define INCLUDE_TCP_PORT_CACHE 1 // ESL / KHS

/* ************************************************************************    */
/* *********************     DIAGNOSTICS            ***********************    */
/* ************************************************************************    */

/* Set to one to enable PERFORMANCE measuring features. This should normally
   be set to zero */
#define INCLUDE_MEASURE_PERFORMANCE 0

/* Set to 1 to include execution profiling code   */
/* Note: Only currently works for NET186 board    */
#define INCLUDE_PROFILING  0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include debug code to track DCUs; if this is set the
   routine display_packet_info() is available in os.c which will
   scan all RTIP internal exchanges, lists, TCP windows etc and
   to gather statistics; this routine will call DEBUG_ERROR to prints
   the results;
   if DEBUG_TRACK in os.c is set to 1 and INCLUDE_TRK_PKTS is set,
   the routine display_packet_info() will be called if call to allocate
   a packet fails;
   display_packet_info() calls DEBUG_ERROR() to display the summary */
/* NOTE: check related values in os.c, i.e. TRACK_WAIT, DEBUG_TRACK   */
#define INCLUDE_TRK_PKTS 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include save history of all PPP input and output packets;
   the history can be displayed by calling display_track_ppp_info().
   The routine display_track_ppp_info() will call DEBUG_ERROR to
   print the error messages */
/* NOTE: CFG_TRK_NUM_PPP controls the number of packets to track                */
/* NOTE: CFG_TRK_PPP_LEN controls the number of bytes per packet to track       */
#define INCLUDE_TRK_PPP 1

/* *********************************************************************   */
/* ******        GUARD PACKET MACROS                               *****   */
/* *********************************************************************   */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                      */
/* set to 1 to include code to guard the end of PACKET which is connected to   */
/* DCU; a fixed pattern will be stored at the end of the packet and each time  */
/* a DCU is freed the guard will be checked to ensure it has not been changed; */
/* if turned on set a break point at break_guard()                             */
/* which will be called if the guard is corrupted                              */
// XHM: enabled 15/11 2006
#define INCLUDE_GUARD_PACKET 1

/* ********************************************************************   */
/* MALLOC                                                                 */
/* ********************************************************************   */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                  */
/* track calls to bget and brel check for invalid frees    */
// XHM: enabled 15/11 2006
#define INCLUDE_DEBUG_MALLOC      1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING          */
/* check for overwritting end of dynamic memory    */
// XHM: enabled 15/11 2006
#define INCLUDE_GUARD_MALLOC      1 

/* number of malloc's to track   */
#if (INCLUDE_SSL)
#define CFG_DEBUG_MALLOC_SIZE 8192
#else
#define CFG_DEBUG_MALLOC_SIZE 4096
#endif

/* ************************************************************************    */
/* *********************     SYSLOG APP DIAGNOSTICS    ********************    */
/* ************************************************************************    */

/* Set to 1 if you will use syslog client. If you don't set to zero            */
/* to save code space and resources                                            */
#define INCLUDE_SYSLOG 0

/* set to a suitable 'number of bytes' to reserve for syslog related run-time
   storage allocation. Don't worry about this one if INCLUDE_BGET is NOT set.
 */
#define CFG_SYSLOG_BGET_CORE_SIZE 4000

/* set to 1 if you want to add the additional '[SEQ:*****]' prefix
  string to the syslog header to track 'dropped' UDP syslog packets
  at the remote syslog server.
 */
#define CFG_ADD_SEQUENCE_NUMBER2SYSLOG  0

/*
  attempt new remote connect N lines 'delayed' after failed connect.

  This value is > 0 if you want to reduce 'syslog' load in case parameters
  and/or remote servers are down/faulty.

  Why would the syslog client facility try anything like this? Say you have
  REDUNDANT servers. A DNS is used to point to the 'active' server. To provide
  the customers of those redundant servers with ~100% 'up-time', you SWITCH
  your DNS to the 'dormant server', wait a while and SHUT DOWN the original
  'active server' for maintenance. Any IP-traffic will reconnect THROUGH your
  DNS and thus arrive at the originally 'dormant' server, which is right now
  the 'active server'. Apart for some 'TCP reconnections' (which are not
  noticable to most of your users anyway) nobody will be burdened by
  'server unreachable' or anything like that.

  Any syslog client should 're-detect' the syslog server IP address through
  a DNS query every once in a while to redirect syslog traffic to an 'active
  server'.
 */
#define CFG_SYSLOG_RETRY_CONNECT_DELAY_COUNT    4

/*
  attempt new filesystem init and/or file open N lines 'delayed' after
  failed init/open.

  This value is > 0 if you want to reduce 'syslog' load in case parameters
  and/or mounted filesystems are down/faulty.
 */
#define CFG_SYSLOG_RETRY_FINIT_DELAY_COUNT      4

/*
   Set to non-zero to allow the xn_syslog() facility to write any syslogging
   to a configurable serial port.
 */
#define CFG_SYSLOG_THROUGH_RS232                0


/* ************************************************************************    */
/* *********************      SECURE SOCKETS: SSL      ********************    */
/* ************************************************************************    */

/* set to a suitable 'number of bytes' to reserve for SSL related run-time
   storage allocation. Don't worry about this one if INCLUDE_BGET is NOT set.
 */
#define CFG_SSL_BGET_CORE_SIZE (256L * 1024L)

/* Set to 1 to include code to test any cryptographic routines      */
/* Set in ssl environment makefiles                                 */
#if (!defined(INCLUDE_TEST_CRYPT))
#define INCLUDE_TEST_CRYPT  0
#endif

/* ************************************************************************       */
/* ***************** SLIP/CSLIP and MODEM *********************************       */
/* ************************************************************************       */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include SLIP                 */
#define INCLUDE_SLIP  0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include CSLIP                */
#define INCLUDE_CSLIP 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* modem support included                   */
#if (INCLUDE_PPP || INCLUDE_SLIP || INCLUDE_CSLIP)
#define INCLUDE_MODEM LARGE_CONFIG
#else
#define INCLUDE_MODEM 0     /* DO NOT CHANGE */
#endif

/* ************************************************************************       */
/* *******************  NUMBER DEVICES   **********************************       */
/* ************************************************************************       */
/* number of comm ports supported (supports COMM1-COMM4)                          */
#define CFG_COMM_NUM 4

/* number of rs232 lines supported     */
#define CFG_NUM_RS232 1

/* Size of the interem buffer used when PPP and SLIP prepare for transmit     */
#define CFG_RS232_XMBUFSIZE 256

/* number of PPP connections supported - must be less than or equal to
   CFG_NUM_RS232 */
#define CFG_NUM_PPP 1

/* ************************************************************************       */
/* ******************* DEVICE TIMERS         ******************************       */
/* ************************************************************************       */

/* number of seconds to wait for transmit of SLIP or PPP output packet
   to complete   */
#define CFG_RS232_XMIT_TIMER 6

/* ************************************************************************       */
/* ******************* VANJ COMPRESSION (CSLIP/PPP) ***********************       */
/* ************************************************************************       */

/* Set to 1 to include Van Jacobson Compression for PPP     */
#if (INCLUDE_PPP)
#define INCLUDE_PPP_VANJC  1
#else
#define INCLUDE_PPP_VANJC  0        /* DO NOT CHANGE */
#endif

#define INCLUDE_CSLIP_XMIT 1        /* compress output packets - CSLIP only */

#define _CFG_MAX_VJC_SLOTS 16       /* maximum number of slots (0-255 where
                                      0 means no compression */
#define _CFG_CSLIP_SLOTS    8       /* number of slots - CSLIP only */

/* ************************************************************************       */
/* ****************************       DCUS     ****************************       */
/* ************************************************************************       */

/* NOTE: Memory packet pool is not used if INCLUDE_MALLOC_DCU_AS_NEEDED is 1   */
/*       in which case DCUs are allocated dynamically                          */

/* set to block at API layer if low on DCUs   */

#if (BUILD_NEW_BINARY)
#define INCLUDE_NO_DCU_BLOCK 0  /* DO NOT CHANGE: not supported */
#else
#define INCLUDE_NO_DCU_BLOCK 1
#endif

/* threshold for blocking waiting for DCUs   */
/* used if INCLUDE_NO_DCU_BLOCK is turned on */
#define CFG_DCU_BLOCK    2   /* block if hit this threshold */
#define CFG_DCU_RESUME   5   /* resume (signal) if hit this threshold */

/* Ethernet data frame size - size of data area in a DCU
   NOTE: DCUs are also used for DNS buffers, therefore, the buffer within
         the DCU should never be less than CFG_DNS_NAME_LEN or 260 bytes
   NOTE: the actual size of the packets are CFG_PACKETSIZEx+CFG_PACKET_ADJ
         which is defined in ethconf.h */

#define _CFG_MAX_PACKETSIZE 1514


/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                        */
/* Memory packet pool configuration.                                             */
/*                                                                               */
/* Network packets vary in size. Many small control packets are sent and         */
/* received such as ARP requests, TCP acknowledgement packets and small          */
/* application specific packets. These packets are typically less then           */
/* 128 bytes in length. Medium sized packets are sent and received               */
/* frequently as well. Examples of applications that use these packets           */
/* are web page downloads, mail downloads and FTP control socket traffic.        */
/* These packets often carry a payload of 512 bytes plus framing overhead.       */
/* Then there are large packets that consume the maximum link layer frame        */
/* size. These large packets are often used during bulk transfer operations      */
/* such as FTP.                                                                  */
/*                                                                               */
/* To handle multiple packet sizes RTIP uses multiple packet pools containing    */
/* packets of different sizes. We default to four pools. One for small           */
/* packets <= 128 bytes,  one for medium packets <= 256, one for medium          */
/* packets <= 596 bytes and one for large packets <= 1518 bytes.                 */
/*                                                                               */
/* When a packet allocation is requested we allocate it from the pool that       */
/* contains the smallest packet that will fit our requirement. If that pool      */
/* is full we look to the larger pools until we find space.                      */
/*                                                                               */
/* We provide a template for allocating six packet pools of the various          */
/* sizes. Note: we only utilize four of the pools but having six pools           */
/* available allows you to add finer granularity memory allocation without       */
/* having to resort to modifying source code.                                    */
/*                                                                               */
/* To configure the packet buffers you must modify several defines               */
/* in xnconf.h There are six pairs of defines that define the layout.            */
/* Each pair defines the number of packets and the packet size of the            */
/* pool where the pool is zero to five.                                          */
/*                                                                               */
/* #define CFG_NUM_PACKETSv YY                                                   */
/* #define CFG_PACKET_SIZEv XX                                                   */
/*                                                                               */
/* Where v is the pool number (0 to 5), YY is the number of packets and          */
/* XX is the packet size.                                                        */
/*                                                                               */
/* Notes:                                                                        */
/* 1. (YY * XX) may not exceed 64K. If you wish to allocate more than            */
/*    64 K of data to a specific packet size then add another pool containing    */
/*    packets of the same size.                                                  */
/* 2. The packet sizes must be specified in ascending order. Packets will        */
/*    be allocated from the first pool that contains packets >= the required     */
/*    size so in order to allocate the smallest packet possible the pools must   */
/*    be declared in ascending order.                                            */
/* 3. The code expects there will be packets of size CFG_MAX_PACKETSIZE.         */
/*    therefore, if you want to cut down on the size of the packets              */
/*    do not just modify CFG_PACKET_SIZEx but also modify CFG_MAX_PACKETSIZE     */
/* 4. In our sample we only use four of the possible six pools. If you add       */
/*    entries then you must insert them into the list in ascending order and     */
/*    will have to move our assignments up. For example if you add a pool of     */
/*    20 packates of size 1200 bytes you must move the 1518 byte pool            */
/*    configuration up and put the 1200 byte configuration in its place.         */
/* 5. Packets should never be smaller than ETHER_MIN_LEN if you are              */
/*    using an ethernet driver (see tc_interface_send in iface.c)                */
/*                                                                               */
/* Eg:                                                                           */
/* Change the configuration block from:                                          */
/*                                                                               */
/* #define CFG_NUM_PACKETS3  10                                                  */
/* #define CFG_PACKET_SIZE3  1518                                                */
/*                                                                               */
/* #define CFG_NUM_PACKETS4  0                                                   */
/* #define CFG_PACKET_SIZE4  0                                                   */
/*                                                                               */
/* To:                                                                           */
/*                                                                               */
/* #define CFG_NUM_PACKETS3  20                                                  */
/* #define CFG_PACKET_SIZE3  1200                                                */
/*                                                                               */
/* #define CFG_NUM_PACKETS4  10                                                  */
/* #define CFG_PACKET_SIZE4  1518                                                */
/*                                                                               */
/*                                                                               */
/*                                                                               */
/*                                                                               */


#define CFG_NUM_FREELISTS 6     /* Set for up to six free lists. Do not
                                   reduce. If you increase it you must add
                                   declarations to rtipdata.c and you must
                                   modify the subroutine os_init_packet_pools()
                                   in os.c */

#define _CFG_NUM_PACKETS0  10    /* allocate room for 10 128 byte packets */
#define _CFG_PACKET_SIZE0  128   /* This utilizes 1280 bytes */

#define _CFG_NUM_PACKETS1  20    /* allocate room for  50 128 byte packets */
#define _CFG_PACKET_SIZE1  256   /* This utilizes 2560 bytes */

#define _CFG_NUM_PACKETS2  10    /* Allocate room for  20 596 byte packets */
#define _CFG_PACKET_SIZE2  512   /* This utilizes 11920 bytes */

#define _CFG_NUM_PACKETS3  20    /* Allocate room for 20 byte packets */
#define _CFG_PACKET_SIZE3  _CFG_MAX_PACKETSIZE

#if (INCLUDE_MTOKEN)
#define _CFG_NUM_PACKETS4  25    /*  Allocate room for 15 byte packets */
#else
#define _CFG_NUM_PACKETS4  15    /*  Allocate room for 15 byte packets */
#endif
#define _CFG_PACKET_SIZE4  _CFG_MAX_PACKETSIZE

#if (INCLUDE_MTOKEN)
#define _CFG_NUM_PACKETS5  20    /*  Place holder. Assign segment size and */
#define _CFG_PACKET_SIZE5  _CFG_MAX_PACKETSIZE
#else
#define _CFG_NUM_PACKETS5  0     /*  Place holder. Assign segment size and */
#define _CFG_PACKET_SIZE5  0     /*  packet size to create more segments */
#endif

/* Lowwater mark where allocation of packets for input will fail.
   This allows sending of packets to succeed even if input packets are
   being blasted in faster than they can be handled.  This number should
   be less than CFG_DCUS. */
#define _CFG_PKT_LOWWATER 2

/* Lowwater mark for packets where all the DCUs in the TCP out-of-order
   lists will be freed.  The packets will be freed by the timer task.
   This number should be less than CFG_DCUS but greater that
   CFG_PKT_LOWWATER. */
#define _CFG_PKT_LOWWATER_OOO 5

/* Lowwater mark where source quench messages will be sent for input packets -
   this should be less than CFG_DCUS and greater than CFG_PKT_LOWWATER;
   INCLUDE_ICMP must also be set */
#define _CFG_PKT_QUENCH  10

/* ************************************************************************       */
/* *******************       POOLS and TABLE SIZES     ********************       */
/* ************************************************************************       */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Number of ports (sockets) available to UDP (minimize to save space);
   UDP sockets and resources are preallocated. Each UDPPORT uses
   approximately 58 bytes, One exchange, one semaphore and
   one message. We pre-allocate 4 structures. It should be set to at
   least one. UDP ports are allocated from far memory. */
#define _CFG_NUDPPORTS  6

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Number of ports (sockets) available to TCP (minimize to save space)
   Ignored if INCLUDE_TCP is zero;
   TCP sockets and resources are preallocated and are in far memory */
#define _CFG_NTCPPORTS  16

/* Set to number of concurent calls to select can be made for a socket   */
#define CFG_NUM_SELECT_P_SOCK 2

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Size of the ARP cache per interface.  The arp cache  is used
   for address resolution (IP to ethernet address) by hosts on the net.
   Each entry has an exchange consisting of all packets waiting for the
   same address to be resolved */
/* NOTE: used if INCLUDE_ARP is set   */
#define _CFG_ARPCLEN 5

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                               */
/* Number of entries IP to ethernet translation table;                  */
/* used for ethernet packets where translation is known by application, */
/* therefore, ARP is not necessary                                      */
/* NOTE: used if INCLUDE_ETH_BUT_NOARP is set                           */
#define _CFG_NUM_IP2ETH_ADDR 5

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Number of interfaces allowed to be open at one time. An interface
   structure (see IFACE in rtip.h) is allocated for each open device).
   Each interface requires two TASKs plus CFG_ARPCLEN exchanges for
   queueing packets for output once an address is resolved.
   Each interface uses 82 plus 16 * (CFG_ARPCLEN).  If INCLUDE_KEEP_STATS is
   one an additional approx 160 bytes is used for statistics.
   Interface structures are in far memory */
#define _CFG_NIFACES    1 // ESL / KHS

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                  */
/* number of entries in device table; if run-time configuration is turned  */
/* on, total_devices can be modified before calling xn_device_table_add to */
/* modify the size of the device table                                     */
#define _CFG_NUM_DEVICES 4

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* maximum number of multicast addresses which each interface can be
   listening for;
   NOTE: if INCLUDE_ICMP is 1, an extra entry is needed for the
         ALL HOSTS GROUP which is automatically added */
#define _CFG_MCLISTSIZE 5

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Size of routing table                    */
#define _CFG_RTSIZE 10

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Size of fragment table                   */
#define _CFG_FRAG_TABLE_SIZE 6

/* Time-to-Live for fragment table entries (in seconds)     */
#define _CFG_IP_FRAG_TTL 100

/* IP time to live - this value is used unless otherwise specified;
   useful to limit lifetime of segments and to terminate routing loops;
   only gateways will discard packets when TTL is exceeded;
   Note: the value used in output packets can be changed by SNMP or DHPC
         client */
/* Units: seconds although each gateway reduces it by one     */
#define _CFG_IP_TTL 60

/* Maximum fragment size (in bytes) will process for all protocols
   except ping; size includes protocol header;
   larger packets will be dropped */
#define _CFG_MAX_FRAG      6000

/* Maximum fragment size (in bytes) will process for ICMP packets;
   size includes protocol header; larger packets will be dropped */
#define _CFG_MAX_FRAG_ICMP 2000

/* ************************************************************************       */
/* ***************** MTU/WINDOW SIZE **************************************       */
/* ************************************************************************       */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                           */
/* MTU/MSS/WINDOW SIZE defines used in device table entries         */
/* NOTE: window size allows 4 max packets per window;               */
/*       you may want to change this to increase performance        */
/*       but if you increase it too much you                        */
/*       might need more DCU (see CFG_NUM_PACKETSx and CFG_NDCUS in */
/*       xnconf.h)                                                  */
/* NOTE: the maximum window size allowed if 0xffff                  */
#define CFG_ETHER_MAX_MSS     1460
#define CFG_ETHER_MAX_MTU     1500
#define CFG_ETHER_MAX_WIN_IN  (CFG_ETHER_MAX_MSS*4)
#define CFG_ETHER_MAX_WIN_OUT (CFG_ETHER_MAX_MSS*4)
/*#define ETHER_MAX_WIN_IN  (0x37ff)   */
/*#define ETHER_MAX_WIN_OUT (0x37ff)   */

#define CFG_RS232_MAX_MSS     CFG_ETHER_MAX_MSS     /* was 216 but max value seems better */
#define CFG_RS232_MAX_MTU     CFG_ETHER_MAX_MTU     /* was 256 but max value seems better */
#define CFG_RS232_MAX_WIN_IN  (CFG_RS232_MAX_MSS*4)
#define CFG_RS232_MAX_WIN_OUT (CFG_RS232_MAX_MSS*4)

/*   *********************************************************************       */
/*   ******               TCP and UDP port numbers            ************       */
/*   *********************************************************************       */

/* mimimum and maximum range of port numbers assigned to TCP and UDP          */
/* sockets (assigned when socket is called)                                   */
/* NOTE: calling bind will overrule the unique random port number assigned    */
/*       by socket                                                            */
/* NOTE: reserved ports:              1-1023                                  */
/*       ports assigned by socket: 1024-5000                                  */
/*       ports for bind:           5001-0xffff                                */
/* NOTE: bind does not check range (it only checks if address is in use)      */
/*       but using the range 5001-0xffff ensures no conflict with reserved    */
/*       ports or ports assigned by socket                                    */
/* NOTE: NFS assigns UDP ports in the range RESV_PORT_FIRST-RESV_PORT_LAST    */
/*       (600-1023)                                                           */
#define _CFG_UDP_PORT_MIN 1024
#define _CFG_UDP_PORT_MAX 5000
#define _CFG_TCP_PORT_MIN 1024
#define _CFG_TCP_PORT_MAX 5000

/*   *********************************************************************       */
/*   ******           TCP Compile Time Tuning Constants       ************       */
/*   ******           (don't change lightly)                  ************       */
/*   *********************************************************************       */

/* Send a keep alive packet if socket is quiescent for > CFG_KA_INTERVAL seconds      */
/* Units: seconds                                                                     */
#define _CFG_KA_INTERVAL     7200

/* Retransmit keep alive packet if still have not heard from other side      */
/* after CFG_KA_RETRY seconds                                                */
/* Units: seconds                                                            */
#define _CFG_KA_RETRY        75

/* Close the connection if do not hear from other side for CFG_KA_TMO
   seconds even though sent keepalive packets.
   Note: CFG_KA_TMO starts counting only after CFG_KA_INTERVAL seconds
   have expired if keepalive packets are sent, i.e. CFG_KA_TMO/CFG_KA_RETRY
   keepalive packets will have been sent before. */
/* Units: seconds     */
#define _CFG_KA_TMO          750

/* Interval between updating timers - must be less time than the timeout
   and delayed ack processing; the smaller this number the more acurate
   the delayed ack and timeout processing will be; this number will be
   converted to ticks and rounded up. */
/* Units: msec     */
#define _CFG_TIMER_FREQ     110
/* Interval to perform TCP timeout processing      */
/* Units: msec                                     */
/* Range: word                                     */
#define _CFG_TMO_PROC  500

/* Interval to send delayed TCP ACKS      */
/* Units: msec                            */
/* Range: word                            */
#define _CFG_MAX_DELAY_ACK  200

/* As packets are acked a round trip time estimate is calculated. Packets
   are resent if not acked within this time. The estimated round trip is
   constantly adjusted. These two values put limits on the estimate. */
/* Units: msec                       */
/* Range: word (see port->rto)       */
#define _CFG_MAXRTO       60000l
#define _CFG_MINRTO       6000l

/* Amount of time TCP will retransmit packets before giving up -
   default is 3 minutes */
/* Units: msec        */
/* Range: dword       */
#define _CFG_RETRANS_TMO 180000l

/* Amount of time TCP will retransmit packets before reporting error to
   application - default is 3*CFG_MINRTO */
/* Units: msec       */
/* Range: word       */
#define _CFG_REPORT_TMO  18000l

/* The minimum interval to send a window probe      */
/* Units: sec                                       */
#define _CFG_WIN_PROBE_MIN 5

/* The maximum interval to send a window probe      */
/* Units: sec                                       */
#define _CFG_WIN_PROBE_MAX 60

/* Waits this long between completing a close handshake and releasing the
   socket (ticks), i.e. after we initiate a close and the other side
   has responded with FIN|ACK, the socket will be released after
   CFG_TWAITTIME seconds */
/* Units: seconds      */
/* Range: word         */
#define _CFG_TWAITTIME      120

/* How long a CLOSE connection non initiater should wait for ACK of the final
   FIN it sent in close before closing (aborting) the socket, i.e.
   after we have received a FIN (close) request from the other side we
   send FIN|ACK. The other side should respond by ACKING the message. When
   he does we release the socket.  If he does respond within LASTTIME
   ticks we close the socket. The default value is set to 4000 (approx. 3.75)
   minutes. */
/* Units: seconds      */
/* Range: word         */
#define _CFG_LASTTIME   20 // DAU!! 220

/* Maximum number of outstanding connection requests allowed for a port;
   used to limit the backlogsize parameter of xn_listen */
#define _CFG_TCP_MAX_CONN    10

/* maximum segment size to use if input SYNC message does not contain a
   MSS option; RFC 1122 says to use 536 */
#define _CFG_NO_MSS_VAL 536     /* equivalent to MTU of 576 */

/* maximum segment size to limit sends to if need to send over a gateway -
   RFC 1122 says to use 536 */
#define _CFG_MSS_GATEWAY  536   /* equivalent to MTU of 576 */

/* total number of DCUs which may be queued on all the TCP out-of-order
   lists; valid only if INCLUDE_OOO_QUE is set */
#define INCLUDE_OOO_QUE_LIMIT   1   /* tbd - move? */
#define _CFG_NUM_OOO_QUE        8

/* *********************************************************************       */
/* ******                    RARP and BOOTP                 ************       */
/* *********************************************************************       */

/*   How many times to retry rarp before giving up     */
#define _CFG_RARP_TRIES   10

/* Number of second to wait before retrying a RARP request     */
#define _CFG_RARP_TMO     30

/* How many times to retry BOOTP before giving up     */
#define _CFG_BOOTP_RETRIES   4

/* Number of routers from BOOTP server which will be processed   */
#define _CFG_BOOTP_RTSIZE 5

/* ************************************************************************       */
/* *******************     DNS and DATABASE  ******************************       */
/* ************************************************************************       */

#define _CFG_MIN_DNS_DELAY           10     /* The minimum (initial) delay between & */
                                            /* name server retries.                        */
                                            /* Units: seconds                              */
#define _CFG_MAX_DNS_DELAY           60     /* The maximum delay between retries */
                                            /* Units: seconds     */
#define _CFG_DNS_RETRIES             3      /* Max # of retries - delay is doubled
                                               for each retry */
#define _CFG_DNS_NAME_LEN            100    /* Maximum length of a hostname */
                                            /* NOTE: Needs to be a multiple of   */
                                            /*       4 for alignment issues      */
#define _CFG_MAX_HT_ENT              6      /* Maximum # of host table entries */
#define _CFG_MAX_HC_ENT              6      /* Maximum # of host cache entries */
#define _CFG_MAX_DNS_SRV             5      /* Maximum # of name servers  */
#define _CFG_MAX_HX_ADDRS            3      /* Size of array h_addr_list in */
                                            /* hostentext (extended hostent)      */
                                            /*  structure.                        */
#define _CFG_MAX_HX_ALIAS            1      /* Size of h_aliases in hostentext */

/* ************************************************************************       */
/* *********************            IGMP            ***********************       */
/* ************************************************************************       */

/* Maximum delay for sending a IGMP report after receiving an IGMP query;
   If report is sent by another IGMP host before the timer expires the
   timer will be stopped and the report will not be sent */
/* Note: This is only used for queries from an IGMP Version 1 router   */
/* Units: seconds                                                      */
#define _CFG_IGMP_MAX_DELAY 10
#endif          /* INCLUDE_RTIP */

/* ************************************************************************          */
/* *********************     PRINTF & FLOATING POINT    *******************          */
/* ************************************************************************          */

/*
 * Set to non-zero value if you want tc/tm_[*]printf functions to support
 * floating point values.
 * NOTE: for this, the RTL sprintf() function is called. So if your RTL
 *       sprintf() doesn't support floating point values, you should add
 *       your own code instead.
 * NOTE2: set this value only to non-zero if your APP requires this type
 *        of floating-point displaying support!
 * NOTE3: SSL/CRYPTO: DES and other (speed measurement) test routines use
 *        floating point printf to report results.
 *        This setting is also used to control availability of 'floating
 *        point printing' in 'openssl/bio/b_print.c' as it has the same
 *        purpose.
 */
#define INCLUDE_TC_PRINTF_FLOAT_SUPPORT     0

/*#if (INCLUDE_TEST_CRYPT) && (INCLUDE_CRYPT || INCLUDE_SSL))   */
/*#undef INCLUDE_TC_PRINTF_FLOAT_SUPPORT                        */
/*#define INCLUDE_TC_PRINTF_FLOAT_SUPPORT     1                 */
/*#endif                                                        */

/* set to the radix (10 or 16 are preferred) of the numbers of any MAC address
   being printed by the tc_*printf() function series.
 */
#define CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX    16

/* ************************************************************************          */
/* *********************     MOUSE                  ***********************          */
/* ************************************************************************          */

/* set to a non-zero value if you want to include the RTIP mouse_*() driver routines   */
#define INCLUDE_MOUSE        0      /* NOT SUPPORTED YET */

/* ************************************************************************          */
/* *********************     Compatibility          ***********************          */
/* ************************************************************************          */

/* set to a non-zero value if you want to use the new UNIX/Winsock compatible
   'struct sockaddr_in' and 'struct in_addr' definitions. This is mandatory
   for SSL and most of the examples, as now you can copy UNIX-style code, like

      server.sin_addr.s_addr = htonl(ipaddress);

   instead of

      tc_mv4((PFBYTE)&server.sin_addr, &ip_addr, IP_LEN);

   Set to ZERO(0) for unconditional RTIP 2.x backwards compatibility.
 */
#if (INCLUDE_SSL)
#define CFG_UNIX_COMPLIANT_SOCK_STRUCTS                   1 /* DO NOT CHANGE */
#else
#define CFG_UNIX_COMPLIANT_SOCK_STRUCTS                   1
#endif

#if (INCLUDE_RTIP)
/* ************************************************************************          */
/* *********************     REMOTE DIAGNOSTICS    ************************          */
/* ************************************************************************          */

/* set to non-zero if you want to use the DIAG_* remote kernel debug &
   diagnostic system instead of the standard DEBUG_* routines.

   Note that all DEBUG_* macro's are mapped to DIAG_* routines if this
   define is non-zero.
 */
#define CFG_USE_DIAG_MSG          0
/* was: CFG_USE_REMOTE_KERNEL_DIAG_SYSTEM   */
#endif /* INCLUDE_RTIP */

#if (INCLUDE_RTIP || INCLUDE_ERTFS)
/* ************************************************************************          */
/* *********************     TASK MANAGER           ***********************          */
/* ************************************************************************          */

/* Configure the task manager section in osport.c     */

/* The total maximum number of tasks we wish to manage. Each task that
   uses the package needs to have one of task control blocks. For internally
   spawned tasks it contains information used for spawning the task, its
   priority, its stack size and its entry point. All tasks have a user
   structure that contains task specific data such as errno values current
   working directory (if rtfs is included) and user defined void data pointers
   Used if USE_RTIP_TASKING_PACKAGE (in osport.h) is set to 1 */
#define CFG_NUM_TASK_CONTROL_BLOCKS   30

/* The first of the task control blocks to be available for storing
   task specific data for tasks that were spawned externally and then
   called the api. There are
    CFG_NUM_TASK_CONTROL_BLOCKS - CFG_FIRST_EXTERNAL_TASK of these
   available (ie: this many externally spawned tasks may use the
   api and have unique contexts.
*/
#define CFG_FIRST_EXTERNAL_TASK       25 /* Here and above are never spawned */
#endif /* INCLUDE_RTIP */

/*****************************************************************      */
/* VFS (POP, WEB etc)                                                   */
/*****************************************************************      */

/* maximum file length including directory name                              */
/* (used for virtual files as well as file system files for pop, web etc)    */
#define CFG_MAX_FILE_LEN  128       /* should be 260 for long file names */

/* ************************************************************************       */
/* ******************        TOD/SNTP                   *******************       */
/* ************************************************************************       */

/* Interval in which time of day should be updated.  This is used if   */
/* KS_SUPPORTS_TIME is 0                                               */
/* Units: seconds                                                      */
#define _CFG_TOD_FREQ 2

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                           */
/* Interval in which time of day should be updated via SNTP         */
/* This is used if INCLUDE_SNTP is 1. This value should be greater  */
/* or equal to _CFG_SNTP_TOD_FREQ.                                  */
/* Units: seconds                                                   */
/* NOTE: this value should be greater than _CFG_TOD_FREQ            */
#define _CFG_SNTP_TOD_FREQ 360    /* 6 minutes */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                  */
/* number of seconds to wait for response to SNTP request  */
/* NOTE: this value should be less than _CFG_SNTP_TOD_FREQ */
/* Units: seconds                                          */
#define _CFG_SNTP_RCV_TMO 20

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                      */
/* difference in hours of local time to Greenwich Mean Time    */
/* NOTE: -5 = EST                                              */
#define _CFG_GMT_DIFF -5

/* ************************************************************************       */
/* ******************       GENERIC SERVER CONFIG       *******************       */
/* ************************************************************************       */
/* Number of second to wait for server read select
   NOTE:  currently only used while waiting for SSL accept
          to be started after returning form standard accept */
#define _CFG_SRV_READ_TMO       10

/* ************************************************************************       */
/* ******************      SERVERS (WEB/FTP/TELNET)     *******************       */
/* ************************************************************************       */
#define CFG_SPAWN_FTP           0   /* DO NOT CHANGE */
#define CFG_FTP_MAX_SPAWN       0   /* DO NOT CHANGE */
#define CFG_SPAWN_WEB           0   /* DO NOT CHANGE */
#define CFG_WEB_MAX_SPAWN       0   /* DO NOT CHANGE */
#define CFG_SPAWN_TELNET        0   /* DO NOT CHANGE */
#define CFG_TELNET_MAX_SPAWN    0   /* DO NOT CHANGE */

#if (INCLUDE_WEB)
/* ************************************************************************      */
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                        */
/* set to 1 to spawn tasks to process WEB connections; this enables
   processing of multiple connections */
#undef CFG_SPAWN_WEB
#define CFG_SPAWN_WEB     0		//JLA (IO)

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
#if (CFG_SPAWN_WEB && !INCLUDE_MALLOC_CONTEXT)
/* maximum number of tasks server can spawn to process incoming      */
/* WEB requests                                                      */
#undef CFG_WEB_MAX_SPAWN
#define CFG_WEB_MAX_SPAWN 5
#endif

#endif

#if (INCLUDE_FTP_SRV)
/* ************************************************************************      */
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                        */
/* set to 1 to spawn tasks to process FTP connections; this enables
   processing of multiple connections */
#undef CFG_SPAWN_FTP
#define CFG_SPAWN_FTP        1

#if (CFG_SPAWN_FTP && !INCLUDE_MALLOC_CONTEXT)
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                              */
/* maximum number of tasks server can spawn to process incoming        */
/* FTP requests (CFG_SPAWN_FTP must be set to 1)                       */
#undef CFG_FTP_MAX_SPAWN
#define CFG_FTP_MAX_SPAWN    5
#endif

#endif

#if (INCLUDE_TELNET_SRV)
/* ************************************************************************      */
/* set to 1 to spawn tasks to process telnet connections; this enables
   processing of multiple connections */
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
#undef CFG_SPAWN_TELNET
#define CFG_SPAWN_TELNET    1

#if (CFG_SPAWN_TELNET && !INCLUDE_MALLOC_CONTEXT)
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                             */
/* maximum number of tasks server can spawn to process incoming       */
/* TELNET requests                                                    */
#undef CFG_TELNET_MAX_SPAWN
#define CFG_TELNET_MAX_SPAWN 5
#endif

#endif

#if (!INCLUDE_MALLOC_CONTEXT)
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                      */
/* maximum number of tasks to spawn (should be set to max of   */
/* CFG_WEB_MAX_SPAWN, CFG_TELNET_MAX_SPAWN, CFG_FTP_MAX_SPAWN) */
#define CFG_MAX_CONTEXT 32
#endif

#if (INCLUDE_FTP_SRV || INCLUDE_FTP_CLI)
/*****************************************************************      */
/* FTP TIMEOUT CONSTANTS                                                */
/*****************************************************************      */
/* backlog values for servers when doing a listen                       */
#define _CFG_FTP_LISTEN_BACKLOG 5

/* timeout for FTP Client to wait for data to arrive                    */
/* units: seconds                                                       */
#define _CFG_FTP_READ_TMO    30

/* timeout waiting for a connection to become established      */
/* units: seconds                                              */
#define _CFG_FTP_WRITE_TMO   30

/* timeout waiting for a command from remote host    */
/* units: seconds                                    */
#define _CFG_FTP_COMMAND_TMO  RTIP_INF

/* timeout waiting for pasv connection to be retried    */
/* units: 1/4 seconds                        __st__     */
#define _CFG_FTP_PASV_CON_TMO 1

/* number of times to retry the pasv connection to      */
/* become established (must be >= 2)         __st__     */
#define _CFG_FTP_PASV_CON_RTY 6

/*****************************************************************      */
/* FTP SERVER CONSTANTS                                                 */
/*****************************************************************      */

/* Set to 1 to strip drive from path returned by FTP server doing PWD command   */
/* NOTE: this needs to be one if running against IE FTP                         */
#define _CFG_FTP_PWD_NO_DRIVE   1

/* Set to 1 if local file system to convert forward slashes to backslashes   */
/* in files sent by client                                                   */
#define _CFG_FILE_BACKSLASH         0

/* Set to 1 to convert backslashes to forward slashes when returning   */
/* paths for PWD to client                                             */
/* NOTE: if set this to 1, need to set CFG_FILE_BACKSLASH to 1 if file */
/*       system uses backslashes                                       */
#define _CFG_RETURN_FORWARDSLASH 0

/* buffer size used by the FTP server to send data;
   should be a multiple of 512 to get best possible file
   system access time; if it is less than
   or equal to CFG_MAX_PACKETSIZE then a DCU is used; otherwise
   bget() is used */
#define _CFG_FTP_BUF_SIZE  8192

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                 */
/* number of large buffers used to xfer data; should be less than         */
/* or equal to CFG_FTP_MAX_SPAWN; if there are not enough large           */
/* buffers, then a DCU is used instead which works except file xfer speed */
/* would not be as optimal                                                */
#define _CFG_FTP_NUM_LARGE_BUF 2

/* If set, code is included to keep current working directory   */
/* for each thread                                              */
#if (INCLUDE_RTFILES)
#define INCLUDE_PER_THREAD_CWD 1
#else
#define INCLUDE_PER_THREAD_CWD 0
#endif

/* Root directory used if INCLUDE_PER_THREAD_CWD is set   */
#define _CFG_FTP_ROOT_DIR "C:\\"

/* length of command line arguments FTP server can handle   */
#define CFG_CMD_ARG_LEN CFG_MAX_FILE_LEN

/* Port number FTP server chooses for passive mode where 0 is default set   */
/* by socket() which is in the range CFG_TCP_PORT_MIN (1024) -              */
/* CFG_TCP_PORT_MAX (5000);                                                 */
/* generally 20 which is data port would be the alternative                 */
#define _CFG_FTP_PASV_PORT 0

/*****************************************************************      */
/* FTP CLIENT CONSTANTS                                                 */
/*****************************************************************      */

/* buffer side used byte the FTP client to send/receive data;   */
/* should be a multiple of 512 to get best possible file        */
/* system access time                                           */
#define _CFG_FTPCLI_BUF_SIZE 1024

#if (INCLUDE_FTPS_CLI)
/* Protected Buffer Size to be used for the size of the SSL       */
/* packets to be transmitted.  If response from server is         */
/* smaller, that value should be used. Default set to 0.          */
/* This is not required by the SSL because it is a "streaming     */
/* protection mechanism" (murray-draft April 2, 2002) being used. */
/* Therefor not implemented as of yet.                            */
#define _CFG_FTPCLI_PBSZ 0
#endif


#define CFG_FTP_CLI_CMD_ARG_LEN 200
#endif /* INCLUDE_FTP_SRV || INCLUDE_FTP_CLI */

/*****************************************************************      */
/* FTP CLIENT MODE                                                      */
/*****************************************************************      */
#if (INCLUDE_FTP_CLI)

/* PASSIVE_MODE = 1 to enable a passive mode client __st__ 2002.04.01
 *
 * To use a passive client you will need to be sure
 * that your server-side firewall permits the following:
 *    access to the server port 21
 *    server can transmit from 21 to ports > 1024
 *    access to the server ports > 1024
 *    server can transmit from ports > 1024 to ports > 1024
 */

/* Set to 1 to run ftp client only in active mode   */
#define INCLUDE_ACTIVE_MODE_STANDALONE      0

#if (INCLUDE_ACTIVE_MODE_STANDALONE)
#define INCLUDE_PASSIVE_MODE_STANDALONE     0   /* DO NOT CHANGE */
#else
/* Set to 1 to run ftp client only in passive mode   */
#define INCLUDE_PASSIVE_MODE_STANDALONE     0
#endif

#if ((INCLUDE_PASSIVE_MODE_STANDALONE) || (INCLUDE_ACTIVE_MODE_STANDALONE))

#if (INCLUDE_PASSIVE_MODE_STANDALONE)
#define INCLUDE_PASSIVE_MODE                1   /* DO NOT CHANGE */
#endif

#if (INCLUDE_ACTIVE_MODE_STANDALONE)
#define INCLUDE_PASSIVE_MODE                0   /* DO NOT CHANGE */
#endif

#else
/* Set to 1 to run ftp client in both active and passive mode             */
/*  user decides at command prompt (in clisrv - PASV flag or ACTIVE flag) */
/*  defaults to PASV mode to start; if you would like to change           */
/*  this default to ACTIVE mode to start you should do so in              */
/*  FTPCLI.C ftpcli_xconnect and grep for the following line:             */
/*     cc->passive=TRUE;                                                  */
/* default is (true) PASV mode - user controlled                          */
#define INCLUDE_PASSIVE_MODE                1
#endif

#endif /* INCLUDE_FTP_CLI */

/*****************************************************************      */
/* TELNET SERVER CONSTANTS                                              */
/*****************************************************************      */

/* backlog values for servers when doing a listen                      */
#define _CFG_TNS_LISTEN_BACKLOG 5

/* number of seconds to wait for Telnet character; this is only     */
/* used when an option character is received and a second character */
/* is expected along with the option character                      */
#define _CFG_TNS_READ_TMO 10

/* timeout waiting to send (echo) character   */
/* units: seconds                             */
#define _CFG_TNS_WRITE_TMO   10

/* *********************************************************************       */
/* ******               TELNET Client Constants             ************       */
/* *********************************************************************       */
/* Number of bytes allocated for incoming Telnet packets                       */
#define _CFG_TNC_INPKTLEN            512

/* Number of bytes allocated for outgoing Telnet packets     */
#define _CFG_TNC_OUTPKTLEN           256

/* maximum number of options the Telnet client can negotiate.  Used as the
   size of the array telnetc_context.options_supported[]. */
#define _CFG_TNC_MAXOPS                16

/* timeout waiting to send (echo) character   */
/* units: seconds                             */
#define _CFG_TNC_WRITE_TMO             10

#if (INCLUDE_TFTP_SRV || INCLUDE_TFTP_CLI)
/*****************************************************************      */
/* TFTP TIMEOUT CONSTANTS                                               */
/*****************************************************************      */
/* timeout used by TFTP                                                 */
/* units: seconds                                                       */
#define _CFG_TFTP_TIMEOUT    5

/* If set, filenames will be converted to upper case   */
#define CFG_TFTP_FILENAME_UPPERCASE 0
#endif

/* ********************************************************************       */
/* *************        PPP (see PPP Reference Manual)     ************       */
/* ********************************************************************       */

#if (INCLUDE_PPP)
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                */
/* Set to 1 to include Challange-Handshake Protocol      */
#define INCLUDE_CHAP (!SMALL_CONFIG)

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING     */
/* Set to 1 to include IPCP DNS Protocol      */
#if (INCLUDE_DNS)
#define INCLUDE_PPP_DNS (!SMALL_CONFIG)
#else
#define INCLUDE_PPP_DNS 0       /* DO NOT CHANGE */
#endif

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include support for IPCP addresses option; NOTE: IPCP
   ADDRESSES option is obsoleted by IPCP ADDRESS so it probably is
   not needed */
#define INCLUDE_PPP_IPCP_ADDRESSES (!SMALL_CONFIG)

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include code for PAP         */
#define INCLUDE_PAP 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include code for PAP Server (i.e. require remote to
   logon) */
#define INCLUDE_PAP_SRV 1

/* ********************************************************************      */
/* default character control map if none negotiated                          */
#define _CFG_LCP_ACCM_DFLT   0xffffffffL

/* default MRU if none negotiated     */
#if (!INCLUDE_PPPOE)
#define _CFG_LCP_MRU_DFLT    1500
#else
#define _CFG_LCP_MRU_DFLT    1492
#endif

/* minimum MRU value to limit value from otherside     */
#define _CFG_LCP_MRU_HI      4096        /* High MRU limit */

/* minimum MRU value to limit value from otherside     */
#define _CFG_LCP_MRU_LO      128         /* Lower MRU limit */

/* ********     */
/* The following constants control the maximum size and number of
   PAP accounts. */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING        */
/* maximum user name length for PAP account      */
#define _CFG_PAP_NAMELEN     30

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING       */
/* maximum password length for PAP account      */
#define _CFG_PAP_PWDLEN      20

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* maximum record length which includes user account, password etc. for
   PAP account */
#define _CFG_USER_REC_LEN    128

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* max number of PAP accounts               */
#define _CFG_PAP_USERS       53

 /* ********                      */
/* TIMEOUT AND RETRY VALUES       */
/* LCP config values              */
#define _CFG_LCP_REQ_TRY     20      /* REQ attempts before giving up */
#define _CFG_LCP_NAK_TRY     10      /* NAK attempts before drop option */
#define _CFG_LCP_TERM_TRY    10      /* TERM REQ attempts before giving up */
#define _CFG_LCP_TIMEOUT     3       /* Seconds to wait for response before
                                       retrying  */

/* PAP config values     */
#define _CFG_PAP_REQ_TRY     100     /* # attempts from us LCP shutdown */
#define _CFG_PAP_FAIL_MAX    5       /* # attempts from remote before  */
                                     /* LCP shutdown     */
#define _CFG_PAP_TIMEOUT     3       /* Seconds to wait for response before
                                       retrying logon request */

/* IPCP config values     */
#define _CFG_IPCP_REQ_TRY    20      /* REQ attempts before giving up */
#define _CFG_IPCP_NAK_TRY    10      /* NAK attempts before REJECT */
#define _CFG_IPCP_TERM_TRY   10      /* TERM REQ attempts before giving up */
#define _CFG_IPCP_TIMEOUT    3       /* Seconds to wait for response to request
                                       before retrying */

/* CHAP config values     */
#define _CFG_CHAP_REQ_TRY 5          /* # times to retry a CHAP challenge before
                                       giving up and shutting down the connection */
#define _CFG_CHAP_TIMEOUT 5          /* Seconds to wait for response before
                                       retrying challange */

/* CHAP                                      */
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING    */
#define _CFG_CHAP_NAMELEN 100        /* max machine name length */
#define _CFG_CHAP_VALLEN  100        /* max len of challenge value */
#define _CFG_MAXSECRETLEN 32         /* max secret length */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* max number of machine name, secrets      */
#define _CFG_CHAP_SECRETS 5

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                              */
/* if set, then the first entry in the secrets table (if there is      */
/* an entry) is always used regardless of machine name                 */
#define CFG_CHAP_SINGLE_SECRET 0

#if (CFG_CHAP_SINGLE_SECRET)
/* if single secret, only need one entry in table   */
#undef CFG_CHAP_SECRETS
#define CFG_CHAP_SECRETS 1
#endif

/* ********************************************************************       */
/* PPPoE                                                                      */
/* ********************************************************************       */
/* Number of times to retry Discovery Initiation Request by PPPoE host        */
#define CFG_PADI_RETRY 1

/* Number of times to retry Discovery Request by PPPoE host   */
#define CFG_PADR_RETRY 3

/* ********************************************************************       */
/* PPP DEBUG                                                                  */
/* ********************************************************************       */
#if (INCLUDE_TRK_PPP)
/* number of PPP packets to track; INCLUDE_TRK_PPP must be 1 to save      */
/* PPP input/output packets                                               */
#define CFG_TRK_NUM_PPP 60

/* number of bytes (minus PPP header) in PPP packet to track             */
/* INCLUDE_TRK_PPP must be set to 1 to save PPP input/output packets;    */
/* packets are saved in track_ppp_msg and may be displayed by            */
/* display_track_ppp_info()                                              */
#define CFG_TRK_PPP_LEN 40
#endif
#endif  /* INCLUDE_PPP */

#if (INCLUDE_SNMP || INCLUDE_SNMPV2 || INCLUDE_SNMPV3)
/* *********************************************************************       */
/* ******     SNMP Includes (see SNMP Reference Manual)     ************       */
/* *********************************************************************       */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include authentication code for V2; must also have
   INCLUDE_SNMPV2 set to 1 for authentication code to be enabled;
   NOTE: SNMPV2c means INCLUDE_SNMPV2 set to 1 and authenication turned off */
#define INCLUDE_SNMPV2_AUTHENTICATE 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include authentication code for V3; must also have
   INCLUDE_SNMPV3 set to 1 for authentication code to be enabled; */
#define INCLUDE_SNMPV3_AUTHENTICATE 0

/* Set to 1 to include code to support SNMP traps     */
#define INCLUDE_TRAPS 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to 1 to include code so RTIP can generate traps; see the SNMP Reference
   Manual for list of traps generated by RTIP */
#define INCLUDE_RTIP_TRAP 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* set if the mib table should include the MIB string; the strings
   are never used by SNMP but are useful when stepping through
   the code with a debugger */
#define INCLUDE_MIB_STRING 1

/* *********************************************************************       */
/* ******    SNMP Constants (see SNMP Reference Manual)     ************       */
/* *********************************************************************       */

/* Maximum length of descrtiption of the node.  See snmp_config_info in
   snmp.h */
/* NOTE: needs to be a multiple of 4 for alignment issues   */
#define _CFG_SNMP_VER_LEN 256

/* Maximum length of contact person for this node.  See snmp_config_info in
   snmp.h */
/* NOTE: needs to be a multiple of 4 for alignment issues   */
#define _CFG_SNMP_CON_LEN 256

/* Maximum number of community names which are acceptable for GET and GETN
   requests.  Valid community names are specified by calling xn_snmp_config() */
#define _CFG_SNMP_MAX_COMM_GET 2

/* Maximum number of community names which are acceptable for SET
   requests.  Valid community names are specified by calling xn_snmp_config() */
#define _CFG_SNMP_MAX_COMM_SET 2

/* Maximum length of physical location of this node.  See snmp_config_info in
   snmp.h */
/* NOTE: needs to be a multiple of 4 for alignment issues   */
#define _CFG_SNMP_LOC_LEN 256

/* Maximum length of name (fully qualified domain name) of the node.
   See snmp_config_info in snmp.h */
/* NOTE: needs to be a multiple of 4 for alignment issues   */
#define _CFG_SNMP_NAME_LEN 256

/* Maximum length of community name.  See snmp_config_info in snmp.h     */
/* NOTE: needs to be a multiple of 4 for alignment issues                */
#define _CFG_SNMP_COMM_LEN 20

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Maximum number of trap managers - trap managers are setup by
   xn_snmp_config() and xn_snmp_config_trap() */
#define _CFG_MAX_TRAP_MGRS 1

/* *********************************************************************      */
/* SNMP V2 configuration                                                      */

/* Maximum username length     */
#define _CFG_V2_USR_LEN  64
/* Maximum password length     */
#define _CFG_V2_PASS_LEN 64

/* set to a suitable 'number of bytes' to reserve for snmpv2/v3 related run-time
   storage allocation. Don't worry about this one if INCLUDE_BGET is NOT set.
 */
#define _CFG_SNMP_BGET_CORE_SIZE 4000

#endif /* INCLUDE_SNMP || INCLUDE_SNMPV2 || INCLUDE_SNMPV3 */

/* *********************************************************************       */
/* ******                     MD5                           ************       */
/* *********************************************************************       */
#if (INCLUDE_SNMPV2_AUTHENTICATE || INCLUDE_SNMPV3_AUTHENTICATE || INCLUDE_SMB_SRV || INCLUDE_SSL)
#define INCLUDE_RSA_MD5  0
#define INCLUDE_SSL_MD5  1
#else
#define INCLUDE_RSA_MD5  1
#define INCLUDE_SSL_MD5  0
#endif

#if (INCLUDE_DHCP_CLI)
/* ************************************************************************       */
/* *********************        DHCP CLIENT         ***********************       */
/* ************************************************************************       */

/* include code so timeout routine will not block when renewing lease   */
#define INCLUDE_DHCP_RENEW_NO_BLOCK 1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                            */
/* maximum number of DHCPOFFER messages the client will receive      */
#define _CFG_DHCP_OFFERS  2

/* number of times the client will re-broadcast the DHCPDISOVER message;
   the message is re-broadcast when it receives no responses */
#define _CFG_DHCP_RETRIES 1

/* number of seconds the client will wait for a server response     */
#define _CFG_DHCP_TIMEOUT 4
#endif /* INCLUDE_DHCP_CLI */

#if (INCLUDE_DHCP_SRV)
/* ************************************************************************       */
/* *********************        DHCP SERVER         ***********************       */
/* ************************************************************************       */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                   */
/* Maximum number of "address bindings" in the table,       */
/* dhcps_config.address_pool.                               */
#define _CFG_DHCPS_MAX_BINDINGS        8

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                       */
/* Maximum # of IP addresses that are "spanned" in the ranges given in the      */
/* address pool -- this is used for the address state table,                    */
/* dhcps_context.address_states;                                                */
#define _CFG_DHCPS_MAX_ADDRESSES     255

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                        */
/* The maximum number of IP addresses that have been OFFERED but not ACKED;      */
/* This specifies the size of dhcps_context.sales_pending.                       */
#define _CFG_DHCPS_MAX_SALES_PENDING   5

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                     */
/* The maximum number of IP addresses that been ACKED;        */
/* This specifies the size of dhcps_context.active_leases.    */
#define _CFG_DHCPS_MAX_ACTIVE_LEASES  10

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                             */
/* Maximum number of DNS servers to configure client with.  This      */
/* specifies the size of dhcps_params.dns_servers.                    */
#define _CFG_DHCPS_MAX_DNS_SERVERS     2

/* Number of seconds for server to keep lease longer than client
   lease time; this is used in case server and clients timers
   are not in sync */
#define _CFG_DHCPS_LEASE_TIME_DIFF    10
#endif /* INCLUDE_DHCP_CLI */

/* ************************************************************************       */
/* *********************            MAIL            ***********************       */
/* ************************************************************************       */

/* POP                                      */
/* size of buffer for server response       */
#define CFG_POP_BUF_SIZE    1000

/* maximum boundary length saved; used to determine end of message
   when reading a multi-part mime message */
#define CFG_POP_MAX_BOUNDARY 70

/* ************************************************************************      */
/* SMTP                                                                          */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING        */
/* Include extended SMTP, i.e. authentication    */
#define INCLUDE_ESMTP 0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* maximum # of recipients for a message (includes recipients to send message
   to and recipients to cc   */
#define CFG_SMTP_MAX_RCPTS  5

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* size of buffer for server response       */
#define CFG_SMTP_BUF_SIZE   500

/* SMTP and POP     */
/* timeout when waiting for response from server;
   Units: seconds */
#define _CFG_SMTP_TIMEOUT       30
#define _CFG_SMTP_INIT_TIMEOUT  60
#define _CFG_POP_TIMEOUT        30

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                           */
/* maximum length of from field in MIME message (POP and SMTP)      */
/* NOTE: needs to be a multiple of 4 for alignment issues           */
#define CFG_MAX_FROM_LEN    100

/* maximum length of subject field in MIME message (POP and SMTP)     */
/* NOTE: needs to be a multiple of 4 for alignment issues             */
#define CFG_MAX_SUBJECT_LEN 100

/* maximum user name and password for LOGIN authentication (INCLUDE_ESMTP)   */
#define CFG_MAX_LOGIN_LEN   100

#if (INCLUDE_RTIP)
/* ************************************************************************          */
/* *********************            STACKS          ***********************          */
/* ************************************************************************          */

/* IGNORE THIS SECTION IF KS_DECLARE_STACK is 0 (see osport.h)   */

#if (INCLUDE_SNMP)
#define SNMP_STACK 1
#else
#define SNMP_STACK 0
#endif

#if (INCLUDE_NFS_SRV)
#define NFS_STACK 1
#else
#define NFS_STACK 0
#endif

#if (INCLUDE_DHCP_SRV)
#define DHCP_STACK 1
#else
#define DHCP_STACK 0
#endif


/* Number of normal (small stacks) that we manage. ie stacks of size
   SIZESTACK_NORMAL. (osport.h). This is only needed if the kernel being
   used requires stacks to be provide by the user (KS_DECLARE_STACK = 1)
   in osport.h. There are two of these provided in case we are running
   in a segmnented architecture and the total stack size does not fit
   in one segment */
/* need 1 for timer, 2 per interface, main, SNMP, DHCP, FTP, TELNET etc   */
/* NOTE: maximum number of stacks for FTP and TELNET would be
         CFG_SPAWN_FTP * CFG_FTP_MAX_SPAWN +
         CFG_SPAWN_TELNET * CFG_TELNET_MAX_SPAWN
         but to cut down on the number of stacks it is defined as * 2 */

#define MAX_STACKS 0    /* declare max possible NORMAL and BIG stacks */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
#define CFG_N_STACKS_NORMAL_1   6

#if (!MAX_STACKS)
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
#define CFG_N_STACKS_NORMAL_2   (2*_CFG_NIFACES+                        \
                                 DHCP_STACK +                           \
                                 CFG_SPAWN_FTP * 2 +                    \
                                 CFG_SPAWN_TELNET * 2)
#else
/* maximum possible needed   */
#define CFG_N_STACKS_NORMAL_2   (2*_CFG_NIFACES+                         \
                                 DHCP_STACK +                           \
                                 CFG_SPAWN_FTP * CFG_FTP_MAX_SPAWN +    \
                                 CFG_SPAWN_TELNET * CFG_TELNET_MAX_SPAWN)
#endif

/* Number of big stacks that we manage. ie stacks of size
   SIZESTACK_BIG. (osport.h). This is only needed if the kernel being
   used requires stacks to be provide by the user (KS_DECLARE_STACK = 1)
   in osport.h.
*/
/* NOTE: maximum number of stacks needed for WEB is
   CFG_SPAWN_WEB*CFG_WEB_MAX_SPAWN
   but we cut down on the number of stacks it is defined as * 2 */
#if (!MAX_STACKS)
/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                   */
/* less than maximum possible needed; i.e. uses less memory */
#define CFG_N_STACKS_BIG         (1 + CFG_SPAWN_WEB * 1 +                   \
                                  SNMP_STACK +                              \
                                  NFS_STACK)
#else
/* maximum possible needed   */
#define CFG_N_STACKS_BIG         (1 + CFG_SPAWN_WEB * CFG_WEB_MAX_SPAWN +   \
                                  SNMP_STACK +                              \
                                  NFS_STACK)
#endif

#define CFG_N_STACKS_HUGE         0
#endif      /* INCLUDE_RTIP */

#if (!INCLUDE_RTIP && INCLUDE_ERTFS)
#define CFG_N_STACKS_NORMAL_1    6
#define CFG_N_STACKS_NORMAL_2    1
#define CFG_N_STACKS_BIG         1
#define CFG_N_STACKS_HUGE        0
#endif  /* !INCLUDE_RTIP && INCLUDE_ERTFS */

#if (INCLUDE_RTIP || INCLUDE_ERTFS)
/* Derive the total number of stacks     */
#define CFG_N_STACKS_TOTAL (CFG_N_STACKS_NORMAL_1 +  \
                            CFG_N_STACKS_NORMAL_2 +  \
                            CFG_N_STACKS_BIG +       \
                            CFG_N_STACKS_HUGE)
#endif      /* INCLUDE_RTIP || INCLUDE_ERTFS */

#if (INCLUDE_RTIP)
/* ************************************************************************          */
/* THE REST OF THE FILE IS NOT CONFIGURATION DATA                                    */
/* set defines for offsets in alloced ports to check                                 */
/* SPR: move this to rtip.h but socket.h uses TOTAL_PORTS                            */
/* ************************************************************************          */
#define FIRST_TCP_PORT      CFG_NUDPPORTS
#define FIRST_UDP_PORT      0
#define TOTAL_TCP_PORTS     (CFG_NUDPPORTS+CFG_NTCPPORTS)
#define TOTAL_UDP_PORTS     CFG_NUDPPORTS
#define TOTAL_PORTS         (CFG_NTCPPORTS+CFG_NUDPPORTS)
#define IS_TCP(PORT)        (1)
#define IS_UDP(PORT)        (1)
#endif  /* INCLUDE_RTIP */


/* ************************************************************************         */
#if (INCLUDE_RIPV1 || INCLUDE_RIPV2)
#define INCLUDE_RIP               1
#else
#define INCLUDE_RIP               0
#endif

/* ************************************************************************         */
/* Number of DCUs; DCUs are the internal RTIP data structure which
   contains information about packets received, about to be sent etc, etc;
   the TCP windows and other internal lists consist of linked lists of
   DCUs; DCUs contain a pointer to the data area whose size is defined
   by CFG_MAX_PACKETSIZE */
#define CFG_NDCUS    (CFG_NUM_PACKETS0+CFG_NUM_PACKETS1+CFG_NUM_PACKETS2+\
                      CFG_NUM_PACKETS3+CFG_NUM_PACKETS4+CFG_NUM_PACKETS5)

/* ************************************************************************         */
/* sanity checks                                                                    */
#if (INCLUDE_SNMPV2 || INCLUDE_SNMPV3)
#undef  INCLUDE_SNMP
#define INCLUDE_SNMP 1
#endif

#if (INCLUDE_SNMPV3)
#undef  INCLUDE_SNMPV2
#define INCLUDE_SNMPV2 1
#endif

#if (!INCLUDE_SNMP)
#undef  INCLUDE_SNMPV2
#define INCLUDE_SNMPV2 0
#undef  INCLUDE_SNMPV3
#define INCLUDE_SNMPV3 0
#endif

#if (!INCLUDE_SNMPV2)
#undef  INCLUDE_SNMPV2_AUTHENTICATE
#define INCLUDE_SNMPV2_AUTHENTICATE 0
#endif

/* integrity checks: - SNMPv3? then also SNMPv2!   */
#if (INCLUDE_SNMP && INCLUDE_SNMPV3)
  #undef INCLUDE_SNMPV2
  #undef INCLUDE_SNMPV2_AUTHENTICATE
/*  #undef INCLUDE_CRYPT   */

  #define INCLUDE_SNMPV2                1
  #define INCLUDE_SNMPV2_AUTHENTICATE   1
/*  #define INCLUDE_CRYPT                 1   */

#elif (INCLUDE_SNMP && INCLUDE_SNMPV2 && INCLUDE_SNMPV2_AUTHENTICATE != 0)
/*  #undef INCLUDE_CRYPT                    */
/*  #define INCLUDE_CRYPT                 1 */

#elif (!INCLUDE_SNMP && !INCLUDE_SSL)
/*  #undef INCLUDE_CRYPT                    */
/*  #define INCLUDE_CRYPT                 0 */

#elif (INCLUDE_PPP && INCLUDE_CHAP)
/*  #undef INCLUDE_CRYPT                    */
/*  #define INCLUDE_CRYPT                 1 */
#endif

/* ************************************************************************         */
#if (INCLUDE_RIP)
#undef  INCLUDE_ROUTING_TABLE
#define INCLUDE_ROUTING_TABLE   1
#undef  INCLUDE_RT_TTL
#define INCLUDE_RT_TTL          1
#endif

#if (INCLUDE_WINETHER)
/* ************************************************************************         */
#undef INCLUDE_SLIP
#undef INCLUDE_CSLIP
#undef INCLUDE_PPP
#undef INCLUDE_MODEM

#define INCLUDE_SLIP    0   /* DO NOT CHANGE */
#define INCLUDE_CSLIP   0   /* DO NOT CHANGE */
#define INCLUDE_PPP     0   /* DO NOT CHANGE */
#define INCLUDE_MODEM   0   /* DO NOT CHANGE */
#endif

/* ********************************************************************   */
#if (POWERPC)
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
#define EBSSMCUART      1       /* Use SMC channel for UART, driver in eth360.c */
#else
#define EBSSMCUART      0       /* Use SMC channel for UART, driver in eth360.c */
#endif
#endif

#endif /* __XNCONF__ */


