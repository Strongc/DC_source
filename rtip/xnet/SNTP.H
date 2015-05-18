/*                                                                        */
/* SNTP.H  - SNTP header file                                             */
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
/*        declarations required to build SNTP,                            */
/*        Simple Network Time Protocol                                    */
/*                                                                        */
#ifndef __SNTP__
#define __SNTP__ 1

#define NTP_PORT 123    /* source and destination ports */

/* ********************************************************************   */
typedef struct ntp_time_format
{
    dword seconds;
    dword frac_sec;
} NTP_TIME_FORMAT;
typedef struct ntp_time_format KS_FAR * PNTP_TIME_FORMAT;

typedef struct ntp_pkt
{
/* mode bits   */
#define NTP_MODE_MASK 0x07
#define NTP_SYM_ACTIVE  1
#define NTP_SYM_PASSIVE 2
#define NTP_REQUEST     3
#define NTP_RESPONSE    4
#define NTP_BROADCAST   5

/* VN - version bits   */
#define NTP_VERSION_MASK    0x38

    byte    li_vn_mode; /* LI (2 bits), VN (3 bits), mode (3 bits) */
    byte    stratum;
    byte    poll;
    byte    precision;
    dword   root_delay;
    dword   root_dispersion;
    char    ref_id[4];                  /* ids the reference source such as */
                                        /* LOCL, PPS, WWVB, GPS, GOES etc.   */
                                        /* set by server:                    */
    NTP_TIME_FORMAT ref_timestamp;      /* reference timestamp */
                                        /* set by client   */
    NTP_TIME_FORMAT orig_timestamp;     /* origional timestamp */
                                        /* set by client   */
    NTP_TIME_FORMAT rcv_timestamp;      /* request received timestamp; */
                                        /* set by server   */
    NTP_TIME_FORMAT transmit_timestamp;/* time reply departed server; */
                                        /* set by server   */
/*  dword   key_id;                                        */
/*  byte    digest[128];                // optional ?      */
} NTP_PKT;
typedef struct ntp_pkt KS_FAR * PNTP_PKT;

/* ********************************************************************   */
/* data passed between timout routine                                     */
/* ********************************************************************   */
typedef struct sntp_timeout_data
{
    int             sntp_socket;
    byte            sntp_ip_addr[IP_ALEN];
    int             sntp_elapsed_sec;   /* ctr for delay between requests */
    int             sntp_select_timer;
    int             sntp_step_timeout;
    NTP_TIME_FORMAT current_timestamp;  /* local time */
    int             version;
    int             mode;               /* NTP_REQUEST or NTP_SYM_PASSIVE */
    dword           ticks_when_sent;
} SNTP_TIMEOUT_DATA;
typedef struct sntp_timeout_data KS_FAR * PSNTP_TIMEOUT_DATA;

/* ********************************************************************   */
/* enables using code as DLL                                              */
/* NOTE: the same define is in rtip.h                                     */
#define SOCKAPI_
/*#define SOCKAPI_  __stdcall __export   */

/* ********************************************************************   */
int sntp_request_time(PFBYTE ip_sntp_server, int *socket_no, 
                  PNTP_TIME_FORMAT current_timestamp,
                  int version, int mode, dword *ticks_when_sent,
                  PEBSTIME res_ebs_tod);
int sntp_process_result(int socket_no, PNTP_TIME_FORMAT current_timestamp,
                        int version, dword ticks_when_sent,
                        PEBSTIME res_ebs_tod);

#endif  /* __SNTP__ */

