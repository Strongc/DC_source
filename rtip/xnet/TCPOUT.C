/*                                                                      */
/* TCPOUT.C - TCP output functions                                      */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains TCP output functions.                      */

/* Functions in this module                                               */
/* tc_tcpsend         -   Send a TCP packet.  All routines should call    */
/*                        this routine to send a packet.                  */
/* transf             -   Calculates TCP & IP checksums, generates IP     */
/*                        sequence number and sends a TCP packet.         */
/* transq             -   Transmits as many bytes as possible to the TCP  */
/*                        socket at the other end of the connection. The  */
/*                        other side's advertized window size controls    */
/*                        how much we can send.                           */

#define DIAG_SECTION_KERNEL DIAG_SECTION_TCP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

#if (INCLUDE_TCP)

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_SWS               0
#define DEBUG_SWS_RSLT          0
#define DEBUG_WINDOW            0
#define DEBUG_DACK              0       /* defined in tcp.c also */
#define DISPLAY_NO_SEND_ALIGN   0

/* ********************************************************************   */
/* DEFINES WHICH CONTROL PROCESSING - DO NOT CHANGE                       */
/* ********************************************************************   */
#define NEW_FIN 1  /* experimental code */
#define NXT_DCU 1  /* must match value in tcp.c */

#if (BUILD_NEW_BINARY)
#define DO_PUSH 0
#else
#define DO_PUSH 0       /* if set will only set PUSH flag in output message */
                        /* if this segment is the last segment in the   */
                        /* output window; if this is not set PUSH will  */
                        /* be set with every output packet which has    */
                        /* data - tbd                                   */
#endif


/* ********************************************************************   */
static int transf(PIFACE pi, PTCPPORT port, DCU msg,    
                  TCP_OUT_PARAMS *params, int dcu_flags);
void tcp_options_format(PTCPPORT port, DCU msg, byte tcp_flags_to_send);

/* ********************************************************************   */
/* calc_n_to_send() - calculate amount of data to send                    */
/*                                                                        */
/*   Calculate how much data in the output window should be sent.         */
/*   Amount to send is the minimum of the following:                      */
/*       - remote window size                                             */
/*       - number unsent bytes in output window                           */
/*       - congestion window                                              */
/*                                                                        */
/*   Returns number of bytes to send.                                     */
/*                                                                        */
word calc_n_to_send(PTCPPORT port, PWINDOW wind, int msg_type)  /*__fn__*/
{
word    bites;      /* # bytes to send */
word    wind_size;  /* othersides window size */
#if (INCLUDE_CWND_SLOW)
int     cwnd;
#endif
word    n_to_send;

    /* find out how many bytes the other side will allow us to send (window)   */
    wind_size = wind->size;   

#if (INCLUDE_CWND_SLOW)
    /* limit amount to send by the congestion window size where the         */
    /* amount of unacked data is subtracted from the congestion window size */
    cwnd = port->cwnd;
    cwnd -= (int)(wind->nxt_to_send - wind->nxt);
    if (cwnd < 0)
        cwnd = 0;

    if ((int)wind_size > cwnd)
    {
        wind_size = (word)cwnd;
    }
#endif

    DEBUG_LOG("calc_n_to_send - other side will accept bytes = ", LEVEL_3, 
        EBS_INT1, wind->size, 0);
    DEBUG_LOG("         bytes in output window = ", LEVEL_3, 
        EBS_INT1, wind->contain, 0);
    DEBUG_LOG("        wind->nxt_to_send, wind->nxt = ", LEVEL_3, LINT2,
        wind->nxt_to_send, wind->nxt);

    /* if less bytes in output window which need to be sent than other side   */
    /* will accept, send all available bytes                                  */
    n_to_send = ((word)(wind->contain - wind->nxt_to_send + wind->nxt));
    bites = wind_size;

    if (n_to_send < bites)  
        bites = n_to_send;

    DEBUG_LOG("    n_to_send, bites = ", LEVEL_3, EBS_INT2,  n_to_send, bites);

    /* **************************************************                    */
    /* if window probe, send 1 byte no matter what the othersides advertises */
    /* for their window size                                                 */
    if (msg_type == WINDOW_PROBE)
    {
        /* just double check we have a byte to send; if not force   */
        /* sending a packet with no data                            */
        if (port->out.contain)
            bites = 1;
        else
            bites = 0;

        DEBUG_ERROR("calc_n_to_send - send window probe - bites, out.contain = ", 
            EBS_INT2, bites, port->out.contain);
    }
    return(bites);
}

/* ********************************************************************   */
/* OUTPUT ROUTINES                                                        */
/* ********************************************************************   */
/* get_new_out_pkt() - allocate a new DCU and set up defaults             */
/*                                                                        */
/*   Allocates and returns a new DCU with the header field from           */
/*   the DCU template in the port structure copied to it.                 */
/*                                                                        */
/*   NOTE: the only flag in tcp_flags_to_send which this routine          */
/*         (actually that tcp_options_format() is interested in           */
/*         is the SYNC bit); if any other is used check all calls         */
/*         to this routine                                                */
/*                                                                        */
/*   Returns the allocated DCU or 0 if none available.                    */
/*                                                                        */

