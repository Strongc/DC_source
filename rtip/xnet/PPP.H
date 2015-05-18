/*                                                                      */
/* PPP.H - defines for PPP                                              */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

/*
 *  This implementation of PPP is declared to be in the public domain.
 *
 *  Jan 91  Bill_Simpson@um.cc.umich.edu
 *      Computer Systems Consulting Services
 *
 *  Acknowledgements and correction history may be found in PPP.C
 */

#ifndef _PPP_H
#define _PPP_H 1

#include "rtip.h"
#include "pppapi.h"

#if (INCLUDE_PPP_VANJC)
#include "slhc.h"
#endif
                           
#if (INCLUDE_PPP)

#if (INCLUDE_CHAP)
#if (INCLUDE_SSL_MD5)
#include "openssl/md5.h" /* MD5 for PPP-CHAP */
#elif (INCLUDE_RSA_MD5)
#include "md5rsa.h"
#endif
#endif

/* *********************************************************************   */
#define DEBUG_SIGNAL 0  /* set to print (DEBUG_ERROR) when signal xn_lcp_open */
#define DEBUG_PHASE  0  /* set to print transitions thru phases */

#define PRECEDENCE_PAP 0    /* negotiate PAP first */

/* *********************************************************************   */
/* ******    General Stuff                                         *****   */
/* *********************************************************************   */

/* structure assign   */
#define ASSIGN(a, b) tc_movebytes((PFBYTE)&(a), (PFBYTE)&(b), sizeof(b));

#define NULLCHAR '\0'
#define NULLCHARP (PFCHAR *)0

#if (INCLUDE_RUN_TIME_CONFIG)
#define PPP_CONSTANT
#else
#define PPP_CONSTANT KS_CONSTANT
#endif

/* See comment above KS_CONSTANT in osenv.h.    */
#if (INCLUDE_RUN_TIME_CONFIG)
#define PPP_CONSTANT
#define PPP_GLOBAL_CONSTANT
#define PPP_EXTERN_GLOBAL_CONSTANT  extern
#else
#define PPP_CONSTANT                KS_CONSTANT
#define PPP_GLOBAL_CONSTANT         KS_GLOBAL_CONSTANT
#define PPP_EXTERN_GLOBAL_CONSTANT  KS_EXTERN_GLOBAL_CONSTANT
#endif

/* *********************************************************************   */
/* ******    Special Char Stuff                                    *****   */
/* *********************************************************************   */
#define SP_CHAR             0x20

/* *********************************************************************   */
/* ******    PPP FRAME FORMAT                                      *****   */
/* *********************************************************************   */
/* Note: We define a structure name  ppp_hdr_alias in rtip.h.              */
/* If you change this definition you must change that one as well          */
 
/* PPP frame format (see pg 6 of RFC 1331)   */
struct ppp_hdr 
{                                
    #define HDLC_ALL_ADDR       0xff    /* HDLC all-station */
    byte   addr;

    #define HDLC_UI             0x03    /* HDLC Unnumbered Information */
    byte   control;

    #define PPP_IP_PROTOCOL     0x0021  /* Internet Protocol */
    #define PPP_COMPR_PROTOCOL  0x002d  /* Van Jacobson Compressed TCP/IP */
    #define PPP_UNCOMP_PROTOCOL 0x002f  /* Van Jacobson Uncompressed TCP/IP */
    #define PPP_IPCP_PROTOCOL   0x8021  /* Internet Protocol Control Protocol */
    #define PPP_LCP_PROTOCOL    0xc021  /* Link Control Protocol */
    #define PPP_PAP_PROTOCOL    0xc023  /* Password Authentication Protocol */
    #define PPP_CHAP_PROTOCOL   0xc223  /* Challenge-Handshake 
                                           Authentication Protocol */
    #define PPP_CCP_PROTOCOL    0x80FD  /* PPP Compression Control Protocol */
    word  protocol;
};
#define PPP_HDR_LEN 4           /* Max bytes for PPP/HDLC envelope header */
typedef struct ppp_hdr KS_FAR * PPPPHDR;

