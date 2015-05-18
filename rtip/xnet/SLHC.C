/*
 * Routines to compress and uncompress tcp packets (for transmission
 * over low speed serial lines).
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
 *  - 01-31-90  initial adaptation (from 1.19)
 *  PPP.05  02-15-90 [ks]
 *  PPP.08  05-02-90 [ks]   use PPP protocol field to signal compression
 *  PPP.15  09-90    [ks]   improve mbuf handling
 *  PPP.16  11-02    [karn] substantially rewritten to use NOS facilities
 *
 *  - Feb 1991  Bill_Simpson@um.cc.umich.edu
 *          variable number of conversation slots
 *          allow zero or one slots
 *          separate routines
 *          status display
 */

#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP


#include "sock.h"
#include "slhc.h"

#if (INCLUDE_PPP_VANJC || INCLUDE_CSLIP)

#define IP_DONTFRAG 0x1  /* tbdtbd - set to correct value, move to rtip.h */

#define DEBUG_SLOT 0

/* global data   */
struct slcompress KS_FAR compress_info[CFG_NUM_RS232];
struct cstate KS_FAR rcv_slot_info[CFG_NUM_RS232*(CFG_MAX_VJC_SLOTS+1)];
struct cstate KS_FAR tx_slot_info[CFG_NUM_RS232*(CFG_MAX_VJC_SLOTS+1)];

char *encode(char *cp, word n);
long  decode(PFBYTE *bpp, int *chlen);
int   slhc_toss(PSLCOMPRESS comp);

/* ********************************************************************   */
/* ADD_NET_L() Adds a net long to a machine-order word array              */
/*                                                                        */
/* This is used for decoding compressed changes to stored packets,        */
/* such as delta-seq, etc.                                                */
/*                                                                        */
/* Returns: nothing                                                       */
/*                                                                        */

void ADD_NET_L(PFWORD X, long Y)
{
long temp2;

    temp2 = net2hl(WARRAY_2_LONG((X)));
    temp2 += Y;
    LONG_2_WARRAY(X, (dword) hl2net(temp2));
}


/* ********************************************************************   */
/* slhc_init() - Initialize compression data structure                    */
/*                                                                        */
/* Slots must be in range 0 to 255 (zero meaning no compression)          */
/* The second two parameters are really only used in PPP, where           */
/* the number of remote and local slots is negotiated.                    */
/*                                                                        */
/* Returns: A pointer to the newly "allocated" data structure.            */
/*                                                                        */

PSLCOMPRESS slhc_init(PIFACE pi, int rslots, int tslots )
{
word i;
PCSTATE ts;
PSLCOMPRESS comp;

    comp = (PSLCOMPRESS)&compress_info[pi->minor_number];

    /* Initializes slot state info to point to free space in global slot_info   */
    if ( rslots > 0  &&  rslots <= CFG_MAX_VJC_SLOTS ) 
    {
        comp->rstate = (PCSTATE)(&rcv_slot_info[(pi->minor_number)*(CFG_MAX_VJC_SLOTS+1)]); /* Is the +1 really necessary? */
        comp->rslot_limit = (byte)(rslots - 1);
    }

    if ( tslots > 0  &&  tslots <= CFG_MAX_VJC_SLOTS ) 
    {
        comp->tstate = (PCSTATE)(&tx_slot_info[(pi->minor_number)*(CFG_MAX_VJC_SLOTS+1)]);
        comp->tslot_limit = (byte)(tslots - 1);
    }
#if (DEBUG_SLOT)
    DEBUG_ERROR("slhc_init: rslot_limit, tslot_limit ", EBS_INT2,
        comp->rslot_limit, comp->tslot_limit);
#endif

    comp->xmit_oldest = 0;
    comp->xmit_current = CFG_MAX_VJC_SLOTS;
    comp->recv_current = CFG_MAX_VJC_SLOTS;

#if (DEBUG_SLOT)
    DEBUG_ERROR("set comp->recv_current to ", EBS_INT1, comp->recv_current, 0);
#endif

    /*Initializes a circularly linked list of tslots   */
    if ( tslots > 0 ) 
    {
        ts = comp->tstate;
        for(i = comp->tslot_limit; i > 0; --i)
        {
            ts[i].csthis = (byte)i;
            ts[i].next = &(ts[i - 1]);
        }
        ts[0].next = &(ts[comp->tslot_limit]);
        ts[0].csthis = 0;
    }
    return comp;
}


/* ********************************************************************   */
/* slhc_free() - Free a compression data structure                        */
/*                                                                        */
/* Simply sets the slot states to null, destroying the linked list        */
/*                                                                        */
/* Returns: nothing                                                       */
/*                                                                        */

void slhc_free(PSLCOMPRESS comp)
{
    if ( comp == NULLSLCOMPR )  /*Make sure it's a valid pointer */
        return;

    comp->rstate = NULLSLSTATE;
    comp->tstate = NULLSLSTATE;
}


