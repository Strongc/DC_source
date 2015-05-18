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

/* Many bug fixes are from Jim Guyton <guyton@rand-unix>   */

/*
 * TFTP User Program -- Protocol Machines
 */

#define DIAG_SECTION_KERNEL DIAG_SECTION_TFTP


#include "rtip.h"
#include "sock.h"

#if (INCLUDE_TFTP_CLI)
#include "tftp.h"
#include "tftpapi.h"

#define TRACE_TFTP 0
#define DEBUG_TFTP 1

#if (TRACE_TFTP || DEBUG_TFTP)
static char KS_FAR display_buffer[80];
#define TFTP_DISPLAY_ERROR(X)  DEBUG_ERROR(X, NOVAR, 0, 0);
#else
#define TFTP_DISPLAY_ERROR(X)
#endif

struct tftphdr *r_init(void);
struct tftphdr *w_init(void);

extern struct sockaddr_in KS_FAR tftpd_s_in;         /* filled in by main */
extern char KS_FAR acktftpbuf[PKTSIZE];
KS_EXTERN_GLOBAL_CONSTANT struct errmsg KS_FAR errmsgs[9];
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR KS_FAR *netascii;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR file_system_error[18];

/* ********************************************************************   */
int makerequest(int request,char *name, char *mode, struct tftphdr *tp);
void tftpcli_nak(int f, int error);
void tpacket(char *s,struct tftphdr *tp, int n);

/* ********************************************************************      */
/* tftpcli_connect() - Connect to a remote host                              */
/*                                                                           */
/* Summary:                                                                  */
/*   int tftpcli_connect(ip_address)                                         */
/*      unsigned long ip_address - IP Address to connect to                  */
/*                                                                           */
/* Description:                                                              */
/*   Initializes a TFTP session by creating a UDP socket, binding the        */
/*   socket and setting up the remote IP address for subsequent sends        */
/*   and receives.  The remote address is ip_address and the remote          */
/*   port number is TFTP_CONTROL_PORT.                                       */
/*                                                                           */
/*   For more details see the FTP/TFTP Manual.                               */
/*                                                                           */
/* Returns:                                                                  */
/*   Returns 0 if successful or -1 upon error.                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP and FTP/TFTP Reference Manuals.                                    */

int tftpcli_connect(unsigned long ip_address)       /*__fn__*/
{
int sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        TFTP_DISPLAY_ERROR("tctpcli_connect: socket failed")
        return(-1);
    }
    tc_memset((char *)&tftpd_s_in, 0, sizeof (tftpd_s_in));
    tftpd_s_in.sin_family = AF_INET;
    if (bind(sock, (PCSOCKADDR)&tftpd_s_in, sizeof (tftpd_s_in)) < 0)
    {
        TFTP_DISPLAY_ERROR("tctpcli_connect: bind failed")
        return(-1);
    }

    /* sin_port contains the destination for send tos    */
    tftpd_s_in.sin_port = hs2net(TFTP_CONTROL_PORT);
#if (CFG_UNIX_COMPLIANT_SOCK_STRUCTS)
    tftpd_s_in.sin_addr.s_addr = ip_address;
#else
    tftpd_s_in.sin_addr = ip_address;
#endif

    return(sock);
}


/* ********************************************************************      */
/* RECEIVE A FILE                                                            */
/* ********************************************************************      */
/* tftpcli_recvfile() - Receive the requested file                           */
/*                                                                           */
/* Summary:                                                                  */
/*   int tftpcli_recvfile(sock localname, name, mode)                        */
/*     int sock         - the socket returned from tftpcli_connect()         */
/*     char *localname  - file to receive                                    */
/*     char *name       - the destination mode;                              */
/*                        set to netascii for asci mode if implemented       */
/*     char *mode       - xfer mode (ignored:all xfers are binary for now)   */
/*                                                                           */
/* Description:                                                              */
/*   Receive the file from the remote host connected to and                  */
/*   save it to file name specified by localname.                            */
/*                                                                           */
/*   NOTE: this routine is non-reentrant and cannot be called if tftp        */
/*         server is running                                                 */
/*                                                                           */
/*   For more details see the FTP/TFTP Manual.                               */
/*                                                                           */
/* Returns:                                                                  */
/*   Returns 0 if successful or -1 upon error.                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP and FTP/TFTP Reference Manuals.                                    */

