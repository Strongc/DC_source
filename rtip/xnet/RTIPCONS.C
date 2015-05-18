/* RTIPCONS.C - RTIP CONSTANT GLOBAL DATA                               */
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
/*      which is never modified.                                        */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS


#include "sock.h"
#include "rtip.h"

#if (INCLUDE_RTIP)
#include "base64.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************   */
/* RTIP Global data                                                       */

KS_GLOBAL_CONSTANT byte KS_FAR broadaddr[ETH_ALEN]  = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
KS_GLOBAL_CONSTANT byte KS_FAR nulladdr[ETH_ALEN]   = {0x0,  0x0,  0x0,  0x0,  0x0,  0x0};
KS_GLOBAL_CONSTANT byte KS_FAR ip_ffaddr[IP_ALEN]   = {0xff, 0xff, 0xff, 0xff};
KS_GLOBAL_CONSTANT byte KS_FAR ip_nulladdr[IP_ALEN] = {0, 0, 0, 0};
KS_GLOBAL_CONSTANT byte KS_FAR ip_lbmask[IP_ALEN]   = {0xff, 0, 0, 0};
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
/* all hosts group multicast IP address   */
KS_GLOBAL_CONSTANT byte KS_FAR ip_igmp_all_hosts[IP_ALEN]  = {0xe0, 0, 0, 1};
#if (INCLUDE_IGMP_V2)
/* all routers group multicast IP address   */
KS_GLOBAL_CONSTANT byte KS_FAR ip_igmp_all_routers[IP_ALEN] = {0xe0, 0, 0, 2};
#endif
#endif

#if (INCLUDE_BOOTP)
KS_GLOBAL_CONSTANT byte KS_FAR bootp_VM_RFC1048[5] = VM_RFC1048;
#endif

/* ********************************************************************   */
/* DEVICES                                                                */
/* ********************************************************************   */

KS_GLOBAL_CONSTANT EDEVTABLE default_device_entry =
{
    (DEVICE_OPEN)0, (DEVICE_CLOSE)0, (DEVICE_XMIT)0, (DEVICE_XMIT_DONE)0,
    (DEVICE_PROCESS_INTERRUPTS)0, (DEVICE_STATS)0, (DEVICE_SETMCAST)0,
    0, "device name", MINOR_0, ETHER_IFACE, 
    SNMP_DEVICE_INFO({MIB_OID}, 0)
    CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS, 
    CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT, 
    IOADD(0x300), EN(0x0), EN(5)
};

/* ********************************************************************   */
/* OSPORT DATA                                                            */
/* ********************************************************************   */

#if (INCLUDE_802_2)
/* ********************************************************************   */
/* 802.2 DATA                                                             */
/* ********************************************************************   */
KS_GLOBAL_CONSTANT byte llc_snap_data[LLC_SNAP_DATA_LEN] = {0xaa, 0xaa, 3, 0, 0, 0};
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
KS_GLOBAL_CONSTANT byte KS_FAR tcp_flags_state[14] =
{
/*  flags to send           state            state number   */
/*  -------------           -----            ------------   */
    NO_TCP_FLAGS,        /* dummy         =  0 (arrays start at 0) */
    NO_TCP_FLAGS,        /* TCP_S_CLOSED  =  1 */
    NO_TCP_FLAGS,        /* TCP_S_LISTEN  =  2 */
    NO_TCP_FLAGS,        /* TCP_S_TWAIT   =  3 (sometimes will send ACK) */
    TCP_F_SYN|TCP_F_ACK, /* TCP_S_SYNR    =  4 */
    TCP_F_SYN,           /* TCP_S_SYNS    =  5 */
    TCP_F_ACK,           /* TCP_S_EST     =  6 */
    TCP_F_FIN|TCP_F_ACK, /* TCP_S_FW1     =  7 */
    TCP_F_ACK,           /* TCP_S_FW2     =  8 */
    TCP_F_FIN|TCP_F_ACK, /* TCP_S_CLOSING =  9 */
    TCP_F_ACK,           /* TCP_S_CWAIT   = 10 */
    TCP_F_FIN|TCP_F_ACK, /* TCP_S_LAST    = 11 */
    NO_TCP_FLAGS,        /* TCP_S_ALLOCED = 12; not needed but just to */
                         /*                     be safe     */
    NO_TCP_FLAGS         /* TCP_S_FREE    = 13; not needed but just to */
                         /*                     be safe     */
};
#endif