/* ********************************************************************   */
/* encode() - Encode a number                                             */
/*                                                                        */
/* Used for encoding word-length change fields into a single character    */
/* This function, when called repeatedly, will fill an array of chars     */
/* With the compressed form of the words passed to it.  There must be at  */
/* least three bytes allocated past cp.                                   */
/*                                                                        */
/* Returns a pointer to the lst bye of data that was just written,        */
/*         usable for the next call to encode.                            */
/*                                                                        */

char *encode(char *cp, word n)
{
word x;

    if (n >= 0x00FF || n == 0x0000) /* See if the number fits in one byte */
    {
        *cp++ = 0;  /* Increments cp to the second byte, setting it to zero */
        x = net2hs(n);                      /* Converts byte order */
        tc_movebytes((PFBYTE)cp, (PFBYTE)&x, 2);    
                                            /* moves bytes into the sequence    */
        cp += 2;                            /* increments cp to point to the  */
                                            /* next block   */
    } 
    else 
    {
        *cp++ = (char)n;            /* Increments cp, putting n in new place */
    }
    return cp;
}

/* ********************************************************************   */
/* decode() - Decode a number                                             */
/*                                                                        */
/* Decodes delta info coming from the other side.                         */
/*                                                                        */
/* Returns:  The decoded information.                                     */
/*                                                                        */

long decode(PFBYTE *inbpp, int *chlen)
{
word x;
PFBYTE bpp = *inbpp;

    *chlen = *chlen+1;
    x = (word)(*bpp);
    bpp++;
    if (x == 0)
    {
        *chlen = *chlen+2;
        tc_movebytes((PFBYTE)&x, bpp, 2);
        bpp += 2;
        *inbpp = bpp;
        return((long)((dword)net2hs(x)));
    } 
    else 
    {
        *inbpp = bpp;
        return (long)((dword)x);        /* -1 if PULLCHAR returned error */
    }
}

/* ********************************************************************   */
DCU slhc_alloc_packet(PIFACE pi, DCU msg, int who)
{
DCU msg_new;

    msg_new = os_alloc_packet(DCUTOPACKET(msg)->length, who); 
    if (!msg_new)
        return(msg_new);

    /* copy dcu_flags to new packet but always want tc_release_message   */
    /* to free the new message                                           */
    DCUTOCONTROL(msg_new).dcu_flags = DCUTOCONTROL(msg).dcu_flags;
    DCUTOCONTROL(msg_new).dcu_flags &= ~(PKT_FLAG_KEEP|PKT_FLAG_KEEP_ERROR);
    DCUTOCONTROL(msg_new).port = DCUTOCONTROL(msg).port;
    DCUTOPACKET(msg_new)->length = DCUTOPACKET(msg)->length;

    /* replace new message for tc_process_output so it will get freed when   */
    /* done                                                                  */
#if (INCLUDE_XMIT_QUE)
/* tbd   */
#else
    pi->xmit_dcu = msg_new;
#endif

    return(msg_new);
}

/* ********************************************************************      */
/* slhc_compress() - Compresses an IP packet using VJ compression            */
/*                                                                           */
/* Compression is done in line.                                              */
/* comp is the compression structure                                         */
/* packet points to IP header                                                */
/* compress_cid is boolean; yes if compression is desired.                   */
/*                                                                           */
/* Returns:  The type of compression used on the packet, i.e. SL_TYPE_IP,    */
/* SL_TYPE_COMPRESSED_TCP, or SL_TYPE_UNCOMPRESSED_TCP.  Should be ORed with */
/* the first byte in the packet for CSLIP;                                   */
/* Sets *length to the new length of the packet                              */
/* Sets *free_msg if a new message is allocated and needs to always be       */
/*   freed                                                                   */
/*                                                                           */

