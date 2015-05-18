/*                                                                        */
/* IP.C - IP, RARP and ARP interpreter                                    */
/*                                                                        */
/* EBS - RTIP                                                             */
/*                                                                        */
/* Copyright Peter Van Oudenaren , 1993                                   */
/* All rights reserved.                                                   */
/* This code may not be redistributed in source or linkable object form   */
/* without the consent of its author.                                     */
/*                                                                        */
/*  Module description:                                                   */
/*     This module contains code to handle the IP layer.  It includes     */
/*     code to interpret and dispatch incoming IP packets, to             */
/*     handle IP fragmentation/packet reassembly and to perform           */
/*     call router module to perform simple router capablilities.         */
/*     NOTE: fragmentation and router code inclusion is controlled by     */
/*           INCLUDE_FRAG and INCLUDE_ROUTER                              */
/*                                                                        */
/* Routines in this module                                                */
/*                                                                        */
/* tc_ip_interpret()     - Take incoming ip packets and dispatch them to  */
/*                         the appropriate handlers. (IP, UDP, ICMP)      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_IP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_FRAG  0
#define DISPLAY_INPUT_PKT   0
#define DEBUG_IP_INTERPRET  0
#define DEBUG_IN_PKTS       0
#define DEBUG_OUT_PKTS      0

/* ********************************************************************   */
/* CONTROLS PROCESSING                                                    */
/* ********************************************************************   */
#define TCP_WAIT_SEND 0     /* TCP: processes all input before sending */
                            /*      an output                        */
                            /* NOTE: must match same define in tcp.c */

/* ********************************************************************   */
/* local routines                                                         */
#if (INCLUDE_802_2)
word process_llc_snap_802_2(DCU msg);
#endif
#if (INCLUDE_IPV4)
void tc_ipv4_interpret(PIFACE pi, DCU msg);
#endif
#if (INCLUDE_FRAG)
DCU ipf_check_complete(int table_entry);
#endif

#if (INCLUDE_ROUTER)
void route_pkg(PIFACE pi, DCU msg, PIPPKT pip);
#if (FRAG_ROUTER && INCLUDE_FRAG)
void tc_send_router_frags(PIFACE pi, DCU orig_msg, PFBYTE send_ip_addr);
#endif
#endif

#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
int ip_options_format(PANYPORT port, DCU msg, int frag_off);
#endif

void process_input_packet(PIFACE pi, DCU msg);

#if (INCLUDE_802_11)
void process_frame_mgmt(PFRAME_FORMAT_802_11 p802_11);
void process_ctrl_mgmt(PFRAME_FORMAT_802_11 p802_11);
RTIP_BOOLEAN process_ctrl_data(PFRAME_FORMAT_802_11 p802_11);
#endif      /* INCLUDE_802_11 */

/* ********************************************************************      */
/* INTERPRET IP PACKETS                                                      */
/* ********************************************************************      */
/* tc_ip_interpret() - interpret incoming IP packet                          */
/*                                                                           */
/* This task takes a data packet from the os specific                        */
/* queuing layer and branches to ARP/RARP handlers or processes it as        */
/* an IP packet. If the packet is IP it performs a checksum and dispatches   */
/* the packet to either the UDP, TCP or ICMP layers. The TCP layer is called */
/* using a message exchange, the others are called directly.                 */
/*                                                                           */

void tc_ip_interpret(PIFACE pi)                                  /*__fn__*/
{
DCU     msg;
RTIP_BOOLEAN    processed_output;
RTIP_BOOLEAN    processed_dcu;
#if (TCP_WAIT_SEND)
RTIP_BOOLEAN processed_one_dcu = FALSE;
#endif

#if (DEBUG_IP_INTERPRET)
    DEBUG_ERROR("ip_interpret entered", NOVAR, 0, 0);
#endif

#if (DEBUG_SEND_NO_BLOCK)
    DEBUG_ERROR("ip_interpret entered", NOVAR, 0, 0);
#endif /* DEBUG_SEND_NO_BLOCK */

    /* **************************************************   */
    /* Get the interface                                    */
    if (!pi || !pi->open_count)
    {
        /* This won't happen unless interface is closed   */
        INCR_SNMP(IpInHdrErrors)
        return;
    }

#    if (DEBUG_SEND_NO_BLOCK)
        DEBUG_ERROR("tc_ip_interpret entered: sig cnt, xmit_done = ", DINT1, 
            pi->xmit_done);
#    endif

    processed_output = TRUE;
    processed_dcu = TRUE;
    while (processed_dcu || processed_output)
    {
        /* **************************************************   */
        /* PROCESS OUTPUT                                       */
        /* **************************************************   */
        processed_output = check_process_output(pi);

        /* **************************************************   */
        /* PROCESS IP PACKET                                    */
        /* **************************************************   */
        KS_TL_DISABLE()         /* Disable interrupts  */
        msg = (DCU) pi->ctrl.exch_list[0];
        if (msg)
        {
            pi->ctrl.exch_list[0] = 
                os_list_remove_off(pi->ctrl.exch_list[0],
                                            (POS_LIST)msg, 
                                            PACKET_OFFSET);
#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = OUT_IP_LIST;
#endif
            KS_TL_ENABLE()          /* enable interrupts */
            processed_dcu = TRUE;
#if (TCP_WAIT_SEND)
            processed_one_dcu = TRUE;
#endif
        }
        else
        {
#        if (DEBUG_SEND_NO_BLOCK)
            DEBUG_ERROR("tc_ip_interpret: OOPPS - IP signal but no msg etc",
                NOVAR, 0, 0);
#        endif
            KS_TL_ENABLE()          /* enable interrupts */
            processed_dcu = FALSE;
            continue;
        }

#if (INCLUDE_MODEM)
        /* **************************************************           */
        /* if this DCU specified to call callback                       */
        /* (i.e. call cb_rs232_connection_drop)                         */
        /* NOTE: this was a special packet queued by xn_interface_close */
        /*       so that closing the interface can be done in sync with */
        /*       the output queue; i.e. so the interface is not         */
        /*       closed while there are packets on the output queue     */
        if (DCUTOCONTROL(msg).dcu_flags & MODEM_DROP) 
        {
            OS_RELEASE_IFACE(pi)
        
            /* callback to application   */
            DEBUG_ERROR("tc_ip_interpret: modem drop", NOVAR, 0, 0);
            CB_RS232_CONNECTION_LOST(pi->ctrl.index);

            OS_CLAIM_IFACE(pi, IP_SEND_CLAIM_IFACE)
        }

        /* **************************************************   */
        else
#    endif      /* INCLUDE_MODEM */
        {
            process_input_packet(pi, msg);
        }
    }       /* end of while loop */

    /* **************************************************   */
#if (INCLUDE_TCP && TCP_WAIT_SEND)
    /* ******                                                             */
    /* if there are no packets on the input exchange then process any     */
    /* TCP outputs, i.e. interpret will not output any TCP packets        */
    /* until all the queued input packets have been processed; this       */
    /* is done as per RFC1122 in order to cut down on the network traffic */

    if (processed_one_dcu)
    {
        tc_tcp_output();
    }
#endif
}