/* ********************************************************************   */
/* UDP                                                                    */
/* ********************************************************************   */


#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_LOOP) 
/* ********************************************************************   */
/* LOOP, SLIP, CSLIP                                                      */
/* ********************************************************************   */
KS_GLOBAL_CONSTANT byte KS_FAR phony_en_addr[ETH_ALEN] = {0x1,0x2,0x3,0x4,0x5,0x6};
#endif

/* ********************************************************************   */
/* DRIVERS                                                                */
/* ********************************************************************   */

#if (!BUILD_NEW_BINARY)
#define DECLARING_CONSTANTS 1
#include "packet.c"
#if (GPL_DRIVERS)
#include "eepro595.c"
#endif
#undef DECLARING_CONSTANTS 
#endif  /* !BUILD_NEW_BINARY */

/* ********************************************************************   */
/* SPRINTF, DEBUG data                                                    */
/* ********************************************************************   */

KS_GLOBAL_CONSTANT char KS_FAR tc_hmap[17] = "0123456789ABCDEF";

/* ********************************************************************   */
/* API data                                                               */
/* ********************************************************************   */
#if (INCLUDE_MODEM)
    KS_GLOBAL_CONSTANT char KS_FAR *modem_off_line = "+++";
    KS_GLOBAL_CONSTANT char KS_FAR *modem_ath0 = "ATH0";
    KS_GLOBAL_CONSTANT char KS_FAR *modem_ok = "OK";
#endif

#if (INCLUDE_API_STR)
    KS_GLOBAL_CONSTANT char KS_FAR xn_rarp_name[8] =            "XN_RARP";
    KS_GLOBAL_CONSTANT char KS_FAR xn_bootp_name[9] =           "XN_BOOTP";
    KS_GLOBAL_CONSTANT char KS_FAR xn_ping_name[8] =            "XN_PING";
    KS_GLOBAL_CONSTANT char KS_FAR xn_set_ip_name[10] =         "XN_SET_IP";
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
    KS_GLOBAL_CONSTANT char KS_FAR xn_attach_name[10] =         "XN_ATTACH";