int slhc_compress(PIFACE pi, PSLCOMPRESS comp, DCU *msg, int compress_cid)
{
PCSTATE  ocs = &(comp->tstate[comp->xmit_oldest]);
PCSTATE  lcs = ocs;
PCSTATE  cs = lcs->next;
word     hlen;
PTCPPKT  oth;       /* previous packet sent */
word     deltaW;
unsigned long deltaS;
unsigned long deltaA;
word     changes = 0;
char     new_seq[16];
char *   cp = new_seq;
PFCHAR   cpf;
TCPPKT   th;          /* packet sending */
IPPKT    iph;
PFBYTE   p1, p2;
int      n_to_copy;
DCU      msg_new;
PFBYTE   packet;
int      length;

    DEBUG_LOG("slhc_compress - entered", LEVEL_3, NOVAR, 0, 0);

#if (DEBUG_SLOT)
    DEBUG_ERROR("slhc_compress: ", NOVAR, 0, 0);
#endif

    packet = DCUTODATA(*msg);
    packet += ETH_HLEN_BYTES;

    /* **************************************************   */
    /* Extract IP header                                    */
    tc_movebytes((PFBYTE)&(iph.ip_verlen), packet, IP_HLEN_BYTES);
    hlen = (word)((iph.ip_verlen & 0x0f) << 2);

    /* **************************************************     */
    /* Bail if this packet isn't TCP, or is an IP fragment    */
    if (iph.ip_proto != PROTTCP || (iph.ip_fragoff & IP_FRAGOFF) || 
       (iph.ip_fragoff & IP_MOREFRAG))
    {
        DEBUG_LOG("slhc_compress - not tcp or is frag: proto, frag = ", LEVEL_3,
            EBS_INT2, iph.ip_proto, iph.ip_fragoff);
        /* Send as regular IP   */
        if (iph.ip_proto != PROTTCP)
            comp->sls_o_nontcp++;
        else
            comp->sls_o_tcp++;
return_same_packet:
        return SL_TYPE_IP;
    }

    /* **************************************************   */
    /* Extract TCP header                                   */
    tc_movebytes((PFBYTE)&th, packet+IP_HLEN_BYTES, TCP_HLEN_BYTES);

    hlen = (word)(hlen + ((th.tcp_hlen & 0xf0) >> 2));

    /* **************************************************   */
    /*  Bail if the TCP packet isn't `compressible' (i.e., ACK isn't set or
     *  some other control bit is set).  
    */
    if (( th.tcp_flags & (TCP_F_RESET|TCP_F_SYN|TCP_F_FIN)) || 
        !(th.tcp_flags & TCP_F_ACK) )
    {
        DEBUG_LOG("slhc_compress - not compressable, flags = ", LEVEL_3,
            EBS_INT1, th.tcp_flags, 0);

        /* TCP connection stuff; send as regular IP   */
        comp->sls_o_tcp++;
        goto return_same_packet;
    }

    /* **************************************************   */
    /*
     * Packet is compressible -- we're going to send either a
     * COMPRESSED_TCP or UNCOMPRESSED_TCP packet.  Either way,
     * we need to locate (or create) the connection state.
     *
     * States are kept in a circularly linked list with
     * xmit_oldest pointing to the end of the list.  The
     * list is kept in lru order by moving a state to the
     * head of the list whenever it is referenced.  Since
     * the list is short and, empirically, the connection
     * we want is almost always near the front, we locate
     * states via linear search.  If we don't find a state
     * for the datagram, the oldest state is (re-)used.
     */
    for ( ; ; ) 
    {
        if ( tc_cmp4(iph.ip_src, cs->cs_ip.ip_src, 4) &&
             tc_cmp4(iph.ip_dest, cs->cs_ip.ip_dest, 4) &&
             th.tcp_source == cs->cs_tcp.tcp_source &&
             th.tcp_dest == cs->cs_tcp.tcp_dest)
            goto found;

#if (DEBUG_SLOT)
        DEBUG_ERROR("slhc_compress: not found", NOVAR, 0, 0);
#endif
        /* if current equal oldest, at end of list   */
        if ( cs == ocs )
            break;
        lcs = cs;
        cs = cs->next;
        comp->sls_o_searches++;
    };

    /*
     * Didn't find it -- re-use oldest cstate.  Send an
     * uncompressed packet that tells the other side what
     * connection number we're using for this conversation.
     *
     * Note that since the state list is circular, the oldest
     * state points to the newest and we only need to set
     * xmit_oldest to update the lru linkage.
     */
    comp->sls_o_misses++;
    comp->xmit_oldest = lcs->csthis;

    DEBUG_LOG("slhc_compress - did not find it", LEVEL_3,
        NOVAR, 0, 0);

    goto uncompressed;

    /* **************************************************   */
found:
    DEBUG_LOG("slhc_compress - found it", LEVEL_3,
        NOVAR, 0, 0);
    /*
     * Found it -- move to the front on the connection list.
     */
#if (DEBUG_SLOT)
    DEBUG_ERROR("slhc_compress: found", NOVAR, 0, 0);
#endif
    if (lcs == ocs) 
    {
        /* found at most recently used   */
    } 
    else if (cs == ocs) 
    {
        /* found at least recently used   */
        comp->xmit_oldest = lcs->csthis;
    } 
    else 
    {
        /* more than 2 elements   */
        lcs->next = cs->next;
        cs->next = ocs->next;
        ocs->next = cs;
    }

    /* **************************************************   */
    /*
     * Make sure that only what we expect to change changed.
     * Check the following:
     * IP protocol version, header length & type of service.
     * The "Don't fragment" bit.
     * The time-to-live field.
     * The TCP header length.
     * IP options, if any.
     * TCP options, if any.
     * If any of these things are different between the previous &
     * current datagram, we send the current datagram `uncompressed'.
     */
    oth = &cs->cs_tcp;

    if (iph.ip_verlen != cs->cs_ip.ip_verlen /* || 
        iph.optlen != cs->cs_ip.optlen */ ||
        iph.ip_tos != cs->cs_ip.ip_tos ||
        (iph.ip_fragoff & IP_DONTFRAG) != (cs->cs_ip.ip_fragoff & IP_DONTFRAG) ||
        iph.ip_ttl != cs->cs_ip.ip_ttl /* || */
/*      th.optlen != cs->cs_tcp.optlen  ||                                                */
/*      (iph.optlen > 0 &&   memcmp(iph.options,cs->cs_ip.options,iph.optlen) != 0) ||    */
/*      (th.optlen > 0 && memcmp(th.options,cs->cs_tcp.options,th.optlen) != 0) */)
    {
        DEBUG_LOG("slhc_compress - unexpected change - uncompress", LEVEL_3,
            NOVAR, 0, 0);
#if (DEBUG_SLOT)
        DEBUG_ERROR("slhc_compress: verify failed", NOVAR, 0, 0);
#endif
        goto uncompressed;
    }

    /* **************************************************   */
    /*
     * Figure out which of the changing fields changed.  The
     * receiver expects changes in the order: urgent, window,
     * ack, seq (the order minimizes the number of temporaries
     * needed in this section of code).
     */
    if (th.tcp_flags & TCP_F_URGENT)
    {
        DEBUG_LOG("slhc_compress - set NEW_U", LEVEL_3,
            NOVAR, 0, 0);
        deltaW = net2hs(th.tcp_urgent);
        cp = encode(cp,deltaW);
        changes |= NEW_U;
    } 
    else if (th.tcp_urgent != oth->tcp_urgent)
    {
        DEBUG_LOG("slhc_compress - URG not set but urp changed => uncompress", 
            LEVEL_3, NOVAR, 0, 0);

        /* argh! URG not set but urp changed -- a sensible
         * implementation should never do this but RFC793
         * doesn't prohibit the change so we have to deal
         * with it. */
        goto uncompressed;
    }

    if ((deltaW = (word)(net2hs(th.tcp_window) - net2hs(oth->tcp_window))) != 0)
    {
        DEBUG_LOG("slhc_compress - set NEW_W", LEVEL_3,
            NOVAR, 0, 0);
        cp = encode(cp,deltaW);
        changes |= NEW_W;

    }

    if ((deltaA = net2hl(WARRAY_2_LONG(th.tcp_ack))
                 -net2hl(WARRAY_2_LONG(oth->tcp_ack))) != 0L) 
    {
        if (deltaA > 0x0000ffff)
        {
            DEBUG_LOG("slhc_compress - ack change too large=>uncompress", 
                LEVEL_3, NOVAR, 0, 0);
            goto uncompressed;
        }
        DEBUG_LOG("slhc_compress - set NEW_A", LEVEL_3,
            NOVAR, 0, 0);
        cp = encode(cp,(word)deltaA);
        changes |= NEW_A;
    }

    if ((deltaS = net2hl(WARRAY_2_LONG(th.tcp_seq))
                 -net2hl(WARRAY_2_LONG(oth->tcp_seq))) != 0L) 
    {
        if ( (deltaS > 0x0000ffff) ||
             (net2hl(WARRAY_2_LONG(th.tcp_seq)) <
              net2hl(WARRAY_2_LONG(oth->tcp_seq))) )
        {
            DEBUG_LOG("slhc_compress - seq change too large=>uncompress", 
                LEVEL_3, NOVAR, 0, 0);
            goto uncompressed;
        }
        DEBUG_LOG("slhc_compress - set NEW_S", LEVEL_3,
            NOVAR, 0, 0);
        cp = encode(cp,(word)deltaS);
        changes |= NEW_S;
    }

    DEBUG_LOG("slhc_compress - changes = ", LEVEL_3, EBS_INT1, changes, 0);
    switch(changes)
    {
    case 0: /* Nothing changed. If this packet contains data and the
         * last one didn't, this is probably a data packet following
         * an ack (normal on an interactive connection) and we send
         * it compressed.  Otherwise it's probably a retransmit,
         * retransmitted ack or window probe.  Send it uncompressed
         * in case the other side missed the compressed version.
         */
        if (iph.ip_len != cs->cs_ip.ip_len && cs->cs_ip.ip_len == hs2net(hlen))
            break;
        DEBUG_LOG("slhc_compress - nothing changes - uncompress", LEVEL_3,
            NOVAR, 0, 0);
        goto uncompressed;
    case SPECIAL_I:
    case SPECIAL_D:
        /* actual changes match one of our special case encodings --
         * send packet uncompressed.
         */
        DEBUG_LOG("slhc_compress - SPECIAL_I, SPECIAL_D", LEVEL_3,
            NOVAR, 0, 0);
        goto uncompressed;
    case NEW_S | NEW_A:
        DEBUG_LOG("slhc_compress - NEW_S and NEW_I", LEVEL_3,
            NOVAR, 0, 0);
        if ( deltaS == deltaA &&
             deltaS == (unsigned long)(net2hs(cs->cs_ip.ip_len) - hlen) )
        {
            /* special case for echoed terminal traffic   */
            changes = SPECIAL_I;
            cp = new_seq;
        }
        break;
    case NEW_S:
        DEBUG_LOG("slhc_compress - NEW_S", LEVEL_3,
            NOVAR, 0, 0);

        if ( deltaS == (unsigned long)(net2hs(cs->cs_ip.ip_len) - hlen) )
        {
            DEBUG_LOG("slhc_compress - set SPECIAL_D", LEVEL_3,
                NOVAR, 0, 0);
            /* special case for data xfer   */
            changes = SPECIAL_D;
            cp = new_seq;
        }
        break;
    }           /* end of switch */

    deltaW = (word)(net2hs(iph.ip_id) - net2hs(cs->cs_ip.ip_id));
    if (deltaW != 1)
    {
        DEBUG_LOG("slhc_compress - set NEW_I", LEVEL_3,
            NOVAR, 0, 0);
        cp = encode(cp,deltaW);
        changes |= NEW_I;
    }
    if (th.tcp_flags & TCP_F_TPUSH)
    {
        DEBUG_LOG("slhc_compress - set PUSH BIT", LEVEL_3,
            NOVAR, 0, 0);
        changes |= TCP_PUSH_BIT;
    }

    /* **************************************************   */
    /* Grab the cksum before we overwrite it below.  Then update our
     * state with this packet's header.
     */
    tc_movebytes((PFBYTE)&deltaA , (PFBYTE)&th.tcp_chk, 2);

    STRUCT_COPY(cs->cs_ip, iph);
    STRUCT_COPY(cs->cs_tcp, th);

    /* We want to use the original packet as our compressed packet.
     * (cp - new_seq) is the number of bytes we need for compressed
     * sequence numbers.  In addition we need one byte for the change
     * mask, one for the connection id and two for the tcp checksum.
     * So, (cp - new_seq) + 4 bytes of header are needed.
     */
    msg_new = slhc_alloc_packet(pi, *msg, SLHC_ALLOC1); 
    if (!msg_new)
    {
        DEBUG_ERROR("slhc_compress: out of DCUs, don't compress", NOVAR, 0, 0);
        return SL_TYPE_IP;
    }

    /* **************************************************   */
    deltaW = (word)(cp - new_seq);
    p1 = packet + hlen;
    if (compress_cid == 0 || comp->xmit_current != cs->csthis)
    {
        length = 4;
        p2 = DCUTODATA(msg_new) + (deltaW + 4) + ETH_HLEN_BYTES;
        n_to_copy = net2hs(iph.ip_len) - hlen;

        /* copy the data   */
        tc_movebytes(p2, p1, n_to_copy);
        cpf = (PFCHAR)DCUTODATA(msg_new) + ETH_HLEN_BYTES;
        *cpf++ = (char)(changes | NEW_C); 
        *cpf++ = cs->csthis; 
        comp->xmit_current = cs->csthis;
#if (DEBUG_SLOT)
        DEBUG_ERROR("slhc_compress: set NEW_C: csthis: ", EBS_INT1,
            cs->csthis, 0);
#endif
    } 
    else 
    {
        length = 3;
        p2 = DCUTODATA(msg_new) + (deltaW + 3) + ETH_HLEN_BYTES;
        n_to_copy = net2hs(iph.ip_len) - hlen;
        tc_movebytes(p2, p1, n_to_copy);
        cpf = (PFCHAR)DCUTODATA(msg_new) + ETH_HLEN_BYTES;
        *cpf++ = (char)changes;
    }

    tc_memmove((PFBYTE)cpf, (PFBYTE)&deltaA, 2);    /* Write TCP checksum */
    cpf += 2;
    tc_memmove((PFBYTE)cpf, (PFBYTE)new_seq, deltaW);   /* Write list of deltas */
    comp->sls_o_compressed++;
    length += (n_to_copy + deltaW); /* return new length of packet */

    /* done with original DCU so free it   */
    DCUTOCONTROL(*msg).dcu_flags &= ~PKT_SEND_IN_PROG;
    FREE_DCU(*msg, DCUTOCONTROL(*msg).dcu_flags)

    /* pass back the new DCU to the caller   */
    *msg = msg_new;
    DCUTOPACKET(msg_new)->length = length + ETH_HLEN_BYTES;

    return SL_TYPE_COMPRESSED_TCP;

    /* **************************************************   */
    /* Update connection state cs & send uncompressed packet (i.e.,
     * a regular ip/tcp packet but with the 'conversation id' we hope
     * to use on future compressed packets in the protocol field).
     */
uncompressed:
    STRUCT_COPY(cs->cs_ip, iph);
    STRUCT_COPY(cs->cs_tcp, th);
    comp->xmit_current = cs->csthis;
    comp->sls_o_uncompressed++;

    msg_new = slhc_alloc_packet(pi, *msg, SLHC_ALLOC2); 
    if (!msg_new)
    {
        DEBUG_ERROR("slhc_compress: out of DCUs, don't compress", NOVAR, 0, 0);
        return SL_TYPE_IP;
    }

    packet = DCUTODATA(msg_new);
    tc_movebytes(packet, DCUTODATA(*msg), DCUTOPACKET(*msg)->length);

    /* replace IP protocol field in the with the connection number   */
    *(packet+9+ETH_HLEN_BYTES) = cs->csthis;    /* ip_proto */
#if (DEBUG_SLOT)
    DEBUG_ERROR("slhc_compress: uncompress: csthis ", EBS_INT1, cs->csthis, 0);
#endif

    /* done with original DCU so free it   */
    DCUTOCONTROL(*msg).dcu_flags &= ~PKT_SEND_IN_PROG;
    FREE_DCU(*msg, DCUTOCONTROL(*msg).dcu_flags)

    /* pass back the new DCU to the caller   */
    *msg = msg_new;

    return SL_TYPE_UNCOMPRESSED_TCP;
}


