/*                                                                        */
/* RTIP.H  - RTIP header file                                             */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains prototypes constants and structure           */
/*        declarations required to build RTIP libraries and applications. */
/*        There should be no need to change values in this file           */
/*        configuration values are in xnconf.h                            */
/*                                                                        */

#ifndef __RTIP__
#define __RTIP__ 1

#include "ebs.h"
#include "os.h"          /* OS specific */

#if (INCLUDE_SNMP)
#include "snmpapi.h"
#endif

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* DEBUG DEFINES                                                           */
/* *********************************************************************   */
#define DEBUG_SEND_NO_BLOCK 0       /* set to aid in debug of xmitting */
                                    /* packets thru the IP layer   */
                                    /* i.e. turns on DEBUG_ERRORs  */

#define DEBUG_NO_DCU_BLOCK 1    /* debug blocking when num DCUs is low */

/* *********************************************************************   */
/* CHECK CONFIGURATION DEFINES                                             */
/* *********************************************************************   */

#if (INCLUDE_RUN_TIME_CONFIG)
#define LOOP_THRU_IFACES(I) for (I = 0; I < CFG_NIFACES; I++)
#define BREAK_IF_LOOP       break;
#define CONTINUE_IF_LOOP    continue;

#elif (CFG_NIFACES > 1)
#define LOOP_THRU_IFACES(I) for (I = 0; I < CFG_NIFACES; I++)
#define BREAK_IF_LOOP       break;
#define CONTINUE_IF_LOOP    continue;

#elif (CFG_NIFACES == 1)
#define LOOP_THRU_IFACES(I) I = 0;
#define BREAK_IF_LOOP
#define CONTINUE_IF_LOOP    return;
#endif

#if (INCLUDE_MALLOC_DCU_INIT && INCLUDE_MALLOC_DCU_AS_NEEDED)
#error: turn off INCLUDE_MALLOC_DCU_INIT or INCLUDE_MALLOC_DCU_AS_NEEDED
#endif

#if (INCLUDE_WEB        || INCLUDE_FTP_SRV || INCLUDE_FTP_CLI ||    \
     INCLUDE_TELNET_SRV || INCLUDE_TELNET_CLI ||                    \
     INCLUDE_SMTP       || INCLUDE_POP3 || INCLUDE_SSL)
#if (!INCLUDE_TCP)
#error: INCLUDE_TCP must be turned on
#endif
#endif

#if (INCLUDE_NFS_CLI || INCLUDE_NFS_CLI || INCLUDE_TFTP_SRV || INCLUDE_TFTP_CLI ||     \
     INCLUDE_DHCP_SRV   || INCLUDE_DHCP_CLI || INCLUDE_SNMP || INCLUDE_DNS)
#if (!INCLUDE_UDP)
#error: INCLUDE_UDP must be turned on 
#endif
#endif

#if (!INCLUDE_UDP && INCLUDE_RAW)
#error: UDP must be on for RAW to be on.
#endif

#if (INCLUDE_TCP)
#if (!INCLUDE_TCP_NO_COPY && !INCLUDE_TCP_COPY)
#error TCP: copy or no copy must be on.
#endif
#endif      /* INCLUDE_TCP */

#if (INCLUDE_RUN_TIME_CONFIG)
#if (!INCLUDE_ROUTING_TABLE)
#error: INCLUDE_ROUTING_TABLE must be on if INCLUDE_RUN_TIME_CONFIG
#endif
#else
#if (!INCLUDE_ROUTING_TABLE && (CFG_NIFACES > 1))
#error: INCLUDE_ROUTING_TABLE must be on if more than 1 interface
#endif
#endif      /* INCLUDE_RUN_TIME_CONFIG */

#if (INCLUDE_NAT && !INCLUDE_ROUTER)
#error: if INCLUDE_NAT is on, INCLUDE_ROUTER must be on
#endif

#if (INCLUDE_SNMP)
#if (!INCLUDE_ROUTING_TABLE || !INCLUDE_ARP)
#error: INCLUDE_ROUTING_TABLE and INCLUDE_ARP must be on for SNMP
#endif
#endif

#if (INCLUDE_IGMP && INCLUDE_IGMP_V2)
#error: Both INCLUDE_IGMP and INCLUDE_IGMP_V2 are set
#endif

#if (!INCLUDE_SMTP && INCLUDE_ESMTP)
#error: INCLUDE_SMTP must be set if INCLUDE_ESMTP is not set
#endif

#if (INCLUDE_ARP && INCLUDE_ETH_BUT_NOARP)
#error: Both INCLUDE_ARP and INCLUDE_ETH_BUT_NO_ARP are set
#endif

#if (INCLUDE_PPP)
#if (INCLUDE_PPP_DNS && !INCLUDE_DNS)
#error: INCLUDE_DNS must be set if INCLUDE_PPP_DNS is set
#endif
#endif      /* INCLUDE_PPP */

#if (!INCLUDE_PPP && INCLUDE_PPPOE)
#error: INCLUDE_PPP must be on if INCLUDE_PPPOE is on
#endif

#if (PPC603)
#if (INCLUDE_TASK_ISRS)
#error: BUG - Feature not implemented for this port yet
#endif
#endif

#if (INCLUDE_PCI && defined (RTKBCPP) && !INCLUDE_MALLOC_DCU_INIT && !INCLUDE_MALLOC_DCU_AS_NEEDED)
/* for power pack we allocate this   */
#error DPMI must set INCLUDE_MALLOC_DCU_INIT for PCI cards
#endif

#if (BUILD_NEW_BINARY && INCLUDE_NO_DCU_BLOCK)
#error: turn of BUILD_NEW_BINARY or INCLUDE_NO_DCU_BLOCK
#endif

#if (INCLUDE_RUN_TIME_CONFIG && KS_DECLARE_STACK)
#error: Not supported yet due to declarations of stack
#endif

#if (INCLUDE_MFS && !INCLUDE_VFS)
#error: VFS must be on for MFS
#endif

#endif      /* INCLUDE_RTIP */

/* ************************************************************************       */
/* ************************************************************************       */
#define FRAG_ROUTER 1       /* router will fragment */

/* ************************************************************************       */
/* *********************      TCP BEHAVIOR   ******************************       */
/* ************************************************************************       */

/* NOTE:                                                               */
/* THE FOLLOW 2 CONSTANTS ARE INCLUDED FOR BACKWARD COMPATABILITY ONLY */
/* IF YOU CHANGE ONE, YOU SHOULD CHANGE BOTH                           */

/* wake up select for TCP when output window is empty (versus any room   */
/* in output window)                                                     */
#define SELECT_TCP_EMPTY 0

/* when send in in non-blocking mode, will return if cannot queue all the   */
/* data (versus queueing what it can)                                       */
#define SEND_TCP_QUE_ALL 0

/* Set to 1 if you want Steven's definition of NAGLE vs RFC   */
#define NAGLE_STEVENS 0

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* GENERAL DEFINES                                                         */
/* *********************************************************************   */

/* directory delimiter used by file system   */
#define DIR_SLASH_CHAR '\\'

/* add-ons use xn_pkt API calls instead of socket calls;   */
/* WARNING: this is not supported                          */
#if (INCLUDE_PKT_API)
#define USE_PKT_API 0   /* DO NOT CHANGE */
#else
#define USE_PKT_API 0
#endif

/* Structure copy causes call to memcpy(), and we can't have that on   */
/* SH1 platform                                                        */
#define STRUCT_COPY(x, y) tc_movebytes((PFBYTE)&(x), (PFBYTE)&(y), sizeof(y))

#if !defined(NULL)
#define NULL ((void *)0)
#endif

/* The NSI standard defines offsetof, but some compiler just don't have it. Or put it
   in weird places. So we define our own and use our preprocessor
   to produce a suitable offsetof() alike operator.

   Returns offset of field with [type] in 'chars' (bytes)

   E.g. xn_offsetof(struct os_list, pprev) --> 2 or 4 of maybe even 6 or 8. Depends on your local pointer size and alignment of structure elements.
        xn_offsetof(struct _arppkt, ar_sha) --> 8 (2+2+1+1+2)
 */
#define xn_offsetof(type, field)                  \
  (int)(((char *)                                 \
         &((type *)0)->field                      \
        ) - ((char *)0))

/* address comparison   */
#define ADDR_GR_EQUAL(addr1, addr2) ( (dword)(addr1) >= (dword)(addr2) )
#define ADDR_LESS_EQUAL(addr1, addr2) ( (dword)(addr1) <= (dword)(addr2) )

/* address addition   */
#define ADDR_ADD(addr1, addr2)  ( (dword)(addr1) + (dword)(addr2) )
/* address difference   */
#define ADDR_DIFF(addr1, addr2) ( (dword)(addr1) - (dword)(addr2) )

#define CONVERT_ADDR_DWORD(ADDR_DWORD, ADDR_ARRAY)  \
    tc_mv4(&ADDR_DWORD, ADDR_ARRAY, IP_ALEN);       \
    ADDR_DWORD = net2hl(ADDR_DWORD);

#define IP_ALEN  4              /* number of bytes in an IP address */
#define ETH_ALEN 6              /* number of bytes in an ETHERNET address */
#define TOK_ALEN 6              /* number of bytes in a Token Ring address */
#define LL_ALEN  6              /* number of bytes in a Link Layer address */
                                /* (token ring or ethernet)   */

#define ETHERSIZE  1514         /* maximum number of bytes in ETHERNET packet */
                                /* (used by ethernet drivers)   */

#define ETHER_MIN_LEN       64  /* minimum number of bytes in an ETHERNET */
                                /* packet   */
#define ETHER_NOCHK_MIN_LEN 60  /* minimum number of bytes in an ETHERNET */
                                /* packet not including checksum   */

/* maximum packet size   */
#define MAX_PACKETSIZE (CFG_MAX_PACKETSIZE+CFG_PACKET_ADJ)

#endif  /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* DCU DEFINES                                                             */
/* *********************************************************************   */

#define DCU PPACKET
#define DCUTOPACKET(X)   ((PPACKET) (X))
#define DCUTOCONTROL(X)  (X)->ctrl

#define DCUDATA(X)       ((PPACKET)(X))->data
#define DCUTODATA(X)     ((PFBYTE)DCUDATA(X))

/* macros which assume PROTOCOL pointers are set up   */
#define DCUTOETHERPKT(X) ( (PETHER)   ((PPACKET) (X))->data )
#define DCUTOTOKPKT(X)   ( (PTOKPKT) ((PPACKET) (X))->data )
#define DCUTOIPPKT(X)    ( (PIPPKT)  (DCUTOPACKET(X)->inet.ptr) )
#define DCUTOARPPKT(X)   ( (PARPPKT) (DCUTOPACKET(X)->inet.ptr) )
#define DCUTOICMPPKT(X)  ( (PICMPPKT)(DCUTOPACKET(X)->prot.ptr) )
#define DCUTOIGMPPKT(X)  ( (PIGMPPKT)(DCUTOPACKET(X)->prot.ptr) )
#define DCUTOTCPPKT(X)   ( (PTCPPKT) (DCUTOPACKET(X)->prot.ptr) )
#define DCUTOUDPPKT(X)   ( (PUDPPKT) (DCUTOPACKET(X)->prot.ptr) )
#define DCUTOPROTPKT(X)  ( (PFBYTE)  (DCUTOPACKET(X)->prot.ptr) )

/* macros which calculate PROTOCOL pointers   */
#if (INCLUDE_802_2)
#define DCUSETUPIPARP(MSG, IS_802_2)                            \
    DCUTOPACKET(MSG)->inet.ptr =                                \
        IS_802_2 ?                                              \
        DCUTODATA(MSG) + LL_HLEN_BYTES + LLC_SNAP_HLEN_BYTES :  \
        DCUTODATA(MSG) + LL_HLEN_BYTES;
#else
#define DCUSETUPIPARP(MSG, IS_802_2)                            \
    DCUTOPACKET(MSG)->inet.ptr =                                \
        ((PFBYTE)(MSG->data))+LL_HLEN_BYTES;
#endif
#if (INCLUDE_802_11)
#define DCUSETUPIPARP_802_11(MSG)                               \
    DCUTOPACKET(MSG)->inet.ptr =                                \
        ((PFBYTE)(MSG->data))+sizeof(struct frame_format_802_11);
#endif

/* macro to setup pointer to start of protocol (UDP, TCP etc) header in packet   */
#define DCUSETUPPROT(MSG)                                       \
    DCUTOPACKET(MSG)->prot.ptr = DCUTOPACKET(MSG)->inet.ptr +   \
                                 IP_HLEN_BYTES + MSG->ip_option_len;

/* macro to setup pointers to start of IP header in packet   */
#define DCUSETUPPTRS(MSG, IS_802_2)         \
    DCUSETUPIPARP(MSG, IS_802_2)            \
    DCUSETUPPROT(MSG)

#if (INCLUDE_802_2)
#define DCUTOSNAP8022(MSG) ((PSNAP)((PFBYTE)DCUTODATA(MSG) + LL_HLEN_BYTES + \
                                            LLC_HLEN_BYTES))
#define DCUTOLLC8022(MSG)  ((PLLCPKT)((PFBYTE)DCUTODATA(MSG) + LL_HLEN_BYTES))
#endif

/* copy DCU (from) to DCU to; flags are set up to not keep and not signal   */
#define COPY_DCU_FLAGS(to, from)                                                \
    tc_movebytes(DCUTODATA(to), DCUTODATA(from), DCUTOPACKET(from)->length);\
    DCUTOCONTROL(to).dcu_flags = NO_DCU_FLAGS;                              \
    DCUTOPACKET(to)->length = (word)DCUTOPACKET(from)->length;

/* *********************************************************************   */
/* ******    TOTAL PACKET SIZE MACROS                              *****   */
/* *********************************************************************   */
/* total header size of IP packet (ethernet and IP with options)           */

#if (INCLUDE_802_2)
#define SEND_802_2(PI, PORT) ( ((PORT) && ((PORT)->options & SOO_802_2))  ||    \
                               ((PI)   && ((PI)->iface_flags & IS_802_2)) )
#else
#define SEND_802_2(PI, PORT) (FALSE)
#endif

/* link layer size; for ethernet it is ETH_ALEN, token ring is TOK_ALEN   */
/* does not include LLC,SNAP headers                                      */
/* NOTE: ethernet and token ring are same size                            */
#define LL_HLEN_BYTES ETH_HLEN_BYTES

#if (INCLUDE_802_2)
#define TOTAL_LL_HLEN_SIZE(SIZE, IS_802, PI, MSG)           \
    SIZE = LL_HLEN_BYTES;                                   \
    if (IS_802)                                             \
        SIZE += LLC_SNAP_HLEN_BYTES;
#else
#define TOTAL_LL_HLEN_SIZE(SIZE, IS_802, PI, MSG)           \
    SIZE = LL_HLEN_BYTES;
#endif

#if (INCLUDE_802_2)
#define TOTAL_ETH_HLEN_SIZE(SIZE, IS_802, MSG)              \
    SIZE = ETH_HLEN_BYTES;                                  \
    if (IS_802)                                             \
        SIZE += LLC_SNAP_HLEN_BYTES;
#else
#define TOTAL_ETH_HLEN_SIZE(SIZE, IS_802, MSG)              \
    SIZE = ETH_HLEN_BYTES;
#endif

/* ********                                                      */
/* total header size of IP packet (ethernet and IP with options) */
#define TOTAL_IP_HLEN_NOOPT_SIZE(SIZE, PORT, PI, MSG)               \
    TOTAL_LL_HLEN_SIZE(SIZE,                                        \
                       SEND_802_2(PI, PORT),                        \
                       PI, MSG)                                     \
    SIZE += IP_HLEN_BYTES;

/* total header size of IP packet (ethernet and IP with options)   */
#define TOTAL_IP_HLEN_SIZE(SIZE, PORT, PI, MSG)                     \
    TOTAL_LL_HLEN_SIZE(SIZE,                                        \
                       SEND_802_2(PI, PORT),                        \
                       PI, MSG)                                     \
    SIZE += IP_HLEN_BYTES;                                          \
    SIZE += PORT->ip_option_len;

/* ********                                                           */
/* total header size of TCP packet (ethernet, IP and TCP headers with */
/* NO options)                                                        */
#define TOTAL_TCP_HLEN_NOOPT_SIZE(SIZE, PORT, PI, MSG)              \
    TOTAL_IP_HLEN_NOOPT_SIZE(SIZE, ((PANYPORT)PORT), PI, MSG)       \
    SIZE = SIZE + TCP_HLEN_BYTES;

/* total header size of TCP packet (ethernet, IP and TCP headers with options)   */
#define TOTAL_TCP_HLEN_SIZE(SIZE, PORT, PI, MSG)                    \
    TOTAL_IP_HLEN_SIZE(SIZE, ((PANYPORT)PORT), PI, MSG)             \
    SIZE = SIZE + TCP_HLEN_BYTES;                                   \
    SIZE += PORT->tcp_option_len;

/* ip_len for a TCP packet   */
#define TOTAL_IP_TCP_SIZE(SIZE, PORT, PI, MSG, DLEN)            \
    SIZE = IP_HLEN_BYTES + PORT->ap.ip_option_len +             \
           TCP_HLEN_BYTES + PORT->tcp_option_len +              \
           DLEN;

/* ********                                                                    */
/* total header size of UDP packet (ethernet, IP and UDP headers with options) */
#define TOTAL_UDP_HLEN_SIZE(SIZE, PORT, PI, MSG)                      \
    SIZE = TOTAL_IP_HLEN_SIZE(SIZE, ((PANYPORT)PORT), PI, MSG)        \
    SIZE += UDP_HLEN_BYTES;

/* *********************************************************************   */
/* ******               LINK LAYER HEADER                          *****   */
/* *********************************************************************   */
#define LL_DEST_ADDR(MSG, PI)                           \
        DCUTOETHERPKT(MSG)->eth_dest

#define LL_SRC_ADDR(MSG, PI)                            \
        DCUTOETHERPKT(MSG)->eth_source


#if (INCLUDE_802_2)
#define SETUP_LL_HDR(MSG, PI, TYPE, IS_802_2, LEN)      \
    SETUP_ETH_HDR(MSG, PI->addr.my_hw_addr, TYPE)       \
    if (IS_802_2)                                       \
        setup_802_2_header(MSG, (word)(LEN));
#else
#define SETUP_LL_HDR(MSG, PI, TYPE, IS_802_2, LEN)      \
    SETUP_ETH_HDR(MSG, pi->addr.my_hw_addr, TYPE)
#endif

#define SETUP_LL_HDR_DEST(PI, MSG, DEST_HW_ADDR)        \
    SETUP_ETH_HDR_DEST(MSG, DEST_HW_ADDR)

#define SETUP_LL_HDR_SRC(PI, MSG, SRC_HW_ADDR)          \
    SETUP_ETH_HDR_SRC(MSG, SRC_HW_ADDR)

#endif      /* INCLUDE_RTIP */

/* *********************************************************************   */
/* ******    Pointers to socket structures                         *****   */
/* *********************************************************************   */
#define PUDPPORT  struct _udpport KS_FAR *
#define PTCPPORT  struct _tcpport KS_FAR *
#define PANYPORT  struct _anyport KS_FAR *
typedef struct _iface KS_FAR * PIFACE;

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******    Control structures                                    *****   */
/* *********************************************************************   */

/* OS specific packet control structure. Objects in this structure are touched   */
/* only by the os specific porting layer                                         */
typedef struct _os_packet_control
{
#define PACKET_OFFSET       0
#define TCP_WINDOW_OFFSET   1
#define TCP_OOO_OFFSET      2
#define OUTPUT_LIST_OFFSET  2
#if (INCLUDE_XMIT_QUE)
#define XMIT_LIST_OFFSET    2
#endif
    OS_LIST list1;          /* Must be at the beginning of the DCU; */
                            /* pointer to next or previous DCU on   */
                            /* free list, arp cache, udp exchange,  */
                            /* input list, ip exchange etc.         */
    OS_LIST list2;          /* Must be second entry of the DCU; */
                            /* pointer to next and previous DCU on   */
                            /* TCP window                            */
    OS_LIST list3;          /* Must be third entry of the DCU; */
                            /* pointer to next and previous DCU on TCP   */
                            /* out of order list, output list            */

#if (INCLUDE_TRK_PKTS)
    int      index;         /* packet number */
    int      requestor;     /* who alloced the packet */
#define IN_NO_LIST          -1
#define IN_FRAG_LIST        1
#define IN_OUT_WIN_LIST     2
#define IN_IN_WIN_LIST      3
#define IN_IP_LIST          4
#define IN_UDP_LIST         5
#define IN_PING_LIST        6
#define IN_INPUT_LIST       7
#define IN_ARP_LIST         8
#define IN_RS232_LIST       9
#define IN_FREE_LIST        10
#define IN_OOO_LIST         11
#define OUT_OOO_LIST        12
#define OUT_IP_LIST         13
#define IN_TCP_LAYER        14
#define IN_UDP_LAYER        15
#define IN_ICMP_LAYER       16
#define IN_IGMP_LAYER       17
#define IN_ROUTE_LAYER      18
#define IN_ARP_LAYER        19
#define IN_RARP_LAYER       20
#define IN_PPPOED_LAYER     21
#define IN_PPPOES_LAYER     22
    int    list_id;
#endif

    /* Flag values (saved based upon parameter to tc_netwrite and other   */
    /* flags related to outputing packets)                                */
    /* NOTE: PKT_FLAG_SIGNAL and PKT_FLAG_KEEP_ERROR do not work in       */
    /*       conjunction but we never use that combination                */
    #define NO_DCU_FLAGS        0x0000  /* No flags are set */
    #define PKT_FLAG_KEEP       0x0001  /* Don't discard after sending if set */
    #define PKT_FLAG_KEEP_ERROR 0x0002  /* Don't discard on error; */
                                        /* only used if API will return           */
                                        /* the error, i.e. for tc_release_message */
                                        /* which is called after the              */
                                        /* packet has been xmitted, this is       */
                                        /* equivalent to NO_DCU_FLAGS             */
    #define PKT_FLAG_SIGNAL     0x0004  /* Signal "sem (below)" after sending */
    /* flags used to ensure packet not freed before it is output   */
    #define PKT_SEND_IN_PROG    0x0008  /* packet on output list */
    #define PKT_FREE            0x0010  /* packet freed but not sent yet */
    /* flags used signal IP layer of event   */
    #define CLOSE_IFACE         0x0020  /* IP task will close interface */
                                        /* when output que empties   */
    #define MODEM_DROP          0x0040  /* IP task will process modem */
                                        /* drop by calling callback routine   */
    int  dcu_flags;

    PIFACE pi;                      /* interface packet came in if input */
                                    /* packet (set up by tc_ip_interpret);    */
                                    /* tc_netwrite() sets it to iface sending */
                                    /* (saved in case put on arp cache)       */
    PANYPORT port;                  /* port associated with the packet; */
                                    /* 1) arp layer: if this packet is         */
                                    /*    from a UDP or TCP port               */
                                    /*    further port specific                */
                                    /*    processing is needed (such as        */
                                    /*    signalling the socket etc. etc)      */
                                    /*    NOTE: the packet is stored on        */
                                    /*    the arp cache                        */
                                    /* 2) out of order processing: if          */
                                    /*    run out of DCUs, scarfs one from     */
                                    /*    the out of order list                */
                                    /* 3) used to pass port to os_alloc_packet */
                                    /*    if possibility of blocking on        */
                                    /*    available packet                     */
} OS_PACKET_CTRL;