#endif
    KS_GLOBAL_CONSTANT char KS_FAR xn_rt_add_name[10] =         "XN_RT_ADD";
    KS_GLOBAL_CONSTANT char KS_FAR xn_rt_del_name[10] =         "XN_RT_DEL";
    KS_GLOBAL_CONSTANT char KS_FAR xn_cycle_gw[12] =            "XN_CYCLE_GW";
    KS_GLOBAL_CONSTANT char KS_FAR xn_abort_name[9] =           "XN_ABORT";
    KS_GLOBAL_CONSTANT char KS_FAR xn_pkt_alloc_name[13] =      "XN_PKT_ALLOC";
    KS_GLOBAL_CONSTANT char KS_FAR xn_pkt_data_max_name[16] =   "XN_PKT_DATA_MAX";
    KS_GLOBAL_CONSTANT char KS_FAR xn_data_pointer_name[16] =   "XN_DATA_POINTER";
    KS_GLOBAL_CONSTANT char KS_FAR xn_pkt_data_size_name[17] =  "XN_PKT_DATA_SIZE";
    KS_GLOBAL_CONSTANT char KS_FAR xn_pkt_recv_name[29] =       "XN_PKT_RECV/XN_PKT_RECV_FROM";
    KS_GLOBAL_CONSTANT char KS_FAR xn_pkt_send_name[27] =       "XN_PKT_SEND/XN_PKT_SEND_TO";
    KS_GLOBAL_CONSTANT char KS_FAR xn_interface_open_name[18] = "XN_INTERFACE_OPEN";
    KS_GLOBAL_CONSTANT char KS_FAR xn_interface_open_config_name[25] ="XN_INTERFACE_OPEN_CONFIG";
    KS_GLOBAL_CONSTANT char KS_FAR xn_interface_opt_name[17] =  "XN_INTERFACE_OPT";
    KS_GLOBAL_CONSTANT char KS_FAR xn_pkt_free_name[12] =       "XN_PKT_FREE";
    KS_GLOBAL_CONSTANT char KS_FAR xn_tcp_is_connect_name[18] = "XN_TCP_IS_CONNECT";
    KS_GLOBAL_CONSTANT char KS_FAR xn_tcp_is_read_name[15] =    "XN_TCP_IS_READ";
    KS_GLOBAL_CONSTANT char KS_FAR xn_tcp_is_write_name[16] =   "XN_TCP_IS_WRITE";
    KS_GLOBAL_CONSTANT char KS_FAR xn_getlasterror_name[16] =   "XN_GETLASTERROR";
    KS_GLOBAL_CONSTANT char KS_FAR xn_geterror_string_name[19]= "XN_GETERROR_STRING";
    KS_GLOBAL_CONSTANT char KS_FAR xn_arp_send_name[12]=        "XN_ARP_SEND";
    KS_GLOBAL_CONSTANT char KS_FAR xn_arp_add_name[11] =        "XN_ARP_ADD";
    KS_GLOBAL_CONSTANT char KS_FAR xn_arp_del_name[11] =        "XN_ARP_DEL";
    KS_GLOBAL_CONSTANT char KS_FAR socket_name[7] =             "SOCKET";
    KS_GLOBAL_CONSTANT char KS_FAR listen_name[7] =             "LISTEN";
    KS_GLOBAL_CONSTANT char KS_FAR connect_name[8] =            "CONNECT";
    KS_GLOBAL_CONSTANT char KS_FAR bind_name[5] =               "BIND";
    KS_GLOBAL_CONSTANT char KS_FAR accept_name[7] =             "ACCEPT";
    KS_GLOBAL_CONSTANT char KS_FAR getsockopt_name[11] =        "GETSOCKOPT";
    KS_GLOBAL_CONSTANT char KS_FAR setsockopt_name[11] =        "SETSOCKOPT";
    KS_GLOBAL_CONSTANT char KS_FAR ioctlsocket_name[12] =       "IOCTLSOCKET";
    KS_GLOBAL_CONSTANT char KS_FAR select_name[7] =             "SELECT";
    KS_GLOBAL_CONSTANT char KS_FAR shutdown_name[9] =           "SHUTDOWN";
    KS_GLOBAL_CONSTANT char KS_FAR closesocket_name[12] =       "CLOSESOCKET";
    KS_GLOBAL_CONSTANT char KS_FAR recv_name[14] =              "RECV/RECVFROM";
    KS_GLOBAL_CONSTANT char KS_FAR send_name[12] =              "SEND/SENDTO";
    KS_GLOBAL_CONSTANT char KS_FAR getpeername_name[12] =       "GETPEERNAME";
    KS_GLOBAL_CONSTANT char KS_FAR getsockname_name[12] =       "GETSOCKNAME";
#endif  

/* ********************************************************************   */
/* SOCKET DATA                                                            */
/* ********************************************************************   */

