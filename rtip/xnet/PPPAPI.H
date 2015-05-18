/*                                                                      */
/* PPPAPI.H - defines for PPP                                           */
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

#ifndef _PPPAPI_H
#define _PPPAPI_H 1

/* *********************************************************************   */
/* ******                CONFIGURATION                            ******   */
/* *********************************************************************   */
#if (INCLUDE_RUN_TIME_CONFIG)
typedef struct cfg_ppp_data
{
    word lcp_mru_hi;                /* High MRU limit */
    word lcp_mru_lo;                /* Lower MRU limit */
    int  pap_namelen;
    int  pap_pwdlen;
    int  user_rec_len;
    int  pap_users;
    int  lcp_req_try;           /* REQ attempts before giving up */
    int  lcp_nak_try;           /* NAK attempts before drop option */
    int  lcp_term_try;          /* TERM REQ attempts before giving up */
    int  lcp_timeout;           /* Seconds to wait for response before
                                   retrying  */
    int  pap_req_try;           /* # attempts from us LCP shutdown */
    int  pap_fail_max;          /* # attempts from remote before  */
    int  pap_timeout;           /* Seconds to wait for response before
                                   retrying  */
    int  ipcp_req_try;          /* REQ attempts before giving up */
    int  ipcp_nak_try;          /* NAK attempts before REJECT */
    int  ipcp_term_try;         /* TERM REQ attempts before giving up */
    int  ipcp_timeout;          /* Seconds to wait for response to request
                                   before retrying */
    int  chap_req_try;          /* # times to retry a CHAP challenge before
                                   retrying  */
    int  chap_timeout;          /* Seconds to wait for response before
                                   retrying challange */
} CFG_PPP_DATA;

extern struct cfg_ppp_data cfg_ppp_data;

#define CFG_LCP_ACCM_DFLT   _CFG_LCP_ACCM_DFLT
#define CFG_LCP_MRU_DFLT    _CFG_LCP_MRU_DFLT
#define CFG_LCP_MRU_HI      cfg_ppp_data.lcp_mru_hi
#define CFG_LCP_MRU_LO      cfg_ppp_data.lcp_mru_lo
#define CFG_PAP_NAMELEN     _CFG_PAP_NAMELEN
#define CFG_PAP_PWDLEN      _CFG_PAP_PWDLEN
#define CFG_USER_REC_LEN    _CFG_USER_REC_LEN
#define CFG_PAP_USERS       _CFG_PAP_USERS
#define CFG_LCP_REQ_TRY     cfg_ppp_data.lcp_req_try
#define CFG_LCP_NAK_TRY     cfg_ppp_data.lcp_nak_try
#define CFG_LCP_TERM_TRY    cfg_ppp_data.lcp_term_try
#define CFG_LCP_TIMEOUT     cfg_ppp_data.lcp_timeout
#define CFG_PAP_REQ_TRY     cfg_ppp_data.pap_req_try
#define CFG_PAP_FAIL_MAX    cfg_ppp_data.pap_fail_max
#define CFG_PAP_TIMEOUT     cfg_ppp_data.pap_timeout
#define CFG_IPCP_REQ_TRY    cfg_ppp_data.ipcp_req_try
#define CFG_IPCP_NAK_TRY    cfg_ppp_data.ipcp_nak_try
#define CFG_IPCP_TERM_TRY   cfg_ppp_data.ipcp_term_try
#define CFG_IPCP_TIMEOUT    cfg_ppp_data.ipcp_timeout
#define CFG_CHAP_REQ_TRY    cfg_ppp_data.chap_req_try
#define CFG_CHAP_TIMEOUT    cfg_ppp_data.chap_timeout
#define CFG_CHAP_NAMELEN    _CFG_CHAP_NAMELEN
#define CFG_CHAP_VALLEN     _CFG_CHAP_VALLEN
#define CFG_MAXSECRETLEN    _CFG_MAXSECRETLEN
#define CFG_CHAP_SECRETS    _CFG_CHAP_SECRETS 
#else
#define CFG_LCP_ACCM_DFLT   _CFG_LCP_ACCM_DFLT
#define CFG_LCP_MRU_DFLT    _CFG_LCP_MRU_DFLT
#define CFG_LCP_MRU_HI      _CFG_LCP_MRU_HI
#define CFG_LCP_MRU_LO      _CFG_LCP_MRU_LO
#define CFG_PAP_NAMELEN     _CFG_PAP_NAMELEN
#define CFG_PAP_PWDLEN      _CFG_PAP_PWDLEN
#define CFG_USER_REC_LEN    _CFG_USER_REC_LEN
#define CFG_PAP_USERS       _CFG_PAP_USERS
#define CFG_LCP_REQ_TRY     _CFG_LCP_REQ_TRY
#define CFG_LCP_NAK_TRY     _CFG_LCP_NAK_TRY
#define CFG_LCP_TERM_TRY    _CFG_LCP_TERM_TRY
#define CFG_LCP_TIMEOUT     _CFG_LCP_TIMEOUT
#define CFG_PAP_REQ_TRY     _CFG_PAP_REQ_TRY
#define CFG_PAP_FAIL_MAX    _CFG_PAP_FAIL_MAX
#define CFG_PAP_TIMEOUT     _CFG_PAP_TIMEOUT
#define CFG_IPCP_REQ_TRY    _CFG_IPCP_REQ_TRY
#define CFG_IPCP_NAK_TRY    _CFG_IPCP_NAK_TRY
#define CFG_IPCP_TERM_TRY   _CFG_IPCP_TERM_TRY
#define CFG_IPCP_TIMEOUT    _CFG_IPCP_TIMEOUT
#define CFG_CHAP_REQ_TRY    _CFG_CHAP_REQ_TRY
#define CFG_CHAP_TIMEOUT    _CFG_CHAP_TIMEOUT
#define CFG_CHAP_NAMELEN    _CFG_CHAP_NAMELEN
#define CFG_CHAP_VALLEN     _CFG_CHAP_VALLEN
#define CFG_MAXSECRETLEN    _CFG_MAXSECRETLEN
#define CFG_CHAP_SECRETS    _CFG_CHAP_SECRETS 
#endif