/* ******                     */
/* HDLC envelope constants    */
#define HDLC_ENVLEN 8           /* Max bytes for HDLC envelope (outgoing) */

#define HDLC_FLAG       0x7e    /* HDLC async start/stop flag */
#define HDLC_ESC_ASYNC  0x7d    /* HDLC transparency escape flag for async */
#define HDLC_ESC_COMPL  0x20    /* HDLC transparency bit complement mask */

#define HDLC_FCS_START  0xffff  /* Starting bit string for FCS calculation */
#define HDLC_FCS_FINAL  0xf0b8  /* FCS when summed over frame and sender FCS */


/* *********************************************************************   */
/* ******    LCP PACKET FORMAT (info field in PPP frame)           *****   */
/* *********************************************************************   */

/* config packet header (see pg 31 of RFC 1331)   */
typedef struct config_hdr 
{
    #define CONFIG_REQ      1
    #define CONFIG_CHAL     1       /* CHAP */
    #define CONFIG_ACK      2
    #define CONFIG_RESP     2       /* CHAP */
    #define CONFIG_NAK      3
    #define CONFIG_SUCC     3       /* CHAP */
    #define CONFIG_REJ      4
    #define CONFIG_FAIL     4       /* CHAP */
    #define TERM_REQ        5
    #define TERM_ACK        6
    #define CODE_REJ        7
    #define PROT_REJ        8
    #define ECHO_REQ        9
    #define ECHO_REPLY      10
    #define DISCARD_REQ     11
    #define QUALITY_REPORT  12
    #define ID_LCP          12
    byte   ccode;

    byte   id;
    word   len;
} CONFIG_HDR;
#define CONFIG_HDR_LEN  4   /* Length of config packet header */
typedef CONFIG_HDR KS_FAR * PCONFIG_HDR;
#define DCUTOCNFPKT(X)    ((PCONFIG_HDR) ((PPACKET)(X))->data)

#define OUT_PKT_HDR_LEN  (PPP_HDR_LEN + CONFIG_HDR_LEN)
#define IN_PKT_HDR_LEN   (CONFIG_HDR_LEN)

/* *********************************************************************   */
/* ******    OPTIONS FORMAT (options field in LCP packet)          *****   */
/* *********************************************************************   */

/* config option header (see page 43 of RFC 1331)   */
typedef struct option_hdr 
{
    byte   type;            /* protocol dependant types */
    byte   len;
} OPTION_HDR;
#define OPTION_HDR_LEN  2   /* Length of option header */
typedef struct option_hdr KS_FAR *POPTION_HDR;


/* *********************************************************************   */
/* ******    IPCP (Internet Protocol Control Protocol - NCP)       *****   */
/* *********************************************************************   */

/* option parameters are LCP_MRU, LCP_ACCM, LCP_AUTHENT, LCP_ENCRYPT,         */
/*                       LCP_MAGIC, LCP_QUALITY, LCP_PFC, LCP_ACFC            */


/* highest # we can handle (see IPCP_ADDRESSES etc in pppapi.h)    */
/* NOTE: disable options are handled by ipcp_option_supported()    */
#if (INCLUDE_PPP_DNS)
#define IPCP_OPTION_LIMIT   0x05    /* highest # we can handle */
#else
#define IPCP_OPTION_LIMIT   0x03    /* highest # we can handle */
#endif