#if (INCLUDE_DB)
/* database for getservbyname() and getservbyport()   */
KS_GLOBAL_CONSTANT struct servent KS_FAR servs[NUM_SERVENT] =
{
    /*  name,       alias, port,    proto,   */
    {   "tcpmux",       0,  1,      "tcp"},
    {   "echo",         0,  7,      "tcp"},
    {   "echo",         0,  7,      "udp"},
    {   "discard",      0,  9,      "tcp"},
    {   "discard",      0,  9,      "udp"},
    {   "systat",       0,  11,     "tcp"},
    {   "daytime",      0,  13,     "tcp"},
    {   "daytime",      0,  13,     "udp"},
    {   "netstat",      0,  15,     "tcp"},
    {   "qotd",         0,  17,     "tcp"},
    {   "msp",          0,  18,     "tcp"},
    {   "msp",          0,  18,     "udp"},
    {   "chargen",      0,  19,     "tcp"},
    {   "chargen",      0,  19,     "udp"},
    {   "ftp-data",     0,  20,     "tcp"},
    {   "ftp",          0,  21,     "tcp"},
    {   "telnet",       0,  23,     "tcp"},
    {   "smtp",         0,  25,     "tcp"},
    {   "time",         0,  37,     "tcp"},
    {   "time",         0,  37,     "udp"},
    {   "rlp",          0,  39,     "udp"},
    {   "nameserver",   0,  42,     "tcp"},
    {   "whois",        0,  43,     "tcp"},
    {   "domain",       0,  53,     "tcp"},
    {   "domain",       0,  53,     "udp"},
    {   "mtp",          0,  57,     "tcp"},
    {   "bootps",       0,  67,     "tcp"},
    {   "bootps",       0,  67,     "udp"},
    {   "bootpc",       0,  68,     "tcp"},
    {   "bootpc",       0,  68,     "udp"},
    {   "tftp",         0,  69,     "udp"},
    {   "gopher",       0,  70,     "tcp"},
    {   "gopher",       0,  70,     "udp"},
    {   "rje",          0,  77,     "tcp"},
    {   "finger",       0,  79,     "tcp"},
    {   "www",          0,  80,     "tcp"},
    {   "www",          0,  80,     "udp"},
    {   "link",         0,  87,     "tcp"},
    {   "kerberos",     0,  88,     "tcp"},
    {   "kerberos",     0,  88,     "udp"},
    {   "supdup",       0,  95,     "tcp"},
    {   "hostnames",    0,  101,    "tcp"},
    {   "iso-tsap",     0,  102,    "tcp"},
    {   "csnet-ns",     0,  105,    "tcp"},
    {   "csnet-ns",     0,  105,    "udp"},
    {   "rtelnet",      0,  107,    "tcp"},
    {   "rtelnet",      0,  107,    "udp"},
    {   "pop2",         0,  109,    "tcp"},
    {   "pop2",         0,  109,    "udp"},
    {   "pop3",         0,  110,    "tcp"},
    {   "pop3",         0,  110,    "udp"},
    {   "sunrpc",       0,  111,    "tcp"},
    {   "sunrpc",       0,  111,    "udp"},
    {   "auth",         0,  113,    "tcp"},
    {   "sftp",         0,  115,    "tcp"},
    {   "uucp-path",    0,  117,    "tcp"},
    {   "nntp",         0,  119,    "tcp"},
    {   "ntp",          0,  123,    "tcp"},
    {   "ntp",          0,  123,    "udp"},
    {   "netbios-ns",   0,  137,    "tcp"},
    {   "netbios-ns",   0,  137,    "udp"},
    {   "netbios-dgm",  0,  138,    "tcp"},
    {   "netbios-dgm",  0,  138,    "udp"},
    {   "netbios-ssn",  0,  139,    "tcp"},
    {   "netbios-ssn",  0,  139,    "udp"},
    {   "imap2",        0,  143,    "tcp"},
    {   "imap2",        0,  143,    "udp"},
    {   "snmp",         0,  161,    "udp"},
    {   "snmp-trap",    0,  162,    "udp"},
    {   "cmip-man",     0,  163,    "tcp"},
    {   "cmip-man",     0,  163,    "udp"},
    {   "cmip-agent",   0,  164,    "tcp"},
    {   "cmip-agent",   0,  164,    "udp"},
    {   "xdmcp",        0,  177,    "tcp"},
    {   "xdmcp",        0,  177,    "udp"},
    {   "nextstep",     0,  178,    "tcp"},
    {   "nextstep",     0,  178,    "udp"},
    {   "bgp",          0,  179,    "tcp"},
    {   "bgp",          0,  179,    "udp"},
    {   "prospero",     0,  191,    "tcp"},
    {   "prospero",     0,  191,    "udp"},
    {   "irc",          0,  194,    "tcp"},
    {   "irc",          0,  194,    "udp"},
    {   "smux",         0,  199,    "tcp"},
    {   "smux",         0,  199,    "udp"},
    {   "at-rtmp",      0,  201,    "tcp"},
    {   "at-rtmp",      0,  201,    "udp"},
    {   "at-nbp",       0,  202,    "tcp"},
    {   "at-nbp",       0,  202,    "udp"},
    {   "at-echo",      0,  204,    "tcp"},
    {   "at-echo",      0,  204,    "udp"},
    {   "at-zis",       0,  206,    "tcp"},
    {   "at-zis",       0,  206,    "udp"},
    {   "z3950",        0,  210,    "tcp"},
    {   "z3950",        0,  210,    "udp"},
    {   "ipx",          0,  213,    "tcp"},
    {   "ipx",          0,  213,    "udp"},
    {   "imap3",        0,  220,    "tcp"},
    {   "imap3",        0,  220,    "udp"},
    {   "ulistserv",    0,  372,    "tcp"},
    {   "ulistserv",    0,  372,    "udp"},
    {   "exec",         0,  512,    "tcp"},
    {   "biff",         0,  512,    "udp"},
    {   "login",        0,  513,    "tcp"},
    {   "who",          0,  513,    "udp"},
    {   "shell",        0,  514,    "tcp"},
    {   "syslog",       0,  514,    "udp"},
    {   "printer",      0,  515,    "tcp"},
    {   "talk",         0,  517,    "udp"},
    {   "ntalk",        0,  518,    "udp"},
    {   "route",        0,  520,    "udp"},
    {   "timed",        0,  525,    "udp"},
    {   "tempo",        0,  526,    "tcp"},
    {   "courier",      0,  530,    "tcp"},
    {   "conference",   0,  531,    "tcp"},
    {   "netnews",      0,  532,    "tcp"},
    {   "netwall",      0,  533,    "udp"},
    {   "uucp",         0,  540,    "tcp"},
    {   "remotefs",     0,  556,    "tcp"},
    {   "klogin",       0,  543,    "tcp"},
    {   "kshell",       0,  544,    "tcp"},
    {   "kerberos-adm", 0,  749,    "tcp"},
    {   "webster",      0,  765,    "tcp"},
    {   "webster",      0,  765,    "udp"},
    {   "ingreslock",   0,  1524,   "tcp"},
    {   "ingreslock",   0,  1524,   "udp"},
    {   "prospero-np",  0,  1525,   "tcp"},
    {   "prospero-np",  0,  1525,   "udp"},
    {   "rfe",          0,  5002,   "tcp"},
    {   "rfe",          0,  5002,   "udp"},
    {   "krbupdate",    0,  760,    "tcp"},
    {   "kpasswd",      0,  761,    "tcp"},
    {   "eklogin",      0,  2105,   "tcp"},
    {   "supfilesrv",   0,  871,    "tcp"},
    {   "supfiledbg",   0,  1127,   "tcp"}
};      

