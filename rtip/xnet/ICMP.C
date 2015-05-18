/*                                                                      */
/* icmp.c - ICMP functions                                              */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Functions in this module                                            */
/*   icmp interpret - Interpret and dispatch an ICMP packet.            */

#define DIAG_SECTION_KERNEL DIAG_SECTION_ICMP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

/* ********************************************************************   */
/* number of bytes to send along with IP header in ICMP message which     */
/* is sent in response to an input pkt                                    */
#define ORIG_DATA_SEND 8        /* 64 bits */

/* ********************************************************************   */
void process_icmp_other(PIFACE pi, DCU msg);
void report_icmp_error(PIFACE pi, DCU msg, int reason, int hard_error);
static int setup_and_send_icmp(DCU msg, int len, byte type, byte ccode);
/* ********************************************************************          */
/* INTERPRET                                                                     */
/* ********************************************************************          */
/* tc_icmp_interpret - Interpret and dispatch an ICMP packet.                    */
/*                                                                               */
/* This routine is called by the IP packet dispatcher to handle ICMP requests.   */
/* It handles ping replies by waking up the process blocked in xn_ping()         */
/* (if there is one). It also echoes ping requests back to hosts that send them. */
/* for other requests it simply gathers statistics.  The packet could be         */
/* fragmented.                                                                   */
/*                                                                               */

