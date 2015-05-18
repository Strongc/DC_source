/* TFTPDATA.C - TFTP GLOBAL DATA                                        */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains all the global data for TFTP               */
/*      which is not constant.                                          */

#define DIAG_SECTION_KERNEL DIAG_SECTION_TFTP


#include "sock.h"

#if (INCLUDE_TFTP_CLI || INCLUDE_TFTP_SRV)
/* ********************************************************************   */
/* TFTP CLIENT AND SERVER                                                 */
/* ********************************************************************   */

#include "tftpapi.h"
#include "tftp.h"

/* ********************************************************************   */
/* CONFIGURATION DATA                                                     */
/* ********************************************************************   */
#if (INCLUDE_RUN_TIME_CONFIG)
struct cfg_tftp_data KS_FAR cfg_tftp_data =
{
    _CFG_TFTP_TIMEOUT
};
#endif


/* ********************************************************************   */
//TEST JSM
//char    KS_FAR acktftpbuf[PKTSIZE];
int KS_FAR acktftpbuf[PKTSIZE/4];
#if(PKTSIZE!=516)
  #error "Chech this code"
#endif

struct  sockaddr_in KS_FAR tftpd_s_in = { AF_INET };
int KS_FAR nextone;     /* index of next buffer to use */
int KS_FAR current;     /* index of buffer in use */

/* control flags for crlf conversions   */
int KS_FAR newline = 0;        /* fillbuf: in middle of newline expansion */
int KS_FAR prevchar = -1;      /* putbuf: previous char (cr check) */

struct bf KS_FAR bfs[2];
#endif      /* INCLUDE_TFTP_CLI or INCLUDE_TFTP_SRV */

#if (INCLUDE_TFTP_SRV)
/* ********************************************************************   */
/* TFTPD - TFTP SERVER                                                    */
/* ********************************************************************   */

IO_CONTEXT KS_FAR tftpd_io_context;     /* I/O context  */

/* socket connected to remote host to send/recv files   */
int KS_FAR tftpd_peer_sock;

char    KS_FAR tftpbuf[PKTSIZE];
struct sockaddr_in KS_FAR tftpd_from;
int KS_FAR tftpd_fromlen;
#endif      /* TFTP Server */


