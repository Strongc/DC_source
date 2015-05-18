/*                                                                       */
/* TCP.C - TCP functions                                                 */
/*                                                                       */
/* EBS - RTIP                                                            */
/*                                                                       */
/* Copyright Peter Van Oudenaren , 1993                                  */
/* All rights reserved.                                                  */
/* This code may not be redistributed in source or linkable object form  */
/* without the consent of its author.                                    */
/*                                                                       */
/*  Module description:                                                  */
/*      This module contains TCP timer function and interpret functions. */


/* Functions in this module                                                  */
/* tc_tcp_timeout     -   Called once/second by tc_timer_main(). Does        */
/*                        retries and freeing of zombie sockets.             */
/* check_option       -   See if an incoming SYN specifies a maximum         */
/*                        segment size for the other side                    */
/* check_ack          -   Take an incoming ACK and remove all ACKED          */
/*                        bytes from our output buffer. If the output        */
/*                        buffer underflows wake up any task sending         */
/*                        on the port so it can refresh the output           */
/*                        buffer and or return status.                       */
/* check_data         -   Process a packet addressed to a open port in       */
/*                        the established, fw1 or fw2 states. If the port's  */
/*                        input buffer is not full and the packet contains   */
/*                        data, it is placed into the input buffer and a     */
/*                        wakeup is issued to the user in receive.           */
/* check_fin          -   Check if the other side is requesting a close of   */
/*                        this socket. If so ACK the FIN and begin close     */
/*                        processing.                                        */
/* tc_tcp_interpret   -   A tcp packet was received. Perform a check sum     */
/*                        and find the port to whom the packet should go.    */
/*                        Send a reset if no port is found otherwise call    */
/*                        tc_tcpdo to process the packet for the port.       */
/* tc_tcpdo           -   Implement the TCP state machine. (this is TCP)     */
/* tc_tcp_reset       -   Send a TCP packet back to its original source      */
/*                        with the reset flag set.                           */
/* enqueue            -   Adds n bytes to an input or output queue           */
/* dequeue            -   Copies up to n bytes from the input queue to a     */
/*                        user buffer. Release the bytes for further         */
/*                        enqueueing.                                        */
/* rmqueue            -   Purges N bytes from a queue.                       */
/* setupwindow        -   Sets up input and out put "WINDOW" structures      */
/*                        for a port. (contains queuing plus connection      */
/*                        sequence information)                              */
/* setup_both_windows -   allocates two windows to the port                  */
/* free_both_windows  -   free both windows belonging to the port            */

#define DIAG_SECTION_KERNEL DIAG_SECTION_TCP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"
/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#if (INCLUDE_MEASURE_PERFORMANCE)
extern RTIP_BOOLEAN do_sock_tcp_cache;
#endif

#define TEST_SEND_DUP_ACK 0
#define TEST_FAST_RETRANS 0
#if (TEST_FAST_RETRANS)
int test_fast_retrans = 0;
#endif
#define TCP_DROP_RANDOM 0       /* drop random packet (for testing) */

/* various defines to turn on some debug code    */
#define DEBUG_WEB   0
#define DEBUG_KA    1
#define DEBUG_DATA_QUE_IN 0
#define DEBUG_ENQUE 0
#define DEBUG_DEQUE 0
#define DEBUG_DACK  0       /* defined in tcpout.c also */
#define DEBUG_RTT   0
#define DEBUG_FR    0       /* debug FAST RETRANS */
#define DEBUG_OOO   0
#define PRINT_OOO   0       /* display out of order list */
#define DEBUG_FAST_TCP 0
#define DEBUG_RESET 0
#define DEBUG_ABORT_CONNECTION 0

#define DISPLAY_ROUTING_TABLE 0 /* display routing table if no route exists */
#define DISPLAY_TIMER 0     /* determines whether DEBUG_LOG should */
                            /* be called to display enter and exit from   */
                            /* timer routines                             */
#define DISPLAY_NORMAL 1    /* determines whether DEBUG_ERROR should */
                            /* be called for certain errors which may   */
                            /* not be actual errors                     */

#define DISPLAY_RETRANS  1  /* display retransmit messages */

#define DISPLAY_NO_MATCH 0  /* display packets which have no associated */
                            /* socket   */
#define DISPLAY_NO_MATCH_PORTS 0    
                            /* display sockets in system when   */
                            /* packets which have no associated */
                            /* socket                           */
#define DISPLAY_CHECK_SEQ 0 /* display results of check_seq (at least */
                            /* if invalid packet)   */
#define DISPLAY_SEND_WINDOW_PROBE 0
#define DISPLAY_MAX_INPUT_QUE 0


/* ********************************************************************   */
/* DEFINES WHICH CONTROL PROCESSING - DO NOT CHANGE                       */
/* ********************************************************************   */
#define NXT_DCU  1          /* must match value in tcpout.c */
#define TCP_WAIT_SEND 0     /* TCP: processes all input before sending */
                            /*      an output                       */
                            /* NOTE: must match same define in ip.c */

#if (DEBUG_WEB)
int listen_fail = 0;   
int listen_sock = 0;   
int return_listen = 0;
extern int web_count;
#endif

#if (DEBUG_FAST_TCP)
long num_fast_tcp = 0;
long num_non_fast_tcp = 0;
#endif

/* ********************************************************************   */
/* account for my FIN/SYN being acked                                     */
#define ACCOUNT_ONE_BYTE(port) {port->out.nxt++; port->out.ack++;}

    /* only retransmit if there is something to retransmit, i.e. a   */
    /* tcp flag or data where the othersize has room in its window   */
    /* NOTE: the remote host is allowed to transmit a zero window    */
    /*       side for as long as it wants and the local host should  */
    /*       not abort the connection                                */
#define CHECK_RETRANS(port) ( (tcp_flags_state[port->state] & TCP_F_SYN) || \
                              (tcp_flags_state[port->state] & TCP_F_FIN) || \
                              (port->out.ad_size && port->out.contain) )

#define DUP_ACK_FR 3        /* number dup acks before retrans; this */
                            /* is defined by RFCs   */

/* ********************************************************************   */
#if (!INCLUDE_TCP)
int tc_tcp_error(int rtip_errno)            /*__fn__*/
{
    if (rtip_errno > 0)
        set_errno(rtip_errno);
    return(-1);
}

int tc_tcp_error_zero(int rtip_errno)   /*__fn__*/
{
    if (rtip_errno > 0)
        set_errno(rtip_errno);
    return(0);
}

int tc_tcp_error_errno(int rtip_errno)  /*__fn__*/
{
    if (rtip_errno > 0)
        set_errno(rtip_errno);
    return(rtip_errno);
}

#else

/* ********************************************************************   */
/* local routine declarations                                             */
static void      do_window_probe(PIFACE pi, PTCPPORT port);
static RTIP_BOOLEAN   to_proc(PIFACE pi, PTCPPORT port);
static void      proc_keepalive(PIFACE pi, PTCPPORT port);
static void      return_listen_state(PTCPPORT port);
void             process_one_tcp_output(PTCPPORT port);
static void      calc_tcp_len(DCU msg, word *hlen, word *dlen, RTIP_BOOLEAN setup);
static void      tc_tcpdo(PIFACE pi, PTCPPORT port, DCU  msg);
#if (INCLUDE_TCP_OUT_OF_ORDER)
static INLINE void tc_tcpdo_other_ooo(PIFACE pi, PTCPPORT port, DCU msg,         
                               word dlen, word hlen, RTIP_BOOLEAN KS_FAR *queued);
#endif
static RTIP_BOOLEAN tc_tcpdo_other(PIFACE pi, PTCPPORT port, DCU  msg, word dlen, word hlen, 
                                RTIP_BOOLEAN KS_FAR *queued, RTIP_BOOLEAN out_of_order);
static void      tc_tcp_reset(PIFACE pi, PTCPPORT port, DCU in_msg);
#if (INCLUDE_TCP_COPY)
word             enqueue_data_in(PTCPPORT port, PFBYTE buffer, word nbytes);
#endif
int              enqueue_pkt(PWINDOW wind, DCU msg, int nbytes);
void             setupwindow(PWINDOW w);
#if (INCLUDE_TCP_OUT_OF_ORDER)
void             add_tcpdcu_list_order(DCU msg, PTCPPORT port, RTIP_BOOLEAN KS_FAR *queued);
void             free_ooo(PTCPPORT port);
#endif  /* INCLUDE_TCP_OUT_OF_ORDER */
#if (PRINT_OOO)
                 void print_ooo(PTCPPORT port);
#endif

/* ********************************************************************   */
/* TIMEOUT ROUTINES                                                       */
/* ********************************************************************   */

/* ********************************************************************      */
/* tc_tcp_timeout() - TCP timer processing                                   */
/*                                                                           */
/*  Periodic TCP timer. Called approximately once/CFG_TIMER_FREQ.            */
/*  Performs tcp retries and other time domain tcp processing.               */
/*                                                                           */
/*  Returns TRUE if this routine should be called again right away (i.e.     */
/*  port was taken off active list, therefore, not all ports were processed) */
/*  Returns FALSE if this routine does not need to be called until the next  */
/*  time out period.                                                         */
/*                                                                           */
RTIP_BOOLEAN tc_tcp_timeout(RTIP_BOOLEAN do_to_proc, RTIP_BOOLEAN do_sec_proc) /*__fn__*/
{
int      i;
PIFACE   pi;
PTCPPORT port;

    /* NOTE: any retransmitting of packets from this timeout routine            */
    /*       needs to alloc a new DCU to be used to send output data            */
    /*       instead of using one connected to port                             */
    /* NOTE: this routine looks at time_since_send in port out window structure */
    /*       to determine time when tcp packet was sent out.  It looks          */
    /*       at retrans_en to see if arp resolved (won't retransmit unless      */
    /*       it has been resolved)                                              */
#if (DISPLAY_TIMER)
    DEBUG_LOG("tc_tcp_active_timeout() entered", LEVEL_3, NOVAR, 0, 0);
#endif

    OS_CLAIM_TCP(TME_CLAIM_TCP)

    /* **************************************************   */
    /* CHECK FOR CLOSED SOCKETS                             */
    /* (ONCE PER SECOND PROCESSING)                         */
    /* **************************************************   */
    if (do_sec_proc)
    {
        for (i=FIRST_TCP_PORT; i < TOTAL_TCP_PORTS; i++)
        {
            port = (PTCPPORT)(alloced_ports[i]);
            if (!port || !IS_TCP_PORT(port))
                continue;

            if ( (port->state == TCP_S_CLOSED) && 
                    (port->ap.list_type != FREE_LIST) )
            {
                /* If the port was closed above or elsewhere, finish the close.   */
                /* i.e. possibly free resources,                                  */
                /* NOTE: cannot keep traversing list of ports since               */
                /*       current port might be taken off the list.                */
                /*       To keep it simple we return TRUE. This will cause        */
                /*       the timer task to re-enter the routine with this port    */
                /*       off the linked list of active ports                      */
                if (tcp_close(port) == PORT_FREED)  /* if freed the port */
                {
                    OS_RELEASE_TCP()    /* Release access to tcp resources */
                    return(TRUE);       /* call me again immediately */
                }                       /* NOTE: port will not be on linked */
            }                           /*       list when called again */

        }
    }

    /* **************************************************           */
    /* ACTIVE LIST PROCESSING                                       */
    /* **************************************************           */
    /* Perform timeout processing for each socket i.e. loop thru    */
    /* list of active ports                                         */
    /* NOTE: do not need to loop thru list of listener ports since  */
    /*       they are all in LISTEN state and no timeout processing */
    /*       is done for these ports                                */
    /* Note: This loop contains a continue                          */
    port = (PTCPPORT)(root_tcp_lists[ACTIVE_LIST]);
    while (port)
    {
#if (DISPLAY_TIMER)
        DEBUG_LOG("tc_tcp_active_timeout - active - state,index = ", LEVEL_3, EBS_INT2,  
            port->state, port->ap.ctrl.index);
#endif

        /* get iface associated with port - check the open count by ourselves   */
        /* this way we won't print unneeded warnings                            */
        pi = port->ap.iface;
        if (!pi || !pi->open_count)
        {
            port = (PTCPPORT)
                os_list_next_entry_off(root_tcp_lists[ACTIVE_LIST], 
                                       (POS_LIST)port, ZERO_OFFSET);
            continue;
        }

        /* **************************************************   */
        /* ONCE PER SECOND PROCESSING                           */
        /* **************************************************   */
        if (do_sec_proc)
        {
            /* NOTE: any retransmitting of packets from this timeout routine            */
            /*       needs to alloc a new DCU to be used to send output data            */
            /*       instead of using one connected to port                             */
            /* NOTE: this routine looks at time_since_send in port out window structure */
            /*       to determine time when tcp packet was sent out.  It looks          */
            /*       at retrans_en to see if arp resolved (won't retransmit unless      */
            /*       it has been resolved)                                              */
#if (DISPLAY_TIMER)
            DEBUG_LOG("tc_tcp_sec_timeout() entered", LEVEL_3, NOVAR, 0, 0);
#endif

    /* **************************************************   */
#if (DISPLAY_TIMER)
            DEBUG_LOG("tc_tcp_sec_timeout - active - state,index = ", LEVEL_3, EBS_INT2,  
                port->state, port->ap.ctrl.index);
#endif

            /* increment timeout counters   */
            if ( (port->state == TCP_S_TWAIT) || 
                 (port->state == TCP_S_LAST)  ||
                 (port->state == TCP_S_CLOSED) )
                port->closetime++;

            /* **************************************************   */
            /* if 0 window size, do window probing                  */
            do_window_probe(pi, port);

            /* **************************************************        */
            /* If we are in LAST, we ACK/acked a FIN, our side closed    */
            /* and we send a FIN.  We are waiting for the other side     */
            /* to ACK/ack our FIN.  Therefore, in case their ACK/ack     */
            /* got lost, shut the socket down after a period of time     */
            /* NOTE: this is not specified in RFC 793 but it makes sense */
            if ( (port->state == TCP_S_LAST) &&  
                (port->closetime > CFG_LASTTIME) )
            {
                trans_state(port, TCP_S_CLOSED, NOT_FROM_INTERPRET, FALSE);
            }
            /* NOTE: falls thru   */

            /* **************************************************           */
            /* If in the timed wait state close the socket if we've waited  */
            /* WAITTIME ticks                                               */
            if (port->state == TCP_S_TWAIT)
            {
                if (port->closetime > CFG_TWAITTIME)
                {
                    /* before closing process any output   */
                    process_one_tcp_output(port);   

                    trans_state(port, TCP_S_CLOSED, NOT_FROM_INTERPRET, FALSE);
                    /* NOTE: Falls through to state=CLOSED   */
                }
            }

#if (INCLUDE_TCP_REMOTE_CLOSE)
            /* *************************************************          */
            /* if graceful close but will only wait so long (linger_secs) */
            /* for the remote to close                                    */
            if ( (port->ap.port_flags & API_CLOSE_DONE) &&
                 (port->state != TCP_S_CLOSED) &&
                 !(port->ap.options & SO_LINGER) &&
                 (port->ap.linger_secs > 0) )
            {
                if (port->ap.linger_secs == 1)
                {
                    DEBUG_ERROR("tcp_timeout: remote never closed=>CLOSE",
                        NOVAR, 0, 0);
                    trans_state(port, TCP_S_CLOSED, NOT_FROM_INTERPRET, FALSE);
                }
                else 
                {
                    DEBUG_ERROR("tcp_timeout: linger_sec set to ", EBS_INT1,
                        port->ap.linger_secs, 0);
                    port->ap.linger_secs--;
                }
            }
#endif

            /* *************************************************   */
            if ( (port->state > TCP_S_TWAIT) &&   /* not TWAIT, LISTEN or CLOSED */
                (port->state != TCP_S_FW2) )
            {
                proc_keepalive(pi, port);
            }    /* end of not LISTEN or CLOSED */
        }

        /* **************************************************   */
        /* ONCE/CFG_TIMER_FREQ PROCESSING                       */
        /* **************************************************   */

#if (INCLUDE_TCP_OUT_OF_ORDER)
        /* **************************************************        */
        /* free out of order packets;                                */
        /* if alloc_packet reached a threshold on the number of DCUs */
        /* left; then it set free_ooo_lists flag                     */
        if (free_ooo_lists)
            free_ooo(port);
#endif  /* INCLUDE_TCP_OUT_OF_ORDER */

        /* **************************************************        */
        /* increment timeout counters every time                     */
        /* tbd - maybe limit counters incase don't retry             */
        /* NOTE: only increment timer for TWAIT and LAST if in those */
        /*       states to prevent overflow                          */
        if (port->ap.port_flags & RETRANS_EN)
        {
            if (port->sincetime == 0)
                port->lasttime = 0;
            port->sincetime += timer_freq;
            port->lasttime += timer_freq;
#if (DEBUG_DACK)
            DEBUG_ERROR("timeout - sincetime,lasttime = ", DINT2, 
                    port->sincetime, port->lasttime);
#endif
        }

#if (INCLUDE_TCP_RTT)
        /* if measuring a round trip time, increment timer   */
        if (port->rtt_nxt)
            port->rtttime += timer_freq;
#endif

        if (port->interp_port_flags & DELAYED_ACK) 
        {
            port->dactime += timer_freq; 
#            if (DEBUG_DACK)
                DEBUG_ERROR("to - incr delayed ack timer, ticks", EBS_INT2, 
                    port->dactime, ks_get_ticks());
#            endif
        }

        /* **************************************************            */
        /* Perform timeout processing at interval specified              */
        /* handle all states except LISTEN and FW2                       */
        /* NOTE: LISTEN was entered when xn_accept was called by app     */
        /*       but it justs waits for a SYNC, therefore, there is      */
        /*       nothing to retry                                        */
        /* NOTE: do not resend ack for TWAIT since other side is closing */
        /*       and if send it after other side closed, the other       */
        /*       side will send us a reset causing us to close without   */
        /*       waiting which defeats the purpose of TWAIT state        */
        /* NOTE: FW2 was entered since we sent a FIN and the other side  */
        /*       acked the FIN, we are waiting for the other side        */
        /*       to send its FIN, therefore, there is nothing to retry   */
        if (do_to_proc)
        {
            if (to_proc(pi, port))
            {
                OS_RELEASE_TCP()    /* Release access to tcp resources */
                return(TRUE);       /* call me again immediately */
                                    /* NOTE: port will not be on active linked   */
            }                       /*       list when called again */
        }

        /* **************************************************   */
        /* if a delayed ack is pending, send it now             */
        if ( (port->interp_port_flags & DELAYED_ACK) && 
             (port->dactime > CFG_MAX_DELAY_ACK) )
        {
#            if (DEBUG_DACK)
                DEBUG_ERROR("to - send delayed ack, dactime, ticks", EBS_INT2, 
                    port->dactime, ks_get_ticks());
#            endif
            tc_tcpsend(port->ap.iface, port, NO_DCU_FLAGS, NORMAL_SEND, 
                       TCP_F_ACK);
        }

        /* *************************************************   */
        /* point to next port in list                          */
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[ACTIVE_LIST], 
                                                (POS_LIST)port, ZERO_OFFSET);
    }   /* end of while port */

#if (INCLUDE_TCP_OUT_OF_ORDER)
    free_ooo_lists = FALSE;
#endif

    OS_RELEASE_TCP()      /* Release access to tcp resources */

    /* ******   */
#if (DISPLAY_TIMER)
    DEBUG_LOG("tc_tcp_active_timeout() exit", LEVEL_3, NOVAR, 0, 0);
#endif
    /* DEBUG_LOG("PORTS", LEVEL_3, PORTS_TCP, 0, 0);   */

    return(FALSE);          /* FALSE means call me after another timer */
                            /* interval passes   */
}

/* ********************************************************************   */
/* start_retrans_timer() - start the retransmission timer                 */
/*                                                                        */
/*   start the TCP retransmission timer                                   */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void start_retrans_timer(PTCPPORT port)    /*__fn__*/
{
    if (!(port->ap.port_flags & RETRANS_RUNNING))
    {
        port->sincetime = 0;
        port->ap.port_flags &= ~REPORT_RETRANS;
        port->ap.port_flags |= RETRANS_RUNNING;
        DEBUG_LOG("start_retrans_timer - cleared sincetime", LEVEL_3, NOVAR, 0, 0);
    }
}

#if (INCLUDE_CWND_SLOW)
/* ********************************************************************   */
/* incr_cwnd() - increment congestion window                              */
/*                                                                        */
/* Increment congestion window by incr but limit it to window size.       */
/* This prevents overflowing/wrapping the congestion window which is      */
/* is always limited to window size anyway                                */
/*                                                                        */
/* Returns nothing                                                        */
/*                                                                        */

void incr_cwnd(PTCPPORT port, word incr)
{
    port->cwnd = (word)(port->cwnd + incr);
    if (port->cwnd > port->out.window_size)
    {
        port->cwnd = port->out.window_size;
    }
}

/* ********************************************************************   */
/* congestion_proc() - congestion processing (timeout or duplicate ACKS)  */
/*                                                                        */
/*   Called when congestion occurred due to timeout or reception of       */
/*   duplicate acks.                                                      */
/*   Updates threshold and if timeout updates congestion window size.     */
/*   to is TRUE if result of timeout.                                     */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void congestion_proc(PTCPPORT port, RTIP_BOOLEAN to)
{
    /* thresh = min(cwnd, othersides window size) / 2   */
    port->ssthresh = port->cwnd;
    if (port->ssthresh > port->out.size)
        port->ssthresh = port->out.size;
    port->ssthresh = (word)(port->ssthresh >> 1);

    /* thresh should be at least 1 segments   */
    if (port->ssthresh < port->out.mss)
        port->ssthresh = port->out.mss;

    /* if timeout, do slow start, i.e. set congestion window to 1 mss   */
    if (to)
        port->cwnd = port->out.mss;
}
#endif

/* ********************************************************************     */
/* reset_nxt_to_send() - set next byte to send to first byte in window      */
/*                                                                          */
/*   This routine sets up information so the next byte to send will be      */
/*   the first byte in the window (i.e. the first unacknowledged byte).     */
/*   Normally the next byte transmitted is the first byte in the window     */
/*   which have never been transmitted.                                     */
/*                                                                          */
/*   NOTE: port->nxt_to_send     = seq number of next byte to send          */
/*         port->nxt_to_send_dcu = dcu containing next byte to send         */
/*         port->nxt_to_send_off = offset in port->nxt_to_send_dcu of       */
/*                                 next byte to send                        */
/*         ---------------------                                            */
/*         port->dcu_start       = first dcu in window (i.e. contains       */
/*                                 first unacked byte)                      */
/*         msg->data_start       = offset in port->dcu_start of first byte  */
/*                                 in window                                */
/*         msg->data_len         = number of bytes in port->dcu_start which */
/*                                 are a part of the window                 */
/*                                                                          */
/*                                                                          */
/*   Returns nothing                                                        */
/*                                                                          */

void reset_nxt_to_send(PTCPPORT port)       /*__fn__*/
{
int  data_start;
DCU  msg;
#if (RTIP_VERSION > 26)
int top_data_start;
#endif

    /* update remote window size; i.e. increase it by number of bytes going   */
    /* to retransmit since we are backing up what bytes we are sending        */
    /* to the beginning of the output window                                  */
    /* NOTE: nxt_to_send should always be >= but just to be sure check for it */
    /*                                                                        */
    if (port->out.nxt_to_send >= port->out.nxt)
    {
        port->out.size = (word)(port->out.size + port->out.nxt_to_send - 
                                port->out.nxt);
    }
    else
    {
        DEBUG_ERROR("          reset_nxt_to_send: nxt > nxt_to_send", DINT2,
            port->out.nxt_to_send, port->out.nxt);

        /* tbd - not sure what to do here   */
        port->out.size = port->out.ad_size;
    }

    /* UPDATE SEQUENCE NUMBER to first unacked byte   */
    port->out.nxt_to_send = port->out.nxt;

#if (DISPLAY_NORMAL)
    DEBUG_ERROR("          reset_nxt_to_send - new window size after update, nxt_to_send = ", 
        DINT2, port->out.size, port->out.nxt_to_send);
#endif

    /* nxt must be in the first packet queued   */
    msg = (DCU)(port->out.dcu_start); 
    if (msg)
    {
        data_start = DCUTOPACKET(msg)->data_start;
        port->out.nxt_to_send_dcu = msg;
        port->out.nxt_to_send_off = data_start;

#if (INCLUDE_TCP_NO_COPY)
        if (port->ap.options & SO_TCP_NO_COPY)
        {
            /* if nxt is in the middle of the data area (i.e.   */
            /* some of the packet has been acked) then move     */
            /* the data to the start of the packet so the       */
            /* seq number will be correct and so we will        */
            /* not send duplicate data                          */
#if (RTIP_VERSION > 26)
            TOTAL_TCP_HLEN_SIZE(top_data_start, port, port->ap.iface, msg)
            if (data_start > top_data_start)
#else
            if (data_start > TCP_TLEN_BYTES)
#endif
            {
#if (RTIP_VERSION > 26)
                tc_movebytes(DCUTODATA(msg) + top_data_start,
                             DCUTODATA(msg) + data_start,
                             DCUTOPACKET(msg)->data_len);
                DCUTOPACKET(msg)->data_start = top_data_start;
                port->out.nxt_to_send_off = top_data_start;
#else
                tc_movebytes(DCUTODATA(msg) + TCP_TLEN_BYTES,
                             DCUTODATA(msg) + data_start,
                             DCUTOPACKET(msg)->data_len);
                DCUTOPACKET(msg)->data_start = TCP_TLEN_BYTES;
                port->out.nxt_to_send_off = TCP_TLEN_BYTES;
#endif
            }
        }
#endif      /* INCLUDE_TCP_NO_COPY */
    }
}

/* ********************************************************************   */
/* do_retrans() - retransmit data due to timeout                          */
/*                                                                        */
/*   This routine is called to actually do setup and perform the          */
/*   retransmit.  The parameter type specifies the reason, i.e.           */
/*   RETRANS for timeout on ack for FAST_RETRANS for fast retransmit      */
/*   due to duplicate acks                                                */
/*                                                                        */
/*   Returns nothing                                                      */

void do_retrans(PIFACE pi, PTCPPORT port, int type)
{
WINDOW wind;

    /* retransit                                       */
    /* NOTE: flags sent is 0, i.e. discard packet when */
    /*       done (since transq will allocate          */
    /*       a new packet) and do not signal (since    */
    /*       timeout should not hang waiting for a     */
    /*       packet to be sent)                        */
    /* NOTE: force all data in buffer to be sent       */
#if (DISPLAY_RETRANS)
    DEBUG_ERROR("TCP RETRY - retrans - state,out.contain = ", 
        EBS_INT2, port->state, port->out.contain);
    DEBUG_ERROR("          retrans - lasttime,sincetime = ", DINT2, 
        port->lasttime, port->sincetime);
    DEBUG_ERROR("          port->out.nxt_to_send, port->out.nxt = ", DINT2,
        port->out.nxt_to_send, port->out.nxt);
    DEBUG_ERROR("          remote window size, output que size = ", DINT2,
        port->out.size, pi->no_output_que);
    DEBUG_ERROR("          out.contain, msg_type = ", DINT2,
        port->out.contain, type);
#endif

    /* save window information for recovery below;                  */
    /* NOTE: this is done since fast retrans only sends one segment */
    if (type == FAST_RETRANS)
        STRUCT_COPY(wind, port->out);

    port->lasttime = 0;

    /* set next byte to send back to the first byte in the window; i.e.   */
    /* set nxt_to_send to nxt                                             */
    reset_nxt_to_send(port);

#if (INCLUDE_TCP_RTT)
    port->rtt_nxt = 0;  /* quit measuring RTT since timeout */
#endif

    /* RETRANSMIT THE DATA (type is RETRANS or FAST_RETRANS)   */
    tc_tcpsend(pi, port, NO_DCU_FLAGS, type, NO_TCP_FLAGS);

    /* restorewindow information from before fast retransmit;       */
    /* NOTE: this is done since fast retrans only sends one segment */
    if (type == FAST_RETRANS)
    {
        STRUCT_COPY(port->out,wind);
    }
    else
    {
        UPDATE_INFO(pi, tcp_retries, 1)
    }
}