/* ********************************************************************   */
void process_input_packet(PIFACE pi, DCU msg)
{
word    type;
PETHER  pe;
#if (INCLUDE_802_2)
RTIP_BOOLEAN is_802_2;
#endif
#if (INCLUDE_802_11)
PFRAME_FORMAT_802_11 p802_11;
#endif

#if (DEBUG_IP_INTERPRET || DEBUG_IN_PKTS)
    DEBUG_ERROR("ip_interpret: process input packet: msg = ", DINT1, msg, 0);
#endif

#if (DISPLAY_INPUT_PKT)
DEBUG_ERROR("MSG IN: ", PKT, DCUTODATA(msg), DCUTOPACKET(msg)->length);
DEBUG_ERROR("MSG IN: len = ", EBS_INT1, DCUTOPACKET(msg)->length, 0);
#endif

#    if (DEBUG_SEND_NO_BLOCK)
    DEBUG_ERROR("tc_ip_interpret: process IP msg: interpret msg,data = ", DINT2, 
        msg, DCUTODATA(msg));
#    endif

    INCR_SNMP(IpInReceives)

    /* save interface packet came in on   */
    DCUTOCONTROL(msg).pi = pi;  

    /* *************************************************   */
    /* set up pointer to IP, ARP or RARP header and        */
    /* get packet type (IP, ARP or RARP)                   */
#if (INCLUDE_802_11)
    if (pi->pdev->iface_type == IEEE_802_11_IFACE)
    {
        p802_11 = DCUTO802_11(msg);
        switch (p802_11->frame_control & FRAME_TYPE)
        {
        case FRAME_T_MGMT:
            process_frame_mgmt(p802_11);
            return;
        case FRAME_T_CTRL:
            process_ctrl_mgmt(p802_11);
            return;
        case FRAME_T_DATA:
            if (!process_ctrl_data(p802_11))
                return;
            break;
        }   /* end of switch */
    }
#endif      /* INCLUDE_802_11 */

    {
        /* NOT TOKEN RING   */
        pe = DCUTOETHERPKT(msg);
        type = pe->eth_type;
#if (INCLUDE_802_2)
        is_802_2 = msg_is_802_2(msg);
        DCUSETUPIPARP(msg, is_802_2)
        if (is_802_2)
            type = process_llc_snap_802_2(msg);
#else
        DCUSETUPIPARP(msg, FALSE)
#endif
    }

    /* *************************************************       */
    /* mc68000 byte order versions of ethernet packet types    */
    switch (type) 
    {
        /* PROCESS ARP PACKETS   */
        case EARP_68K:
#            if (DEBUG_IP_INTERPRET)
                DEBUG_ERROR("ip_interpret: ARP packet", NOVAR, 0, 0);
#            endif
            UPDATE_INFO(pi, rcv_arp_packets, 1)
#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = IN_ARP_LAYER;
#endif
            tc_arp_interpret(pi, msg);
            break;

        /* PROCESS RARP PACKETS   */
        case ERARP_68K:
#            if (DEBUG_IP_INTERPRET)
                DEBUG_ERROR("ip_interpret: RARP packet", NOVAR, 0, 0);
#            endif

            UPDATE_INFO(pi, rcv_rarp_packets, 1)
#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = IN_RARP_LAYER;
#endif
            tc_arp_interpret(pi, msg);
            break;

#if (INCLUDE_PPPOE)
        /* PROCESS PPPOE PACKETS   */
        case PPPOED_68K:    /* PPPOE Discovery */
#            if (DEBUG_IP_INTERPRET)
                DEBUG_ERROR("ip_interpret: PPPOED packet", NOVAR, 0, 0);
#            endif
#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = IN_PPPOED_LAYER;
#endif
            tc_pppoed_interpret(pi, msg);
            break;

        case PPPOES_68K:    /* PPPOE Session */
#            if (DEBUG_IP_INTERPRET)
                DEBUG_ERROR("ip_interpret: PPPOES packet", NOVAR, 0, 0);
#            endif
#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = IN_PPPOES_LAYER;
#endif
            tc_pppoes_interpret(pi, msg);
            break;
#endif      /* INCLUDE_PPPOE */

        /* PROCESS IP PACKETS   */
#if (INCLUDE_IPV4)
        case EIP_68K:
#            if (DEBUG_IP_INTERPRET || DEBUG_SEND_NO_BLOCK)
                DEBUG_ERROR("ip_interpret: IP packet", NOVAR, 0, 0);
#            endif
            DEBUG_LOG("tc_ip_interpret - IP", LEVEL_3, NOVAR, 0, 0);

            UPDATE_INFO(pi, rcv_ip_packets, 1)
            tc_ipv4_interpret(pi, msg);
            break;
#endif              /* INCLUDE_IPV4 */

        default:
            /* PROCESS UNKNOWN PROTOCOL PACKETS (NOT RARP, BOOTP OR IP)   */
#            if (DEBUG_IP_INTERPRET || DEBUG_SEND_NO_BLOCK)
                DEBUG_ERROR("ip_interpret: not ARP, RARP or IP packet", NOVAR, 0, 0);
#            endif
#if (INCLUDE_ICMP)
/*          if (check_send_icmp(msg))                                     */
/*          {                                                             */
/*              tc_icmp_send(msg, ICMP_T_DEST_UNREACH, ICMP_C_PROTUR, 0); */
/*          }                                                             */
#endif
            DEBUG_LOG("tc_ip_interpret - OOPS unk packet, ether type =", 
                LEVEL_3, EBS_INT1, type, 0);
            UPDATE_INFO(pi, unk_packets, 1)
            INCR_SNMP(IpInUnknownProtos)
            os_free_packet(msg);                /* Free the input message */
            break;
    }       /* switch ether type */
}


#if (INCLUDE_802_2)
/* ********************************************************************   */
/* returns type if it is an 802.2, i.e. LLC and SNAP headers are valid    */
/* returns 0 if not 802.2 packet                                          */
word process_llc_snap_802_2(DCU msg)
{
PLLCPKT  pllc;
PSNAP psnap;

    pllc = DCUTOLLC8022(msg);

    /* first verify LLC and SNAP headers   */
    if (tc_comparen(pllc, llc_snap_data, LLC_SNAP_DATA_LEN))
    {
        psnap = DCUTOSNAP8022(msg);
        return(psnap->snap_type);
    }
    return(0);
}

/* ********************************************************************   */
RTIP_BOOLEAN msg_is_802_2(DCU msg)
{
PETHER pe;

    pe = DCUTOETHERPKT(msg);

    switch (pe->eth_type)
    {
        case EARP_68K:
        case ERARP_68K:
#if (INCLUDE_IPV4)
        case EIP_68K:
#endif      
            return(FALSE);
    }
    return(TRUE);
}

/* ********************************************************************   */
/* setup_802_2 - convert ethernet packet to 802.2 ethernet packet         */
/*                                                                        */
/*   Converts an ethernet packet to 802.2 ethernet packet.                */
/*   This is done by setting up the LLC and SNAP headers after the        */
/*   ethernet header, writing the len into the eth_type field of the      */
/*   ethernet header and copying the ethernet type from the origional     */
/*   ethernet packet to the type field in the SNAP header.                */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */
void setup_802_2_header(DCU msg, word len)
{
PSNAP     psnap;
PLLCPKT   pllc;
PETHER    pe;
word      save_eth_type;

    pe = DCUTOETHERPKT(msg);
    pllc = DCUTOLLC8022(msg);

    /* the ethernet type is really the length of packet not including   */
    /* CRC which follows the ETHERNET header                            */
    save_eth_type = pe->eth_type;
    pe->eth_type = hs2net(len);
    tc_movebytes(pllc, llc_snap_data, LLC_SNAP_DATA_LEN);
    psnap = DCUTOSNAP8022(msg);
    psnap->snap_type = save_eth_type;   /* EIP_68K or EARP_68K etc */
}

#endif /* INCLUDE_802_2 */