void tc_icmp_interpret(PIFACE pi, DCU msg)            /*__fn__*/
{
PICMPPKT pic;
word     len;
DCU      curr_msg;
PIPPKT   pip;
PIPPKT   pip_err;
word     seq = 0; 
#if (INCLUDE_PING)
PANYPORT port;
PFWORD   pw;
PIFACE   ping_pi;
int      iface_off;
#endif

#if (!INCLUDE_PING && !INCLUDE_ICMP && !INCLUDE_KEEP_STATS && !INCLUDE_ROUTING_TABLE)
    ARGSUSED_PVOID(pi);
#endif

    UPDATE_INFO(pi, icmp_packets, 1)
    INCR_SNMP(IcmpInMsgs)

    DEBUG_LOG("tc_icmp_interpret - called", LEVEL_3, NOVAR, 0, 0);

    /* Get the length of data from the message   */
    curr_msg = msg;
    len = 0;
#if (INCLUDE_FRAG)
    while (curr_msg)    /* loop thru fragments */
    {
#endif
        /* data len = ip packet length - ip header length (bytes)    */
        /*            (includes ICMP header len)                     */
        len = (word)(len + IP_LEN_DCU(curr_msg));
        len = (word)(len - IP_HLEN_DCU(curr_msg));

#if (INCLUDE_FRAG)
        curr_msg = DCUTOPACKET(curr_msg)->frag_next;
    }
#endif

    pip = DCUTOIPPKT(msg);
    pic = DCUTOICMPPKT(msg);
    DEBUG_LOG("tc_icmp_interpret - type = ", LEVEL_3, 
        EBS_INT1, pic->icmp_type, 0);

    /* verify the ICMP checksum   */
    if ((pic->icmp_chk) && ipf_icmp_chksum(msg, len))
    {
        UPDATE_INFO(pi, icmp_cksum, 1)
        DEBUG_ERROR("ERROR(RTIP): tc_icmp_interpret() - ICMP chksum len =",
            EBS_INT1, len, 0);
/*        DEBUG_ERROR("ERROR(RTIP): tc_icmp_interpret() - ICMP chksum err-type,chksum =",   */
/*          EBS_INT2, pic->icmp_type, ipf_icmp_chksum(msg, len));                           */

        /* Discard   */
        goto discard_it;
    }

    switch (pic->icmp_type)
    {
        case ICMP_T_ECHO_REPLY:
#if (INCLUDE_PING)
            DEBUG_LOG("tc_icmp_interpret - echo reply", LEVEL_3, NOVAR, 0, 0);

            pw = (PFWORD)pic->_4bytes;
            pw++;
            seq = net2hs((word)*pw);       /* sequence number */

            LOOP_THRU_IFACES(iface_off)
            {
                /* get interface   */
                ping_pi = tc_ino2_iface(iface_off, DONT_SET_ERRNO);
                if (ping_pi)
                {
                    /* Somebody replied to our ping request (sent via xn_ping())   */
                    /* loop thru ports on interface to find one to                 */
                    /* send PING reply to                                          */
                    OS_CLAIM_TABLE(PING_SEARCH_TABLE_CLAIM)
                    port = (PANYPORT)(ping_pi->ctrl.root_ping_ports);
                    while (port)
                    {
                        if (port->ping_sequence == (int)seq)
                        {
                            OS_RELEASE_TABLE()
                            OS_SNDX_PING_EXCHG(port, msg);
                            INCR_SNMP(IcmpInEchoReps)
                            return;     /* don't free packet */
                        }
                        port = (PANYPORT)
                            os_list_next_entry_off(ping_pi->ctrl.root_ping_ports,
                                                (POS_LIST)port, ZERO_OFFSET);
                    }
                    OS_RELEASE_TABLE()
                }
            }
#endif
            DEBUG_ERROR("tc_icmp_interpret: no match to PING reply: remote_ip = ",
                IPADDR, DCUTOIPPKT(msg)->ip_src, 0);
            DEBUG_ERROR("tc_icmp_interpret: seq = ", EBS_INT1, seq, 0);
            DEBUG_ERROR("tc_icmp_interpret: PING PORTS = ", PORTS_PING, 0, 0);

            /* Not found so fall through and free the packet   */
            break;

        case ICMP_T_ECHO_REQUEST:
            DEBUG_LOG("tc_icmp_interpret - echo request", LEVEL_3, NOVAR, 0, 0);
            setup_and_send_icmp(msg, len, ICMP_T_ECHO_REPLY, 0);
            UPDATE_INFO(pi, icmp_echo_requests, 1)
            INCR_SNMP(IcmpInEchos)
            INCR_SNMP(IcmpOutEchoReps)
            INCR_SNMP(IcmpOutMsgs)
            return;

#if (INCLUDE_ICMP)
        case ICMP_T_TIME_EXCEEDED:
        case ICMP_T_PARAM_PROBLEM:
        case ICMP_T_DEST_UNREACH:
        case ICMP_T_SOURCE_QUENCH:
            process_icmp_other(pi, msg);
            break;
#endif

        case ICMP_T_REDIRECT:
            /* point to the IP packet which caused the ICMP msg to be sent   */
            /* NOTE: there are 4 extra words for the ICMP header so          */
            /*       point 4 words down - NOTE: cannot access any ethernet   */
            /*       header info since will not be pointing to it but        */
            /*       IP info will be in the right place                      */
/*          pip_err = DCUTOIPPKT(msg);                                       */
            pip_err = (PIPPKT)DCUTOICMPPKT(msg);
            pip_err = (PIPPKT)((PFWORD)pip_err + 4);    
            rt_redirect(pi, pip->ip_src, pip_err->ip_dest, pic->_4bytes);
            UPDATE_INFO(pi, icmp_redir, 1)
            INCR_SNMP(IcmpInRedirects)
            break;

        /* NOTE: RFC1122 says a host should not implement info request         */
        /*       and info reply since RARP and BOOTP provide a better          */
        /*       mechanism for discovering it own IP address                   */
        /* NOTE: RFC1122 says mask requests (option 3) may be implemented but  */
        /*       since we require the mask to be defined during system         */
        /*       initialization by xn_set_ip (option 2), it is not implemented */
        /* NOTE: RFC1122 says a system must not send address mask replys       */
        /*       unless it is an authoritative agent for address masks         */
        /* NOTE: timestamp not implemented                                     */
        case ICMP_T_TIME_REQUEST:
            INCR_SNMP(IcmpInTimestamps)
            goto not_support;
        case ICMP_T_TIME_REPLY:
            INCR_SNMP(IcmpInTimestampReps)
            goto not_support;
        case ICMP_T_MASK_REQUEST:
            INCR_SNMP(IcmpInAddrMasks)
            goto not_support;
        case ICMP_T_MASK_REPLY:
            INCR_SNMP(IcmpInAddrMaskReps)
            goto not_support;
        case ICMP_T_INFO_REQUEST:
        case ICMP_T_INFO_REPLY:
            goto not_support;
    }

not_support:
        UPDATE_INFO(pi, icmp_not_handled, 1)

discard_it:
        INCR_SNMP(IcmpInErrors)
        os_free_packet(msg);        /* Free the input packet */
        return;
}