/* ********************************************************************     */
/* to_proc() - Checks for and performs data transmission timeout processing */
/*                                                                          */
/*   Returns TRUE if returned port to the LISTEN list                       */
/*                                                                          */
static RTIP_BOOLEAN to_proc(PIFACE pi, PTCPPORT port)
{
RTIP_BOOLEAN  retrans_pkt;

    if ( (port->state > TCP_S_TWAIT) &&   /* not TWAIT, LISTEN or CLOSED */
            (port->state != TCP_S_FW2) )
    {
        /* **************************************************              */
        /* if timeout from tcp pkt sent (i.e. arp was resolved             */
        /* and tcp send was done)                                          */
        /* NOTE: for EST state, if otherside resends the SYNC (possibly    */
        /*       due to our ACK being lost) the SYNC will be rejected      */
        /*       since it will not be within the window                    */
        /* NOTE: if in CWAIT we are waiting for app to close; the          */
        /*       remote host is in FW1 until it receives ack to its fin is */
        /*       in FW1;  if the other side is stuck in                    */
        /*       the FW1 state (if our ack got lost) two different         */
        /*       scenerios could happen:                                   */
        /*       1) other side resends a FIN, CWAIT will process           */
        /*          it and ack it, then otherside will transition out      */
        /*          of FW1                                                 */
        /*       2) when app closes we will send the fin along with        */
        /*          ACK/ack so other side will transition from FW1         */
        /*          to TWAIT                                               */
        /*       Therefore, for CWAIT we will not resend the ACK,ack       */
        /*       but we will resend any data in output window              */
        retrans_pkt = FALSE;
        if ( (port->state != TCP_S_EST) && (port->state != TCP_S_CWAIT) ) 
            retrans_pkt = TRUE;
        else if (port->out.contain)
            retrans_pkt = TRUE;

#if (DISPLAY_TIMER)
        DEBUG_LOG("timeout - port_flags = ", LEVEL_3, EBS_INT1, 
            port->ap.port_flags, 0);
#endif

        if ( (port->ap.port_flags & RETRANS_EN) &&  
             (port->ap.port_flags & RETRANS_RUNNING) &&
             (port->lasttime > port->rto) &&
             retrans_pkt && CHECK_RETRANS(port) )
        {

            /* if hit second threshold of retransmits, close connection, i.e.   */
            /* check for max number of retries, i.e. if PKT_INTERVAL_TO         */
            /* has elapsed since first retry then do not send                   */
            /* anymore retries for the current seq byte                         */
            /* NOTE: check_ack will reset sincetime whenever                    */
            /*       there was a byte rmqueued from the output                  */
            /*       window so the retry can start over with                    */
            /*       the new first byte; it will also be reset                  */
            /*       when a pkt is sent                                         */
            /* NOTE: only retries if arp is resolved, but if arp                */
            /*       timeout and gives up it will set the condition             */
            /*       such that this test will pass (causing send() to           */
            /*       fail and the socket to close)                              */
            if (port->sincetime > CFG_RETRANS_TMO)
            {
                if (port->from_state == TCP_S_LISTEN)
                {
                    DEBUG_ERROR("to_proc - stop retrying; return listen state: lasttime, sincetime=", 
                        DINT2, port->lasttime, port->sincetime);
                    return_listen_state(port);
                    return(TRUE);
                }
                else
                {
                    /* signal reader and writers with error; signal user   */
                    /* by closing port; do not free port until application */
                    /* calls close                                         */
                    tc_tcp_abort_connection(port, 
                        READ_WAKEUP_FAIL|WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP,
                        ETIMEDOUT, NOT_FROM_INTERPRET);
#if (DEBUG_ABORT_CONNECTION)
                    DEBUG_ERROR("to_proc: abort connection", NOVAR, 0, 0);
#endif
                    DEBUG_ERROR("to_proc - stop retrying: lasttime, sincetime=", 
                        DINT2, port->lasttime, port->sincetime);
                }
            }
            else
            {
                /* if hit first threshold of retransmits, report error to application   */
                /* but only the first time hit this threshold for this timing           */
                if ( (port->sincetime > CFG_REPORT_TMO) &&
                     (!(port->ap.port_flags & REPORT_RETRANS)) )
                {
                    /* inform user application   */
                    CB_ERROR_REPORT(port->ap.ctrl.index, TCP_RETRANS, 0);
                    port->ap.port_flags |= REPORT_RETRANS;
                    DEBUG_LOG("to_proc - report to application: lasttime, sincetime=", 
                        LEVEL_3, DINT2, port->lasttime, port->sincetime);
                }

                /* if transmission timeout occurs,                   */
                /* exponential back-off timeout period.  Only        */
                /* increment to max amount to avoid long delays (for */
                /* systems with a lot of retries) before aborting    */
                /* connection.                                       */
                /* This number returns toward the correct value by   */
                /* the RTT measurement code in check_ack.            */
                port->rto <<= 1;        /* double it  */
                if (port->rto > CFG_MAXRTO) 
                    port->rto = CFG_MAXRTO;
#if (DEBUG_RTT)
                DEBUG_ERROR("timeout-port->rto", EBS_INT1, port->rto, 0);
#endif
                DEBUG_LOG("timeout-port->rto", LEVEL_3, EBS_INT1, port->rto, 0);

#if (INCLUDE_CWND_SLOW)
                /* congestion window processing (update cwnd and ssthresh)   */
                congestion_proc(port, TRUE);
#endif

                /* RETRANSMIT THE DATA   */
                do_retrans(pi, port, RETRANS);
            }   
        }
    }    /* end of not LISTEN or CLOSED */
    return(FALSE);
}

/* ********************************************************************       */
/* do_window_probe() - probe 0 window size on remote host                     */
/*                                                                            */
/* Persist timer; used to find out window size of other side in case they     */
/* had a window size of 0 then they sent us updated size which got lost; i.e. */
/* if this side is only doing writes and the other side is only doing reads   */
/* and the window update got lost we would never do a write (until response   */
/* from timeout is received)                                                  */
/*                                                                            */
/* if 0 window size, do window probing                                        */
/* this is called every 1 second by the timeout processing                    */
/* if 0 window size then we send the remote host a one byte window            */
/* probe; we send the next seq number since tcp is allowed to send            */
/* one byte past the end of the window (we will not update our next           */
/* byte to send to reflect the one byte window probe);  this forces the       */
/* otherside to send us an ack with an updated window size; if the            */
/* othersides window size is still really 0, it will not ack this             */
/* one byte                                                                   */
/*                                                                            */

static void do_window_probe(PIFACE pi, PTCPPORT port)    /*__fn__*/
{
word data_already_sent;

    /*   out.size = actual room in other sides input window   */
    /*   out = room advertised in othersides input window     */
    if ( (port->out.size == 0) || (port->out.ad_size == 0) )
    {
        data_already_sent = (word)(port->out.nxt_to_send - port->out.nxt);
        if ( ((port->state == TCP_S_EST)     ||
              (port->state == TCP_S_CWAIT))  &&
             (data_already_sent < port->out.contain) )
        {
            /* if first window probe, reset timers   */
            if ( !(port->ap.port_flags & WINDOW_PROBE_STARTED) )
            {
                port->ap.port_flags |= WINDOW_PROBE_STARTED;
                port->wp_sincetime = 0;
                port->wp_sendtime = CFG_WIN_PROBE_MIN;
            }

            port->wp_sincetime++;
            if (port->wp_sincetime >= port->wp_sendtime)
            {
#if (DISPLAY_SEND_WINDOW_PROBE)
                DEBUG_ERROR("SEND WINDOW PROBE - size, ad_size = ", 
                    EBS_INT2, port->out.size, port->out.ad_size);
                DEBUG_ERROR("      - data already sent, out.contain = ", EBS_INT2, 
                    data_already_sent, port->out.contain);
                DEBUG_ERROR("      - nxt, nxt_to_send = ", DINT2, 
                    port->out.nxt, port->out.nxt_to_send);
                DEBUG_ERROR("         port->sincetime, port->lasttime", DINT2,
                    port->sincetime, port->lasttime);

                DEBUG_ERROR("      - port->ap.port_flags", EBS_INT1,
                    port->ap.port_flags, 0);
                DEBUG_ERROR("      - RETRANS_EN, RETRANS_RUNNING", EBS_INT2,
                    RETRANS_EN, RETRANS_RUNNING);
                DEBUG_ERROR("      - port->rto ", EBS_INT1,
                    port->rto, 0);
#endif

                tc_tcpsend(pi, port, NO_DCU_FLAGS, WINDOW_PROBE, 
                           NO_TCP_FLAGS);
                port->wp_sincetime = 0; /* restart timer */

                /* double the timer but limit it   */
                port->wp_sendtime = port->wp_sendtime << 1;
                if (port->wp_sendtime > CFG_WIN_PROBE_MAX)
                    port->wp_sendtime = CFG_WIN_PROBE_MAX;
                UPDATE_INFO(pi, tcp_window_probes, 1)
            }
        }
    }
    else
        port->ap.port_flags &= ~WINDOW_PROBE_STARTED;
}

/* ********************************************************************   */
static void proc_keepalive(PIFACE pi, PTCPPORT port)        /*__fn__*/
{
    port->intime++;
/*  DEBUG_LOG("   intime", LEVEL_3, DINT1, port->intime, 0);   */

    /* send keep alive packets if needed; used to check if other side   */
    /* is up and running if haven't heard from them in a while          */
    /* NOTE: our implementation is based upon 4.3BSD and sends          */
    /*       a packet with no data and an invalid seq number            */
    /*       i.e. one that already has been acked                       */
    /* NOTE: according to Stevens, keepalives are not part of           */
    /*       Host Req RFC 793.  The RFC gives 3 reasons not to          */
    /*       use them,                                                  */
    /*       1) can cause good connections to crash                     */
    /*       2) consume unnecessary bandwidth                           */
    /*       3) internet charges                                        */
    /*       but he points out reasons, especially for server           */
    /*       where it would be a good idea to use it, such as           */
    /*       remote user (client) loging in using Telnt and             */
    /*       does not logoff but but just powers off.  If the           */
    /*       connection is not closed the user will keep                */
    /*       incurring compter costs                                    */
    /* NOTE: only send keepalives if user enabled them                  */
    /*       (we always respond to keepalive pkts)                      */
    /* NOTE: could switch to 4.2BSD (i.e. 1 byte of garbage             */
    /*       data) halfway thru timeout interval since                  */
    /*       some systems won't respond                                 */

    /* first check if should do keep alive   */
    if ( port->ap.options & SO_KEEPALIVE &&
         (port->intime >= port->ka_interval) &&
         (port->state == TCP_S_EST) )       /* tbd CWAIT? */
    {

        /* update timer since started sending keepalives   */
        port->ka_sincetime++;
/*      DEBUG_LOG("proc_keepalive: ka_sincetime, ka_started = ",               */
/*          LEVEL_3, EBS_INT1, port->ka_sincetime, port->ka_started);          */
/*      DEBUG_ERROR("proc_keepalive:  ka_sincetime, ka_started = ", EBS_INT2,  */
/*          port->ka_sincetime, port->ka_started);                             */

        /* if retried max number of times close the port   */
        if (port->ka_sincetime >= port->ka_tmo)
        {    
            DEBUG_ERROR("proc_keepalive() - close due to KA", NOVAR, 0, 0);

            /* signal reader and writers with error; signal user   */
            /* by closing port; do not free port until application */
            /* calls close                                         */
            tc_tcp_abort_connection(port, 
                READ_WAKEUP_FAIL|WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP,
                ENETUNREACH, NOT_FROM_INTERPRET);
#if (DEBUG_ABORT_CONNECTION)
            DEBUG_ERROR("proc_keepalive: abort connection", NOVAR, 0, 0);
#endif
        }

        else
        {
            /* start or in process of keepalive timeout   */
            if ( !port->ka_started ||
                 ((port->ka_sincetime % port->ka_retry) == 0) )
            {
#if (DEBUG_KA)
                DEBUG_ERROR("send keep-alive: ka_started = ", 
                    EBS_INT1, port->ka_started, 0);
#endif

                /* if this is first time trying keepalive    */
                if (!port->ka_started)
                {
                    port->ka_started = TRUE;
                    UPDATE_INFO(pi, tcp_keepalives, 1) 
                }
                else           /* if not first then retry */
                    UPDATE_INFO(pi, tcp_keepalive_retries, 1) 

                /* have not retried enough times, try again (or for   */
                /* the first time)                                    */
                /* NOTE: send invalid seq number forcing other side   */
                /*       to respond to the probe with an ACK          */
                tc_tcpsend(pi, port, NO_DCU_FLAGS, KEEPALIVE, NO_TCP_FLAGS); 
            }
            
        }
    }

    /* we have gotten an input pkt in time   */
    else
    { 
#if (DEBUG_KA)
/*      DEBUG_ERROR("GOT INPUT->STOP KA, ka_sincetime, ka_started = ", EBS_INT2,    */
/*          port->ka_sincetime, port->ka_started);                                  */
#endif
        port->ka_started = FALSE;
        port->ka_sincetime = 0;
    }
}

/* ********************************************************************   */
/* MISC ROUTINES                                                          */
/* ********************************************************************   */

/* ********************************************************************   */
/* void wake_up()  -  signal read/accept and/or write/connect             */
/*                                                                        */
/*   Signals tasks block on read/write signal depending upon signal       */
/*   flags.  Either the signalling is done directly from this routine     */
/*   if not called from interpret, or if called from interpret flags      */
/*   are set so the signalling will be done at the end of interpret.      */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void wake_up(PTCPPORT port, RTIP_BOOLEAN interpret, int signal_flags,   /*__fn__*/
             int errno_val)                                        /*__fn__*/
{
    if ( (signal_flags & READ_WAKEUP_FAIL) || 
         (signal_flags & WRITE_WAKEUP_FAIL) )
    {
        port->ap.ctrl.rtip_errno = errno_val;   /* pass to set signal routine */
        if ( (interpret == NOT_FROM_INTERPRET) &&
             (signal_flags & WRITE_WAKEUP_FAIL) )
            os_set_sent_signal((PANYPORT)port, FALSE);  /* wake-up ARP */
    }

    if (interpret == NOT_FROM_INTERPRET)
    {
        if (signal_flags & READ_WAKEUP_FAIL) 
            os_set_read_signal((PANYPORT)port, FALSE);

        if (signal_flags & WRITE_WAKEUP_FAIL)
            os_set_write_signal((PANYPORT)port, FALSE);

        if (signal_flags & READ_WAKEUP_SUCCESS) 
            os_set_read_signal((PANYPORT)port, TRUE);

        /* NOTE: os_set_write_signal will signal select if    */
        /* SELECT_WRITE_WAKEUP is set also                    */
        if (signal_flags & WRITE_WAKEUP_SUCCESS)
            os_set_write_signal((PANYPORT)port, TRUE);
    }
    else
    {
        if (signal_flags & READ_WAKEUP_FAIL)
            port->interp_port_flags |= READ_WAKEUP_FAIL;    

        if (signal_flags & WRITE_WAKEUP_FAIL)
            port->interp_port_flags |= WRITE_WAKEUP_FAIL;   

        if (signal_flags & READ_WAKEUP_SUCCESS)
            port->interp_port_flags |= READ_WAKEUP_SUCCESS; 

        if (signal_flags & WRITE_WAKEUP_SUCCESS)
            port->interp_port_flags |= WRITE_WAKEUP_SUCCESS;    

        if (signal_flags & SELECT_WRITE_WAKEUP)
            port->interp_port_flags |= SELECT_WRITE_WAKEUP; 
    }
}

/* ********************************************************************     */
/* void tc_tcp_abort_connection() - signal api layer due to error           */
/*                                                                          */
/*   This routine processes an error which will close the port and possibly */
/*   signal api layer with error status.                                    */
/*                                                                          */
void tc_tcp_abort_connection(PTCPPORT port, int signal_flags, int errno_val,  /*__fn__*/
                             RTIP_BOOLEAN interpret)  /*__fn__*/
{
    /* NOTE: signalling user done by not getting rid of port until closesocket done   */
    /* but all api calls will fail;                                                   */
    /* NOTE: another option would be to sent signal to api here                       */
    /* and change code so tcp_close will free port even if api                        */
    /* not called;                                                                    */
#    if (INCLUDE_SNMP)
        if ( (port->state == TCP_S_SYNS) ||
             (port->state == TCP_S_SYNR) )
            INCR_SNMP(TcpAttemptFails)

        if ( (port->state == TCP_S_EST) ||
             (port->state == TCP_S_CWAIT) )
        {
            INCR_SNMP(TcpEstabResets)
            DECR_SNMP(TcpCurrEstab)
        }
#    endif

#if (INCLUDE_POSTMESSAGE)
    if (port->from_state != TCP_S_LISTEN)
    {
        if ( (port->state == TCP_S_SYNS) || (port->state == TCP_S_SYNR) )
        {
            PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_CONNECT, 0);
        }
    }
#endif

    /* transition to CLOSED state and free DCUs in input window   */
    trans_state(port, TCP_S_CLOSED, interpret, TRUE);

    wake_up(port, interpret, signal_flags, errno_val);

    /* we are closed so forget about delayed ack in progress   */
    port->interp_port_flags &= ~DELAYED_ACK;

    DEBUG_LOG("tc_tcp_abort_connection - close socket - signals = ", 
        LEVEL_3, EBS_INT1, signal_flags, 0);
}

/* ********************************************************************   */
/* INPUT PACKET PROCESSING ROUTINES                                       */
/* ********************************************************************   */

/* ********************************************************************   */
/* void trans_state() - perform a state transition                        */
/*                                                                        */
/*   Performs state transition processing by checking if need to wake up  */
/*   any readers or writers and by setting the state to the new state.    */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void trans_state(PTCPPORT port, int new_state, RTIP_BOOLEAN interpret,  /*__fn__*/
                 RTIP_BOOLEAN free_in_win)                              /*__fn__*/
{
    DEBUG_LOG("trans state: from, to = ", LEVEL_3, EBS_INT2, port->state, new_state);
#if (DEBUG_WEB)
    DEBUG_ERROR("trans state: from, to = ", EBS_INT2, port->state, new_state);
#endif

    /* NOTE: if transiting to CLOSED, caller will determine if need to   */
    /*       signal                                                      */

    /* check if need to wake up read; i.e. if transition to an illegal state   */
    /* for read or if transition to CWAIT; CWAIT is legal but we got a FIN,    */
    /* therefore, no more data will arrive so should wakeup any readers        */
    /* NOTE: when read wakes up it will check state, therefore, can wake       */
    /*       up with success                                                   */
    switch (port->state)
    {
    case TCP_S_TWAIT:
        port->closetime = 0;       /* start timeout timer (until close) */
    case TCP_S_LAST:
    case TCP_S_CWAIT:
    case TCP_S_CLOSING:
        wake_up(port, interpret, READ_WAKEUP_SUCCESS, 0);
        break;
#if (INCLUDE_POSTMESSAGE)
    case TCP_S_SYNR:
        /* connection request arrives - should only call if       */
        /* FD_ACCEPT not already posted but PostMessage will have */
        /* to deal with that                                      */
        PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_ACCEPT, 0);
#endif
    }

    switch (port->state)
    {
    case TCP_S_FW1:
    case TCP_S_LAST:
        /* if need to wake up write; i.e. if transition to an illegal state   */
        /* for write                                                          */
        /* NOTE: when read wakes up it will check state, therefore, can wake  */
        /*       up with success                                              */
        wake_up(port, interpret, WRITE_WAKEUP_SUCCESS|SELECT_WRITE_WAKEUP, 0);
        break;
    }

    if (new_state == TCP_S_CLOSED)
    {
        /* for non-blocking connect, reset connection in progress   */
        /* (allows select to return)                                */
        port->ap.port_flags &= ~TCP_CONN_IN_PROG;
    }

    /* free any DCUs in input and output windows     */
    /* NOTE: freeing input window; that is up to the */
    /* caller to determine, i.e. free_in_win         */
    if ( (new_state == TCP_S_CLOSED) || (new_state == TCP_S_TWAIT) ||
         (new_state == TCP_S_LAST) )
        free_both_windows(port, free_in_win);

    port->state = new_state;
}

/* ********************************************************************   */
/* void trans_state_est() - transition to established state               */
/*                                                                        */
/*   Do processes associated with transitioning to ESTABLISHED state      */
/*                                                                        */
void trans_state_est(PTCPPORT port)
{
    /* stop retrans timer in case timing for ACK of SYNC   */
    TCP_STOP_RETRANS(port)

    trans_state(port, TCP_S_EST, TRUE, FALSE);

#if (INCLUDE_POSTMESSAGE)
    if (port->from_state != TCP_S_LISTEN)
    {
        PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_CONNECT, 0);
    }

    /* if connection established after ACCEPT or CONNECT called   */
    if (port->master_port && 
        port->master_port->ap.post_flags & ACCEPT_OR_CONNECT_CALLED)
    {
        port->master_port->ap.post_flags &= ~ACCEPT_OR_CONNECT_CALLED;
        PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_WRITE, 0);
    }

    if (port->ap.post_flags & ACCEPT_OR_CONNECT_CALLED)
    {
        port->ap.post_flags &= ~ACCEPT_OR_CONNECT_CALLED;
        PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_WRITE, 0);
    }
#endif

    INCR_SNMP(TcpCurrEstab)

    /* set up our output window size based upon interface will send   */
    /* out on                                                         */
    setup_ws((PWINDOW)&port->out, port->ap.iface->pdev->window_size_out, 
             EBS_OUTPUT);

#if (INCLUDE_CWND_SLOW)
    /* initialize congestion window and threshold now that mss is known   */
    port->cwnd = port->out.mss;
    port->ssthresh = 0xffff;
#endif
#if (INCLUDE_FAST_RETRANS)
    port->dup_ack_cnt = 0;
    port->fr_nxt = 0;
#endif

    /* wake up listen() or connect()   */
    if (port->from_state == TCP_S_LISTEN)
    {
        port->interp_port_flags |= READ_WAKEUP_SUCCESS;
        DEBUG_LOG("TRANS TO EST -> signal read", LEVEL_3, NOVAR, 0, 0);
    }
    else
        port->interp_port_flags |= (WRITE_WAKEUP_SUCCESS|SELECT_WRITE_WAKEUP);

    /* clear sincetime in case data is already in output queue   */
    /* (enqueue cleared it if in EST or CWAIT state so if        */
    /* in SYNS or SYNR state and go to EST state do it here)     */
    if (port->out.contain)
        start_retrans_timer(port);
    DEBUG_LOG("trans_state_est - cleared sincetime", LEVEL_3, NOVAR, 0, 0);

    /* for non-blocking sockets which did a connect(), let select()   */
    /* know the connection has been established                       */
    if (port->ap.port_flags & TCP_CONN_IN_PROG)
        port->ap.port_flags |= TCP_CONN_EST;

    /* if any data queued while in SYNR and SYNS state, transmit it   */
    if (port->out.contain)
        port->interp_port_flags |= SEND_DATA_PKT;    
}

/* ********************************************************************    */
/* fix_master() - fix state of a master port now that is has a listener    */
/*                                                                         */
/*   This routine is called for a master port which previously had         */
/*   a problem adding a listener either due to backlog used up or          */
/*   listen() failed probably due to out of sockets.  After the            */
/*   problem has been corrected, i.e. a listener has been successfully     */
/*   added, this routine is called to either move the master from the      */
/*   master list back to the socket list or to set up the correct backlog. */
/*                                                                         */
/*   NOTE: assumes TCP semaphore is claimed                                */
/*                                                                         */

void fix_master(PTCPPORT mport)  /*__fn__*/
{
    /* now fix the master port since we now have a listener   */
    if (mport->ap.list_type == MASTER_LIST) 
    {
        delete_tcpport_list(mport);
        add_tcpport_list(mport, SOCKET_LIST);
    }
    if (mport->tcp_port_type == MASTER_NO_LISTENS)
    {
        DEBUG_LOG("fix master - set port_type to 0", LEVEL_3, NOVAR, 0, 0);
        mport->tcp_port_type = 0;
    }
}

/* ********************************************************************   */
/* error and got here with  passive OPEN (i.e. came from LISTEN)          */
/* return to LISTEN; do not inform user, i.e. accept()                    */
/* NOTE: this could cause two slave ports from the same                   */
/*       master to be on LISTEN list but that is ok the                   */
/*       only reason we only put on one at a time is                      */
/*       to save on resources                                             */
static void return_listen_state(PTCPPORT port)      /*__fn__*/
{
PTCPPORT mport;

    INCR_SNMP(TcpAttemptFails)

    mport = port->master_port;
    if (mport)
    {
        /* if there was not a problem the last time tried to put   */
        /* a socket on the listen list, then there should be       */
        /* a listener already connected to the master so           */
        /* close this socket; and since the application never      */
        /* had control of the slave port, make sure it is freed    */
        /* by setting API_CLOSE_DONE flag                          */
        if ( (mport->tcp_port_type != MASTER_NO_LISTENS) &&
             (mport->ap.list_type != MASTER_LIST) )
        {
            DEBUG_LOG("return_listen_state() - abort the connection", LEVEL_3, NOVAR, 0, 0);

            /* delete from master list     */
            port->from_state = 0;
            if (port->tcp_port_type == SLAVE_PORT)
                delete_tcpport_master_list(port);
            mport->tcp_port_type++;     /* allow the master another listener */

            tc_tcp_abort_connection(port, NO_SIGNAL_FLAGS, 0, FROM_INTERPRET); 
#if (DEBUG_ABORT_CONNECTION)
            DEBUG_ERROR("return_listen_state: abort connection", NOVAR, 0, 0);
#endif
            port->ap.port_flags |= API_CLOSE_DONE;
            return;
        }
    }

    DEBUG_LOG("return_listen_state() - return to listen", LEVEL_3, NOVAR, 0, 0);

    trans_state(port, TCP_S_LISTEN, FROM_INTERPRET, TRUE);

    free_both_windows(port, TRUE);
    setup_both_windows(port);
    port->in.size = 0;
    port->out.port = 0;         /* accept any outside port # */

    delete_tcpport_list(port);
    add_tcpport_list(port, LISTEN_LIST);

    /* now fix the master port since we now have a listener   */
    fix_master(mport);

#if (DEBUG_WEB)
    return_listen++;
#endif
}

/* ********************************************************************   */
/* void proc_reset() - process reset flag in incoming packet              */
/*                                                                        */
/* Process a reset in the incoming packet and processes the reset         */
/* according to the current tcp state                                     */
/*                                                                        */
/* Returns nothing                                                        */
/*                                                                        */
static void proc_reset(PTCPPORT port)        /*__fn__*/
{ 
    DEBUG_LOG("proc_reset() - reset received,state =", LEVEL_3,
        EBS_INT1, port->state, 0);
#if (DEBUG_RESET)
    DEBUG_ERROR("proc_reset() - reset received,state =", 
        EBS_INT1, port->state, 0);
#endif

#if (INCLUDE_POSTMESSAGE)
    PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_CLOSE, 0);