#if (INCLUDE_IPV4)
/* ********************************************************************   */
/* INTERPRET IPV4 PACKETS                                                 */
/* ********************************************************************   */
void tc_ipv4_interpret(PIFACE pi, DCU msg)
{
PIPPKT  pip;
PFBYTE  ipn;
int     header_len_words;
int     header_len_bytes;
#if (INCLUDE_RAW)
RTIP_BOOLEAN found_raw;
#endif
#if (INCLUDE_KEEP_STATS)
RTIP_BOOLEAN is_net_bc;
#endif

    /* **************************************************   */
    DEBUG_LOG("ip_interpret - INPUT PKT", LEVEL_3, NOVAR, 0, 0);
    DEBUG_LOG("PACKET = ", LEVEL_3, PKT, (PFBYTE)DCUTOETHERPKT(msg), 40);

    /* **************************************************   */
    pip = DCUTOIPPKT(msg);

    /* get number of words in header; value in packet is number   */
    /* of 4 bytes words (i.e. number of dwords) but we want       */
    /* number of 2 byte words                                     */
    header_len_words = IP_HLEN_WORDS(pip);
    header_len_bytes = header_len_words << 1;

    DCUTOPACKET(msg)->ip_option_len = header_len_bytes - IP_HLEN_BYTES;

    /* setup PROTOCOL header; must be done after ip_option_len is setup   */
    DCUSETUPPROT(msg);

    /* save info from packet in DCU   */
    IP_HLEN_DCU(msg) = (byte)(IP_HLEN(pip));
    IP_LEN_DCU(msg)  = net2hs(pip->ip_len);

    /* **************************************************               */
    /* filter the packet if it is multicast; some drivers cannot be     */
    /* set up to filter or they have a limit on the number of addresses */
    /* which can be filtered; when this limit is reached the driver     */
    /* is configured to accept all multicast packets                    */
    ipn = pip->ip_dest;
    if (IP_ISMCAST_ADDR(ipn[0]) && !is_local_mcast(pi, ipn))
    {
        DEBUG_ERROR("tc_ip_interpret: MULICAST NOT LISTENING TO: iface = ",
            EBS_INT1, pi->ctrl.index, 0);
        /* DEBUG_ERROR("PKT:", PKT, DCUTODATA(msg), DCUTOPACKET(msg)->length);   */
        DEBUG_ERROR("                 IP PROTOCOL:", EBS_INT1, pip->ip_proto, 0);
        DEBUG_ERROR("                 IP dest:", IPADDR, ipn, 0);
        DEBUG_ERROR("                 IP src :", IPADDR, pip->ip_src, 0);
        os_free_packet(msg);        /* Free the input message */
        return;
    }

    /* **************************************************   */
    /* update statistics                                    */
    UPDATE_INFO(pi, rcv_packets, 1)
    UPDATE_INFO(pi, rcv_bytes, DCUTOPACKET(msg)->length)

#if (INCLUDE_KEEP_STATS)
    /* Check if it is a net-directed broadcast. If so use the ethernet    */
    /* broadcast address and send to the specified interface              */
    is_net_bc = is_net_broadcast(pi, ipn);

    if ( ((ipn[0] == ip_broadaddr[0]) &&           /* 0xff */
            tc_cmp4(ipn,  ip_broadaddr, 4)) ||
            IP_ISMCAST_ADDR(ipn[0]) || is_net_bc )
    {
        INCR_INFO(pi, rcv_nucast)
    }
    else
    {
        INCR_INFO(pi, rcv_ucast)
    }
#endif

    /* **************************************************          */
    /* if getting low on packets send quench for this input packet */
#if (INCLUDE_ICMP && !INCLUDE_MALLOC_DCU_AS_NEEDED)
    if ( (current_free_packets <= CFG_PKT_QUENCH) &&
            (check_send_icmp(msg)) )
    {
        tc_icmp_send(msg, ICMP_T_SOURCE_QUENCH, 
                     ICMP_C_SOURCE_QUENCH, 0);
    }
#endif

    /* **************************************************   */
    /* Checksum calculation                                 */
    if ( (pip->ip_cksum) &&
         tc_ip_chksum((PFWORD )&pip->ip_verlen, header_len_words))
    {
        DEBUG_ERROR("PACKET = ", PKT, (PFBYTE)pip, 20);
#if (INCLUDE_ICMP)
        if (check_send_icmp(msg))
        {
            tc_icmp_send(msg, ICMP_T_PARAM_PROBLEM, 
                         ICMP_C_PTR_VALID, ICMP_P_IPCHKSUM);
        }
#endif
        UPDATE_INFO(pi, ip_cksum, 1)
        INCR_SNMP(IpInHdrErrors)
        DEBUG_ERROR("IP Checksum Error-chksum,hdr interface = ", EBS_INT2, 
            tc_ip_chksum((PFWORD)&pip->ip_verlen, pi->ctrl.index), 
            pi->ctrl.index);
        DEBUG_ERROR("                   header size, total len = ",
            EBS_INT2, header_len_words, net2hs(pip->ip_len));
        /* DEBUG_ERROR("PACKET = ", PKT, (PFBYTE)DCUTOETHERPKT(msg), 60);   */
        os_free_packet(msg);        /* Free the input message */
        return;
    }


    /* **************************************************       */
    /* check if packet destined for                             */
    /* - my IP address according to any of the interfaces       */
    /*   interface structure (TCP and UDP find ports will check */
    /*   it against ports IP address bound to)                  */
    /* - UDP limited broadcast (255.255.255.255)                */
    /* - net directed broadcast (netid.255 for class C)         */
    /* - LOOPBACK address                                       */

    /* First check if it is destined for this interface before   */
    /* Incurring overhead of calling tc_get_local_pi()           */
    if (!tc_cmp4(pi->addr.my_ip_addr, pip->ip_dest, 4))
    {
        if (!tc_get_local_pi(pip->ip_dest) &&
            !(pip->ip_proto==PROTUDP   && 
            tc_cmp4(ip_broadaddr, pip->ip_dest, IP_ALEN)) &&
            !(pip->ip_proto==PROTUDP && 
            tc_cmp4(pi->addr.my_net_bc_ip_addr, pip->ip_dest, IP_ALEN)) &&
            !(pip->ip_dest[0] == IP_LOOP_ADDR))
        {
            /* Drop/forward everything else but                       */
            /* - multicast UDP, IGMP                                  */
            /* - ip addresses with dest == 0000 if udp. (for bootp)   */
            /*   i.e. bootp is used to determine its IP address, thus */
            /*   it does not know it yet and puts a IP addr of 0 for  */
            /*   its source IP address                                */
            if ( (!(IP_ISMCAST_ADDR(pip->ip_dest[0]) && 
                    (pip->ip_proto == PROTUDP))) &&
                    (!(pip->ip_proto==PROTUDP && 
                    tc_cmp4(pi->addr.my_ip_addr, ip_nulladdr, IP_ALEN))) &&
                    (!(IP_ISMCAST_ADDR(pip->ip_dest[0]) && 
                    (pip->ip_proto == PROTIGMP))) )
            {
                /* before dropping, if we can perform simple routing   */
                /* forward the packet if possible                      */
                /* NOTE: RTIP IS NOT A "ROUTER"                        */
                /* NOTE: the code will not fall through here if the    */
                /*       msg is limited broadcast, 0xff.0xff.0xff.0xff */
                /*       (i.e. limited broadcast are not forwarded);   */
                /*       the code will fall through to here for net-   */
                /*       directed broadcasts (netid.0xff) which        */
                /*       are not for our network (i.e. net-directed    */
                /*       broadcasts for another network are            */
                /*       forwarded if possible)                        */
#        if (INCLUDE_ROUTER)
                if (ip_forwarding == 1)       /* if enabled  */
                                                /* (2=disabled)   */
                {
#if (INCLUDE_TRK_PKTS)
                    DCUTOPACKET(msg)->ctrl.list_id = IN_ROUTE_LAYER;
#endif
                    route_pkg(pi, msg, pip);  /* will free msg */
                }
                else
                {
                    /* Not ours. drop the packet   */
                    os_free_packet(msg);

                    UPDATE_INFO(pi, ip_dropped, 1)
                    INCR_SNMP(IpInAddrErrors)
                    DEBUG_LOG("tc_ip_interpret - dropped pkt", LEVEL_3, NOVAR, 0, 0);
                }
#        else
                /* Not ours. drop the packet   */
                os_free_packet(msg);

                UPDATE_INFO(pi, ip_dropped, 1)
                INCR_SNMP(IpInAddrErrors)
                DEBUG_LOG("tc_ip_interpret - dropped pkt", LEVEL_3, NOVAR, 0, 0);
#        endif

                return;
            }
        }
    }

    /* **************************************************   */
#if (INCLUDE_KEEP_STATS)
    if ((header_len_bytes) > IP_HLEN_BYTES)
    {
        UPDATE_INFO(pi, ip_options, 1)
    }
#endif

    /* **************************************************        */
    /* Check if fragmented packet                                */
    /* NOTE: if more frag bit or frag offset is set must be frag */
    if (pip->ip_fragoff & IP_HASFRAG_68K)
    {
        UPDATE_INFO(pi, ip_fragments, 1)
        INCR_SNMP(IpReasmReqds)
#if (INCLUDE_FRAG)
        /* save fragment in fragment table    */
        msg = ipf_add(pi, msg);

        /* if all the fragments have not arrived do nothing else   */
        if (!msg)
            return;

        /* Look at the packet       */
        pip = DCUTOIPPKT(msg);
#else
        DEBUG_ERROR("IP Fragmentation not supported", NOVAR, 0, 0);
        INCR_SNMP(IpInHdrErrors)
        os_free_packet(msg);        /* Free the input message */
        return;
#endif
    }

    /* **************************************************   */
#if (INCLUDE_RAW)
    /* pass to all raw sockets                          */
    /* NOTE: RAW interpret will NOT free ms             */
    /* Only call raw_interpret if we have any listeners */
    if (root_raw_lists[ACTIVE_LIST])
        found_raw = tc_raw_interpret(pi, msg);
    else
        found_raw = FALSE;
#endif

    /* **************************************************   */
    /* Now dispatch the packet to the appropriate handler   */
    switch (pip->ip_proto)
    {
#if (INCLUDE_UDP)
        /* PROCESS UDP PACKETS   */
        case PROTUDP:
#if (DEBUG_IP_INTERPRET)
            DEBUG_ERROR("ipv4_interpret: UDP packet", NOVAR, 0, 0);
#endif
            DEBUG_LOG("tc_ip_interpret - UDP", LEVEL_3, NOVAR, 0, 0);
            INCR_SNMP(IpInDelivers)

#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = IN_UDP_LAYER;
#endif
            tc_udp_interpret(pi, msg);
            break;
#endif

#if (INCLUDE_TCP)
        /* PROCESS TCP PACKETS   */
        case PROTTCP:
#if (DEBUG_IP_INTERPRET)
            DEBUG_ERROR("ipv4_interpret: TCP packet", NOVAR, 0, 0);
#endif
            INCR_SNMP(IpInDelivers)
#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = IN_TCP_LAYER;
#endif
            tc_tcp_interpret(pi, msg);
            break;

#endif

        /* PROCESS ICMP PACKETS   */
        case PROTICMP:
#if (DEBUG_IP_INTERPRET)
            DEBUG_ERROR("ipv4_interpret: ICMP packet", NOVAR, 0, 0);
#endif
            INCR_SNMP(IpInDelivers)
#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = IN_ICMP_LAYER;
#endif
            tc_icmp_interpret(pi, msg);
            break;

#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
        /* PROCESS IGMP PACKETS   */
        case PROTIGMP:
#if (DEBUG_IP_INTERPRET)
            DEBUG_ERROR("ipv4_interpret: IGMP packet", NOVAR, 0, 0);
#endif
            INCR_SNMP(IpInDelivers)
#if (INCLUDE_TRK_PKTS)
            DCUTOPACKET(msg)->ctrl.list_id = IN_IGMP_LAYER;
#endif
            tc_igmp_interpret(pi, msg);
            break;
#endif

        /* PROCESS UNKNOWN PROTOCOL PACKETS (not UDP, TCP or ICMP)   */
        /* NOTE: if a RAW packet matched the protocol then no        */
        /*       error occurred                                      */
        default:
#    if (INCLUDE_RAW)
            if (!found_raw)
            {
#    endif
#        if (INCLUDE_ICMP)
/*              if (check_send_icmp(msg))                                   */
/*              {                                                           */
/*                  tc_icmp_send(msg,ICMP_T_DEST_UNREACH,ICMP_C_PROTUR, 0); */
/*              }                                                           */
#        endif
                                        
                UPDATE_INFO(pi, ip_unknown, 1)
                INCR_SNMP(IpInUnknownProtos)
                DEBUG_ERROR("ERROR(RTIP): tc_ip_interpret() - Unknown Protocol Type", NOVAR, 0, 0);
#    if (INCLUDE_RAW)
            }
#    endif
            os_free_packet(msg);        /* Free the input message */
            break;
    }   /* switch iptype */
}
#endif

/* ********************************************************************   */
/* IP UTILITY ROUTINES                                                    */
/* ********************************************************************   */

/* ********************************************************************   */
/* ip_alloc_packet() - allocate a packet for output                       */
/*                                                                        */
/*   Allocates a packet and does general setup for output, i.e.           */
/*     - sets up prot.ptr field in the DCU to point to the                */
/*       start of the TCP/UDP/ICMP/IGMP header                            */
/*     - sets up IP option length                                         */
/*   If port parameter is 0 then there are no IP options and will not     */
/*   wait to alloc a packet.                                              */
/*   NOTE: port->ctrl.block_ticks must also be set to block               */
/*                                                                        */
/*   Returns the DCU                                                      */