/* ********************************************************************     */
/* slhc_uncompress() - Uncompresses an incoming compressed packet           */
/*                                                                          */
/* Uncompress is expecting an ethernet header in msg;                       */
/* comp is the compression structure, found in iface (for PPP) and found in */
/* RS232_IF_INFO (for CSLIP)                                                */
/*                                                                          */
/* Returns: Length of the new packet, 0 on error                            */
/*                                                                          */

int slhc_uncompress(PSLCOMPRESS comp, DCU msg)
{
int     changes;
long    x;
PTCPPKT thp;
PCSTATE cs;
int     len;
int     chlen;
PIPPKT  pip;
PTCPPKT ptcp;
PFBYTE  bpp;
word    dlen;

    DEBUG_LOG("slhc_uncompress - entered", LEVEL_3, NOVAR, 0, 0);
#if (DEBUG_SLOT)
    DEBUG_ERROR("slhc_uncompress: ", NOVAR, 0, 0);
#endif

    /* We've got a compressed packet; read the change byte   */
    comp->sls_i_compressed++;
    if (DCUTOPACKET(msg)->length < 3)
    {
        DEBUG_LOG("slhc_uncompress - error - length = ", LEVEL_3, EBS_INT1, 
            DCUTOPACKET(msg)->length, 0);
        comp->sls_i_error++;
        return 0;
    }
    bpp = DCUTODATA(msg);
    bpp += ETH_HLEN_BYTES;
    changes = *bpp++;   /* "Can't fail" */
    chlen = 1;
    if (changes & NEW_C)
    {
        DEBUG_LOG("slhc_uncompress - change & NEW_C", LEVEL_3, NOVAR, 0, 0);

        /* Make sure the state index is in range, then grab the state.
         * If we have a good state index, clear the 'discard' flag.
         */
        x = *bpp++; /* Read conn index */
#if (DEBUG_SLOT)
        DEBUG_ERROR("slhc_uncompress: slot number is ", EBS_INT1, x, 0);
#endif
        chlen++;
        if (x < 0 || x > comp->rslot_limit)
        {
            DEBUG_LOG("slhc_uncompress - conn index out of range", LEVEL_3,
                EBS_INT2, x, comp->rslot_limit);
#if (DEBUG_SLOT)
            DEBUG_ERROR("slhc_uncompress - conn index out of range", 
                EBS_INT2, x, comp->rslot_limit);
#endif
            goto bad;
        }

        comp->flags &=~ SLF_TOSS;
        comp->recv_current = (byte)x;
    } 
    else 
    {
        /* this packet has an implicit state index.  If we've
         * had a line error since the last time we got an
         * explicit state index, we have to toss the packet. */
        if (comp->flags & SLF_TOSS)
        {
            DEBUG_LOG("slhc_uncompress - SLF_TOSS", LEVEL_3, NOVAR, 0, 0);
            comp->sls_i_tossed++;
            return 0;
        }
    }

    cs = &comp->rstate[comp->recv_current];
    thp = &cs->cs_tcp;

    chlen += 2;
    tc_movebytes((PFBYTE)&thp->tcp_chk, bpp, 2);
    bpp += 2;
    if (thp->tcp_chk == 0xffff)
    {
        DEBUG_LOG("slhc_uncompress - tcp_chk = 0xffff", LEVEL_3, NOVAR, 0, 0);
        goto bad;
    }

    if (changes & TCP_PUSH_BIT)
        thp->tcp_flags |= TCP_F_TPUSH;
    else
        thp->tcp_flags &= ~TCP_F_TPUSH;

    switch (changes & SPECIALS_MASK)
    {
    case SPECIAL_I:     /* Echoed terminal traffic */
        {
        word i;
        DEBUG_LOG("slhc_uncompress - changes & SPECIAL_I", LEVEL_3, NOVAR, 0, 0);
        i = net2hs(cs->cs_ip.ip_len);
/*      i -= (cs->cs_ip.optlen + IP_HLEN_BYTES + TCP_HLEN_BYTES);   */
        i = (word)(i - (IP_HLEN_BYTES + TCP_HLEN_BYTES));

        /* add i to ack   */
        ADD_NET_L(thp->tcp_ack, i);

        /* add i to seq   */
        ADD_NET_L(thp->tcp_seq, i);
        }
        break;

    case SPECIAL_D:         /* Unidirectional data */
        DEBUG_LOG("slhc_uncompress - changes & SPECIAL_D", LEVEL_3, NOVAR, 0, 0);
/*      thp->tcp_seq += cs->cs_ip.ip_len - (cs->cs_ip.optlen +IP_HLEN_BYTES + TCP_HLEN_BYTES);   */
        ADD_NET_L(thp->tcp_seq, net2hs(cs->cs_ip.ip_len) - (IP_HLEN_BYTES + TCP_HLEN_BYTES));
        break;

    default:
        if (changes & NEW_U)
        {
            DEBUG_LOG("slhc_uncompress - changes & NEW_U", LEVEL_3, NOVAR, 0, 0);
            thp->tcp_flags |= TCP_F_URGENT;
            if ((x = decode(&bpp, &chlen)) == -1)
            {
                DEBUG_LOG("slhc_uncompress - changes & NEW_U - decode failed", 
                    LEVEL_3, NOVAR, 0, 0);
                goto bad;
            }
            thp->tcp_urgent = (word)x;
        } 
        else
            thp->tcp_flags &= ~TCP_F_URGENT;
        if (changes & NEW_W)
        {
            if ((x = decode(&bpp, &chlen)) == -1)
            {
                DEBUG_LOG("slhc_uncompress - changes & NEW_W - decode failed", 
                    LEVEL_3, NOVAR, 0, 0);
                goto bad;
            }

            /* add x to window size   */
            thp->tcp_window = (word)(hs2net((word)(net2hs(thp->tcp_window) + 
                                               (word)x)));
/*          thp->tcp_window = (word)(thp->tcp_window + (word)x);   */
        }
        if (changes & NEW_A)
        {
            if ((x = decode(&bpp, &chlen)) == -1)
            {
                DEBUG_LOG("slhc_uncompress - changes & NEW_A - decode failed", 
                    LEVEL_3, NOVAR, 0, 0);
                goto bad;
            }

            /* add x to ack   */
            ADD_NET_L(thp->tcp_ack, x);
        }
        if (changes & NEW_S)
        {
            if ((x = decode(&bpp, &chlen)) == -1)
            {
                DEBUG_LOG("slhc_uncompress - changes & NEW_S - decode failed", 
                    LEVEL_3, NOVAR, 0, 0);
                goto bad;
            }
            ADD_NET_L(thp->tcp_seq, x);
        }
        break;
    }
    if (changes & NEW_I)
    {
        if ((x = decode(&bpp, &chlen)) == -1)
        {
            DEBUG_LOG("slhc_uncompress - changes & NEW_I - decode failed", 
                LEVEL_3, NOVAR, 0, 0);
            goto bad;
        }
        cs->cs_ip.ip_id = (word)(hs2net((word)(net2hs(cs->cs_ip.ip_id) + 
                                               (word)x)));
    } 
    else 
    {
        cs->cs_ip.ip_id = (word)(hs2net((word)(net2hs(cs->cs_ip.ip_id) + 
                                               1)));
    }

    /*
     * At this point, bpp points to the first byte of data in the
     * packet.  Put the reconstructed TCP and IP headers back on the
     * packet.  Recalculate IP checksum (but not TCP checksum).
     */
    /* len = len_p(*bpp) + IP_HLEN_BYTES + TCP_HLEN_BYTES + cs->cs_ip.optlen;   */
    dlen = (word)(DCUTOPACKET(msg)->length - chlen - ETH_HLEN_BYTES);
    len = dlen + IP_HLEN_BYTES + TCP_HLEN_BYTES;   /* + cs->cs_ip.optlen */

    /* copy the data - guarenteed to work with overlapping fields   */
    tc_memmove(DCUTODATA(msg) + IP_HLEN_BYTES + TCP_HLEN_BYTES + ETH_HLEN_BYTES,
               DCUTODATA(msg) + chlen + ETH_HLEN_BYTES, dlen);

    /* set the new total packet length   */
    cs->cs_ip.ip_len = hs2net((word)len);
    DCUTOPACKET(msg)->length = len + ETH_HLEN_BYTES;

    /* copy the uncompressed IP header and TCP headers but first need   */
    /* to setup pointers to PROTOCOL address                            */
    DCUTOPACKET(msg)->ip_option_len = 0;    /* tbd: support options */
    DCUSETUPPTRS(msg, FALSE);

    pip = DCUTOIPPKT(msg);
    ptcp = DCUTOTCPPKT(msg);

   tc_movebytes((PFBYTE)ptcp , (PFBYTE)thp, sizeof(*thp));
   tc_movebytes((PFBYTE)pip , (PFBYTE)&cs->cs_ip, sizeof(cs->cs_ip));

#if (INCLUDE_CSLIP)
    /* For CSLIP, the first 4 bits have to be reset to 0x40 before chksum   */
    /* This doesn't matter for PPP, because first 4 bytes are always 0x40   */
    pip->ip_verlen &= 0x0F;
    pip->ip_verlen |= 0x40;
#endif

    /* recalc IP checksum   */
    pip->ip_cksum = 0;
    pip->ip_cksum = tc_ip_chksum((PFWORD) &pip->ip_verlen, 10);

    return len;
bad:
    DEBUG_LOG("slhc_uncompress - bad => toss", LEVEL_3, NOVAR, 0, 0);
    comp->sls_i_error++;
    return slhc_toss( comp );
}