DCU get_new_out_pkt(PTCPPORT port, byte tcp_flags_to_send, int size)   /*__fn__*/
{
DCU msg;

    /* **************************************************         */
    /* get DCU to use                                             */
    /* After it is sent out on the wire it will be deallocated by */
    /* tc_release_message since it will be set to NO_KEEP         */
    if ( (msg = ip_alloc_packet(port->ap.iface, (PANYPORT)port, 
                                size, 0,
                                TCP_OUTPUT_ALLOC)) == (DCU)0 )
    {
        DEBUG_ERROR("ERROR(rtip) - get_new_out_pkt - ip_alloc_packet failed", NOVAR, 0, 0);
        return((DCU)0);
    }

    /* copy packet info from template to new packet (ethernet, IP and TCP   */
    /* headers)                                                             */
    /* tbd: IP options                                                      */
    tc_movebytes((PFBYTE)DCUTOETHERPKT(msg), 
                 (PFBYTE)&(port->out_eth_temp), 
                 ETH_HLEN_BYTES);
#if (INCLUDE_IPV4)
    tc_movebytes((PFBYTE)DCUTOIPPKT(msg),
                 (PFBYTE)&(port->out_ip_temp), 
                 IP_HLEN_BYTES);
#else
    tbd
#endif
    tc_movebytes((PFBYTE)DCUTOTCPPKT(msg),
                (PFBYTE)&(port->out_template), 
                sizeof(TCPPKT));

    /* **************************************************            */
    /* setup any tcp output options (sets up port->tcp_option_len to */
    /* be length of options)                                         */
    tcp_options_format(port, msg, tcp_flags_to_send);

    /* save the options length used when formatted packet   */
    DCUTOPACKET(msg)->tcp_option_len = port->tcp_option_len;

    return(msg);  
}
    

/* ********************************************************************   */
/* TRANSQ                                                                 */
/* ********************************************************************   */

/* ********************************************************************   */
/* check_sws_send() - checks if silly window syndrome for sender          */
/*                                                                        */
/*   Returns TRUE if should send data and FALSE is should not due         */
/*   to silly window syndrome or NAGLE algorithm.                         */

RTIP_BOOLEAN check_sws_send(PTCPPORT port, word send_this_msg)  /*__fn__*/
{
long    unacked_data;
RTIP_BOOLEAN ret_val;

    /* if closesocket called, don't wait to send data since no more   */
    /* data will be queued                                            */
    if (port->ap.port_flags & API_CLOSE_DONE) 
        return(TRUE);

    ret_val = FALSE;

    send_this_msg += (word)(port->tcp_option_len + port->ap.ip_option_len);
    unacked_data = SUB_SEGMENT(port->out.nxt_to_send, port->out.nxt);

    /* if there is no outstanding data to be acked, always send   */
    if (unacked_data == 0)
        ret_val = TRUE;

    /* full size data seg can be sent   */
    if (send_this_msg >= port->out.mss) 
    {
#if (DEBUG_SWS)
        DEBUG_ERROR("check_sws_send: can send since full size packet",
            NOVAR, 0, 0);
#endif
        ret_val = TRUE;
    }

    /* if we can send at least 1/2 othersides maximum window size   */
    else if (send_this_msg >= (port->out.max_size >> 1) )
    {
#if (DEBUG_SWS)
        DEBUG_ERROR("check_sws_send: can send since 1/2 other window size",
            NOVAR, 0, 0);
#endif
        ret_val = TRUE;
    }

    /* if (we can send everything we have)    */
    else if ((word)(port->out.contain-unacked_data) <= port->out.size) 
    {
#if (DEBUG_SWS)
        DEBUG_ERROR("check_sws_send: can send since can send everything",
            NOVAR, 0, 0);
#endif
        ret_val = TRUE;
    }

    /* NAGLE ALGORITHM : do not send small data packets while there     */
    /* is outstanding small data packet which has not been acknowledged */
    /* NOTE: NAGLE ALGORITHM implemented by unacked_data checks         */
    /* calculated amount of unacknowledged data                         */
#if (NAGLE_STEVENS)
    if (port->ap.options & SO_NAGLE)
    {
        /* if latest small packet has been acked, then there is   */
        /* no outstanding small packet                            */
        if (port->small_unacked)
        {
            if (CHECK_GREATER_EQUAL(port->out.nxt, port->small_unacked))
                port->small_unacked = 0;
        }

        /* check if shouldn't send due to NAGLE   */
        if ((send_this_msg < port->out.mss) &&  /* non-full size seg */
            (port->small_unacked) )             /* there is non-full size */
                                                /* outstanding   */
        {
#if (DEBUG_SWS)
            DEBUG_ERROR("check_sws_send: can't send NAGLE(STEVENS)",
                NOVAR, 0, 0);
#endif
            ret_val = FALSE;
        }

        /* if going to send a small packet, save the sequence number   */
        /* of the ack value which when received from the remote        */
        /* will acknowledge the packet                                 */
        if (ret_val && (send_this_msg < port->out.mss)) 
        {
            port->small_unacked = port->out.nxt_to_send + send_this_msg;
        }
    }
#else
    if ( (unacked_data != 0)                &&  /* unacked data */
         ((long)port->out.contain-unacked_data < 
          (long)port->out.mss) &&               /* not a seg worth to send */
         (port->ap.options & SO_NAGLE) )     /* NAGLE enabled */
    {
#if (DEBUG_SWS)
        DEBUG_ERROR("check_sws_send: can't send NAGLE: amount to send(must be MSS) ",
            EBS_INT1, port->out.contain-unacked_data, 0);
        DEBUG_ERROR("check_sws_send: can't send NAGLE: contain, unacked data ",
            EBS_INT2, port->out.contain, unacked_data);
#endif
        ret_val = FALSE;
    }
#endif

    return(ret_val);
}