#endif

    switch (port->state)
    {
    /* ******   */
    case TCP_S_SYNR:
        /* if got here with  passive OPEN (i.e. came from LISTEN)   */
        /* return to LISTEN; do not inform user                     */
        /* NOTE: this could cause two slave ports from the same     */
        /*       master to be on LISTEN list but that is ok the     */
        /*       only reason we only put on one at a time is        */
        /*       to save on resources                               */
        if (port->from_state == TCP_S_LISTEN)
        {
            DEBUG_LOG("proc_reset - SYNR - return to listen", 
                LEVEL_3, NOVAR, 0, 0);
            return_listen_state(port);
        }
        else   /* not from LISTEN state (i.e. it was an acitve open */
                /* (connect), therefore, there are no slave ports)   */
        {
            DEBUG_LOG("  proc_reset() - other side reset:SYNR->CLOSED", 
                LEVEL_3, NOVAR, 0, 0);
            tc_tcp_abort_connection(port, 
                WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, ECONNREFUSED,
                FROM_INTERPRET);        /* wake up connect() which will  */
                                        /* fail (i.e. signal user-RFC 793    */
                                        /* pg 70)                            */
#if (DEBUG_ABORT_CONNECTION)
            DEBUG_ERROR("proc_reset: abort connection", NOVAR, 0, 0);
#endif
            /* NOTE: no retransmissions will occur since state is   */
            /*       closed (RFC 793 pg 70)                         */
        }
        break;

    /* *****   */
    case TCP_S_EST:     
    case TCP_S_FW1:                     /* waiting for ACK of FIN  */
    case TCP_S_FW2:                     /* want FIN  */
    case TCP_S_CWAIT:                   /* FIN received */
        /* if came from listen state and socket has not been   */
        /* accepted yet, then return to the listen state       */
        if (port->from_state == TCP_S_LISTEN)
        {
            DEBUG_LOG("proc_reset - return to listen: state = ", 
                LEVEL_3, EBS_INT1, port->state, 0);
            return_listen_state(port);
            return;
        }
        /* signal reader and writers with error; signal user   */
        else
        {
            /* by closing port; do not free port until application       */
            /* calls close                                               */
            /* NOTE: let tc_tcp_interpret() do signalling                */
            /*       instead of tc_tcp_abort_connection in case want to  */
            /*       signal error for any other reason then won't        */
            /*       signal twice although it shouldn't matter           */
            DEBUG_LOG("ERROR(rtip) : proc_reset - other side reset, => CLOSED+wakeup", 
                LEVEL_3, NOVAR, 0, 0);
            tc_tcp_abort_connection(port, 
                READ_WAKEUP_FAIL|WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, 
                ENOTCONN, FROM_INTERPRET); 
                                /* wake up any readers or writers   */
#if (DEBUG_ABORT_CONNECTION)
            DEBUG_ERROR("proc_reset: 2: abort connection", NOVAR, 0, 0);
#endif
        }
        break;

    /* *****   */
    case TCP_S_LAST:                    /* FIN sent waiting for ACK */
    case TCP_S_CLOSING:                 /* want ACK of FIN  */
    case TCP_S_TWAIT:                   /* ack FIN again?  */
        DEBUG_LOG("proc_reset - other side reset, => CLOSED", LEVEL_3, NOVAR, 0, 0);
        trans_state(port, TCP_S_CLOSED, FROM_INTERPRET, TRUE);
        break;

    /* *****   */
    default:
        DEBUG_ERROR("ERROR(rtip) : proc_reset - unknown state", NOVAR, 0, 0);
        break;
    }   /* end of switch */
} 

/* ********************************************************************            */
/* void check_option() - process options field in incoming packet                  */
/*                                                                                 */
/*  Look at incoming SYN packet and check for the options field/                   */
/*  containing a TCP Maximum segment size option.  If it has one,                  */
/*  then set the port's internal value to make sure that it never                  */
/*  exceeds that segment size.                                                     */
/*                                                                                 */
static INLINE void check_option(PIFACE pi, PTCPPORT port, DCU msg, word hlen)   /*__fn__*/
{
PFBYTE  pb;
PFWORD  pw;
word    mss;
word    max_mss;
int     opt_len;
int     opt_offset;
RTIP_BOOLEAN mss_rcvd;
PTCPPKT pt;

    pt  = DCUTOTCPPKT(msg);

/*  DEBUG_LOG("check_option entered, hlen = ", LEVEL_3, EBS_INT1, hlen, 0);      */
/*  DEBUG_LOG("check_option:tcp_flags = ", LEVEL_3, EBS_INT1, pt->tcp_flags, 0); */

    opt_len = hlen - TCP_HLEN_BYTES;  /* length of option field (bytes) */
    opt_offset = 0;

    pb = (PFBYTE)pt + TCP_HLEN_BYTES;

    mss_rcvd = FALSE;
    max_mss = pi->pdev->max_mss_out;

    while (opt_offset < opt_len)
    {
        /* MSS - NOTE: tcpinit() will limit mss if sending to gateway   */
        if ( (pb[opt_offset] == TCP_MSS_OPTION) && (pb[opt_offset+1] == 4) )
        {
            /* in no SYN ignore MSS   */
            if (pt->tcp_flags & TCP_F_SYN)
            {
                pw = (PFWORD)(&(pb[opt_offset+2]));  
                mss = net2hs(*pw);

                if (mss < max_mss)        /* we have our own limits too due to */
                    port->out.mss = mss;  /* size of our packet (see MAX_PACKETSIZE) */
                else
                    port->out.mss = max_mss;
                DEBUG_LOG("mss received = ", LEVEL_3, EBS_INT1, mss, 0);
                mss_rcvd = TRUE;
            }
            opt_offset += 4;
            UPDATE_INFO(pi, tcp_options, 1)
        }

        else if (pb[opt_offset] == TCP_TIMESTAMP_OPTION)
        {
#if (INCLUDE_TCP_TIMESTAMP)
            /* save timestamp value so will send back timestamp in reply   */
            port->ap.port_flags |= GOT_TIMESTAMP;
            tc_mv4((PFBYTE)&(port->tsvalue), (PFBYTE)(pb+opt_offset+2), 4);
            if (pt->tcp_flags & TCP_F_SYN) 
            {
                /* save fact that got timestamp, therefore, can send   */
                /* timestamp option                                    */
                port->ap.port_flags |= GOT_TIMESTAMP_SYNC;
            }
#endif
            opt_offset += 10;
            UPDATE_INFO(pi, tcp_options, 1)
        }

        else if (pb[opt_offset] == TCP_EOL_OPTION)
        {
            /* opt_offset += 1;   */
            UPDATE_INFO(pi, tcp_options, 1)
            break;
        }

        else if (pb[opt_offset] == TCP_NOP_OPTION)
        {
            opt_offset += 1;
            UPDATE_INFO(pi, tcp_options, 1)
        }
        else    /* illegal or unsupported option */
        {
            opt_offset += *(pb+1);
            UPDATE_INFO(pi, tcp_options_us, 1)
        }
    }       /* end of while thru options */

    /* if there was not a MSS option in the SYNC message, set it   */
    /* NOTE: might be limited by tcpinit()                         */
    if ( (pt->tcp_flags & TCP_F_SYN) && (!mss_rcvd) )
    {
        port->out.mss = CFG_NO_MSS_VAL; 
        DEBUG_LOG("mss set to default = ", LEVEL_3, EBS_INT1, 
            port->out.mss, 0);
    }
}

/* ********************************************************************      */
/* check_seq() - check sequence number in incoming packet                    */
/*                                                                           */
/*   Take an incoming packet and see if it has acceptable seq number.        */
/*   Proccesses any errors.                                                  */
/*                                                                           */
/*   Returns VALID_PKT if all is ok with the seq number                      */
/*           DROP_PKT if seq number is invalid                               */
/*           PROC_FLAGS_ONLY invalid sequence number but ACKs, URGs and RSTs */
/*               should still be processed                                   */
/*                                                                           */
static int check_seq(PTCPPORT port, PTCPPKT pt, word dlen)  /*__fn__*/
{
dword sq, want;
dword last_sq, last_want;
int   ret_val;

    DEBUG_LOG("check_seq() - entered", LEVEL_3, NOVAR, 0, 0);

    /* get seq from pkt, nxt seq expecting and last seq which will fit in   */
    /* window                                                               */
    sq = WARRAY_2_LONG(pt->tcp_seq);
    want = port->in.nxt;
    last_want = want + port->in.window_size - port->in.contain - 1;

    last_sq = sq + (dword)dlen - 1;
    if (pt->tcp_flags & TCP_F_SYN)
        last_sq++;
    /* if not what want, maybe still want it, maybe not   */
    if ( (dlen > 0) && (sq != want) )   
    {
        /* check if want part of message, i.e. overlap   */
        if (CHECK_GREATER(want, sq) && CHECK_GREATER(sq+dlen, want))
        {     
            DEBUG_LOG("check_seq - overlap - want, sq = ", LEVEL_3, DINT2,
                want, sq);
            DEBUG_LOG("check_seq - overlap - dlen = ", LEVEL_3, DINT1, dlen, 0);

            sq += (word)(want-sq);        /* data off desired, skip this much */
            dlen = (word)(dlen-want+sq);      /* adjust length by amt skiped */

            DEBUG_LOG("check_seq - overlap - want, sq = ", LEVEL_3, DINT2,
                want, sq);
            DEBUG_LOG("check_seq - overlap - dlen = ", LEVEL_3, DINT1, dlen, 0);
        }
    }

    DEBUG_LOG("check_seq : seq want, seq get", LEVEL_3, DINT2, want, sq);
    DEBUG_LOG("check_seq : last seq want, last seq get", LEVEL_3, DINT2,  
        last_want, last_sq);
    DEBUG_LOG("check_seq : data len", LEVEL_3, DINT1, dlen, 0);

    ret_val = DROP_PKT;

    /* receive window full or will not accept any more data cuz got FIN   */
    if (!port->in.size)  /* if no available space in window */
    {
        /* receive window full, if no data and sq is what we want   */
        if ( !dlen && (sq == want) )       
            ret_val = VALID_PKT;

        /* receive window full, data    */
        else
            ret_val = PROC_FLAGS_ONLY;  /* invalid but process ACKs, URGs and RSTs */
    }

    /* receive window not empty   */
    else
    {
        /* receive window not empty   */
        if ( CHECK_GREATER_EQUAL(sq, want) && 
             CHECK_GREATER_EQUAL(last_want,sq) )
            ret_val = VALID_PKT;

        /* receive window not empty, data                                  */
        /* NOTE: if they both wrapped than the comparision is valid        */
        /*       if last_want wrapped but last_sq did not then test passes */
        /*       if last_sq wrapped but last_want did not then test fails  */
        else if ( dlen && 
                  CHECK_GREATER_EQUAL(last_sq, want) && 
                  CHECK_GREATER_EQUAL(last_want, last_sq) )
            ret_val = VALID_PKT;
    }

    /* if reset with 0 seq number process it                             */
    /* NOTE: RFC 793 does not explicitly say to accept this but it says  */
    /*       to send a reset with 0 seq number so process the reset      */
    if (ret_val != VALID_PKT)
    {
        if ( (pt->tcp_flags & TCP_F_RESET) && (sq == 0) )
            ret_val = PROC_FLAGS_ONLY;  /* invalid but process ACKs, URGs and RSTs */
    }

    if ( (ret_val == DROP_PKT) || (ret_val == PROC_FLAGS_ONLY) )
    { 
        /* send ack, could be keepalive packet or maybe a sync from   */
        /* otherside and our ack was lost                             */
        if ( !(pt->tcp_flags & TCP_F_RESET) )
            port->interp_port_flags |= (SEND_PKT | SEND_ACK);      /* send ACK */

#if (DISPLAY_CHECK_SEQ)
        if (ret_val == DROP_PKT)
        {
            DEBUG_ERROR("check_seq : DROP; state,flags = ", EBS_INT2,  
                port->state, pt->tcp_flags);
            DEBUG_ERROR("          : seq want, seq get", DINT2,  
                want, sq);
            DEBUG_ERROR("          : dlen", EBS_INT1, dlen, 0);
            DEBUG_LOG("check_seq : DROP; state,flags = ", LEVEL_3, EBS_INT2,  
                port->state, pt->tcp_flags);
            DEBUG_LOG("check_seq : seq want, seq get", LEVEL_3, DINT2,  
                want, sq);
        }
        else if (ret_val == PROC_FLAGS_ONLY)
        {
            DEBUG_ERROR("check_seq : PROC_FLAGS_ONLY: state,flags = ", EBS_INT2,  
                port->state, pt->tcp_flags);
            DEBUG_ERROR("check_seq : seq want, seq get", DINT2,  
                want, sq);
            DEBUG_LOG("check_seq : PROC_FLAGS_ONLY; state,flags = ", LEVEL_3, EBS_INT2,  
                port->state, pt->tcp_flags);
            DEBUG_LOG("check_seq : seq want, seq get", LEVEL_3, DINT2,  
                want, sq);
        }
        else
        {
            DEBUG_LOG("check_seq : FO; state,flags = ", LEVEL_3, EBS_INT2,  
                port->state, pt->tcp_flags);
            DEBUG_LOG("check_seq : seq want, seq get", LEVEL_3, DINT2,  
                want, sq);
        }
#endif      /* DISPLAY_CHECK_SEQ */
    }

    return(ret_val);
}    

/* ********************************************************************    */
/* proc_syn() - process sync flag in incoming packet                       */
/*                                                                         */
/*   Take an incoming packet and see if it has SYNC bit set.  This routine */
/*   is called for states in which it is an error to have SYNC bit set.    */
/*   Proccesses any errors.                                                */
/*                                                                         */
/*   Returns nothing                                                       */
/*                                                                         */
static void proc_syn(PIFACE pi, PTCPPORT port, DCU  msg)                /*__fn__*/
{
    if (port->from_state == TCP_S_LISTEN)
    {
        DEBUG_LOG("proc_syn - error cuz sync bit set - return to listen", 
            LEVEL_3, NOVAR, 0, 0);
        return_listen_state(port);
    }
    else
    {
        /* signal reader and writers with error; signal user           */
        /* by closing port; do not free port until application         */
        /* calls close                                                 */
        /* NOTE: let tc_tcp_interpret() do signalling                  */
        /*       instead of tc_tcp_abort_connection in case want to    */
        /*       signal error for any other reason then won't          */
        /*       signal twice although it shouldn't matter             */
        /* NOTE: connect will fix errno                                */
        /* NOTE: wake up any readers or writers, accept() or connect() */
        DEBUG_LOG("proc_syn - error: send reset, => CLOSED", LEVEL_3, NOVAR, 0, 0);
        tc_tcp_abort_connection(port, 
            READ_WAKEUP_FAIL|WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, 
            ENOTCONN, FROM_INTERPRET);  
#if (DEBUG_ABORT_CONNECTION)
        DEBUG_ERROR("proc_syn: abort connection", NOVAR, 0, 0);
#endif

        DEBUG_ERROR("proc_syn - send reset", NOVAR, 0, 0);
        tc_tcp_reset(pi, port, msg); /* if it is a reset then tc_tcp_reset */
                                        /* will not actually send the reset   */
    }
}    

/* ********************************************************************   */
/* check_fin() - process FIN bit in incoming packet                       */
/*                                                                        */
/*   Check the FIN bit of an incoming packet to see if the connection     */
/*   should be closing, ACK it if we need to.  This code only handles     */
/*   FINs for the EST, FW1 and FW2 states.                                */
/*   Only called for packet which are not out of order.                   */
/*                                                                        */

static RTIP_BOOLEAN check_fin(PTCPPORT port, PTCPPKT pt, word dlen, int new_state) /*__fn__*/
{
    if (pt->tcp_flags & TCP_F_FIN)
    {
        DEBUG_LOG("check_fin - fin bit set, state=>state", LEVEL_3, EBS_INT2,  
                           port->state, new_state);

        /* fin bit found                                                       */
        /* NOTE: if CWAIT,CLOSING,LAST or TWAIT, this must be a resend of      */
        /* the FIN bit due to the other side not receiving our ack; therefore, */
        /* send the ack again but do not account for their FIN byte again      */
        /* since it was done the first time and the first FIN must be resent   */
        /* (never a new one)                                                   */
        /* NOTE: if the new state is the same as the current state, then       */
        /*       this must be a resend of the same FIN bit                     */
        if (port->state != new_state)
        {
#if (INCLUDE_POSTMESSAGE)
            /* if remote initiated the close   */
            if (port->state == TCP_S_EST)
            {
                port->ap.post_flags |= REMOTE_GRACEFUL_CLOSE;
                DEBUG_ERROR("set REMOTE_GRACEFUL_CLOSE: index ", 
                    EBS_INT1, port->ap.ctrl.index, 0);
            }
#endif

            /* check sequence number of FIN bit; it is not spelled out        */
            /* in RFC 793 what to do in this case i.e. don't process the fin, */
            /* ACK the FIN bit specified in packet, or ACK one past           */
            /* current seq number;                                            */
            /* this code does the second (ACK FIN specified in packet)        */
            if ( port->in.nxt != (WARRAY_2_LONG(pt->tcp_seq) + dlen) )       
            {
                DEBUG_ERROR("check_fin - FIN seq num exp, act = ",
                    DINT2, port->in.nxt, WARRAY_2_LONG(pt->tcp_seq));
                port->interp_port_flags |= (SEND_PKT | SEND_ACK); /* force ACK to be sent  */
                return(FALSE);
            }
#if (1)
            /* count the FIN bit                                      */
            /* NOTE: if read shutdown in.nxt not incremented for data */
            port->in.nxt = WARRAY_2_LONG(pt->tcp_seq) + dlen + 1;       
#else
            port->in.nxt++;
#endif
            port->in.size = 0;          /* set input window size to 0, i.e. */
                                        /* do not accept anymore incoming        */
                                        /* (also needs to be done for check_seq) */
            port->in.ad_size = 0;       /* set window size to advertize  */
            DEBUG_LOG("check_fin - send ack of FIN - in.nxt = ", LEVEL_3, DINT1,
                port->in.nxt, 0);
            DEBUG_LOG("          - set in.ad_size to 0", LEVEL_3, NOVAR, 0, 0);
            if (new_state == TCP_S_TWAIT)
                trans_state(port, TCP_S_TWAIT, TRUE, FALSE);
            else
            {
                trans_state(port, new_state, FROM_INTERPRET, FALSE);   
                                                    /* go to CWAIT or CLOSING    */
#                if (INCLUDE_SNMP)
                    if (port->state != new_state)
                    {
                        /* CWAIT->LAST;                                 */
                        /* NOTE: EST->CWAIT will not incr or decr count */
                        if (port->state == TCP_S_CWAIT) 
                            DECR_SNMP(TcpCurrEstab)
                    }
#                endif
            }
        }

        /* if READ is shutdown, still ack the fin; let transf() know this   */
        /* by setting nxt_to_send back to 0 forcing transf to ack the       */
        /* value in nxt instead of nxt_to_send                              */
        /* NOTE: nxt_to_send contains ack value when shutdown done and ack  */
        /*       keeps getting updated to reflect received data             */
        if (port->ap.port_flags & READ_SHUTDOWN)
            port->in.nxt_to_send = 0;

        port->interp_port_flags |= (SEND_PKT | SEND_ACK); /* force ACK to be sent  */
        port->interp_port_flags |= READ_WAKEUP_SUCCESS;   /* wake up readers */

        /* let application know FIN was received   */
        CB_ERROR_REPORT(port->ap.ctrl.index, TCP_CONNECTION_CLOSING, 0);
    
        /* **************************************************                   */
        /* At this point, we know that we have received all data that the other */
        /* side is allowed to send.                                             */
        /* NOTE: there still can be data in input window, which is ok and       */
        /*       window should be emptied by reads                              */
        port->credit = 0;                     

#if (INCLUDE_POSTMESSAGE)
        if (port->in.contain == 0)
        {
            if (port->ap.post_flags & REMOTE_GRACEFUL_CLOSE)
            {
                PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_CLOSE, 0);
            }
            /* local system did graceful close with shutdown, and remote   */
            /* responded with END OF DATA notification                     */
            else if (port->ap.post_flags & GRACEFUL_CLOSE)
            {
                PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_CLOSE, 0);
            }
        }
        else
        {
            if (port->ap.post_flags & (REMOTE_GRACEFUL_CLOSE|GRACEFUL_CLOSE))
            {
                port->ap.post_flags &= 
                    ~(REMOTE_GRACEFUL_CLOSE | GRACEFUL_CLOSE);
                port->ap.post_flags |= WAIT_IN_WIN_EMPTY;
                DEBUG_ERROR("set WAIT_IN_WIN_EMPTY", NOVAR, 0, 0);
            }
        }
#endif

        return(TRUE);
    }
    return(FALSE);
}    

/* ********************************************************************   */
/* check_close() - continue processing socket close                       */
/*                                                                        */
/*   Finish process a socket close for the case where API did a close     */
/*   but had to wait for output data to be acked before transitioning     */
/*   to FW1 (we did close first) or LAST (otherside did close first)      */
/*                                                                        */
void check_close(PTCPPORT port)        /*__fn__*/
{
    /* if all output data has been acked and app previously            */
    /* did an closesocket() send FIN go to FW1 or LAST state to        */
    /* wait for ack, i.e. closesocket was executed but state           */
    /* transition was not done since not all output had been           */
    /* acked                                                           */
    /* NOTE: ACK bit must be set with FIN bit (see RFC 793 pg 72. i.e. */
    /*       other side drops segment if ACK not set)                  */
    if ( !port->out.contain && (port->ap.port_flags & API_CLOSE_DONE) )
    {
        DEBUG_LOG("check_close, send FIN: old state=", LEVEL_3, EBS_INT1, port->state, 0);
        if (port->state == TCP_S_EST)
            trans_state(port, TCP_S_FW1, FROM_INTERPRET, FALSE);
        else if (port->state == TCP_S_CWAIT)
        {
            trans_state(port, TCP_S_LAST, FROM_INTERPRET, FALSE);
            port->closetime = 0;       /* start timeout timer (until close) */
        }
        else 
            return;

        DECR_SNMP(TcpCurrEstab)
        port->interp_port_flags |= (SEND_PKT | SEND_FIN | SEND_ACK);   /* send FIN  */
    }
}