/* ********************************************************************   */
/* free DCU depending upon dcu_flags; this is used to determine whether   */
/* to free the DCU when if an error occurs, this error will be returned   */
/* to the application                                                     */
/* NOTE: xn_pkt_send will set PKT_FLAG_KEEP_ERROR for UDP when            */
/*       wait count is 0                                                  */
/* NOTE: tc_release_msg will not check PKT_FLAG_KEEP_ERROR since the      */
/*       API will already have returned if PKT_FLAG_KEEP_ERROR is         */
/*       set                                                              */
#define FREE_DCU(MSG, DCU_FLAGS)                                            \
    if (!(DCU_FLAGS & PKT_FLAG_KEEP) && !(DCU_FLAGS & PKT_FLAG_KEEP_ERROR))     \
        os_free_packet(MSG);        /* Free message */


/* ********************************************************************   */
/* set up protocols which use exchange                                    */
#if (INCLUDE_UDP || INCLUDE_RAW || INCLUDE_PING)
#define USE_PORT_EXCHANGE 1
#else
#define USE_PORT_EXCHANGE 0
#endif

/* OS specific port control structure. Objects in this structure are touched   */
/* only by the os specific porting layer                                       */
typedef struct _os_port_control
{
#define PORT_GEN_OFFSET 0
#define PORT_DCU_OFFSET 1
    OS_LIST  list1;         /* Must be at the beginning of the PORT; */
                            /* pointer to next socket on Free list, PING,   */
                            /* list, or various TCP or UDP lists            */
#if (INCLUDE_NO_DCU_BLOCK)
    OS_LIST  list2;         /* Must be at the second in the PORT; */
                            /* pointer to next socket on list of sockets   */
                            /* waiting for more DCUs                       */
#endif
#if (USE_PORT_EXCHANGE)
    POS_LIST exch_list[1];  /* UDP/PING input packet exchange, */
                            /* 0 = UDP, 0 = PING                   */
                            /* this works since PING uses a socket */
#endif
    int      index;         /* port number */
    int      sent_status;   /* If port is waiting for ARP to complete it */
                            /* checks here for the result after being   */
                            /* signaled                                 */
                            /*   0 = success;                           */
                            /*   nonzero = failure = value of errno     */
    int      read_status;   /* If port is waiting for input data it */
                            /* checks here for the result after being   */
                            /* signaled                                 */
                            /*   0 = success;                           */
                            /*   nonzero = failure = value of errno     */
    int      write_status;  /* If port is waiting for output data to be acked */
                            /* it checks here for the result after be   */
                            /*   0 = success;                           */
                            /*   nonzero = failure = value of errno     */
    PANYPORT select_root[CFG_NUM_SELECT_P_SOCK];    
                            /* points to the first socket in the group if   */
                            /* this socket is in a select group. Only       */
                            /* valid during a select call.                  */
    #define WRITE_SELECT     0x01
    #define READ_SELECT      0x02
    #define EXCEPTION_SELECT 0x04
    int     select_flags[CFG_NUM_SELECT_P_SOCK];    
                            /* lets os layer know which event select is   */
                            /* waiting to be signalled                    */
    int     rtip_errno;     /* errno value for port; used by os layer and */
                            /* by tc_tcp_interpret   */
#if (INCLUDE_NO_DCU_BLOCK)
    word    block_ticks;    /* number of ticks to block */
#endif
} OS_PORT_CONTROL;

/* ********************************************************************   */
/* OS specific arp cache control structure. Objects in this structure     */
/* are touched only by the os specific porting layer                      */
typedef struct _os_arpc_control
{
    POS_LIST msg_list;      /* Queue for outgoing packets awaiting */
                            /* resolution                               */
                            /* When resolved they are dequeued and sent */
} OS_ARPC_CONTROL;

/* ********************************************************************   */
/* OS specific interface control structure. Objects in this structure     */
/* are touched only by the os specific porting layer                      */
typedef struct _os_iface_control
{
    int      index;
    POS_LIST exch_list[1];    /* 0 == IP */
    POS_LIST list_output;     /* Packets which need to be sent */
#if (INCLUDE_XMIT_QUE)
    POS_LIST list_xmit;       /* Packets which xmit has started */
#endif
    POS_LIST list_input;      /* Packets received by driver ISR layer */
                              /* but not yet placed on the ip exchange   */
    POS_LIST root_ping_ports; /* List of ports waiting for PING response */
#if (INCLUDE_PPP)
    #define PPP_SIGNAL_SUCCESS  1
    #define PPP_SIGNAL_FAIL     2
    int      signal_status;   /* status send to xn_lcp_open() which */
                              /* set the PPP signal   */
#endif

#if (INCLUDE_PPP)
    PANYPORT echo_port;       /* PORT used for echo */
#endif

    KS_RTIPSEM ifsem;
    KS_RTIPSIG ifsig[NUM_SIG_PER_IFACE];
} OS_IFACE_CONTROL;
#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******     PROTOCOL FRAMES as they appear "on the wire"  ************   */
/* ******     Note that the structures are nested. with     ************   */
/* ******     ethernet at the lowest layer.                 ************   */
/* ******     This makes the implementation ethernet        ************   */
/* ******     specific or at least the transport layer      ************   */
/* ******     must emulate ethernet at the driver layer     ************   */
/* ******     see slip and ppp for examples                 ************   */
/* *********************************************************************   */

/* Protocol frames   */

#if (INCLUDE_802_11)
#define DCUTO802_11(X)   ( (PFRAME_FORMAT_802_11) ((PPACKET) (X))->data )

typedef struct frame_format_802_11
{
    #define PROTOCOL_VERSION_MASK   0xc000

        #define FRAME_T_MGMT            0x0000
        #define FRAME_T_CTRL            0x1000
        #define FRAME_T_DATA            0x2000
    #define FRAME_TYPE              0x3000

        /* FRAME_TYPE = FRAME_T_MGMT   */
        #define FRAME_ST_ASS_REQ        0x0000
        #define FRAME_ST_ASS_RESP       0x0100
        #define FRAME_ST_REASS_REQ      0x0200
        #define FRAME_ST_REASS_RESP     0x0300
        #define FRAME_ST_PROBE_REQ      0x0400
        #define FRAME_ST_PROBE_RESP     0x0500
        #define FRAME_ST_BEACON         0x0800
        #define FRAME_ST_ATIM           0x0900
        #define FRAME_ST_DISASS         0x0a00
        #define FRAME_ST_AUTH           0x0b00
        #define FRAME_ST_DEAUTH         0x0c00

        /* FRAME_TYPE = FRAME_T_CTRL   */
        #define FRAME_ST_PS             0x0a00
        #define FRAME_ST_RTS            0x0b00
        #define FRAME_ST_CTS            0x0c00
        #define FRAME_ST_ACK            0x0d00
        #define FRAME_ST_CF             0x0e00
        #define FRAME_ST_CF_END_ACK     0x0f00

        /* FRAME_TYPE = FRAME_T_DATA   */
        #define FRAME_ST_DATA           0x0000
        #define FRAME_ST_DATA_ACK       0x0100
        #define FRAME_ST_DATA_POLL      0x0200
        #define FRAME_ST_DATA_ACK_POLL  0x0300
        #define FRAME_ST_NDATA          0x0400
        #define FRAME_ST_NDATA_ACK      0x0500
        #define FRAME_ST_NDATA_POL      0x0600
        #define FRAME_ST_NDATA_ACK_POLL 0x0700
    #define FRAME_SUBTYPE           0x0f00

    #define TO_DS                   0x0080
    #define FROM_DS                 0x0040
    #define MORE_FLAGS              0x0020
    #define RETRY                   0X0010
    #define POWER_MGT               0x0008
    #define MORE_DATA               0x0004
    #define WEP                     0x0002
    #define ORDER                   0x0001
    word frame_control;

    word duration_id;
    byte address_1[6];
    byte address_2[6];
    byte address_3[6];
    word seq_ctrl;
    byte address_4[6];
} FRAME_FORMAT_802_11;
typedef FRAME_FORMAT_802_11 KS_FAR * PFRAME_FORMAT_802_11;

#endif      /* INCLUDE_802_11 */

/* ********                          */
/* Ethernet protocol frame structure */
#define ETH_HLEN_BYTES 14

#define IS_ETH_ADDR_EQUAL(ADDR1, ADDR2) \
    tc_comparen(ADDR1, ADDR2, ETH_ALEN)

#if (RTIP_VERSION >= 26)
#define SETUP_ETH_HDR(MSG, MY_HW_ADDR, TYPE)                            \
    DCUTOETHERPKT(MSG)->eth_type = TYPE;                                \
    tc_movebytes(DCUTOETHERPKT(MSG)->eth_source, MY_HW_ADDR, ETH_ALEN);
#define SETUP_ETH_HDR_DEST(MSG, DEST_HW_ADDR)                           \
    tc_movebytes(DCUTOETHERPKT(MSG)->eth_dest, DEST_HW_ADDR, ETH_ALEN);
#define SETUP_ETH_HDR_SRC(MSG, SRC_HW_ADDR)                             \
    tc_movebytes(DCUTOETHERPKT(MSG)->eth_source, SRC_HW_ADDR, ETH_ALEN);
#endif

typedef struct _ether
{
    byte eth_dest[ETH_ALEN];    /* where the packet is going */
    byte eth_source[ETH_ALEN];  /* who it came from */
    word eth_type;              /* Ethernet packet type for 802.3 */
                                /* or length of (IP layer + LLC,SNAP) if 802.2   */
} ETHER;
typedef ETHER KS_FAR * PETHER;
typedef ETHER ETHERPKT;

#if (INCLUDE_TOKEN || INCLUDE_802_2)
/* ********   */
#define LLC_SNAP_HLEN_BYTES 8
#define LLC_SNAP_DATA_LEN   6   /* LLC, SNAP header length which is the same */
#define LLC_HLEN_BYTES      3

typedef struct _LLCPKT
{
    byte llc_dsap;          /* destination service access point */
    byte llc_ssap;          /* source service access point */
    byte llc_cntl;
} LLCPKT;
typedef LLCPKT KS_FAR * PLLCPKT;

typedef struct _SNAP
{
    byte snap_org_code[3];
    word snap_type;         /* same as eth_type in ethernet header */
} SNAP;
typedef SNAP KS_FAR * PSNAP;

#endif

/* ********                     */
/* Arp protocol frame structure */
typedef struct _arppkt
{
    word    ar_hw_type;
    word    ar_pr_type;
    byte    ar_hw_len;
    byte    ar_pr_len;
    word    ar_opcode;

    /* These fields are wired for IP over Ethernet. The sizes are   */
    /* actually variable length depending on the values above.      */
    byte    ar_sha[ETH_ALEN];    /* Sender's hardware address */
    byte    ar_spa[IP_ALEN];     /* Sender's protocol address */
    byte    ar_tha[ETH_ALEN];    /* Target's hardware address */
    byte    ar_tpa[IP_ALEN];     /* Target's protocol address */
} ARPPKT;
typedef struct _arppkt KS_FAR * PARPPKT;

/* ********                    */
/* ip protocol frame structure */

#define IPV4 4      /* ip_verlen field (hi order 4 bytes) for IPV4 */
#define IPV6 6      /* ipv6_verlen field (hi order 4 bytes) for IPV6 */

#define IP_ISMCAST_ADDR(X)  (((X) & 0xf0) == 0xe0)

#define IP_LOOP_ADDR  0x7f      /* most significant byte of loopback addr */
                                /* least significant 3 bytes are don't care   */
#define ip_broadaddr  ip_ffaddr /* broadcast address */

#define IP_HLEN_BYTES 20        /* min num of bytes in IP header */
#define IP_TLEN_BYTES (IP_HLEN_BYTES+LL_HLEN_BYTES)

/* macros to determine number of bytes in an IP header; converts number   */
/* of dwords (32-bit words) to number of bytes                            */
#define IP_HLEN(pip)       ((pip->ip_verlen&0x0f)<<2)
#define IP_HLEN_WORDS(pip) ((pip->ip_verlen&0x0f) << 1)

typedef struct _ippkt
{
    byte    ip_verlen;          /* 4 hi bits == version, lo 4 == length */
                                /* len includes options (# 32 bit words)   */
    byte    ip_tos;             /* service type */
    word    ip_len;             /* total len(including options + data (# bytes)) of */
                                /* of packet   */
    word    ip_id;              /* */
    word    ip_fragoff;         /* 3 hi bits == flags, lo 13 == frag offset */
    byte    ip_ttl;             /* time to live */
    byte    ip_proto;           /* protocol i.e. TCP, UDP */
    word    ip_cksum;           /* header checksum */
    byte    ip_src[IP_ALEN];
    byte    ip_dest[IP_ALEN];

    /* IP options and padding (if any) are here   */
    #define END_OF_LIST_OPTION  0   /* end of list */
    #define NOP_OPTION          1   /* NOP */
    #define LOOSE_ROUTE_OPTION  3   /* loose source route */
    #define TIMESTAMP_OPTION    4   /* timestamp */
    #define RECORD_ROUTE_OPTION 7   /* record route */
    #define STRICT_ROUTE_OPTION 9   /* strict source route */
#if (INCLUDE_IGMP_V2)
    #define ROUTER_ALERT_OPTION 0x94 /* router alert Type 1|00|10100 */
                                     /* copied flag = 1, option class = 0   */
                                     /* option number = 20                  */
    #define ROUTER_ALERT_LEN    4
#endif

} IPPKT;
typedef IPPKT KS_FAR * PIPPKT;

/* ********                     */
/* udp protocol frame structure */

#define UDP_HLEN_BYTES 8        /* UDP header len in units of bytes */
#define UDP_TLEN_BYTES (UDP_HLEN_BYTES+IP_HLEN_BYTES+LL_HLEN_BYTES)

typedef struct _udp
{
    word    udp_source;         /* Source port bound to */
    word    udp_dest;           /* Destination port connected to */
    word    udp_len;            /* Length of data to follow */
    word    udp_chk;            /* checksum */
} UDPPKT;
typedef UDPPKT KS_FAR * PUDPPKT;


/* ********                     */
/* tcp protocol frame structure */

#define TCP_HLEN_SFT     5<<4 /* TCP header len with no options in units of bytes */
                              /* shifted to proper bits in field in packet   */
#define TCP_HLEN_BYTES   20   /* number of bytes in TCP header */
#define TCP_TLEN_BYTES (TCP_HLEN_BYTES+IP_HLEN_BYTES+LL_HLEN_BYTES)

typedef struct _tcp
{
    word    tcp_source;        /* Source port */
    word    tcp_dest;          /* Destination port */

    /* Note tcp_seq and tcp_ack are longs, but on some architectures such   */
    /* as Alpha and PowerPC longs must be long aligned. With ethernet       */
    /* frames this is not true so we make them arrays of words.             */
    word    tcp_seq[2];        /* Sequence # this packet */
    word    tcp_ack[2];        /* If ACK valid flag sequence number acked by */
                               /* this packet   */
    byte    tcp_hlen;          /* length of TCP header (includes options) */
                               /* length is in dwords in the high 4 bits   */
                               /* so (tcp_hlen&0xf0)>>2 gives len in bytes */

    /* Flag values   */
    #define NO_TCP_FLAGS  0x00     /* no flags are set */
    #define TCP_F_URGENT  0x20     /* urgent pointer field is valid */
    #define TCP_F_ACK     0x10     /* ack field is valid */
    #define TCP_F_TPUSH   0x08     /* this segment requests push */
    #define TCP_F_RESET   0x04     /* reset the connection */
    #define TCP_F_SYN     0x02     /* want to start up a sequence */
    #define TCP_F_FIN     0x01     /* sender has reached end of its byte stream */
                                   /* i.e. connection closed in input direction   */
    byte    tcp_flags;         /* Packet flags see preceeding lines */
    word    tcp_window;        /* receiver side window width */
    word    tcp_chk;           /* checksum */
    word    tcp_urgent;        /* Urgent pointer if flag set */

    /* options and padding (if any) are here   */
    /* Option types                            */
    #define TCP_EOL_OPTION       0
    #define TCP_NOP_OPTION       1
    #define TCP_MSS_OPTION       2
    #define TCP_TIMESTAMP_OPTION 8
} TCPPKT;
typedef TCPPKT KS_FAR * PTCPPKT;


/* ********                      */
/* icmp protocol frame structure */
#define ICMP_HLEN_BYTES 8
#define ICMP_TLEN_BYTES (ICMP_HLEN_BYTES+IP_HLEN_BYTES+LL_HLEN_BYTES)

typedef struct _icmp
{
    byte    icmp_type;      /* Type field See above */
    byte    icmp_code;      /* Code field See above */
    word    icmp_chk;       /* Check sum */
    byte    _4bytes[4];     /* Context dependent. (see source in icmp.c) */
} ICMPPKT;
typedef ICMPPKT KS_FAR * PICMPPKT;

#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
/* ********                      */
/* igmp protocol frame structure */
#define IGMP_HLEN       4
#define IGMP_HLEN_BYTES 8
#define IGMP_TLEN_BYTES (IGMP_HLEN_BYTES+IP_HLEN_BYTES+LL_HLEN_BYTES)

#endif

#if (INCLUDE_IGMP)
typedef struct _igmp
{
    byte    igmp_vertype;               /* 4 hi bits == version, lo 4 == type */
    byte    igmp_unused;
    word    igmp_chk;                               /* checksum */
    byte    igmp_ip_addr[IP_ALEN];  /* group address */
} IGMPPKT;

typedef IGMPPKT KS_FAR * PIGMPPKT;

/* IGMP types (lower 4 bits of igmp_vertype field of IGMP message)   */
#define IGMP_QUERY      1               /* host membership query */
#define IGMP_REPORT     2               /* host membership report */

#endif

#if (INCLUDE_IGMP_V2)
typedef struct _igmpv2
{
    byte    igmp_type;            /* just type, version not included anymore */
    byte    igmp_max_resp_time;       /* Max Response Time */
    word    igmp_chk;             /* checksum */
    byte    igmp_ip_addr[IP_ALEN];/* group address */
} IGMPPKT2;

typedef IGMPPKT2 KS_FAR * PIGMPPKT;

/* IGMP types   */
#define IGMP_QUERY    0x11      /*host membership query */
                                /* group addr = 0 is general query        */
                                /* group addr filled in is specific query */
                                /* for that group                         */
#define IGMP_REPORT2  0x16      /* Version 2 host membership report */
#define IGMP_LEAVE    0x17      /* Version 2 host leave group */
#define IGMP_REPORT1  0x12      /* Version 1 host membership report for */
                                /* backwards compatibility   */
                                /* with version 1            */

/* Various default values   */
#define IGMP_V1   0x01   /* used for type of querier on a given interface */
#define IGMP_V2   0x02

#define VERS1_ROUTER_PRESENT_TMO 400 /* how long in secs  a host must wait after */
                     /* hearing a V1 Query before may send V2 messages   */
#endif

#endif  /* INCLUDE_RTIP */


#if (INCLUDE_RTIP)
/* ****************************************************************************   */
/* ******  PACKET (DCU) structure. We use a message exchange as a free   *****    */
/* ******  list for this structure. The driver loads the packet and      *****    */
/* ******  places it on another exchange for use by the IP demuxer. This *****    */
/* ******  in turn either uses the packet (arp, icmp) or demuxes         *****    */
/* ******  it further to UPD or TCP                                      *****    */
/* ****************************************************************************   */

#if (INCLUDE_BOOTP)
typedef struct bootpkt KS_FAR * PBOOTPKT;
#endif


/* packet structure - structure used to contain packets and packet info for   */
/* both input and output packets                                              */
typedef struct _epacket
{
    OS_PACKET_CTRL  ctrl;   /* ptr to next epacket; epacket could be in free */
                            /* list (header=root_dcu) or an exchange or frag   */
                            /* chain (header=PEXCHANGE->packet_list)           */

#if (INCLUDE_PPP)
    word protocol;          /* protocol */
#endif

    int     root_dcu_index;     /* Which list in root_dcu_array[] is it from; */
                                /* used to know which list when freeing packet   */

    /* address of IP header   */
    union
    {
        PFBYTE    ptr;
        PARPPKT   arp;
#if (INCLUDE_BOOTP)
        PBOOTPKT  bootp;
#endif
        PIPPKT    ipv4;
    } inet;

    /* address of protocol header (i.e. TCP, UDP etc)   */
    union
    {
        PFBYTE    ptr;
        PICMPPKT  icmp;
#if (INCLUDE_IGMP)
        PIGMPPKT  igmp;
#endif
        PTCPPKT   tcp;
        PUDPPKT   udp;
    } prot;

    int    ip_route_option_len; /* length of the routing option, */
                            /* i.e. SOURCE_ROUTE, RECORD_ROUTE etc     */
                            /* NOTE: only one can be turned on at time */

    struct
    {
        byte ip_hlen;
        word ip_len;
    } ip_info;
#define IP_HLEN_DCU(MSG) (DCUTOPACKET(MSG)->ip_info.ip_hlen)
#define IP_LEN_DCU(MSG)  (DCUTOPACKET(MSG)->ip_info.ip_len)

#if (INCLUDE_TCP)
    /* WINDOW STUFF; chain of DCUs where head is stored in port structure   */
    /* and next pointer is stored in OS_PACKET_CTRL                         */
    int  data_start;        /* offset in data of start of data in TCP window */
    int  data_len;          /* length of data in this packet which is */
                            /* in TCP window (includes all frags)   */
    int  frag_data_len;     /* length of data in this fragment which is */
                            /* in TCP window   */
#endif

#if (INCLUDE_FRAG)
    word    fragoff_info;
    struct  _epacket KS_FAR *frag_next; /* next DCU in fragmented packet */
#endif

    int     ip_option_len;      /* length of IP options which will be */
                                /* written to output pkt              */
                                /* NOTE: the same value was           */
                                /*       stored in the PORT           */
                                /*       structure for output packets */

    int     tcp_option_len;     /* length of TCP options which will be */
                                /* written to output pkt              */
                                /* NOTE: the same value was           */
                                /*       stored in the PORT           */
                                /*       structure for output packets */
#if (INCLUDE_IGMP_V2)
    int     ip_options;         /* ip options associated per message */
#endif
#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
    int     nbytes;             /* size of data */
#endif
    /* Below here gets passed to the device driver   */
    int    length;              /* message length */
    PFBYTE data;                /* ethernet frame as seen on the wire */
} EPACKET;
typedef EPACKET KS_FAR * PPACKET;

#endif  /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ERRNO                                                                   */
/* *********************************************************************   */
#if (INCLUDE_ERRNO_STR)

typedef struct _errno_strings
{
    int     errno_val;
    PFCCHAR errno_string;       /* descriptive string for errno value */
} ERRNO_STRINGS;
#endif
#endif  /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* STATISTICS                                                              */
/* *********************************************************************   */
#if (INCLUDE_KEEP_STATS)
#define UPDATE_INFO(X,Y,Z)      X->stats.Y += Z;
#define UPDATE_SET_INFO(X,Y,Z)  X->stats.Y = Z;
#define INCR_INFO(X,Y)          X->stats.Y++;