int tftpcli_recvfile(int sock, /*char *localname,*/ char *name, char *mode)                    /*__fn__*/
{
IO_CONTEXT         tftp_io_context;     /* I/O context for control socket */
struct tftphdr *   ap;
struct tftphdr *   dp;
int                block = 1;
int                n, size;
struct sockaddr_in from;
int                fromlen, firsttrip = 1;
int                convert;              /* true if converting crlf -> lf */
int                ret_val, ntries;

    ret_val = -1;                   /* Assume failure to start */
    /*if ((tftp_io_context.virtual_fd =
        vf_open(localname,VO_WRONLY|VO_CREAT|VO_TRUNC,(VS_IWRITE | VS_IREAD))) < 0)
    {
        return(set_errno(EFOPENFAILED));
        return(-1);
    }*/

    /* init for write-behind   */
    dp = w_init();

    ap = (struct tftphdr *)acktftpbuf;
    convert = !tc_strcmp(mode, netascii);

    /* sin_port contains the destination for send tos    */
    tftpd_s_in.sin_port = hs2net(TFTP_CONTROL_PORT);

    do
    {
        if (firsttrip)
        {
            size = makerequest(RRQ, name, mode, ap);
            firsttrip = 0;
        }
        else
        {
            ap->th_opcode = hs2net((u_short)ACK);
            ap->th_block = hs2net((u_short)(block));
            size = 4;
            block++;
        }
        ntries = 0;
send_ack:
#        if (TRACE_TFTP)
            tpacket("sent", ap, size);
#        endif

        if (sendto(sock, acktftpbuf, size, 0, (PSOCKADDR)&tftpd_s_in,
                   sizeof (tftpd_s_in)) != size)
        {
            TFTP_DISPLAY_ERROR("tftpcli_recvfile: sendto failed");
            goto abort;
        }

        write_behind((PIO_CONTEXT)&tftp_io_context, convert);
        for ( ;; )
        {
            fromlen = sizeof(from);
            n = timed_recvfrom(sock, CFG_TFTP_TIMEOUT, (PFCHAR) dp, PKTSIZE, 0,
                (PSOCKADDR)&from, &fromlen);
            if (n < 0 && ntries++ < 5)
                goto send_ack;
            if (n < 0)
            {
                TFTP_DISPLAY_ERROR("tftpcli_recvfile: recvfrom failed")
                goto abort;
            }
            tftpd_s_in.sin_port = from.sin_port;   /* added */
#            if (TRACE_TFTP)
                tpacket("received", dp, n);
#            endif
            /* should verify client address   */
            dp->th_opcode = net2hs(dp->th_opcode);
            dp->th_block = net2hs(dp->th_block);
            if (dp->th_opcode == TFTPERROR)
            {
#if (DEBUG_TFTP)
                tc_sprintf(display_buffer,
                           "tftpcli_recvfile: Error code %d: %s\n",
                           dp->th_code, dp->th_msg);
                TFTP_DISPLAY_ERROR(display_buffer)
#endif
                set_errno(ETFTPERROR);
                goto abort;
            }
            if (dp->th_opcode == DATA)
            {
                if (dp->th_block == block)
                    break;          /* have next packet */

                /* On an error, try to synchronize both sides.   */
#if (DEBUG_TFTP && TRACE_TFTP)
                {
                int j;
                    /* Re-synchronize with the other side, i.e. read all data
                       on net */
                    j = synch_net(sock, acktftpbuf, sizeof(acktftpbuf));
                    if (j)
                    {
                        tc_sprintf(display_buffer,
                            "tftpcli_recvfile: discarded %d packets\n", j);
                        TFTP_DISPLAY_ERROR(display_buffer)
                    }
                }
#else
                /* Re-synchronize with the other side, i.e. read all data
                   on net */
                synch_net(sock, acktftpbuf, sizeof(acktftpbuf));
#endif
                if (dp->th_block == (block-1))
                    goto send_ack;  /* resend ack */
            }
        }
        size = writeit((PIO_CONTEXT)&tftp_io_context, &dp, n - 4, convert);
        if (size < 0)
        {
            DEBUG_ERROR("tftpcli_recvfile: call nak", NOVAR, 0, 0);
            tftpcli_nak(sock, EIOERROR);
            set_errno(EFWRITEFAILED);
            goto abort;
        }
    } while (size == SEGSIZE);

    ret_val = 0;  /* It worked */

abort:                                       /* ok to ack, since user */
    ap->th_opcode = hs2net((u_short)ACK);    /* has seen err msg */
    ap->th_block = hs2net((u_short)block);

    sendto(sock, acktftpbuf, 4, 0, (PSOCKADDR)&tftpd_s_in,
           sizeof (tftpd_s_in));
    write_behind((PIO_CONTEXT)&tftp_io_context, convert);   /* flush last buffer */

    //vf_close(tftp_io_context.virtual_fd);
    return(ret_val);
}

