/*                                                                      */
/* TFTPAPI.H                                                            */
/* Copyright Peter Van Oudenaren , 1995                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*                                                                      */
/*  Module description:                                                 */
/*      This module provides an tftp API for client and server          */

#ifndef _TFTPAPI_H_
#define _TFTPAPI_H_ 1

/* *********************************************************************   */
/* ******                CONFIGURATION                            ******   */
/* *********************************************************************   */
#if (INCLUDE_RUN_TIME_CONFIG)
typedef struct cfg_tftp_data
{
    int tftp_timeout;
} CFG_TFTP_DATA;

extern CFG_TFTP_DATA KS_FAR cfg_tftp_data;

#define CFG_TFTP_TIMEOUT    cfg_tftp_data.tftp_timeout
#else
#define CFG_TFTP_TIMEOUT    _CFG_TFTP_TIMEOUT
#endif

/* *********************************************************************   */
/* ******                TFTP API                                 ******   */
/* *********************************************************************   */
#ifdef __cplusplus
extern "C" {
#endif

/* TFTP CLIENT API   */
int tftpcli_sendfile(int sock, char *localname, char *name, char *mode);
int tftpcli_recvfile(int sock,  /*char *localname,*/ char *name, char *mode);
int tftpcli_connect(unsigned long ip_address);

/* TFTP SERVER API   */
int tftp_server_daemon(void);

#ifdef __cplusplus
}
#endif

#endif /* _TFTPAPI_H_ */