/* ********************************************************************   */
/* send_no_data() - send a TCP packets without data                       */
/*                                                                        */
/*   Send a packet with no data                                           */
/*                                                                        */
/*   Returns 0 if successful, errno if not                                */
/*                                                                        */
int send_no_data(PIFACE pi, PTCPPORT port, int dcu_flags, int msg_type,  /*__fn__*/
                 byte tcp_flags)  /*__fn__*/
{
DCU msg;
TCP_OUT_PARAMS params;
int  size;

    /* if only send pkt if data can be sent, then do not send anything   */
    if (msg_type == NORMAL_DATA_ONLY)
        return(0);

    TOTAL_TCP_HLEN_SIZE(size, port, pi, 0)
    if ( (msg = get_new_out_pkt(port, tcp_flags, size)) != (DCU)0 )
    {
        tcp_flags &= ~TCP_F_TPUSH;        /* else clear push - no data */
        params.dlen      = 0;
        params.tcp_flags = tcp_flags;
        params.msg_type  = msg_type;

        return(transf(pi, port, msg, &params, dcu_flags));   /* send it  */
    }
    return(ENOPKTS);
}

/* ********************************************************************   */
#if (DEBUG_WINDOW)
void log_it(PWINDOW wind, int bites, int msg_type, word send_this_msg)
{
    DEBUG_ERROR("log_it - other side will accept bytes, bites = ", 
        EBS_INT2, wind->size, bites);
    DEBUG_ERROR("         bytes in output window = ", EBS_INT1, wind->contain, 0);
    DEBUG_ERROR("        wind->nxt_to_send, wind->nxt = ", LEBS_INT2,
            wind->nxt_to_send, wind->nxt);
    DEBUG_ERROR("msg_type == ", EBS_INT1, msg_type, 0);
    DEBUG_ERROR("log_it() - send bytes", EBS_INT1, send_this_msg, 0); 
}
#endif