DCU ip_alloc_packet(PIFACE pi, PANYPORT port, int nbytes, int frag_off, 
                    int who)
{
DCU     msg;
#if (INCLUDE_802_2 || INCLUDE_TOKEN)
RTIP_BOOLEAN is_802_2;
#endif

#if (!INCLUDE_IP_OPTIONS)
    ARGSUSED_INT(frag_off)
    ARGSUSED_PVOID(port)
#endif

#if (!INCLUDE_802_2)
    ARGSUSED_PVOID(pi)
#endif

#if (INCLUDE_NO_DCU_BLOCK)
    if ( (msg = os_alloc_packet_wait(nbytes, port, who)) == (DCU)0 )
#else
    if ( (msg = os_alloc_packet(nbytes, who)) == (DCU)0 )
#endif
    {
        DEBUG_ERROR("ERROR(rtip) - ip_alloc_packet - os_alloc_packet failed", 
            NOVAR, 0, 0);
        return((DCU)0);
    }

    /* determine if need 802.2 (SNAP,LLC) header   */
#if (INCLUDE_802_2)
    is_802_2 = (RTIP_BOOLEAN)(SEND_802_2(pi, port));
#endif

    DCUSETUPIPARP(msg, is_802_2)
    /* calculate and save the length of the options   */
    DCUTOPACKET(msg)->ip_option_len = 0;

#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
    /* Only call ip_options_format if there are ip_options   */
    if (port && port->ip_options)
    {
        /* set up options length and write the options in the output packet   */
        DCUTOPACKET(msg)->ip_option_len = ip_options_format(port, msg, 
                                                            frag_off);
    }
#endif
#if (INCLUDE_IGMP_V2)
    else if (who == IGMP_ALLOC)
    {
        DCUTOPACKET(msg)->ip_options |= RALERT_OPTION;
        DCUTOPACKET(msg)->ip_option_len = ip_options_format((PANYPORT)0,
                                                            msg, frag_off);
    }
#endif

    /* set up pointer to protocol header   */
    DCUSETUPPROT(msg)

    return(msg);
}

#if (INCLUDE_IPV4)
/* ********************************************************************   */
/* setup_ipv4_header() - sets up an IP header                             */
/*                                                                        */
/*   Sets up IPV4 header including the checksum based upon input          */
/*   parameters.  len is ip_len (total length of current frag which       */
/*   includes IP header, protocol header and data                         */
/*                                                                        */
/*   len = IP length, i.e. IP header + protocol                           */
/*                                                                        */
/*   If ttl is non-zero, uses parameter ttl for IP header                 */
/*                                                                        */
/*   NOTE: assumes protocol, ip src, ip dest and fragment offset          */
/*         fields are already set up                                      */
/*                                                                        */
/*   Returns nothing                                                      */

void setup_ipv4_header(DCU msg, PANYPORT port, word len, word frag_off, 
                       RTIP_BOOLEAN more_frag, int ttl, RTIP_BOOLEAN set_ip_seq)
{
PIPPKT pip;
int    ip_hlen;

    pip = DCUTOIPPKT(msg);

    ip_hlen = IP_HLEN_BYTES + msg->ip_option_len;

    /* set frag offset    */
    pip->ip_fragoff = hs2net((word)(frag_off >> 3)); 

    /* NOTE: UDP: the template has not been written yet   */
    if (more_frag)       /* if not last fragment */
        pip->ip_fragoff |= IP_MOREFRAG_68K;   /* set frag bit on all but last frag */

    /* convert header length to words   */
    pip->ip_verlen = (byte)((ip_hlen) >> 2);
    pip->ip_verlen |= 0x40;     /* set to version 4 */

    /* TOS set by setsockopt-SO_TOS otherwise it is 0   */
    if (port)
        pip->ip_tos = port->tos;      /* type of service */
    else
        pip->ip_tos = 0;


    /* only set ip seq not frag   */
    if (set_ip_seq && (frag_off == 0))
    {
        pip->ip_id = hs2net(ipsequence++);
    }

    /* set IP TTL; if setsockopt() was called to set a TTL value, use it   */
    /* otherwise use the default                                           */
    if (ttl > 0)
        pip->ip_ttl = (byte)ttl;
    else if ( (port && port->ip_ttl != -1) &&
         (!IP_ISMCAST_ADDR(pip->ip_dest[0])) )
        pip->ip_ttl = (byte)port->ip_ttl;
    else if ( (port && port->mcast_ip_ttl != -1) &&
          (IP_ISMCAST_ADDR(pip->ip_dest[0])) )
        pip->ip_ttl = (byte)port->mcast_ip_ttl;
    else
    {
        /* use default: if multicast (UDP only) the default is 1    */
        /* (as per RFC 1112);                                       */
        /* otherwise use value set by configuration (CFG_IP_TTL)    */
        /* NOTE: default(CFG_IP_TTL) can also be changed by SNMP    */
        if ( IP_ISMCAST_ADDR(pip->ip_dest[0]) || 
             (pip->ip_proto == PROTIGMP) )
            pip->ip_ttl = 1;
        else
            pip->ip_ttl = (byte)CFG_IP_TTL;
    }

    pip->ip_len = hs2net(len);

    /* calc ip checksum; pass in header length in terms of 16-bit words   */
    /* NOTE: ip_hlen is in terms of bytes                                 */
    pip->ip_cksum = 0;
    pip->ip_cksum = tc_ip_chksum((PFWORD)&(pip->ip_verlen), ip_hlen>>1);

    IP_LEN_DCU(msg) = len;
    IP_HLEN_DCU(msg) = (byte)ip_hlen;
}
#endif      /* INCLUDE_IPV4 */


/* ********************************************************************   */
/* check if address is net-directed broadcast address                     */
RTIP_BOOLEAN is_net_broadcast(PIFACE pi, PFBYTE ipn)
{
int  i;
byte curr_net_mask;
byte curr_mask;


    if (pi->pdev && pi->pdev->device_id == LOOP_DEVICE)
        return(FALSE);

    for (i=3; i>= 0; i--)
    {
        curr_mask = pi->addr.my_ip_mask[i];
        curr_net_mask = (byte)~(curr_mask);
        if ( ((ipn[i] & curr_net_mask) != curr_net_mask) ||
             ((ipn[i] & curr_mask) != (pi->addr.my_ip_addr[i] & curr_mask)) )
        {
            return(FALSE);
        }
    }
    return(TRUE);
}

/* ********************************************************************   */
/* tc_get_local_pi() - locate interface structure based by IP address     */
/*                                                                        */
/* This routine locates the pi interface structure for the supplied IP    */
/* address.  If it is found, the pi address is returned.  Otherwise,      */
/* the IP address is not on 'us' so we cannot ARP reply.                  */

PIFACE tc_get_local_pi(PFBYTE ip_addr)  /*__fn__*/
{
int    iface_no;
PIFACE pi;

    LOOP_THRU_IFACES(iface_no)
    {
        PI_FROM_OFF(pi, iface_no)
        if ( pi && pi->open_count && (pi->addr.iface_flags & IP_ADDR_VALID) &&
             (pi->pdev->device_id != LOOP_DEVICE) )
        {
            if (tc_cmp4(ip_addr, pi->addr.my_ip_addr, 4)) 
            {
                return(pi);
            }
        }
    }
    return((PIFACE)0);
}

/* ********************************************************************   */
/* is_local_mcast() - checks if address is local multicast address        */
/*                                                                        */
/*   Returns TRUE if the address ipn is a local multicast address         */
/*   listening on.                                                        */

RTIP_BOOLEAN is_local_mcast(PIFACE pi, PFBYTE ipn)
{
RTIP_BOOLEAN found;
int i;

    found = FALSE;

    /* filter the multicast input packet                          */
    /* NOTE: statistics will not be updated since this could only */
    /*       occur if the driver did not filter the packet        */
    if ( IP_ISMCAST_ADDR(ipn[0]) )
    {
        for (i=0; i < pi->mcast.lenmclist; i++)
        {
            if ( tc_cmp4(&(pi->mcast.mclist_ip[i*IP_ALEN]), ipn, 4) )
                found = TRUE;
        }
    }
    return(found);
}

#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
/* ********************************************************************   */
/* IP OPTIONS                                                             */
/* ********************************************************************   */
/* format IP options in msg                                               */
int ip_options_format(PANYPORT port, DCU msg, int frag_off)
{
PFBYTE pb;
int    option_len;

    option_len = 0;
    pb = (PFBYTE)DCUTOIPPKT(msg)+IP_HLEN_BYTES;  /* pt to option area of outgoing */

#if (INCLUDE_IGMP_V2)
    /*process router alert option   */

    /* this must be first since port is 0 for this option and if 0 is       */
    /* used in all the other if statements the PC could reboot              */
    /* if this is true, the other options should not exist at the same time */

    if (DCUTOPACKET(msg)->ip_options & RALERT_OPTION) 
    {
        *pb = ROUTER_ALERT_OPTION;  /*set type field */
        *(pb+1) = ROUTER_ALERT_LEN;     /*set length field */
        *(pb+2) = 0;                       /*set data field */
        *(pb+3) = 0;                       /*set data field */
        pb += ROUTER_ALERT_LEN;
        option_len += ROUTER_ALERT_LEN;
    }
    else
    {
#endif

    /* process record route option   */
    if ( (port->ip_options & SO_RECORD_ROUTE_OPTION) && (frag_off == 0) )
    {
        *pb = RECORD_ROUTE_OPTION;                    /* set type field */
        *(pb+1) = (byte)(port->route_option_len + 3); /* set length field */
        *(pb+2) = 4;                                  /* set pointer field */
        tc_memset(pb+3, 0, port->route_option_len);
        pb += (port->route_option_len + 3);
        option_len += (port->route_option_len + 3);
    }

    /* process loose source route option   */
    if (port->ip_options & SO_LOOSE_ROUTE_OPTION)
    {
        *pb = LOOSE_ROUTE_OPTION;
        *(pb+1) = (byte)(port->route_option_len + 3);  /* set length field */
        *(pb+2) = 4;                                   /* set pointer field */
        tc_movebytes(pb+3, port->route_option_addresses, 
                     port->route_option_len);
        pb += (port->route_option_len + 3);
        option_len += (port->route_option_len + 3);
    }

    /* process strict source route option   */
    if (port->ip_options & SO_STRICT_ROUTE_OPTION)
    {
        *pb = STRICT_ROUTE_OPTION;
        *(pb+1) = (byte)(port->route_option_len + 3); /* set length field */
        *(pb+2) = 4;                                  /* set pointer field */
        tc_movebytes(pb+3, port->route_option_addresses, 
                     port->route_option_len);
        pb += (port->route_option_len + 3);
        option_len += (port->route_option_len + 3);
    }

    if (port->ip_options & SO_TIMESTAMP_OPTION)
    {
        *pb = TIMESTAMP_OPTION;
        *(pb+1) = (byte)(port->route_option_len + 3); /* set length field */
        *(pb+2) = 4;                                  /* set pointer field */
        tc_memset(pb+3, 0, port->route_option_len);
        pb += (port->route_option_len + 3);
        option_len += (port->route_option_len + 3);
    }

#if (INCLUDE_IGMP_V2)
    }
#endif

    /* pad with NOP to put on mod 4 boundary (tbd)   */
    while (option_len % 4)
    {
        *pb = NOP_OPTION;
        option_len += 1;
        pb += 1;
    }
    return(option_len);
}