/* ********************************************************************    */
/* slhc_remember() - Remembers an UNCOMPRESSED packet                      */
/*                                                                         */
/* Pass uncompressed packets to this function, with the compression slots. */
/* function will manage slots automatically                                */
/* slhc_remember is expecting an ethernet header on in_msg!                */
/*                                                                         */
/* Returns: length of the packet (in_msg)                                  */
/*                                                                         */

int slhc_remember(PSLCOMPRESS comp, DCU in_msg)
{
PCSTATE cs;
TCPPKT  th;
IPPKT   iph;

#if (DEBUG_SLOT)
    DEBUG_ERROR("slhc_remember: ", NOVAR, 0, 0);
#endif

    DEBUG_LOG("slhc_remember - entered", LEVEL_3, NOVAR, 0, 0);

    /* Extract IP and TCP headers and verify conn ID but first need
       to set up PROTOCOL headers */
    DCUTOPACKET(in_msg)->ip_option_len = 0; /* tbd: support options */
    DCUSETUPPTRS(in_msg, FALSE);

    tc_movebytes((PFBYTE)&iph , (PFBYTE)(DCUTOIPPKT(in_msg)), sizeof(iph));
    tc_movebytes((PFBYTE)&th , (PFBYTE)(DCUTOTCPPKT(in_msg)), sizeof(th));

#if (DEBUG_SLOT)
    DEBUG_ERROR("slhc_remember: slot value,limit ", 
        EBS_INT2, iph.ip_proto, comp->rslot_limit);
#endif

    /* slot number is passed in the ip_proto field of uncompressed   */
    /* packets                                                       */
    if ((byte)iph.ip_proto > comp->rslot_limit) 
    {
#if (DEBUG_SLOT)
        DEBUG_ERROR("slhc_remember: slot out of range: value,limit ", 
            EBS_INT2, iph.ip_proto, comp->rslot_limit);
#endif
        comp->sls_i_error++;
        return slhc_toss(comp);
    }

    /* Update local state   */
    cs = &comp->rstate[comp->recv_current = (byte)(iph.ip_proto)];
    comp->flags &=~ SLF_TOSS;
    iph.ip_proto = PROTTCP;
    tc_movebytes((PFBYTE)&cs->cs_ip , (PFBYTE)&iph, sizeof(iph));
    tc_movebytes((PFBYTE)&cs->cs_tcp , (PFBYTE)&th, sizeof(th));


    /* Put headers back on packet
     * Neither header checksum is recalculated
     */
    (DCUTOIPPKT(in_msg))->ip_proto = PROTTCP;
    comp->sls_i_uncompressed++;

#if (INCLUDE_CSLIP)
    /* For CSLIP, the first 4 bits have to be reset to 0x40 before chksum   */
    /* This doesn't matter for PPP, because first 4 bytes are always 0x40   */
    DCUTOIPPKT(in_msg)->ip_verlen &= 0x0F;
    DCUTOIPPKT(in_msg)->ip_verlen |= 0x40;

    /* recalc IP checksum   */
    DCUTOIPPKT(in_msg)->ip_cksum = 0;
    DCUTOIPPKT(in_msg)->ip_cksum = tc_ip_chksum((PFWORD) &DCUTOIPPKT(in_msg)->ip_verlen, 10);
#endif

    return (DCUTOPACKET(in_msg)->length);
}


/* ********************************************************************   */
/* slhc_toss() - toss an invalid compressed slot                          */
/*                                                                        */
/*                                                                        */
/* This returns zero becuase it is always used to return error condition  */
/*                                                                        */

int slhc_toss(PSLCOMPRESS comp)
{
    if ( comp != NULLSLCOMPR )
        comp->flags |= SLF_TOSS;
    return 0;
}

#endif