/* ********************************************************************     */
/* check_ack() - process ack and window size in incoming TCP packet         */
/*                                                                          */
/*   Take an incoming packet which has an ACK for the outgoing              */
/*   side.  Use that ACK to dequeue outgoing data.  Also, pick up the other */
/*   sides window size and if we have more data to send, send it (now       */
/*   that we have new window size)                                          */
/*                                                                          */
/*   Returns TRUE if data was removed from input window.  FALSE if it was   */
/*   not.                                                                   */
/*                                                                          */
static void check_ack(PIFACE pi, PTCPPORT port, PTCPPKT pt, word dlen) /*__fn__*/
{
dword  ak;
int    temp;
int    i;
RTIP_BOOLEAN skip_win_update;
word   size_before;
word   unacked_data;
word   ws;
long   err;
long   calc_rto;
#if (INCLUDE_FAST_RETRANS)
PIFACE pi_out;
#endif
long   out_size;

#if (!INCLUDE_KEEP_STATS || !INCLUDE_FAST_RETRANS)
    ARGSUSED_PVOID(pi);
#endif

#if (!INCLUDE_FAST_RETRANS)
    ARGSUSED_INT(dlen);
#endif

    /* **************************************************         */
    /* save other sides window size (allowable transmission size) */
    port->out.ad_size = pt->tcp_window;  /* save so timeout can */
                                         /* set to advertized size   */

    size_before = port->out.size;

    /* **************************************************              */
    /* calculate new window size                                       */
    /* if remote is acking more than nxt_to_send (due to window        */
    /* probe or shrinking window),                                     */
    /* NOTE: nxt_to_send could be less than tcp_ack if previously sent */
    /*       a window probe OR it is a duplicate/old ack               */
    skip_win_update = FALSE;
    if (CHECK_GREATER(port->last_ack, WARRAY_2_LONG(pt->tcp_ack)))
    {
        DEBUG_ERROR("SKIP WINDOW UPDATE: last_ack, tcp_ack: ",
            DINT2, port->last_ack, WARRAY_2_LONG(pt->tcp_ack));
        skip_win_update = TRUE;
        /* skip updating window size   */
    }
    else
        port->last_ack = WARRAY_2_LONG(pt->tcp_ack);

    if (!skip_win_update && 
        CHECK_GREATER_EQUAL(port->out.nxt_to_send, WARRAY_2_LONG(pt->tcp_ack)))
    {
#if (DEBUG_DEQUE)
        DEBUG_ERROR("check_ack: acked data = ", DINT2,
            port->out.nxt_to_send, WARRAY_2_LONG(pt->tcp_ack));
#endif

        /* new window size is advertised window size minus any            */
        /* unacked data                                                   */
        /* NOTE: take into consideration which byte they are acking to    */
        /*       go along with the window size advertised in case we have */
        /*       sent more data than what they are acking, subtract       */
        /*       off the difference                                       */
        out_size = (long)pt->tcp_window - 
                (long)(port->out.nxt_to_send - WARRAY_2_LONG(pt->tcp_ack));      
        DEBUG_LOG("check_ack: nxt_to_send, tcp_ack = ", LEVEL_3, DINT2,
            port->out.nxt_to_send, WARRAY_2_LONG(pt->tcp_ack));
        DEBUG_LOG("check_ack: out_size, pt->tcp_window",
            LEVEL_3, DINT2, out_size, pt->tcp_window);

        if ( (out_size >= 0) && (out_size <= 0xffff) )
            port->out.size = (word)out_size;
        else
        {
            /* if window size is negative, the remote must have shrunk   */
            /* its window; while the remote should not do that, it       */
            /* still must be guarded against;                            */
            /* ignore the new window size and force a window probe       */
            /* also stop the retransmit timer to avoid timing out        */
            /* on data the remote does not have in its shrunken window;  */
            DEBUG_ERROR("ws < 0; calc new ws, seq = ", DINT2, 
                out_size, WARRAY_2_LONG(pt->tcp_ack));
            DEBUG_ERROR("parameters to calc new ws = ", DINT2,
                (long)pt->tcp_window, (long)(port->out.nxt_to_send));
            DEBUG_ERROR("parameters to calc new ws = ", DINT2,
                WARRAY_2_LONG(pt->tcp_ack), 
                port->out.nxt_to_send - WARRAY_2_LONG(pt->tcp_ack));      

            /* stop retrans timer in case timing for ACK of SYNC   */
            TCP_STOP_RETRANS(port)

            port->out.size = 0;     /* forces a window probe */
        }
    }

    /* **************************************************                  */
    /* reset probing in case timeout does not come in to reset it          */
    /* before another window probe starts; this is done to ensure          */
    /* for the case a non-zero window size comes in followed by a zero     */
    /* window size before the 1 sec timeout, in this case the timeout      */
    /* value for sending window probes should start from the default value */
    if ( (port->out.size != 0) && (port->out.ad_size != 0) )
            port->ap.port_flags &= ~WINDOW_PROBE_STARTED;

    /* get max window size otherside has ever advertised   */
    unacked_data = (word)(port->out.nxt_to_send - port->out.nxt);
    ws = (word)(port->out.ad_size - unacked_data);
    if (ws >= port->out.max_size)
        port->out.max_size = ws;

    /* **************************************************   */
    if (!port->out.contain)
        return;

    /* **************************************************                    */
    /*  rmqueue any bytes which have been ACKed, update port->out.nxt to the */
    /*  new next seq number for outgoing.  Update send window.               */
    ak = WARRAY_2_LONG(pt->tcp_ack);            /* other side's ACK  */

    /* If ak is not increasing (above port->out.nxt) then we should assume      */
    /* that it is a duplicate packet or a keepalive packet that 4.2BSD sends    */
    /* out (tbd - why does non increasing ack signal keepalive - non increasing */
    /* seq num means keepalive - see pg 335 of Stevens)                         */
    /* NOTE: if they are equal then there is 1 byte being acked, it ak is       */
    /*       the highest byte being acked and out.nxt is the first byte         */
    /*       which has not been acked                                           */
    if (CHECK_GREATER(ak, port->out.nxt))
    {
        /* DATA HAS BEEN ACKED, REMOVE ACKED DATA FROM OUTPUT WINDOW          */
        /* Do the subtraction through temp. This will correctly subtract the  */
        /* two even if there is a segment wrap.                               */
        temp = (int)( (long)((long)ak - (long)port->out.ack) );
        i = rmqueue(&port->out, temp);

        /* update seq # and what other side has acked (just take into account   */
        /* data which was in window-not FIN or SYNC                             */
        port->out.nxt = port->out.nxt + (long)i;
        port->out.ack = port->out.ack + (long)i;
        DEBUG_LOG("check_ack - after update out.nxt, out.nxt,i = ", LEVEL_3, DINT2, 
                  port->out.nxt, (long)i);

        /* if out.nxt is larger than nxt_to_send then fix it by setting   */
        /* nxt_to_send to nxt;                                            */
        /* NOTE: nxt is first unacked byte and nxt_to_send is first       */
        /*       unsent byte                                              */
        /* NOTE: this could occur if a retransmit and the response from   */
        /*       the origional message comes in before all of the         */
        /*       retransmit data may be sent; possibly if the remote      */
        /*       side decreased it window;                                */
        /*       this could also occur if the remote host acks a window   */
        /*       probe; nxt would be one more than nxt_to_send; it is     */
        /*       easy for this to happen if the remote host happens to    */
        /*       do a read and room opens up in its input window just as  */
        /*       we have decided we need to send a window probe           */
        /* NOTE: this could also occur if a retransmit occurs and the     */
        /*       remote side handles out of order packets;  due to the    */
        /*       congestion window processing, the retransmit code will   */
        /*       not transmit all the data in the output window;  when    */
        /*       the remote host receives part of the data since it keeps */
        /*       out of order packets, it could be acknowleding more      */
        /*       data than we have sent since the retransmit (remember    */
        /*       the retransmit sets nxt_to_send back to nxt)             */
        /* NOTE: only do this if have not sent a FIN yet or will mess     */
        /*       up the close procedure                                   */
        if (CHECK_GREATER(port->out.nxt, port->out.nxt_to_send) &&
            tc_is_write_state(port))
        {
            DEBUG_ERROR("check_ack - NOTE: nxt > nxt_to_send - nxt, nxt_to_send = ",
                DINT2, port->out.nxt, port->out.nxt_to_send);

            /* set next byte to send back to the first byte in the window; i.e.   */
            /* set nxt_to_send to nxt                                             */
            reset_nxt_to_send(port);
        }

#if (INCLUDE_TCP_RTT)
        /* **************************************************                */
        /*  Check to see if this acked our transmission we are measuring.    */
        /*  If so, adjust the RTO value to reflect the newly measured RTT.   */
        /*  This formula reduces the RTO value so that it gradually          */
        /*  approaches the most recent round trip measurement.               */
        /*  When a packet is retransmitted, this value is doubled            */
        /*  (exponential backoff).                                           */
        if (port->rtt_nxt && 
            CHECK_GREATER(port->out.nxt, port->rtt_nxt))
        {
            err = (long)port->rtttime - port->smooth_rtt;
            port->smooth_rtt += (err >> 3);
            if (err < 0)
                err = -err;
            port->smooth_mean_dev += 
                ((err - port->smooth_mean_dev) >> 2);
            calc_rto = port->smooth_rtt + (port->smooth_mean_dev << 2);

#if (DEBUG_RTT)
            DEBUG_ERROR("**** rtttime, smooth_rtt", EBS_INT2, 
                port->rtttime, port->smooth_rtt);
            DEBUG_ERROR("     smooth_mean_dev,rto = ", EBS_INT2, 
                port->smooth_mean_dev, port->rto);
#endif

            /* limit the calculation of rto   */
            if (calc_rto > (long)CFG_MAXRTO)
                port->rto = CFG_MAXRTO;
            else if (calc_rto < (long)CFG_MINRTO)
                port->rto = CFG_MINRTO;
            else
                port->rto = calc_rto;
#if (DEBUG_RTT)
            DEBUG_ERROR("   port->rto = ", EBS_INT1, port->rto, 0);
#endif
            port->rtt_nxt = 0;  /* reset so can measure again */
            port->rtttime = 0;
        }
#endif  /* INCLUDE_TCP_RTT */

#if (INCLUDE_FAST_RETRANS)
        /* **************************************************       */
        /* if ack for byte tracking for fast retransmit has arrived */
        /* do not need to do fast retransmit                        */
        if (port->fr_nxt &&
            CHECK_GREATER(port->out.nxt, port->fr_nxt))
        {
#if (INCLUDE_CWND_SLOW)
            port->cwnd = port->ssthresh;
            incr_cwnd(port, 0);     /* limit cwnd */
#endif      /* INCLUDE_CWND_SLOW */

            /* reset duplicate ack count   */
            port->dup_ack_cnt = 0;
            port->fr_nxt = 0;

#if (DEBUG_FR)
            DEBUG_ERROR("check_ack - reset dup_ack_cnt - fr_nxt = ", DINT1, 
                port->fr_nxt, 0);
            DEBUG_ERROR("            nxt, nxt_to_send =", DINT2, 
                port->out.nxt, port->out.nxt_to_send);
#endif
        }

#if (DEBUG_FR)
        DEBUG_ASSERT(!port->fr_nxt, "check_ack - DO NOT reset dup_ack_cnt - fr_nxt, =", DINT1, 
            port->fr_nxt, 0);
        DEBUG_ASSERT(!port->fr_nxt, "            nxt, nxt_to_send =", DINT2, 
            port->out.nxt, port->out.nxt_to_send);
#endif
#endif  /* INCLUDE_FAST_RETRANS */

        /* **************************************************           */
        /* if any data bytes were removed from the output window do the */
        /* following related to timeout processing                      */
        if (i)
        {
            /* stop retransmission timer   */
            TCP_STOP_RETRANS(port)

            /* start retransmission timer over on first byte which   */
            /* has already been sent but not acknowledged            */
            if (port->out.nxt < port->out.nxt_to_send)
                start_retrans_timer(port);

            /* - always wake up send (i.e. tc_tcp_write - API layer)           */
            /* in order to queue more data                                     */
            /* - only wake up close if output window is empty                  */
            /* - for SELECT_TCP_EMPTY:                                         */
            /*   only wake up select if the output window  is empty;           */
            /* - for !SELECT_TCP_EMPTY:                                        */
            /*   if blocking:     wake up select when window empty             */
            /*   if non-blocking: wake up select when any data is removed      */
            /*                    from the output window; this way for         */
            /*                    non-blocking mode the application            */
            /*                    will wake up and queue more data.            */
            /*                    NOTE: For !SELECT_TCP_EMPTY it makes no      */
            /*                    sense to call select in blocking mode since  */
            /*                    write does not return until the output       */
            /*                    window empties                               */
            if (!port->out.contain || 
                !(port->ap.port_flags & WAIT_GRACE_CLOSE) )
            {
                port->interp_port_flags |= WRITE_WAKEUP_SUCCESS;    
                DEBUG_LOG("check_ack - wake up any writers", LEVEL_3, NOVAR, 0, 0);
            }

#if (INCLUDE_POSTMESSAGE)
            /* if previous send failed due to EWOULDBLOCK since            */
            /* data was ack then then Post Message in case send()/sendto() */
            /* might now succeed                                           */
            if (port->ap.post_flags & EWOULDBLOCK_WRITE)
            {
                port->ap.post_flags &= ~EWOULDBLOCK_WRITE;
                PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_WRITE, 0);
            }

            /* if local or remote initiated the close, if the window   */
            /* is now empty then Post Message                          */
            if ( (port->ap.post_flags & WAIT_IN_WIN_EMPTY) &&
                  !port->out.contain )
            {
                DEBUG_ERROR("call PostMessage due to WAIT_IN_WIN_EMPTY",
                    NOVAR, 0, 0);
                port->ap.post_flags &= ~WAIT_IN_WIN_EMPTY;
                PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_CLOSE, 0);
            }
#endif
            /* check if should wake up select                                */
            /* if select_size is set, this overrides the behavior of         */
            /* select() in that it will be ready when select_size free bytes */
            /* in output window                                              */
            if (port->select_size)
            {
                if ( (port->out.contain + port->select_size) <=
                      port->out.window_size )
                    port->interp_port_flags |= SELECT_WRITE_WAKEUP;    
            }
#if (SELECT_TCP_EMPTY)
            else if (!port->out.contain)
            {
                DEBUG_LOG("check_ack - wake up select", LEVEL_3, NOVAR, 0, 0);
                port->interp_port_flags |= SELECT_WRITE_WAKEUP;    
                                                        /* wake up writers   */
            }
#else
            else if ( ((port->ap.options & IO_BLOCK_OPT) && 
                       !port->out.contain) ||
                      !(port->ap.options & IO_BLOCK_OPT) )
            {
                port->interp_port_flags |= SELECT_WRITE_WAKEUP;    
            }
#endif

#if (INCLUDE_CWND_SLOW)
            /* increase congestion window                               */
            /* NOTE: congestion avoidance adds (mss*mss)/cwnd           */
            /* NOTE: slow start doubles congestion window by adding one */
            /*       mss for each ack; the intention is to increase     */
            /*       by small amounts with each ack to make the         */
            /*       increase average approximately one segment over    */
            /*       an entire window (see Comar Vol 2 - pg 274-275)    */
            if (port->cwnd <= port->ssthresh)   /* if slow start */
            {
                incr_cwnd(port, port->out.mss);
            }
            else                                /* if congestion avoidance */
                incr_cwnd(port, (word)((port->out.mss * port->out.mss)/port->cwnd));
#endif      /* INCLUDE_CWND_SLOW */
        }

        /* push flag   */
        if (!port->out.contain && port->out.push) /* if the queue emptied and */
                                                  /* push was set, clear push    */
        {
            port->out.push = 0;
        }


#if (TEST_FAST_RETRANS)
{
        if (test_fast_retrans)
        {
            test_fast_retrans = 0;
            do_retrans(port->ap.iface, port, FAST_RETRANS); 
        }
}
#endif

    }       /* if ack has increased */


#if (INCLUDE_FAST_RETRANS)
    /* **************************************************             */
    /* check if it is a duplicate - i.e. ack value has not increased, */
    /* there is no additional data in the packet and the window size  */
    /* has not increased                                              */
    else if ( (dlen == 0) && (size_before == port->out.size) &&
              (CHECK_RETRANS(port)) )
    {
        port->dup_ack_cnt++;
#if (DEBUG_FR)
        DEBUG_ERROR("check_ack - dup ack cnt,dlen = ", EBS_INT2, 
            port->dup_ack_cnt, dlen);
        DEBUG_ERROR("          - size_before, out.size", EBS_INT2,
            size_before, port->out.size);
#endif
        
        /* if first dup packet, start tracking for fast retransmit   */
        if (port->dup_ack_cnt == 1)
            port->fr_nxt = port->out.nxt_to_send;

        /* third dup packet has arrived while tracking for fast transmit,       */
        /* start congestion window processing; update ssthresh; retransmit then */
        /* update cwnd                                                          */
        if (port->dup_ack_cnt == DUP_ACK_FR)
        {
#if (DEBUG_FR)
            DEBUG_ERROR("check_ack - third dup_ack_cnt - cong avoid, retrans: nxt, nxt_to_send ",
                DINT2, port->out.nxt, port->out.nxt_to_send);
#endif
#if (INCLUDE_CWND_SLOW)
            /* update cwnd   */
            congestion_proc(port, FALSE);
#endif      /* INCLUDE_CWND_SLOW */

            /* FAST RETRANSMIT A SEGMENT but only if retransmission is enabled,   */
            /* i.e. arp is resolved                                               */
            /* get iface associated with port - check the open count by ourselves */
            /* this way we won't print unneeded warnings                          */
            pi_out = port->ap.iface;
            if ( pi_out && pi_out->open_count &&
                 (port->ap.port_flags & RETRANS_EN) )
            {
                do_retrans(pi_out, port, FAST_RETRANS);

#if (INCLUDE_CWND_SLOW)
                port->cwnd = (word)(port->ssthresh + 3*port->out.mss);
                incr_cwnd(port, 0);     /* limit cwnd */
#endif      /* INCLUDE_CWND_SLOW */
            }

        }

        /* another duplicate ack while tracking for fast retransmit;    */
        /* update cwnd and retransmit                                   */
        else if (port->dup_ack_cnt > DUP_ACK_FR)
        {
#if (DEBUG_FR)
            DEBUG_ERROR("check_ack - > 3 dup_ack_cnt - trans: nxt, nxt_to_send ",
                DINT2, port->out.nxt, port->out.nxt_to_send);
#endif
#if (INCLUDE_CWND_SLOW)
            incr_cwnd(port, port->out.mss);
#endif      /* INCLUDE_CWND_SLOW */

            /* ANOTHER DUP ACK; transmit a packet (one which has   */
            /* never been transmitted) based upon cwnd             */
            port->interp_port_flags |= SEND_ONE_SEG_DATA; 
        }

        UPDATE_INFO(pi, tcp_duplicates, 1)
    }
#endif          /* INCLUDE_FAST_RETRANS */

    /* **************************************************               */
    /* if there still is data in output window which has never been     */
    /* sent, send it;                                                   */
    /* this could occur if a previous write could not send all of its   */
    /* data due to otherside's window size not being large enough       */
    /* or slow start or congestion avoidence is in effect;              */
    /* now that we got a message from the otherside with its current    */
    /* window size and all acked data has been removed from the window, */
    /* we can try again to send the data                                */
    /* NOTE: if just send all data in the window instead of what has    */
    /*       not been sent, it slows performace down since write will   */
    /*       not wait until completed causing data to be in the window  */
    /*       causing many extra unnecessary packet sends                */
    /* NOTE: if pkt is lost causing data to be in the window, timeout   */
    /*       will resend the whole buffer; it takes awhile for timeout  */
    /*       to detect this but it should rarely happen                 */
    if (port->out.nxt+port->out.contain > port->out.nxt_to_send) 
        port->interp_port_flags |= SEND_DATA_PKT; /* force a transmit */

}    

/* ********************************************************************   */
/* check_data() - process data in incoming TCP packet                     */
/*                                                                        */
/*   Take a packet which has arrived for an established connection and    */
/*   if it contains data put it in the input window if it is not out of   */
/*   order.  If it is out of order, put it in the out of order list.      */
/*                                                                        */
/*   Returns TRUE if packet is not out of order.  FALSE if it is.         */
/*                                                                        */
static RTIP_BOOLEAN check_data(PTCPPORT port, DCU msg, PTCPPKT pt, /*__fn__*/
                          word dlen, word hlen,                          /*__fn__*/
                          RTIP_BOOLEAN KS_FAR *queued, RTIP_BOOLEAN out_of_order)  /*__fn__*/
{
dword   sq,want;
#if (INCLUDE_FRAG && INCLUDE_TCP_COPY)
word    loop_dlen;
#endif
word    orig_dlen;
word    tot_que;
PTCPPKT curr_pt;
#if (INCLUDE_TCP_COPY)
PFBYTE  p;
word    curr_que;
word    curr_dlen;
#endif
#if (INCLUDE_FRAG)
word    save_hlen;
DCU     curr_msg;
int     curr_data_len;
#endif

    DEBUG_LOG("check_data() - entered", LEVEL_3, NOVAR, 0, 0);
#if (!INCLUDE_TCP_OUT_OF_ORDER)
    ARGSUSED_INT(out_of_order);
#endif

    /*  see if we want this packet, or is it a duplicate?   */
    sq = WARRAY_2_LONG(pt->tcp_seq);
    want = port->in.nxt;
    DEBUG_LOG("check_data : seq want, seq get", LEVEL_3, DINT2,  want, sq);

    orig_dlen = dlen;
    if (sq != want)   /* if not what want, maybe still want it, maybe not */
    {
        /* check if want part of message, i.e. overlap   */
        if (dlen>0 && sq<want && (sq+dlen)>want) 
        {                                   
            hlen = (word)(hlen+want-sq);   /* data off desired, skip this much */
            dlen = (word)(dlen-want+sq);   /* adjust length by amt skiped */
        }

        /* there is a gap in seq # or everything in message we already have    */
        /* NOTE: this could be an old implementation of keepalive based        */
        /*       upon 4.2BSD which sends a garbage byte with seq number        */
        /*       which has already been acked; must respond to it              */
        else
        {                                  /* tough it  */
            DEBUG_LOG("check_data - don't want pkt", LEVEL_3, NOVAR, 0, 0);

            /* check for out of order; i.e. if (sq > want)   */
            if (CHECK_GREATER(sq, want))
            {
                DEBUG_ERROR("check_data - out of order: sq, want", 
                    DINT2, sq, want);
#if (INCLUDE_TCP_OUT_OF_ORDER)
                if (!out_of_order)
                {
                    /* force a ACK to be sent (but don't wakeup); this   */
                    /* is necessary so remote FAST RETRANSMIT will kick  */
                    /* in on a lost segment                              */
                    port->interp_port_flags |= SEND_PKT;                            
    
                    /* add segment to out of order list   */
                    add_tcpdcu_list_order(msg, port, queued);
                }
#endif  /* INCLUDE_TCP_OUT_OF_ORDER */
                return(FALSE);
            }

            /* must be a duplicate packet   */
            DEBUG_ASSERT(dlen == 0, "check_data - dup", DINT2, sq, want);
            DEBUG_ASSERT(dlen == 0, "check_data - flags, dlen", EBS_INT2, 
                pt->tcp_flags, dlen);
            return(TRUE);
        }
    }

    /* since no data it could be only an ACK packet or a keepalive, i.e.   */
    /* no data and invalid seq number (i.e. one that has already been      */
    /* acked); if it is an invalid seq number that it is a keepalive       */
     /* and we must respond with ACK/ack                                   */
    else if (dlen == 0)               /* only an ACK packet  */
    {
        DEBUG_LOG("check_data - dlen = 0", LEVEL_3, NOVAR, 0, 0);
        if (sq != want)    /* if not what want, must be keepalive  */
        {
            port->interp_port_flags |= SEND_PKT;
        }
        return(TRUE);
    }

    /* **************************************************   */
    if (port->ap.port_flags & READ_SHUTDOWN)
    {
        tc_tcp_abort_connection(port, 
            READ_WAKEUP_FAIL|WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, 
            ENOTCONN, FROM_INTERPRET);  
#if (DEBUG_ABORT_CONNECTION)
        DEBUG_ERROR("check_data: abort connection", NOVAR, 0, 0);
#endif
        tc_tcp_reset(port->ap.iface, port, msg); 
        return(TRUE);
    }

    /* **************************************************                     */
    /* PROCESS DATA                                                           */
    /* we have data; queue it in input window and update window sizes,        */
    /* etc etc                                                                */
    /* NOTE: if they sent more than we advertized and more than have room for */
    /*       then it SHOULD be a window probe; probably if there is           */
    /*       room for the data we could queue it and don't do the             */
    /*       delayed ACK but if they are close to the ragged edge just        */
    /*       treat it as a probe and don't queue the data; it works           */
    /*       just as well, takes less code and is less trusting of            */
    /*       a packet sent outside of the input window                        */
    if ( (port->in.ad_size >= dlen) && (port->in.size >= dlen) )
    {
#if (DEBUG_DATA_QUE_IN)
        DEBUG_ERROR("check_data: queue data in input window: dlen = ",
            EBS_INT1, dlen, 0);
#endif

        /* QUEUE PACKET IN INPUT WINDOW   */
#if (INCLUDE_TCP_NO_COPY)
        if (port->ap.options & SO_TCP_NO_COPY)
        {
            /* set up in case socket recv called;                            */
            /* NOTE: handles overlap since hlen and dlen were adjusted above */
            TOTAL_LL_HLEN_SIZE(DCUTOPACKET(msg)->data_start, 
                               msg_is_802_2(msg), DCUTOCONTROL(msg).pi, msg)
            DCUTOPACKET(msg)->data_start += (hlen + IP_HLEN_DCU(msg));
            DCUTOPACKET(msg)->data_len = DCUTOPACKET(msg)->frag_data_len = 
                dlen;

#if (INCLUDE_FRAG)
            curr_msg = msg;
            save_hlen = hlen;
            while (curr_msg)
            {
                DCUTOPACKET(curr_msg)->frag_data_len = curr_data_len = 
                    (word)(IP_LEN_DCU(curr_msg) -      /* IP len */
                           IP_HLEN_DCU(curr_msg) -     /* IP header len */
                           hlen);                      /* TCP header len */
                if ((int)hlen > curr_data_len)
                {
                }
                hlen = 0;
                curr_msg = DCUTOPACKET(curr_msg)->frag_next;
            }
            hlen = save_hlen;
#endif

            /* enqueue packet even if fragmented   */
            enqueue_pkt((PWINDOW)&port->in, msg, dlen);
            tot_que = dlen;
#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = IN_IN_WIN_LIST;
#endif
            *queued = TRUE;     /* pkt queued->interpret should not free */
        }
        else        /* COPY MODE */
#endif              /* INCLUDE_TCP_NO_COPY */
        {
#if (INCLUDE_TCP_COPY)
            tot_que = 0;
#        if (!INCLUDE_FRAG)
            curr_dlen = dlen;
            curr_pt = DCUTOTCPPKT(msg);
#        else
            loop_dlen = dlen;
            curr_msg = msg;      
            while (curr_msg && loop_dlen)
            {
                curr_pt = DCUTOTCPPKT(curr_msg);
                curr_dlen = (word)(IP_LEN_DCU(curr_msg) - IP_HLEN_DCU(curr_msg) - 
                                   hlen);
#        endif
                /* COPY mode so pass in as buffers, i.e. there will   */
                /* not be any frags in input window for COPY mode     */
                /* p points past header and TCP options               */
                p = (PFBYTE)curr_pt + hlen;
                curr_que = enqueue_data_in(port, p, curr_dlen);
                tot_que += curr_que;

#        if (INCLUDE_FRAG)
                /* if there was a problem queueing do not continue   */
                if (curr_que < curr_dlen)
                    break;

                loop_dlen -= curr_dlen;
                curr_msg = DCUTOPACKET(curr_msg)->frag_next;
                hlen = 0;       /* only 1st frag has a TCP header */
                                /* NOTE: hlen not used again in this routine so ok   */
            }
#        endif
#endif          /* INCLUDE_TCP_COPY */
        }

#if (DISPLAY_MAX_INPUT_QUE)
{
static long max_in_que = 0;

        if ((long)port->in.contain > max_in_que)
        {
            max_in_que = port->in.contain;
            DEBUG_ERROR("MAX IN QUE: INPUT QUE ", LINT1, max_in_que, 0);
        }
}
#endif

        /* all data not necessarily queued (i.e. if ran out of DCUs while   */
        /* trying to queue); so adjust accordingly to make sure do not      */
        /* ack the data which was not queued                                */
        orig_dlen = (word)(orig_dlen - (dlen-tot_que));

        /* update the ACK field values                                          */
        /* NOTE: update ack value using dlen before it was adjusted for overlap */
        DEBUG_LOG("check_data - there is room, dlen = ", 
            LEVEL_3, EBS_INT1, dlen, 0);
        port->in.nxt = WARRAY_2_LONG(pt->tcp_seq) + orig_dlen;

        /* update input window sizes    */
        port->in.size -= tot_que;           /* real size  */
        port->in.ad_size -= tot_que;        /* size to advertize */
        DEBUG_LOG("check_data: set ad_size to, tot_que", LEVEL_3, DINT2,
            port->in.ad_size, tot_que);

        /* wake up readers when queue empty before enqueue and force        */
        /* sending ACK response and                                         */
        /* Post Message: FD_READ - data has arrived                         */
        /* NOTE: tc_tcp_read will only wait on signal if no data in window, */
        /*       so to prevent to many signals being send, only send it     */
        /*       if need to, i.e. if other side does lots of sends before   */
        /*       our side does a read could get signal overflow queue       */
        /* NOTE: check in.contain before calling enqueue                    */
        if (tot_que)
        {
#if (INCLUDE_POSTMESSAGE)
            PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_READ, 0);
#endif
            port->interp_port_flags |= READ_WAKEUP_SUCCESS;         
        }

        /* DELAY ACK : do not send ack right away (wait for delay timeout)      */
        /* but in stream of full sized segments, need to send ack every second  */
        /* segment                                                              */
        /* NOTE: this option has to be enabled in order to delay the acks;      */
        /*       default is to have the option enabled                          */
        if ( !(port->ap.options & SO_DELAYED_ACK) ||
             (CFG_MAX_DELAY_ACK <= 0) ||
             ((dlen == port->in.mss) && 
              (port->interp_port_flags & DELAYED_LAST)) )
        {
            port->interp_port_flags |= (SEND_PKT | SEND_ACK);             
            port->interp_port_flags &= ~DELAYED_LAST;
#            if (DEBUG_DACK)
                DEBUG_ERROR("check_data - delayed last so send", NOVAR, 0, 0);
#            endif
        }
        else
        {
#            if (DEBUG_DACK)
                DEBUG_ERROR("check_data - set delayed ack", NOVAR, 0, 0);
#            endif
            if (dlen == port->in.mss) 
            {
                port->interp_port_flags |= DELAYED_LAST;
#                if (DEBUG_DACK)
                    DEBUG_ERROR("check_data - set delayed last", NOVAR, 0, 0);
#                endif
            }

            /* if not already timing a delayed ack then reset the timer   */
            if ( !(port->interp_port_flags & DELAYED_ACK) )             
            {
#                if (DEBUG_DACK)
                    DEBUG_ERROR("check_data - reset timer", NOVAR, 0, 0);
#                endif
                port->dactime = 0;  /* NOTE: this is only place need to reset  */
                                    /*       since time out processing won't    */
                                    /*       increment unless flag set          */
            }
            port->interp_port_flags |= DELAYED_ACK; /* NOTE: must be done  */
                                                    /* after resetting timer   */
        }

    }       /* end of in.size >= dlen */

    /* **************************************************   */
    /* if we do not have room for the data in input window  */
    else
    {                                      
        port->interp_port_flags |= (SEND_PKT | SEND_ACK);     /* force re-transmit of ACK */
        DEBUG_ERROR("check_data - NO room, dlen,in.contain = ", EBS_INT2,  
            dlen, port->in.contain);
        DEBUG_ERROR("check_data - NO room, in.ad_size,in.size = ", EBS_INT2,  
            port->in.ad_size, port->in.size);
    }

    return(TRUE);
}