#if (INCLUDE_ICMP)
void process_icmp_other(PIFACE pi, DCU msg) /*__fn__*/
{
PICMPPKT pic;
RTIP_BOOLEAN  hard_error;

    pic = DCUTOICMPPKT(msg);

    hard_error = FALSE;

    switch (pic->icmp_type)
    {
        case ICMP_T_SOURCE_QUENCH:
            report_icmp_error(pi, msg, ICMP_QUENCH, hard_error);
            UPDATE_INFO(pi, icmp_quench, 1)
            INCR_SNMP(IcmpInSrcQuenchs)
            break;

        case ICMP_T_TIME_EXCEEDED:
            report_icmp_error(pi, msg, ICMP_TIME_EXCEEDED, hard_error);
            UPDATE_INFO(pi, icmp_time_exceeded, 1)
            INCR_SNMP(IcmpInTimeExcds)
            break;

        case ICMP_T_PARAM_PROBLEM:
            report_icmp_error(pi, msg, ICMP_PARAM_PROBLEM, hard_error);
            UPDATE_INFO(pi, icmp_param_problem, 1)
            INCR_SNMP(IcmpInParmProbs)
            break;

        case ICMP_T_DEST_UNREACH:
            /* NOTE: RFC1122 specifies error 6-12 but not whether they         */
            /*       are hard or soft error - tbd assume they are soft for now */
            if ( ((pic->icmp_code  >= ICMP_C_PROTUR) &&  
                  (pic->icmp_code  <= ICMP_C_FRAG))   ||
                  (pic->icmp_code >= ICMP_C_NETUK) )
                hard_error = TRUE;
            report_icmp_error(pi, msg, ICMP_UNREACH, hard_error);
            UPDATE_INFO(pi, icmp_unreach, 1)
            INCR_SNMP(IcmpInDestUnreachs)
            break;

    }
}

/* ********************************************************************   */
/* REPORT AN ERROR VIA ICMP                                               */
/* ********************************************************************   */
/* switch IP and port addresses (source vs dest) in pip                   */
void switch_port_and_ip_addr(DCU msg, PIPPKT pip)    /*__fn__*/
{
byte temp_ip[4];
word temp_port;
PTCPPKT ptcp;
PUDPPKT pudp;

    /* swap IP addresses   */
    tc_mv4(temp_ip, pip->ip_dest, 4);  
    tc_mv4(pip->ip_dest, pip->ip_src, 4);  
    tc_mv4(pip->ip_src, temp_ip, 4); 

    /* swap port #s   */
    if (pip->ip_proto == PROTTCP)
    {
        ptcp = DCUTOTCPPKT(msg);
        temp_port = ptcp->tcp_dest;
        ptcp->tcp_dest = ptcp->tcp_source;     
        ptcp->tcp_source = temp_port;
    }

    else if (pip->ip_proto == PROTUDP)
    {
        pudp = DCUTOUDPPKT(msg);
        temp_port = pudp->udp_dest;
        pudp->udp_dest = pudp->udp_source;     
        pudp->udp_source = temp_port;
    }
}

/* ********************************************************************   */
/* check_send_icmp() - check if should send ICMP message                  */
/*                                                                        */
/*   Check incoming message (msg) and see if it is legal to send an       */
/*   ICMP message in response to the message; i.e. it is illegal to       */
/*   send an ICMP message in response to an ICMP message and it is        */
/*   illegal to send an ICMP message in response to a fragment which is   */
/*   not the first fragment in the message.                               */
/*                                                                        */
/*   Returns TRUE if it is legal to send an ICMP message.  FALSE if it is */
/*   not.                                                                 */

RTIP_BOOLEAN check_send_icmp(DCU msg)   /*__fn__*/
{
PIPPKT  pip;
int     fragoff;

    pip = DCUTOIPPKT(msg);

    /* do no send ICMP error message in response to an ICMP message   */
    if (pip->ip_proto == PROTICMP)
        return(FALSE);

    /* do no send ICMP error message in response to non-initial fragment   */
    if (pip->ip_fragoff & IP_HASFRAG_68K)
    {
        fragoff = net2hs(pip->ip_fragoff) & IP_FRAGOFF;
        fragoff <<= 3;
        if (fragoff != 0)
            return(FALSE);
    }
    return(TRUE);
}

