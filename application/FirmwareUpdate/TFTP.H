/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  from: @(#)tftp.h    5.4 (Berkeley) 4/3/91
 *  $Id: TFTP.H,v 1.1 2005/07/13 12:22:48 jla Exp $
 */

#ifndef _TFTP_H_
#define _TFTP_H_ 1

#include "vfile.h"
#include "netutil.h"

#define TFTP_SHOW_PROGRESS(STR)  DEBUG_ERROR(STR, NOVAR, 0, 0);

#define TFTP_CONTROL_PORT 69

#define u_short unsigned short

/* Trivial File Transfer Protocol (IEN-133)   */
#define SEGSIZE    512          /* data segment size */
#define PKTSIZE    (SEGSIZE+4)  /* data segment plus op code */

/* Packet types.   */
#define RRQ         01      /* read request */
#define WRQ         02      /* write request */
#define DATA        03      /* data packet */
#define ACK         04      /* acknowledgement */
#define TFTPERROR   05      /* error code */

struct  tftphdr 
{
    short   th_opcode;          /* packet type */
    union 
    {                       
        short   tu_block;       /* block # */
        short   tu_code;        /* error code */
        char    tu_stuff[1];    /* request packet stuff */
    } th_u;
    char    th_data[1];         /* data or error string */
};

#define th_block    th_u.tu_block
#define th_code     th_u.tu_code
#define th_stuff    th_u.tu_stuff
#define th_msg      th_data

/* Error codes (if change the number need to modify the array errmsgs).   */
#define EIOERROR    0       /* File read or write operation failed */
#define ENOTFOUND   1       /* file not found */
#define EACCESS     2       /* access violation */
#define ENOSPACE    3       /* disk full or allocation exceeded */
#define EBADOP      4       /* illegal TFTP operation */
#define EBADID      5       /* unknown transfer ID */
#define EEXISTS     6       /* file already exists */
#define ENOUSER     7       /* no such user */

struct errmsg 
{
    int     e_code;
    PFCCHAR e_msg;
};

/* ********************************************************************   */
struct formats 
{
    PFCCHAR f_mode;
    int (*f_validate)(char *filename, int mode);
    int (*f_send)(KS_CONSTANT struct formats KS_FAR *pf);
    int (*f_recv)(KS_CONSTANT struct formats KS_FAR *pf);
    int f_convert;
};

/* *********************************************************************   */
/* tftpsubs.c                                                              */

struct bf 
{
    int counter;            /* size of data in buffer, or flag */
    char buf[PKTSIZE];      /* room for data packet */
};

/* Values for bf.counter    */
#define BF_ALLOC -3             /* alloc'd but not yet filled */
#define BF_FREE  -2             /* free */
/* [-1 .. SEGSIZE] = size of data in the data buffer   */

/* *********************************************************************   */
/* SUPPORT ROUTINES                                                        */
int  readit(PIO_CONTEXT pio_context, struct tftphdr **dpp, int convert);
void read_ahead(PIO_CONTEXT pio_context, int  convert);
int  writeit(PIO_CONTEXT pio_context, struct tftphdr **dpp, int ct, int convert);
int  write_behind(PIO_CONTEXT pio_context, int convert);
int  timed_recv(int socket_no, int seconds, char *buf, int n);
int  timed_recvfrom(int socket_no, int seconds, char *buf, int n, int flags, 
                   PSOCKADDR from, int *fromlen); 
int  synch_net(int sock, char * buf, int n);

#endif /* !_TFTP_H_ */






