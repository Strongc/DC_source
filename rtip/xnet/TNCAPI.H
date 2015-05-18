/*                                                                      */
/* TNCAPI.H - Defines & prototypes for Telnet client                    */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1998                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __TNCAPI_H__
#define __TNCAPI_H__

/* *********************************************************************   */
/* ******                CONFIGURATION                            ******   */
/* *********************************************************************   */
#if (INCLUDE_RUN_TIME_CONFIG)
typedef struct cfg_tnc_data
{
    int tnc_write_tmo;
} CFG_TNC_DATA;

extern CFG_TNC_DATA KS_FAR cfg_tnc_data;

#define CFG_TNC_INPKTLEN        _CFG_TNC_INPKTLEN
#define CFG_TNC_OUTPKTLEN       _CFG_TNC_OUTPKTLEN
#define CFG_TNC_MAXOPS          _CFG_TNC_MAXOPS
#define CFG_TNC_WRITE_TMO       cfg_tnc_data.tnc_write_tmo

#else   /* INCLUDE_RUN_TIME_CONFIG */
#define CFG_TNC_INPKTLEN        _CFG_TNC_INPKTLEN
#define CFG_TNC_OUTPKTLEN       _CFG_TNC_OUTPKTLEN
#define CFG_TNC_MAXOPS          _CFG_TNC_MAXOPS
#define CFG_TNC_WRITE_TMO       _CFG_TNC_WRITE_TMO
#endif


/* *********************************************************************   */
/* Default telnet port                                                     */
#define TELNET_SERVER_PORT          23

struct telnetc_config
{
    byte    remote_ip[4];               /* remote IP address of Telnet server */
    word    port;                       /* port to connect to 
                                           (TELNET_SERVER_PORT by default) */
    int     local_echo;                 /* 1 to echo characters locally, 
                                           0 to wait for remote side to
                                           send the characters back */
    int     vt100_emulation;            /* 1 to do VT100 terminal emulation.  
                                           0 for dumb terminal */
    int     naws;                       /* 1 to Negotiate About Window Size, 
                                           0 to use default window size. */
    int     width;                      /* Number of columns if naws is used. */
    int     height;                     /* Number of if naws is used. */
    void    (*connect_notify)(void);    /* Callback to notify user that client 
                                           has connected. */
    void    (*disconnect_notify)(void); /* Callback to notify user that 
                                           client has disconencted. */
};                      

/* *********************************************************************   */
/* API ROUTINES                                                            */
/* *********************************************************************   */

#ifdef __cplusplus
extern "C" {
#endif

int xn_telnet_client(struct telnetc_config *cfg);

#ifdef __cplusplus
}
#endif

#endif  /* __TNCAPI_H__ */