#else
#define UPDATE_INFO(X,Y,Z)
#define UPDATE_SET_INFO(X,Y,Z)
#define INCR_INFO(X,Y)
#endif
#endif      /* INCLUDE_RTIP */


#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* SNMP                                                                    */
/* *********************************************************************   */
#if (INCLUDE_SNMP)
#define INCR_SNMP(X)                MIB_VAR(X) += 1;
#define DECR_SNMP(X)                MIB_VAR(X) -= 1;
#define RST_SNMP(X)                 MIB_VAR(X) = 0;
#define SET_SNMP(X, Y)              MIB_VAR(X) = Y;
#define SNMP_DEVICE_INFO(A, B)      A,B,

#else       /* INCLUDE_SNMP */
#define INCR_SNMP(X)
#define DECR_SNMP(X)
#define RST_SNMP(X)
#define SET_SNMP(X, Y)
#define SNMP_DEVICE_INFO(A, B)
#endif      /* INCLUDE_SNMP */

#if (INCLUDE_SNMP && INCLUDE_TRAPS)
typedef int (*SEND_TRAP_FNC)(int gn_trap_no, int sp_trap_no, POID enterprise_oid,
                         PAPI_REQ_DESC in_desc, PFBYTE buffer1,
                         int buf_len, int snmp_version);
/* macro for sending traps; lets SNMP alloc buffers to send traps,   */
/* therefore, assumes size of data need to send will fit in a DCU    */
#if (INCLUDE_SNMPV3)
#define SNMP_TRAP_VERSION SNMP_VERSION_V3
#elif (INCLUDE_SNMPV2)
#define SNMP_TRAP_VERSION SNMP_VERSION_V2C
#else
#define SNMP_TRAP_VERSION SNMP_VERSION_V1
#endif

#define SEND_SNMP_TRAP(w, x, y, z)  \
    if (snmp_trap_fnc)              \
        (*snmp_trap_fnc)(w, x, y, z, (PFBYTE)0, 0, SNMP_TRAP_VERSION)
#else
#define SEND_SNMP_TRAP(w, x, y, z)
#endif      /* INCLUDE_SNMP && INCLUDE_TRAPS */

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* MD5 - openSSL or RSA's MD5                                              */
/* *********************************************************************   */
#if (INCLUDE_SSL_MD5)
#define MD5_INIT(context)              MD5_Init(context)
#define MD5_UPDATE(context, sec, len)  MD5_Update(context, sec, len)
#define MD5_FINAL(key, context)        MD5_Final(key, context)
#elif (INCLUDE_RSA_MD5)
#define MD5_INIT(context)              MD5Init(context)
#define MD5_UPDATE(context, sec, len)  MD5Update(context, sec, len)
#define MD5_FINAL(key, context)        MD5Final(context);                   \
                                       tc_movebytes(key, (context)->digest, \
                                                    16)
#endif
#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******                                                   ************   */
/* ******     Protocol elements in network byte order       ************   */
/* ******                                                   ************   */
/* *********************************************************************   */
#if (KS_LITTLE_ENDIAN)          /* INTEL */
#define RARPR_68K   0x0400  /* RARP reply, from host - 0004 */
#define RARPQ_68K   0x0300  /* RARP request - 0003 */
#define ARPREP_68K  0x0200  /* reply - 0002 */
#define ARPREQ_68K  0x0100  /* request - 0001 */
#define ARPPRO_68K  0x0008  /* IP protocol - 0800 */
#define HTYPE_68K   0x0100  /* Ethernet hardware type - 0001 */
#define HTYPE_TOKEN_68K   0x0600  /* TOKEN RING hardware type - 0006 */

/* frame types in Ethernet frame   */
#define  EIP_68K    0x0008  /* IPV4 - 0800 */
#define  EIPV6_68K  0xdd86  /* IPV6 - 86dd */
#define  EARP_68K   0x0608  /* ARP  - 0806 */
#define  ERARP_68K  0x3580  /* RARP - 8035 */

/* corresponds to ip_fragoff in IPPKT   */
#define IP_MOREFRAG_68K 0x0020  /* & w/ip_fragoff for more frags flag */
#define IP_DONTFRAG_68K 0x0040  /* & w/ip_fragoff for don't frag flag */
#define IP_FRAGOFF_68K  0xff1f  /* & w/ip_fragoff for frag offset */
#define IP_HASFRAG_68K  0xff3f  /* & w/ip_fragoff to test if pkt is a fragment */

/* ppp over ethernet   */
#define  PPPOED_68K 0x6388  /* PPPOE Discovery - 8863 */
#define  PPPOES_68K 0x6488  /* PPPOE Session -   8864 */

/* Big Endian   */
#else                           /* 68000 */
#define RARPR_68K   0x0004      /* RARP reply, from host - 0004 */
#define RARPQ_68K   0x0003      /* RARP request - 0003 */
#define ARPREP_68K  0x0002      /* reply - 0002 */
#define ARPREQ_68K  0x0001      /* request - 0001 */
#define ARPPRO_68K  0x0800      /* IP protocol - 0800 */
#define HTYPE_68K   0x0001      /* Ethernet hardware type - 0001 */
#define HTYPE_TOKEN_68K   0x0006    /* Token Ring hardware type - 0006 */

/* frame types in Ethernet frame   */
#define  EIP_68K    0x0800      /* IPV4 - 0800 */
#define  EIPV6_68K  0x86dd      /* IPV6 - 86dd */
#define  EARP_68K   0x0806      /* ARP  - 0806 */
#define  ERARP_68K  0x8035      /* RARP - 8035 */

/* fragmentation flags - corresponds to ip_fragoff in IPPKT   */
#define IP_MOREFRAG_68K 0x2000  /* & w/ip_fragoff for more frags flag */
#define IP_DONTFRAG_68K 0x4000  /* & w/ip_fragoff for don't frag flag */
#define IP_FRAGOFF_68K  0x1fff  /* & w/ip_fragoff for frag offset */
#define IP_HASFRAG_68K  0x3fff  /* & w/ip_fragoff to test if pkt is a fragment */

/* ppp over ethernet   */
#define  PPPOED_68K 0x8863  /* PPPOE Discovery - 8863 */
#define  PPPOES_68K 0x8864  /* PPPOE Session -   8864 */


#endif

/* standard protocol types for IP - ip_proto field of IP packet   */
#define PROTUDP     17
#define PROTTCP     6
#define PROTICMP    1
#define PROTIGMP    2
/* ICMP TYPES - (icmp_type field)   */
#define ICMP_T_ECHO_REPLY       0
#define ICMP_T_DEST_UNREACH     3
#define ICMP_T_SOURCE_QUENCH    4
#define ICMP_T_REDIRECT         5
#define ICMP_T_ECHO_REQUEST     8
#define ICMP_T_TIME_EXCEEDED    11
#define ICMP_T_PARAM_PROBLEM    12
#define ICMP_T_TIME_REQUEST     13
#define ICMP_T_TIME_REPLY       14
#define ICMP_T_INFO_REQUEST     15
#define ICMP_T_INFO_REPLY       16
#define ICMP_T_MASK_REQUEST     17
#define ICMP_T_MASK_REPLY       18

/* Codes to DEST_UNREACH - (icmp_code field)   */
#define ICMP_C_NETUR            0       /* network unreachable */
#define ICMP_C_HOSTUR           1       /* host unreachable */
#define ICMP_C_PROTUR           2       /* protocol unreachable */
#define ICMP_C_PORTUR           3       /* port unreachable */
#define ICMP_C_FRAG             4       /* fragmentation needed and DF set */
#define ICMP_C_SRCROUTE         5       /* bad source route */
#define ICMP_C_NETUK            6       /* dest network unknown */
#define ICMP_C_HOSTUK           7       /* dest host unknown */
#define ICMP_C_HOSTISS          8       /* source host isolated */
#define ICMP_C_NETPROH          9       /* dest net administratively prohibited */
#define ICMP_C_HOSTPROH        10       /* dest host administratively prohibited */
#define ICMP_C_NETTOS          11       /* net unreachable for TOS */
#define ICMP_C_HOSTTOS         12       /* host unreachable for TOS */
#define ICMP_C_FILTERED        13       /* administratively prohibited by filterin */
#define ICMP_C_HOST_PRECEDENCE 14       /* host precendence violation */
#define ICMP_C_PRECEDENCE_CUTOFF 15     /* precendence cutoff in effect */

/* Codes to REDIRECT   */
#define ICMP_C_REDIR_NET        0
#define ICMP_C_REDIR_HOST       1
#define ICMP_C_REDIR_TOSN       2
#define ICMP_C_REDIR_TOSH       3

/* Codes to TIME_EXCEEDED   */
#define ICMP_C_TEXP_TTL         0       /* time exceeded */
#define ICMP_C_TEXP_FRAG        1       /* reassembly timeout */

/* Codes to PARAM_PROBLEM   */
#define ICMP_C_PTR_VALID        0

/* Codes to QUENCH   */
#define ICMP_C_SOURCE_QUENCH    0       /* source quench */

/* Pointer values to PARAM_PROBLEM   */
#define ICMP_P_IPCHKSUM        10       /* IP checksum error */
#define ICMP_P_TCPCHKSUM       36       /* TCP checksum error */
#define ICMP_P_TCPSEQ          24       /* TCP sequence number not in window */
#define ICMP_P_UDPCHKSUM       26       /* UDP checksum error */
#endif      /* INCLUDE_RTIP */


#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******            Routing Table                                ******   */
/* *********************************************************************   */
#if (INCLUDE_ROUTING_TABLE)
typedef struct _route KS_FAR * PROUTE;
typedef struct _route
{
#define ROUTE_OFFSET        0
    OS_LIST list;               /* Must be at the beginning of the ROUTE; */
                                /* pointer to next or previous ROUTE in   */
                                /* routing table; currently used for      */
                                /* list default gateways                  */
    byte    rt_dest[IP_ALEN];   /* network address; */
                                /* all 0's if default entry   */
    byte    rt_mask[IP_ALEN];   /* IP mask; */
                                /* (all f's for host entries,              */
                                /*  network and subnet or just network     */
                                /*  for non-host entries)                  */
                                /* determines whether entry is host or not */
    byte    rt_gw[IP_ALEN];     /* gateway address (if gateway entry); */
                                /* local IP address of outgoing interface (if   */
                                /*   not gateway entry-ignored anyway)          */
    dword   rt_metric;
    int     rt_iface;           /* interface number */
    byte    rt_masklen;         /* length of mask in bits (used for */
                                /*   determining best fit match in routing   */
                                /*   table)                                  */
#if (INCLUDE_RT_TTL)
    int     rt_ttl;             /* time to live (seconds); */
                                /* entry is invalid if ttl = 0   */
#endif
#if (INCLUDE_RT_LOCK)
    int     rt_usecnt;          /* use count; (cannot be deleted if not 0) */
                                /* TCP locks entry for life of connection   */
#endif
    int     rt_opencnt;         /* number of times current entry was */
                                /*   added (used to determine when   */
                                /*   entry can be deleted)           */

    #define RT_INUSE 0x01       /* entry in use */
    #define RT_DEL   0x02       /* entry needs to be deleted when no longer */
                                /* in use (it is invalid to new accesses)   */
    #define RT_GW    0x04       /* entry is a gateway (but not a multicast */
                                /* router   */
    #define RT_CHANGED 0x08
    word    rt_flags;           /* general routine table flags */
    int     rt_tag;             /* used by RIP */

#if (INCLUDE_SNMP)
    /* SNMP information   */
    int     snmp_proto;         /* who added this entry */
    dword   route_age;          /* sysUpTime when route last changed or */
                                /* determined to be correct             */
                                /* NOTE: currently no determination is  */
                                /*       made that the route is correct */
#endif

} ROUTE;

#if (INCLUDE_RT_TTL)
#define RT_FREE(X)           (X->rt_ttl <= 0)
#define RT_SET_FREE_INDEX(I) rt_table[i].rt_ttl = 0;
#define RT_SET_FREE(X)       X->rt_ttl = 0;
#else
#define RT_FREE(X)           (!(X->rt_flags & RT_INUSE))
#define RT_SET_FREE_INDEX(X) rt_table[i].rt_flags &= ~RT_INUSE;
#define RT_SET_FREE(X)       X->rt_flags &= ~RT_INUSE;
#endif

typedef struct rtinfo
{
    PROUTE  rt_default;     /* points to default entry (may be 0) */
} RTINFO;

#else       /* INCLUDE_ROUTING_TABLE */
#define rt_get(A)                   0
#define rt_add_from_iface(A)        0
#define rt_del_from_iface(A)        0
#define rt_del_iface(A)
#define rt_del(A, B)                0
#define rt_redirect(A, B, C, D)
#if (INCLUDE_RT_LOCK)
#define rt_free(A)
#endif
#if (INCLUDE_RT_TTL)
#define rt_timer()
#endif
#endif      /* INCLUDE_ROUTING_TABLE */

/* parameter to rt_add   */
#define SNMP_OTHER      1
#define SNMP_LOCAL      2
#define SNMP_NETMGMT    3
#define SNMP_ICMP       4


/* *********************************************************************   */
#if (INCLUDE_RIP)
#include "rip.h"
#endif /* RIP */

#if (INCLUDE_PCMCIA)
/*#ifndef BOOLEAN      */
/*typedef int BOOLEAN; */
/*#endif               */
#include "pcmcia.h"
#if (!INCLUDE_ERTFS)
#define rtfs_ticks_p_sec        ks_ticks_p_sec
#define rtfs_hook_interrupt     ks_hook_interrupt
#define rtfs_msec_p_tick        ks_msec_p_tick
#define rtfs_strcmp             tc_strcmp
#define rtfs_stricmp            tc_stricmp
#define rtfs_sleep              ks_sleep
#define rtfs_enable             ks_enable
#define rtfs_disable            ks_disable
#endif
#endif      /* INCLUDE_PCMCIA */

#if (INCLUDE_ERTFS_CONF)
#include "pcconf.h"
#endif

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******           Fragmentation                                 ******   */
/* *********************************************************************   */

#define IP_MOREFRAG 0x2000          /* & w/ip_fragoff for more frags flag */
#define IP_FRAGOFF  0x1fff

/* entry in fragment table   */
typedef struct _ip_fraglist
{
    dword   ipf_src;                /* IP src addr saved as dword for speed */
    word    ipf_id;
    int     ipf_ttl;
    byte    ipf_prot;
    struct  _epacket KS_FAR *ipf_next;  /* The first fragment in the chain; */
                                        /* if 0 then list is empty   */
} IP_FRAGLIST;
typedef IP_FRAGLIST KS_FAR * PIP_FRAGLIST;

#endif          /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******     PORT STRUCTURES (TCP and UDP/RAW) i.e. sockets API  ******   */
/* ******     calls use port structures in the same way that the  ******   */
/* ******     Unix sockets implementation uses socket numbers     ******   */
/* ******     Most calls take either UDP, RAW or TCP ports as     ******   */
/* ******     arguments. Thus we have created the _anyport        ******   */
/* ******     structure which includes fields common to both UDP, ******   */
/* ******     RAW and TCP port structures. These fields contain   ******   */
/* ******     the port type UDPPORTTYPE or TCPPORTTYPE that allow ******   */
/* ******     the api call to process the request correctly.      ******   */
/* *********************************************************************   */

/* list types for TCP and UDP (list_type entry in _anyport)   */
#define     FREE_LIST   0    /* not allocated (ctrl.list is head) */
#define     SOCKET_LIST 1    /* not active; i.e. socket done */
#define     ACTIVE_LIST 2    /* xn_connect done */
#define     LISTEN_LIST 3    /* xn_listen done, i.e. slave only (TCP only) */
#define     MASTER_LIST 4    /* xn_listen done, i.e. master with problem (TCP only) */

#define IS_TCP_PORT(PORT)   (PORT->ap.port_type == TCPPORTTYPE)
#define IS_UDP_PORT(PORT)   (PORT->ap.port_type == UDPPORTTYPE)

#define IS_TCP_PORT_TYPE(PORT)   (PORT->port_type == TCPPORTTYPE)
#define IS_UDP_PORT_TYPE(PORT)   (PORT->port_type == UDPPORTTYPE)

/* total number of lists used for TCP and UDP   */
#define     NUM_TCP_LISTS 6
#define     NUM_UDP_LISTS 3  /* FREE, SOCKET and ACTIVE Lists */

/* common elements of both TCP and UDP ports   */
typedef struct _anyport
{
    /* Note: The first 4 fields must be the same for both udp and tcp ports   */
    OS_PORT_CONTROL  ctrl;      /* OS specific object for port managment */
#if (INCLUDE_MALLOC_PORTS || INCLUDE_RUN_TIME_CONFIG)
    /* need to attach signals to port so can free them   */
    KS_RTIPSIG       portsig[NUM_SIG_PER_PORT];
#endif
    PIFACE           iface;     /* Iface structure the port is attached to */
                                /* i.e. iface send msgs out on; can come   */
                                /* in over any iface; based on ip addrs    */
                                /* connect to or receive first msg from    */
                                /* for listen (TCP only)                   */
    byte             send_ip_addr[IP_ALEN];
                                /* IP address to send packets to    */
                                /* either a gateway or same address */
                                /* as in outgoing packet (TCP only) */
#if (INCLUDE_SOCK_ARP_CACHE)
    int              cached_arp_index;
                                /* This is the index in the arp table   */
                                /* that matched the IP address of the   */
                                /* previous IP address we resolved      */
                                /* when transferring over this socket   */
                                /* This is true only if                 */
                                /* SOCK_ARP_CACHE_VALID in              */
                                /* port_cache_flags                     */

#endif

#if (INCLUDE_ROUTING_TABLE && INCLUDE_RT_LOCK)
    PROUTE           prt;       /* routing table entry to use for the */
                                /* connection (TCP only)   */
#endif

    /* For API calls that take UDP or TCP ports this identifies the socket   */
    #define UDPPORTTYPE 1
    #define TCPPORTTYPE 2
    #define RAWPORTTYPE 3
    byte             port_type;

    int              list_type;   /* which list port is on (active, */
                                  /* listen or none if free - see above)   */

#if (INCLUDE_PING)
    int              ping_sequence; /* sequence number in ping; used to */
                                    /* match reply with request   */
#endif

    /* options set by setsockopt and ioctlsocket    */
    /* (see SO_xxx, SOO_xxx and IO_xxx in socket.h) */
    long             options;

    struct timeval   send_timeval;      /* set by setsockopt */
    struct timeval   recv_timeval;      /* set by setsockopt */

#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
    /* (see SO_xxx in rtipapi.h)   */
    int              ip_options;        /* IP options set for the port */

    int              route_option_len;  /* length of route data in */
                                        /* one of the routing option   */
    PFBYTE           route_option_addresses;
                                        /* buffer of addresses to put in   */
                                        /* STRICT and LOOSE route options  */
#endif
    int              ip_option_len;     /* length of IP options which */
                                        /* will be written to output pkt   */
                                        /* NOTE: the same value will be    */
                                        /*       stored in the DCU when    */
                                        /*       it is allocated           */
    int              linger_secs;       /* seconds to linger before close; */
                                        /* set by setsockopt() when linger   */
                                        /* option is specified (TCP only)    */

    #define NO_FLAGS              0x0000
    #define PORT_BOUND            0x0001    /* TCP and UDP */
    #define PORT_WILD             0x0002    /* TCP and UDP */
    #define READ_SHUTDOWN         0x0004    /* read shutdown by shutdown */
    #define WRITE_SHUTDOWN        0x0008    /* write shutdown by shutdown */
    #define TCP_CONN_IN_PROG      0x0010    /* flags used for select when */
                                            /* TCP conn in progress   */
    #define TCP_CONN_EST          0x0020    /*   connecting on non-blocking */
                                            /*   socket   */
    #define RETRANS_EN            0x0040    /* retransmission enabled */
                                            /* (i.e. arp resolved)   */
    #define REPORT_RETRANS        0x0080    /* retransmission reported to app */
    #define API_CLOSE_DONE        0x0100    /* app has done xn_close; */
    #define WAIT_CLOSE_SEG        0x0200    /* wait to send fin until data */
                                            /* segmentized   */
    #define WAIT_GRACE_CLOSE      0x0400    /* waiting on graceful close */
                                            /* with timeout   */
    #define WINDOW_PROBE_STARTED  0x0800    /* window probe in progress */
    #define RETRANS_RUNNING       0x1000    /* retransmit timer running */
    #define EOF_READ_RETURNED     0x2000    /* EOF has been returned by read */
    #define GOT_TIMESTAMP_SYNC    0x4000    /* got timestamp option in sync=> */
                                            /* can send timestamp option   */
    #define GOT_TIMESTAMP         0x8000    /* got timestamp in current */
                                            /* packet   */
    word        port_flags;                 /* general flags used by TCP */
                                            /* and UDP   */


    #define LOCK_RT               0x0001    /* lock routing table when finding */
                                            /* a route   */
    word        anyport_flags;


    #define UDP_PORT_CACHEABLE    0x0001    /* If 1 then the port number */
                                            /* of this socket is unqiue for   */
                                            /* all udp packets.               */
    word        port_cache_flags;           /* Flags used to manage caching. */


    #define UDP_IFACE_CACHE_VALID 0x0002    /* If true then the values iface */
                                            /* and send_ip_addr[] cached in           */
                                            /* the port structure are the valid       */
                                            /* interface and ip address to send       */
                                            /* the packet to if the desination        */
                                            /* ip address matched last_sent_ip_addr[] */
                                            /* in the udpport structure               */
    #define SOCK_ARP_CACHE_VALID  0x0004    /* If 1 then the value for */
                                            /* cached_arp_index           */
                                            /* is valid and we should see */
                                            /* if we can bypass the arp   */
                                            /* lookup in tc_netwrite      */

                                            /*   */
    int         ip_ttl;                     /* time to live set by socket */
                                            /* option (-1 means not set    */
                                            /* by sockopt so use default); */
                                            /* legal values are:           */
                                            /*   -1     = not set up       */
                                            /*   0-0xff = IP ttl           */
    int         mcast_ip_ttl;               /* time to live set by socket option */
                                            /* for multicast packets   */
    byte        tos;                        /* Value for TOS in IP header */
                                            /* (set by setsockopt-SO_TOS)   */
#if (INCLUDE_NO_DCU_BLOCK)
    int         block_on_nbytes;            /* number of bytes that */
                                            /* port is blocked on      */
                                            /* waiting to be available */
                                            /* see ip_alloc_packet()   */
#endif
#if (INCLUDE_POSTMESSAGE)
    HWND       hwnd;
    MSGID      msgid;
#define ACCEPT_OR_CONNECT_CALLED 0x01
#define EWOULDBLOCK_WRITE        0x02
#define WAIT_IN_WIN_EMPTY        0x04
#define GRACEFUL_CLOSE           0x08
#define REMOTE_GRACEFUL_CLOSE    0x10
    int        post_flags;
#endif
} ANYPPORT;