/* calculate length of IP options which will be sent upon output   */
int calc_ip_option_len(PANYPORT port)
{
int    option_len;

    option_len = 0;

#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
    /* process record route option   */
    if (port->ip_options & SO_RECORD_ROUTE_OPTION)
    {
        option_len += (port->route_option_len + 3);
    }

    /* process loose source route option   */
    if (port->ip_options & SO_LOOSE_ROUTE_OPTION)
    {
        option_len += (port->route_option_len + 3);
    }

    /* process strict source route option   */
    if (port->ip_options & SO_STRICT_ROUTE_OPTION)
    {
        option_len += (port->route_option_len + 3);
    }

    if (port->ip_options & SO_TIMESTAMP_OPTION)
    {
        option_len += (port->route_option_len + 3);
    }

    /* pad with NOP to put on mod 4 boundary   */
    while (option_len % 4)
    {
        option_len += 1;
    }
#endif
    return(option_len);
}

/* set up an IP option information in the port structure and calculate   */
/* the new length of IP options for the port                             */
int ip_set_option(PANYPORT port, int option_name, PFCCHAR option_value)
{
PROUTE_INFO proute;

    switch (option_name)
    {
#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
        case SO_RECORD_ROUTE_OPTION:
        case SO_LOOSE_ROUTE_OPTION:
        case SO_STRICT_ROUTE_OPTION:
            /* save the route info in the port structure   */
            proute = (PROUTE_INFO)option_value;
            port->route_option_len = proute->route_len;
            port->route_option_addresses  = proute->route_addresses;
            break;
        case SO_TIMESTAMP_OPTION:  
#endif
        default:
            return(set_errno(EOPNOTSUPPORT));
    }

    /* set length of option (this might be different for fragments)   */
    port->ip_option_len = calc_ip_option_len(port);
    return(0);
}
#endif


/* ********************************************************************   */
/* Fragmentation Utilities                                                */
/* ********************************************************************   */

/* is_frag() - determines if msg fragmented                             */
/*                                                                      */
/*   Determines if msg is part of a fragmented packet, i.e. if the      */
/*   frag offset field is not zero; to be more specific the fragment    */
/*   bit must be set (this field consists of a flag and an offset which */
/*   must both be 0 on a non-fragmented packet)                         */
/*                                                                      */
/*   Returns TRUE if packet is fragmented, FALSE if it is not           */

RTIP_BOOLEAN is_frag(DCU msg)   /*__fn__*/
{
PIPPKT pip;

    pip = DCUTOIPPKT(msg);

    if (pip->ip_fragoff & IP_HASFRAG_68K)
        return(TRUE);

    return(FALSE);
}

/* is_frag_not_first() - determines if msg fragmented and not first fragment   */
/*                                                                             */
/*   Determines if msg is part of a fragmented packet and it is not the        */
/*   first fragment, i.e. it is fragmented if the frag bit is set and it       */
/*   is not the first if the frag offset field is not zero.  This routine      */
/*   is used to determine if there is a protocol field in the header, i.e.     */
/*   if it is a fragmented packet but not the first fragment then there        */
/*   is not a protocol header.  This applies to UDP and TCP.  Raw packets      */
/*   never have a protocol header.                                             */
/*                                                                             */
/*   Returns TRUE if packet is fragmented but not first frag, FALSE if it is   */
/*   not fragmented or the first fragment in a packet                          */

RTIP_BOOLEAN is_frag_not_first(DCU msg) /*__fn__*/
{
PIPPKT pip;
int fragoff;

    pip = DCUTOIPPKT(msg);

    fragoff = (word) (net2hs(DCUTOIPPKT(msg)->ip_fragoff) & IP_FRAGOFF);
    if ( (pip->ip_fragoff & IP_HASFRAG_68K) && (fragoff != 0) )
        return(TRUE);

    return(FALSE);
}

#if (INCLUDE_FRAG)
/* ********************************************************************   */
/* Fragmentation Table Routines (Interpret and Timer)                     */
/* ********************************************************************   */

/* ********************************************************************   */
/* ipf_add() - add fragment to fragment table, check if complete message  */
/*                                                                        */
/*    Put the incoming IP fragment msg in the fragment                    */
/*    table in the entry which contains the fragment for the same         */
/*    message or start a new entry in the table.  The chain of fragments  */
/*    for each entry are keep in order.                                   */
/*                                                                        */
/*    Returns the completed message or 0 if not all fragments for the     */
/*    message have been received or error (no free table entries)         */

DCU ipf_add(PIFACE pi, DCU msg)     /*__fn__*/
{
int     i;
int     table_entry;
word    ipf_id;
dword   ipf_src;
byte    ipf_prot;
DCU     curr_fr_msg; 
DCU     pprev;
DCU     return_msg;
word    fragoff;
word    data_len;
DCU     msg_tbl;
PIPPKT  pip;

#if (!INCLUDE_KEEP_STATS)
    ARGSUSED_PVOID(pi);
#endif

#if (DEBUG_FRAG)
    DEBUG_ERROR("ipf add ", NOVAR, 0, 0);
#endif

    OS_CLAIM_TABLE(IPF_ADD_TABLE_CLAIM)     /* protect the frag table  */
                                            /* list while modifying   */

    ipf_id   = DCUTOIPPKT(msg)->ip_id;
    tc_mv4((PFBYTE)&ipf_src, DCUTOIPPKT(msg)->ip_src, IP_ALEN);
    ipf_prot = DCUTOIPPKT(msg)->ip_proto;

    DCUTOPACKET(msg)->frag_next = (DCU)0;      /* Fragment list management */

    /* Get the fragment offset from the packet and store it in native form   */
    fragoff = (word) (net2hs(DCUTOIPPKT(msg)->ip_fragoff) & IP_FRAGOFF);
    fragoff <<= 3;
    DCUTOPACKET(msg)->fragoff_info = fragoff;

    /* **************************************************           */
    /* loop thru fragment table looking for an entry which contains */
    /* fragments from the same message (before it was fragmented);  */
    /* sets table_entry to match or a free entry                    */
    table_entry = -1;
    for (i = 0; i < CFG_FRAG_TABLE_SIZE; i++)
    {
        /* check for match                                           */
        /* NOTE: IP interpret already checked if msg intended for us */
        /* NOTE: IP id, src IP address and protocol must match       */
        if ( (ipf_id   == frag_table[i].ipf_id)  && 
             (ipf_src  == frag_table[i].ipf_src) &&
             (ipf_prot == frag_table[i].ipf_prot) )
        {
            table_entry = i;
            break;
        }

        /* if first free entry, save it in case don't find a match   */
        else if (table_entry < 0 && !frag_table[i].ipf_next)
            table_entry = i;
    }

    /* **************************************************             */
    /* if did not find an entry in table, i.e. this is first fragment */
    /* of a message and the table is full, then return                */
    if (table_entry == -1)
    {
        OS_RELEASE_TABLE()  
        UPDATE_INFO(pi, ip_dropped, 1)
        DEBUG_ERROR("ERROR: rtip - ipfadd() - frag dropped cuz no room",
            NOVAR, 0, 0);
        os_free_packet(msg);        /* Free the input message */
        return((DCU)0);
    }

#if (DEBUG_FRAG)
    DEBUG_ERROR("ipf add at offset", EBS_INT1, table_entry, 0);
#endif

    /* **************************************************            */
    /* check if fragment offset exceeds limit; if so drop the packet */
    /* and free any entries in the table for that fragment           */
    pip = DCUTOIPPKT(msg);
    data_len = (word)(net2hs(pip->ip_len) - IP_HLEN(pip));

    if ( ((ipf_prot == PROTICMP) && 
          ((int)(fragoff + data_len - ICMP_HLEN_BYTES) > CFG_MAX_FRAG_ICMP)) ||
         ((ipf_prot != PROTICMP) && (fragoff + data_len > (word)CFG_MAX_FRAG)) )
    {
        msg_tbl = frag_table[table_entry].ipf_next;
        if (msg_tbl)
        {
            DEBUG_ERROR("ipf add: packet dropped-offset exceeds limit(CFG_MAX_FRAG)", 
                DINT2, fragoff, data_len);

            os_free_packet(msg_tbl);
            frag_table[i].ipf_next = (DCU)0;    /* mark empty */
        }
        OS_RELEASE_TABLE()  
        os_free_packet(msg);
        return((DCU)0);
    }

    /* **************************************************     */
    /* add the fragment to the linked list at the table entry */

    /* if first fragment of message, add it to the beginning of list   */
    if (!frag_table[table_entry].ipf_next)  
    {
        frag_table[table_entry].ipf_ttl = CFG_IP_FRAG_TTL; 
        frag_table[table_entry].ipf_id = ipf_id;
        frag_table[table_entry].ipf_src = ipf_src;  
        frag_table[table_entry].ipf_prot = ipf_prot;    

        frag_table[table_entry].ipf_next = msg;
    }

    /* if other fragments have previously arrived, loop thru fragment    */
    /* previously received for this message and insert the message       */
    /* in spot keeping the fragment in the correct order                 */
    else   
    {
        curr_fr_msg = frag_table[table_entry].ipf_next;

        pprev = (DCU)0;  
        while (curr_fr_msg)
        {
            /* if we are less than an element in the list, insert before;   */
            /* this is done instead of adding to end since could be out     */
            /* of order                                                     */
            if (DCUTOPACKET(msg)->fragoff_info < 
                DCUTOPACKET(curr_fr_msg)->fragoff_info)
            {
                DCUTOPACKET(msg)->frag_next = curr_fr_msg;
                if (pprev)
                    DCUTOPACKET(pprev)->frag_next = msg;
                else
                    frag_table[table_entry].ipf_next = msg;
                break;
            }
 
            /* if we are greater than an element in the list keep going;   */
            /* if there is another element in the list, get it and         */
            /* loop again                                                  */
            if (!DCUTOPACKET(curr_fr_msg)->frag_next)
            {
                /* We are at the end of the list.   */
                DCUTOPACKET(curr_fr_msg)->frag_next =  msg;
                break;
            }

            /* if we are not at the end of the list, continue   */
            pprev = curr_fr_msg;
            curr_fr_msg =   DCUTOPACKET(curr_fr_msg)->frag_next;
        }            
    }

    /* **************************************************          */
    /* check if have the complete packet; if complete return it to */
    /* IP layer and free the fragment table entry                  */
    return_msg = ipf_check_complete(table_entry);
    if (return_msg)
    {
        frag_table[table_entry].ipf_next = (DCU)0;  /* mark entry free */
        INCR_SNMP(IpReasmOKs)
    }

    OS_RELEASE_TABLE()

    return(return_msg);
}