/* ********************************************************************   */
int makerequest(int request,char *name, char *mode, struct tftphdr *tp) /* __fn__*/
{
char *cp;

    tp->th_opcode = hs2net((u_short)request);
    cp = tp->th_stuff;
    tc_strcpy(cp, name);
    cp += tc_strlen(name);
    *cp++ = '\0';
    tc_strcpy(cp, mode);
    cp += tc_strlen(mode);
    *cp++ = '\0';
    return ((int)(cp - (char *)tp));
}

/* ********************************************************************   */
/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or EIOERROR for file system error
 */
void tftpcli_nak(int f, int error)                                  /* __fn__*/
{
KS_CONSTANT struct errmsg KS_FAR *pe;
struct tftphdr *tp;
int length;

    DEBUG_ERROR("tftpcli_nak: send nak", NOVAR, 0, 0);

    tp = (struct tftphdr *)acktftpbuf;
    tp->th_opcode = hs2net((u_short)TFTPERROR);
    tp->th_code = hs2net((u_short)error);
    length = 0;
    for (pe = errmsgs; pe->e_code >= 0; pe++)
    {
        if (pe->e_code == error)
        {
            tc_strcpy(tp->th_msg, pe->e_msg);
            length = tc_strlen(pe->e_msg) + 4;
            break;
        }
    }
    if (pe->e_code < 0)
    {
        tc_strcpy(tp->th_msg, file_system_error);
        length = tc_strlen(file_system_error);
        tp->th_code = EIOERROR;
    }
#    if (TRACE_TFTP)
        tpacket("tftpcli_nak: sent", tp, length);
#    endif
    if (sendto(f, acktftpbuf, length, 0, (PSOCKADDR)&tftpd_s_in,
        sizeof (tftpd_s_in)) != length)
    {
        TFTP_DISPLAY_ERROR("tftpcli_nak: sendto failed")
    }
}

/* ********************************************************************   */
#if (TRACE_TFTP)
void tpacket(char *s,struct tftphdr *tp, int n)              /* __fn__*/
{
static char *opcodes[] =
    { "#0", "RRQ", "WRQ", "DATA", "ACK", "ERROR" };

    char *cp, *file;
    u_short op = net2hs(tp->th_opcode);
    char *index();

    if (op < RRQ || op > TFTPERROR)
        tc_sprintf(display_buffer,"%s opcode=%x ", s, op);
    else
        tc_sprintf(display_buffer,"%s %s ", s, opcodes[op]);
    TFTP_SHOW_PROGRESS(display_buffer)

    switch (op)
    {

    case RRQ:
    case WRQ:
        n -= 2;
        file = cp = tp->th_stuff;
        while (*cp) cp++; /* cp = index(cp, '\0'); */
        tc_sprintf(display_buffer,"<file=%s, mode=%s>\n", file, cp + 1);
        break;

    case DATA:
        tc_sprintf(display_buffer,"<block=%d, %d bytes>\n", net2hs(tp->th_block), n - 4);
        break;

    case ACK:
        tc_sprintf(display_buffer,"<block=%d>\n", net2hs(tp->th_block));
        break;

    case TFTPERROR:
        tc_sprintf(display_buffer,"<code=%d, msg=%s>\n", net2hs(tp->th_code), tp->th_msg);
        break;

    default:
        tc_sprintf(display_buffer,"Error function lost");
        break;

    }
    TFTP_DISPLAY_ERROR(display_buffer)
}
#endif
#endif      /* INCLUDE_TFTP_CLI */