/* database for getprotobynumber() and getprotobyname()   */
KS_GLOBAL_CONSTANT struct protoent KS_FAR protos[NUM_PROTOENT] =
{                             
    {   "ip",           0,      0,  0, "IP"},       
    {   "icmp",         0,      1,  0, "ICMP"},      
    {   "igmp",         0,      2,  0, "IGMP"}, 
    {   "ggp",          0,      3,  0, "GGP"},      
    {   "ipencap",      0,      4,  0, "IP-ENCAP"}, 
    {   "st",           0,      5,  0, "ST"},       
    {   "tcp",          0,      6,  0, "TCP"},       
    {   "egp",          0,      8,  0, "EGP"},       
    {   "pup",          0,      12, 0, "PUP"},      
    {   "udp",          0,      17, 0, "UDP"},      
    {   "hmp",          0,      20, 0, "HMP"},      
    {   "xns-idp",      0,      22, 0, "XNS-IDP"},
    {   "rdp",          0,      27, 0, "RDP"},      
    {   "iso-tp4",      0,      29, 0, "ISO-TP4"},       
    {   "xtp",          0,      36, 0, "XTP"},      
    {   "idpr-cmtp",    0,      39, 0, "IDPR-CMTP"},     
    {   "vmtp",         0,      81, 0, "VMTP"},     
    {   "ospf",         0,      89, 0, "OSPFIGP"},              
    {   "ipip",         0,      94, 0, "IPIP"},     
    {   "encap",        0,      98, 0, "ENCAP"}
};  
#endif

#if (INCLUDE_POP3 || INCLUDE_SMTP || INCLUDE_WEB || BUILD_BINARY)
/* ********************************************************************      */
/* POP or SMTP                                                               */
/* ********************************************************************      */
KS_GLOBAL_CONSTANT char KS_FAR *mime_term_field = "\r\n";

KS_GLOBAL_CONSTANT char KS_FAR *base64_alphabet = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
#endif