/* ********************************************************************      */
/* transq() - send a TCP packets with data from output window                */
/*                                                                           */
/*   Transmit the entire queue (window) or the last part of window depending */
/*   upon msg_type to the other host without expecting any sort of           */
/*   acknowledgement. bites is the total number of bytes to send.            */
/*   transq() always sends ACK,ack.                                          */
/*                                                                           */
/*   Returns 0 if successful, errno if not                                   */
/*                                                                           */
static int transq(PIFACE pi, PTCPPORT port, int dcu_flags, int msg_type, 
                  byte tcp_flags, word bites)  /*__fn__*/
{
long    send_this_msg;      /* max size is word but make long so can */
                            /* do signed arithmetic   */
long    i;   
int     rest_buffer;
PWINDOW wind;
PFBYTE  nxtb, baseb, baseout;
DCU     nxt_dcu;
TCP_OUT_PARAMS params;
DCU     msg;
int     status;
RTIP_BOOLEAN no_copy;
int     data_end;
#if (INCLUDE_TCP_NO_COPY)
int     hlen;
POS_LIST entry_info;
#endif
#if (RTIP_VERSION >= 26)
long    size;
#endif

    /* **************************************************   */
    tcp_flags |= TCP_F_ACK;         

    wind = (PWINDOW)&port->out;

    /* **************************************************               */
    /* TRANSMIT DATA LOOP                                               */
    /* in a loop, transmit what the otherside will accept up to the     */
    /* entire queue of data                                             */
    /* NOTE: bites is total amount to sent limited by what is available */
    /*       to send and by the othersides window size                  */
#if (!DO_PUSH)
    tcp_flags |= TCP_F_TPUSH;
#endif
    for (i = 0; i < (long)bites; i = i+send_this_msg)
    {
        send_this_msg = tcp_pkt_data_max(port);

        DEBUG_LOG("transq() - bytes, seq #", LEVEL_3, DINT2, 
            send_this_msg, wind->nxt_to_send); 

        /* limit amount to send by what is left   */
        if (i+send_this_msg > (long)bites)     
            send_this_msg = bites - i;  

        /* if cannot send this segment due to silly window syndrome   */
        /* or nagle algorithm then we are done                        */
        if ( (msg_type == NORMAL_DATA) || (msg_type == NORMAL_DATA_ONLY) )
        {
            if (!check_sws_send(port, (word)send_this_msg))
            {
#if (DEBUG_SWS_RSLT)
                DEBUG_ERROR("transq: don't send due to SWS", NOVAR, 0, 0);
#endif
                return(0);
            }
#if (DEBUG_SWS_RSLT)
            DEBUG_ERROR("transq: DO send even with to SWS", NOVAR, 0, 0);
#endif

            /* if streaming data, only send full sized packets   */
            if ( (port->ap.options & SO_TCP_STREAM) &&
                 ((word)send_this_msg < port->out.mss) )
            {
                DEBUG_ERROR("transq: don't send cuz STREAMING", NOVAR, 0, 0);
                return(0);      /* tbd - need to start timer? */
            }
        }

        /* amt of data from start of data to end of DCU   */
        if (!wind->nxt_to_send_dcu)
        {
            DEBUG_ERROR("transq - nxt_to_send_dcu = 0", NOVAR, 0, 0);
#if (DEBUG_WINDOW)
            log_it(wind, bites, msg_type, (word)send_this_msg);
#endif
            return(send_no_data(pi, port, dcu_flags, msg_type, tcp_flags));
        }

#if (DO_PUSH)
        if (wind->nxt_to_send+send_this_msg >= wind->nxt+port->out.contain)
            tcp_flags |= TCP_F_TPUSH;
#endif

#if (!NXT_DCU)
        /* if used up all the data in nxt_to_send DCU then get the   */
        /* next one                                                  */
        data_end = DCUTOPACKET(wind->nxt_to_send_dcu)->data_start +
                   DCUTOPACKET(wind->nxt_to_send_dcu)->data_len;
        if (wind->nxt_to_send_off >= data_end)
        {
            wind->nxt_to_send_dcu = nxt_dcu;
            if (nxt_dcu)
                wind->nxt_to_send_off = DCUTOPACKET(nxt_dcu)->data_start;
            else
                wind->nxt_to_send_off = 0;
        }
#endif

        /* calc new start of data to send    */
        baseb = DCUTODATA(wind->nxt_to_send_dcu) + wind->nxt_to_send_off;

        /* calc amount of data in current window DCU to send   */
        rest_buffer = DCUTOPACKET(wind->nxt_to_send_dcu)->data_len -
                      wind->nxt_to_send_off +
                      DCUTOPACKET(wind->nxt_to_send_dcu)->data_start;

        nxt_dcu = (DCU)
            os_list_next_entry_off((POS_LIST)wind->dcu_start, 
                                   (POS_LIST)wind->nxt_to_send_dcu,
                                   TCP_WINDOW_OFFSET);

#if (INCLUDE_TCP_NO_COPY)
        /* ******   */
        if (port->ap.options & SO_TCP_NO_COPY)
        {
            /* only send what can in current packet to resync on pkt   */
            /* boundary - could happen if window size on othersize is  */
            /* less than MSS                                           */
            if ((long)rest_buffer < send_this_msg) 
                send_this_msg = (long)rest_buffer;

            /* if already aligned on packet but not sending the whole        */
            /* packet then do not send now - wait for output to empty        */
            /* so can try to send whole packet then - if the user specified  */
            /* packets then in the long run, it is faster to stay packet     */
            /* aligned (tbd - possibly make this an option)                  */
            if (msg_type != WINDOW_PROBE)
            {
                TOTAL_TCP_HLEN_SIZE(hlen, port, pi, 0)
                if ( (wind->nxt_to_send_off == hlen) &&
                     ((long)(DCUTOPACKET(wind->nxt_to_send_dcu)->data_len) > 
                      send_this_msg) )
                {
#if (DISPLAY_NO_SEND_ALIGN)
                    DEBUG_ERROR("transq: don't send cuz NO COPY-align", 
                        EBS_INT2,send_this_msg
#endif
                    return(0);      /* do not send at all */
                }
            }
        }       /* end of no copy */
#endif  /* INCLUDE_TCP_NO_COPY */

        /* ******                                                                  */
        /* get pkt to send -                                                       */
        /* NOTE: if sending a whole packet then just send it with no               */
        /*       copy + tell xmit not to free it                                   */
        /* NOTE: must be SO_TCP_NO_COPY mode else the packets in the output window */
        /*       do not have room for the header, therefore, they cannot           */
        /*       be sent directly                                                  */
        /* NOTE: if data is not at the beginning of the packet, then               */
        /*       copy it to a new one; this would be the case where                */
        /*       the othersides window size is less than MSS then                  */
        /*       the previous send would have only send part of the pkt;           */
        /*       cannot just move the data up in case it has not been acked        */
        msg = wind->nxt_to_send_dcu;
#if (INCLUDE_TCP_NO_COPY)
        TOTAL_TCP_HLEN_SIZE(hlen, port, pi, 0)
        if ( (port->ap.options & SO_TCP_NO_COPY) &&
             (wind->nxt_to_send_off == hlen) )
        {
            dcu_flags |= PKT_FLAG_KEEP; /* do not free packet in case */
                                        /* retransmit, will be freed     */
                                        /* when all data in packet       */
                                        /* has been acked (see check_ack */
                                        /* and rmqueue)                  */

            /* Check to make sure the packet is not already on     */
            /* the output list; this could happen since the socket */
            /* is in no copy mode and if a retransmit occurs       */
            /* before the packet is actually output then           */
            /* this will corrupt output list                       */
            entry_info = POS_ENTRY_OFF(msg, OUTPUT_LIST_OFFSET);
            if (entry_info->pnext != 0 || entry_info->pprev != 0)
            {
                DEBUG_ERROR("transq: DID NOT SEND SINCE IN OUTPUT LIST",
                    DINT1, msg, 0);
                return(0);
            }

            no_copy = TRUE;

            /* NOTE: options are not supported for NO_COPY (tbd)   */
/*          port->tcp_option_len = 0;                              */
        }
        else
#endif          /* INCLUDE_TCP_NO_COPY */
        {
            no_copy = FALSE;

#if (RTIP_VERSION >= 26)
            TOTAL_TCP_HLEN_SIZE(size, port, pi, 0)
            size += send_this_msg;
            msg = get_new_out_pkt(port, tcp_flags, (int)size);
#else
            msg = get_new_out_pkt(port, tcp_flags, 
                                  (int)(TCP_TLEN_BYTES + 
                                        port->ap.ip_option_len + 
                                        port->tcp_option_len + 
                                        send_this_msg));
#endif
            if (!msg)
                break;
        }

        /* ******                                                          */
        /* COPY DATA (if needed) AND UPDATE DATA POINTERS TO OUTPUT WINDOW */

        /* find start of data area taking in account options length   */
        TOTAL_TCP_HLEN_SIZE(size, port, pi, 0)
        baseout = DCUTODATA(msg) + (int)size;

        /* if data crosses a DCU                                                 */
        /* NOTE: data can never cross more than 1 DCU boundary since             */
        /*       - if SO_NO_COPY will only send current packet                   */
        /*       - if not SO_NO_COPY then MTU value must be larger               */
        /*         than packet size in DCU (MAX_PACKETSIZE) adjusted by headers; */
        /*         if (MAX_PACKETSIZE < MTU+header than will have problems       */
        /*         receiving packets anyway                                      */
        if ((long)rest_buffer < send_this_msg)   
        {
            DEBUG_LOG("transq - cross DCU boundry - rest_buffer, send_this_msg = ", 
                LEVEL_3, EBS_INT1, rest_buffer, send_this_msg);

            if (!nxt_dcu)
            {
                DEBUG_ERROR("transq_out - nxt_dcu = 0", NOVAR, 0, 0);
#if (DEBUG_WINDOW)
                log_it(wind, bites, msg_type, send_this_msg);
#endif
#if (INCLUDE_TCP_COPY)
                if (!no_copy)
                    os_free_packet(msg);
#endif
                DEBUG_ERROR("transq: don't send cuz no nxt_dcu", NOVAR, 0, 0);
                return(send_no_data(pi, port, dcu_flags, msg_type, tcp_flags));
            }

            nxtb = DCUTODATA(nxt_dcu) + DCUTOPACKET(nxt_dcu)->data_start;

            tc_movebytes(baseout, baseb, rest_buffer);
                                               /* write to end of window   */
            tc_movebytes(baseout+rest_buffer, nxtb, 
                         (int)(send_this_msg-rest_buffer)); 

            /* update pointers to window past current data sending;    */
            /* NOTE: should never be window probe which is 1 byte but  */
            /*       just to be safe (1 byte shouldn't cross boundary) */
            if (msg_type != WINDOW_PROBE)   
            {
                wind->nxt_to_send_dcu = nxt_dcu;
                wind->nxt_to_send_off = 
                    (int)(DCUTOPACKET(nxt_dcu)->data_start +
                          send_this_msg-rest_buffer);
            }
        }

        /* if all data to send is contiguous   */
        else                      
        {
#if (INCLUDE_TCP_COPY)
            if (!no_copy)
                tc_movebytes(baseout, baseb, (word)send_this_msg);
#endif

            if (msg_type != WINDOW_PROBE)
            {
                wind->nxt_to_send_off = (int)(wind->nxt_to_send_off + 
                                              send_this_msg);

#if (NXT_DCU)
                /* if used up all the data in nxt_to_send DCU then get the   */
                /* next one                                                  */
                data_end = DCUTOPACKET(wind->nxt_to_send_dcu)->data_start +
                           DCUTOPACKET(wind->nxt_to_send_dcu)->data_len;
                if (wind->nxt_to_send_off >= data_end)
                {
                    wind->nxt_to_send_dcu = nxt_dcu;
                    if (nxt_dcu)
                        wind->nxt_to_send_off = DCUTOPACKET(nxt_dcu)->data_start;
                    else
                        wind->nxt_to_send_off = 0;
                }
            }
#endif
        }

#if (INCLUDE_TCP_RTT)
        /* ******                                                       */
        /* SETUP FOR RTT MEASUREMENT                                    */
        /* if not already measuring and not a retrans, measure this pkt */
        if ( !port->rtt_nxt && (msg_type != RETRANS) && 
             (msg_type != WINDOW_PROBE) && (msg_type != FAST_RETRANS) )
        {
            port->rtt_nxt = port->out.nxt_to_send;
            port->rtttime = 0;
        }
#endif

        /* ******                                              */
        /* START RETRANSMIT TIMER if it is not already running */
        if (msg_type != WINDOW_PROBE)
            start_retrans_timer(port);

        /* ******                                                           */
        /* TRANSMIT THE CURRENT PKT and UPDATE nxt_to_send SEQUENCE NUMBERS */
        /* and OTHERSIDES ACTUAL WINDOW SIZE                                */
        params.dlen      = (word)send_this_msg;
        params.tcp_flags = tcp_flags;
        params.msg_type  = msg_type;
        status = transf(pi, port, msg, &params, dcu_flags);

        if (msg_type != WINDOW_PROBE)
        {
            /* NOTE: do not update seq number and output window size until   */
            /*       transf() is called; and do not check status and return  */
            /*       until they have been updated or will corrupt output     */
            wind->nxt_to_send += send_this_msg;  /* seq # to send nxt (used */
                                                 /* by transf)   */
            port->out.size = (word)(port->out.size - send_this_msg);     
                                    /* adjust what we know othersides   */
                                    /* window size is since it will     */
                                    /* be awhile until the new window   */
                                    /* size comes in                    */
            if (wind->nxt_to_send > wind->max_nxt_to_send)
                wind->max_nxt_to_send = wind->nxt_to_send;

            /* if waiting to start the close on our side until all the   */
            /* data has been segmentized, check if can start it now      */
            /* that nxt_to_send has been updated                         */
            if (port->ap.port_flags & WAIT_CLOSE_SEG)
            {
                DEBUG_LOG("WAIT_CLOSE_SEG set: sent data - call close", LEVEL_3, NOVAR, 0, 0);
                if (port->ap.port_flags & WAIT_CLOSE_SEG)
                {
                    DEBUG_LOG("WAIT_CLOSE_SEG set: sent data - call close", LEVEL_3, NOVAR, 0, 0);
                    if (port->ap.port_flags & WRITE_SHUTDOWN)
                        shutdown(port->ap.ctrl.index, 1);
                    else
                        tcp_close(port);
                }
            }
            else
            {
                DEBUG_LOG("sent data - flag not set => dont close", 
                    LEVEL_3, NOVAR, 0, 0);
            }
        }

        /* if there was an error, do not continue   */
        if (status)
        {
            DEBUG_ERROR("transq - transf failed", NOVAR, 0, 0);
            DEBUG_ERROR("transq: don't send cuz status=", EBS_INT1, status, 0);
            return(status);
        }

        /* if doing a fast retrans, only transmit one data packet   */
        if (msg_type == FAST_RETRANS)
            break;
    }       /* end of TRANSMIT DATA for loop */

    return(0);                        /* success */
}

/* ********************************************************************   */
void calc_tcp_options_len(PTCPPORT port, byte tcp_flags_to_send)
{
    port->tcp_option_len = 0;

    if (tcp_flags_to_send & TCP_F_SYN)
    {
        /* Send our maximum segment size   */
        port->tcp_option_len += 4;
    }

#if (INCLUDE_TCP_TIMESTAMP)
    /* respond to a timestamp   */
    if (port->ap.port_flags & GOT_TIMESTAMP)
    {
        port->tcp_option_len += 10;
    }

    /* initiate a timestamp   */
    else if ( (port->ap.options & SO_TCP_TIMESTAMP)  &&
              ((port->ap.port_flags & GOT_TIMESTAMP_SYNC) ||
               (tcp_flags_to_send & TCP_F_SYN)) )
    {
        port->tcp_option_len += 10;
    }
#endif

    /* pad with NOP to put on mod 4 boundary (tbd)   */
    while (port->tcp_option_len % 4)
    {
        port->tcp_option_len += 1;
    }
}

void tcp_options_format(PTCPPORT port, DCU msg, byte tcp_flags_to_send)
{
PFBYTE pb;

    port->tcp_option_len = 0;
    pb = (PFBYTE)DCUTOTCPPKT(msg) + TCP_HLEN_BYTES;

    if (tcp_flags_to_send & TCP_F_SYN)
    {
        /* Send our maximum segment size   */
        *pb = TCP_MSS_OPTION;   /* kind of option - i.e. max seg size */
        *(pb+1) = 4;            /* length of option */
        *((PFWORD)(pb+2)) = hs2net(port->in.mss);  
                                /* put max seg size in msg   */
        port->tcp_option_len += 4;
        pb += 4;
    }

#if (INCLUDE_TCP_TIMESTAMP)
    /* respond to a timestamp   */
    if (port->ap.port_flags & GOT_TIMESTAMP)
    {
        port->tcp_option_len += 10;
        *pb = TCP_TIMESTAMP_OPTION;
        *(pb+1) = 10;
        LONG_2_WARRAY((PFWORD)(pb+2), hl2net(ks_get_ticks()));
        tc_mv4(pb+6, (PFBYTE)&(port->tsvalue), 4);
                                            /* reply with timestamp from   */
                                            /* input packet                */
        pb += 10;
    }

    /* initiate a timestamp   */
    else if ( (port->ap.options & SO_TCP_TIMESTAMP)  &&
              ((port->ap.port_flags & GOT_TIMESTAMP_SYNC) ||
               (tcp_flags_to_send & TCP_F_SYN)) )
    {
        port->tcp_option_len += 10;
        *pb = TCP_TIMESTAMP_OPTION;
        *(pb+1) = 10;
        LONG_2_WARRAY((PFWORD)(pb+2), hl2net(ks_get_ticks()));
        long_to_bytes((PFDWORD)(pb+6), 0);      /* *((PFDWORD)(pb+6)) = 0; */
        pb += 10;
    }
#endif

    /* pad with NOP to put on mod 4 boundary (tbd)   */
    while (port->tcp_option_len % 4)
    {
        *pb = TCP_NOP_OPTION;
        port->tcp_option_len += 1;
        pb += 1;
    }
}

/* ********************************************************************   */
/* tc_tcpsend() - setup and send TCP packet (top level)                   */
/*                                                                        */
/*   Returns 0 if successful, errno if not                                */
/*                                                                        */
int tc_tcpsend(PIFACE pi, PTCPPORT port, int dcu_flags,   /*__fn__*/
               int msg_type, byte tcp_flags)              /*__fn__*/
{
TCP_OUT_PARAMS params;
DCU  msg;
byte tcp_flags_to_send;
int  ret_val;
word bites;         /* # bytes to send */
RTIP_BOOLEAN send_fin;
#if (!NEW_FIN)
dword save_nts;
#endif
PWINDOW wind;
int  size;

    wind = (PWINDOW)&port->out;

    /* calculate flags to send; in order to send SYNC or FIN it must   */
    /* be specified by caller or be a retrans                          */
    tcp_flags_to_send = tcp_flags_state[port->state];
    if ( (msg_type != RETRANS) && (msg_type != FAST_RETRANS) )
    {
        /* not retransmit due to timeout, turn off SYNC and FIN bits   */
        tcp_flags_to_send &= (~TCP_F_SYN);
        tcp_flags_to_send &= (~TCP_F_FIN);
        INCR_SNMP(TcpOutSegs)
    }
#if (INCLUDE_SNMP)
    else
    {
        INCR_SNMP(TcpRetransSegs)
    }
#endif
    INCR_SNMP(IpOutRequests)

    tcp_flags_to_send |= tcp_flags;  /* turn on any flags specified by caller */

#if (INCLUDE_SNMP)
    if (tcp_flags_to_send & TCP_F_RESET)
    {
        INCR_SNMP(TcpOutRsts)
    }
#endif

    /* set size of TCP options to send   */
    calc_tcp_options_len(port, tcp_flags_to_send);

    /* EST and CWAIT are only states which can send data;          */
    /* NOTE: data can be queued in SYNS and SYNR states but cannot */
    /*       be sent until in EST state                            */
    if ( ((port->state == TCP_S_EST)     ||
          (port->state == TCP_S_CWAIT))  &&
         ((msg_type == NORMAL_DATA_ONLY) ||
          (msg_type == NORMAL_DATA)      ||
          (msg_type == RETRANS)          ||
          (msg_type == FAST_RETRANS)     ||
          (msg_type == WINDOW_PROBE)) )
    {
        /* **************************************************   */
        /* CALCULATE NUMBER OF BYTES TO SEND                    */
        bites = calc_n_to_send(port, wind, msg_type);

        /* **************************************************                */
        /* if no data to send or no room in othersides input window,         */
        /* send ACK and return                                               */
        /* NOTE: only can send data in EST or CWAIT state                    */
        /* NOTE: if no room in otherside's input window but we have data to  */
        /*       send, check_ack will send the data when the otherside sends */
        /*       us a nonzero window size                                    */
        if ( (bites == 0) || 
             ((port->state != TCP_S_EST) && (port->state != TCP_S_CWAIT)) )
        {
            DEBUG_LOG("tc_tcpsend() - send 0 bytes - either cuz 0 or state", 
                LEVEL_3, EBS_INT2, bites, wind->contain);
            if (msg_type == NORMAL_DATA_ONLY)
                return(0);
        }
        else
        {
            /* if the FIN bit is set, send 2 packets, one with data and the   */
            /* second with FIN bit set; this is done to ensure the FIN        */
            /* bit has the correct seq #                                      */
            send_fin = FALSE;
            if (tcp_flags_to_send & TCP_F_FIN)
            {
                send_fin = TRUE;
                tcp_flags_to_send &= ~TCP_F_FIN;
            }
            ret_val = transq(pi, port, dcu_flags, msg_type, tcp_flags_to_send, 
                             bites);
            if (!send_fin)
                return(ret_val);
            tcp_flags_to_send |= (TCP_F_FIN | TCP_F_ACK);
        }
    }
    
    /* send no data   */
    TOTAL_TCP_HLEN_SIZE(size, port, pi, 0)
    if ( (msg = get_new_out_pkt(port, tcp_flags_to_send, size)) != (DCU)0 )
    {
#if (!NEW_FIN)
        /* if the FIN bit is set, make sure the sequence number is the   */
        /* first one never set; if a graceful close with timeout or a    */
        /* hard close is done, nxt_to_send might not be the correct seq  */
        /* number for the FIN bit                                        */
        save_nts = port->out.nxt_to_send;
        if (tcp_flags_to_send & TCP_F_FIN)
        {
            DEBUG_LOG("set fin:nts, max_nts = ", LEVEL_3, DINT2, 
                port->out.nxt_to_send, port->out.max_nxt_to_send);

            port->out.nxt_to_send = port->out.max_nxt_to_send;
        }
#endif
        params.dlen      = 0;
        params.tcp_flags = tcp_flags_to_send;
        params.msg_type  = msg_type;
        ret_val = transf(pi, port, msg, &params, NO_DCU_FLAGS);

#if (!NEW_FIN)
        /* set nxt_to_send back to orig val                           */
        /* NOTE: all data is segmatized before FIN is sent and no     */
        /*       more data may be queued after FIN is sent (due to    */
        /*       TCP state), therefore, max_nxt_to_send is always     */
        /*       the seq no for FIN bit and once FIN is sent, it will */
        /*       never be changed                                     */
        if (tcp_flags_to_send & TCP_F_FIN)
            port->out.nxt_to_send = save_nts;
#endif
    }

    else
        return(ENOPKTS);

    return(ret_val);
}

/* ********************************************************************     */
/* transf() - set up and send a TCP packet (any data already setup)         */
/*                                                                          */
/*   transmits a TCP packet.                                                */
/*    - sets IP ident,check,total len                                       */
/*    - sets TCP seq and window from port information,                      */
/*    - fills in the pseudo header and computes the checksum.               */
/*                                                                          */
/*   NOTE: assumes all values in output packet not passed as parameter      */
/*         were previously setup in msg; i.e. when packet allocated         */
/*         values from template were copied to message; template contains   */
/*         values which are fixed, i.e. from initialization or connect/bind */
/*                                                                          */
/*   Returns 0 if successful, errno if not                                  */
/*                                                                          */

static int transf(PIFACE pi, PTCPPORT port, DCU msg,         /*__fn__*/
                  TCP_OUT_PARAMS *params, int dcu_flags)      /*__fn__*/
{
PTCPPKT pt_out;
#if (INCLUDE_IPV4)
PIPPKT  pip_out;
int     ip_len;
#endif

    /* **************************************************   */
    pt_out  = DCUTOTCPPKT(msg);

    /* **************************************************                 */
    /* if SYN or FIN bit, restart timeout counter but not if sending from */
    /* timeout routine                                                    */
    /* NOTE: if hard close data will not be resent anyway due to state,   */
    /*       therefore, it is ok to reset timeout counter which is also   */
    /*       used for first byte in output window                         */
    if ( (params->tcp_flags & TCP_F_SYN) || (params->tcp_flags & TCP_F_FIN) )
    {
        if ( (params->msg_type != RETRANS) && 
             (params->msg_type != WINDOW_PROBE) &&
             (params->msg_type != FAST_RETRANS) )
        {
            start_retrans_timer(port);
        }
    }

    DEBUG_LOG("transf - dcu flags, tcp flags = ", LEVEL_3, 
                EBS_INT2, dcu_flags, params->tcp_flags);

    /* **************************************************       */
    /* send max seg size option if and only if it is a SYNC msg */
    if (params->tcp_flags & TCP_F_SYN)
    {
        /* set up input window size based upon device using to send   */
        /* first sync message                                         */
        setup_ws((PWINDOW)&port->in, pi->pdev->window_size_in, EBS_INPUT);
        port->credit = port->in.window_size;    /* Start with no choke */
                                                /* NOTE: do this after setup_ws   */
    }

#if (INCLUDE_IPV4)
    /* **************************************************                 */
    /* do IP header first                                                 */
    /* if bound to wild IP address and do not have a legitimate source    */
    /* address, put in a legitimate source address into                   */
    /* the outgoing msg; at this point all we know is the output iface    */
    /* so we will use that one; this could occur if we bound to wild then */
    /* sent the first SYNC; tbd would this be legal or even work          */
    pip_out = DCUTOIPPKT(msg);
    if ( (port->ap.port_flags & PORT_WILD) &&
         (tc_cmp4(pip_out->ip_src, ip_nulladdr, 4)) )
    {
        /* ip address   */
        tc_mv4(pip_out->ip_src, pi->addr.my_ip_addr, 4);
    }

    /* set up rest of header and perform checksum   */
    TOTAL_IP_TCP_SIZE(ip_len, port, pi, 0, params->dlen)
    setup_ipv4_header(msg, (PANYPORT)port, (word)ip_len, 0, FALSE, 0, TRUE);
#endif

    /* **************************************************   */
    /* set msg->length to total packet size                 */
    TOTAL_TCP_HLEN_SIZE(DCUTOPACKET(msg)->length, port, pi, 0)
    DCUTOPACKET(msg)->length += params->dlen;

    /* **************************************************   */
    /* source h/w address based upon output interface       */
    SETUP_LL_HDR(msg, pi, EIP_68K, SEND_802_2(pi, (PANYPORT)port),
                 DCUTOPACKET(msg)->length - LL_HLEN_BYTES);

    /* **************************************************   */
    /* do TCP header                                        */

    /* set up TCP header length;                                      */
    /* header includes 20 bytes+options (divided by 4 since words and */
    /* times 16 since need to shift left 4 to put in hi 4 bites, i.e. */
    /* multipy it by 4 (NOTE: it is guarenteed to have low 4 bites    */
    /* as zero after operation since padded to get on mod 4 boundary) */
    pt_out->tcp_hlen = (byte)((TCP_HLEN_BYTES+
                               DCUTOPACKET(msg)->tcp_option_len)<<2);

    if (params->msg_type == KEEPALIVE)
    {
        LONG_2_WARRAY(pt_out->tcp_seq, hl2net(port->out.nxt-1));  /* invalid seq #  */
    }
    else
    {
        LONG_2_WARRAY(pt_out->tcp_seq, hl2net(port->out.nxt_to_send));    
    }

    DEBUG_LOG("tcpsend - seq = ", LEVEL_3, DINT1, 
        (dword)net2hl(WARRAY_2_LONG(pt_out->tcp_seq)), 0);
    pt_out->tcp_flags = params->tcp_flags;

    /* set up the tcp packet for this, ACK field is same for all packets   */
    if (params->tcp_flags & TCP_F_ACK)
    {
        /* if write is shutdown do not ack any new data (shutdown saved       */
        /* nxt in nxt_to_send) but still ack a FIN (check_fin set nxt_to_send */
        /* back to 0)                                                         */
        if ( (port->ap.port_flags & READ_SHUTDOWN) && (port->in.nxt_to_send) )
        {
            LONG_2_WARRAY(pt_out->tcp_ack, hl2net(port->in.nxt_to_send));
        }
        else
            LONG_2_WARRAY(pt_out->tcp_ack, hl2net(port->in.nxt));
    }
    else
        LONG_2_WARRAY(pt_out->tcp_ack, 0L);

    DEBUG_LOG("tcpsend - tcp_ack = ", LEVEL_3, DINT1, port->in.nxt, 0);
    DEBUG_LOG("tcpsend - tcp_flags", LEVEL_3, EBS_INT1, pt_out->tcp_flags, 0);

    /* **************************************************          */
    /* if the port has some credit limit, use it instead of large  */
    /* window buffer.  Generally demanded by hardware limitations. */
    if (port->credit < port->in.ad_size)
        pt_out->tcp_window = net2hs(port->credit);
    else
        pt_out->tcp_window = net2hs(port->in.ad_size);

    DEBUG_LOG("setup window size: ad_size, credit", LEVEL_3, DINT2,
        port->in.ad_size, port->credit);
    DEBUG_LOG("setup window size: contain, pt_out->tcp_window", LEVEL_3, DINT2,
        port->in.contain, hs2net(pt_out->tcp_window));

    /* **************************************************   */
    pt_out->tcp_chk = 0;    
    pt_out->tcp_chk = tc_tcp_chksum(msg);

/*  DEBUG_LOG("  tcpsend() - tcp checksum = ", LEVEL_3, EBS_INT1, pt_out->tcp_chk, 0);   */
/*  DEBUG_LOG("  tcpsend() - calc checksum for out msg", 3, NOVAR, 0, 0);                */
/*  DEBUG_LOG("PACKET = ", PKT, (PFBYTE)pt_out, TCP_TLEN_BYTES);                         */

    /* **************************************************   */
    port->ap.port_flags &= ~RETRANS_EN;  /* don't retrans unless arp resolved */
#    if (DEBUG_DACK)
        DEBUG_ERROR("tcpsend - reset delayed ack, dactime, ticks", EBS_INT2, 
            port->dactime, ks_get_ticks());
#    endif
    port->interp_port_flags &= ~DELAYED_ACK;  /* since sending ack if there */
    port->interp_port_flags &= ~DELAYED_LAST; /* is a delayed ack pending */
                                              /* clear it   */

#if (NEW_FIN)
    if ( (params->tcp_flags & TCP_F_FIN) ||
         (params->tcp_flags & TCP_F_SYN) )
    {
        port->out.nxt_to_send++;        /* account for FIN/SYNC */
    }
#endif

    /* Send the packet   */
    return(tc_netwrite(pi, (PANYPORT)port, msg, port->ap.send_ip_addr, 
                       dcu_flags, (word)RTIP_INF));
}

#endif      /* INCLUDE_TCP */