#if (DEBUG_FRAG)
int frag_entries_used(void)
{
int num_entries_used;

    num_entries_used = 0;

    for (i = 0; i < CFG_MAXFRAGS; i++)
    {
        /* if free entry   */
        if (frag_table[i].ipf_next)
            num_entries_used++;
    }
    return(num_entries_used);
}

void display_frag_table(int offset)
{
DCU     curr_fr_msg;
PIPPKT  pip;

    curr_fr_msg = frag_table[offset].ipf_next;
    while (curr_fr_msg)
    {
        pip = DCUTOIPPKT(curr_fr_msg);
        DEBUG_ERROR("fragoff_info, data len: ", DINT2,
            DCUTOPACKET(curr_fr_msg)->fragoff_info, 
            net2hs(pip->ip_len) - IP_HLEN(pip));
        DEBUG_ERROR("ip_fragoff, IP_MOREFRAG_68K = ", DINT2, 
            pip->ip_fragoff, IP_MOREFRAG_68K);
        curr_fr_msg = DCUTOPACKET(curr_fr_msg)->frag_next;
    }
}
#endif

/* ********************************************************************     */
/* ipf_check_complete() - Check if all fragments for a message have arrived */
/*                                                                          */
/*   Checks if all the fragments have arrived and calls ipf_join() to       */
/*   joins UDP/TCP/ICMP header into the first fragment if all               */
/*   the fragments of the message have been received.                       */
/*                                                                          */
/* Returns the message if there is all the fragment for the package have    */
/* arrived or 0 if there is a hole in message                               */

DCU ipf_check_complete(int table_entry)     /*__fn__*/
{
DCU     curr_fr_msg;
DCU     plast;
word    off;
PIPPKT  pip;
 
    /* first check if all the fragments for the message have arrived;   */
    /* return 0 if there are any holes                                  */
    off = 0;
    plast = curr_fr_msg = frag_table[table_entry].ipf_next;
    while (curr_fr_msg)
    {
        plast = curr_fr_msg;

        /* If the fragment offsets are not contiguous we don't have a   */
        /* complete packet                                              */
        if (off != DCUTOPACKET(curr_fr_msg)->fragoff_info)
        {
#if (DEBUG_FRAG)
            DEBUG_ERROR("ipf_check_complete: not contiguous", DINT2, 
                off, DCUTOPACKET(curr_fr_msg)->fragoff_info);
#endif
            return((DCU)0);
        }

        /* no hole, therefore, get expected offset of next frag    */
        pip = DCUTOIPPKT(plast);
        off = (word) (DCUTOPACKET(curr_fr_msg)->fragoff_info + 
              net2hs(pip->ip_len) - IP_HLEN(pip));

        curr_fr_msg = DCUTOPACKET(curr_fr_msg)->frag_next;
    }

    if (!plast)
    {
#if (DEBUG_FRAG)
        DEBUG_ERROR("ipf_check_complete: no last ", NOVAR, 0, 0);
#endif
        return((DCU)0);
    }

    /* If more frags is set we're not at the end   */
    if (DCUTOIPPKT(plast)->ip_fragoff & IP_MOREFRAG_68K)
    {
#if (DEBUG_FRAG)
        DEBUG_ERROR("ipf_check_complete: more frags, ip_fragoff = ", 
            DINT2, DCUTOIPPKT(plast)->ip_fragoff, 0);
        display_frag_table(0);
#endif
        return((DCU)0);
    }

    /* **************************************************             */
    /* If we are here all the fragments for the message have arrived, */
    /* return the entry                                               */
    return(ipf_join(table_entry));
}

/* ********************************************************************    */
/* ipf_join() - Join packets until get UDP/TCP/ICMP header in 1st pkt      */
/*                                                                         */
/* Joins the fragments of entry table_entry in the fragment table          */
/* together into one message to the point where the UDP/TCP/ICMP           */
/* header are in the first packet.                                         */
/*                                                                         */
/* Returns the fragmented message consisting of the header joined together */

DCU ipf_join(int table_entry)     /*__fn__*/
{
DCU    plast;
PFBYTE p_data_to;
PFBYTE p_data_from;
word   nxt_dlen;
word   total_dlen;
word   head_ip_len;
word   curr_ip_len;
word   dlen_to_copy;
PIPPKT head_pip;
PIPPKT pip_nxt;
DCU    head_msg;
DCU    msg_nxt;
 
    /* join at least the UDP/ICMP/TCP header into the first pkt   */
    /* NOTE: there always is enough room in the first packet for  */
    /*       the UDP/TCP/ICMP header                              */
    head_msg = frag_table[table_entry].ipf_next;

    head_pip = DCUTOIPPKT(head_msg);
    head_ip_len = net2hs(head_pip->ip_len);
    total_dlen = (word) (head_ip_len - IP_HLEN(head_pip));
    p_data_to = DCUTODATA(head_msg) + ETH_HLEN_BYTES + head_ip_len;

    msg_nxt = DCUTOPACKET(head_msg)->frag_next;

    /* NOTE: the TCP and ICMP header lengths are the same and both are   */
    /*       larger than the UDP header length so join the larger amount */
    /* NOTE: dlen refers to DATA and header                              */
    while ( msg_nxt && (total_dlen < TCP_HLEN_BYTES) )
    {
        pip_nxt = DCUTOIPPKT(msg_nxt);
        curr_ip_len = net2hs(pip_nxt->ip_len);
        nxt_dlen = (word) (curr_ip_len - IP_HLEN(pip_nxt));

        /* determine amount of data to copy   */
        dlen_to_copy = nxt_dlen;
        if (total_dlen + ETH_HLEN_BYTES + nxt_dlen > (dword)MAX_PACKETSIZE)
            dlen_to_copy = (word) (MAX_PACKETSIZE - head_ip_len - ETH_HLEN_BYTES);

        /* copy data from start of next DCU to end of head DCU   */
        p_data_from = DCUTODATA(msg_nxt) + ETH_HLEN_BYTES + IP_HLEN(pip_nxt);
        tc_movebytes(p_data_to, p_data_from, dlen_to_copy);

        p_data_to += dlen_to_copy;
        total_dlen = (word)(total_dlen + dlen_to_copy);
        head_ip_len = (word)(head_ip_len + dlen_to_copy);
        curr_ip_len = (word)(curr_ip_len - dlen_to_copy);

        /* set up for next fragment   */
        plast = msg_nxt;
        msg_nxt = DCUTOPACKET(msg_nxt)->frag_next;

        /* if packet exhausted, free it                                   */
        /* NOTE: always freeing the second packet in the list             */
        /* NOTE: the udp_len never changes                                */
        /* NOTE: the ip_len of the head packet is adjusted after the loop */
        if (dlen_to_copy >= nxt_dlen)
        {
            DCUTOPACKET(plast)->frag_next = (DCU)0;
            os_free_packet(plast);    
            DCUTOPACKET(head_msg)->frag_next = msg_nxt;  
        }
        /* packet not exhausted so slide the data to the   */
        /* beginning of packet and adjust the IP len       */
        else
        {
            tc_movebytes(p_data_from, p_data_from+dlen_to_copy, 
                nxt_dlen-dlen_to_copy);
            pip_nxt->ip_len = hs2net(curr_ip_len);
        }

    } 

    DCUTOIPPKT(head_msg)->ip_len = hs2net(head_ip_len);

    return(head_msg);
}

/* ********************************************************************   */
/* ipf_timer() - performs maintance of the fragment table                 */
/*                                                                        */
/*    Called once per second to discard any messages in the fragment      */
/*    table which have expired                                            */
/*                                                                        */
/*    Returns nothing                                                     */

void ipf_timer(void)   /*__fn__*/
{
int i;
DCU msg;

    OS_CLAIM_TABLE(IPF_TIMER1_TABLE_CLAIM)

    for (i=0; i < CFG_FRAG_TABLE_SIZE; i++)
    {
        /* if entry is empty   */
        if (!frag_table[i].ipf_next)
            continue;

        /* decrement time-to-live and check for timeout   */
        frag_table[i].ipf_ttl--;
        if (frag_table[i].ipf_ttl <= 0)
        {
            /* timeout   */
            msg = frag_table[i].ipf_next;

            OS_RELEASE_TABLE()  /* release for icmp send */

            /* send an ICMP message; will only send if the first fragment   */
            /* has been received (RFC1122 - 3.2.2)                          */
            /* i.e. the offset of the first frag must be 0                  */
            DEBUG_ERROR("frag table: time limit exceeded", NOVAR, 0, 0);
            tc_icmp_send(msg, ICMP_T_TIME_EXCEEDED, ICMP_C_TEXP_FRAG, 0);

            OS_CLAIM_TABLE(IPF_TIMER2_TABLE_CLAIM)

            /* free the entry   */
            os_free_packet(msg);
            frag_table[i].ipf_next = (DCU)0;    /* mark empty */
            INCR_SNMP(IpReasmFails)
        }
    }

    OS_RELEASE_TABLE()
}


