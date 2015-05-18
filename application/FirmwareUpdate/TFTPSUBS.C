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
 */

/* Simple minded read-ahead/write-behind subroutines for tftp user and
   server.  Written originally with multiple buffers in mind, but current
   implementation has two buffer logic wired in.

   Todo:  add some sort of final error check so when the write-buffer
   is finally flushed, the caller can detect if the disk filled up
   (or had an i/o error) and return a nak to the other side.

            Jim Guyton 10/85
 */

#define DIAG_SECTION_KERNEL DIAG_SECTION_TFTP


#include "sock.h"
#include "socket.h"

#if (INCLUDE_TFTP_CLI || INCLUDE_TFTP_SRV)
#include "tftp.h"

#include "MPCBasicFirmwareUpdater_C.h"

/* ********************************************************************   */
extern int KS_FAR nextone;     /* index of next buffer to use */
extern int KS_FAR current;     /* index of buffer in use */

/* control flags for crlf conversions   */
extern int KS_FAR newline;        /* fillbuf: in middle of newline expansion */
extern int KS_FAR prevchar;      /* putbuf: previous char (cr check) */
extern struct bf KS_FAR bfs[2];

/* ********************************************************************   */
struct tftphdr *rw_init(int x);

/* ********************************************************************   */
/* init for write-behind                                                  */
struct tftphdr *w_init(void)
{
    return rw_init(0);
}

/* init for read-ahead   */
struct tftphdr *r_init(void)
{
    return rw_init(1);
}

/* ********************************************************************   */
/* init for either read-ahead or write-behind                             */
/* zero for write-behind, one for read-head                               */
struct tftphdr *rw_init(int x) /*__fn__*/
{
    newline = 0;            /* init crlf flag */
    prevchar = -1;
    bfs[0].counter =  BF_ALLOC;     /* pass out the first buffer */
    current = 0;
    bfs[1].counter = BF_FREE;
    nextone = x;                    /* ahead or behind? */
    return (struct tftphdr *)bfs[0].buf;
}


/* ********************************************************************   */
/* Have emptied current buffer by sending to net and getting ack.
   Free it and return next buffer filled with data.
 */
int readit(PIO_CONTEXT pio_context, struct tftphdr **dpp, int convert)  /*__fn__ */
{
struct bf *b;

    bfs[current].counter = BF_FREE; /* free old one */
    current = !current;             /* "incr" current */

    b = &bfs[current];              /* look at new buffer */
    if (b->counter == BF_FREE)      /* if it's empty */
        read_ahead(pio_context, convert);      /* fill it */
    *dpp = (struct tftphdr *)b->buf;        /* set caller's ptr */
    return b->counter;
}

/* ********************************************************************   */
/*
 * fill the input buffer, doing ascii conversions if requested
 * conversions are  lf -> cr,lf  and cr -> cr, nul
 */
void read_ahead(PIO_CONTEXT pio_context, int  convert)  /*__fn__*/
{
struct bf *b;
struct tftphdr *dp;

    ARGSUSED_INT(convert);
    b = &bfs[nextone];              /* look at "next" buffer */
    if (b->counter != BF_FREE)      /* nop if not free */
        return;
    nextone = !nextone;             /* "incr" next buffer ptr */

    dp = (struct tftphdr *)b->buf;

        b->counter = (int)vf_read(pio_context->virtual_fd,
                                  (PFBYTE)&dp->th_data,
                                  SEGSIZE);
        return;
}

/* ********************************************************************   */
/* Update count associated with the buffer, get new buffer
   from the queue.  Calls write_behind only if next buffer not
   available.
*/
int writeit(PIO_CONTEXT pio_context, struct tftphdr **dpp, int ct, int convert)    /*__fn__*/
{
    bfs[current].counter = ct;      /* set size of data to write */
    current = !current;             /* switch to other buffer */
    if (bfs[current].counter != BF_FREE)     /* if not free */
        write_behind(pio_context, convert);     /* flush it */
    bfs[current].counter = BF_ALLOC;        /* mark as alloc'd */
    *dpp =  (struct tftphdr *)bfs[current].buf;
    return ct;                      /* this is a lie of course */
}

/* ********************************************************************   */
/*
 * Output a buffer to a file, converting from netascii if requested.
 * CR,NUL -> CR  and CR,LF => LF.
 * Note spec is undefined if we get CR as last byte of file or a
 * CR followed by anything else.  In this case we leave it alone.
 */
int write_behind(PIO_CONTEXT pio_context, int convert)         /*__fn__*/
{
char *buf;
int count;
struct bf *b;
struct tftphdr *dp;

    ARGSUSED_INT(convert);
    b = &bfs[nextone];
    if (b->counter < -1)            /* anything to flush? */
        return 0;               /* just nop if nothing to do */

    count = b->counter;             /* remember byte count */
    b->counter = BF_FREE;           /* reset flag */
    dp = (struct tftphdr *)b->buf;
    nextone = !nextone;             /* incr for next time */
    buf = dp->th_data;

    if (count <= 0)
        return -1;      /* nak logic? */
    //return((int)vf_write(pio_context->virtual_fd, (PFBYTE)buf, (word)count));

    return packetRecvCallback_C(buf, (word)count);

}


/* ********************************************************************   */
int timed_recv(int socket_no, int seconds, char *buf, int n)    /*__fn__*/
{
fd_set fd_read;
timeval tmo;

    tmo.tv_sec = seconds;
    tmo.tv_usec = 0;

    FD_ZERO(&fd_read);
    FD_SET(socket_no, &fd_read);

    if (select(1, (fd_set KS_FAR *)&fd_read, (fd_set KS_FAR *)0,
                   (fd_set KS_FAR *)0, (timeval KS_FAR *)&tmo) != 1)
    {
        return(-1);
    }
    else
        return(recv(socket_no, buf, n, 0));
}


/* ********************************************************************   */
int timed_recvfrom(int socket_no, int seconds, char *buf, int n, int flags, /*__fn__*/
                   PSOCKADDR from, int *fromlen) /*__fn__*/
{
fd_set fd_read;
timeval tmo;

    tmo.tv_sec = seconds;
    tmo.tv_usec = 0;

    FD_ZERO(&fd_read);
    FD_SET(socket_no, &fd_read);

    if (select(1, (fd_set KS_FAR *)&fd_read, (fd_set KS_FAR *)0,
                   (fd_set KS_FAR *)0, (timeval KS_FAR *)&tmo) != 1)
    {
        return(-1);
    }
    else
    {
        *fromlen = sizeof(* from);
        return(recvfrom(socket_no, buf, n, flags, from, fromlen));
    }
}

/* ********************************************************************   */
/* When an error has occurred, it is possible that the two sides
 * are out of synch.  i.e.: that what I think is the other side's
 * response to packet N is really their response to packet N-1.
 *
 * So, to try to prevent that, we flush all the input queued up
 * for us on the network connection on our host.
 *
 * We return the number of packets we flushed (mostly for reporting
 * when trace is active).
 */

int synch_net(int sock, char * buf, int n)             /*__fn__*/
{
struct sockaddr_in from;
int fromlen;
int j;

    j = 0;
    while (timed_recvfrom(sock, 0, buf, n, 0,
                         (PSOCKADDR)&from, &fromlen) > 0)
        j++;
    return(j);
}

#endif  /* INCLUDE_TFTP_CLI or INCLUDE_TFTP_SRV */