/* Table for IPCP configuration requests                         */
/* One structure exists for local and one for remote information */
typedef struct ipcp_value_s 
{
    #define IPCP_N_ADDRESSES        (1 << IPCP_ADDRESSES)
    #define IPCP_N_COMPRESS         (1 << IPCP_COMPRESS)
    #define IPCP_N_ADDRESS          (1 << IPCP_ADDRESS)
    #define IPCP_ADDRESS_NEGOTIABLE 0x8000  /* used for will_negotiate only */
    #define IPCP_N_DNS_PRIMARY      (1 << IPCP_DNS_PRIMARY_SH)
    #define IPCP_N_DNS_SECOND       (1 << IPCP_DNS_SECOND_SH)
    #define IPCP_DNS_NEGOTIABLE     0x4000  /* used for will_negotiate only */
    word  negotiate;        /* negotiation flags */

    byte address[IP_ALEN];  /* local:  address for this side */
                            /* remote: address of other side   */
    byte other[IP_ALEN];    /* local:  address for other side */
                            /* remote: address of this side   */
#if (INCLUDE_PPP_DNS)
    byte dns1_addr[IP_ALEN];/* local:  DNS address for this side */
                            /* remote: DNS address of other side   */
    byte dns2_addr[IP_ALEN];/* local:  DNS address for this side */
                            /* remote: DNS address of other side   */
#endif  /* INCLUDE_PPP_DNS */

    word  compression;      /* Compression protocol */
    byte  slots;            /* Slots-actual number of slots (transmits slots-1)*/
    byte  slot_compress;    /* Slots may be compressed (flag)*/
} IPCP_VALUE_S;
typedef struct ipcp_value_s KS_FAR *PIPCPVALUES;

/* tbd - move to xnconf.h   */
#define IPCP_SLOT_DEFAULT   16      /* Default # of slots */
#define IPCP_SLOT_HI        64      /* Maximum # of slots */
#define IPCP_SLOT_LO         1      /* Minimum # of slots */
#define IPCP_SLOT_COMPRESS  0x01    /* May compress slot id */

typedef struct ipcp_side_s 
{
    word   will_negotiate;
    struct ipcp_value_s want;  /* want set by api calls */
    struct ipcp_value_s work;  /* working values during negotiation */
} IPCP_SIDE_S;
typedef struct ipcp_side_s KS_FAR *PIPCPSIDES;

/* IPCP control block - there will be one per interface   */
typedef struct ipcp_s 
{
    /* address pool   */
    byte peer_min[4];               /* first IP address */
    byte peer_max[4];               /* last IP address */
    byte peer_curr[4];              /* next address to be assigned */

    struct ipcp_side_s local_entry;
    struct ipcp_side_s remote_entry;

#if (INCLUDE_PPP_VANJC)
    struct slcompress KS_FAR *slhcp;    /* pointer to compression block */
#endif
} IPCP_S;
typedef struct ipcp_s KS_FAR *PIPCPS;


/* *********************************************************************   */
/* ******    LCP (Link Control Protocol)                           *****   */
/* *********************************************************************   */

/* highest # we can handle (see LCP_MRU etc in pppapi.h)   */
#define LCP_OPTION_LIMIT    0x08    

/* Table for LCP configuration requests   */
typedef struct lcp_value_s 
{
    #define LCP_N_MRU       (1 << LCP_MRU)
    #define LCP_N_ACCM      (1 << LCP_ACCM)
    #define LCP_N_AUTHENT   (1 << LCP_AUTHENT)
    #define LCP_N_ENCRYPT   (1 << LCP_ENCRYPT)
    #define LCP_N_MAGIC     (1 << LCP_MAGIC)
    #define LCP_N_QUALITY   (1 << LCP_QUALITY)
    #define LCP_N_PFC       (1 << LCP_PFC)
    #define LCP_N_ACFC      (1 << LCP_ACFC)
    #define LCP_CHAP_MD5_NAK_TRIED   0x8000 /* used for will_negotiate only */
                                        /* NAK tried once    */
    #define LCP_PAP_NAK_TRIED   0x4000  /* used for will_negotiate only */
                                        /* NAK tried once    */
    #define LCP_WANT_CHAP_MD5   0x2000  /* used for will_negotiate only */
                                        /* NAK tried once    */
    #define LCP_WANT_PAP        0x1000  /* used for will_negotiate only */
                                        /* NAK tried once    */
    #define LCP_MRU_NAK_TRIED   0x0800  /* used for will_negotiate only */
                                        /* NAK tried once (used for   */
                                        /* MRU is negotiable)         */
    #define LCP_MRU_NEGOTIABLE  0x0400  /* used for will_negotiate only */
                                        /* MRU is negotiable   */
    word  negotiate;            /* negotiation flags */

    word  mru;                  /* Maximum Receive Unit */
    dword accm;                 /* Async Control Char Map */
    word  authentication;       /* Authentication protocol */
#if (INCLUDE_CHAP)
    word  chap_algorithm;       /* only supports MD5 */
#endif
    word  encryption;           /* Encryption protocol */
    dword magic_number;         /* Magic number value */
    dword reporting_period;     /* Link Quality reporting period */
} LCP_VALUE_S;
typedef struct lcp_value_s KS_FAR *PLCPVALUES;