/* ********************************************************************   */
/* report_icmp_error() - report the icmp error to the application         */
/*                                                                        */
/* NOTE: the reason is translated to the codes expected by                */
/*       ERROR_REPORT, i.e. ICMP_T_PARAM_PROBLEM (code in icmp            */
/*       message: reason is set to ICMP_PARAM_PROBLEM                     */
/*                                                                        */
/* side effect: switches the IP address and port number in pic (in data   */
/*              portion of ICMP packet)                                   */
void report_icmp_error(PIFACE pi, DCU msg, int reason, int hard_error)  /*__fn__*/
{
PIPPKT   pip;
#if (INCLUDE_TCP)
PTCPPORT tport;
#endif
#if (INCLUDE_UDP)
PUDPPORT port;
PUDPPORT listener_port;
#endif
int      subreason;
PICMPPKT pic;
int     header_len_bytes;

#if (!INCLUDE_TCP)
    ARGSUSED_INT(hard_error)
#endif

#if (!INCLUDE_UDP)
    ARGSUSED_PVOID(pi)
#endif

    pic = DCUTOICMPPKT(msg);

    subreason = pic->icmp_code;

    /* point to the IP packet which caused the ICMP msg to be sent     */
    /* NOTE: point past the IP header of the origional msg plus there  */
    /*       are 4 extra words (8 bytes) for the ICMP header so        */
    /*       point 4 words down                                        */
    pip = (PIPPKT)((PFBYTE)pic + sizeof(ICMPPKT));

    /* reset up pointers to IP and PROTOCOL headers   */
    header_len_bytes = (pip->ip_verlen&0x0f) << 2;
    DCUTOPACKET(msg)->inet.ptr += (sizeof(ICMPPKT) + header_len_bytes);
    DCUTOPACKET(msg)->prot.ptr += (sizeof(ICMPPKT) + header_len_bytes);

    /* switch addresses in messages for find_port - this is needed since   */
    /* find port works on incoming msgs and the msg we are looking at      */
    /* which is in the data portion of the ICMP msg is really the outgoing */
    /* msg we sent                                                         */
    switch_port_and_ip_addr(msg, pip);

    switch (pip->ip_proto)
    {
#if (INCLUDE_UDP)
        case PROTUDP:
            udp_find_port(pi, &port, &listener_port, msg);
            if (!port)
                port = listener_port;

            if (port)
            {
                CB_ERROR_REPORT(port->ap.ctrl.index, reason, subreason);
            }

            /* if cannot find port maybe it is already closed, report it   */
            /* to the application with -1 for the socket number            */
            else
            {
                CB_ERROR_REPORT(-1, reason, subreason);
            }
            break;
#endif

#if (INCLUDE_TCP)
        case PROTTCP:
            tport = tc_find_port(msg);
            if (tport)
            {
                if (!hard_error)
                {
                    CB_ERROR_REPORT(tport->ap.ctrl.index, reason, subreason);
                }
                else if (reason == ICMP_QUENCH)
                {
#if (INCLUDE_CWND_SLOW)
                    tport->cwnd = tport->out.mss;   /* perform slow start */
#endif
                }
                else if (reason == ICMP_UNREACH)
                {
                    tc_tcp_abort_connection(tport, 
                        READ_WAKEUP_FAIL|WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP,
                        EDESTUNREACH, NOT_FROM_INTERPRET);
                }
            }

            /* if cannot find port maybe it is already closed, report it   */
            /* to the application with -1 for the socket number            */
            else
            {
                CB_ERROR_REPORT(-1, reason, subreason);
            }

            break;
#endif
    }
}