/* ********************************************************************   */
/* listen_proc() - process incoming TCP packet for LISTEN state           */
/*                                                                        */
/*   Called from LISTEN state to process an incoming TCP packet.          */
/*   If the ACK flag is set, a reset is sent.  If the SYNC                */
/*   flag is set, the handshaking has started.                            */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void listen_proc(PIFACE pi, PTCPPORT port, DCU msg)   /*__fn__*/
{
PTCPPKT pt;
PIPPKT  pip;
PTCPPORT mport;

    /* NOTE: LISTEN ignores RESETS (RFC 793 pg 37)   */

    /* **************************************************                */
    /* listen_proc() : perform the following                             */
    /*  if get sync                                                      */
    /*          - go to SYNR                                             */
    /*          - move port from listen to active list                   */
    /*          - if port_type is SLAVE                                  */
    /*                   if master port_type is 0 (backlog size reached) */
    /*                      set master's port_type to MASTER_NO_LISTENS  */
    /*                   else                                            */
    /*              tc_tcp_listen(port_type)                             */
    /* **************************************************                */
    pt  = DCUTOTCPPKT(msg);
    pip = DCUTOIPPKT(msg);

    DEBUG_LOG("  listen_proc:tcp_flags = ", LEVEL_3, EBS_INT1, pt->tcp_flags, 0);

    /* ******                                       */
    /* if there is an ACK, reset (RFC 793 pg 36,65) */
    /* i.e. when in LISTEN should not get an ACK    */
    if (pt->tcp_flags & TCP_F_ACK)
    {                                               
        DEBUG_ERROR("listen_proc() - send reset", NOVAR, 0, 0);
        tc_tcp_reset(pi, port, msg); /* if it is a reset then tc_tcp_reset */
                                     /* will not actually send the reset   */
        return;
    }

     /* ******   */
    if (pt->tcp_flags & TCP_F_SYN)   /* receive SYN  */
    {    
        /* remember anything important from the incoming TCP header    */
        DEBUG_LOG("listen_proc - in seq = ", LEVEL_3, 
            DINT1, WARRAY_2_LONG(pt->tcp_seq), 0);
        port->out.size = port->out.ad_size = port->out.max_size = 
            pt->tcp_window;  
        port->out.port = net2hs(pt->tcp_source);    /* remote port */
        port->in.nxt = WARRAY_2_LONG(pt->tcp_seq)+1;       

        /* set the necessary fields in the outgoing TCP packet   */
        port->out_template.tcp_dest = pt->tcp_source;

        /* Copy the client's IP address into outgoing pkt's dest addr   */
        /* NOTE: if client sent LOOP BACK send a LOOP BACK back         */
        /*       i.e. send it back to itself                            */
        /* NOTE: if it is not LOOP BACK, use clients dest address as    */
        /*       the source address to send packets to                  */
        if (pip->ip_dest[0] == IP_LOOP_ADDR)
            port->out_ip_temp.ip_dest[0] = IP_LOOP_ADDR;
        else
            tc_mv4(port->out_ip_temp.ip_dest, pip->ip_src, 4);

        /* set TCP state to syn received   */
        port->from_state = TCP_S_LISTEN;  /* save in case reset while in */
                                          /*    SYNR state   */
        trans_state(port, TCP_S_SYNR, FROM_INTERPRET, FALSE);
        INCR_SNMP(TcpPassiveOpens)
        DEBUG_LOG("  listen_proc() - LISTEN->SYNR    ", LEVEL_3, NOVAR, 0, 0);

        /* move port from listen list to active list   */
        delete_tcpport_list(port);
        add_tcpport_list(port, ACTIVE_LIST);

        /* if port is a slave create a new listener if number of in progress       */
        /* connections has not been reached, if it has save that info so xn_accept */
        /* can create a new one when it takes a slave port off master's            */
        /* list                                                                    */
        /* NOTE: this slave ports stays on master's list until port                */
        /*       is accepted                                                       */
        /* NOTE: if tc_tcp_listen fails there is nothing we can do at              */
        /*       this point and since we are not at an api call there              */
        /*       is no error we can return but any incoming connection             */
        /*       requests will find an unknown port and will send a reset          */
        /*       back                                                              */
        /* also : tc_tcp_listen will set errno which we don't want to do           */
        /* also : if tc_tcp_listen fails due to no more ports could:               */
        /*        - set flag in master port                                        */
        /*        - link master port into a new list (root_master_list)            */
        /*        - if no ports found try searching root_master_list -             */
        /*          if match found, try allocating a new port at that              */
        /*          time, if succeeds                                              */
        /*                   - accept msg + put new port on appropriate list       */
        /*                   - take master of root_master_list and clear flag      */
        /*                if fails                                                 */
        /*                   - reject msg and send reset                           */
        if (port->tcp_port_type == SLAVE_PORT) 
        {
            /* insert a new listener on list if backlog size has not been   */
            /* reached; if it has then set type so can try again after      */
            /* slave port is accept() and backlog size will not be at       */
            /* its limit                                                    */
            /* NOTE: there will always be a master port but just to be      */
            /*       safe it is checked                                     */
            mport = port->master_port;
            if (mport)
            {
                if (mport->tcp_port_type == 0)
                {
                    mport->tcp_port_type = MASTER_NO_LISTENS;
                    DEBUG_LOG("proc_listen - set port type to MASTER_NO_LISTEN",
                        LEVEL_3, NOVAR, 0, 0);

#                    if (DEBUG_WEB)
                        listen_sock++;
                        DEBUG_ERROR("listen_proc - set to MASTER_NO_LISTENS",
                            NOVAR, 0, 0);
#                    endif
                }
                else
                {
                    DEBUG_LOG("listen_proc - call tc_tcp_listen()", LEVEL_3,
                        EBS_INT1, mport->tcp_port_type, 0);
                    if (tc_tcp_listen(mport, mport->tcp_port_type, TRUE))
                    {
                        DEBUG_ERROR("listen_proc - tc_tcp_listen() failed - errno = ",
                            EBS_INT1, xn_getlasterror(), 0);
                        /* move from socket list to master list with no backlog   */
                        /* listerner list; then when sockets are freed another    */
                        /* attempt will be made to allocate a backlog listener    */
                        delete_tcpport_list(mport);
                        add_tcpport_list(mport, MASTER_LIST);
#if (DEBUG_WEB)
                        listen_fail++;
#endif
                    }
#if (DEBUG_WEB)
                    else
                    {
                        DEBUG_LOG("listen_proc - after call tc_tcp_listen()", LEVEL_3,
                            EBS_INT1, mport->tcp_port_type, 0);
                    }
#endif
                }
            }
            else
            {
                DEBUG_ERROR("OOPS! - listen_proc - no master port", NOVAR, 0, 0);
            }
        }
        else
        {
            DEBUG_ERROR("OOPS! - listen_proc - type is not SLAVE", NOVAR, 0, 0);
        }

        /* send sync and ack msg to other side    */
        /* NOTE: do not wake up yet since not EST */
        port->interp_port_flags |= (SEND_PKT | SEND_ACK | SEND_SYN);  
    }
    else
    {
        DEBUG_LOG("listen_proc - no SYNC", LEVEL_3, NOVAR, 0, 0);
    }
}


/* ********************************************************************   */
/* syns_proc() - process incoming TCP packet for SYNS state               */
/*                                                                        */
void syns_proc(PIFACE pi, PTCPPORT port, DCU msg,                /*__fn__*/
               word dlen, word hlen, RTIP_BOOLEAN KS_FAR *queued)   /*__fn__*/
{
PTCPPKT pt;

    /* ******   */
    pt = DCUTOTCPPKT(msg);

    /* ******                                                   */
    /* NOTE: application did a connect() to get into SYNS state */

    /* ******                                                         */
    /* check to see if it ACKS correctly i.e. ensure ACK specifies    */
    /* correct seq number in response to our SYNC msg                 */
    /* NOTE: remember that pkt_template is pre-set-up                 */
    /* NOTE: if otherside never acks our SYN correctly connect() will */
    /*       timeout                                                  */
    if ( (pt->tcp_flags & TCP_F_ACK) && 
         (WARRAY_2_LONG(pt->tcp_ack) != (port->out.nxt+1)) )
    {
        DEBUG_ERROR("syns_proc(): acked incorrectly-send reset", NOVAR, 0, 0);
        DEBUG_ERROR("pt->tcp_ack, port->out.nxt = ", DINT2, 
           WARRAY_2_LONG(pt->tcp_ack), port->out.nxt);
        tc_tcp_reset(pi, port, msg); /* if it is a reset then tc_tcp_reset */
                                     /* will not actually send the reset   */
        return;
    }

    /* ******                                                      */
    /* process a reset (it was acked correctly due to above check) */
    /* NOTE: if got reset which was acked incorrectly will         */
    /*       ignore the reset by processing above;                 */
    /*       this could happen if when sync                        */
    /*       was set otherside wasn't listening yet and sent       */
    /*       us a reset; when the otherside starts listening       */
    /*       and the timeout retries the connection will be        */
    /*       established                                           */
    if (pt->tcp_flags & TCP_F_RESET)   /* NOTE: it was acked correctly */
                                       /*       (see above check)   */
    {
        if (pt->tcp_flags & TCP_F_ACK) 
        {
            /* signal reader and writers with error; signal user         */
            /* by closing port; do not free port until application       */
            /* calls close                                               */
            /* NOTE: let tc_tcp_interpret() do signalling                */
            /*       instead of tc_tcp_abort_connection in case want to  */
            /*       signal error for any other reason then won't        */
            /*       signal twice although it shouldn't matter           */
            /* NOTE: application did a connect() to get into SYNS state  */
            /* NOTE: state is sync sent, therefore, we know we did not   */
            /*       origionate in the LISTEN state and thus definately  */
            /*       do not have to return there                         */
            /* NOTE: wake up connect()                                   */
            DEBUG_LOG("  syns_proc() - SYNS->CLOSED", LEVEL_3, NOVAR, 0, 0);
            tc_tcp_abort_connection(port, 
                WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, ECONNREFUSED,
                FROM_INTERPRET);
#if (DEBUG_ABORT_CONNECTION)
            DEBUG_ERROR("syns_proc: abort connection", NOVAR, 0, 0);
#endif
        }
        return;  /* if reset, (ack or not) no further processing; */
                /* i.e. reset with no ack gets dropped   */
    }

    /* ******                                         */
    /* if here, (ACK,ack is ok or no ACK) and no RST; */

    /* if receive a SYNC   */
    if (pt->tcp_flags & TCP_F_SYN)
    {        
        DEBUG_LOG("  syns_proc() - SYNS - received SYNC", LEVEL_3, NOVAR, 0, 0);
        port->in.nxt = WARRAY_2_LONG(pt->tcp_seq) + 1;
        port->out.size = port->out.ad_size = port->out.max_size = 
            pt->tcp_window; 
        port->interp_port_flags |= (SEND_PKT | SEND_ACK);     /* ensure sends ACK */

        /* if got an ACK with the SYNC go to EST state         */
        /* NOTE: from above check, if ACK there ack is correct */
        if (pt->tcp_flags & TCP_F_ACK)
        {     
            DEBUG_LOG("  syns_proc() - ack rcvd - SYNS->EST", LEVEL_3, NOVAR, 0, 0);
            ACCOUNT_ONE_BYTE(port);        /* account for my SYN */
            port->out.max_nxt_to_send = port->out.nxt_to_send;
            trans_state_est(port);

            /* process any data                                             */
            /* NOTE: at this point there cannot be any out of order packets */
            /*       which need processing; if an out of order packet comes */
            /*       in before the SYNC, it is tossed since without the     */
            /*       initial SYNC bit, verifying the sequence number        */
            /*       of a packet is impossible                              */
            /* NOTE: this packet is cannot be out of order since it has the */
            /*       SYNC bit set                                           */
            check_data(port, msg, pt, dlen, hlen, queued, FALSE);
            DEBUG_LOG("syns_proc - back from check_data", LEVEL_3, NOVAR, 0, 0);
        }

        /* if got an ACK with no SYNC go to SYNR state   */
        else  
        {
            DEBUG_LOG("  syns_proc() - ack not rcvd - SYNS->SYNR", LEVEL_3, NOVAR, 0, 0);
            port->from_state = TCP_S_SYNS;  /* i.e. SYNS (save in  */
                                            /* case reset during   */
                                            /* SYNR state          */
            trans_state(port, TCP_S_SYNR, FROM_INTERPRET, FALSE);    
                                            /* syn received            */
                                             /* don't wakeup yet since */
                                             /* not est                */
            port->interp_port_flags |= (SEND_PKT | SEND_SYN | SEND_ACK);   /* send SYNC and ACK */

            /* tbd - when handle out of order packets if there     */
            /*       is any controls or data in this packet it     */
            /*       should be handle when transition to EST state */
            /*       (see pg 68 of RFC 793)                        */
        }
    }
            
    /* NOTE: do nothing for ACK,ack with no SYNC according       */
    /*       to state transition rules (see pg 68 of RFC 793 and */
    /*       state transition diagrams)                          */

    /* NOTE: no processing is done for FINs since the seq number          */
    /*       cannot be validated, therefore, the segment should           */
    /*       be dropped but since this is the last check dropping         */
    /*       the segment is the same as doing nothing (see RFC 793 pg 75) */

}

/* ********************************************************************      */
/* process_one_tcp_output() - send any output packets associated with a port */
/*                                                                           */
/*   Processes any outstanding output for one TCP port.                      */
/*   This routine is called after all input packets have been                */
/*   processed thus minimizing the number of output packets which will be    */
/*   sent, i.e. if two input packets for the same port are queued and both   */
/*   trigger an output packet, only 1 output packet will be sent             */
/*                                                                           */
/*   Returns nothing                                                         */

void process_one_tcp_output(PTCPPORT port)  /*__fn__*/
{
byte     tcp_flags;
int      type;

#if (DEBUG_DACK)
        DEBUG_ERROR("process_one_tcp_output - interpret_port_flags,before = ", 
            EBS_INT1, port->interp_port_flags, 0);
#endif
    /* see if we have to process flags   */
    if (port->interp_port_flags & 
        (SEND_PKT | SEND_DATA_PKT | SEND_ONE_SEG_DATA))
    {
        /* If we have a send request send the msg with port's data   */
        tcp_flags = NO_TCP_FLAGS;
        if (port->interp_port_flags & SEND_SYN)
            tcp_flags = TCP_F_SYN;

        if (port->interp_port_flags & SEND_FIN)
            tcp_flags |= (TCP_F_FIN | TCP_F_ACK);   /* FIN must always have ACK */

        if (port->interp_port_flags & SEND_ACK)
            tcp_flags |= TCP_F_ACK;

        if (port->interp_port_flags & SEND_PKT)
        {
            type = NORMAL_SEND;
            if (port->interp_port_flags & SEND_DATA_PKT)
                type = NORMAL_DATA;

            tc_tcpsend(port->ap.iface, port, NO_DCU_FLAGS, type, tcp_flags);
        }

        else if (port->interp_port_flags & SEND_DATA_PKT)
        {
            tc_tcpsend(port->ap.iface, port, NO_DCU_FLAGS, NORMAL_DATA_ONLY, 
                    tcp_flags);
        }

        else if (port->interp_port_flags & SEND_ONE_SEG_DATA)
        {
            tc_tcpsend(port->ap.iface, port, NO_DCU_FLAGS, FAST_RETRANS, 
                    tcp_flags);
        }

    }
    /* turn off any flags processed   */
    port->interp_port_flags &= ~(SEND_PKT | SEND_DATA_PKT | SEND_SYN | 
                                 SEND_FIN | SEND_ACK | SEND_ONE_SEG_DATA);
}

#if (TCP_WAIT_SEND)
/* ********************************************************************       */
/* tc_tcp_output() - send any output packets associated with ports            */
/*                                                                            */
/*   Loops through the ports in the active list and processes any outstanding */
/*   output.  This routine is called after all input packets have been        */
/*   processed thus minimizing the number of output packets which will be     */
/*   sent, i.e. if two input packets for the same port are queued and both    */
/*   trigger an output packet, only 1 output packet will be sent              */
/*                                                                            */
/*   Returns nothing                                                          */

void tc_tcp_output(void)        /*__fn__*/
{
PTCPPORT port;
PTCPPORT prev_port;

    OS_CLAIM_TCP(OUT_CLAIM_TCP)

    /* only need to process the active list since if a message was received   */
    /* in the listen state, it will either send a reset, ignore it, or if     */
    /* it is a valid sync will move the port to the active list               */
    port = (PTCPPORT)(root_tcp_lists[ACTIVE_LIST]);
    while (port)
    {
        process_one_tcp_output(port);

#if (1)
        /* ******   */
        prev_port = port;
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[ACTIVE_LIST], 
                                                (POS_LIST)port, ZERO_OFFSET);

        /* ******                                                         */
        /* NOTE: ok to free port since only on list with forward pointers */
        /*       (i.e. port does not point back to prev_port)             */
        if (prev_port->state == TCP_S_CLOSED)
            tcp_close(prev_port);                   /* free the resources */
#else
        if (port->state == TCP_S_CLOSED)
        {
            tcp_close(port);                    /* free the resources */
            port = (PTCPPORT)(root_tcp_lists[ACTIVE_LIST]);
            continue;
        }

        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[ACTIVE_LIST], 
                                                (POS_LIST)port, ZERO_OFFSET);
#endif
    }

    OS_RELEASE_TCP()
}
#endif

/* ********************************************************************     */
/* tc_find_port() - find port incoming TCP packet is intended for           */
/*                                                                          */
/*   Loops thru list of active ports and list of listener ports to find     */
/*   a socket which the incoming packet, pt, is intended for.               */
/*   A port on the active list matches if both IP and port numbers match.   */
/*   A port on listen list matches if my port number matches packets dest   */
/*   port, it is a sync message and the port is not connected to any other  */
/*   port.                                                                  */
/*                                                                          */
/*   Returns port matched or 0 if did not match any.                        */
/*                                                                          */

PTCPPORT tc_find_port(DCU msg)     /*__fn__*/
{
PTCPPKT  pt;
PIPPKT   pip;
word     myport,hisport;      /* ports extracted from msg received */
PTCPPORT port;
RTIP_BOOLEAN  match;

    pt = DCUTOTCPPKT(msg);
    pip = DCUTOIPPKT(msg);

    myport = net2hs(pt->tcp_dest);
    hisport = net2hs(pt->tcp_source);

    /* **************************************************   */
    /* first look for connected port                        */
    port = (PTCPPORT)(root_tcp_lists[ACTIVE_LIST]);
    while (port)
    {
        DEBUG_LOG("  index = ", LEVEL_3, EBS_INT1, port->ap.ctrl.index, 0);
        DEBUG_LOG("    active-port.in = ", LEVEL_3, EBS_INT1, port->in.port, 0);
        DEBUG_LOG("    active-port.out = ", LEVEL_3, EBS_INT1, port->out.port, 0);

        /* If HIS destination port(from msg rcvd) equals ME (set by xn_bind or    */
        /* xn_socket)                                                             */
        if ( (myport == port->in.port) && (hisport == port->out.port) )
        {
            DEBUG_LOG("  pkt_template ip_dest", LEVEL_3, IPADDR, port->out_ip_temp.ip_dest, 0);
            DEBUG_LOG("  pkt_template ip_src", LEVEL_3, IPADDR, port->out_ip_temp.ip_src, 0);

            /* Found a connection if IP addresses match                          */
            /* NOTE: ip_src set up by bind; ip_dest set up by connect or         */
            /*       listen_proc                                                 */
            /* NOTE: found match if our dest ip matches sender's IP from msg and */
            /*       (our source match dest in pkt or bound to any address)      */

            /* if sending to LOOP BACK, it matches but only if   */
            /* previously connected to LOOP BACK                 */
            match = FALSE;
            if ( (pip->ip_dest[0] == IP_LOOP_ADDR) &&
                 (port->out_ip_temp.ip_dest[0] == IP_LOOP_ADDR) )
                match = TRUE;

            if ( tc_cmp4(port->out_ip_temp.ip_dest, pip->ip_src, 4) && 
                 (tc_cmp4(port->out_ip_temp.ip_src, pip->ip_dest, 4) ||
                  port->ap.port_flags & PORT_WILD) )
                match = TRUE;

            if (match)
            {
                /* if bound to wild save find out who we are from the     */
                /* packet, i.e. our IP address can vary depending upon    */
                /* interface came in on so we want to set our address in  */
                /* outgoing packet based upon who sender says we are      */
                /* NOTE: address are saved in the template since flag set */
                /*       by bind is used to determine if bound to wild    */
                /*       IP address                                       */
                /* NOTE: if still no src IP then must have bound to wild  */
                /*       then done connect                                */
                if ( (port->ap.port_flags & PORT_WILD) &&
                     (tc_cmp4(port->out_ip_temp.ip_src, ip_nulladdr, 4)) )
                {
                    DEBUG_LOG("find_tcp_port - conn found, wild, no src ip", LEVEL_3, NOVAR, 0, 0);

                    /* ip address   */
                    tc_mv4(port->out_ip_temp.ip_src, pip->ip_dest, 4);
                }
                DEBUG_LOG("  connection found, index =", LEVEL_3, EBS_INT1, 
                        port->ap.ctrl.index, 0);
                break;
            }
        }
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[ACTIVE_LIST], 
                                                (POS_LIST)port, ZERO_OFFSET);
    }

    /* **************************************************   */
    /* if did not find active port, look for listener port  */
    if (!port)
    {
        /* loop at listener ports   */
        port = (PTCPPORT)(root_tcp_lists[LISTEN_LIST]);
        while (port)
        {
            DEBUG_LOG("  listen-port.in, my = ", LEVEL_3, EBS_INT2, 
                port->in.port, myport);
            DEBUG_LOG("  listen-port.out = ", LEVEL_3, EBS_INT1, port->out.port, 0);
            DEBUG_LOG("  listen-ip - template", LEVEL_3, IPADDR, 
                port->out_ip_temp.ip_src, 0);
            DEBUG_LOG("  pkt_template ip_dest", LEVEL_3, IPADDR, port->out_ip_temp.ip_dest, 0);
            DEBUG_LOG("  pkt_template ip_src", LEVEL_3, IPADDR, port->out_ip_temp.ip_src, 0);

            /* check if listener i.e.                                               */
            /* If HIS destination port(from msg rcvd) equals ME (set by xn_bind or  */
            /* xn_socket) and HIS ip destination (from msg rcvd) equals ME          */
            /* NOTE: my IP address could be wild                                    */
            if ( (myport == port->in.port) &&
                    (tc_cmp4(port->out_ip_temp.ip_src, pip->ip_dest, 4) ||
                    (pip->ip_dest[0] == IP_LOOP_ADDR) ||
                    port->ap.port_flags & PORT_WILD) )
            {
                    /* See if we have a listener on this port   */
                if (!port->out.port && (pt->tcp_flags & TCP_F_SYN))
                {
                    DEBUG_LOG("  listener port found(hex), port,index = ", LEVEL_3, EBS_INT2,  
                                myport, port->ap.ctrl.index);

                    /* if bound to wild save find out who we are from the     */
                    /* packet, i.e. our IP address can vary depending upon    */
                    /* interface came in on so we want to set our address in  */
                    /* outgoing packet based upon who sender says we are      */
                    /* NOTE: address are saved in the template since flag set */
                    /*       by bind is used to determine if bound to wild    */
                    /*       IP address                                       */
                    if (port->ap.port_flags & PORT_WILD) 
                    {
                        /* ip address   */
                        tc_mv4(port->out_ip_temp.ip_src, pip->ip_dest, 4);
                    }

                    /* initialize TCP connection passing address of remote host;   */
                    /* if unsuccessful, try next port                              */
                    if (!tcpinit(port, pip->ip_src))
                    {
                        port = (PTCPPORT)
                            os_list_next_entry_off(root_tcp_lists[LISTEN_LIST], 
                                                    (POS_LIST)port, ZERO_OFFSET);
                        continue;
                    }

                    break;     /* get first listener (since can have multiple listens */
                               /* for the same port want the parent first)   */
                }
            }
            port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[LISTEN_LIST], 
                                                    (POS_LIST)port, ZERO_OFFSET);
        }
    }
#if (INCLUDE_TCP_PORT_CACHE)
    /* If tcp cacheing is turned on save the address of the port structure   */
    /* of the port we just found. This will make tcp faster when more        */
    /* packets arrive                                                        */
    else
        cached_tcp_port = port;
#endif
    
#if (DISPLAY_NO_MATCH)
    if (!port)
    {
        DEBUG_ERROR("tcp_interpret-no match or listener found, send reset", NOVAR, 0, 0);
        DEBUG_ERROR("  in msg: dest port (myport) = ", EBS_INT1, 
                    net2hs(pt->tcp_dest), 0);
        DEBUG_ERROR("          src port (hisport) = ", EBS_INT1, 
                    net2hs(pt->tcp_source), 0);
        DEBUG_ERROR("  pt ip_dest: ", IPADDR, pip->ip_dest, 0);
        DEBUG_ERROR("  pt ip_src: ", IPADDR, pip->ip_src, 0);
        DEBUG_ERROR("  pt tcp_flags", EBS_INT1, pt->tcp_flags, 0);
#if (DISPLAY_NO_MATCH_PORTS)
        DEBUG_ERROR("interpret: PORT SUMMARY = ", PORTS_TCP, 0, 0); 
#endif
    }
#endif

    return(port);
}

/* ********************************************************************   */
/* tcp_signal_task() - signal (READ or WRITE) a task                      */
/*                                                                        */
/*    Signals a task waiting for a tcp event such as data in input        */
/*    window or room in output window to queue data.  The signal          */
/*    needs to be sent as a result of processing a TCP input packet.      */
/*    The parameter enter_state is set to the TCP state before the        */
/*    incoming packet was processed.                                      */
/*    Flags are set in port->interp_port_flags when a signal needs        */
/*    to be set during processing of the incoming packet.                 */
/*                                                                        */
void tcp_signal_task(PTCPPORT port, int enter_state)        /*__fn__*/
{
PTCPPORT mport;

    /* if time to wakeup task waiting for event (for example        */
    /*    listen or connect waiting for established state or        */
    /*    read waiting for data available) then signal the task     */
    /* NOTE: use wakeup flag instead of calling directly to protect */
    /*       sending the same signal more than once                 */
    if ( (port->interp_port_flags & READ_WAKEUP_SUCCESS) || 
         (port->interp_port_flags & READ_WAKEUP_FAIL) )
    {
        /* Signal the caller here - i.e. read                         */
        /* NOTE: if we are signalling a slave port                    */
        /*       (signal due to an error or entering                  */
        /*       EST state) then we are signalling xn_accept which    */
        /*       does not know which slave port will be signaled;     */
        /*       therefore, signal the master port and xn_accept will */
        /*       look at all the master's slaves to see which one     */
        /*       entered EST state (NOTE: then it will take it        */
        /*       off the master's slave list)                         */
        if ( (port->tcp_port_type == SLAVE_PORT) &&
             ((enter_state == TCP_S_LISTEN) ||
              (enter_state == TCP_S_SYNR) ||
              (enter_state == TCP_S_SYNS)) )
        {
            mport = port->master_port;
            mport->ap.ctrl.rtip_errno = port->ap.ctrl.rtip_errno;
            mport->interp_port_flags  = port->interp_port_flags;

            DEBUG_LOG("  signaled master port - index = ", LEVEL_3, EBS_INT1, 
                port->master_port->ap.ctrl.index, 0);

            port = mport;
        }
    }

    /* signal any API tasks waiting on select/read/write processing   */
    /* this packet determined it should be done;                      */
    /* we are in interpret but set parameter to NOT_FROM_INTERPRET so */
    /* wake_up will send the signal instead of just setting flag      */
    /* to signal later (when interpret is done) now that interpret is */
    /* nearly done                                                    */
    wake_up(port, NOT_FROM_INTERPRET, port->interp_port_flags,
            port->ap.ctrl.rtip_errno);
}