/* ********************************************************************   */
/* TOOLS.C                                                                */
/* ********************************************************************   */
#if (INCLUDE_ERRNO_STR)

    KS_GLOBAL_CONSTANT ERRNO_STRINGS KS_FAR error_strings[] = 
    {
        { EADDRNOTAVAIL,    "EADDRNOTAVAIL" },
        { EADDRINUSE,       "EADDRINUSE" },
        { EAFNOSUPPORT,     "EAFNOSUPPORT" },
        { EARPFULL,         "EARPFULL" },
        { EBADBAUD,         "EBADBAUD" },
        { EBADCOMMNUM,      "EBADCOMMNUM" },
        { EBADDEVICE,       "EBADDEVICE" },
        { EBADIFACE,        "EBADIFACE" },
        { EBADMASK,         "EBADMASK" },
        { EBADRESP,         "EBADRESP" },
        { ECONNREFUSED,     "ECONNREFUSED" },
        { EDESTADDREQ,      "EDESTADDREQ" },
        { EDESTUNREACH,     "EDESTUNREACH" },
        { EFAULT,           "EFAULT" },
        { EIFACECLOSED,     "EIFACECLOSED" },
        { EIFACEFULL,       "EIFACEFULL" },
        { EIFACEOPENFAIL,   "EIFACEOPENFAIL" },
        { EINPROGRESS,      "EINPROGRESS" },
        { EINVAL,           "EINVAL" },
        { EISCONN,          "EISCONN" },
        { EMCASTFULL,       "EMCASTFULL" },
        { EMCASTNOTFOUND,   "EMCASTNOTFOUND" },
        { EMFILE,           "EMFILE" },
        { ENETDOWN,         "ENETDOWN" },
        { ENETUNREACH,      "ENETUNREACH" },
        { ENOPKTS,          "ENOPKTS" },
        { ENOPROTOOPT,      "ENOPROTOOPT" },
        { ENOTCONN,         "ENOTCONN" },
        { ENOTINITIALIZED,  "ENOTINITIALIZED" },
        { ENOTSOCK,         "ENOTSOCK" },
        { ENUMDEVICE,       "ENUMDEVICE" },
        { EOPNOTSUPPORT,    "EOPNOTSUPPORT" },
        { EOUTPUTFULL,      "EOUTPUTFULL" },
        { EPROBEFAIL,       "EPROBEFAIL" },
        { ERENTRANT,        "ERENTRANT" },
        { ERTNOTFOUND,      "ERTNOTFOUND" },
        { ERTFULL,          "ERTFULL" },
        { ERSCINITFAIL,     "ERSCINITFAIL" },
        { ESHUTDOWN,        "ESHUTDOWN" },
        { ETIMEDOUT,        "ETIMEDOUT" },
        { ETNOSUPPORT,      "ETNOSUPPORT" },
        { EWOULDARP,        "EWOULDARP" },
        { EWOULDBLOCK,      "EWOULDBLOCK" },
        { ETOOSMALL,        "ETOOSMALL" },
        { EOUTAMEM,         "EOUTAMEM" },
        { ETABLEFULL,       "ETABLEFULL" },

        { EHTABLEFULL,      "EHTABLEFULL" },
        { EENTRYNOTFOUND,   "EENTRYNOTFOUND" },
        { ETOOMANYSERVERS,  "ETOOMANYSERVERS" },
        { ENO_RECOVERY,     "ENO_RECOVERY" },
        { ENAME_TOO_LONG,   "ENAME_TOO_LONG" },
        { ETRYAGAIN,        "ETRYAGAIN" },
        { ENODATA,          "ENODATA" },
        { ENORESPONSE,      "ENORESPONSE" },
        { ENOSERVERS,       "ENOSERVERS" },
#if (INCLUDE_MODEM)
        { EMODEMNORING,     "EMODEMNORING" },
        { EMODEMSENDFAILED, "EMODEMSENDFAILED" },
        { EMODEMBADRESP,    "EMODEMBADRESP" },
#endif
#if (INCLUDE_PPP)
        { EPPPFULL,         "EPPPFULL" },
        { EPPPNOTOPEN,      "EPPPNOTOPEN" },
        { EPPPLINEDOWN,     "EPPPLINEDOWN" },
        { EPPPNOLINKHDR,    "EPPPNOLINKHDR" },
        { EPPPBADPKT,       "EPPPBADPKT" },
        { EPPPTIMEDOUT,     "EPPPTIMEDOUT" },
        { EPPPNOTCLOSED,    "EPPPNOTCLOSED" },
#endif
#if (INCLUDE_SNMP)
        { ESNMPFOUND,       "ESNMPFOUND" },
        { ESNMPFULL,        "ESNMPFULL" },
        { ESNMPNOTFOUND,    "ESNMPNOTFOUND" },
        { ESNMPSEND,        "ESNMPSEND" },
        { ETOOSMALL,        "ETOOSMALL" },
#endif
#if (INCLUDE_ORTFS)
        { ERTFSINITFAIL,    "ERTFSINITFAIL" },
#endif
#if (INCLUDE_DHCP_CLI)
        { EDHCPSERVNORESP,  "EDHCPSERVNORESP" },
        { EDHCPSERVBADRESP, "EDHCPSERVBADRESP" },
        { EDHCPSERVNOOFFER, "EDHCPSERVNOOFFER" },
        { EDHCPSERVNOACK,   "EDHCPSERVNOACK" },
        { EDHCPSERVNOACKNAK,"EDHCPSERVNOACKNAK" },
        { EDHCPOPNOTFOUND,  "EDHCPOPNOTFOUND" },
        { EDHCPOPOVERFLOW,  "EDHCPOPOVERFLOW" },
        { EDHCPOPBADLEN,    "EDHCPOPBADLEN" },
        { EDHCPBADPARAM,    "EDHCPBADPARAM" },
#endif

#if (INCLUDE_NFS_SRV)
        { ENFSEXPORTFOUND,  "ENFSEXPORTFOUND" },
        { ENFSEXPORTFULL,   "ENFSEXPORTFULL" },
        { ENFSEXPORTNOTFOUND, "ENFSEXPORTNOTFOUND" },
        { ENFSNOBUFS,       "ENFSNOBUFS" },
        { ENFSNOPROC,       "ENFSNOPROC" },
        { ENFSNOPROG,       "ENFSNOPROG" },
        { ENFSPMAPFULL,     "ENFSPMAPFULL" },
        { ENFSREGERR,       "ENFSREGERR" },
        { ENFSUSERFOUND,    "ENFSUSERFOUND" },
        { ENFSUSERFULL,     "ENFSUSERFULL" },
        { ENFSUSERNOTFOUND, "ENFSUSERNOTFOUND" },
#endif

#if (INCLUDE_SMTP || INCLUDE_POP3)
        { ESRVBADRESP,      "ESRVBADRESP" },
        { ESRVDOWN ,        "ESRVDOWN" },
#endif

#if (INCLUDE_TFTP_CLI || INCLUDE_TFTP_SRV)
        { EREQNOTSUPPORT,   "EREQNOTSUPPORT" },
        { EBADREQ,          "EBADREQ" },
        { EMODENOTSUPPORT,  "EMODENOTSUPPORT" },
        { EIOERR,           "EIOERR" },
        { ESENDFAILED,      "ESENDFAILED" },
        { ECLIERROR,        "ECLIERROR" },
        { ETFTPERROR,       "ETFTPERROR" },
#endif

        { EFOPENFAILED,     "EFOPENFAILED" },
        { EFREADFAILED,     "EFREADFAILED" },
        { EFWRITEFAILED,    "EFWRITEFAILED" },
        { 0,   "" }                 /* END OF TABLE MARKER */
    };