/*
 *  local_entry.will_negotiate: Options to accept in a NAK from remote.
 *          Initially set to default and modified by xn_xxx_set_option()
 *  local_entry.want:   Options to request.
 *          Initially set to default and modified by xn_xxx_want_xxx()
 *          Contains desired value.
 *          Only non-default options need to be negotiated.
 *  local_entry.work:   Options to request which are currently being negotiated.
 *          Value is valid only when negotiate bit is set.
 *          Contains desired value and values are used to send REQ.
 *
 *  remote_entry.will_negotiate:    Options to accept in a REQ from remote.
 *          Initially set to default and modified by xn_xxx_set_option()
 *  remote_entry.want:  Options to suggest by NAK if not present in REQ.
 *          Initially set to default and modified by xn_xxx_want_xxx()
 *          Contains desired value and values are used to send NAK.
 *  remote_entry.work:  Options currently being negotiated.
 *          Value is valid only when negotiate bit is set.
 */

typedef struct lcp_side_s 
{
    word    will_negotiate;
    struct lcp_value_s want;
    struct lcp_value_s work;
} LCP_SIDE_S;
typedef struct lcp_side_s KS_FAR *PLCPSIDES;

/* LCP control block   */
typedef struct lcp_s 
{
    struct lcp_side_s local_entry;
    struct lcp_side_s remote_entry;
} LCP_S;
typedef struct lcp_s KS_FAR *PLCPS;


/* *********************************************************************   */
/* ******    AUTHENTICATION (PAP and CHAP)                         *****   */
/* *********************************************************************   */

/* AUTH types   */
#define AUTH_PAP    0x01
#define AUTH_CHAP   0x02

/* *********************************************************************   */
/* ******    CHAP (Challenge Handshake Authentication Protocol)    *****   */
/* *********************************************************************   */
#define MD5_ALGORITHM 5     /* CHAP algorithm for LCP CHAP AUTH message */

/* CHAP control block   */
typedef struct chap_s 
{
    byte md5_value[CFG_CHAP_VALLEN];    /* value for sending challenge */
    byte md5_value_len;                 /* length of value field */
    int  algorithm;                     /* 5=MD5 (0-4 unused, reserved) */
    byte id;
} CHAPS;
typedef struct chap_s KS_FAR *PCHAPS;

/* secret info for CHAP challenges   */
typedef struct _chap_secrets
{
    char name[CFG_CHAP_NAMELEN+1];      /* machine name: for challenge  */
    byte secret[CFG_MAXSECRETLEN];      /* secret for challenge */
    int  secret_len;
} CHAP_SECRETS;

/* *********************************************************************   */
/* ******    PAP (Password Autentication Protocol)                 *****   */
/* *********************************************************************   */
#define PAP_FILE 0      /* pap info is in a file (not debugged) */

/* PAP control block   */
typedef struct pap_s 
{
    char username[CFG_PAP_NAMELEN];     /* Username for REQ */
    char password[CFG_PAP_PWDLEN];      /* Password for REQ */
} PAPS;
typedef struct pap_s KS_FAR *PPAPS;

#define PPP_ACCESS_PRIV 0x0100      /* Priv bit for PPP connection */
#define PPP_PWD_LOOKUP  0x0200      /* Priv bit for peer ID/pass lookup */