/* *********************************************************************   */
#ifndef MD5_DIGEST_LENGTH
#define MD5_DIGEST_LENGTH 16
#endif

/* *********************************************************************   */
/* ******        parameter values                                  *****   */
/* *********************************************************************   */

/* type of LCP OPEN   */
#define ACTIVE  1
#define PASSIVE 2

#define LOCAL_HOST  1
#define REMOTE_HOST 2

/* *********************************************************************   */
/* ******    LCP (Link Control Protocol)                           *****   */
/* *********************************************************************   */

/* LCP option types (see page 43 of RFC 1331)   */
#define LCP_MRU             0x01
#define LCP_ACCM            0x02
#define LCP_AUTHENT         0x03
#define LCP_ENCRYPT         0x04
#define LCP_MAGIC           0x05
#define LCP_QUALITY         0x06
#define LCP_PFC             0x07
#define LCP_ACFC            0x08

/* *********************************************************************   */
/* ******    IPCP (Internet Protocol Control Protocol - NCP)       *****   */
/* *********************************************************************   */

/* IPCP option types   */
#define IPCP_ADDRESSES      0x01
#define IPCP_COMPRESS       0x02
#define IPCP_ADDRESS        0x03
#define IPCP_DNS_PRIMARY_SH 0x04    /* shift value for flags */
#define IPCP_DNS_SECOND_SH  0x05    /* shift value for flags */
#define IPCP_DNS_PRIMARY     129
#define IPCP_DNS_SECOND      131

/* *********************************************************************   */
#ifdef __cplusplus
extern "C" {
#endif

/* In pppapi.c   */
int xn_lcp_set_option(int iface_no, int host, int option, RTIP_BOOLEAN turn_on);
int xn_ipcp_set_option(int iface_no, int host, int option, RTIP_BOOLEAN turn_on);
int xn_lcp_want_accm(int iface_no, int host, dword opt_val);
int xn_lcp_want_pap(int iface_no, int host);
int xn_lcp_want_chap(int iface_no, int host);
int xn_lcp_want_option(int iface_no, int host, int option, RTIP_BOOLEAN turn_on);
int xn_lcp_want_magic(int iface_no, dword local_seed, dword remote_seed);
int xn_lcp_want_mru(int iface_no, int host, int opt_val, RTIP_BOOLEAN negotiable);
int xn_ipcp_want_compress(int iface_no, int host, int num_slots);
int xn_ipcp_want_address(int iface_no, int host, PFCBYTE ip_address, RTIP_BOOLEAN negotiable);
int xn_ipcp_want_pool(int iface_no, PFBYTE ip_address_min, PFBYTE ip_address_max);
#if (INCLUDE_PPP_DNS)
int xn_ipcp_want_dns_address(int iface_no, int host, PFCBYTE dns1_addr, 
                             PFCBYTE dns2_addr, RTIP_BOOLEAN negotiable);
#endif
int xn_pap_user(int iface_no, PFCHAR user_name, PFCHAR pass_word);
int xn_pap_add_user(PFCHAR user_name, PFCHAR pass_word);
int xn_chap_clear_secret(void);
int xn_chap_add_secret(PFCHAR machine, PFBYTE secret, int secret_len);
int xn_chap_machine_name(PFCHAR machine);
int xn_lcp_echo(int iface_no, int wait_cnt, int size);
int xn_ppp_init(int iface_no, RTIP_BOOLEAN async_link);
int xn_ppp_poll_state(int iface_no, RTIP_BOOLEAN poll_close, RTIP_BOOLEAN poll_open);
int xn_lcp_open(int iface_no, int type, long close_wait_cnt, long open_wait_cnt);
int xn_ipcp_open(int iface_no);
int xn_ppp_close(int iface_no, int wait);
int xn_ppp_down(int iface_no);
int xn_ppp_wait_down(int iface_no, int wait_cnt);
int xn_bind_ppp(int minor_number);
int xn_bind_pppoe(int minor_number);

#ifdef __cplusplus
}
#endif

#endif  /* _PPPAPI_H */

