/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef _SLHC_H
#define _SLHC_H

#include "rtip.h"

#if (INCLUDE_PPP_VANJC || INCLUDE_CSLIP)

/*
 * Definitions for tcp compression routines.
 *
 * $Header: /var/lib/dev-cvs/ebs/rtip/v4.1/source/xnet/slhc.h,v 1.1 2003/07/02 19:09:19 sarah Exp $
 *
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  Van Jacobson (van@helios.ee.lbl.gov), Dec 31, 1989:
 *  - Initial distribution.
 *
 *
 * modified for KA9Q Internet Software Package by
 * Katie Stevens (dkstevens@ucdavis.edu)
 * University of California, Davis
 * Computing Services
 *  - 01-31-90  initial adaptation
 *
 *  - Feb 1991  Bill_Simpson@um.cc.umich.edu
 *          variable number of conversation slots
 *          allow zero or one slots
 *          separate routines
 *          status display
 */

/* ********************************************************************   */
/*
 * Compressed packet format:
 *
 * The first octet contains the packet type (top 3 bits), TCP
 * 'push' bit, and flags that indicate which of the 4 TCP sequence
 * numbers have changed (bottom 5 bits).  The next octet is a
 * conversation number that associates a saved IP/TCP header with
 * the compressed packet.  The next two octets are the TCP checksum
 * from the original datagram.  The next 0 to 15 octets are
 * sequence number changes, one change per bit set in the header
 * (there may be no changes and there are two special cases where
 * the receiver implicitly knows what changed -- see below).
 *
 * There are 5 numbers which can change (they are always inserted
 * in the following order): TCP urgent pointer, window,
 * acknowlegement, sequence number and IP ID.  (The urgent pointer
 * is different from the others in that its value is sent, not the
 * change in value.)  Since typical use of SLIP links is biased
 * toward small packets (see comments on MTU/MSS below), changes
 * use a variable length coding with one octet for numbers in the
 * range 1 - 255 and 3 octets (0, MSB, LSB) for numbers in the
 * range 256 - 65535 or 0.  (If the change in sequence number or
 * ack is more than 65535, an uncompressed packet is sent.)
 */

/*
 * Packet types (must not conflict with IP protocol version)
 *
 * The top nibble of the first octet is the packet type.  There are
 * three possible types: IP (not proto TCP or tcp with one of the
 * control flags set); uncompressed TCP (a normal IP/TCP packet but
 * with the 8-bit protocol field replaced by an 8-bit connection id --
 * this type of packet syncs the sender & receiver); and compressed
 * TCP (described above).
 *
 * LSB of 4-bit field is TCP "PUSH" bit (a worthless anachronism) and
 * is logically part of the 4-bit "changes" field that follows.  Top
 * three bits are actual packet type.  For backward compatibility
 * and in the interest of conserving bits, numbers are chosen so the
 * IP protocol version number (4) which normally appears in this nibble
 * means "IP packet".
 */

/* SLIP compression masks for len/vers byte   */
#define SL_TYPE_IP 0x40
#define SL_TYPE_UNCOMPRESSED_TCP 0x70
#define SL_TYPE_COMPRESSED_TCP 0x80
#define SL_TYPE_ERROR 0x00

/* Bits in first octet of compressed packet   */
/* flag bits for what changed in a packet     */
#define NEW_C   0x40    
#define NEW_I   0x20
#define NEW_S   0x08        /* new SEQ number */
#define NEW_A   0x04        /* new ACK value */
#define NEW_W   0x02        /* new window size */
#define NEW_U   0x01        /* new urgent pointer */

/* reserved, special-case values of above   */
#define SPECIAL_I (NEW_S|NEW_W|NEW_U)       /* echoed interactive traffic */
#define SPECIAL_D (NEW_S|NEW_A|NEW_W|NEW_U) /* unidirectional data */
#define SPECIALS_MASK (NEW_S|NEW_A|NEW_W|NEW_U)

#define TCP_PUSH_BIT 0x10

typedef struct cstate KS_FAR *PCSTATE;
/*
 * "state" data for each active tcp conversation on the wire.  This is
 * basically a copy of the entire IP/TCP header from the last packet
 * we saw from the conversation together with a small identifier
 * the transmit & receive ends of the line use to locate saved header.
 */
typedef struct cstate 
{
    byte    csthis;     /* connection id number (xmit) */
    PCSTATE next;       /* next in ring (xmit) */
    IPPKT   cs_ip;      /* ip/tcp hdr from most recent packet */
    TCPPKT  cs_tcp;
} CSTATE;
#define NULLSLSTATE (PCSTATE )0

/*
 * all the state data for one serial line (we need one of these per line).
 */
struct slcompress 
{
    PCSTATE tstate;             /* transmit connection states (array)*/
    PCSTATE rstate;             /* receive connection states (array)*/

    byte tslot_limit;           /* highest transmit slot id (0-l)*/
    byte rslot_limit;           /* highest receive slot id (0-l)*/

    byte xmit_oldest;           /* oldest xmit in ring */
    byte xmit_current;          /* most recent xmit id */
    byte recv_current;          /* most recent rcvd id */

    byte flags;
#define SLF_TOSS    0x01        /* tossing rcvd frames until id received */

    long sls_o_nontcp;          /* outbound non-TCP packets */
    long sls_o_tcp;             /* outbound TCP packets */
    long sls_o_uncompressed;    /* outbound uncompressed packets */
    long sls_o_compressed;      /* outbound compressed packets */
    long sls_o_searches;        /* searches for connection state */
    long sls_o_misses;          /* times couldn't find conn. state */

    long sls_i_uncompressed;    /* inbound uncompressed packets */
    long sls_i_compressed;      /* inbound compressed packets */
    long sls_i_error;           /* inbound error packets */
    long sls_i_tossed;          /* inbound packets tossed because of error */
};
#define NULLSLCOMPR (struct slcompress *)0
typedef struct slcompress KS_FAR *PSLCOMPRESS;

/* slhc.c   */
PSLCOMPRESS slhc_init(PIFACE pi, int rslots, int tslots);
void slhc_free(PSLCOMPRESS comp);
int  slhc_compress(PIFACE pi, PSLCOMPRESS comp, DCU *msg, int compress_cid);
int  slhc_uncompress(PSLCOMPRESS comp, DCU msg);
int  slhc_remember(PSLCOMPRESS comp, DCU in_msg);
int  slhc_toss(PSLCOMPRESS comp);

#endif

#endif  /* _SLHC_H */