/* ********   */
/* ********                   */
/* UDP AND RAW PORT STRUCTURE */
typedef struct _udpport
{
    ANYPPORT    ap;             /* Common to UDP and TCP */
    int         protocol;       /* protocol for RAW sockets only (analogous */
                                /* ip_proto in IP header)   */
    PIFACE      broadcast_pi;   /* if broadcast should go to specific interface */
#if (RTIP_VERSION < 26)
    ETHER       eth_connection; /* Contains ethernet address info for the port */
#endif
    IPPKT       ip_connection;  /* Contains port/ip address info for the port */
    UDPPKT      udp_connection; /* Contains port/ip address info for the port */
                                /* by providing a template for sends and a database   */
                                /* for demuxing incoming packets                      */
    int         max_udp_que;    /* maximum number of udp packet allowed */
                                /* in input queue; only valid if option   */
                                /* SO_MAX_UDP_QUE is on                   */
    int         no_udp_que;     /* number of packets queued */
#if (INCLUDE_UDP_IFACE_CACHE)
    byte        last_sent_ip_addr[IP_ALEN];
                                /* Last destination IP address we addressed         */
                                /* A packet to. In tc_udp_write. If                 */
                                /* (port->port_cache_flags & UDP_IFACE_CACHE_VALID) */
                                /* we compare port->last_sent_ip_addr               */
                                /* to the current ip address to send to.            */
                                /* If they match we do not have to look             */
                                /* to the routing table for the interface.          */
#endif

} UDPPORT;

/* ********           */
/* TCP PORT STRUCTURE */

/* States in the tranport control block state table.   */
#define TCP_S_CLOSED    1
#define TCP_S_LISTEN    2
#define TCP_S_TWAIT     3
#define TCP_S_SYNR      4
#define TCP_S_SYNS      5
#define TCP_S_EST       6
#define TCP_S_CWAIT     10
#define TCP_S_FW1       7
#define TCP_S_FW2       8
#define TCP_S_CLOSING   9
#define TCP_S_LAST      11
#define TCP_S_ALLOCED   12  /* Special state. Allocated, xn_socket but Connect */
                            /* or Accept were never called. The socket is not   */
                            /* on the interface socket list                     */
#define TCP_S_FREE      13  /* Special state. Not allocated. */

/* window buffers and connection management for input and output              */
/*  Tcp queuing and port structures are pretty similar to NCSA.               */
/*  TCP is messy & NCSA has evolved to be a good implementation of it         */
/*  so we follow their implementation closely where it is practical to do so. */
/*   TCP queuing                                                              */
/*   data types for all TCP queuing operations                                */
/*   Each open port will have one of these structures assigned to it.         */
typedef struct window
{
    dword  nxt;                /* sequence number, not byte-swapped */
                               /*   in = nxt seq # expect (i.e. value to ack)   */
                               /*   out = seq # of first byte in output window  */
                               /*   NOTE: for out not incremented when sent but */
                               /*         it is incremented when acked          */
    dword  nxt_to_send;        /* sequence number, not byte-swapped */
                               /*   in = if write is shutdown and FIN has not   */
                               /*        been received, set to ack              */
                               /*        value to send to otherside             */
                               /*   out = seq # of first byte which has not     */
                               /*         been sent                             */
    dword  max_nxt_to_send;    /* largest nxt_to_send */
                               /*   out = largest seq # of first byte which       */
                               /*         has not been sent; need to track incase */
                               /*         graceful close with timeout is done     */
                               /*         and retransmit is in prog; i.e. to      */
                               /*         avoid sending FIN with seq # which      */
                               /*         is in the middle of data already sent   */
                               /*         i.e. this is used to determine          */
                               /*         seq no of FIN bit                       */
    dword  ack;                /* what the other machine acked, ie first seq # */
                               /*   not acked   */
    /* WINDOW STUFF                             */
    POS_LIST dcu_start;        /* first DCU in window */
    DCU    nxt_to_send_dcu;    /* output only */
    int    nxt_to_send_off;    /* output only */

    byte   push;               /* flag for TCP push */
    word   window_size;        /* our total window size */
                               /*   in = total input window size   */
                               /*   out = total output window size */
    word   size;               /* size of window available */
                               /*   in = actual room left in input window         */
                               /*   out = actual room in other sides input window */
    word   ad_size;            /*   in = room advertised in input window */
                               /*   out = room advertised in othersides input window   */
    word   max_size;           /*   in  = not used */
                               /*   out = largest input window size of otherside   */
    word   mss;                /* MTU value for this connection */
                               /*   in = mss to send in sync msg (max to recv)   */
                               /*   out = mss received (max to send)             */
    word   port;               /* port numbers from one host or another */
                               /*   in = local port bound to, set to random #     */
                               /*        by xn_socket(), can be overwritten by    */
                               /*        xn_bind()                                */
                               /*   out = port connected to (thru xn_connect() or */
                               /*         receive sync when in LISTEN state);     */
                               /*         0 means accept any port #               */
    word   contain;            /* how many bytes in queue? */
} WINDOW;
typedef WINDOW KS_FAR * PWINDOW;


typedef struct _tcpport
{
    ANYPPORT    ap;             /* Common to UDP and TCP */

    #define  INVALID_PORT_TYPE   -4
    #define  NOT_SLAVE_OR_MASTER -3 /* actively opened port (xn_connect) */
    #define  SLAVE_PORT          -2 /* passive opened slave port (xn_listen) */
    #define  MASTER_NO_LISTENS   -1 /* master port with no slaves in LISTEN */
                                    /* state                               */
                                    /* 0..TCP_MAX_CONNECTIONS for master   */
                                    /*   ports (xn_listen) with at least 1 */
                                    /*   in LISTEN state                   */
    int         tcp_port_type;      /* type of port; for master port */
                                    /* >= -1  means number of connections   */
                                    /* which can go in listen               */
                                    /* state if needed                      */

    PTCPPORT    next_slave;     /* next slave port in linked list of slave */
                                /* ports connected to a master port   */
    PTCPPORT    master_port;    /* points to master port */
                                /* (used for SLAVE only)   */
    ETHER       out_eth_temp;   /* Template used for sending packets */
                                /* Preinitialized as much as useful in   */
                                /* tc_tcp_socket().                      */
    IPPKT       out_ip_temp;    /* Template used for sending packets */
                                /* Preinitialized as much as useful in   */
                                /* tc_tcp_socket().                      */
    TCPPKT      out_template;   /* Template used for sending packets */
                                /* Preinitialized as much as useful in   */
                                /* tc_tcp_socket().                      */

    /* NOTE: to conserve signals, listen, connect and read use same signal   */
    /*       read and write need to be different signals                     */
    /* NOTE: the following flag values are passes to signal_api_error also   */
    #define NO_SIGNAL_FLAGS       0x0000
    #define READ_WAKEUP_SUCCESS   0x0001
    #define READ_WAKEUP_FAIL      0x0002
    #define WRITE_WAKEUP_SUCCESS  0x0004
    #define WRITE_WAKEUP_FAIL     0x0008    /* signal write and sent signals */
    #define SELECT_WRITE_WAKEUP   0x0010    /* signal select signal (when set */
                                            /* write signal - when set read   */
                                            /* signal, alway signals select   */
    #define SEND_PKT              0x0020
    #define SEND_DATA_PKT         0x0040
    #define SEND_SYN              0x0080
    #define SEND_FIN              0x0100
    #define SEND_ACK              0x0200
    #define DELAYED_ACK           0x0400    /* delay the ack */
    #define DELAYED_LAST          0x0800    /* last pkt, ack was delayed on */
                                            /* size pkt   */
    #define SEND_ONE_SEG_DATA     0x1000
    int         interp_port_flags; /* general flags used by state machine etc */

    /* SELECT behavior override   */
    word        select_size;    /* if this is set to anything other than 0, */
                                /* select will wake up on write when   */
                                /* there are this many free bytes      */

    /* WINDOWS   */
    WINDOW      in;             /* Buffer & connection mgmnt incoming */
    WINDOW      out;            /* Buffer & connection mgmnt outgoing */

    dword       last_ack;       /* the largest ack value we have received; */
                                /* used to not update window size if    */
                                /* the packet is older than last packet */
                                /* processed                            */
#if (NAGLE_STEVENS)
    dword       small_unacked;  /* sequence number of small segment */
                                /* which needs to be acked before   */
                                /* another small segment can be     */
                                /* sent (if NAGLE is on)            */
#endif

#if (INCLUDE_TCP_OUT_OF_ORDER)
    POS_LIST    out_order_dcu;  /* head of out of order input packets; */
                                /* saved in sequence order   */
#endif

    int         state;          /* connection state */
    int         from_state;     /* previous state (used for SYNR state only) */
    int         closetime;      /* time since entered TWAIT or LAST state; */
                                /* used to close port when timeout;         */
                                /* can't use sincetime since while in TWAIT */
                                /* and LAST will retrans, therefore, will   */
                                /* reset sincetime                          */
                                /* units = sec                              */
    /* timers                                                               */
    long        lasttime;       /* used for timeout checking */
                                /*   out = time simce last retry msg sent   */
                                /*         units = msec                     */
    long        sincetime;      /* used for timeout checking */
                                /* time since origional msg sent   */
                                /*         units = msec            */
    dword       intime;         /* time since last msg rcvd */
                                /*         units = sec           */
                                /* DHCP specifies this is 32-bit */
                                /* unsigned                      */

    /* values used to determine keepalive   */
    dword       ka_interval;    /* _CFG_KA_INTERVAL */
    int         ka_retry;       /* _CFG_KA_RETRY */
    int         ka_tmo;         /* _CFG_KA_TMO */

    /* keepalive timer   */
    int         ka_sincetime;   /* used for keepalive timeout checking; */
                                /* time last keepalive pkt sent   */
                                /* units = sec                    */
    RTIP_BOOLEAN ka_started;    /* used for keepalive timeout checking; */
                                /* set if in middle of keepalive   */
    int         wp_sincetime;   /* time since last window probe */
                                /* units = sec   */
    int         wp_sendtime;    /* interval to send window probe (ranges from */
                                /* WINDOW_PROBE_MIN to WINDOW_PROBE_MAX   */
                                /* units = sec                            */

    word        credit;         /* choked-down window for fast hosts */
    long        rto;            /* retrans timeout (units : msecs) */
                                /* limited see MINRTO and MAXRTO) in xnconf.h   */
#if (INCLUDE_TCP_RTT)
    long        smooth_rtt;     /* smoothed RTT */
    long        smooth_mean_dev; /* smoothed mean deviation */
    int         rtttime;        /* used to measure round trip time */
                                /*   i.e. time since origional msg sent   */
                                /*   units = msec                         */
    dword       rtt_nxt;        /* seq number measuring RTT for */
#endif
    int         dactime;        /* timer used to measure delayed acks */
                                /*   units = msec   */
#if (INCLUDE_CWND_SLOW)
    word        cwnd;           /* congestion window for output */
    word        ssthresh;       /* slow start threshold size */
#endif
#if (INCLUDE_FAST_RETRANS)
    dword       fr_nxt;         /* seq number waiting for during */
                                /* fast retransmit   */
    word        dup_ack_cnt;    /* number of dup acks in a row */
#endif
#if (INCLUDE_TCP_TIMESTAMP)
    dword       tsvalue;        /* time stamp option value from input packet; */
                                /* saved in network byte order   */
#endif
    int         tcp_option_len; /* length of output options */
#if (INCLUDE_SSL)               /* __st__ 2002.05.20 */
    /*  Don't need to declare the structure
     *  of the following pointer, as it
     *  will be allocated by the appropriate
     *  SSL functions
     */
    void        *ssl;           /* must be cast as SSL*     */

    /* Should point to the corresponding SSL_CTX
     * contained in the protocols main context because
     * this information should only be set once, not
     * every time a socket is created.
     * NOTE: The fields within the SSL_CTX to which
     *       this points should not need to be
     *       modified at this layer at any point.
     *       This is merely an accessor.
     */
    void        *sctx;          /* must be cast as SSL_CTX* */
#endif
} TCPPORT;


/* enable TCP retransmit   */
#define TCP_SETUP_RETRANS(port)   \
    port->ap.port_flags |= RETRANS_EN;
#define TCP_STOP_RETRANS(port)   \
    port->ap.port_flags &= ~RETRANS_RUNNING;

/* ********************************************************************   */
/* checks if X greater then Y taking into consideration wrapping of       */
/* sequence numbers                                                       */
/* NOTE: All of this casting eliminates the segment wrap problem.         */
/*       If ak is a small number and nxt is very large and they differ by */
/*       less then 0xfffffff/2. Then ak will be > nxt by this calculation */
/*       which is what we want. (see Comer Vol II page 171)               */
#define SUB_SEGMENT(X, Y)    \
           (long)( (long)(X) - (long)(Y) )
#define CHECK_GREATER(X, Y)    \
           ( ((long)((long)(X) - (long)(Y))) > 0L )
#define CHECK_GREATER_EQUAL(X, Y)    \
           ( ((long)((long)(X) - (long)(Y))) >= 0L )

/* *********************************************************************   */
/* ******   ARP CACHE entry.                                      ******   */
/* ******   Each interface structure contains a table of          ******   */
/* ******   these entries. We search the table to map IP          ******   */
/* ******   addresses to ethernet addresses. If the IP address    ******   */
/* ******   has not been determined we queue the packet for later ******   */
/* ******   sending and send an ARP request. When a reply comes   ******   */
/* ******   comes from an ARP server we transmit all queued       ******   */
/* ******   packets.                                              ******   */
/* *********************************************************************   */

typedef struct _arpcache
{
    OS_ARPC_CONTROL ctrl;
    byte arpc_hw_addr[ETH_ALEN]; /* hardware address for this IP address */
    byte arpc_ip_addr[IP_ALEN];  /* the IP # in question */
    #define ARPC_STATE_FREE     0   /* Arp state definitions (do not change */
                                    /* since table initialized to all 0s)   */
    #define ARPC_STATE_PENDING  1
    #define ARPC_STATE_RESOLVED 2
    byte arpc_state;

    #define ARPC_REQ_TIMEOUT    1   /* Requests are retried every second */
    #define ARPC_RES_TIMEOUT    600 /* Resolutions are stale after 10 minutes */
                                    /* i.e. 600 secs                     */
                                    /* These values assume gran is 1 sec */
                                    /* (see tc_arp_or_gw_to())           */
                                    /* This value is stored in global    */
                                    /* variable arpc_res_timeout which   */
                                    /* might be changed by DHCP          */
    #define ARP_TTL_INF 0xfffffffful /* timeout value which means never */
                                     /* expire   */
    dword arpc_ttl;                 /* time-to-live (in seconds) of the */
                                    /* arp table entry             */
                                    /* NOTE: DHCP specifies ttl as */
                                    /*       32-bit unsigned       */
    #define ARPC_MAX_RETRIES    4   /* Maximum of four retries */
    int arpc_nretries;          /* number of times to retry arp */
    PIFACE pi;                  /* Interface sent arp request out on */
                                /* Saved for retries for unresolved entries   */
                                /* and for SNMP for resolved entries          */
} ARPCACHE;
typedef ARPCACHE KS_FAR * PARPCACHE;

#if (INCLUDE_ETH_BUT_NOARP)
/* *********************************************************************   */
/* IP TO ETHERNET TRANSLATION TABLE ENTRY                                  */
/* *********************************************************************   */
/* IP to ethernet translation table; used for ethernet packets where       */
/* translation is known by application, therefore, ARP is not              */
/* necessary                                                               */
typedef struct ip2eth
{
    byte hw_addr[ETH_ALEN]; /* hardware address for this IP address */
    byte ip_addr[IP_ALEN];  /* the IP # in question */
} IP2ETH;
typedef IP2ETH KS_FAR * PIP2ETH;

#endif /* INCLUDE_ETH_BUT_NOARP */

/* *********************************************************************   */
/* RARP TABLE                                                              */
/* *********************************************************************   */

#if (INCLUDE_RARP_SRV)

#define RT_TABLE_SIZE 3

struct rarp_table
{
    byte ip_addr[IP_ALEN];
    byte ether_addr[ETH_ALEN];
};
#endif

/*****************************************************************   */
/* VIRTUAL FILE LAYER                                                */
/*****************************************************************   */

/* virtual file table entry   */
typedef struct virtual_file_entry
{
    KS_CONSTANT char * name;  /* file name (URL name for WEB for example) */
    PFBYTE data;        /* pointer to web page data in memory */
    long   curr_size;   /* current size of file (will change if write */
                        /* file via ftp, tftp or WEB PUT)   */
} virtual_file_entry;

typedef struct virtual_file_entry KS_FAR *PVFILEE;


/* end of table marker for virtual file table; name field in last   */
/* entry is set to END_OF TABLE                                     */
#define END_OF_TABLE 0

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_MALLOC_CONTEXT || INCLUDE_MALLOC_PORTS)
/* ********************************************************************   */
/* MALLOC FREE DATA WHICH HAS LIMIT ON NUMBER WHICH CAN BE MALLOCed       */
/* ********************************************************************   */
#define MALLOC_WITH_LIMIT(ctx, size, limit, who)            \
    ctx = 0;                                                \
    if ( (cfg_limits_data.limit == -1) ||                   \
         (num_alloced_data.limit <                          \
          cfg_limits_data.limit) )                          \
    {                                                       \
        if (cfg_limits_data.limit != -1)                    \
            num_alloced_data.limit++;                       \
        ctx = (PFBYTE)ks_malloc(1, size, who);              \
    }

#define FREE_WITH_LIMIT(ctx, size, limit)                   \
    ks_free((PFBYTE)ctx, 1, size);                          \
    if (cfg_limits_data.limit != -1)                        \
    {                                                       \
        num_alloced_data.limit--;                           \
    }
#endif

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******    PACKET GUARD                                         ******   */
/* *********************************************************************   */
#if (INCLUDE_GUARD_PACKET)
#define PKT_GUARD_SIZE 16
#else
#define PKT_GUARD_SIZE 0    /* DO NOT CHANGE */
#endif

/* *********************************************************************   */
/* ******    MALLOC GUARD                                         ******   */
/* *********************************************************************   */
#define BGET_OVERHEAD 16

#if (INCLUDE_GUARD_MALLOC)
#define GUARD_MALLOC_SIZE 16        /* number of guard bytes */
#else
#define GUARD_MALLOC_SIZE 0         /* DO NOT CHANGE */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if  (INCLUDE_SMB_SRV || (INCLUDE_SSL && (INCLUDE_TEST_CRYPT || TEST_SSL_ALL || TEST_SSL_ONE_ONLY)))
#define xn_malloc(A, B)             ks_malloc(A, 1, B)
#define xn_realloc(ptr, len, id)    realloc(ptr, len)
#define xn_free(A)                  ks_free(A, 0, 0)
#endif

#ifdef __cplusplus
}
#endif

/*****************************************************************   */
/* TERMINAL STUFF                                                    */
/* NOTE: this is default configuration; YOU MIGHT WANT TO CHANGE     */
/*       USE_PCVID_OUTPUT, USE_PCVID_INPUT or USE_KEYSCAN            */
/*****************************************************************   */
#if (AT_MOTHERBOARD)
#if ((CONSOLE_DEVICE==CONDEV_PCVIDEO) || INCLUDE_VT100C)
#define USE_PCVID_OUTPUT 1        /* ebs screen IO */
#else
#define USE_PCVID_OUTPUT 0
#endif

#else
#define USE_PCVID_OUTPUT 0   /* always off if not AT_MOTHERBOARD */
#endif

#if (AT_MOTHERBOARD)
#if ((CONSOLE_DEVICE==CONDEV_PCVIDEO) || (CONSOLE_DEVICE==CONDEV_VT100C) || (CONSOLE_DEVICE==CONDEV_SMX))
#define USE_PCVID_INPUT 1       /* EBS keyboard IO */
#else
#define USE_PCVID_INPUT 0
#endif
#else
#define USE_PCVID_INPUT 0   /* always off for AT_MOTHERBOARD */
#endif

#if (AT_MOTHERBOARD && USE_PCVID_INPUT)
#define USE_KEYSCAN      1
#else
#define USE_KEYSCAN      0
#endif

/* Macro to tell if we are using a 16550 device   */
#define IS_16550_DEVICE (CONSOLE_DEVICE==CONDEV_16550_TX || \
                        CONSOLE_DEVICE==CONDEV_16550_1   || \
                        CONSOLE_DEVICE==CONDEV_16550_2   || \
                        CONSOLE_DEVICE==CONDEV_16550_3   || \
                        CONSOLE_DEVICE==CONDEV_16550_4)

#if (INCLUDE_MOUSE)
#define MOUSE_LEFT_BUTTON_DOWN  0x02
#define MOUSE_RIGHT_BUTTON_DOWN 0x01
#endif

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******           INCLUDE Various header files            ************   */
/* *********************************************************************   */

#include "netutil.h"

#if (INCLUDE_WEB)
#include "webapi.h"
#include "web.h"
#endif

#include "inlines.h"     /* in-line functions */
#include "bget.h"        /* needed by xn_malloc et al. If !INCLUDE_BGET, bget() et al will map to malloc() */
#if (INCLUDE_SNMP)
#include "snmp.h"
#endif

#if (INCLUDE_PPP)
#include "ppp.h"
#endif

#if (INCLUDE_CSLIP)
#include "slhc.h"
#endif

#if (INCLUDE_ERTFS_API)
#ifndef BOOLEAN
typedef int  BOOLEAN;           /* Don't change */
#endif
#include "rtfsapi.h"
#endif

#include "debugapi.h"
#include "debug.h"

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* ***********************************************************************   */
/* ******    Device table structure.                                 *****   */
/* ***********************************************************************   */

#if (IX86)
/* IRQs   */
#define IRQ1   1
#define IRQ2   2
#define IRQ3   3
#define IRQ4   4
#define IRQ5   5
#define IRQ6   6
#define IRQ7   7
#define IRQ8   8
#define IRQ9   IRQ2
#define IRQ10 10
#define IRQ11 11
#define IRQ12 12
#define IRQ13 13
#define IRQ14 14
#define IRQ15 15
#endif

/* null pointer to a function; used for end of table marker etc   */
#define NULLP_FUNC 0

#define EN(X) {X}
#define IOADD(X) {(IOADDRESS)(X)}

/* default comm port   */
#define DFLT_PORT   1

typedef RTIP_BOOLEAN (*DEVICE_OPEN)(struct _iface KS_FAR *pi);
typedef void    (*DEVICE_CLOSE)(struct _iface KS_FAR *pi);
typedef int     (*DEVICE_XMIT)(struct _iface KS_FAR *pi, DCU msg);
typedef RTIP_BOOLEAN (*DEVICE_XMIT_DONE)(struct _iface KS_FAR *pi, DCU msg, RTIP_BOOLEAN success);
typedef void    (*DEVICE_PROCESS_INTERRUPTS)(struct _iface KS_FAR *pi);
typedef RTIP_BOOLEAN (*DEVICE_STATS)(struct _iface KS_FAR *pi);
typedef RTIP_BOOLEAN (*DEVICE_SETMCAST)(struct _iface KS_FAR *pi);