/* ********************************************************************   */
/* SEND ICMP                                                              */
/* ********************************************************************   */
/* tc_icmp_send() - set up and send a ICMP packet                         */
/*                                                                        */
/*   Transmits a ICMP packet.                                             */
/*    - sets IP ident,check,total len                                     */
/*   The parameter ptr is used by PARAMETER PROBLEM only; it contains     */
/*   the value of the pointer the field which caused the problem in the   */
/*   origional message.                                                   */
/*                                                                        */
/*   Called for all ICMP messages except PING.                            */
/*                                                                        */
/*   Returns 0 if successful, errno if not                                */
/*                                                                        */
int tc_icmp_send(DCU msg, byte type, byte ccode, byte ptr)      /*__fn__*/
{
DCU      new_msg;   /* get output packet */
PIFACE   pi;        /* output interface */
PIPPKT   pip_new;
PIPPKT   pip;
word     dlen;
word     plen;
#if (INCLUDE_802_2)
RTIP_BOOLEAN  is_802_2;
#endif
#if (RTIP_VERSION >= 26)
int      header_len;        /* length of ETHER header + IP header */
#endif

    pip = DCUTOIPPKT(msg);  /* message which caused the problem */
    INCR_SNMP(IcmpOutMsgs)
    INCR_SNMP(IpOutRequests)

    /* get interface to send packet to (routing table look-up)   */
    pi = msg->ctrl.pi;
    if (!pi)
    {
        DEBUG_ERROR("tc_icmp_send - pi not set up in DCU: ip addr = ", 
            IPADDR, pip->ip_src, 0);
        INCR_SNMP(IpOutNoRoutes)
        os_free_packet(msg);        /* Free the input packet */
        return(EADDRNOTAVAIL);
    }

    /* Calculate the size of the packet we will need   */
#if (INCLUDE_802_2)
    is_802_2 = FALSE;
    if (pi->iface_flags & IS_802_2)
    {
        is_802_2 = TRUE;
    }
#endif

#if (RTIP_VERSION >= 26)
    TOTAL_LL_HLEN_SIZE(header_len, is_802_2, pi, msg)
    header_len += IP_HLEN(pip);

    plen = (word)(header_len + ICMP_HLEN_BYTES + ORIG_DATA_SEND);
#else
    plen = (word)(ICMP_TLEN_BYTES + IP_HLEN(pip) + ORIG_DATA_SEND);
#endif

    /* **************************************************         */
    /* get DCU to use                                             */
    /* After it is sent out on the wire it will be deallocated by */
    /* tc_release_message since it will be set to NO_KEEP         */
    if ( (new_msg = ip_alloc_packet(pi, (PANYPORT)0, plen, 0, ICMP_ALLOC)) == 
         (DCU)0 )
    {
        INCR_SNMP(IcmpOutErrors)
        DEBUG_ERROR("ERROR(rtip) - ip_alloc_packet failed", NOVAR, 0, 0);
        return(ENOPKTS);
    }

    /* copy the ETHER and IP headers to the new msg since they will be used   */
    /* to setup fields for output message                                     */
    /* NOTE: they will be switched by setup_and_send_icmp                     */
#if (RTIP_VERSION >= 26)
    tc_movebytes(DCUTODATA(new_msg), DCUTODATA(msg), header_len);
#else
    tc_movebytes(DCUTODATA(new_msg), DCUTODATA(msg), 
                 ETH_HLEN_BYTES + IP_HLEN(pip));
#endif

    /* save interface bad packet came in on   */
    new_msg->ctrl.pi = msg->ctrl.pi;

    pip_new = DCUTOIPPKT(new_msg);
#if (INCLUDE_FRAG)
    DCUTOPACKET(new_msg)->frag_next = (DCU)0;
#endif

    dlen = 0;

    /* **************************************************   */
    tc_memset(&(DCUTOICMPPKT(new_msg)->_4bytes[0]), 0, 4); 
    switch (type)
    {
        /* NOTE: redirect is sent by gateway only                    */
        /* NOTE: echo request done by xn_ping(), echo reply done by  */
        /*       setup_and_send_icmp()                               */
        case ICMP_T_PARAM_PROBLEM:
            DCUTOICMPPKT(new_msg)->_4bytes[0] = ptr;
            INCR_SNMP(IcmpOutParmProbs)

            /* NOTE: fall thru   */

        case ICMP_T_TIME_EXCEEDED:
#            if (INCLUDE_SNMP)
                if (type == ICMP_T_TIME_EXCEEDED)
                    INCR_SNMP(IcmpOutTimeExcds)
#            endif

            /* NOTE: fall thru   */

        case ICMP_T_DEST_UNREACH:
#            if (INCLUDE_SNMP)
                if (type == ICMP_T_DEST_UNREACH)
                    INCR_SNMP(IcmpOutDestUnreachs)
#            endif

            /* NOTE: fall thru   */

        case ICMP_T_SOURCE_QUENCH:
#            if (INCLUDE_SNMP)
                if (type == ICMP_T_SOURCE_QUENCH)
                    INCR_SNMP(IcmpOutSrcQuenchs)
#            endif

            /* do not send ICMP to in response to a packet destined   */
            /* to a multicast address                                 */
            /* check for dest unread, param problem, quence, redirect */
            if (IP_ISMCAST_ADDR(pip->ip_dest[0]))
            {
                os_free_packet(new_msg);
                return(0);
            }

            dlen = (word)(IP_HLEN(pip) + ORIG_DATA_SEND);

            /* copy packet info (IP header + 64 bytes of data) from pkt    */
            /* received to new packet                                      */
#if (RTIP_VERSION >= 26)
            tc_movebytes(DCUTODATA(new_msg) + header_len + ICMP_HLEN_BYTES,
                         (PFBYTE)(&pip->ip_verlen), dlen);
#else
            tc_movebytes(DCUTODATA(new_msg) + ICMP_TLEN_BYTES,
                         (PFBYTE)(&pip->ip_verlen), dlen);
#endif
            break;

        case ICMP_T_REDIRECT:
            INCR_SNMP(IcmpOutRedirects)

            /* do not send ICMP to in response to a packet destined   */
            /* to a multicast address                                 */
            /* check for dest unread, param problem, quence, redirect */
            if (IP_ISMCAST_ADDR(pip->ip_dest[0]))
            {
                os_free_packet(new_msg);
                return(0);
            }
            break;

        case ICMP_T_TIME_REQUEST:
            INCR_SNMP(IcmpOutTimestamps)
            break;

        case ICMP_T_TIME_REPLY: 
            INCR_SNMP(IcmpOutTimestampReps)
            break;

        case ICMP_T_MASK_REQUEST:     
            INCR_SNMP(IcmpOutAddrMasks)
            break;

        case ICMP_T_MASK_REPLY:       
            INCR_SNMP(IcmpOutAddrMaskReps)
            break;
    }

    /* **************************************************              */
    /* set up some IP header fields not setup by setup_and_send_icmp() */
    IP_LEN_DCU(new_msg) = (byte)(ICMP_TLEN_BYTES - ETH_HLEN_BYTES + dlen);
    pip_new->ip_fragoff= 0;

#if (RTIP_VERSION >= 26)
    DCUTOPACKET(new_msg)->length = header_len + ICMP_HLEN_BYTES + dlen;  
                        /* pass msg len to device driver   */
#else
    DCUTOPACKET(new_msg)->length = ICMP_TLEN_BYTES + dlen;  
                        /* pass msg len to device driver   */
#endif

    /* Send the packet   */
    return(setup_and_send_icmp(new_msg, dlen+ICMP_HLEN_BYTES, 
                               type, ccode));
}