/* ********************************************************************       */
/*  tc_tcp_interpret() - interpret incoming TCP packet                        */
/*                                                                            */
/*   Called by tc_ip_interpret to process an incoming TCP packet.  Perfors    */
/*   the following:                                                           */
/*      - finds the socket which the packet is intended for                   */
/*      - calls tc_tcpdo to process the packet                                */
/*      - signals any tasks which need to be woken up                         */
/*      - sends a response to the remote host (could be just a acknowledgment */
/*        or response could include data if the remote host sent an updated   */
/*        window size)                                                        */
/*                                                                            */
void tc_tcp_interpret(PIFACE pi, DCU msg)                      /*__fn__*/
{
PTCPPKT  pt;              /* packet received */
PTCPPORT port;            /* used to loop thru active list */
int      enter_state;

    /* **************************************************   */
    /* **************************************************   */
#    if (DEBUG_WEB)
        DEBUG_ERROR("interpret: list fails, MASTER NO LISTEN  = ", EBS_INT2, listen_fail, listen_sock);
        DEBUG_ERROR("interpret: sockets return to listen, web count = ", EBS_INT2, return_listen, web_count);
        DEBUG_LOG("PORTS", LEVEL_3, PORTS_TCP, 0, 0);
#    endif

    /* **************************************************   */
    UPDATE_INFO(pi, rcv_tcp_packets, 1)
    INCR_SNMP(TcpInSegs)

    /* **************************************************   */
    /* Get the packet from the message and perform checksum */
    pt  = DCUTOTCPPKT(msg);  

    DEBUG_LOG("tc_tcp_interpret - seq # = ", LEVEL_3, DINT1,
        net2hl(WARRAY_2_LONG(pt->tcp_seq)), 0);

    /* **************************************************              */
    /* if the computed checksum does not match one in msg then error   */
    /* NOTE: tc_tcp_chksum will return 0 if no error since checksum is */
    /*       calculated such that if checksum is in the message the    */
    /*       checksum will be 0                                        */
    if (tc_tcp_chksum(msg))
    {
        DEBUG_ERROR("TCP chksum err - calc chksum, msg chksum = ", 
            EBS_INT2, tc_tcp_chksum(msg), pt->tcp_chk);
        DEBUG_ERROR("      pkt len = ", EBS_INT1, IP_LEN_DCU(msg), 0);
        DEBUG_LOG("PACKET = ", LEVEL_3, PKT, (PFBYTE)DCUTODATA(msg), 
            IP_LEN_DCU(msg)+ETH_HLEN_BYTES);
/*        DEBUG_ERROR("PACKET = ", PKT, (PFBYTE)DCUTODATA(msg),    */
/*          IP_LEN_DCU(msg)+ETH_HLEN_BYTES);                       */

        tc_icmp_send(msg, ICMP_T_PARAM_PROBLEM, ICMP_C_PTR_VALID, 
                     ICMP_P_TCPCHKSUM);
        UPDATE_INFO(pi, tcp_cksum, 1)
        INCR_SNMP(TcpInErrs)

        /* Discard   */
        os_free_packet(msg);        /* Free the input message */
        return;
    }

    /* **************************************************                 */
    /* change fields in packet to host byte order                         */
    /* NOTE: tcp_seq and tcp_ack are 2 byte arrays of words to avoid some */
    /*       architectures which long align longs                         */
    LONG_2_WARRAY(pt->tcp_seq, net2hl(WARRAY_2_LONG(pt->tcp_seq)));
    LONG_2_WARRAY(pt->tcp_ack, net2hl(WARRAY_2_LONG(pt->tcp_ack)));
    pt->tcp_window = net2hs(pt->tcp_window);

    /* **************************************************   */
    /* Scan the list of active tcp ports on the interface   */
    OS_CLAIM_TCP(INT_CLAIM_TCP)     /* Exclusive access to tcp resources */
                         /* NOTE: needs to be claim for whole interpret not          */
                         /*       only since scanning list but for values            */
                         /*       within port structure such as state information    */
                         /*       needs to be protected from things like closesocket */
                         /*       until msg is fully processed                       */
#if (INCLUDE_TCP_PORT_CACHE)
    /* First check the last active socket we received to.    */
   if ( 
#if (INCLUDE_MEASURE_PERFORMANCE)
        do_sock_tcp_cache &&
#endif
         cached_tcp_port &&
         (net2hs(pt->tcp_dest) == cached_tcp_port->in.port) && 
         (net2hs(pt->tcp_source) == cached_tcp_port->out.port) &&
         tc_cmp4(cached_tcp_port->out_ip_temp.ip_dest, (DCUTOIPPKT(msg))->ip_src, 4) && 
         tc_cmp4(cached_tcp_port->out_ip_temp.ip_src, (DCUTOIPPKT(msg))->ip_dest, 4) )
        port = cached_tcp_port;
    else
#endif
        port = tc_find_port(msg);

    if (!port)  /* did not find match or listener */
    {
        /* We don't have a socket for this connection. Use the packet    */
        /* to send a reset back to the caller                            */
        UPDATE_INFO(pi, tcp_dropped, 1)
        tc_tcp_reset(pi, (PTCPPORT)0, msg);  /* NOTE: port is only needed for tc_ */
                                             /*       netwrite which will   */
                                             /*       work with no port     */
            
        os_free_packet(msg);    /* since will not call tc_tcpdo,  */
                                /* free incoming DCU now   */
    }
    else      /* found connection or listener */
    {
        DEBUG_LOG("tc_tcp_interpret - state = ", LEVEL_3, EBS_INT1, port->state, 0);
        enter_state = port->state;
        port->intime = 0; /* keep track of last time heard from other */
                          /* side for keepalive   */

        /* turn off flags which might be set by tc_tcpdo and processed when   */
        /* returned                                                           */
        /* NOTE: DELAYED_ACK, SEND_xxx is not cleared                         */
        port->interp_port_flags &= ~(READ_WAKEUP_SUCCESS | READ_WAKEUP_FAIL |
                                     WRITE_WAKEUP_SUCCESS | WRITE_WAKEUP_FAIL |
                                     SELECT_WRITE_WAKEUP);
        port->ap.ctrl.rtip_errno = 0;

        /* ******             */
        /* PROCESS THE PACKET */
        tc_tcpdo(pi, port, msg);

        /* ******                                                          */
        /* SIGNAL TASKS if necessary; an API task could be blocked waiting */
        /* for data to arrive (recv or select), or an API task could       */
        /* be blocked waiting on room in output window, etc                */
        tcp_signal_task(port, enter_state);

        /* ******                                      */
        /* send TCP packet to remote host if necessary */
#if (!TCP_WAIT_SEND)
        process_one_tcp_output(port); 
        DEBUG_LOG("tc_tcp_interpret - back from process_one_tcp_output - out.contain = ", 
            LEVEL_3, EBS_INT1, port->out.contain, 0);
#endif
    }
    OS_RELEASE_TCP()      /* Exclusive access to tcp resources */
#if (DEBUG_WEB)
    DEBUG_ERROR("interpret done", NOVAR, 0, 0);
#endif
}

/* ********************************************************************   */
/* calc_tcp_len() - calculate the packets header and total lengths        */
/*                                                                        */
/*    Calculate header length based up header length in tcp header and    */
/*    calculates total data length based up total length specified in     */
/*    the IP header and header length.  If the packet is fragmented,      */
/*    calls ipf_tcp_tlen to loop through all the fragments to sum up      */
/*    the total data length.                                              */
/*    If setup is TRUE it also sets up the data_start and data_len        */
/*    entries in the DCU structure.                                       */
/*                                                                        */
/*    Returns nothing                                                     */

static void calc_tcp_len(DCU msg, word *hlen, word *dlen,  /*__fn__*/
                         RTIP_BOOLEAN setup)                                  /*__fn__*/
{
PTCPPKT pt;
word tlen;
#if (INCLUDE_FRAG)
int  hdr;
#endif

    pt  = DCUTOTCPPKT(msg);

    /* Tcp header length in bytes           */
    /* i.e. ((x&0xf0)>>4)<<2); i.e.         */
    /*   - mask out lower 4 bits (reserved) */
    /*   - shift upper 4 bits to lower bits */
    /*   - convert # words to # bytes       */
    /* which is equivalent to ((x&0xf0)>>2) */
    *hlen = (word) ((pt->tcp_hlen & 0xf0)>>2);  

    /* Get total tcp packet length excluding IP section    */
#if (INCLUDE_FRAG)
    if (DCUTOPACKET(msg)->frag_next)    /* if packet is fragmented */
    {
        if (!setup)
            hdr = -1;                   /* tell ipf_tcp_tlen not to setup */
                                        /* data_start and data_len   */
        else
            hdr = *hlen;
        tlen = ipf_tcp_tlen(msg, hdr);      /* also sets data_start and  */
                                            /* data_len for each packet   */
                                            /* based upon setup param     */
    }
    else
    {
#endif
        /* packet is not fragmented, so the total length can be calculated   */
        /* from the first packet                                             */
        tlen = IP_LEN_DCU(msg);
        tlen = (word)(tlen - IP_HLEN_DCU(msg));  /* sub # bytes in header */

        /* start of data is past header and options; hlen was also adjusted    */
        /* for any duplicate data                                              */
        if (setup)
        {
            DCUTOPACKET(msg)->data_start = *hlen;
            DCUTOPACKET(msg)->data_len = DCUTOPACKET(msg)->frag_data_len = 
                tlen - *hlen;
        }
#if (INCLUDE_FRAG)
    }       /* end of if fragmented else */
#endif

    /* length in the data area of the TCP packet   */
    if (*hlen > tlen)
        *dlen = 0;
    else
        *dlen = (word) (tlen-*hlen);
}

/* ********************************************************************   */
/* tc_tcpdo() - do state processing for incoming TCP packet               */
/*                                                                        */
/*   Process the incoming message based upon current TCP state.  This     */
/*   routine might cause state transitions based upon received message.   */
/*                                                                        */
static void tc_tcpdo(PIFACE pi, PTCPPORT port, DCU msg)                /*__fn__*/
{
word hlen;
word dlen;
RTIP_BOOLEAN queued;
#if (INCLUDE_FAST_TCP)
PTCPPKT pt;
#endif

    queued = FALSE;

    /* calculate total len and header length and setup data_start and   */
    /* data_len info in the DCU                                         */
    calc_tcp_len(msg, &hlen, &dlen, TRUE);

    /* **************************************************   */
    /* check for options                                    */
    check_option(pi, port, msg, hlen); 

    /* **************************************************   */
    switch (port->state)
    {
        /* **************************************************   */
        case TCP_S_EST:     
#if (INCLUDE_FAST_TCP)
{
dword sq, want;
dword last_sq, last_want;
#define FAST_FLAGS (TCP_F_ACK | TCP_F_URGENT | TCP_F_TPUSH)

            pt = DCUTOTCPPKT(msg);

            /* if only some of FAST_FLAGS are on then we can process   */
            /* packet with this stripped down "fast" code              */
            if ( (pt->tcp_flags & ~FAST_FLAGS) == 0)
            {
                sq = WARRAY_2_LONG(pt->tcp_seq);
                want = port->in.nxt;
                last_want = want + port->in.window_size - port->in.contain - 1;
    
                last_sq = sq + (dword)dlen - 1;

                /* if data in packet and it is in window with no wrap AND   */
                /*    no new acknowledge AND                                */
                /*    no new window update                                  */
                if ( (dlen > 0) && (sq == want) && (last_sq < last_want) &&
                     (WARRAY_2_LONG(pt->tcp_ack) == port->out.ack) &&
                     (pt->tcp_window == port->out.size) )
                {
#if (INCLUDE_TCP_NO_COPY)
                    if (port->ap.options & SO_TCP_NO_COPY)
                    {
#                    if (DEBUG_FAST_TCP)
                        num_fast_tcp++;
#                    endif

                        /* **************************************************            */
                        /* queue the packet                                              */
                        /* set up in case socket recv called;                            */
                        /* NOTE: handles overlap since hlen and dlen were adjusted above */
                        TOTAL_LL_HLEN_SIZE(DCUTOPACKET(msg)->data_start, 
                                           msg_is_802_2(msg), pi, msg)
                        DCUTOPACKET(msg)->data_start += (hlen + 
                                                         IP_HLEN_DCU(msg));
                        DCUTOPACKET(msg)->data_len = 
                            DCUTOPACKET(msg)->frag_data_len = dlen;
                        enqueue_pkt((PWINDOW)&port->in, msg, dlen);
#                if (INCLUDE_TRK_PKTS)
                        DCUTOPACKET(msg)->ctrl.list_id = IN_IN_WIN_LIST;
#                endif

                        /* **************************************************                   */
                        /* update the ACK field values                                          */
                        /* NOTE: update ack value using dlen before it was adjusted for overlap */
                        DEBUG_LOG("tc_tcpdo - there is room, dlen = ", 
                            LEVEL_3, EBS_INT1, dlen, 0);
                        port->in.nxt = WARRAY_2_LONG(pt->tcp_seq) + dlen;
                        port->in.size -= dlen;           /* real window size  */
                        port->in.ad_size -= dlen;        /* advertised window size  */
                        DEBUG_LOG("tc_tcpdo: set ad_size to, dlen", LEVEL_3, DINT2,
                            port->in.ad_size, dlen);

                        /* **************************************************               */
                        /* wake up readers when queue empty before enqueue and force        */
                        /* sending ACK response                                             */
                        /* NOTE: tc_tcp_read will only wait on signal if no data in window, */
                        /*       so to prevent to many signals being send, only send it     */
                        /*       if need to, i.e. if other side does lots of sends before   */
                        /*       our side does a read could get signal overflow queue       */
                        /* NOTE: check in.contain before calling enqueue                    */
                        port->interp_port_flags |= READ_WAKEUP_SUCCESS;         

                        /* **************************************************                   */
                        /* DELAY ACK : do not send ack right away (wait for delay timeout)      */
                        /* but in stream of full sized segments, need to send ack every second  */
                        /* segment                                                              */
                        /* NOTE: this option has to be enabled in order to delay the acks;      */
                        /*       default is to have the option enabled                          */
                        if ( !(port->ap.options & SO_DELAYED_ACK) ||
                            (CFG_MAX_DELAY_ACK <= 0) ||
                            ((dlen == port->in.mss) && 
                            (port->interp_port_flags & DELAYED_LAST)) )
                        {
                            port->interp_port_flags |= (SEND_PKT | SEND_ACK);             
                            port->interp_port_flags &= ~DELAYED_LAST;
#                            if (DEBUG_DACK)
                                DEBUG_ERROR("tc_tcpdo - delayed last so send", NOVAR, 0, 0);
#                            endif
                        }
                        else
                        {
#                            if (DEBUG_DACK)
                                DEBUG_ERROR("tc_tcpdo - set delayed ack", NOVAR, 0, 0);
#                            endif
                            if (dlen == port->in.mss) 
                            {
                                port->interp_port_flags |= DELAYED_LAST;
#                                if (DEBUG_DACK)
                                    DEBUG_ERROR("tc_tcpdo - set delayed last", NOVAR, 0, 0);
#                                endif
                            }

                            /* if not already timing a delayed ack then reset the timer   */
                            if ( !(port->interp_port_flags & DELAYED_ACK) )             
                            {
#                                if (DEBUG_DACK)
                                    DEBUG_ERROR("tc_tcpdo - reset timer", NOVAR, 0, 0);
#                                endif
                                port->dactime = 0;  /* NOTE: this is only place need to reset  */
                                                    /*       since time out processing won't    */
                                                    /*       increment unless flag set          */
                            }
                            port->interp_port_flags |= DELAYED_ACK; /* NOTE: must be done  */
                                                                    /* after resetting timer   */
                        }
                        return;             /* we are done with this pkt and */
                                            /* it does not need to be freed   */
                    }
#endif                  /* INCLUDE_TCP_NO_COPY */
                }
            }
#        if (DEBUG_FAST_TCP)
            num_non_fast_tcp++;
#        endif
}
#endif
        case TCP_S_SYNR:
        case TCP_S_FW1:                     /* waiting for ACK of FIN  */
        case TCP_S_FW2:                     /* want FIN  */
        case TCP_S_CWAIT:                   /* FIN received */
        case TCP_S_LAST:                    /* FIN sent waiting for ACK */
        case TCP_S_CLOSING:                 /* want ACK of FIN  */
        case TCP_S_TWAIT:                   /* ack FIN again?  */
#if (INCLUDE_TCP_OUT_OF_ORDER)
            tc_tcpdo_other_ooo(pi, port, msg, dlen, hlen, 
                               (RTIP_BOOLEAN KS_FAR *)&queued);
#else
            tc_tcpdo_other(pi, port, msg, dlen, hlen, 
                           (RTIP_BOOLEAN KS_FAR *)&queued, FALSE);
#endif
            DEBUG_LOG("tc_tcpdo - back from tc_tcpdo_other", LEVEL_3, NOVAR, 0, 0);
            break;

        /* **************************************************   */
        case TCP_S_LISTEN:                  /* passive open */
            DEBUG_LOG("  tc_tcpdo() - LISTEN", LEVEL_3, NOVAR, 0, 0);
                            
            listen_proc(pi, port, msg);

            /* NOTE: no processing is done for FINs since the seq number          */
            /*       cannot be validated, therefore, the segment should           */
            /*       be dropped but since this is the last check dropping         */
            /*       the segment is the same as doing nothing (see RFC 793 pg 75) */

            break;


        /* **************************************************   */
        case TCP_S_SYNS:                            /* Active open */
            DEBUG_LOG("  tc_tcpdo() - SYNS", LEVEL_3, NOVAR, 0, 0);

            syns_proc(pi, port, msg, dlen, hlen, (RTIP_BOOLEAN KS_FAR *)&queued);
            break;
          
        /* **************************************************   */
        case TCP_S_CLOSED:
#if (DISPLAY_NORMAL)
            DEBUG_ERROR("  tc_tcpdo() - CLOSED, send reset", NOVAR, 0, 0);
#endif

            /* closed represents a socked that has been closed due   */
            /* to api request or error, therefore, incoming msgs     */
            /* generates a reset                                     */
            tc_tcp_reset(pi, port, msg); 

            port->in.port = port->out.port = 0;

            /* NOTE: no processing is done for FINs since the seq number          */
            /*       cannot be validated, therefore, the segment should           */
            /*       be dropped but since this is the last check dropping         */
            /*       the segment is the same as doing nothing (see RFC 793 pg 75) */

            break;

         /* **************************************************   */
         default:
            DEBUG_ERROR("ERROR(RTIP): tcpdo() - unknown state", NOVAR, 0, 0);
             break;
    }     /* end of switch */

    /* **************************************************   */
    /* free the input message                               */
    if (!queued)
        os_free_packet(msg);        

    return;
}

#if (INCLUDE_TCP_OUT_OF_ORDER)
/* ********************************************************************   */
/* tc_tcpdo_other_ooo() - do state processing for incoming TCP packet     */
/*                                                                        */
/*   Process the incoming message based upon current TCP state.  This     */
/*   routine might cause state transitions based upon received message.   */
/*   After processing the current packet, check if any packets which      */
/*   previously arrived out of order may be processed now.                */
/*                                                                        */
/*   Returns TRUE if packet is not out of order.  FALSE if it is out of   */
/*   order.                                                               */
/*                                                                        */
static INLINE void tc_tcpdo_other_ooo(PIFACE pi, PTCPPORT port, DCU msg,              /*__fn__*/
                               word dlen, word hlen, RTIP_BOOLEAN KS_FAR *queued) /*__fn__*/
{
RTIP_BOOLEAN not_ooo;
DCU     curr_msg;
RTIP_BOOLEAN queued_ooo;
word    curr_hlen;
word    curr_dlen;
RTIP_BOOLEAN curr_not_ooo;

    not_ooo = tc_tcpdo_other(pi, port, msg, dlen, hlen, queued, FALSE);

    /* see if any of the previously out of order packets can now be   */
    /* processed                                                      */
    if (not_ooo)        /* if not out of order */
    {
        while (port->out_order_dcu)
        {
            curr_msg = (DCU)(port->out_order_dcu);

            /* calculate total len and header length for current packet   */
            calc_tcp_len(curr_msg, &curr_hlen, &curr_dlen, FALSE);

            /* if the out of order packet previously received is not   */
            /* out of order now, then process it;                      */
            queued_ooo = FALSE;
#if (DEBUG_OOO)
            DEBUG_ERROR(" ", NOVAR, 0, 0);
            DEBUG_ERROR("process an out of order: seq, flags", DINT2,
                WARRAY_2_LONG(DCUTOTCPPKT(curr_msg)->tcp_seq), 
                DCUTOTCPPKT(curr_msg)->tcp_flags);
#endif
            curr_not_ooo = tc_tcpdo_other(pi, port, curr_msg, curr_dlen, 
                                          curr_hlen, 
                                          (RTIP_BOOLEAN KS_FAR *)&queued_ooo,
                                          TRUE);

            /* if packet from out-of-order list is still out of order    */
            /* then done processing out-of-order list                    */
            if (!curr_not_ooo)   
            {
                DEBUG_ERROR("packet still out of order", NOVAR, 0, 0);
                break;
            }

#if (PRINT_OOO)
            DEBUG_ERROR("---- before remove ------", NOVAR, 0, 0);
            print_ooo(port);
#endif
            /* processed an out of order, therefore, take it out of out    */
            /* of the out of order list;                                   */
            /* NOTE: it will always be the first one on the list           */
            port->out_order_dcu = (POS_LIST)
                os_list_remove_off((POS_LIST)port->out_order_dcu, 
                                   (POS_LIST)port->out_order_dcu,
                                    TCP_OOO_OFFSET);
#if (PRINT_OOO)
            DEBUG_ERROR("---- after remove ------", NOVAR, 0, 0);
            print_ooo(port);
#endif
#if (INCLUDE_OOO_QUE_LIMIT)
            num_ooo_que--;
#endif

#if (DEBUG_OOO)
            DEBUG_ERROR("out of order packet processed: new head list ", 
                DINT1, port->out_order_dcu, 0);
            if (port->out_order_dcu)
            {
                DEBUG_ERROR("       sq of new ooo = ", DINT1, 
                   WARRAY_2_LONG(DCUTOTCPPKT(curr_msg)->tcp_seq), 0);
            }
#if (PRINT_OOO)
            print_ooo(port);
#endif
#endif

#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(curr_msg)->ctrl.list_id = IN_NO_LIST;
#endif
            /* the DCU was taken out of the out of order list above; if it was    */
            /* queued in the TCP window then do not free it; otherwise            */
            /* we are done with the packet                                        */
            if (!queued_ooo)
                os_free_packet(curr_msg);
        }
    }
}
#endif      /* INCLUDE_TCP_OUT_OF_ORDER */