/* *********************************************************************   */
/* ******    STATE MACHINE and LINK PHASES                         *****   */
/* *********************************************************************   */

/* Supported Configuration Protocol index   */
enum 
{
    Lcp,            /* Link Control Protocol */
    Pap,            /* Password Authentication Protocol */
    Chap,           /* Challenge-Handshake Authentication Protocol */
    IPcp,
    fsmi_Size
};

/* forward declarations   */
#ifndef __PPP_TYPES
#define __PPP_TYPES
typedef struct ppp_s KS_FAR * PPPPS;    
#endif
typedef struct fsm_s KS_FAR * PFSMS;
                     

/* Protocol Constants needed by State Machine   */
struct fsm_constant_s 
{
    word   protocol;        /* Protocol number */
    word   recognize;       /* Config codes to use (bits) */

    byte   fsmi;            /* Finite State Machine index */
    byte   try_req;         /* # tries for request */
    byte   try_nak;         /* # tries for nak substitutes */
    byte   try_terminate;   /* # tries for terminate */
    int    timeout;         /* Time for timeouts (seconds)*/

    /* To free structure   */
    void (*free)        (PFSMS fsm_p);

    /* Set negotiation to initial values   */
    void (*reset)       (PFSMS fsm_p);
    /* When leaving Closed or Listen   */
    void (*starting)    (PFSMS fsm_p);
    /* When entering Opened   */
    void (*opening)     (PFSMS fsm_p);
    /* When leaving Opened   */
    void (*closing)     (PFSMS fsm_p);
    /* When entering Closed or Listen (after termination)   */
    void (*stopping)   (PFSMS fsm_p);

    DCU (*makereq)     (PFSMS fsm_p);

    int (*request)     (PFSMS fsm_p,
                        PCONFIG_HDR hdr,
                        DCU bp);
    int (*ack)         (PFSMS fsm_p,
                        PCONFIG_HDR hdr,
                        DCU bp);
    int (*nak)         (PFSMS fsm_p,
                        PCONFIG_HDR hdr,
                        DCU bp);
    int (*reject)      (PFSMS fsm_p,
                        PCONFIG_HDR hdr,
                        DCU bp);
};
typedef PPP_CONSTANT struct fsm_constant_s KS_FAR *PFSM_CONSTS;

/* FSM states (see page 16 of RFC 1331)   */
enum 
{
    fsmCLOSED,
    fsmLISTEN,
    fsmREQ_Sent,
    fsmACK_Rcvd,
    fsmACK_Sent,
    fsmOPENED,
    fsmTERM_Sent,
    fsmState_Size
};

#define CHECK_PPP_NOT_OPEN(PI)      \
    (!PI->edv || PI->edv->fsm[IPcp].ppp_state != fsmOPENED) 

/* ********************************************************************   */
#if (INCLUDE_POSTMESSAGE)
void ppp_trans_state(PFSMS fsm_s, byte new_state);
#define PPP_TRANS_STATE(FSM, NEW_STATE) ppp_trans_state(FSM, NEW_STATE)
#else
#define PPP_TRANS_STATE(FSM, NEW_STATE) (FSM)->ppp_state = (NEW_STATE)
#endif

/* State Machine Control Block   */
struct fsm_s 
{
    byte         ppp_state;         /* FSM state */
    byte         lastid;            /* ID of last REQ we sent */

    #define PPP_ESCAPED 0x01
    #define PPP_TOSS    0x02        /* discard PPP pkt reading */
    #define FSM_PASSIVE 0x40        /* opened passive */
    #define FSM_ACTIVE  0x80        /* opened active */
    /* NOTE: PPP_AP_LOCAL  (local authentication) and     */
    /*       PPP_AP_REMOTE (remote authentication) also   */
    byte         flags;

    byte         retry;             /* counter for timeouts */
    byte         try_req;           /* # tries for request */
    byte         try_terminate;     /* # tries for terminate */

    byte         retry_nak;         /* counter for naks of requests */
    byte         try_nak;           /* # tries for nak substitutes */