/* ********************************************************************   */
/* Various Fragmentation Routines                                         */
/* ********************************************************************   */

#if (INCLUDE_UDP || INCLUDE_RAW)
/* ********************************************************************   */
/* ipf_extract_data() - extract UDP data from fragmented message          */
/*                                                                        */
/*    Copies data from UDP fragmented packet into buffer.  Copies at      */
/*    most buflen bytes.                                                  */
/*    prototcol_hlen is the size of the protocol header (possibly         */
/*    a UDP header).  Only the first fragment has a protocol header.      */
/*                                                                        */
/*    Returns number of bytes written to buffer.                          */
/*                                                                        */

int ipf_extract_data(DCU msg, PFBYTE buffer, int buflen, int protocol_hlen,
                     RTIP_BOOLEAN save_ip_hdr)   /*__fn__*/
{
int tot_len;
int ip_hlen;
int dlen;
PIPPKT pip;
PFBYTE pdata;

    tot_len = 0;

    while (msg && buflen)
    {
        pip = DCUTOIPPKT(msg);

        if ( save_ip_hdr && (tot_len == 0) )
            ip_hlen = 0;
        else
            ip_hlen = (word)IP_HLEN(pip);       
        dlen = net2hs(pip->ip_len) - ip_hlen - protocol_hlen;  

        /* if will overflow buffer, limit it   */
        if (dlen > buflen)
            dlen = buflen;

        /* copy the data from curr frag to the buffer   */
#if (RTIP_VERSION >= 26)
        pdata = DCUTOPROTPKT(msg) + protocol_hlen;
#else
        pdata = DCUTODATA(msg) + ip_hlen + ETH_HLEN_BYTES + protocol_hlen;
#endif
        tc_movebytes(buffer+tot_len, pdata, dlen);

        tot_len += dlen;
        buflen -= dlen;
        protocol_hlen = 0;  /* only the first frag has a prototol (UDP) header */
        msg = DCUTOPACKET(msg)->frag_next;
    }

    return(tot_len);
}
#endif

#if (INCLUDE_UDP || INCLUDE_RAW || INCLUDE_PING || INCLUDE_ROUTER)
/* ********************************************************************      */
/* ipf_create_pkt() - create fragmented UDP/ICMP packet                      */
/*                                                                           */
/*   Creates a fragmented UDP/ICMP packet (i.e. linked DCUs) with frag info  */
/*   and len filled in the IP header but without the data filled in.         */
/*   msg is the first fragment which contains the ETHERNET, IP and protocol  */
/*   headers but no data.                                                    */
/*                                                                           */
/*   NOTE: if port is 0 there are no IP options                              */
/*                                                                           */
/*   Returns the fragmented UDP/ICMP packet or if an error is detected       */
/*   returns 0 and frees msg                                                 */
/*                                                                           */

DCU ipf_create_pkt(PIFACE pi, PANYPORT port, DCU msg, int buflen, int protocol_hlen)   /*__fn__*/
{
DCU       head_msg;
PIPPKT    pipnew;
int       maxdlen;      /* amt of data (including UDP header) to send in each frag  */
#if (FRAG_ROUTER)
int       ip_hlen;
word      orig_frag;
#endif
word      frag_offset;   
int       curr_dlen;    /* amount of data (not including UDP header) to send */
                        /* in current frag   */
int       curr_hlen;
DCU       pprev;
int       phlen;
RTIP_BOOLEAN   more_frag;

    /* NOTE: UDP and ICMP header are 8 and 5 bytes repectively, therefore,   */
    /*       the header will always fit in the first fragment                */

    /* get data length for each fragment packet (does not include IP header)   */
    /* NOTE: must be multiple of 8 octects                                     */
    /* NOTE: packet is ICMP or UDP only or comes from router                   */
    pipnew = DCUTOIPPKT(msg);
    maxdlen = (pi->addr.mtu - IP_HLEN(pipnew)) & ~7; 
#if (FRAG_ROUTER)
    ip_hlen = (word)IP_HLEN(pipnew);        

    /* save frag from head message since if router is fragmenting   */
    /* it will turn on frag bit in the head message                 */
    orig_frag = pipnew->ip_fragoff;
#endif

    /* get frag offset of origional msg; for msg created by local machine    */
    /* this will always be 0 except for msgs which ROUTER code is forwarding */
    /* which might not be 0                                                  */
    frag_offset = (word)(net2hs(pipnew->ip_fragoff) & IP_FRAGOFF);
    frag_offset <<= 3;

    /* all but last frag   */
    head_msg = (DCU)0;
    pprev = (DCU)0;

    phlen = 0;

    while (buflen > 0)
    {
        /* get lengths (IP header, protocol headers, data)   */
        curr_dlen = maxdlen;
        curr_hlen = 0;
        if (!head_msg)       /* if first packet */
        {
            /* first frag has protcol header   */
            curr_dlen = (word)((curr_dlen - protocol_hlen) & ~7);
            curr_hlen = (word)protocol_hlen;
        }

        /* on last loop only send what is left     */
        if (buflen < curr_dlen)
            curr_dlen = (word)buflen;

        /* if not first pkt, get new fragment and link it in   */
        if (head_msg)
        {
#if (DEBUG_FRAG)
            DEBUG_ERROR("ipf_create_pkt: alloc new pkt", NOVAR, 0, 0);
#endif
            /* alloc packet and set up IP options   */
            msg = ip_alloc_packet(pi, port, MAX_PACKETSIZE, 
                                  frag_offset+protocol_hlen, FRAG_ALLOC);
            if (!msg)
            {
                os_free_packet(head_msg);
                set_errno(ENOPKTS);
                return((DCU)0);
            }

            /* set up link layer header based upon infor from first msg   */
            SETUP_LL_HDR(msg, pi, EIP_68K, 
                         SEND_802_2(pi, port),
                         curr_dlen + curr_hlen + IP_HLEN_BYTES + 
                           DCUTOPACKET(msg)->ip_option_len + 
                           LLC_SNAP_HLEN_BYTES)

            SETUP_LL_HDR_DEST(pi, msg, LL_SRC_ADDR(msg, pi))
#if (INCLUDE_IPV4)
            pipnew = DCUTOIPPKT(msg);

            /* copy IP header from first msg   */
            tc_movebytes((PFBYTE)pipnew, DCUTOIPPKT(head_msg), 
                         IP_HLEN_BYTES);
#endif
            DCUTOPACKET(pprev)->frag_next = msg;

#if (FRAG_ROUTER)
            /* set up length (need to router only)   */
            /* will be overwritten in some cases??   */
            DCUTOPACKET(msg)->length = curr_dlen + ip_hlen + ETH_HLEN_BYTES;
#endif
        }
        else        /* first frag */
        {
            head_msg = msg;
        }

        /* determine if more frag bit should be set                  */
        /* NOTE: if router is fragmenting should always set fragment */
        /*       bit if it is set in the origional message           */
        more_frag = FALSE;
        if (curr_dlen < buflen)
            more_frag = TRUE;
#if (FRAG_ROUTER)
        if (port == (PANYPORT)0)
        {
            if (orig_frag & IP_MOREFRAG_68K)
                more_frag = TRUE;
        }
#endif

        /* set up selected entries in IP header                        */
        /* NOTE: UDP: the template has not been written yet            */
        /* NOTE: even if first msg, curr_dlen might have been modified */
        setup_ipv4_header(msg, (PANYPORT)port, 
                          /* ip_len   */
                          (word)(curr_dlen + curr_hlen +
                                 IP_HLEN_BYTES + 
                                 DCUTOPACKET(msg)->ip_option_len),  
                          /* frag_off     */
                          (word)(frag_offset+phlen),
                          more_frag, 0, FALSE);

        /* done by os_alloc but just to be sure do it again   */
        DCUTOPACKET(msg)->frag_next = (DCU)0;

        /* get ready for next loop   */
        pprev = msg;
        buflen = buflen - curr_dlen;
        frag_offset = (word)(frag_offset + curr_dlen);
        phlen = protocol_hlen;

        INCR_SNMP(IpFragCreates)
    }
    INCR_SNMP(IpFragOKs)
    return(head_msg);
}
#endif

#if (INCLUDE_UDP || INCLUDE_RAW || INCLUDE_ROUTER || INCLUDE_PING)
/* ********************************************************************   */
/* ipf_fill_pkt() - fill in data in fragmented packet                     */
/*                                                                        */
/*   Fills data in a fragmented packet (i.e. linked DCUs).  Also          */
/*   sets up the len.  Used to create fragmented UDP packets and by the   */
/*   router to fragment a packet it is forwarding.                        */
/*                                                                        */
/*   msg is the fragmented packet.                                        */
/*   buffer is data portion of packet.                                    */
/*                                                                        */
/*   Returns the fragmented packet or 0 upon error                        */
/*                                                                        */