/* ********************************************************************   */
/* tc_tcpdo_other() - do state processing for incoming TCP packet         */
/*                                                                        */
/*   Process the incoming message based upon current TCP state.  This     */
/*   routine might cause state transitions based upon received message.   */
/*                                                                        */
/*   Returns TRUE if packet is not out of order.  FALSE if it is out of   */
/*   order.                                                               */
/*                                                                        */
static RTIP_BOOLEAN tc_tcpdo_other(PIFACE pi, PTCPPORT port, DCU msg,              /*__fn__*/
                              word dlen, word hlen, RTIP_BOOLEAN KS_FAR *queued, /*__fn__*/
                              RTIP_BOOLEAN out_of_order)                          /*__fn__*/    
{
PTCPPKT pt;
int  seq_status;
RTIP_BOOLEAN ret_val;

    ret_val = TRUE;

    pt = DCUTOTCPPKT(msg);

    /* **************************************************               */
    /* check seq number in packet (see pg 69 RFC 793)                   */
    /* tbd - if seg unacceptable and window is 0 process ACKS, URGs and */
    /*       RSTs (not FINs)                                            */
    seq_status = check_seq(port, pt, dlen);
    if (seq_status == DROP_PKT)
    {
        /* ICMP send removed since SKO UNIX resets the connection when                  */
        /* it receives this ICMP message.  According to RFC 793, a parameter            */
        /* problem ICMP messagse should not abort the connection but should             */
        /* be passed to the application via CB_ERROR_REPORT. This message would be send */
        /* when a duplicate packet is receive.  Probably it would                       */
        /* only be sent if the packet is to the right of the window and                 */
        /* not if it is a duplicat                                                      */
        /*tc_icmp_send(msg, ICMP_T_PARAM_PROBLEM, ICMP_C_PTR_VALID,                     */
        /*             ICMP_P_TCPSEQ);                                                  */
        return(ret_val);
    }

    /* **************************************************                  */
    /*                                                                     */
    /* NOTE: at this point, packets have either a valid seq number or they */
    /*       should have their ACKs, URGs (tbd), and RST flags processed   */
    /*                                                                     */
    /* **************************************************                  */
    /* check for resets                                                    */
    if (pt->tcp_flags & TCP_F_RESET)
    {
        proc_reset(port);
        return(ret_val);
    }

    /* **************************************************          */
    /* check SYN bit, error if it is in window (see pg 71 RFC 793) */
    /* NOTE: if SYN is not in window would not get here due        */
    /*       to seq check                                          */
    if (seq_status == VALID_PKT)
    {
        if (pt->tcp_flags & TCP_F_SYN)   /* receive SYN  */
        {
            proc_syn(pi, port, msg);
            DEBUG_LOG("tcpdo - error - syn set", LEVEL_3, NOVAR, 0, 0);
            return(ret_val);
        }
    }

    /* **************************************************     */
    /* if the ACK bit is off, drop the packet (RFC 793 pg 72) */
    if (!(pt->tcp_flags & TCP_F_ACK))
    {
         DEBUG_LOG("tcpdo - error - no ACK", LEVEL_3, NOVAR, 0, 0);
         return(ret_val);
    }
 
    /* remove any bytes from output window that have been acked   */
    /* check and accept a possible piggybacked ack even if SEQ    */
    /* number is invalid (RFC 793 pg 69)                          */
    if ( (port->state != TCP_S_TWAIT) &&        /* no check for  TWAIT or */
         (port->state != TCP_S_LAST) &&         /* LAST */
         (port->state != TCP_S_SYNR) )          /* SYNR does it own checks */
        check_ack(pi, port, pt, dlen);

    /* **************************************************   */
    /* process segment data and FINS                        */
    /* tbd - URGs                                           */
    switch(port->state)
    {
        /* *****   */
        case TCP_S_SYNR:
 
            DEBUG_LOG("  SYNR", LEVEL_3, NOVAR, 0, 0);

            /* if there is an incorrect ACK reset (RFC 793 pg 36) reset;   */
            /* NOTE: out.next has not been updated for SYNC sent           */
            /*       when transitions LISTEN->SYNR                         */
            if ( WARRAY_2_LONG(pt->tcp_ack) != (port->out.nxt_to_send) ) 
            {   
                DEBUG_ERROR("  tc_tcpdo_other() - SYNR: send reset: ack, nxt", DINT2,
                    WARRAY_2_LONG(pt->tcp_ack), port->out.nxt_to_send);
                DEBUG_LOG("pt->tcp_ack, out.nxt", LEVEL_3, DINT2,
                    WARRAY_2_LONG(pt->tcp_ack), port->out.nxt_to_send);
                tc_tcp_reset(pi, port, msg); /* if it is a reset then tc_tcp_reset */
                                             /* will not actually send the reset   */
                break;
            }
                            
            /* correct ACK,ack was received, transition to EST              */
            /* NOTE: increment out.nxt to count SYNC sent when transition   */
            /*       from LISTEN->SYNR now that the SYNC has been correctly */
            /*       acked; it is not incremented when sending SYNC|ACK     */
            /*       since might resend it in listen_proc()                 */
            /*                                                              */
            DEBUG_LOG("  ACK from our SYN|ACK came; SYNR->EST", LEVEL_3, 
                NOVAR, 0, 0);
            ACCOUNT_ONE_BYTE(port);        /* account for my SYN */
            port->out.max_nxt_to_send = port->out.nxt_to_send;

            port->out.size = pt->tcp_window;   /* allowed window  */
            trans_state_est(port);

            /* fall through incase any incoming data in msg which could happen   */
            /* especially if other sides ACK in response to out SYNC|ACK go lost */

        /* *****   */
        case TCP_S_EST:                               /* normal data trans. */
            DEBUG_LOG("  tc_tcpdo_other() - EST", LEVEL_3, NOVAR, 0, 0);

            /* check seq number and enqueues any data              */
            /* process any data, if not out of order check for FIN */
            if (seq_status == VALID_PKT)
            {
                ret_val = check_data(port, msg, pt, dlen, hlen, queued,
                                     out_of_order);
                if (port->state == TCP_S_CLOSED)
                    break;

                DEBUG_LOG("tc_tcpdo_other - back from check_data", 
                    LEVEL_3, NOVAR, 0, 0);

                if (ret_val)   /* don't FIN yet if out of order  */
                {
                    check_fin(port, pt, dlen, TCP_S_CWAIT);         
                    DEBUG_LOG("tc_tcpdo_other - back from check_fin", 
                        LEVEL_3, NOVAR, 0, 0);
                }
            }

            /* if all output data has been acked and app previously     */
            /* did an closesocket(), send FIN and go to FW1 (if in EST) */
            /* or CWAIT state (if in CWAIT) to                          */
            /* wait for ack, i.e. closesocket was executed but state    */
            /* transition was not done since not all output had been    */
            /* acked                                                    */
            /* NOTE: at this point we could be either in EST or CWAIT,  */
            /*       therefore, if we are in CWAIT (i.e. we got a FIN   */
            /*       in this msg) and the application previously did    */
            /*       a close and this msg has acked all our output data */
            /*       then we must go to LAST                            */
            check_close(port);
            DEBUG_LOG("tc_tcpdo_other - back from check_close", 
                LEVEL_3, NOVAR, 0, 0);

            break;

        /* *****                                                    */
        /* FIN received, waiting for application to issue a close   */
        /* process; transition to LAST is handled by tc_tcp_write() */
        /* and tcp_close()                                          */
        case TCP_S_CWAIT:                           /* FIN received */
            DEBUG_LOG("  tc_tcpdo_other() - CWAIT", LEVEL_3, NOVAR, 0, 0);

            /* ******                                            */
            /* process any resends of FIN by sending an ACK only */
            if (seq_status == VALID_PKT)
                check_fin(port, pt, dlen, port->state);

            /* ******                                                 */
            /* if all output data has been acked and app previously   */
            /* did an closesocket(), send FIN and go to LAST state to */
            /* wait for ack, i.e. closesocket was executed but state  */
            /* transition was not done since not all output had been  */
            /* acked                                                  */
            check_close(port);

            break;

        /* *****   */
        case TCP_S_LAST:     /* FIN sent waiting for ACK */
            DEBUG_LOG("  tc_tcpdo_other() - LAST", LEVEL_3, NOVAR, 0, 0);

            /* ******                                               */
            /* if our FIN was acked, go to CLOSED                   */
            /* NOTE: don't bother updating out.nxt or out.ack since */
            /*       won't be needed any more                       */
            if ( WARRAY_2_LONG(pt->tcp_ack) >= (port->out.max_nxt_to_send+1) )
            {
                ACCOUNT_ONE_BYTE(port);        /* account for my FIN */
                trans_state(port, TCP_S_CLOSED, FROM_INTERPRET, FALSE);
            }
            
            /* ******                                                */
            /* process any resends of FIN by sending an ACK only but */
            /* do not change state since it must be a retry FIN      */
            if (seq_status == VALID_PKT)
                check_fin(port, pt, dlen, port->state);

            break;

        /* ******   */
        case TCP_S_FW1:                     /* waiting for ACK of FIN  */
            DEBUG_LOG("  tc_tcpdo_other() - FW1", LEVEL_3, NOVAR, 0, 0);

            /* process any incoming data; if out of order, drop pkt   */
            if (seq_status == VALID_PKT)
            {
                if (!check_data(port, msg, pt, dlen, hlen, queued, 
                                out_of_order))
                {
                    DEBUG_ERROR("tc_tcpdo_other - FW1 - out of order pkt", NOVAR, 0, 0);
                    break;
                }
            }

/*          DEBUG_LOG("tc_tcpdo_other - ack got, out.nxt", LEVEL_3, DINT2,     */
/*              WARRAY_2_LONG(pt->tcp_ack), port->out.nxt);                    */

            /* if my FIN has not been acked   */
            if (WARRAY_2_LONG(pt->tcp_ack) != (port->out.max_nxt_to_send+1))
            {   
                if (seq_status == VALID_PKT)
                {
                    /* got FIN but no ACK for mine->CLOSING   */
                    if (!check_fin(port, pt, dlen, TCP_S_CLOSING) )
                    {
                        DEBUG_LOG("tcpdo - FW1 - no ack or FIN, resend FIN:ack,nxt_to_send+1=", 
                            LEVEL_3, DINT2, WARRAY_2_LONG(pt->tcp_ack), port->out.nxt_to_send+1);
                    }
                }
                break;
            }

            /* my FIN has been acked                                     */
            /* all output bytes have been acked, if FIN then we are done */
            /* go to TWAIT to perform delay before closing               */
            ACCOUNT_ONE_BYTE(port);        /* account for my FIN */
                                           /* fw1->fw2 and fw1->twait   */
            if (seq_status == VALID_PKT)
            {
                if (!check_fin(port, pt, dlen, TCP_S_TWAIT) )
                {                              /* got ACK, no FIN  */
                    /* all output bytes have been acked and no FIN   */
                    DEBUG_LOG("tcpdo - FW1 => FW2", LEVEL_3, NOVAR, 0, 0);
                    trans_state(port, TCP_S_FW2, FROM_INTERPRET, FALSE);
                }
            }
            break;

        /* *****   */
        case TCP_S_FW2:                                /* want FIN  */
            DEBUG_LOG("  tc_tcpdo_other() - FW2", LEVEL_3, NOVAR, 0, 0);
 
            /* process any incoming data; if out of order, drop pkt   */
            if (seq_status == VALID_PKT)
            {
                if (!check_data(port, msg, pt, dlen, hlen, queued, 
                                out_of_order))
                {
                    DEBUG_LOG("tc_tcpdo_other - FW2 - out of order pkt", LEVEL_3, NOVAR, 0, 0);
                    break;
                }

                check_fin(port, pt, dlen, TCP_S_TWAIT);         
            }
            break;

        /* *****   */
        case TCP_S_CLOSING:                        /* want ACK of FIN  */
            DEBUG_LOG("  tc_tcpdo_other() - CLOSING", LEVEL_3, NOVAR, 0, 0);

            /* ******   */
            if (WARRAY_2_LONG(pt->tcp_ack) == (port->out.max_nxt_to_send+1))
            {
                ACCOUNT_ONE_BYTE(port);        /* account for my FIN */

                trans_state(port, TCP_S_TWAIT, TRUE, FALSE);
                DEBUG_LOG("tcpdo - CLOSING->TWAIT", LEVEL_3, NOVAR, 0, 0);
            }
            else
            {
                DEBUG_LOG("tcpdo - CLOSING - no ack : ack,nxt_to_send+1=", 
                    LEVEL_3, DINT2, WARRAY_2_LONG(pt->tcp_ack), port->out.nxt_to_send+1);
            }

            /* ******                                                */
            /* process any resends of FIN by sending an ACK only but */
            /* do not change state since it must be a retry FIN      */
            if (seq_status == VALID_PKT)
                check_fin(port, pt, dlen, port->state);

            break;

        /* *****   */
        case TCP_S_TWAIT:                        /* ack FIN again?  */
            DEBUG_LOG("  tc_tcpdo_other() - TWAIT", LEVEL_3, NOVAR, 0, 0);

            /* ******                                                */
            /* process any resends of FIN by sending an ACK only but */
            /* do not change state since it must be a retry FIN      */
            if (seq_status == VALID_PKT)
            {
                if (check_fin(port, pt, dlen, port->state))
                {
                    port->closetime = 0;
                }
            }
            break;
 
        /* *****   */
        default:     /* should never happen */
            DEBUG_ERROR("ERROR(RTIP): tcpdo_other() - unknown state", NOVAR, 0, 0);
            break;

    }     /* end of switch */

    return(ret_val);
}

/* ********************************************************************      */
/* tc_tcp_reset() - send a reset packet                                      */
/*                                                                           */
/*   Send a reset packet back to sender.                                     */
/*   Uses the packet which just came in as a template to return when sending */
/*   a reset back to sender.  Fill in all of the fields necessary and send   */
/*   a newly allocated packet.  Newly allocated DCU will be freed after      */
/*   it is sent on the wire.                                                 */
/*                                                                           */
/*   NOTE: port could be 0                                                   */
/*                                                                           */
/*   Assumes TCP semaphore is claimed when called.                           */
/*                                                                           */

static void tc_tcp_reset(PIFACE pi, PTCPPORT port, DCU in_msg)         /*__fn__*/
{
PTCPPKT pt;          /* input pkt */
#if (INCLUDE_IPV4)
PIPPKT  pip;         /* input pkt */
PIFACE  pi_out;
#endif
dword   ltemp;
DCU     out_msg;
PTCPPKT out_pt;
PIPPKT  out_pip;
byte    send_ip_addr[4];
#if (RTIP_VERSION >= 26)
int     pkt_len;
#endif

    DEBUG_LOG("tc_tcp_reset() entered", LEVEL_3, NOVAR, 0, 0);
#if (DEBUG_RESET)
    DEBUG_ERROR("tc_tcp_reset() - RESET sent", NOVAR, 0, 0);
#endif

    pip = DCUTOIPPKT(in_msg);
    pt  = DCUTOTCPPKT(in_msg);

    if (pt->tcp_flags & TCP_F_RESET)        /* don't reset a reset  */
    {
#if (DISPLAY_NORMAL)
        DEBUG_ERROR("tc_tcp_reset - don't send reset cuz got reset", NOVAR, 0, 0);
#endif
        return;
    }

#if (INCLUDE_IPV4)
    /* get interface from routing table for output;                  */
    /* if no match in routing table, use interface packet came in on */
    pi_out = rt_get_iface(pip->ip_src, (PFBYTE)send_ip_addr, 
                          (PANYPORT)0, (RTIP_BOOLEAN *)0);
    if (!pi_out)
        pi_out = pi;
#endif

#if (RTIP_VERSION >= 26)
    /* get total packet size with no options    */
    TOTAL_TCP_HLEN_NOOPT_SIZE(pkt_len, port, pi, 0)

    /* get new packet to send reset msg                            */
    /* NOTE: might not have a port associated with error so cannot */
    /*       call get_new_out_pkt                                  */
    if ( (out_msg = ip_alloc_packet(pi_out, (PANYPORT)port, pkt_len, 0,
                                    TCP_RESET_ALLOC)) == (DCU)0 )
    {
        DEBUG_ERROR("ERROR(rtip) - tc_tcp_reset() - ip_alloc_packet failed", 
            NOVAR, 0, 0);
        return;
    }
#else
    /* get new packet to send reset msg                            */
    /* NOTE: might not have a port associated with error so cannot */
    /*       call get_new_out_pkt                                  */
    if ( (out_msg = ip_alloc_packet((PANYPORT)port, TCP_TLEN_BYTES, 0,
                                    TCP_RESET_ALLOC)) == (DCU)0 )
    {
        DEBUG_ERROR("ERROR(rtip) - tc_tcp_reset() - ip_alloc_packet failed", NOVAR, 0, 0);
        return;
    }
#endif

    /* copy IP header info from pkt received to new packet         */
    /* before setting up individual fields; don't copy any options */
#if (INCLUDE_IPV4)
    tc_movebytes((PFBYTE)DCUTOIPPKT(out_msg), (PFBYTE)DCUTOIPPKT(in_msg), 
                 IP_HLEN_BYTES);
#else
    tbd
#endif

    out_pt  = DCUTOTCPPKT(out_msg);

    /* if incoming packet which caused the problem contains an ACK,   */
    /* take the seq number from incoming ack field                    */
    if (pt->tcp_flags & TCP_F_ACK)                  
    {
        out_pt->tcp_flags = TCP_F_RESET;
        LONG_2_WARRAY(out_pt->tcp_seq, hl2net(WARRAY_2_LONG(pt->tcp_ack)));
                                                    /* ack becomes next seq #    */
        LONG_2_WARRAY(out_pt->tcp_ack, 0L);         /* ack # is 0  */
    }

    else
    {
        /* Ack the sequence # minus the size of the whole TCP packet   */
        out_pt->tcp_flags = TCP_F_RESET|TCP_F_ACK;
        LONG_2_WARRAY(out_pt->tcp_seq, 0L);

        /* set ack value based upon incoming seq #   */
        ltemp = WARRAY_2_LONG(pt->tcp_seq);
        ltemp += (dword)IP_LEN_DCU(in_msg);
        ltemp -= (dword) IP_HLEN_BYTES;         /* IP header */
        ltemp -= (dword) (pt->tcp_hlen>>2);     /* TCP header */
        if (pt->tcp_flags & TCP_F_FIN)          /* acknowledge the fin bit */
            ltemp++;
        if (pt->tcp_flags & TCP_F_SYN)          /* acknowledge the sync bit */
            ltemp++;
        LONG_2_WARRAY(out_pt->tcp_ack, hl2net(ltemp));
    }

    out_pt->tcp_source = pt->tcp_dest;     /* swap port #s */
    out_pt->tcp_dest   = pt->tcp_source;
    out_pt->tcp_hlen   = TCP_HLEN_SFT;       /* header len includes no options */
    out_pt->tcp_window = 0;
    out_pt->tcp_urgent = 0;

    out_pip = DCUTOIPPKT(out_msg);

    /* set up ethernet header   */
    SETUP_LL_HDR(out_msg, pi, EIP_68K, SEND_802_2(pi, (PANYPORT)port),
                 pkt_len - ETH_HLEN_BYTES)
    SETUP_LL_HDR_DEST(pi, out_msg, LL_SRC_ADDR(in_msg, pi))

#if (INCLUDE_IPV4)
    /*  IP and data link layers   */
    tc_mv4(out_pip->ip_dest, pip->ip_src, IP_ALEN);  /* machine it came from  */
    tc_mv4(out_pip->ip_src, pi->addr.my_ip_addr, IP_ALEN); 
    setup_ipv4_header(out_msg, (PANYPORT)port, TCP_HLEN_BYTES+IP_HLEN_BYTES, 
                      0, FALSE, 0, TRUE);
#endif

    out_pt->tcp_chk = 0;    
    out_pt->tcp_chk = tc_tcp_chksum(out_msg);

#if (RTIP_VERSION >= 26)
    DCUTOPACKET(out_msg)->length = pkt_len;
#else
    DCUTOPACKET(out_msg)->length = TCP_TLEN_BYTES;
#endif

    INCR_SNMP(IpOutRequests)
    INCR_SNMP(TcpOutSegs)
    INCR_SNMP(TcpOutRsts)

#if (INCLUDE_IPV4)
    /* Send the packet.  Discard it when through.  Do not signal.   */
    /* Do not retransmit on timeout                                 */
    tc_netwrite(pi_out, (PANYPORT)port, out_msg, send_ip_addr, 
                NO_DCU_FLAGS, (word)RTIP_INF);
#endif
}
                              

/* ********************************************************************   */
/* TCP PORT LIST MANAGEMENT ROUTINES                                      */
/* ********************************************************************   */

#if (INCLUDE_TCP_OUT_OF_ORDER)
/* ********************************************************************   */
/* add_tcpdcu_list_order() - add DCU to an ordered list                   */
/*                                                                        */
/*   Adds the DCU msg to the ordered list port->out_order_dcu.  The       */
/*   list is ordered based upon sequence numbers.                         */
/*                                                                        */
/*   Assumes TCP sem claimed when called.                                 */

void add_tcpdcu_list_order(DCU msg, PTCPPORT port, RTIP_BOOLEAN KS_FAR *queued)   /*__fn__*/
{
DCU curr_msg, prev_msg;
PTCPPKT pt, pt_msg;
dword sq, sq_msg;
long sq_diff;

#if (INCLUDE_OOO_QUE_LIMIT)
    if (num_ooo_que >= CFG_NUM_OOO_QUE)
        return;
#endif

    curr_msg = (DCU)(port->out_order_dcu);
    prev_msg = (DCU)0;

    pt_msg = DCUTOTCPPKT(msg);
    sq_msg = WARRAY_2_LONG(pt_msg->tcp_seq);

#if (DEBUG_OOO)
    sq = 0;     /* compiler warning */
    DEBUG_ERROR("add_tcpdcu_list_order: add it ooo: seq, flags", DINT2,
        WARRAY_2_LONG(pt_msg->tcp_seq), pt_msg->tcp_flags);
#endif

    while (curr_msg)
    {
        pt = DCUTOTCPPKT(curr_msg);
        sq = WARRAY_2_LONG(pt->tcp_seq);

        /* check if this is the spot to insert the out of order packet   */
        /* i.e. before curr_msg;                                         */
        /* subtration done instead of direct compare to handle sequence  */
        /* wrap correctly                                                */
        sq_diff = (long)((long)sq - (long)sq_msg);

        /* if out-of-order packet is already queued then drop it   */
        /* i.e. do not queue it in the out of order list           */
        if (sq_diff == 0L)
        {
#if (DEBUG_OOO)
            DEBUG_ERROR("in list already: sq, add before sq = ", DINT2, sq_msg, sq);
#endif
            return;
        }

        /* if sq > sq_msg then insert sq_msg (msg) before sq (curr_msg);   */
        /* i.e. insert it after prev_msg                                   */
        if (sq_diff > 0L)
            break;

        prev_msg = curr_msg;

        curr_msg = (DCU)
            os_list_next_entry_off((POS_LIST)port->out_order_dcu,
                                   (POS_LIST)curr_msg,
                                   TCP_OOO_OFFSET);
    }       /* end of loop thru ooo list */

#if (DEBUG_OOO)
    if (!prev_msg)
    {
        DEBUG_ERROR("add to front out of order: add to front: sq_msg = ", 
            DINT1, sq_msg, 0);
    }
    else if (!curr_msg)
    {
        DEBUG_ERROR("add to end out of order: sq_msg, add before sq = ", 
            DINT2, sq_msg, sq);
    }
    else
    {
        DEBUG_ERROR("add to out of order: sq_msg, add before sq = ", 
            DINT2, sq_msg, sq);
    }
#endif

    /* insert msg before curr_msg (i.e. after prev_msg)   */
    port->out_order_dcu = 
        os_list_add_middle_off(port->out_order_dcu,
                               (POS_LIST)msg,
                               (POS_LIST)prev_msg,
                                      TCP_OOO_OFFSET);
    *queued = TRUE;

#if (INCLUDE_OOO_QUE_LIMIT)
    num_ooo_que++;
#endif
#if (INCLUDE_TRK_PKTS)
    DCUTOPACKET(msg)->ctrl.list_id = OUT_OOO_LIST;
#endif
#if (PRINT_OOO)
    print_ooo(port);
#endif
}

#if (PRINT_OOO)
void print_ooo(PTCPPORT port)
{
DCU curr_msg;
POS_LIST curr_msg_info;
PTCPPKT pt;

    DEBUG_ERROR("list forward", NOVAR, 0, 0);
    curr_msg = (DCU)(port->out_order_dcu);
    while (curr_msg)
    {
        pt = DCUTOTCPPKT(curr_msg);
        sq = WARRAY_2_LONG(pt->tcp_seq);
        DEBUG_ERROR("print_ooo: entry: ", DINT1, sq, 0);
        curr_msg = (DCU)
            os_list_next_entry_off((POS_LIST)port->out_order_dcu,
                                   (POS_LIST)curr_msg,
                                   TCP_OOO_OFFSET);
    }

    DEBUG_ERROR("list backward", NOVAR, 0, 0);
    curr_msg = (DCU)(port->out_order_dcu);
    if (curr_msg)
    {
        curr_msg_info = POS_ENTRY_OFF(curr_msg, TCP_OOO_OFFSET);
        curr_msg = (DCU)(curr_msg_info->pprev);     /* last in list */
    }
    while (curr_msg)
    {
        pt = DCUTOTCPPKT(curr_msg);
        DEBUG_ERROR("print_ooo: entry: ", DINT1, pt->tcp_sq, 0);

        if (curr_msg == (DCU)(port->out_order_dcu))
            break;

        curr_msg_info = POS_ENTRY_OFF(curr_msg, TCP_OOO_OFFSET);
        curr_msg = (DCU)(curr_msg_info->pprev);

    }

}
#endif
#endif  /* INCLUDE_TCP_OUT_OF_ORDER */

/* ********************************************************************   */
/* add_tcpport_list() - add port to a list                                */
/*                                                                        */
/*   Assumes TCP sem claimed when called.                                 */

void add_tcpport_list(PTCPPORT port, int list_type)   /*__fn__*/
{
    port->ap.list_type = list_type;
    root_tcp_lists[list_type] = 
        os_list_add_rear_off(root_tcp_lists[list_type], 
                             (POS_LIST)port, ZERO_OFFSET);
}

/* ********************************************************************   */
/* delete_tcpport_list() - delete port from a list                        */
/*                                                                        */
/*   Assumes TCP sem claimed when called.                                 */

void delete_tcpport_list(PTCPPORT port)  /*__fn__*/
{
int list_type;

    list_type = port->ap.list_type;

#if (INCLUDE_TCP_PORT_CACHE)
    /* If tcp cacheing is turned and we are removing the cached port   */
    /* from the active list then zero out the cached port.             */
    if (list_type == ACTIVE_LIST && cached_tcp_port == port)
        cached_tcp_port = 0;
#endif
    root_tcp_lists[list_type] = 
        os_list_remove_off(root_tcp_lists[list_type], (POS_LIST)port, 
                           ZERO_OFFSET);
}

/* ********************************************************************   */
/* TCP INITIALIZATION ROUTINE                                             */
/* ********************************************************************   */
/* tcpinit() - initialize TCP connection                                  */
/*                                                                        */
/*   Initializes TCP connection by determining output interface and       */
/*   mss value to send.  If pi is not null then it is used as             */
/*   interface to send to if cannot find entry in the routing table.      */
/*                                                                        */
/*   Called when SYN message received or when connect() is called.        */
/*                                                                        */
/*   Returns TRUE is successful else FALSE                                */
/*                                                                        */

RTIP_BOOLEAN    tcpinit(PTCPPORT port, PFBYTE ip_dest)
{
PIFACE  pi_out;
RTIP_BOOLEAN is_gw;

    /* save interface info for output                       */
    /* NOTE: this is slave port on the listener list        */
    /* NOTE: if can't find interface associated with source */
    /*       IP address use interface pkt came in on as the */
    /*       default                                        */
    pi_out = rt_get_iface_lock(ip_dest, port->ap.send_ip_addr, (PANYPORT)port, 
                               (RTIP_BOOLEAN *)&is_gw);
    if (pi_out)   
        port->ap.iface = pi_out;
    else
    {
        DEBUG_ERROR("tcpinit failed: route not found in routing table",
            IPADDR, ip_dest, 0);
#if (DISPLAY_ROUTING_TABLE)
        DEBUG_ERROR("ROUTING TABLE: ", RT, FALSE, 0);
#endif

        /* could not find an interface, so send_ip_addr could not be   */
        /* set up either; return failure                               */
        return(FALSE);
    }

    /* if will send packets thru a gateway, limit mss to 536 as per    */
    /* RFC 1122 pg 60                                                  */
    if ( is_gw && (port->out.mss > CFG_MSS_GATEWAY) )
    {
        port->out.mss = CFG_MSS_GATEWAY;
    }
    
    /* set MSS value to send with sync message                                */
    /* if local send address was set to the dest address (see rt_get_iface()) */
    /* NOTE: prt save in port if port passed to rt_get_iface so prt should    */
    /*       always be set but just as a precaution make sure                 */
    port->in.mss = (word)(pi_out->addr.mtu - TCP_HLEN_BYTES - 
                          IP_HLEN_BYTES);

    DEBUG_LOG("tcpinit: mss sent = ", LEVEL_3, EBS_INT1, port->in.mss, 0);

    return(TRUE);

}


/* ********************************************************************   */
/* QUEUING FUNCTIONS                                                      */
/* ********************************************************************   */

/* ********************************************************************   */
/* enqueue_pkt() - add data to TCP queue (window)                         */
/*                                                                        */
/*  add data to a TCP queue (window).  Used by both 'write()' and         */
/*  tc_tcp_interpret()                                                    */
/*                                                                        */
/*  Returns number of bytes written to queue                              */
/*                                                                        */
int enqueue_pkt(PWINDOW wind, DCU msg, int nbytes)         /*__fn__*/
{
    /* **************************************************   */
    /* put at end of list                                   */
    wind->dcu_start = os_list_add_rear_off(wind->dcu_start, 
                                           (POS_LIST)msg,
                                           TCP_WINDOW_OFFSET);

    wind->contain += (word)nbytes;     
    return(nbytes);
}

#if (INCLUDE_TCP_COPY)
/* ********************************************************************   */
/* enqueue_data_in() - add data to TCP queue (input window)               */
/*                                                                        */
/*  Queue data to a TCP queue (input window) for copy mode.               */
/*  Used by 'tc_tcp_interpret()' i.e. check_data.                         */
/*  Updates wind->contain.                                                */
/*                                                                        */
/*  Returns number of bytes written to queue                              */
/*                                                                        */
word enqueue_data_in(PTCPPORT port, PFBYTE buffer, word nbytes)         /*__fn__*/
{
word i, tot_que;
word que_this_msg;
word bytes_per_pkt;
word data_end;
PWINDOW wind;
DCU msg;

#if (DEBUG_ENQUE)
    DEBUG_ERROR("enqueue_data_in: nbytes = ", EBS_INT1, nbytes, 0);
#endif

    wind = (PWINDOW) &port->in;
    DEBUG_LOG("enqueue_data_in() - try to queue nbytes, in->contain = ", LEVEL_3, 
        EBS_INT2, nbytes, wind->contain);

    bytes_per_pkt = (word)MAX_PACKETSIZE;   /* max to queue per msg */

    /* **************************************************   */
    if (wind->contain >= wind->window_size)  
        return(0);
    i = (word)(wind->window_size - wind->contain);  /* # bytes left in window */
    if (i==0 || nbytes==0)
        return(0);                            /* no room in window */

    /* only put bytes in queue which there is room for   */
    if (nbytes > i)
        nbytes = i;

    DEBUG_LOG("enqueue_data_in() - actually queue nbytes = ", LEVEL_3, 
        EBS_INT1, nbytes, 0);

    /* **************************************************   */
    /* put what data will fit in the last pkt               */
    tot_que = 0;

    if (wind->dcu_start)
    {
        /* get last entry   */
        msg = (DCU)(os_list_last_off(wind->dcu_start, TCP_WINDOW_OFFSET));
        if (!msg)
        {
            DEBUG_ERROR("enqueue_data_in - dcu_prev = 0", NOVAR, 0, 0);
            return(0);  /* should never happen */
        }

        /* get amt of room at end of buffer then limit it by number of bytes    */
        /* to queue                                                             */
        que_this_msg = (byte)(bytes_per_pkt - DCUTOPACKET(msg)->data_start - 
                              DCUTOPACKET(msg)->data_len);
        if (que_this_msg > nbytes)
            que_this_msg = nbytes;

        /* if any room at end of current buffer                          */
        /* NOTE: if there is room and window was empty, nxt_to_send_dcu  */
        /*       already points to end of current DCU                    */
        if (que_this_msg > 0)
        {
            DEBUG_LOG("enqueue_data_in() - queue at end of buffer - que_this_msg = ", LEVEL_3, 
                EBS_INT1, que_this_msg, 0);
            data_end = (word)(DCUTOPACKET(msg)->data_start + 
                       DCUTOPACKET(msg)->data_len);
            tc_movebytes(DCUTODATA(msg) + data_end, buffer, que_this_msg);
#if (DEBUG_ENQUE)
            DEBUG_ERROR("enqueue_data_in: que in pkt = ", EBS_INT1, 
                que_this_msg, 0);
            DEBUG_ERROR("       pkt: ", PKT, buffer, 5);
#endif

            buffer += que_this_msg;
            tot_que += que_this_msg;
            wind->contain += que_this_msg;   /* NOTE: won't call enqueue */
                                             /*       so need to do this here   */

            DCUTOPACKET(msg)->frag_data_len += que_this_msg;
            DCUTOPACKET(msg)->data_len += que_this_msg;
            DCUTOPACKET(msg)->length += que_this_msg;
        }
    }

    /* **************************************************        */
    /* in a loop, queue what the otherside will accept up to the */
    /* entire queue of data                                      */
    while (tot_que < nbytes)
    {
        /* get packet from DCU - copy mode, therefore,   */
        /* the whole buffer is part of the window        */
        msg = os_alloc_packet(MAX_PACKETSIZE, TCP_QUE_IN_ALLOC);  
        if (!msg)
        {
            DEBUG_ERROR("enqueue_data_in - alloc packet failed", NOVAR, 0, 0);
            break;      /* drop the rest of the data */
        }

        que_this_msg = bytes_per_pkt;

        if (tot_que+que_this_msg > nbytes)     
            que_this_msg = (word)(nbytes-tot_que);  /* send rest of data (ie last time thru loop) */

        DEBUG_LOG("enqueue_data_in() - in loop - queue_this_msg = ", LEVEL_3, EBS_INT1, 
            que_this_msg, 0);
#if (DEBUG_ENQUE)
        DEBUG_ERROR("enqueue_data_in: que in NEW pkt = ", EBS_INT1, 
            que_this_msg, 0);
        DEBUG_ERROR("       pkt: ", PKT, buffer, 5);
#endif

        tc_movebytes(DCUTODATA(msg), buffer, que_this_msg);

        buffer += que_this_msg;
        tot_que += que_this_msg;

        DCUTOPACKET(msg)->data_start = 0;
        DCUTOPACKET(msg)->data_len = DCUTOPACKET(msg)->frag_data_len = 
            que_this_msg;
        DCUTOPACKET(msg)->length = que_this_msg;

        /* put on end of input list   */
        enqueue_pkt(wind, msg, que_this_msg);
#if (INCLUDE_TRK_PKTS)
        DCUTOPACKET(msg)->ctrl.list_id = IN_IN_WIN_LIST;
#endif
    }

    DEBUG_LOG("enqueue_data_in() - queue done - tot_que, out->contain = ", LEVEL_3, 
        EBS_INT2, tot_que, wind->contain);

    return(tot_que);
}
#endif  /* INCLUDE_TCP_COPY */