    PPPPS        ppp_p;             /* the ppp we belong to */
    EBS_TIMER    timer_info;        /* timer information */
    PFSM_CONSTS  pdc;               /* protocol dependent constants */
    void KS_FAR *pdv;               /* protocol dependent variables */
};                                  /* LCP  : struct lcp_s *  = PLCPS */
                                    /* IPCP : struct ipcp_s * = PIPCPS   */
                                    /* PAP  : struct pap_s *  = PPAPS    */
                                    /* CHAP : struct chap_s * = PCHAPS   */


/* Link Phases - (see page 10 or RFC 1331)   */
enum 
{
    pppDEAD,        /* Waiting for physical layer */
    pppOPEN,        /* Physical layer open - can accept input and xmit */
    pppLCP,         /* Link Control Phase */
    pppAP,          /* Authentication Phase */
    pppREADY,       /* Link ready for traffic (network phase), i.e. IPCP */
    pppTERMINATE,   /* Termination Phase */
    pppPhase_Size
};

#if (INCLUDE_POSTMESSAGE)
void ppp_trans_phase(PPPPS ppp_p, byte new_phase);
#define PPP_TRANS_PHASE(PPPPS, phase) ppp_trans_phase(PPPPS, phase)
#else
#define PPP_TRANS_PHASE(PPPPS, phase) (PPPPS)->ppp_phase = (phase)
#endif

/* PPP control block                                        */
/* NOTE: iface structure contains pointer to this structure */
struct ppp_s 
{
    PIFACE iface;                   /* pointer to interface block */
#if (INCLUDE_POSTMESSAGE)
    HWND       hwnd;
    MSGID      msgid;
#endif
    byte   ppp_phase;               /* phase of link initialization */
    byte   id;                      /* id counter for connection */

    #define PPP_AP_LOCAL    0x10    /* local authentication */
    #define PPP_AP_REMOTE   0x20    /* remote authentication */
    byte   flags;
      
    struct fsm_s fsm[fsmi_Size];    /* finite state machines */

    char peername[CFG_PAP_NAMELEN]; /* Peername from remote (if any) */

    dword OutTxOctetCount;          /* # octets sent */
    dword OutOpenFlag;              /* # of open flags sent */
    word  OutNCP[fsmi_Size];        /* # NCP packets sent by protocol */
    word  OutError;                 /* # packets with error on send */
    word  OutMemory;                /* # alloc failures on send */

    dword InRxOctetCount;           /* # octets received */
    dword InOpenFlag;               /* # of open flags */
    word  InNCP[fsmi_Size];         /* # NCP packets by protocol */
    word  InUnknown;                /* # unknown packets received */
    word  InChecksum;               /* # packets with bad checksum */
    word  InFrame;                  /* # packets with frame error */
    word  InError;                  /* # packets with other error */
    word  InMemory;                 /* # alloc failures */
};

/* *********************************************************************   */
/* ******    TRACK PPP PACKETS                                     *****   */
/* *********************************************************************   */
#if (INCLUDE_TRK_PPP)
typedef struct _trk_ppp_msg
{
    RTIP_BOOLEAN input;
    word    protocol;           /* c021=LCP etc */
    /* packet minus the 7e,ff,03,prot   */
    byte    packet[CFG_TRK_PPP_LEN];
    int     length;
} trk_ppp_msg;
#endif

/* *********************************************************************   */
/* ******    FUNCTION DECLARATIONS                                 *****   */
/* *********************************************************************   */

/* In ppp.c:   */
#if (DEBUG_SIGNAL)
void ppp_signal_set(void);
#endif
int  proc_fsm_close(PIFACE pi, int fsmi);
void ppp_init (PPPPS ppp_p, RTIP_BOOLEAN async_link);
int  ppp_free (PIFACE pi);
int  check_ppp_open(PIFACE pi);
#if (INCLUDE_TRK_PPP)
void display_track_ppp_info(void);
void track_ppp_reset(void);
#endif