typedef struct _edevtable
{
    /* Hardware level access data and routines   */
    DEVICE_OPEN open;
    DEVICE_CLOSE close;
    DEVICE_XMIT xmit;
    DEVICE_XMIT_DONE xmit_done;
    DEVICE_PROCESS_INTERRUPTS proc_interrupts;
    DEVICE_STATS statistics;
    DEVICE_SETMCAST setmcast;

    int     device_id;   /* device type - same as device type in iface struct */

    #define MAX_DEVICE_NAME_LEN 22
    char device_name[MAX_DEVICE_NAME_LEN];

    int minor_number;           /* minor number - if one device of a type */
                                /* it is 0, if two devices, the devices   */
                                /* are 0 and 1, etc.                      */

    int     iface_type;         /* interface type (ETHER_IFACE for example) */

#if (INCLUDE_SNMP)
    struct oid media_mib;           /* mib definitions specific to the interface */
    dword      speed;               /* bandwidth in bits/sec */
                                    /* NOTE: for RS232 baud rate is used   */
#endif

    word        mtu;            /* MAX IP packet size; used for the following: */
                                /* - to determine mss sent in TCP sync msg   */
                                /*   if on the same network, i.e. max amount */
                                /*   of data which can be received in one    */
                                /*   packet; if not on the same network      */
                                /*   send 536 for our mss)                   */
                                /* - if need to fragment and size of         */
                                /*   fragments                               */
                                /* MTU = data + IP header + TCP header       */
    word        max_mss_out;    /* MAX TCP data size to send regardless */
                                /* of mss received in TCP sync message   */
                                /* MSS = data                            */

    /* TCP WINDOW SIZES                                                     */
    /* They should be relative to the MTU size, i.e. for a MTU of 1500, the */
    /* MSS would be 1460, therefore with window size of 5840, 4 full-sized  */
    /* packets would fit in the window                                      */
    word        window_size_in; /* size of TCP input window */
    word        window_size_out;/* size of TCP outut window */

    /* defaults   */
    union
    {
        IOADDRESS  io_address;
        int        comm_port;
    } default1;

    union
    {
        word mem_address;

        /* cable type used by 3c590 device driver;                */
        /* NOTE: there is no way to override the value set in the */
        /*       device table;                                    */
        #define TENBASET    1
        #define THINNET     2
        #define THICKNET    3
        word cable_type;

        word baud_rate;
    } default2;

    union
    {
        int irq;
        char handshake_type;
    } default3;
} EDEVTABLE;

typedef struct _edevtable KS_FAR * PDEVTABLE;
#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)

/* *********************************************************************   */
/* ******  INTERFACE STRUCTURE - Each interface on the system     ******   */
/* ******  (there will typically only  be one). Is controlled by  ******   */
/* ******  one of these structures.                               ******   */
/* *********************************************************************   */
#if (INCLUDE_DHCP_CLI)
#if (INCLUDE_DHCP_RENEW_NO_BLOCK)
#include "dhcpcapi.h"
#include "dhcp.h"
#endif
#endif

/* multicast information   */
typedef struct _mcast_info
{
    int     lenmclist;                  /* number of address */
#if (INCLUDE_RUN_TIME_CONFIG)
    byte *  mclist_ip;
    byte *  mclist;
    int *   mcast_cnt;                  /* number of tasks set up */
                                        /* each address   */
#else
    byte    mclist_ip[CFG_MCLISTSIZE][IP_ALEN];
    byte    mclist[CFG_MCLISTSIZE * ETH_ALEN];
    int     mcast_cnt[CFG_MCLISTSIZE];  /* number of tasks set up */
                                        /* each address   */
#endif

#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
#if (INCLUDE_RUN_TIME_CONFIG)
    dword * report_timer;                /* timer to delay sending */
                                         /* reports after receiving   */
#else
    dword   report_timer[CFG_MCLISTSIZE]; /* timer to delay sending */
                                          /* reports after receiving   */
#endif
#endif      /* INCLUDE_IGMP || INCLUDE_IGMP_V2 */

#if (INCLUDE_IGMP_V2)
#if (INCLUDE_RUN_TIME_CONFIG)
    byte *  last_host_toreply;                  /* True if this host was the last */
                                                /* to reply to a query   */
#else
    byte    last_host_toreply[CFG_MCLISTSIZE];  /* True if this host was the last */
                                                /* to reply to a query   */
#endif
    byte    mcast_querier;                      /* state variable determines */
                                                /* if querier                         */
                                                /* for the interface is IGMP V1 or V2 */
    int     router1_present_tmr;                /* counter is positive if */
                                                /* querier is V1   */
#endif      /* INCLUDE_IGMP_V2 */

} MCAST_INFO;

/* ********                        */
/* IP/ETHERNET address information */
typedef struct _iface_ip
{
    byte        my_hw_addr[ETH_ALEN];       /* Ethernet address on the board */
    byte        my_ip_addr[IP_ALEN];        /* My IP address (192.42.55.1 for example) */
    byte        my_ip_mask[IP_ALEN];        /* subnet mask ff.ff.ff.00 for example */
    byte        my_net_bc_ip_addr[IP_ALEN]; /* My IP net-directed broadcast */
                                            /* address   */
    byte        his_ip_addr[IP_ALEN];       /* remote IP address for dedicated lines */
                                            /* (SLIP and PPP)   */
#if (INCLUDE_BOOTP)
    byte        bootp_ip_addr[IP_ALEN];     /* BOOTP IP address */
    char        bootp_file_name[BOOTP_FILE_LEN];
#endif

    #define   IP_ADDR_VALID 0x01            /* xn_set_ip done */
    #define   DHCP_CLI_DDNS 0x02            /* DHCP did DOMAIN_NAME option */
    byte        iface_flags;                /* ip addr and ip mask valid flags */
    word        mtu;                        /* MAX IP packet size from device */
                                            /* table (may be overwritten by   */
                                            /* PPP negotiation)               */
    word        remote_mtu;                 /* remote MTU negotiated */
                                            /* valid only for PPP   */
    word        max_mss_out;                /* MAX TCP data size (regardless */
                                            /* of sync message); set from   */
                                            /* value in device table        */
    dword       metric;
#if (INCLUDE_DHCP_CLI)
    dword       orig_lease_time;            /* amount of lease time given by */
                                            /* server   */
    dword       lease_time;                 /* amount of lease time left */
    dword       renew_lease_time;           /* time left before renew */
                                            /* (typically .5 x orig)   */
    dword       rebind_lease_time;          /* time left before rebind */
                                            /* (typically .875 x orig)   */
    byte        dhcp_server_ip_addr[IP_ALEN];   /* IP address of server; */
                                                /* set by xn_dhcp;        */
                                                /* used for sending       */
                                                /* extend (renew) request */
#if (INCLUDE_DHCP_RENEW_NO_BLOCK)
    /* extend lease from timer task information                               */
    /* NOTE: need state machine to avoid blocking or sleeping from timer task */
    #define DHCP_EXTEND_IDLE   0            /* extend is not in progress */
                                            /* NOTE: this must be 0 since the   */
                                            /*       addr structure is zeroed   */
                                            /*       at interface open          */
    #define DHCP_EXTEND_INPROG 1            /* extend is in progress */
    int          dhcp_extend_status;        /* status of extend lease */
    DHCP_session dhcp_session;              /* session used to extend lease */
#endif

#endif

} IFACE_IP;

/* ********   */
#if (INCLUDE_PPP)
#ifndef __PPP_TYPES
#define __PPP_TYPES
    typedef struct ppp_s KS_FAR * PPPPS;
#endif
#endif

typedef struct _iface
{
    OS_IFACE_CONTROL ctrl;              /* Iface Output and ARP Output queues */

    PDEVTABLE   pdev;                   /* IO routines and device type */
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS)
    int         irq_no;                 /* IRQ number to use when task calls */
                                        /* rtip_handler[irq](rtip_args[irq])   */
#endif
    int         xmit_que_depth;         /* depth of driver's output que; */
                                        /* default is 1; can be set higher   */
                                        /* by device open                    */
    int         xmit_done_counter;      /* count of number of signals */
                                        /* which are due to send complete   */
#if (INCLUDE_XMIT_QUE)
    int         xmit_started;           /* count of number xmits in progress */
                                        /* which are due to send complete   */
#endif
#if (!INCLUDE_XMIT_QUE)
    DCU         xmit_dcu;               /* DCU which was previously sent */
#endif
    int         xmit_done_timer;        /* timer used by timer task to */
                                        /* detect send timeouts   */
    int         xmit_status;            /* used to pass xmit status (errno) */
                                        /* to IP layer   */
    int         open_count;             /* if 0, not in use */

    #define CLOSE_IN_PROGRESS 0x01      /* IP task will close interface */
                                        /* (cb_rs232_connection_lost)   */
    #define IS_802_2          0x02      /* send 802.2 packets on this interface */
    int         iface_flags;

    int         minor_number;           /* Offset from first entry of same type */
                                        /*   for the device (SLIP and PPP   */
                                        /*   both use RS232)                */
    /* the following 3 values take precedence over values from device       */
    /* table                                                                */
    IOADDRESS   io_address;             /* io_address (if not 0) */
    int         irq_val;                /* IRQ (if not -1) */
#if (INCLUDE_SMC8XXX || INCLUDE_3C503 || INCLUDE_CS89X0 || INCLUDE_LAN89X0)
    word        mem_address;            /* memory address (if not 0) */
#endif

    int         max_output_que;         /* maximum number of packets which */
                                        /*   can be queued on output list   */
                                        /*   where -1 = no limit (default)  */
    int         no_output_que;          /* number of packets queued on */
                                        /*   output list   */

    IFACE_IP    addr;                   /* IP/ETHERNET address information */

#    if (INCLUDE_PPP)
#    if (INCLUDE_PPPOE)
        PIFACE   pppoe_ether_pi;        /* pointer from PPPOE interface */
                                        /* to ethernet interface   */
        int      pppoe_session_handle;
#    endif
        PPPPS   edv;                    /* PPP control block */
#    endif

#    if (INCLUDE_KEEP_STATS)
        IFACE_STATS stats;              /* Statistics accumulated during */
                                        /* operation   */
#    endif

    union
    {
        PETHER_STATS ether_stats;           /* ethernet specific statistics */
        PRS232_STATS rs232_stats;           /* rs232 specific statistics */
    } driver_stats;

    /* When the user calls ed_set_multicast() we put the addresses here
       and the size of the array.
       The array is broken up into six byte chunks, each representing
       an address. The length of the array is in bytes not addresses. */
    MCAST_INFO mcast;

#    if (INCLUDE_SNMP)
        dword   lastchange;             /* sysUpTime when interface opened */
                                        /* or closed   */
#    endif
} IFACE;

/* convert offset to interface structure   */
#define PI_FROM_OFF(pi, iface_no)                       \
    if ((iface_no) < 0 || (iface_no) >= CFG_NIFACES)    \
        pi = (PIFACE)0;                                 \
    else                                                \
        pi = (PIFACE) &ifaces[(iface_no)];


#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* ******  RS232                                                 ******   */
/* ********************************************************************   */
#define SLIP_END 0300
#define SLIP_ESC 0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

/* PPP frame format (see pg 6 of RFC 1331)   */
#if (INCLUDE_PPP)
/* Define this structure here to untangle header include orders.
   see ppp.h for ppp_hdr */
struct ppp_hdr_alias
{
    byte   addr;
    byte   control;
    word   protocol;
};
#endif
/* ********   */
typedef struct _rs232_if_info
{
    int     index;      /* index into rs232_if_info_arry */
    /* These three field pass info between the app layer and the driver   */
    int     comm_port;    /* Comm Port */
    int     baud_rate;   /* Baud rate - see above */
    char    handshake_type;     /* (N)one (D)tr/Dsr (R)ts/Cts */

    PIFACE  rs232_pi;
    byte    rs232_end;
    struct  rs232_statistics stats;
    PFBYTE  input_buffer;
    int     input_buffer_len;
    /* circular output buffer written to by uart_send   */
    PFBYTE  output_buffer;
    int     output_buffer_len;
#if (INCLUDE_CSLIP)
    /* Van Jacobson TCP/IP header compression   */
    struct slcompress KS_FAR *cslip_slhcp;
#endif
#if (INCLUDE_PPP)
    RTIP_BOOLEAN in_header;
    word    calc_fcs;
    byte    header[4];
    dword   rs232_accm;
    struct ppp_hdr_alias ph;
#endif
    /* escape buffer   */
    /* We escacpe chars to send in this buffer before calling uart_send.
       Note: Leave the +10.. the blocking code needs some breathing room 
             to store the FCS which could be escaped and the start/stop
             flag */
    byte out_buf_esc_base[CFG_RS232_XMBUFSIZE+10];

    /* pointers to escaped buffer (out_buf_esc_base) which is being output;   */
    /* this is need in case need to wait for room to open up in               */
    /* circular output buffer                                                 */
    PFBYTE   out_buf_esc_out;   /* pointer to next byte to send to uart_send */
    int      nout_esc;          /* number of chars in out_buf_esc_out */
    PFBYTE   out_buf_esc_in;    /* pointer to next byte to write */

#if (INCLUDE_PPP)
    word     out_buf_calc_fcs;  /* FCS calculation */
    dword    out_buf_accm;
#endif

    /* pointer to DCU which is being output; this is needed in case   */
    /* need to wait for room to open up in circular output buffer     */
    PFBYTE   packet_out;
    int      packet_out_left;

    /* INPUT INFO   */
#if (INCLUDE_MODEM)
    RTIP_BOOLEAN  raw_mode;
    PFCHAR   raw_mode_buffer;
    int      raw_mode_index;
    DCU      raw_mode_buffer_dcu;
#endif
    DCU      msg_in;        /* input packet - will be sent to input list */
                            /* when pkt separator received   */
    PFBYTE   pb_in;
    RTIP_BOOLEAN  esc_chars;
    RTIP_BOOLEAN  toss_chars;
    int      length;
} RS232_IF_INFO;
typedef struct _rs232_if_info KS_FAR *PRS232_IF_INFO;

/* *********************************************************************   */
/* ***** SET UP CALLS FROM RS232 to PPP                                    */
/* *********************************************************************   */
typedef RTIP_BOOLEAN (*PPP_XMIT_ESCAPE_FNC)(PRS232_IF_INFO pif_info);
typedef void (*PPP_GIVE_STRING_FNC)(PRS232_IF_INFO pif_info, PFBYTE buffer, int n);

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* BOOTP                                                                   */
/* *********************************************************************   */
#if (INCLUDE_BOOTP)
/* BOOTP support   */

/* This is the bootp packet sent between client & server   */
struct bootpkt
{
    byte    bp_op;          /* packet opcode type */

    #define    BOOTREQUEST  1
    #define    BOOTREPLY    2
    byte    bp_htype;               /* hardware addr type */
    byte    bp_hlen;                /* hardware addr length */
    byte    bp_hops;                /* gateway hops */
    dword   bp_xid;                 /* transaction ID */
    word    bp_secs;                /* seconds since boot began */
    word    bp_unused;
    byte    bp_ciaddr[IP_ALEN];     /* client IP address */
    byte    bp_yiaddr[IP_ALEN];     /* 'your' IP address */
    byte    bp_siaddr[IP_ALEN];     /* server IP address */
    byte    bp_giaddr[IP_ALEN];     /* gateway IP address */
    byte    bp_chaddr[16];          /* client hardware address */
    byte    bp_sname[64];           /* server host name */
    byte    bp_file[BOOTP_FILE_LEN];/* boot file name */
    byte    bp_vend[64];            /* vendor-specific area */
};

/* UDP port numbers, server and client.   */
#define IPPORT_BOOTPS        67
#define IPPORT_BOOTPC        68

/* *********************************************************************   */
/* "vendor" data permitted for Stanford boot clients.                      */
struct vend
{
    byte    v_magic[4];    /* magic number */
    dword   v_flags;       /* flags/opcodes, etc. */
    byte    v_unused[56];  /* currently unused */
};

#define VM_STANFORD    "STAN"               /* v_magic for Stanford */
#define VM_RFC1048      "\143\202\123\143"  /* same as 99.130.83.99 */

/* v_flags values   */
#define VF_PCBOOT          1    /* an IBMPC or Mac wants environment info */
#define VF_HELP            2    /* help me, I'm not registered */
#define TAG_BOOTFILE_SIZE  13   /* tag used by vend fields rfc 1048 */

void parse_bootpacket(PBOOTPKT pb, PBOOTR  presults);

#endif      /* (INCLUDE_BOOTP) */
#endif      /* INCLUDE_RTIP */

/* ********************************************************************   */
/* who parameter to xn_malloc                                             */
/*
 * Used by the DEBUG/TRACKING code in BGET to detect (maximum)
 * heap consumption per 'who': display_bget() et al
 */
#define CONFIG_WHO_BASE_MALLOC     (0x1000)

/* NOTE: who parameter used for xn_malloc   */
#define XDRBYTES_MALLOC         (CONFIG_WHO_BASE_MALLOC+ 1)
#define XDRSTRING_MALLOC        (CONFIG_WHO_BASE_MALLOC+ 2)
#define XDRARR_MALLOC           (CONFIG_WHO_BASE_MALLOC+ 3)
#define XDRREF_MALLOC           (CONFIG_WHO_BASE_MALLOC+ 4)
#define XDRCALL1_MALLOC         (CONFIG_WHO_BASE_MALLOC+ 5)
#define XDRCALL2_MALLOC         (CONFIG_WHO_BASE_MALLOC+ 6)
#define XDRREC1_MALLOC          (CONFIG_WHO_BASE_MALLOC+ 7)
#define XDRREC2_MALLOC          (CONFIG_WHO_BASE_MALLOC+ 8)
#define NSPROG_MALLOC           (CONFIG_WHO_BASE_MALLOC+ 9)
#define NSREAD_MALLOC           (CONFIG_WHO_BASE_MALLOC+10)
#define NSPATH_MALLOC           (CONFIG_WHO_BASE_MALLOC+11)
#define ALLOCDIR_MALLOC         (CONFIG_WHO_BASE_MALLOC+12)
#define UNIX_MALLOC             (CONFIG_WHO_BASE_MALLOC+13)
#define CACHE_MALLOC            (CONFIG_WHO_BASE_MALLOC+14)
#define XDRBUF_MALLOC           (CONFIG_WHO_BASE_MALLOC+15)
#define COOKIE_MALLOC           (CONFIG_WHO_BASE_MALLOC+17)
#define COOKIE_NAME_MALLOC      (CONFIG_WHO_BASE_MALLOC+18)
#define FHANDLE_MALLOC          (CONFIG_WHO_BASE_MALLOC+19)

#define SNMP_PWD2KEYV3_ALLOC    (CONFIG_WHO_BASE_MALLOC+20)

#define SYSLOG_BUFFER_MALLOC    (CONFIG_WHO_BASE_MALLOC+21)     /* syslog.c */

#define SSL_MALLOC              (CONFIG_WHO_BASE_MALLOC+22)     /* SSL+crypto */
#define SSL_WRAPPER_MALLOC      (CONFIG_WHO_BASE_MALLOC+23)     /* rtipreal.c */

#define DEVTAB_MALLOC           (CONFIG_WHO_BASE_MALLOC+24)

#define APP_MALLOC              (CONFIG_WHO_BASE_MALLOC+25)     /* clisrv et al */

#define RTTABLE_MALLOC          (CONFIG_WHO_BASE_MALLOC+26)
#define FRAGTABLE_MALLOC        (CONFIG_WHO_BASE_MALLOC+27)
#define IP2ETH_MALLOC           (CONFIG_WHO_BASE_MALLOC+28)
#define DEVTBL_MALLOC           (CONFIG_WHO_BASE_MALLOC+29)
#define MCLISTIP_MALLOC         (CONFIG_WHO_BASE_MALLOC+30)
#define MCLISTETH_MALLOC        (CONFIG_WHO_BASE_MALLOC+31)
#define MCASTCNT_MALLOC         (CONFIG_WHO_BASE_MALLOC+32)
#define MCASTREPORT_MALLOC      (CONFIG_WHO_BASE_MALLOC+32)
#define MCASTLHTR_MALLOC        (CONFIG_WHO_BASE_MALLOC+32)
#define ARPCACHE_MALLOC         (CONFIG_WHO_BASE_MALLOC+33)
#define IFACES_MALLOC           (CONFIG_WHO_BASE_MALLOC+33)
#define ALLOCED_PORTS_MALLOC    (CONFIG_WHO_BASE_MALLOC+34)
#define DCU_MALLOC              (CONFIG_WHO_BASE_MALLOC+35)
#define EPACKET_MALLOC          (CONFIG_WHO_BASE_MALLOC+36)
#define DCU_DATA_MALLOC         (CONFIG_WHO_BASE_MALLOC+37)
#define PACKET_POOL_MALLOC      (CONFIG_WHO_BASE_MALLOC+38)
#define UDPPORT_MALLOC          (CONFIG_WHO_BASE_MALLOC+39)
#define TCPPORT_MALLOC          (CONFIG_WHO_BASE_MALLOC+40)
#define DCU_TABLE_MALLOC        (CONFIG_WHO_BASE_MALLOC+41)
#define IFSIG_MALLOC            (CONFIG_WHO_BASE_MALLOC+43)
#define IFSEM_MALLOC            (CONFIG_WHO_BASE_MALLOC+44)
#define BOOTP_RT1_MALLOC        (CONFIG_WHO_BASE_MALLOC+44)
#define BOOTP_DNS_MALLOC        (CONFIG_WHO_BASE_MALLOC+44)
#define BOOTP_RT2_MALLOC        (CONFIG_WHO_BASE_MALLOC+44)
#define I82559_MALLOC           (CONFIG_WHO_BASE_MALLOC+45)
#define ALLOCPORTS_MALLOC       (CONFIG_WHO_BASE_MALLOC+46)

#define NETUTIL_MALLOC          (CONFIG_WHO_BASE_MALLOC+47)
#define VF_MALLOC               (CONFIG_WHO_BASE_MALLOC+48)

#define SRV_MALLOC              (CONFIG_WHO_BASE_MALLOC+49)
#define FTP_SRV_MALLOC          (CONFIG_WHO_BASE_MALLOC+50)
#define FTP_XFER_MALLOC         (CONFIG_WHO_BASE_MALLOC+51)
#define WEB_SRV_MALLOC          (CONFIG_WHO_BASE_MALLOC+52)
#define TNS_SRV_MALLOC          (CONFIG_WHO_BASE_MALLOC+53)
#define BOOTP_MALLOC            (CONFIG_WHO_BASE_MALLOC+54)
#define TASK_MALLOC             (CONFIG_WHO_BASE_MALLOC+55)
#define ARP_MALLOC              (CONFIG_WHO_BASE_MALLOC+56)
#define SMB_MALLOC              (CONFIG_WHO_BASE_MALLOC+57)
#define DRV_MALLOC              (CONFIG_WHO_BASE_MALLOC+58)

#define CONFIG_MAX_WHO_MALLOC   (59)

/* *********************************************************************   */
/* ******    MULTICAST                                            ******   */
/* *********************************************************************   */

/*
 * Macro to map an IP multicast address to an Ethernet multicast address.
 * The high-order 25 bits of the Ethernet address are statically assigned,
 * and the low-order 23 bits are taken from the low end of the IP address.
 * Complements NetBsd
 */