/* ********************************************************************   */
/* enqueue_out() - add data to TCP queue (output window)                  */
/*                                                                        */
/*  add data to a TCP queue (window).  Used by 'tc_tcp_write()'.          */
/*  window_size is the size limitation of the advertised window.          */
/*                                                                        */
/*  Returns number of bytes written to queue.                             */
/*  Does not set errno upon error.                                        */
/*                                                                        */
int enqueue_out(PTCPPORT port, PFBYTE buffer, int nbytes, RTIP_BOOLEAN *pkt_err)         /*__fn__*/
{
int i, tot_que;
int que_this_msg;
int bytes_per_pkt;
int data_start;
#if (INCLUDE_TCP_COPY)
int data_end;
#endif
PWINDOW wind;
DCU msg;

    wind = (PWINDOW) &port->out;
    DEBUG_LOG("enqueue_out() - try to queue nbytes, out->contain = ", LEVEL_3, 
        EBS_INT2, nbytes, wind->contain);

#if (INCLUDE_TCP_NO_COPY)
    if (port->ap.options & SO_TCP_NO_COPY)  
    {
        /* NO COPY MODE                    */
        /* set size of TCP options to send */
        calc_tcp_options_len(port, 0);

        /* adjust by options which were setup by get_new_out_pkt   */
        bytes_per_pkt = tcp_pkt_data_max(port);
        TOTAL_TCP_HLEN_SIZE(data_start, port, port->ap.iface, 0) 
    }
    else
#endif      /* INCLUDE_TCP_NO_COPY */
    {
#if (INCLUDE_TCP_COPY)
        /* COPY MODE   */
        bytes_per_pkt = MAX_PACKETSIZE;   /* max to queue per msg */
        data_start = 0;
#endif      /* INCLUDE_TCP_COPY */
    }

    if (!bytes_per_pkt)
        return(0);

    /* **************************************************   */
    if (wind->contain >= wind->window_size)  
        return(0);
    i = wind->window_size - wind->contain;  /* # bytes left in window */
    if (i==0 || nbytes==0)
        return(0);                            /* no room in window */

    /* only put bytes in queue which there is room for   */
    if (nbytes > i)
        nbytes = i;

    DEBUG_LOG("enqueue_out() - actually queue nbytes = ", LEVEL_3, EBS_INT1, nbytes, 0);

    /* **************************************************   */
    /* put what data will fit in the last pkt               */
    tot_que = 0;
#if (INCLUDE_TCP_COPY)
    if (!(port->ap.options & SO_TCP_NO_COPY))
    {
        /* COPY MODE   */
        if (wind->dcu_start)
        {
            msg = (DCU)(os_list_last_off(wind->dcu_start, TCP_WINDOW_OFFSET));
            if (!msg)
            {
                DEBUG_ERROR("enqueue_out - dcu_prev = 0", NOVAR, 0, 0);
                return(0);      /* this shouldn't happen */
            }
    
            /* get amt of room at end of buffer then limit it by number of bytes    */
            /* to queue                                                             */
            que_this_msg = bytes_per_pkt - DCUTOPACKET(msg)->data_start - 
                           DCUTOPACKET(msg)->data_len + data_start;
            if (que_this_msg > nbytes)
                que_this_msg = nbytes;

            /* if any room at end of current buffer                          */
            /* NOTE: if there is room and window was empty, nxt_to_send_dcu  */
            /*       already points to end of current DCU                    */
            if (que_this_msg > 0)
            {
                DEBUG_LOG("enqueue_out() - queue at end of buffer - que_this_msg = ", LEVEL_3, 
                    EBS_INT1, que_this_msg, 0);
                data_end = DCUTOPACKET(msg)->data_start + 
                           DCUTOPACKET(msg)->data_len;
                tc_movebytes(DCUTODATA(msg) + data_end, buffer, que_this_msg);

                buffer += que_this_msg;
                tot_que += que_this_msg;
                wind->contain += (word)que_this_msg;   /* NOTE: won't call enqueue */
                                                       /*       so need to do this here   */
    
#    if (NXT_DCU)
                if (!wind->nxt_to_send_dcu)
                {
                    wind->nxt_to_send_dcu = msg;
                    wind->nxt_to_send_off = data_end;
                }
#    endif
    
                DCUTOPACKET(msg)->data_len += que_this_msg;
                DCUTOPACKET(msg)->length += que_this_msg;
            }
        }
    }       /* end of copy mode */
#endif      /* INCLUDE_TCP_COPY */

    /* **************************************************        */
    /* in a loop, queue what the otherside will accept up to the */
    /* entire queue of data                                      */
    while (tot_que < nbytes)
    {
#if (RTIP_VERSION >= 26)
        que_this_msg = bytes_per_pkt;

        if (tot_que+que_this_msg > nbytes)     
            que_this_msg = nbytes - tot_que;  /* send rest of data (ie last time thru loop) */

        DEBUG_LOG("enqueue_out() - in loop - queue_this_msg = ", LEVEL_3, EBS_INT1, 
            que_this_msg, 0);
#endif

        /* get packet from DCU - if packet mode copy tcp header info   */
        /* else the whole buffer is part of the window                 */
#if (INCLUDE_TCP_NO_COPY)
        if (port->ap.options & SO_TCP_NO_COPY)
        {
            /* NO COPY MODE   */
#if (RTIP_VERSION >= 26)
            msg = get_new_out_pkt(port, 0, MAX_PACKETSIZE); /* will set up IP options */
#else
            msg = get_new_out_pkt(port, 0, MAX_PACKETSIZE); /* will set up IP options */
#endif
        }
        else
#endif          /* INCLUDE_TCP_NO_COPY */
        {
#if (INCLUDE_TCP_COPY)
            /* COPY MODE   */
            msg = os_alloc_packet(MAX_PACKETSIZE, TCP_QUE_OUT_ALLOC);  
#endif          /* INCLUDE_TCP_COPY */
        }
        if (!msg)
        {
            DEBUG_ERROR("enqueue_out - alloc packet failed", NOVAR, 0, 0);
            *pkt_err = TRUE;
            return(tot_que);    /* only que what we have enough */
                                /* DCUs for   */
        }

#if (RTIP_VERSION < 26)
        que_this_msg = bytes_per_pkt;

        if (tot_que+que_this_msg > nbytes)     
            que_this_msg = nbytes - tot_que;  /* send rest of data (ie last time thru loop) */

        DEBUG_LOG("enqueue_out() - in loop - queue_this_msg = ", LEVEL_3, EBS_INT1, 
            que_this_msg, 0);
#endif

        /* copy the data to the packet   */
        tc_movebytes(DCUTODATA(msg)+data_start, buffer, que_this_msg);

        buffer += que_this_msg;
        tot_que += que_this_msg;

        DCUTOPACKET(msg)->data_start = data_start;
        DCUTOPACKET(msg)->data_len = que_this_msg;
        DCUTOPACKET(msg)->length = que_this_msg;

        /* put on end of output list   */
        enqueue_pkt(wind, msg, que_this_msg);
#if (INCLUDE_TRK_PKTS)
        DCUTOPACKET(msg)->ctrl.list_id = IN_OUT_WIN_LIST;
#endif

        if (!wind->nxt_to_send_dcu)
        {
            wind->nxt_to_send_dcu = msg;
            wind->nxt_to_send_off = DCUTOPACKET(msg)->data_start;
        }
    }

    DEBUG_LOG("enqueue_out() - queue done - tot_que, out->contain = ", LEVEL_3, 
        EBS_INT2, tot_que, wind->contain);

    return(tot_que);
}

#if (INCLUDE_PKT_API)
/* ********************************************************************   */
/* enqueue_pkt_out() - add data to TCP queue (window)                     */
/*                                                                        */
/*  Add data to a TCP output window.  Called by tc_tcp_write() for        */
/*  xn_pkt_send().                                                        */
/*  window_size is the size limitation of the advertised window.          */
/*                                                                        */
/*  NOTE: either queues whole packet or does not queue and returns 0      */
/*                                                                        */
/*  Returns number of bytes written to queue (possibly 0)                 */
/*                                                                        */
int enqueue_pkt_out(PIFACE pi, PTCPPORT port, DCU msg)         /*__fn__*/
{
int     que_this_msg;
int     data_start;
PWINDOW wind;
int     i;

    wind = (PWINDOW) &port->out;

    /* NOTE: pkt already within MSS else returned error   */
    /* NOTE: does not support options                     */
    TOTAL_TCP_HLEN_NOOPT_SIZE(data_start, port, pi, (DCU)0)     
    que_this_msg = DCUTOPACKET(msg)->length - data_start;

    DEBUG_LOG("enqueue_pkt_out() - try to queue nbytes, out->contain = ", 
        LEVEL_3, EBS_INT2, que_this_msg, wind->contain);

    /* **************************************************            */
    /* if no room for whole pkt in window do not que any of it since */
    /* window is in packets                                          */
    if (wind->contain >= wind->window_size)  
        return(0);
    i = wind->window_size - wind->contain;  /* # bytes left in window */
    if (!que_this_msg || que_this_msg > i)
        return(0);                            

    DEBUG_LOG("enqueue_pkt_out() - actually queue que_this_msg = ", 
        LEVEL_3, EBS_INT1, que_this_msg, 0);

    /* **************************************************   */
    DEBUG_LOG("enqueue_pkt_out() - in loop - que_this_msg = ", 
        LEVEL_3, EBS_INT1, que_this_msg, 0);

    DCUTOPACKET(msg)->data_start = data_start;
    DCUTOPACKET(msg)->data_len = que_this_msg;
    DCUTOPACKET(msg)->length = que_this_msg;

    /* copy packet info from template to new packet (ethernet, IP and TCP   */
    /* headers)                                                             */
    /* tbd: IP options                                                      */
    SETUP_LL_HDR(msg, pi, EIP_68K, SEND_802_2(pi, (PANYPORT)port),
                 DCUTOPACKET(msg)->length - LL_HLEN_BYTES);
    tc_movebytes((PFBYTE)DCUTOIPPKT(msg),
                 (PFBYTE)&(port->out_ip_temp), 
                 IP_HLEN_BYTES);
    tc_movebytes((PFBYTE)DCUTOTCPPKT(msg),
                (PFBYTE)&(port->out_template), 
                sizeof(TCPPKT));

    /* put on end of output list   */
    enqueue_pkt(wind, msg, que_this_msg);
#if (INCLUDE_TRK_PKTS)
    DCUTOPACKET(msg)->ctrl.list_id = IN_OUT_WIN_LIST;
#endif

    if (!wind->nxt_to_send_dcu)
    {
        wind->nxt_to_send_dcu = msg;
        wind->nxt_to_send_off = DCUTOPACKET(msg)->data_start;
    }

    DEBUG_LOG("enqueue_pkt_out() - queue done - out->contain = ", LEVEL_3, 
        EBS_INT1, wind->contain, 0);

    return(que_this_msg);
}
#endif  /* INCLUDE_PKT_API */

/* ********************************************************************       */
/* dequeue() - copy and remove data from TCP window                           */
/*                                                                            */
/*   Used by read, this copies up to nbytes of data out of the queue and then */
/*   deallocates it from the queue.  The window consists of a linked          */
/*   list of DCUs which could be fragmented.  msg->data_len is length         */
/*   of packet and all its fragments.                                         */
/*                                                                            */
/*   Returns number of bytes copied from the queue                            */
/*                                                                            */
int dequeue(PWINDOW wind, PFBYTE buffer, int nbytes, int flags)         /*__fn__*/
{
DCU     msg, curr_msg;
int     nleft, curr_n, data_len;
int     data_start;

#if (DEBUG_DEQUE)
    DEBUG_ERROR("dequeue: nbytes = ", EBS_INT1, nbytes, 0);
#endif
    if (wind->contain == 0)
        return(0);

    /* limit by number of bytes in window   */
    if ((long)wind->contain < (long)nbytes)
        nbytes = wind->contain;

    nleft = nbytes;
    msg = (DCU)(wind->dcu_start);
    data_start = DCUTOPACKET(msg)->data_start;

    while (msg && nleft)
    {
        /* msg->data_len is length of all frags, so get it for curr msg   */
        /* data_len = data length of current frag                         */
        /* curr_n   = amt from current frag dequeueing                    */
        /* nleft    = total amount dequeueing                             */
        data_len = curr_n = DCUTOPACKET(msg)->frag_data_len;

        if (curr_n > nleft)  
            curr_n = nleft;

        /* in case of fragment, calculate data_len    */
        /* copy data to user buffer                   */
        tc_movebytes(buffer, DCUTODATA(msg) + data_start, curr_n);
#if (DEBUG_DEQUE)
        DEBUG_ERROR("dequeue: move data: curr_n = ", EBS_INT1, curr_n, 0);
        DEBUG_ERROR("     PKT: ", PKT, buffer, 5);
#endif

        nleft = nleft - curr_n;
        buffer += curr_n;
        data_len   = data_len - curr_n;
        data_start = data_start + curr_n;

        if (!(flags & MSG_PEEK))
        {
            DCUTOPACKET(msg)->data_len = 
                DCUTOPACKET(msg)->data_len - curr_n;
            DCUTOPACKET(msg)->frag_data_len = 
                DCUTOPACKET(msg)->frag_data_len - curr_n;
            DCUTOPACKET(msg)->data_start = 
                DCUTOPACKET(msg)->data_start + curr_n;
        }

        /* if dequeued all data from current packet   */
        if (data_len <= 0)
        {
            curr_msg = msg;

            if (!(flags & MSG_PEEK))
            {
                /* take first entry off the list;                              */
                /* NOTE: if packets is fragmented, the next fragment will      */
                /*       become the head of the list; if current packet is not */
                /*       fragmented the next packet on the list becomes        */
                /*       the head of the list                                  */
                wind->dcu_start = os_list_remove_off(wind->dcu_start, 
                                                     wind->dcu_start,
                                                     TCP_WINDOW_OFFSET);
                msg = (DCU)(wind->dcu_start);       /* set-up for next loop */
            }
            else
            {
                msg = (DCU)os_list_next_entry_off(wind->dcu_start, 
                                                  (POS_LIST)msg,
                                                  TCP_WINDOW_OFFSET);
            }
#if (INCLUDE_FRAG)
            /* if remove packet is a fragment, put the next fragment   */
            /* at the head of the window                               */
            if (curr_msg->frag_next)
            {
                msg = DCUTOPACKET(curr_msg)->frag_next;     
                if (!(flags & MSG_PEEK))
                {
                    /* NOTE: if packets is fragmented, the next fragment becomes   */
                    /*       the head of the list;                                 */
                    wind->dcu_start = os_list_add_front_off(wind->dcu_start, 
                                                            (POS_LIST)msg,
                                                            TCP_WINDOW_OFFSET);

                    /* make sure freeing packet does not free the whole list   */
                    curr_msg->frag_next = (DCU)0;
                }
            }
#endif
            if (!(flags & MSG_PEEK))
            {
                os_free_packet(curr_msg);
            }
                
            if (msg)
            {
                data_start = DCUTOPACKET(msg)->data_start;
            }
        }       /* end of if data in current msg exhausted */
    }           /* end of while */

    if (!(flags & MSG_PEEK))
        wind->contain = (word)(wind->contain - nbytes);
    return(nbytes);
}


#if (INCLUDE_PKT_API)
/* ********************************************************************   */
/* dequeue_pkt() - removes and returns packet from TCP window             */
/*                                                                        */
/*   Used by read, this remove one packet out of the queue.  It does      */
/*   not free the packet.                                                 */
/*                                                                        */
/*   Returns packet                                                       */
/*                                                                        */
DCU dequeue_pkt(PWINDOW wind, int flags)         /*__fn__*/
{
DCU curr_msg;
int nbytes;

    if (wind->contain == 0)
        return((DCU)0);

    curr_msg = (DCU)wind->dcu_start;
    if (!curr_msg)
    {
        DEBUG_ERROR("dequeue_pkt - no packets in window", NOVAR, 0, 0);
        return((DCU)0);
    }
    nbytes = DCUTOPACKET(curr_msg)->data_len;

    if (!(flags & MSG_PEEK))
    {
        wind->dcu_start = os_list_remove_off(wind->dcu_start, 
                                             wind->dcu_start,
                                             TCP_WINDOW_OFFSET);

        wind->contain = (word)(wind->contain - nbytes);
    }
    return(curr_msg);
}
#endif  /* INCLUDE_PKT_API */


/* ********************************************************************   */
/*  rmqueue() - remove data from TCP queue (window)                       */
/*                                                                        */
/*   does the queue deallocation (used for output window as data acked)   */
/*                                                                        */
/*   rmqueue of WINDOWSIZE or greater bytes will empty the queue          */
/*                                                                        */
int rmqueue(PWINDOW wind, int nbytes)         /*__fn__*/
{
DCU msg;
int nleft, curr_n;

    DEBUG_LOG("rmqueue - called - nbytes, out->contain = ", LEVEL_3, EBS_INT2, 
                nbytes, wind->contain);
    /* limit by number of bytes in window                                  */
    /* NOTE: this could happen if the otherside is acking a FIN along with */
    /*       the data; i.e. nbytes could be 1 byte to large; it is easier  */
    /*       to fix it here than test for it before calling this routine   */
    if ((long)wind->contain < (long)nbytes)
    {
        DEBUG_LOG("rmqueue - limit by number bytes in window - nbyte, wind->contain = ",
            LEVEL_3, EBS_INT2, nbytes, wind->contain);
        nbytes = wind->contain;
    }

    nleft = nbytes;
    while (wind->dcu_start && nleft)
    {
        msg = (DCU)(wind->dcu_start);

        curr_n = DCUTOPACKET(msg)->data_len;
        DEBUG_LOG("rmqueue - in loop - curr_n, nleft = ", LEVEL_3, EBS_INT2, 
                    curr_n, nleft);
        if (curr_n > nleft)  
            curr_n = nleft;

        nleft = nleft - curr_n;
        DCUTOPACKET(msg)->data_len = DCUTOPACKET(msg)->data_len - curr_n;
        DCUTOPACKET(msg)->data_start = DCUTOPACKET(msg)->data_start + curr_n;

        if (DCUTOPACKET(msg)->data_len <= 0)
        {
            /* take first entry off the list   */
            wind->dcu_start = os_list_remove_off(wind->dcu_start, 
                                                 wind->dcu_start,
                                                 TCP_WINDOW_OFFSET);
            os_free_packet(msg);
        }
    }

    if (!wind->dcu_start)
    {
        wind->nxt_to_send_dcu = (DCU)0;
        wind->nxt_to_send_off = 0;
    }

    wind->contain = (word)(wind->contain - nbytes);
    DEBUG_LOG("rmqueue - done - nbytes, out->contain = ", LEVEL_3, EBS_INT2, 
                nbytes, wind->contain);
    return(nbytes);
}


/* ********************************************************************   */
/* setup_both_windows() - setup input and output windows                  */
/*                                                                        */
void setup_both_windows(PTCPPORT port)  /*__fn__*/
{
    port->lasttime = 0;
    port->sincetime = 0;
    port->intime = 0;

    /* Set up input & output queues.   */
    setupwindow((PWINDOW)&port->in);
    port->in.nxt_to_send = 0;  /* used for SHUTDOWN (it was set to nxt which */
                               /* is value output window needs for its use)   */
    setupwindow((PWINDOW)&port->out);
}

#if (INCLUDE_TCP_OUT_OF_ORDER)
/* ********************************************************************     */
/* free_ooo() - free all DCUs assosicated with the port's out of order list */
/*                                                                          */
/*   Frees all DCUs associated with ports out of order list                 */
/*                                                                          */
void free_ooo(PTCPPORT port)  /*__fn__*/
{
DCU msg;

    /* free any order or order packets received   */
    while (port->out_order_dcu)
    {
        msg = (DCU)(port->out_order_dcu);
        port->out_order_dcu = 
            os_list_remove_off(port->out_order_dcu, 
                               port->out_order_dcu,
                               TCP_OOO_OFFSET);

        os_free_packet(msg); 
#if (INCLUDE_OOO_QUE_LIMIT)
        num_ooo_que--;
#endif
    }
}
#endif  /* INCLUDE_TCP_OUT_OF_ORDER */

/* ********************************************************************   */
/* free_both_windows() - free all DCUs assosicated with the port          */
/*                                                                        */
/*   Frees all DCUs associated with port; i.e. both input (conditionally) */
/*   and output windows; out of order packets.  The parameter free_in     */
/*   specifies whether the input window should be freed.                  */
/*                                                                        */
void free_both_windows(PTCPPORT port, RTIP_BOOLEAN free_in)  /*__fn__*/
{
DCU msg;

    /* NOTE: this could be called when windows are already freed   */
    
    /* free input TCP window   */
    if (free_in)
    {
        while (port->in.dcu_start)
        {
            msg = (DCU)(port->in.dcu_start);
            port->in.dcu_start = 
                os_list_remove_off(port->in.dcu_start, 
                                   port->in.dcu_start,
                                   TCP_WINDOW_OFFSET);
            os_free_packet(msg); 
        }
        port->in.contain = 0;
    }

    /* free output TCP window   */
    while (port->out.dcu_start)
    {
        msg = (DCU)(port->out.dcu_start);
        port->out.dcu_start = 
            os_list_remove_off(port->out.dcu_start, 
                               port->out.dcu_start,
                               TCP_WINDOW_OFFSET);

        os_free_packet(msg); 
    }
    port->out.contain = 0;

#if (INCLUDE_TCP_OUT_OF_ORDER)
    /* free any order or order packets received   */
    free_ooo(port);
#endif
}


/* ********************************************************************   */
/* setupwindow() - set up a TCP window                                    */
/*                                                                        */
/*    Configure information about a window *w*                            */
/*                                                                        */
void setupwindow(PWINDOW w)                 /*__fn__*/
{
dword d;

    w->contain = 0;                        /* nothing here yet  */
    w->push = 0;

    /* base this on time of day clock, for uniqueness         */
/*    w->ack = w->nxt = ((ks_get_ticks()<<12) & 0xffffffful); */
    d = 0xffffffful;
    w->ack = w->nxt = ((ks_get_ticks()<<12) & d);

    w->nxt_to_send = w->max_nxt_to_send = w->nxt;   /* used for output window only */
    w->dcu_start = (POS_LIST)0;
    w->nxt_to_send_dcu = (DCU)0;
    w->nxt_to_send_off = 0;
}

/* ********************************************************************   */
/* setup_ws() - sets up TCP window size info                              */
/*                                                                        */
/*    Configure information about a window *w*                            */
/*    Called for INPUT window by transf() when sending SYNC message       */
/*    Called for OUTPUT window by trans_state_est() when transition       */
/*       to established TCP state                                         */

void setup_ws(PWINDOW w, word wsize, int io_type)              /*__fn__*/
{
    w->window_size = wsize;
    if (io_type == EBS_INPUT)   /* output is set from ws in pkt */
    {
        w->size = wsize;
        w->ad_size = wsize;
    }
    DEBUG_LOG("io_type, window size = ", LEVEL_3, EBS_INT2, io_type, wsize);
}

/* ********************************************************************   */
/* tcp_invalidate_sockets() - abort sockets bound to IP address           */
/*                                                                        */
/*    Sends reset and frees all sockets in system bound to the IP         */
/*    address ip_addr.  Called by DHCP client.                            */

void tcp_invalidate_sockets(PFBYTE ip_addr)
{
PTCPPORT tport;
int      i;

    for (i=FIRST_TCP_PORT; i < TOTAL_TCP_PORTS; i++)
    {
        tport = (PTCPPORT)(alloced_ports[i]);
        if (!tport || !IS_TCP_PORT(tport) || tport->state == TCP_S_FREE)
            continue;

        if ( (tport->ap.port_flags & PORT_BOUND) &&
             tc_cmp4(ip_addr, tport->out_ip_temp.ip_src, IP_ALEN) )
        {
            tc_tcp_abort_connection(tport, 
                READ_WAKEUP_FAIL|WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP,
                ENOTCONN, NOT_FROM_INTERPRET);
        }
    }
}

/* ********************************************************************     */
/* tcp_pkt_data_max() - Maximum number of bytes transferable per packet     */
/*                                                                          */
/*   The data max size for TCP is the value is the maximum packet size      */
/*   limited byt segment size sent in the initial sync message, therefore,  */
/*   this routine should not be called until the connection                 */
/*   has been established.                                                  */
/*                                                                          */
/*   Returns max data size or 0 upon error; sets errno if error             */
/*                                                                          */

int tcp_pkt_data_max(PTCPPORT pport)        /* __ fn __ */
{
int      ret_val;
int      hlen;
int      dlen_mss;

    /* must be connected to know mss from sync message   */
    if (tc_cmp4(pport->out_ip_temp.ip_dest, ip_nulladdr, 4)) 
    {
        set_errno(ENOTCONN);        
        return(0);
    }

    /* size is minimum of othersides MTU and size of pkt   */
    calc_tcp_options_len(pport, 0);
    TOTAL_TCP_HLEN_SIZE(hlen, pport, pport->ap.iface, 0)
    ret_val = CFG_MAX_PACKETSIZE - hlen;
    dlen_mss = (int)(pport->out.mss - pport->ap.ip_option_len -
                     pport->tcp_option_len);
    if (ret_val > dlen_mss)
        ret_val = dlen_mss;
    return(ret_val);
}

#endif      /* INCLUDE_TCP */