/* pppfsm.c   */
int  ntohcnf(PCONFIG_HDR cnf, DCU msg);
int  ntohopt(POPTION_HDR opt, PFBYTE pb);
void fsm_no_action(PFSMS fsm_p);
int  fsm_no_check(PFSMS fsm_p, PCONFIG_HDR hdr, DCU bp);
void fsm_log(PFSMS fsm_p, PFCHAR comment);
int  fsm_passive(PFSMS fsm_p);
int  fsm_active(PFSMS fsm_p);
void fsm_timer(PFSMS fsm_p);
int  fsm_send(PFSMS fsm_p, byte ccode, byte id, DCU data);
int  fsm_sendreq(PFSMS fsm_p);
void fsm_reset (PFSMS fsm_p);
void fsm_proc(PIFACE pi, PFSMS fsm_p, DCU bp);
void fsm_start(PFSMS fsm_p);
void fsm_down(PFSMS fsm_p);
void fsm_close(PFSMS fsm_p);
void fsm_init(PFSMS fsm_p);
void fsm_free(PFSMS fsm_p);

/* in ppipcp.c   */
void    ipcp_init(PPPPS ppp_p);
RTIP_BOOLEAN ipcp_open(PPPPS ppp_p);

/* ppplcp.c   */
void    proc_lcp_echo_reply(PIFACE pi, DCU msg);
RTIP_BOOLEAN proc_lcp_echo_request(PFSMS fsm_p, DCU msg, byte id);
int     ppp_discard(PIFACE pi, DCU msg);
int     ppp_echo(PIFACE pi, word wait_count, int size);
void    ppp_ready(PPPPS ppp_p);
RTIP_BOOLEAN lcp_open(PPPPS ppp_p);
void    lcp_init(PPPPS ppp_p, RTIP_BOOLEAN async_link);

/* pppap.c   */
int     pap_local(PPPPS ppp_p);
int     pap_remote(PPPPS ppp_p);
void    pap_down(PFSMS fsm_p);
void    pap_proc(PFSMS fsm_p, DCU bp);
RTIP_BOOLEAN pap_open(PPPPS ppp_p);
void    pap_init(PPPPS ppp_p);

#if (INCLUDE_CHAP)
/* pppchap.c   */
int     chap_local(PPPPS ppp_p);
int     chap_remote(PPPPS ppp_p);
void    chap_down(PFSMS fsm_p);
void    chap_proc(PFSMS fsm_p, DCU bp, byte phase);
RTIP_BOOLEAN chap_open(PPPPS ppp_p);
void    chap_init(PPPPS ppp_p);
int     chap_add_secret(PFCHAR machine, PFBYTE secret, int secret_len);

#endif

/* memory allocation routines   */
PPPPS  os_alloc_ppp_cb(PIFACE pi);
void   os_free_ppp_cb(PPPPS ppp_p);
PIPCPS os_alloc_ipcp_cb(PIFACE pi);
void   os_free_ipcp_cb(PIPCPS ipcp_p);
PPAPS  os_alloc_pap_cb(PIFACE pi);
void   os_free_pap_cb(PPAPS pap_p);
#if (INCLUDE_CHAP)
PCHAPS os_alloc_chap_cb(PIFACE pi);
void   os_free_chap_cb(PCHAPS chap_p);
#endif
PLCPS  os_alloc_lcp_cb(PIFACE pi);
void   os_free_lcp_cb(PLCPS lcp_p);

/* in pppuser.c   */
#if (!PAP_FILE)
    void user_init(void);
    int pap_add_user(PFCHAR user_name, PFCHAR pass_word);
#endif
void userlookup(PFCHAR username, PFCHAR *password, PFCHAR *directory,
                PFINT permission, PFBYTE ip_address, PFCHAR buf);
void pap_pwdlookup(PPAPS pap_p);
int  userlogin(PFCHAR name, PFCHAR pass, PFCHAR path, int len, 
               PFINT pwdignore);



#endif  /* INCLUDE_PPP */
#endif  /* _PPP_H  */