#define ETHER_MAP_IP_MULTICAST(ipaddr, enaddr) \
    /* PFBYTE ipaddr; */                       \
    /* PFBYTE enaddr;      */                  \
{                                              \
    (enaddr)[0] = (byte)0x01;                  \
    (enaddr)[1] = (byte)0x00;                  \
    (enaddr)[2] = (byte)0x5e;                  \
    (enaddr)[3] = (byte) ((ipaddr)[1] & 0x7f); \
    (enaddr)[4] = (ipaddr)[2];                 \
    (enaddr)[5] = (ipaddr)[3];                 \
}

/* Macro to set address I to B0, B1, B2, B3   */
#define SETMCASTADDR(P, I, B0, B1, B2, B3) \
    *(P+(4*I)+0) = B0; *(P+(4*I)+1) = B1; *(P+(4*I)+2) = B2; *(P+(4*I)+3) = B3;
#endif      /* INCLUDE_RTIP */

#if (INCLUDE_SNMP)
/* ********************************************************************   */
/* snmp variables                                                         */
/* ********************************************************************   */

/* mib variables which are kept solely for SNMP purposes   */
struct mib_variables
{
    struct oid  SysObjectID;

    dword   LastMsgID;

    /* System & Interface MIB   */
    char    SysDescr[CFG_SNMP_VER_LEN+1];
    char    SysContact[CFG_SNMP_CON_LEN+1];
    char    SysName[CFG_SNMP_NAME_LEN+1];
    char    SysLocation[CFG_SNMP_LOC_LEN+1];
    dword   SysUpTime;                           /* uptime in 1/100th of seconds */
    dword   SysServices;
    dword   IfNumber;

    /* IP MIB   */
    dword   IpInReceives;
    dword   IpInHdrErrors;
    dword   IpInAddrErrors;
    dword   IpForwDatagrams;
    dword   IpInUnknownProtos;
    dword   IpInDiscards;
    dword   IpInDelivers;
    dword   IpOutRequests;
    dword   IpOutDiscards;
    dword   IpOutNoRoutes;
    dword   IpReasmTimeout;
    dword   IpReasmReqds;
    dword   IpReasmOKs;
    dword   IpReasmFails;
    dword   IpFragOKs;
    dword   IpFragFails;
    dword   IpFragCreates;
    dword   IpRoutingDiscards;

    /* ICMP MIB   */
    dword IcmpInMsgs;
    dword   IcmpInErrors;
    dword   IcmpInDestUnreachs;
    dword   IcmpInTimeExcds;
    dword   IcmpInParmProbs;
    dword   IcmpInSrcQuenchs;
    dword   IcmpInRedirects;
    dword   IcmpInEchos;
    dword   IcmpInEchoReps;
    dword   IcmpInTimestamps;
    dword   IcmpInTimestampReps;
    dword   IcmpInAddrMasks;
    dword   IcmpInAddrMaskReps;
    dword   IcmpOutMsgs;
    dword   IcmpOutErrors;
    dword   IcmpOutDestUnreachs;
    dword   IcmpOutTimeExcds;
    dword   IcmpOutParmProbs;
    dword   IcmpOutSrcQuenchs;
    dword   IcmpOutRedirects;
    dword   IcmpOutEchos;
    dword   IcmpOutEchoReps;
    dword   IcmpOutTimestamps;
    dword   IcmpOutTimestampReps;
    dword   IcmpOutAddrMasks;
    dword   IcmpOutAddrMaskReps;

    /* UDP MIB   */
    dword   UdpInDatagrams;
    dword   UdpNoPorts;
    dword   UdpInErrors;
    dword   UdpOutDatagrams;

    /* TCP MIB   */
    dword   TcpRtoAlgorithm;
    dword   TcpRtoMin;
    dword   TcpRtoMax;
    dword   TcpMaxConn;
    dword   TcpActiveOpens;
    dword   TcpPassiveOpens;
    dword   TcpAttemptFails;
    dword   TcpEstabResets;
    dword   TcpCurrEstab;
    dword   TcpInSegs;
    dword   TcpOutSegs;
    dword   TcpRetransSegs;
    dword   TcpInErrs;
    dword   TcpOutRsts;

    /* SNMP MIB   */
    dword   SnmpInPkts;
    dword   SnmpOutPkts;
    dword   SnmpInBadVersions;
    dword   SnmpInBadCommunityNames;
    dword   SnmpInBadCommunityUses;
    dword   SnmpInASNParseErrs;
    dword   SnmpInTooBigs;
    dword   SnmpInNoSuchNames;
    dword   SnmpInBadValues;
    dword   SnmpInReadOnlys;
    dword   SnmpInGenErrs;
    dword   SnmpInTotalReqVars;
    dword   SnmpInTotalSetVars;
    dword   SnmpInGetRequests;
    dword   SnmpInGetNexts;
    dword   SnmpInSetRequests;
    dword   SnmpInGetResponses;
    dword   SnmpInTraps;
    dword   SnmpOutTooBigs;
    dword   SnmpOutNoSuchNames;
    dword   SnmpOutBadValues;
    dword   SnmpOutGenErrs;
    dword   SnmpOutGetRequests;
    dword   SnmpOutGetNexts;
    dword   SnmpOutSetRequests;
    dword   SnmpOutGetResponses;
    dword   SnmpOutTraps;
    dword   SnmpOutEnableAuthTraps;

    /* TRANSMISSION - RS232   */
    dword   Rs232Number;

    /* TRANSMISSION - RS232 - Async Port Table   */
#if (INCLUDE_SNMPV2)
    byte    AgentID[13];
    dword   AgentBoots;
    dword   AgentSize;
    dword   UsecStatsUnsupportedQos;
    dword   UsecStatsNotInWindows;
    dword   UsecStatsUnknownUserNames;
    dword   UsecStatsWrongDigestValues;
    dword   UsecStatsUnknownContexts;
    dword   UsecStatsBadParameters;
    dword   UsecStatsUnauthorizedOperations;
#endif

    dword   snmpEngineMaxMessageSize;  /* maximum accepted size of SNMP packets; ETHERSIZE by default */

#if (INCLUDE_SNMPV3)
    byte    snmpEngineID[33];
    dword   snmpEngineBoots;
    dword   snmpEngineTime;                           /* uptime in seconds */
    dword   snmpUnknownSecurityModels;
    dword   snmpInvalidMsgs;
    dword   snmpUnknownPDUHandlers;
    dword   snmpTargetSpinLock;
    dword   snmpUnavailableContexts;
    dword   snmpUnknownContexts;
    dword   usmStatsUnsupportedSecLevels;
    dword   usmStatsNotInTimeWindows;
    dword   usmStatsUnknownUserNames;
    dword   usmStatsUnknownEngineIDs;
    dword   usmStatsWrongDigests;
    dword   usmStatsDecryptionErrors;
    dword   usmUserSpinLock;
    dword   vacmViewSpinLock;
    dword   snmpEncryptionSaltCounter;
#endif
};

#if (USE_OFFSET)
#define MIB_VAR(X)     (rc->mib_vars.X)
#define MIB_VAR_OFF(X) ((PFCHAR)STRUCT_OFFSET(rtip_context,mib_vars.X))
#else
#define MIB_VAR(X)     mib_vars.X
#endif
#endif /* INCLUDE_SNMP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******    space saving function macros                         ******   */
/* *********************************************************************   */

#if (!INCLUDE_ERROR_CHECKING)
#define api_sock_to_port(socket_index) IS_INVALID_SOCKET(socket_index) ? \
                                       0 : alloced_ports[(socket_index)]
#endif

/* *********************************************************************   */
/* ******    parameter, return values etc                         ******   */
/* *********************************************************************   */

/* parameter to trans_state()   */
#define FROM_INTERPRET      TRUE
#define NOT_FROM_INTERPRET  FALSE

/* parameter options to tcpsend()   */
#define NO_OPTIONS 1
#define MSS_OPTION 2

/* parameters to tcpsend() which are plugged in tcp output packet   */
typedef struct tcp_out_params
{
    word    dlen;
    byte    tcp_flags;
    int     msg_type;           /* keepalive pkt */
} TCP_OUT_PARAMS;

/* parameter to tc_tcpsend()   */
#define KEEPALIVE        1
#define NORMAL_SEND      2
#define NORMAL_DATA_ONLY 3  /* send packet only if can send data; */
                            /* already passed sender SWS and NAGLE test   */
#define NORMAL_DATA      4  /* send packet along with any data; */
                            /* passed sender SWS and NAGLE test   */
#define RETRANS          5  /* retransmit due to timeout */
#define FAST_RETRANS     6  /* fast retransmit due to dup acks */
#define WINDOW_PROBE     7  /* window probe - i.e. send 1 byte */

/* return value from device driver xmit   */
#define REPORT_XMIT_DONE 0x4000     /* xmit done */
#define REPORT_XMIT_MORE 0x2000     /* can xmit more */

/* parameter to tc_ino2_iface   */
#define SET_ERRNO        TRUE
#define DONT_SET_ERRNO   FALSE

/* return values from check_syn()   */
#define VALID_PKT       1
#define DROP_PKT        2
#define PROC_FLAGS_ONLY 3

/* return values from closesocket   */
/* NOTE: errno can also be returned */
#define CLOSE_SUCCESS   0
#define PORT_FREED     -1

/* parameter to xn_pkt_data_pointer   */
#define INPUT_PKT       1   /* pkt was input, therefore, all fields are */
                            /* set up   */
#define OUTPUT_PKT      2   /* pkt will be used for output, no fields in */
                            /* packet been set up   */
#define OUTPUT_FRAG_PKT 3   /* pkt will be used for output, frag fields are */
                            /* already set up   */

/* parameter to tc_find_accept_port   */
#define TAKE_SLAVE_OFF      TRUE
#define NO_TAKE_SLAVE_OFF   FALSE

/* parameter to tc_udp_pk_send   */
#define NO_FRAG 0

/* parameter to xn_abort   */
#define SEND_RESET      TRUE
#define DONT_SEND_RESET FALSE

/* tcp - parameter to setup_ws   */
#define EBS_INPUT  1
#define EBS_OUTPUT 2

#endif      /* INCLUDE_RTIP */

/* values for resource_initialized   */
#define INIT_NOT_DONE 0
#define INIT_DONE     1
#define INIT_IN_PROG  2

#if (INCLUDE_RTIP)
/*****************************************************************   */
/* TASK STUFF                                                        */
/*****************************************************************   */

/* maximum number of tasks which will be spawned to handle server requests   */
#define TASKS_MAX_SPAWN \
    (CFG_WEB_MAX_SPAWN+CFG_FTP_MAX_SPAWN+CFG_TELNET_MAX_SPAWN)
#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* CALLBACKS                                                               */
/* *********************************************************************   */

#if (INCLUDE_PPP && INCLUDE_CHAP)
#define CB_CHAP_GET_RANDOM_VALUE(a, b)          \
    if (rtip_callbacks && rtip_callbacks->cb_chap_get_random_value_fnc)           \
        rtip_callbacks->cb_chap_get_random_value_fnc(a, b)
#endif

#define CB_WR_SCREEN_STRING(a, b)                                       \
    if (rtip_callbacks && rtip_callbacks->cb_wr_screen_string_fnc)      \
        rtip_callbacks->cb_wr_screen_string_fnc(a, b)
#define CB_WR_INTERRUPT_STRING()                                        \
    if (rtip_callbacks && rtip_callbacks->cb_wr_interrupt_string_fnc)   \
        rtip_callbacks->cb_wr_interrupt_string_fnc()

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
#define CB_RAW_MODE_IN_CHAR(a)                                          \
    if (rtip_callbacks && rtip_callbacks->cb_raw_mode_in_char_fnc)      \
        rtip_callbacks->cb_raw_mode_in_char_fnc(a)
#endif
#if (INCLUDE_PPP || INCLUDE_SLIP)
#define CB_RS232_CONNECTION_LOST(a)                                     \
    if (rtip_callbacks && rtip_callbacks->cb_rs232_connection_lost_fnc) \
        rtip_callbacks->cb_rs232_connection_lost_fnc(a)
#endif

#define CB_ERROR_REPORT(a, b, c)                                        \
    if (rtip_callbacks && rtip_callbacks->cb_error_report_fnc)          \
        rtip_callbacks->cb_error_report_fnc(a, b, c)

/* CALLBACK DECLARATIONS   */
#define CB_TELNET_INIT(a)       \
    if (rtip_callbacks && rtip_callbacks->cb_telnet_init_fnc)           \
        rtip_callbacks->cb_telnet_init_fnc(a)

#define CB_TELNET_INCHAR(a, b, n)  \
    if (rtip_callbacks && rtip_callbacks->cb_telnet_inchar_fnc)         \
        n = rtip_callbacks->cb_telnet_inchar_fnc(a, b)

#define CB_FTP_CHECK_USER_NAME(a, b)  \
    if (rtip_callbacks && rtip_callbacks->cb_ftp_check_user_name_fnc)   \
        return(rtip_callbacks->cb_ftp_check_user_name_fnc(a, b));

#define CB_FTP_CHECK_PASSWORD(a, b)  \
    if (rtip_callbacks && rtip_callbacks->cb_ftp_check_password_fnc)    \
        return(rtip_callbacks->cb_ftp_check_password_fnc(a, b));

#if (INCLUDE_DHCP_CLI)
#define CB_DHCP_NEW_IP(a)                                               \
    if (rtip_callbacks && rtip_callbacks->cb_dhcp_new_ip_fnc)           \
        rtip_callbacks->cb_dhcp_new_ip_fnc(a);
#define CB_DHCP_NO_IP(a)                                                \
    if (rtip_callbacks && rtip_callbacks->cb_dhcp_no_ip_fnc)           \
        rtip_callbacks->cb_dhcp_no_ip_fnc(a);
#endif

/* *********************************************************************   */
/* ******                  ENTER API MACRO                        ******   */
/* *********************************************************************   */

/* parmeter to RTIP_API_ENTER   */
#if (INCLUDE_API_STR)

#ifdef __cplusplus
extern "C" {
#endif

    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_rarp_name[8];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_bootp_name[9];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_ping_name[8];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_set_ip_name[10];
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_attach_name[10];
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
    KS_EXTERN_GLOBAL_CONSTANT char KS_FAR xn_interface_open_config_name[25];
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

#ifdef __cplusplus
}
#endif

#endif

/* *********************************************************************   */
/* ******   redefinitions of TCP functions if TCP not defined     ******   */
/* *********************************************************************   */
#if (!INCLUDE_TCP)

#ifdef __cplusplus
extern "C" {
#endif

int tc_tcp_error(int rtip_errno);
int tc_tcp_error_zero(int rtip_errno);
int tc_tcp_error_errno(int rtip_errno);

#define tc_tcp_sock_alloc(A)         (PTCPPORT) tc_tcp_error_zero(EOPNOTSUPPORT)
#define tc_tcp_bind(A,B,C,D)         tc_tcp_error(EOPNOTSUPPORT)
#define tc_tcp_listen(A,B,C)         tc_tcp_error(EOPNOTSUPPORT)
#define tc_tcp_accept(A)             (PTCPPORT) tc_tcp_error_zero(EOPNOTSUPPORT)
#define tc_tcp_connect(A,B,C)        tc_tcp_error(EOPNOTSUPPORT)
#define tc_tcp_pk_peer_address(A,B,C) tc_tcp_error(0)
#define tc_tcp_read(A,B,C,D,E,F)     tc_tcp_error(EOPNOTSUPPORT)
#define tc_tcp_write(A,B,C,D,E,F)    tc_tcp_error(EOPNOTSUPPORT)
#define tc_tcp_abort(A,B)            tc_tcp_error(EOPNOTSUPPORT)
#define tc_tcp_close(A)              tc_tcp_error(EOPNOTSUPPORT)
#define tc_find_accept_port(A,B)     (PTCPPORT)tc_tcp_error_zero(EOPNOTSUPPORT)
#define tc_is_read_state(A)          (RTIP_BOOLEAN)tc_tcp_error_zero(0)
#define tc_is_write_state(A)         (RTIP_BOOLEAN)tc_tcp_error_zero(0)
#define tc_tcpsend(A,B,C,D,E)        tc_tcp_error_errno(EOPNOTSUPPORT)
#define tc_tcp_timeout(A, B)         tc_tcp_error(0)

#ifdef __cplusplus
}
#endif

#endif

#if (!INCLUDE_UDP && !INCLUDE_RAW)

#ifdef __cplusplus
extern "C" {
#endif

int tc_udp_error(int rtip_errno);
int tc_udp_error_zero(int rtip_errno);
int tc_udp_error_errno(int rtip_errno);

#define tc_udp_sock_alloc(A, B)      (PTCPPORT) tc_udp_error_zero(EOPNOTSUPPORT)
#define tc_udp_bind(A, B, C)         tc_udp_error(EOPNOTSUPPORT)
#define tc_udp_connect(A, B, C)      tc_udp_error(EOPNOTSUPPORT)
#define tc_udp_pk_peer_address(A, B, C) tc_udp_error(0)
#define tc_udp_close(A)              tc_udp_error(EOPNOTSUPPORT)

#ifdef __cplusplus
}
#endif

#endif

/* *********************************************************************   */
/* ******  redefinitions of ICMP functions if ICMP not defined    ******   */
/* *********************************************************************   */
#if (!INCLUDE_ICMP)
#define tc_icmp_send(A, B, C, D)
#define check_send_icmp(A)          FALSE
#endif

/* *********************************************************************   */
/* ******    redefinitions of DNS functions if DNS not defined    ******   */
/* *********************************************************************   */
#if (!INCLUDE_DNS)
#define init_dns()
#endif

/* *********************************************************************   */
/* ***** definition of who called os_alloc_packet (passed as a parameter); */
/* ***** used for tracking packets (see INCLUDE_TRK_PACKETS)               */
/* ***** NOTE: also passed to xn_malloc                                    */
/* *********************************************************************   */
#define ARP_SEND_ALLOC          1       /* arp.c */
#define CAST_ALLOC              2       /* arp.c */
#define RARP_SEND_ALLOC         3       /* arp.c */
#define DE4X5_ALLOC             4       /* de4x5.c */
#define DNS_STRING_ALLOC        5       /* dns.c */
#define DNS_SPACE_ALLOC         6       /* dns.c */
#define DNS_HOST1_ALLOC         7       /* dns.c */
#define DNS_HOST2_ALLOC         8       /* dns.c */
#define DNS_HOST3_ALLOC         9       /* dns.c */
#define FRAG_ALLOC              10      /* fragment.c */
#define ICMP_ALLOC              11      /* icmp.c */
#define IGMP_ALLOC              12      /* igmp.c */
#define DRIVER_ALLOC            13      /* drivers, e.g. ifed.c etc */
#define PPP_ALLOC               21      /* ppp.c */
#define FSM_TR_ALLOC            22      /* pppfsm.c */
#define FSM_TA_ALLOC            23      /* pppfsm.c */
#define IPCP_MR_ALLOC           24      /* pppipcp.c */
#define IPCP_REPLY_ALLOC        25      /* pppipcp.c */
#define LCP_MR_ALLOC            26      /* ppplcp.c */
#define LCP_REPLY_ALLOC         27      /* ppplcp.c */
#define PAP_MR_ALLOC            28      /* ppppap.c */
#define PAP_REPLY_ALLOC         29      /* ppppap.c */
#define CHAP_MR_ALLOC           30      /* pppchap.c */
#define CHAP_REPLYC_ALLOC       31      /* pppchap.c */
#define CHAP_REPLYR_ALLOC       32      /* pppchap.c */
#define ECHO_ALLOC              33      /* pppfsm.c */
#define RS232_IN_ALLOC          34      /* rs232.c */
#define RS232_OUT1_ALLOC        35      /* rs232.c */
#define RS232_OUT2_ALLOC        36      /* rs232.c */
#define RS232_BUF_ALLOC         37      /* rs232.c */
#define PING_ALLOC              39      /* rtipapi.c */
#define USER_ALLOC              40      /* rtipapi.c */
#define MODEM_ALLOC             41      /* rtipapi.c */
#define MODEM_ANS_ALLOC         42      /* rtipapi.c */
#define SLIP_ALLOC              43      /* slip.c */
#define SENDTO_ALLOC            45      /* socket.c */
#define TCP_RESET_ALLOC         46      /* tcp.c */
#define TCP_OUTPUT_ALLOC        47      /* tcp.c */
#define TCP_QUE_IN_ALLOC        48      /* tcp.c */
#define TCP_QUE_OUT_ALLOC       49      /* tcp.c */
#define RAW_RCV                 50      /* udp.c */
#define LINE_INIT1_ALLOC        51      /* rtipapi.c */
#define LINE_INIT2_ALLOC        52      /* rtipapi.c */
#define LINE_PUT_ALLOC          53      /* rtipapi.c */
#define RS232_RAW_ALLOC         54      /* rs232.c */
#define CLOSE_IFACE_ALLOC       58      /* iface.c */
#define DCU_ALLOC_CORE_ALLOC    59      /* os.c */
#define NFS_AUTHU_ALLOC        100      /* ncauth.c */
#define NFS_DYN_ALLOC          101      /* nfs_util.c */
#define NFS_UNIVERSAL_ALLOC    102      /* nsrpc.c */
#define NFS_MNT_ALLOC          103      /* nsmount.c */
#define NFS_UMNT_ALLOC         104      /* nsmount.c */
#define NFS_LOOKUP_ALLOC       105      /* nsnfs.c */
#define NFS_CREATE_ALLOC       106      /* nsnfs.c */
#define NFS_REMOVE_ALLOC       107      /* nsnfs.c */
#define NFS_RENAME_FROM_ALLOC  108      /* nsnfs.c */
#define NFS_RENAME_TO_ALLOC    109      /* nsnfs.c */
#define NFS_MKDIR_ALLOC        110      /* nsnfs.c */
#define NFS_RMDIR_ALLOC        111      /* nsnfs.c */
#define NFS_READDIR_ALLOC      112      /* nsnfs.c */
#define FTP_DAEMON_ALLOC       113      /* ftpsrv.c */
#define SNMP_IO_ALLOC          114      /* snmpapi.c */
#define SNMP_RQD_ALLOC         115      /* snmpapi.c */
#define SNMP_DYN_ALLOC         116      /* snmputil.c */
#define SNMP_SNB2A_ALLOC       117      /* snmputil.c */
#define SNMP_RSLV_ALLOC        118      /* snmputil.c */
#define SNMP_MKSNMP_ALLOC      119      /* snmpproc.c */
#define SNMP_MKTRAP_ALLOC      120      /* snmpproc.c */
#define SNMP_MKTRAPV2_ALLOC    121      /* snmpproc.c */
#define SNMP_MKTRAPV3_ALLOC    122      /* snmpproc.c */
#define WEB_USER_ALLOC         124      /* in webserm directory */
#define WEB_MHDR_ALLOC         125      /* webproc.c */
#define WEB_XFER_ALLOC         126      /* webproc.c */
#define RIP_ALLOC              127      /* rip.c */
#define SMTP_FORMAT_ALLOC      128      /* smtp.c */
#define SSL_CRYPTO_ALLOC       129      /* crypto/rsa */
#define SLHC_ALLOC1            201      /* slhc.c */
#define SLHC_ALLOC2            202      /* slhc.c */
#define DHCPS_ALLOC            203      /* dhcps.c */
#define TNC_ALLOC              204      /* telnetc.c */
#define PPPOE_ALLOC            205      /* pppoe.c */