#endif

/* ********************************************************************   */
/* setup_and_send_icmp - sets up and sends an ICMP message                */
/*                                                                        */
/*   Sets up header IP header and sends ICMP message.  Called for         */
/*   all ICMP messages including PING.  The packet is freed.              */
/*                                                                        */
/*   NOTE: it is assumed fragmentation fields and IP len in IP header     */
/*         are already setup                                              */
/*                                                                        */
/*   Returns 0 upon success or errno upon failure                         */

static int setup_and_send_icmp(DCU msg, int len, byte type, byte ccode)    /*__fn__*/
{
PICMPPKT pic;
PIPPKT   pip;
PIFACE   pi;
byte     send_to_addr[IP_ALEN]; /* could be actual ip dest or gateway ip dest */
DCU      head_msg;
byte     temp_ip[IP_ALEN];
word     save_ip_id = 0;
#if (INCLUDE_802_2)
RTIP_BOOLEAN     is_802_2;
#endif
    /* keep first msg since will use msg to loop thru the fragments;   */
    /* will pass head_msg to tc_netwrite()                             */
    head_msg = msg;

    pic = DCUTOICMPPKT(msg);
    pip = DCUTOIPPKT(msg);

    pi = msg->ctrl.pi;
    if (!pi)
    {
        DEBUG_ERROR("setup_and_send_icmp - msg not set up in DCU", NOVAR, 0, 0);
        INCR_SNMP(IpOutNoRoutes)
        os_free_packet(msg);        /* Free the input packet */
        return(EADDRNOTAVAIL);
    }

    /* never send an ICMP in response to a broadcast   */
    if (IS_ETH_ADDR_EQUAL(LL_DEST_ADDR(msg, pi), broadaddr))
    {
        os_free_packet(msg);        /* Free the input packet */
        return(0);
    }

    /* Check if it is a net-directed broadcast. If so do not   */
    /* send it                                                 */
    if (is_net_broadcast(pi, send_to_addr))
    {
        os_free_packet(msg);        /* Free the input packet */
        return(0);
    }

    /* get interface to send packet to (routing table look-up)   */
    pi = rt_get_iface((PFBYTE)pip->ip_src, (PFBYTE)send_to_addr, 
                      (PANYPORT)0, (RTIP_BOOLEAN *)0);
    if (!pi)
    {
        DEBUG_ERROR("setup_and_send_icmp - rt_get_iface failed: ip addr = ", 
            IPADDR, pip->ip_src, 0);
        INCR_SNMP(IpOutNoRoutes)
        os_free_packet(msg);        /* Free the input packet */
        return(EADDRNOTAVAIL);
    }

    /* Make it a reply   */
    pic->icmp_type = type;
    pic->icmp_code = ccode;

    /* loop thru the fragments   */
#if (INCLUDE_FRAG)
    while (msg)
    {
#endif
        /* for echo reply, send the same options back in the reply;   */
        /* for all others do not send options                         */
        if (type != ICMP_T_ECHO_REPLY)
            DCUTOPACKET(msg)->ip_option_len = 0;

#if (INCLUDE_802_2)
        is_802_2 = msg_is_802_2(msg);
#endif

        /* Reverse the Link Layer header                        */
        /* NOTE: dest will get set by arp; source based upon pi */
        /*       sending out over                               */
        SETUP_LL_HDR(msg, pi, EIP_68K, is_802_2,
                     DCUTOPACKET(msg)->length - LL_HLEN_BYTES);

#if (INCLUDE_IPV4)
        /* Reverse the ip packet                                          */
        /* NOTE: source IP is set based upon interface sending out on     */
        /*       for all but ECHO REPLY and TIME REPLY which set source   */
        /*       IP based upon the request which came in on; this is      */
        /*       done since the origionator of ECHO/TIME request verifies */
        /*       the response by comparing destination IP address of the  */
        /*       request with the source IP address of the response       */
        pip = DCUTOIPPKT(msg);
        tc_mv4(temp_ip, pip->ip_dest, IP_ALEN);
        tc_mv4(pip->ip_dest, pip->ip_src, IP_ALEN);  /* machine it came from  */
        if ( (type == ICMP_T_ECHO_REPLY) || (type == ICMP_T_TIME_REPLY) )
            tc_mv4(pip->ip_src, temp_ip, IP_ALEN); 
        else
            tc_mv4(pip->ip_src, pi->addr.my_ip_addr, IP_ALEN); 

        /* setup misc stuff in IP and ethernet header                       */
        /* NOTE: the first time setup_ipv4_header is called, frag offset is */
        /*       0 so it will set up ip_id; after that we want to use       */
        /*       the same value so we save it and set it up in the packet;  */
        /*       for these subsequent packets the ip_id will not be set     */
        /*       up for setup_ipv4_header since frag offset is not 0        */
        /* NOTE: fragoff is already setup in packet                         */
        /* NOTE: don't pass a port so it will not set up and IP options     */
        pip->ip_proto = PROTICMP;
        pip->ip_id = save_ip_id;
        setup_ipv4_header(msg, (PANYPORT)0, IP_LEN_DCU(msg),
                          (word)(net2hs((word)(pip->ip_fragoff & IP_FRAGOFF_68K)) << 3),
                          (RTIP_BOOLEAN)(pip->ip_fragoff & IP_MOREFRAG_68K), 0,
                          TRUE);    
        save_ip_id = pip->ip_id;

#endif

#if (INCLUDE_FRAG)
        /* get next msg for next loop   */
        msg = DCUTOPACKET(msg)->frag_next;
    }
#endif

    /* prepare ICMP checksum    */
    pic->icmp_chk = 0;
    pic->icmp_chk = ipf_icmp_chksum(head_msg, len);

#if (!INCLUDE_FRAG)
     /* Call netwrite to send it through - netwrite will free packet   */
     /* Note: the packet length is unchanged so we don't set the       */
     /* length field in the message                                    */
     return(tc_netwrite(pi, (PANYPORT)0, head_msg, (PFBYTE)send_to_addr,
                        NO_DCU_FLAGS, 0));
#else
     return(ipf_netwrite(pi, (PANYPORT)0, head_msg, (PFBYTE)send_to_addr,
                         NO_DCU_FLAGS, 0));
#endif
}