DCU ipf_fill_pkt(PIFACE pi, DCU msg, PFBYTE buffer, int buflen, int protocol_hlen) /*__fn__*/
{
DCU     head_msg;
PIPPKT  pipnew;
word    maxdlen;        /* amt of data (include any header) to send in each frag  */
word    buf_offset;   
word    curr_dlen;      /* amount of data (not including any header) to send */
                        /* in current frag   */
word    ip_hlen;
PFBYTE  pdata;
#if (INCLUDE_PING)
word    off;
#endif

    /* NOTE: UDP and ICMP header are 8 and 5 bytes repectively, therefore,   */
    /*       the header will always fit in the first fragment                */

    /* get data length for each fragment packet (does not include IP header)   */
    /* NOTE: must be multiple of 8 octects                                     */
    /* NOTE: packet is ICMP or UDP only (not TCP which uses port->sendsize)    */
    /* NOTE: leaves room for options - tbd maybe pass in options?              */
    pipnew = DCUTOIPPKT(msg);
    maxdlen = (word)((pi->addr.mtu - IP_HLEN(pipnew)) & ~7); 

    buf_offset = 0;

    /* all but last frag   */
    head_msg = (DCU)0;
    while (buflen > 0)
    {
        /* if not first pkt, get new fragment and link it in   */
        if (head_msg)
        {
            msg = DCUTOPACKET(msg)->frag_next;
            if (!msg)   /* if run out of DCUs; should never happen unless */
                        /* there is a bug in ipf_create_pkt()   */
            {
                os_free_packet(head_msg);
                return((DCU)0);     /* tbd: what errno?? */
            }
        }
        else        /* first frag */
            head_msg = msg;
        
        /* get pointer to where to copy the data; the first packet needs   */
        /* room for the header (UDP header if there is one)                */
        pipnew = DCUTOIPPKT(msg);
        ip_hlen = (word)IP_HLEN(pipnew);        

        pdata = (PFBYTE)pipnew + ip_hlen;

        curr_dlen = maxdlen;
        if (msg == head_msg)       /* if first packet */
        {
            pdata += protocol_hlen;
            curr_dlen = (word)(curr_dlen - protocol_hlen);
        }

        /* on last loop only send what is left     */
        if ((word)buflen < curr_dlen)
            curr_dlen = (word)buflen;

        if (buffer)
        {
            /* UDP or RAW                                              */
            /* copy the data bytes into the packet that we are sending */
            tc_movebytes(pdata, buffer+buf_offset, curr_dlen);
        }
#if (INCLUDE_PING)
        else
        {
            /* PING                                                          */
            /* fill in the packet that we are sending with a fixed data byte */
            for (off = 0; off < (word)curr_dlen; off++) 
                *pdata++ = 0x5e;

            DCUTOPACKET(msg)->length = 0;
            TOTAL_LL_HLEN_SIZE(DCUTOPACKET(msg)->length, msg_is_802_2(msg), 
                               pi, msg)
            if (msg == head_msg)
                DCUTOPACKET(msg)->length += ICMP_HLEN_BYTES;

            DCUTOPACKET(msg)->length = DCUTOPACKET(msg)->length + 
                                       IP_HLEN_BYTES + 
                                       DCUTOPACKET(msg)->ip_option_len +
                                       curr_dlen;
        }
#endif          /* INCLUDE_PING */

        /* get ready for next loop   */
        buflen = (word)(buflen - curr_dlen);
        buf_offset = (word)(buf_offset + curr_dlen);
    }
    return(head_msg);
}
#endif

#if (INCLUDE_UDP)
/* ********************************************************************   */
/* ipf_send_udp_pkt() - send a fragmented UDP packet                      */
/*                                                                        */
/*   Sends a fragmented UDP packet by calling tc_udp_pk_send() for each   */
/*   fragment.  All the fields except for the checksums were filled in    */
/*   before calling ipf_send_udp_pkt().                                   */
/*                                                                        */
/*   Returns 0 if the send was sent or errno if an error was detected     */
/*                                                                        */

int ipf_send_udp_pkt(PIFACE pi, PFBYTE to, PUDPPORT pport, DCU msg,   /*__fn__*/
                     word total_dlen, word wait_count, int dcu_flags) /*__fn__*/
{
int  status;
DCU  next_msg;
word dlen;
PIPPKT  pip;

    while (msg)
    {
        next_msg = DCUTOPACKET(msg)->frag_next;

        /* if packet is going to be freed after it is sent, unhook it from    */
        /* the fragment to prevent netwrite from freeing all the fragments    */
        if (!(dcu_flags & PKT_FLAG_KEEP))
        {
            DCUTOPACKET(msg)->frag_next = (DCU)0;
        }

        /* call UDP send -                                                   */
        /* NOTE: the len and fragoff fields in the IP header were set up by  */
        /*       ipf_create_pkt(); set the length for pk_send assumming      */
        /*       there is a UDP header since pk_send recalculates ip_len     */
        /*       assuming there is one                                       */
        pip = DCUTOIPPKT(msg);

        dlen = (word)(net2hs(pip->ip_len) - IP_HLEN(pip));
        if ( (pport->ap.port_type == UDPPORTTYPE) &&
             !is_frag_not_first(msg))
        {
            dlen = (word)(dlen - UDP_HLEN_BYTES);
        }
        status = tc_udp_pk_send(pi, to, pport, msg, dlen, total_dlen, 
                                wait_count, dcu_flags);
        if (status)
        {
            /* error so don't continue but if caller is expecting netwrite   */
            /* to free the packet then free the rest of the dcus on the      */
            /* frag list that won't be sent                                  */
            /* NOTE: netwrite always frees packet if specified even if       */
            /*       error                                                   */
            if (!(dcu_flags & PKT_FLAG_KEEP))
                os_free_packet(next_msg);   
            return(status);
        }

        msg = next_msg;
    }
    return(0);
}
#endif      /* INCLUDE_UDP */

#if (INCLUDE_PING || INCLUDE_ROUTER || INCLUDE_FRAG)
/* ********************************************************************   */
int ipf_netwrite(PIFACE pi, PANYPORT port, DCU msg, PFBYTE ipn, word flags, word wait_count) /*__fn__*/
{
int  status;
DCU  next_msg;

    while (msg)
    {
#if (DEBUG_FRAG)
        DEBUG_ERROR("ipf_netwrite: frag = ", EBS_INT1, 
            DCUTOIPPKT(msg)->ip_fragoff, 0);
#endif
        next_msg = DCUTOPACKET(msg)->frag_next;

        /* if netwrite is going to free the packet, unhook it from the   */
        /* fragment to prevent netwrite from freeing all the fragments   */
        if (!(flags & PKT_FLAG_KEEP))
            DCUTOPACKET(msg)->frag_next = (DCU)0;

        status = tc_netwrite(pi, port, msg, ipn, flags, wait_count);
        if (status)
        {
            if (!(flags & PKT_FLAG_KEEP))
                os_free_packet(next_msg);
            return(status);
        }

        msg = next_msg;
    }
    return(0);
}
#endif

#if (INCLUDE_TCP)
/* ********************************************************************   */
/* calculate length of tcp header and tcp data                            */
/* if hlen = -1 if should not set data_start or data_len                  */
word ipf_tcp_tlen(DCU msg, int hlen)        /*__fn__*/
{
word   tlen;
DCU    curr_msg;
PIPPKT pip;
int    size;

    curr_msg = msg;
    tlen = 0;

    /* probably a faster way to do this    */
    while (curr_msg)
    {
        pip = DCUTOIPPKT(curr_msg);
        tlen = (word)(tlen + net2hs(pip->ip_len) - IP_HLEN(pip));   

        if (hlen >= 0)
        {
            TOTAL_LL_HLEN_SIZE(size, msg_is_802_2(curr_msg), 
                               DCUTOCONTROL(msg).pi, curr_msg)
            /* fix these up for enqueue()   */
            DCUTOPACKET(curr_msg)->data_start = 
                (word)(size + IP_HLEN_BYTES + hlen);
            DCUTOPACKET(curr_msg)->data_len = (word) (net2hs(pip->ip_len) - 
                                              IP_HLEN(pip) - hlen);
            hlen = 0;       /* only first pkt has protocol header */
        }
        curr_msg = DCUTOPACKET(curr_msg)->frag_next;
    }

    return(tlen);
}
#endif      /* INCLUDE_TCP */

#endif      /* INCLUDE_FRAGMENT */

#if (INCLUDE_802_11)
void process_frame_mgmt(PFRAME_FORMAT_802_11 p802_11)
{
    switch (p802_11->frame_control & FRAME_SUBTYPE)
    {
    case FRAME_ST_ASS_REQ  :
    case FRAME_ST_ASS_RESP :
    case FRAME_ST_REASS_REQ :
    case FRAME_ST_REASS_RESP:
    case FRAME_ST_PROBE_REQ:
    case FRAME_ST_PROBE_RESP:
    case FRAME_ST_BEACON:
    case FRAME_ST_ATIM:
    case FRAME_ST_DISASS:
    case FRAME_ST_AUTH:
    case FRAME_ST_DEAUTH:
        break;
    }   /* end of switch */
}

void process_ctrl_mgmt(PFRAME_FORMAT_802_11 p802_11)
{
    switch (p802_11->frame_control & FRAME_SUBTYPE)
    {
    case FRAME_ST_PS:
    case FRAME_ST_RTS:
    case FRAME_ST_CTS:
    case FRAME_ST_ACK:
    case FRAME_ST_CF:
    case FRAME_ST_CF_END_ACK:
        break;
    }   /* end of switch */
}

/* returns whether packet has data   */
RTIP_BOOLEAN process_ctrl_data(PFRAME_FORMAT_802_11 p802_11)
{
RTIP_BOOLEAN has_data;

    has_data = FALSE;
    switch (p802_11->frame_control & FRAME_SUBTYPE)
    {
    case FRAME_ST_DATA:
    case FRAME_ST_DATA_ACK:
    case FRAME_ST_DATA_POLL:
    case FRAME_ST_DATA_ACK_POLL:
        has_data = TRUE;
        break;
    case FRAME_ST_NDATA:
    case FRAME_ST_NDATA_ACK:
    case FRAME_ST_NDATA_POL:
    case FRAME_ST_NDATA_ACK_POLL:
        break;
    }   /* end of switch */

    return(has_data);
}
#endif      /* INCLUDE_802_11 */