/* *********************************************************************   */
/* ***** definition of who called OS_CLAIM_XXX (passed as a parameter);    */
/* ***** used for tracking packets (see INCLUDE_TRK_PACKETS)               */
/* ***** NOTE: SEMAPHORE DEFINITIONS ARE IN os.h                           */
/* *********************************************************************   */

/* TABLE SEMAPHORE   */
#define ARP_SEARCH_TABLE_CLAIM      1   /* arp.c */
#define ARP_ADD_TABLE_CLAIM         2   /* arp.c */
#define ARP_DEL_TABLE_CLAIM         3   /* arp.c */
#define ARP_REMOVE_TABLE_CLAIM      4   /* arp.c */
#define ARP_ADDIN_TABLE_CLAIM       5   /* arp.c */
#define ARP_INTERPRET_TABLE_CLAIM   6   /* arp.c */
#define ARP_REPLY_TABLE_CLAIM       7   /* arp.c */
#define ARP_TMEOUT_TABLE_CLAIM      8   /* arp.c */
#define ARP_CLOSE_ARP_TABLE_CLAIM   9   /* arp.c */
#define ARP_SOCK_ARP_TABLE_CLAIM    10  /* arp.c */
#define RT_GET_TABLE_CLAIM          11  /* iface.c */
#define RT_ADD_TABLE_CLAIM          12  /* iface.c */
#define RT_DEL_TABLE_CLAIM          13  /* iface.c */
#define RT_DIFACE_TABLE_CLAIM       14  /* iface.c */
#define RT_REDIRECT_TABLE_CLAIM     15  /* iface.c */
#define RT_FREE_TABLE_CLAIM         16  /* iface.c */
#define RT_TIMER_TABLE_CLAIM        17  /* iface.c */
#define RT_GETR_TABLE_CLAIM         18  /* iface.c */
#define IPF_ADD_TABLE_CLAIM         19  /* ip.c */
#define IPF_TIMER1_TABLE_CLAIM      20  /* ip.c */
#define IPF_TIMER2_TABLE_CLAIM      21  /* ip.c */
#define SERVER1_TABLE_CLAIM         22  /* tasks.c */
#define SERVER2_TABLE_CLAIM         23  /* tasks.c */
#define SERVER3_TABLE_CLAIM         24  /* tasks.c */
#define SERVER4_TABLE_CLAIM         25  /* tasks.c */
#define SERVER5_TABLE_CLAIM         26  /* tasks.c */
#define DNS_INIT_TABLE_CLAIM        27  /* dns.c */
#define DNS_HOST1_TABLE_CLAIM       28  /* dns.c */
#define DNS_HOST2_TABLE_CLAIM       29  /* dns.c */
#define DNS_HOST3_TABLE_CLAIM       30  /* dns.c */
#define DNS_ADD_TABLE_CLAIM         31  /* dns.c */
#define DNS_DEL_TABLE_CLAIM         32  /* dns.c */
#define SNMP_CONFIG_TABLE_CLAIM     33  /* snmpapi.c */
#define SNMP_CONFIGT_TABLE_CLAIM    34  /* snmpapi.c */
#define SNMP_TRAP1_TABLE_CLAIM      35  /* snmpapi.c */
#define SNMP_TRAP2_TABLE_CLAIM      36  /* snmpapi.c */
#define NFS_USER_TABLE_CLAIM        37  /* ncutil.c */
#define NFS_USER2_TABLE_CLAIM       38  /* ncutil.c */
#define NFS_FREE_USER_TABLE_CLAIM   39  /* ncutil.c */
#define NFS_ALLOC_TABLE_CLAIM       40  /* ncutil.c */
#define WEB_PAGE_TABLE_CLAIM        41  /* httppstf.c */
#define NFS_MNT_CLAIM               42  /* nsmount.c */
#define USER_TABLE_CLAIM            43  /* osport.c, tools.c */
#define PING_ADD_TABLE_CLAIM        44  /* rtipapi.c */
#define PING_DEL_TABLE_CLAIM        45  /* rtipapi.c */
#define PING_SEARCH_TABLE_CLAIM     46  /* icmp.c */
#define NFS_DIR_INFO1_TABLE_CLAIM   47  /* nsutil.c */
#define NFS_DIR_INFO2_TABLE_CLAIM   48  /* nsutil.c */
#define NFS_DIR_INFO3_TABLE_CLAIM   49  /* nsutil.c */
#define NFS_DIR_INFO4_TABLE_CLAIM   50  /* nsutil.c */
#define KERNEL_TABLE_CLAIM          51  /* ostask.c */
#define PPPOE_CLIENT_TABLE_CLAIM    52  /* pppoe.c */


/* TCP SEMAPHORE   */
#define TME_CLAIM_TCP               62  /* tcp.c */
#define OUT_CLAIM_TCP               63  /* tcp.c */
#define INT_CLAIM_TCP               64  /* tcp.c */
#define SOCK_CLAIM_TCP              65  /* tcpuser.c */
#define BIND_CLAIM_TCP              66  /* tcpuser.c */
#define LISTEN_CLAIM_TCP            67  /* tcpuser.c */
#define ACCEPT_CLAIM_TCP            68  /* tcpuser.c */
#define CONNECT_CLAIM_TCP           69  /* tcpuser.c */
#define READ1_CLAIM_TCP             70  /* tcpuser.c */
#define READ2_CLAIM_TCP             71  /* tcpuser.c */
#define WRITE_CLAIM_TCP             72  /* tcpuser.c */
#define ABORT_CLAIM_TCP             73  /* tcpuser.c */
#define CLOSE1_CLAIM_TCP            74  /* tcpuser.c */
#define CLOSE2_CLAIM_TCP            75  /* tcpuser.c */
#define SELECT1_CLAIM_TCP           76  /* socket.c */
#define SELECT2_CLAIM_TCP           77  /* socket.c */
#define SHUT_CLAIM_TCP              78  /* socket.c */
#define SAY_PORTS_CLAIM_TCP         79  /* debug.c */

/* UDP SEMAPHORE   */
#define SOCK_CLAIM_UDP              80  /* udp.c */
#define BIND_CLAIM_UDP              81  /* udp.c */
#define CONNECT_CLAIM_UDP           82  /* udp.c */
#define WRITE_CLAIM_UDP             82  /* udp.c */
#define CLOSE_CLAIM_UDP             84  /* udp.c */
#define INTERPRET_CLAIM_UDP         85  /* udp.c */
#define INTERPRETR_CLAIM_UDP        86  /* udp.c */
#define HEADER_CLAIM_UDP            87  /* udp.c */
#define SELECT1_CLAIM_UDP           88  /* socket.c */
#define SELECT2_CLAIM_UDP           89  /* socket.c */
#define CLEAR_CACHE_CLAIM_UDP       90  /* udp.c */
#define SAY_PORTS_CLAIM_UDP         91  /* debug.c */

/* IFACE SEMAPHORE   */
#define DEVICE_CLAIM_IFACE          100 /* iface.c */
#define MCAST1_CLAIM_IFACE          101 /* iface.c */
#define MCAST2_CLAIM_IFACE          102 /* iface.c */
#define MCAST3_CLAIM_IFACE          103 /* iface.c */
#define OPEN1_CLAIM_IFACE           104 /* iface.c */
#define OPEN2_CLAIM_IFACE           105 /* iface.c */
#define SEND_CLAIM_IFACE            106 /* iface.c */
#define OUTPUT_CLAIM_IFACE          107 /* iface.c */
#define IP_SEND_CLAIM_IFACE         108 /* iface.c */
#define STATS_CLAIM_IFACE           109 /* iface.c */
#define CLOSE_CLAIM_IFACE           110 /* iface.c */
#define FCLOSE_CLAIM_IFACE          111 /* iface.c */
#define IGMP_TIMER_CLAIM_IFACE      112 /* igmp.c */
#define IGMP_STIMER_CLAIM_IFACE     113 /* igmp.c */
#define IGMP_RESET_CLAIM_IFACE      114 /* igmp.c */
#define PPP_CLAIM_IFACE             115 /* ppp.c */
#define PPP_CLOSE_CLAIM_IFACE       116 /* pppapi.c */
#define ATTACH_CLAIM_IFACE          117 /* rtipapi.c */
#define XMIT_DONE_IFACE             118 /* iface.c */
#define MCAST4_CLAIM_IFACE          119 /* iface.c */
#define DHCP_TO_CLAIM_IFACE         120 /* dhcpproc.c */
#define DHCP_RELEASE_CLAIM_IFACE    121 /* dhcpapi.c */
#define DHCP_EXTEND_CLAIM_IFACE     122 /* dhcpapi.c */

/* CRITICAL SEMAPHORE   */
#define ARPRM_CLAIM_CRITICAL        130 /* os.c */
#define ARPS_CLAIM_CRITICAL         131 /* os.c */
#define ARPCP_CLAIM_CRITICAL        132 /* arp.c */
#define ARPRC_CLAIM_CRITICAL        133 /* os.c */
#define EXCHS_CLAIM_CRITICAL        134 /* os.c */
#define EXCHR_CLAIM_CRITICAL        135 /* os.c */
#define EXCHC_CLAIM_CRITICAL        136 /* os.c */
#define ALLOCU_CLAIM_CRITICAL       137 /* os.c */
#define FREEU_CLAIM_CRITICAL        138 /* os.c */
#define ALLOCT_CLAIM_CRITICAL       139 /* os.c */
#define FREET_CLAIM_CRITICAL        140 /* os.c */
#define SNMP_CLAIM_CRITICAL         141 /* snmputil.c */
#define SNMP_RELEASE_CRITICAL       142 /* snmputil.c */
#define NFS_CLIENTA_CRITICAL        143 /* nfsutil.c */
#define NFS_CLIENTF_CRITICAL        144 /* nfsutil.c */
#define NFS_CTA_CRITICAL            145 /* nfsutil.c */
#define NFS_CTF_CRITICAL            146 /* nfsutil.c */
#define NFS_PMAPA_CRITICAL          147 /* nfsutil.c */
#define NFS_PMAPF_CRITICAL          148 /* nfsutil.c */
#define NFS_OPSA_CRITICAL           149 /* nfsutil.c */
#define NFS_OPSF_CRITICAL           150     /* nfsutil.c */
#define NFS_DCUA_CRITICAL           151 /* nfsutil.c */
#define NFS_DCUF_CRITICAL           152 /* nfsutil.c */
#define NFS_SDCUF_CRITICAL          153 /* nfsutil.c */
#define NFS_SVCA_CRITICAL           154 /* nfsutil.c */
#define NFS_SVCF_CRITICAL           155 /* nfsutil.c */
#define NFS_SVCXA_CRITICAL          156 /* nfsutil.c */
#define NFS_SVCXF_CRITICAL          157 /* nfsutil.c */
#define NFS_SVCBUFA_CRITICAL        158 /* nfsutil.c */
#define NFS_SVCBUFF_CRITICAL        159 /* nfsutil.c */
#define NFS_NSUA_CRITICAL           160 /* nfsutil.c */
#define NFS_NSUF_CRITICAL           161 /* nfsutil.c */

/* MEMFILE SEMAPHORE   */
#define ENTER_CLAIM_MEMFILE         200 /* vfile.c */

/* SYSLOG SEMAPHORE   */
#define VSYSLOG_CLAIM_SYSLOG        250 /* syslog.c */
#define WRITE2SYSLOG_CLAIM_SYSLOG   251 /* syslog.c */
#define OPENLOG_CLAIM_SYSLOG        252 /* syslog.c */
#define CLOSELOG_CLAIM_SYSLOG       253 /* syslog.c */
#define SETLOGMASK_CLAIM_SYSLOG     254 /* syslog.c */
#define ADDTASKIDX_CLAIM_SYSLOG     255 /* syslog.c */

#endif      /* INCLUDE_RTIP */


#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******           Function prototypes                     ************   */
/* *********************************************************************   */

/* enables using code as DLL            */
/* NOTE: the same define in in socket.h */
#define SOCKAPI_
/*#define SOCKAPI_  __stdcall __export   */

#ifdef __cplusplus
extern "C" {
#endif

/* File DNS.C:   */
#if (INCLUDE_DNS)
void     init_dns(void);
#if (INCLUDE_DNS_CACHE)
void     dns_timer(void);
#endif
#endif

/* File PPPOE.C:   */
void     tc_pppoed_interpret(PIFACE pi, DCU msg);
void     tc_pppoes_interpret(PIFACE pi, DCU msg);

/* File ARP.C:   */
int      tc_arp_remote(PIFACE pi, PANYPORT port, DCU msg, PFBYTE ipn, int flags, word wait_count);
int      tc_do_arp_send(PIFACE pi, PFBYTE ipn, PFBYTE lln, word type);
void     tc_arp_interpret(PIFACE pi, DCU msg);
void     tc_release_message(DCU msg, int errno_val);
#if (INCLUDE_ARP)
void     tc_arp_purge_timeout(int i);
void     tc_arp_timeout(dword gran);
void     tc_arp_closeport(PANYPORT port);
#endif

int      tc_rarp_send(PIFACE pi);
void     tc_rarp_reply(PIFACE pi, DCU msg, PFBYTE ip_address);
PFBYTE   tc_rarp_lookup(PFBYTE ether_address);

/* File ICMP.C:   */
void     tc_icmp_interpret(PIFACE pi, DCU msg);
#if (INCLUDE_ICMP)
RTIP_BOOLEAN    check_send_icmp(DCU msg);
int             tc_icmp_send(DCU msg, byte type, byte code, byte ptr);
#endif

/* File IFACE.C:   */
PIFACE          tc_ino2_iface(int iface_no, RTIP_BOOLEAN do_set_errno);
PIFACE          tc_ino2_device(int device_off);
int             tc_device_id(int iface_no);
int             tc_find_device_minor(int device_id, int minor_number);
int             get_local_ifaceno(PFBYTE ip_addr);
int             tc_find_iface_minor(int iface_type, int minor_number);
int             tc_iface_type(int iface_no);

#if (INCLUDE_ROUTER)
PIFACE          tc_get_route(PFBYTE ip_addr);
#endif
PIFACE          tc_get_nxt_bc_iface(int iface);
PIFACE          tc_get_src_ip(PFBYTE src_ip_addr, PFBYTE dest_ip_addr);

int             tc_interface_open(int device_id, int minor_number,
                              IOADDRESS io_address, int irq_val, word mem_address);
RTIP_BOOLEAN    tc_interface_mcast(int iface_no, PFBYTE mclist,
                               int nipaddrs, int add);
int             tc_interface_send(PIFACE pi, DCU msg);
int             tc_do_interface_send(PIFACE pi, DCU msg);
int             tc_netwrite(PIFACE pi, PANYPORT pport,DCU msg,PFBYTE ipn,int flags,word wait_count);
RTIP_BOOLEAN    check_process_output(PIFACE pi);
RTIP_BOOLEAN    tc_interface_stats(int iface_no);
int             signal_ip_via_dcu(PIFACE pi, int flag);
int             _interface_write(PIFACE pi, DCU msg);
void            xmit_done_timer(void);
int             mcast_write_loop(DCU msg);

/* File IGMP.C:   */
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
void            tc_igmp_interpret(PIFACE pi, DCU msg);
void            igmp_timer(void);
#endif

/* File IP.C:   */
void        tc_ip_interpret(PIFACE pi);
DCU         ip_alloc_packet(PIFACE pi, PANYPORT port, int nbytes, int frag_off, int who);
#if (INCLUDE_IPV4)
void        setup_ipv4_header(DCU msg, PANYPORT port, word len, word ip_frag, RTIP_BOOLEAN more_frag, int ttl, RTIP_BOOLEAN set_ip_seq);
#endif
RTIP_BOOLEAN   is_frag_not_first(DCU msg);
RTIP_BOOLEAN   is_frag(DCU msg);
#if (INCLUDE_802_2)
RTIP_BOOLEAN   msg_is_802_2(DCU msg);
void           setup_802_2_header(DCU msg, word len);
#endif
#if (INCLUDE_FRAG)
DCU         ipf_add(PIFACE pi, DCU msg);
DCU         ipf_join(int table_entry);
int         ipf_extract_data(DCU msg, PFBYTE buffer, int buflen, int protocol_hlen,
                             RTIP_BOOLEAN save_ip_hdr);
DCU         ipf_create_pkt(PIFACE pi, PANYPORT port, DCU msg, int buflen, int hlen);
DCU         ipf_fill_pkt(PIFACE pi, DCU msg, PFBYTE buffer, int buflen, int hlen);
int         ipf_send_udp_pkt(PIFACE pi, PFBYTE to, PUDPPORT pport, DCU msg,
                     word total_dlen, word wait_count, int dcu_flags);
word        ipf_tcp_tlen(DCU msg, int hlen);
int         ipf_netwrite(PIFACE pi, PANYPORT port, DCU msg, PFBYTE ipn,
                         word flags, word wait_count);
void        ipf_timer(void);
#endif
#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
int            ip_set_option(PANYPORT port, int option_name, PFCCHAR option_value);
#endif
RTIP_BOOLEAN   is_net_broadcast(PIFACE pi, PFBYTE ipn);
PIFACE         tc_get_local_pi(PFBYTE ip_addr);
RTIP_BOOLEAN   is_local_mcast(PIFACE pi, PFBYTE ipn);

/* FTP routines needed by NAT   */
#if (INCLUDE_FTP_SRV || INCLUDE_NAT)
void format_pasv_resp(PFCHAR buffer, PFBYTE ip_src, word port);
#endif
#if (INCLUDE_FTP_CLI || INCLUDE_NAT)
word parse_pasv_response(PFBYTE ip_addr, PFCHAR buf, int buflen);
#endif

/* NAT.C   */
RTIP_BOOLEAN    is_address_nat_internal(PFBYTE ip_addr);
RTIP_BOOLEAN    is_address_nat_unique(PFBYTE ip_addr);
void            nat_modify_outgoing(DCU msg);
void            nat_modify_incoming(DCU msg);

/* File PACKET.C:   */

/* File OS.C:   */
void        os_exchange_bind(word sig_handle,  PFVOID pobject);
void        os_exchange_send(word sig_handle,  PFVOID pobject, DCU msg);
DCU         os_exchange_rcv(word sig_handle,   PFVOID pobject, word wait_count);
void        os_exchange_clear(word sig_handle, PFVOID pobject);
void        os_lock_task(void);
void        os_unlock_task(void);
void        os_lock_task(void);
void        os_unlock_task(void);
void        os_set_sent_signal(PANYPORT port, RTIP_BOOLEAN success);
void        os_set_write_signal(PANYPORT port, RTIP_BOOLEAN success);
void        os_set_read_signal(PANYPORT port, RTIP_BOOLEAN success);
void        os_set_select_signal(PANYPORT port, int select_off);
DCU         os_rcvx_input_list(PIFACE pi);
void        os_sndx_input_list(PIFACE pi, DCU msg);
RTIP_BOOLEAN os_memory_init(RTIP_BOOLEAN restart);
#if (INCLUDE_MALLOC_DCU_INIT || INCLUDE_MALLOC_PORTS || BUILD_NEW_BINARY)
void        os_memory_free(void);
#endif
POS_LIST    os_list_add_front_off(POS_LIST head, POS_LIST entry, int offset);
POS_LIST    os_list_add_rear_off(POS_LIST head, POS_LIST entry, int offset);
POS_LIST    os_list_add_middle_off(POS_LIST head, POS_LIST entry,
                                   POS_LIST entry_prev, int offset);
POS_LIST    os_list_remove_off(POS_LIST head, POS_LIST entry, int offset);
POS_LIST    os_list_next_entry_off(POS_LIST head, POS_LIST entry, int offset);
POS_LIST    os_list_last_off(POS_LIST head, int offset);

void        os_track_allocation_stats_dump(void);

#if (INCLUDE_NO_DCU_BLOCK)
int         cnt_num_avail_pkts(int nbytes);
#endif
DCU         os_alloc_packet(int nbytes, int who);
DCU         os_alloc_packet_wait(int nbytes, PANYPORT port, int who);
DCU         os_alloc_packet_input(int nbytes, int who);
void        os_free_packet(DCU msg);
PFBYTE      dcu_alloc_core(int size);
void        dcu_free_core(PFBYTE p);
PUDPPORT    os_alloc_udpport(void);
void        os_free_udpport(PUDPPORT port);
PTCPPORT    os_alloc_tcpport(void);
void        os_free_tcpport(PTCPPORT port);
PFBYTE      os_alloc_tcpwind(void);
void        os_free_tcpwind(PFBYTE wind);
void        os_clear_udpapp_exchg(PUDPPORT pport);
void        os_sndx_udpapp_exchg(PUDPPORT pport, DCU msg);
DCU         os_rcvx_udpapp_exchg(PUDPPORT pport, word wait_count, RTIP_BOOLEAN delete_from_queue);
RTIP_BOOLEAN    os_udp_pkt_avail(PUDPPORT pport);
int         os_udp_first_pkt_size(PUDPPORT pport);
void        os_sndx_arpcache_list(int index, DCU msg);
DCU         os_rcvx_arpcache_list(int index);
void        os_rmvx_arpcache_list(int index, DCU msg);

#if (INCLUDE_TRK_PKTS)
void            cnt_pos_list(int *cnt, DCU msg, DCU root_msg, PTRACK_DCU pdcu_track_info);
void            display_sem_info(void);
void            display_xmit_info(void);
#endif      /* INCLUDE_TRK_PKTS */
void            display_packet_lowwater(void);

/* File OSPORT.C   */
void        ks_invoke_input(PIFACE pi,DCU msg);
void        ks_invoke_output(PIFACE pi, int xmit_complete_cnt);
void        ks_invoke_interrupt(PIFACE pi);

/* File OSTASK.C   */
void        ks_kernel_timer_callback(void);

/* File OSENV.C                                    */
/* See the RTIP/RTFS common section below for more */

/* ROUTE.C   */
#if (INCLUDE_ROUTING_TABLE)
PROUTE      rt_get(PFCBYTE dest_ip_addr);
int         rt_add_from_iface(PIFACE pi);
int         rt_del_from_iface(PIFACE pi);
void        rt_del_iface(int iface_no);
int         rt_del(PFBYTE net, PFCBYTE mask);
void        rt_redirect(PIFACE pi, PFBYTE src_ip_addr, PFBYTE orig_dest_ip_addr, PFBYTE new_gw_addr);
#if (INCLUDE_RT_LOCK)
void        rt_free(PROUTE prt);
#endif
#if (INCLUDE_RT_TTL)
void        rt_timer(void);
#endif
#endif      /* INCLUDE_ROUTING_TABLE */
/* ROUTING TABLE ROUTINES WHICH EXIST EVEN IF ROUTING TABLE DOES NOT EXIST   */
void        rt_init(void);
int         rt_add(PFCBYTE net, PFCBYTE mask, PFCBYTE gw, dword metric,
                   int iface, int ttl, int tag, int snmp_proto);
PIFACE      rt_get_iface_lock(PFBYTE ip_addr, PFBYTE send_addr, PANYPORT port, 
                         RTIP_BOOLEAN *is_gw);
PIFACE      rt_get_iface(PFBYTE ip_addr, PFBYTE send_addr, PANYPORT port, RTIP_BOOLEAN *is_gw);
int         rt_get_output(PFBYTE dest_ip_addr, PFBYTE send_addr, PANYPORT port, RTIP_BOOLEAN *is_gw);

/* ROUTER.C   */
void route_pkg(PIFACE pi, DCU msg, PIPPKT pip);

/* File RS232.C   */
void            rs232_xmit_init(PRS232_IF_INFO pif_info, DCU msg, RTIP_BOOLEAN ether_encap);
RTIP_BOOLEAN    rs232_xmit_uart(PRS232_IF_INFO pif_info);
RTIP_BOOLEAN    rs232_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
void            rs232_raw_mode(PIFACE pi, RTIP_BOOLEAN raw_on);
void            rs232_give_string(int minor_number, PFBYTE buffer, int n);
RTIP_BOOLEAN    rs232_init(PIFACE pi);
void            rs232_signal_event(int minor_device_number, int event);
void            rs232_close(PIFACE pi);
void            rs232_error(int channel, int error);
void            rs232_signal_event(int minor_device_number, int event);

/* File RTIPAPI.C   */
#if (INCLUDE_MODEM)
void        buffer_char(char chr, PRS232_IF_INFO pif_info);
#endif
DCU         SOCKAPI_ xn_pkt_alloc(int nbytes, RTIP_BOOLEAN is_802_2);
int         SOCKAPI_ xn_pkt_free(DCU msg);
int         SOCKAPI_ xn_pkt_data_max(int socket, PFBYTE dest_ip_addr);
PFBYTE      SOCKAPI_ xn_pkt_data_pointer(int socket, DCU msg, int type);
#if (INCLUDE_TOKEN || INCLUDE_802_2)
PFBYTE      SOCKAPI_ xn_pkt_data_pointer_iface(int iface_no, int socket, DCU msg, int type);
DCU         SOCKAPI_ xn_pkt_alloc_iface(int iface_no, int nbytes, RTIP_BOOLEAN is_802_2);
#endif
int         SOCKAPI_ xn_pkt_data_size(DCU msg, RTIP_BOOLEAN count_frags);
DCU         SOCKAPI_ xn_pkt_recv_from(int socket, PFBYTE from, PFINT from_port, unsigned int wait_count);
int         SOCKAPI_ xn_pkt_send_to(int socket, DCU msg, int len, PFBYTE to, int to_port, unsigned int wait_count);
int         pkt_data_size(DCU msg);

/* File TASKS.c   */
void        tc_ip_main(int iface_no);
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS)
void        tc_interrupt_main(int iface_no);
#endif      /* SLIP || CSLIP || INCLUDE_PPP */