/* Pointer to structure; this is done this way due to avoid some compilers   */
/* giving you an error if you extern an array without its size               */
KS_GLOBAL_CONSTANT ERRNO_STRINGS KS_FAR *error_strings_ptr = error_strings + RTIP_ERRNO;
#endif

PFCCHAR bad_errno_string = "WARNING: unknown errno or INCLUDE_ERRNO is not defined";

#endif      /* INCLUDE_RTIP */

/* ********************************************************************   */
/* TASKS                                                                  */
/* ********************************************************************   */
/* Class names. These are provided to assign names to tasks as they are
  spawned */
KS_GLOBAL_CONSTANT char KS_FAR class_names[TASKCLASS_EXTERN_APPTASK+1][16] = 
{
    "CURRENT",          /* illegal class */
    "TIMER",
    "IPTASK",
    "INTERRUPT",
    "FTPDAEMON",
    "WEBDAEMON",
    "HTTPSDAEMON",      /*** [ADD][19/Nov/2002][AIC]  ***/
    "TELDAEMON",
    "FTP_SERVER",
    "TFTPDAEMON",       /*** [ADD][19/Nov/2002][AIC]  ***/
    "WEB_SERVER",
    "HTTPS_SERVER",
    "TEL_SERVER",
    "NFS_SERVER",
    "SNMP_AGENT",
    "DHCP_SERVER",      /*** [ADD][19/Nov/2002][AIC]  ***/
    "MAIN_APP",
    "DEMO_APP",
    "USER_APP",
    "RIPDAEMON",        /*** [ADD][19/Nov/2002][AIC]  ***/
    "EXTERN"
};

#ifdef __cplusplus
};
#endif