/* timer functions   */
void        ebs_timeout(PTIMER timers);
void        ebs_set_timer(PTIMER t, int interval, RTIP_BOOLEAN is_sec);
void        ebs_start_timer(PTIMER t);
void        ebs_stop_timer(PTIMER timer);

/* File TCP.C:   */
#if (INCLUDE_TCP)
void        start_retrans_timer(PTCPPORT port);
RTIP_BOOLEAN     tc_tcp_timeout(RTIP_BOOLEAN do_to_proc, RTIP_BOOLEAN do_sec_proc);
void        fix_master(PTCPPORT mport);
void        trans_state(PTCPPORT port, int new_state, RTIP_BOOLEAN interpret, RTIP_BOOLEAN free_in_win);
void        tc_tcp_abort_connection(PTCPPORT port, int signal_flags, int errno_val, RTIP_BOOLEAN interpret);
void        wake_up(PTCPPORT port, RTIP_BOOLEAN interpret, int signal_flags,
                    int errno_val);
PTCPPORT    tc_find_port(DCU msg);
void        tc_tcp_interpret(PIFACE pi, DCU msg);
void        tc_tcp_output(void);
DCU         get_new_out_pkt(PTCPPORT port, byte tcp_flags_to_send, int size);
void        add_tcpport_list(PTCPPORT port, int list_type);
void        delete_tcpport_list(PTCPPORT port);
RTIP_BOOLEAN tcpinit(PTCPPORT port, PFBYTE ip_dest);
int         enqueue_out(PTCPPORT port, PFBYTE buffer, int nbytes, RTIP_BOOLEAN *pkt_err);
int         enqueue_pkt_out(PIFACE pi, PTCPPORT port, DCU msg);
int         enqueue(PWINDOW wind, DCU msg, int nbytes);
DCU         dequeue_pkt(PWINDOW wind, int flags);
int         dequeue(PWINDOW wind, PFBYTE buffer, int nbytes, int flags);
int         rmqueue(PWINDOW wind, int nbytes);
void        setup_both_windows(PTCPPORT port);
void        free_both_windows(PTCPPORT port, RTIP_BOOLEAN free_in);
RTIP_BOOLEAN     tc_is_write_state(PTCPPORT port);
RTIP_BOOLEAN     tc_is_read_state(PTCPPORT port);
void        setup_ws(PWINDOW w, word wsize, int io_type);
void        tcp_invalidate_sockets(PFBYTE ip_addr);
int         tcp_pkt_data_max(PTCPPORT pport);
int         tcp_pkt_data_size(DCU msg, RTIP_BOOLEAN count_frags);

/* File TCPOUT.C:   */
void        calc_tcp_options_len(PTCPPORT port, byte tcp_flags_to_send);
int         tc_tcpsend(PIFACE pi, PTCPPORT port, int dcu_flags,
                       int msg_type, byte tcp_flags);
/* File TCPUSER.C:   */
PTCPPORT    tc_tcp_sock_alloc(RTIP_BOOLEAN sem_claimed);
int         tc_tcp_bind(PTCPPORT port, PFBYTE ip_addr, word port_number, RTIP_BOOLEAN sem_claimed);
int         tc_tcp_listen(PTCPPORT port, int backlogsize, RTIP_BOOLEAN sem_claimed);
PTCPPORT    tc_find_accept_port(PTCPPORT mport, RTIP_BOOLEAN take_off_flag);
PTCPPORT    tc_tcp_accept(PTCPPORT port);
int         tc_tcp_connect(PTCPPORT port,  PFBYTE to, word to_port);
RTIP_BOOLEAN tc_send_syn(PTCPPORT port, int dcu_flags, byte tcp_flags);
void        tc_tcp_pk_peer_address(PTCPPORT port, PFBYTE host_ip, PFINT host_port);
int         tc_tcp_read(PTCPPORT port, PFBYTE buf, int n, DCU KS_FAR *pmsg, int flags, word wait_count);
int         tc_tcp_write(PTCPPORT port, PFBYTE buf, int n, DCU msg, int flags, word wait_count);
void        tcp_abort(PTCPPORT port, RTIP_BOOLEAN tcp_send_reset);
void        tc_tcp_abort(PTCPPORT port, RTIP_BOOLEAN tcp_send_reset);
int         tcp_close(PTCPPORT port);
int         tc_tcp_close(PTCPPORT port);
void        delete_tcpport_master_list(PTCPPORT port);
void        free_tcpport_avail(PTCPPORT port, RTIP_BOOLEAN sem_claimed);
#endif      /* INCLUDE_TCP */

/* File TOOLS.C:   */
PANYPORT    sock_to_port(int socket_index);
#if (INCLUDE_ERROR_CHECKING)
PANYPORT    api_sock_to_port(int socket_index);
#endif
word        tc_ip_chksum(PFWORD pw, int len);
word        ipf_icmp_chksum(DCU msg, int len);
word        tc_udp_chksum(DCU msg);
word        tc_tcp_chksum(DCU msg);
#if (KS_LITTLE_ENDIAN)      /* see macros in osenv.h for big endian */
#if (!INLINE_SUPPORT)
word        net2hs(word w);
word        hs2net(word w);
#endif
dword       net2hl(dword l);
dword       hl2net(dword l);
#endif                      /* KS_LITTLE_ENDIAN */
RTIP_BOOLEAN     do_read_select(int socket, long wait);
RTIP_BOOLEAN     do_write_select(int socket_no, long wait);
RTIP_BOOLEAN     set_non_blocking_mode(int sock);

#if (INCLUDE_TCP_TIMESTAMP || INCLUDE_SNMP || INCLUDE_NFS_CLI || INCLUDE_NFS_SRV || INCLUDE_RIP)
dword       byte_to_long(PFDWORD pd);
void        long_to_bytes(PFDWORD pd, dword d);
#endif


#if (!KS_USE_ASCII_LIB)
PFCHAR      tc_strrchr(PFCCHAR str, int find_chr);
int         tc_isprint(int c);
#if (INCLUDE_FTP_SRV || INCLUDE_TFTP_CLI || INCLUDE_NFS_CLI || INCLUDE_NFS_SRV)
int         tc_toupper(int c);
#endif
#if (INCLUDE_TFTP_SRV)
int         tc_isupper(int c);
int         tc_tolower(int c);
#endif
#endif      /* KS_USE_ASCII_LIB */
#if (INCLUDE_POP3 || INCLUDE_MODEM || INCLUDE_WEB_BROWSER || INCLUDE_IMAP)
PFCHAR      tc_stristr(PFCHAR str, PFCCHAR find_str);
#endif
char *      xn_strdup(KS_CONSTANT char *str, int ID);

RTIP_BOOLEAN    ip_bin_to_str(PFCHAR string, PFCBYTE n);
RTIP_BOOLEAN    ip_str_to_bin(PFBYTE address, PFCCHAR string);
dword           calc_sys_up_time(void);

/* File: UART.C   */
int         uart_send(int index, PFBYTE pb, int tot_to_send);
void        uart_close(int  index);
void        uart_receive(int minor_number);
RTIP_BOOLEAN uart_init(
    int         minor_number,       /* minor number (index into uart_info_arry) */
    PIFACE      rs232_pi,           /* Pointer to the device driver. */
    char        handshake_type,     /* (N)one (D)tr/Dsr (R)ts/Cts */
    int         baud_rate,          /* Baud rate - see above */
    int         comm_port,          /* Comm Port (1-4) */
    byte        framing_char,       /* PPP Framing Character */
    PFBYTE      input_buffer,       /* Pre-allocated buffer       (optional) */
    int         input_buffer_len,   /* Pre-allocated buffer size  (optional) */
    PFBYTE      output_buffer,      /* Pre-allocated buffer       (optional) */
    int         output_buffer_len   /* Pre-allocated buffer size  (optional) */
    );
void        uart_raw_mode(int index, RTIP_BOOLEAN raw_on);
void        uart_setouttrigger(PRS232_IF_INFO pif_info, int trigger);

/* File UDP.C:   */
#if (INCLUDE_UDP)
int         tc_udp_connect(PUDPPORT pport, PFBYTE to, word to_port);
int         tc_udp_bind(PUDPPORT pu_port, PFCBYTE ip_addr, word port_number);
PUDPPORT    tc_udp_sock_alloc(int type, int protocol);
void        udp_close(PUDPPORT pu_port);
void        tc_udp_close(PUDPPORT pu_port);
void        udp_find_port(PIFACE pi, PUDPPORT *port, PUDPPORT *listener_port, DCU msg);
void        tc_udp_interpret(PIFACE pi, DCU msg);
RTIP_BOOLEAN tc_raw_interpret(PIFACE pi, DCU msg);
int         tc_udp_pk_send(PIFACE pi, PFBYTE send_to_addr, PUDPPORT pport,
                           DCU msg, word pkt_len, word total_pkt_len,
                           word wait_count, int dcu_flags);
int         tc_udp_get_iface(PUDPPORT pport, PFBYTE to,
                             PIFACE KS_FAR *ppi, PFBYTE send_to_addr);
int         tc_udp_set_header(PUDPPORT pport, DCU msg, PFBYTE to, word to_port,
                              PIFACE pi, RTIP_BOOLEAN set_prot_hdr, int data_len,
                              int total_data_len);
void        tc_udp_pk_peer_address(DCU msg, PFBYTE host_ip, PFINT host_port);
#if (INCLUDE_UDP_IFACE_CACHE && INCLUDE_SYNCHRONIZE_UDP_IFACE_CACHE)
void        clear_udp_iface_cache(void);
#endif
#endif
void        udp_invalidate_sockets(PFBYTE ip_addr);

/* File MOUSE.C   */
#if (INCLUDE_MOUSE)
typedef     void (*MOUSE_UPDATE_FN)(int);

void        mouse_set_update(MOUSE_UPDATE_FN fn, int arg);
void        mouse_reset(void);
int         mouse_init(int comm_port);
void        mouse_poll_state(PFWORD x_pos, PFWORD y_pos, PFBYTE buttons);
#endif

/* *********************************************************************   */
/* SYSLOG stuff                                                            */
/* *********************************************************************   */

/*
 * priorities (these are ordered)
 */
#define LOG_EMERG     0 /* system is unusable */
#define LOG_ALERT     1 /* action must be taken immediately */
#define LOG_CRIT      2 /* critical conditions */
#define LOG_ERR       3 /* error conditions */
#define LOG_WARNING   4 /* warning conditions */
#define LOG_NOTICE    5 /* normal but significant condition */
#define LOG_INFO      6 /* informational */
#define LOG_DEBUG     7 /* debug-level messages */

#define LOG_PRIMASK 0x07    /* mask to extract priority part (internal) */
                                  /* extract priority   */
#define LOG_PRI(p)  ((p) & LOG_PRIMASK)

/* facility codes   */
#define LOG_KERN        (0<<3)  /* kernel messages */
#define LOG_USER        (1<<3)  /* random user-level messages */
#define LOG_MAIL        (2<<3)  /* mail system */
#define LOG_DAEMON      (3<<3)  /* system daemons */
#define LOG_AUTH        (4<<3)  /* security/authorization messages */
#define LOG_SYSLOG      (5<<3)  /* messages generated internally by syslogd */
#define LOG_LPR         (6<<3)  /* line printer subsystem */
#define LOG_NEWS        (7<<3)  /* network news subsystem */
#define LOG_UUCP        (8<<3)  /* UUCP subsystem */
#define LOG_CRON        (9<<3)  /* clock daemon */
#define LOG_AUTHPRIV    (10<<3) /* security/authorization messages (private) */
#define LOG_FTP         (11<<3) /* ftp daemon */

/* other codes through 15 reserved for system use   */
#define LOG_LOCAL0    (16<<3)   /* reserved for local use */
#define LOG_LOCAL1    (17<<3)   /* reserved for local use */
#define LOG_LOCAL2    (18<<3)   /* reserved for local use */
#define LOG_LOCAL3    (19<<3)   /* reserved for local use */
#define LOG_LOCAL4    (20<<3)   /* reserved for local use */
#define LOG_LOCAL5    (21<<3)   /* reserved for local use */
#define LOG_LOCAL6    (22<<3)   /* reserved for local use */
#define LOG_LOCAL7    (23<<3)   /* reserved for local use */

#define LOG_CUSTOM_FAC(fac)      (((fac) << 3) & LOG_FACMASK) /* create your own non-standard facility codes */

#define LOG_NFACILITIES       24    /* current number of facilities */
#define LOG_FACMASK       0x03f8    /* mask to extract facility part */
                                    /* facility of pri   */
#define LOG_FAC(p)         (((p) & LOG_FACMASK) /* >> 3 */)

#define LOG_MAKEPRI(fac, pri)   (((fac) & LOG_FACMASK) | ((pri) & LOG_PRIMASK))

/*
 * arguments to setlogmask.
 */
#define LOG_MASK(pri)   (1 << LOG_PRI(pri))             /* mask for one priority */
#define LOG_UPTO(pri)   ((1 << (LOG_PRI(pri)+1)) - 1)   /* all priorities through pri */

/*
 * Option flags for openlog.
 *
 * LOG_ODELAY does nothing.
 * LOG_NDELAY does nothing.
 */
#define LOG_PID         0x0001  /* log the pid with each message */
#define LOG_CONS        0x0002  /* log on the console if errors in sending */
#define LOG_ODELAY      0x0004  /* delay open until first syslog() (default) */
#define LOG_NDELAY      0x0008  /* don't delay open */
#define LOG_NOWAIT      0x0010  /* don't wait for console forks: DEPRECATED */
#define LOG_PERROR      0x0020  /* log to stderr as well */

#define LOG_LOCALFILE   0x0100  /* log to local file as well */
#define LOG_REMOTE      0x0200  /* log to remote syslog deamon as well (default) */
#define LOG_MULTILINE   0x0400  /* keep newlines in the message: will end up as multiple lines in the logfile - bad style though! */
#define LOG_SERIALPORT  0x0800  /* log to local serial port */
#define LOG_DEVNULL     0x1000  /* log to /dev/nul (bit bucket). Sounds like 'nonsense' to you, but this is handy to prevent syslog()
                                 * form logging to CONSOLE if no other outputs have been selected or syslog output failed. */

#define LOG_ALL_OUTPUTS  (LOG_CONS | LOG_PERROR | LOG_LOCALFILE | LOG_REMOTE | LOG_SERIALPORT | LOG_DEVNULL)

#define XD_SYSLOG_SERVERPORT           514 /* syslog/udp */ /* getservbyname("syslog", "udp")->s_port */

#if (INCLUDE_SYSLOG)

typedef struct xn_syslog_code
{
    const char *c_name;
    int c_val;
} XNSYSLOG_CODE;

extern XNSYSLOG_CODE *xd_sl_prioritynames;

extern XNSYSLOG_CODE *xd_sl_facilitynames;

/* tc_vzprintf:I/O method for syslogging...   */
int         xn_write2syslog(struct tc_printf_io *io, KS_CONSTANT char *msg, int msglen);

void        xn_vsyslog(int priority_and_facility, const char *msg, va_list args);
void        xn_syslog(int priority, const char *message, ...);
void        xn_openlog
                      (const char *ident
                      ,int logopt
                      ,int facility
                      ,const char *remote_servername
                      ,int remote_serverport
                      ,int reconnect_after_count_msgs
                      ,const char *local_logfilename
                      ,int close_logfile_after_count_msgs
                      ,int serial_comm_port
                      ,long int serial_baud_rate
                      );
void        xn_closelog(void);
int         xn_setlogmask(int maskpri);

/* internally used   */
void        xnsl_add_taskindex_to_blockinglist(int taskindex);
void        xnsl_lock_blockinglist(RTIP_BOOLEAN flag);

#endif /* INCLUDE_SYSLOG */

/* *********************************************************************   */
/* API MACROS                                                              */
/* *********************************************************************   */
/* These macros implement functions documented in the API layer            */
#define     xn_pkt_recv(socket, wait_count)             \
        xn_pkt_recv_from((socket), (PFBYTE)0, (PFINT)0, (wait_count))
#define     xn_pkt_send(socket, msg, len, wait_count)   \
        xn_pkt_send_to((socket), (msg), (len), (PFBYTE)0, 0, (wait_count) )

#ifdef __cplusplus
}
#endif

#endif      /* INCLUDE_RTIP */

#if (INCLUDE_RTIP)
/* *********************************************************************   */
/* ******     MISC stuff which needs to be after os.h       ************   */
/* *********************************************************************   */

/* define strings used at RTIP_API_ENTER() beginning of API routines   */
/* whose main purpose is to step state machine for POLLOS              */
#if (INCLUDE_API_STR)
    #define API_XN_RARP             xn_rarp_name
    #define API_XN_BOOTP            xn_bootp_name
    #define API_XN_PING             xn_ping_name
    #define API_XN_SET_IP           xn_set_ip_name
    #define API_XN_ATTACH           xn_attach_name
    #define API_XN_RT_ADD           xn_rt_add_name
    #define API_XN_RT_DEL           xn_rt_del_name
    #define API_XN_CYCLE_GW         xn_cycle_gw
    #define API_XN_ABORT            xn_abort_name
    #define API_XN_PKT_ALLOC        xn_pkt_alloc_name
    #define API_XN_PKT_DATA_MAX     xn_pkt_data_max_name
    #define API_XN_DATA_POINTER     xn_data_pointer_name
    #define API_XN_PKT_DATA_SIZE    xn_pkt_data_size_name
    #define API_XN_PKT_RECV         xn_pkt_recv_name
    #define API_XN_PKT_SEND         xn_pkt_send_name
    #define API_XN_INTERFACE_OPEN   xn_interface_open_name
    #define API_XN_INTERFACE_OPEN_CONFIG xn_interface_open_config_name
    #define API_XN_INTERFACE_OPT    xn_interface_opt_name
    #define API_XN_PKT_FREE         xn_pkt_free_name
    #define API_XN_TCP_IS_CONNECT   xn_tcp_is_connect_name
    #define API_XN_TCP_IS_READ      xn_tcp_is_read_name
    #define API_XN_TCP_IS_WRITE     xn_tcp_is_write_name
    #define API_XN_GETLASTERROR     xn_getlasterror_name
    #define API_XN_GETERROR_STRING  xn_geterror_string_name
    #define API_XN_ARP_SEND         xn_arp_send_name
    #define API_XN_ARP_ADD          xn_arp_add_name
    #define API_XN_ARP_DEL          xn_arp_del_name
    #define API_SOCKET              socket_name
    #define API_LISTEN              listen_name
    #define API_CONNECT             connect_name
    #define API_BIND                bind_name
    #define API_ACCEPT              accept_name
    #define API_GETSOCKOPT          getsockopt_name
    #define API_SETSOCKOPT          setsockopt_name
    #define API_IOCTLSOCKET         ioctlsocket_name
    #define API_SELECT              select_name
    #define API_SHUTDOWN            shutdown_name
    #define API_CLOSESOCKET         closesocket_name
    #define API_RECV                recv_name
    #define API_SEND                send_name
    #define API_GETPEERNAME         getpeername_name
    #define API_GETSOCKNAME         getsockname_name

    #define RTIP_API_ENTER(X)   {DEBUG_LOG(X, LEVEL_3, NOVAR, 0, 0);    \
                                 KS_API_ENTER()}
    #define RTIP_API_EXIT(X)    {}
#else
    #define API_XN_RARP             0
    #define API_XN_BOOTP            0
    #define API_XN_PING             0
    #define API_XN_SET_IP           0
    #define API_XN_ATTACH           0
    #define API_XN_RT_ADD           0
    #define API_XN_RT_DEL           0
    #define API_XN_CYCLE_GW         0
    #define API_XN_ABORT            0
    #define API_XN_PKT_ALLOC        0
    #define API_XN_PKT_DATA_MAX     0
    #define API_XN_DATA_POINTER     0
    #define API_XN_PKT_DATA_SIZE    0
    #define API_XN_PKT_RECV         0
    #define API_XN_PKT_SEND         0
    #define API_XN_INTERFACE_OPEN   0
    #define API_XN_INTERFACE_OPT    0
    #define API_XN_PKT_FREE         0
    #define API_SOCKET              0
    #define API_XN_ARP_SEND         0
    #define API_LISTEN              0
    #define API_CONNECT             0
    #define API_BIND                0
    #define API_ACCEPT              0
    #define API_GETSOCKOPT          0
    #define API_SETSOCKOPT          0
    #define API_IOCTLSOCKET         0
    #define API_SELECT              0
    #define API_SHUTDOWN            0
    #define API_CLOSESOCKET         0
    #define API_RECV                0
    #define API_SEND                0
    #define API_GETPEERNAME         0
    #define API_GETSOCKNAME         0

    #define RTIP_API_ENTER(X)   {KS_API_ENTER()}
    #define RTIP_API_EXIT(X)    {}

#endif
#endif      /* INCLUDE_RTIP */

#endif /* __RTIP__ */


