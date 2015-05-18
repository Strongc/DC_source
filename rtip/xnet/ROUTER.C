/*                                                                      */
/* ROUTER.C - ROUTER code                                               */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*     This module contains code to handle routing packets.             */
/*     NOTE: router code inclusion is controlled by INCLUDE_ROUTER      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_IP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

#if (FRAG_ROUTER && INCLUDE_FRAG)
void tc_send_router_frags(PIFACE pi_out, DCU orig_msg, PFBYTE send_ip_addr);
#endif

#if (INCLUDE_ROUTER)
/* ********************************************************************       */
/* ROUTER ROUTINES                                                            */
/* ********************************************************************       */
/* process_router_options() - processes router options                        */
/*                                                                            */
/*   Processes options that a router is interested in.                        */
/*                                                                            */
/*   Returns TRUE if successful or FALSE if not (i.e. could not follow        */
/*   the strict routing requirements).  If returns FALSE, pi and send_ip_addr */
/*   might be set to invalid values.                                          */
/*                                                                            */
RTIP_BOOLEAN process_router_options(PIFACE *pi_out, PFBYTE send_ip_addr, /*__fn__*/
                               PIPPKT pip, int header_len_bytes)    /*__fn__*/
{
PIFACE pi;
PFBYTE pb;
int option_len;
int ptr;
int len;
    
    pb = (PFBYTE)&(pip->ip_verlen) + IP_HLEN_BYTES; /* point to code field */
                                                    /* of first option   */
    option_len = header_len_bytes - IP_HLEN_BYTES;

    /* options are of the form: code:length:pointer where each field   */
    /* is 1 byte                                                       */
    while (option_len > 0)
    {
        len = *(pb+1);
        ptr = *(pb+2) - 1;
        switch (*pb & 0x1f)     /* option number (mask out copy and option */
                                /* class fields   */
        {
            case END_OF_LIST_OPTION:    /* end of options */
                return(TRUE);

            case NOP_OPTION:            /* NOP */
                break;

            /* process record route option   */
            case RECORD_ROUTE_OPTION:
                /* insert local IP address into the route option but only    */
                /* if there is room, i.e. the ptr field is less then length  */
                /* field                                                     */
                if (len > ptr)
                {
                DEBUG_ERROR("add a route", NOVAR, 0, 0);
                    tc_mv4((PFBYTE)(pb+ptr), (*pi_out)->addr.my_ip_addr, 
                           IP_ALEN);    /* NOTE: insert IP address interface  */
                                        /*       sending out on   */
                    *(pb+2) += (byte)4; /* increment ptr by 4 */
                }
                else
                {
                    DEBUG_ERROR("DO NOT add a route", NOVAR, 0, 0);
                }
                break;

            /* process loose source route option   */
            case LOOSE_ROUTE_OPTION:
                if (len >= ptr)
                {
                    pi = tc_get_route((PFBYTE)(pb+ptr));
                    if (pi)
                    {
                        *pi_out = pi;
                        tc_mv4(send_ip_addr, (PFBYTE)(pb+ptr), 4);
                        tc_mv4((PFBYTE)(pb+ptr), (*pi_out)->addr.my_ip_addr, 4);  
                                        /* NOTE: insert IP address interface pkt   */
                                        /*       sending out on                    */
                        *(pb+2) += (byte)4;
                    }
                }
                break;

            /* process strict source route option   */
            case STRICT_ROUTE_OPTION:
                *pi_out = tc_get_route((PFBYTE)(pb+ptr));
                tc_mv4(send_ip_addr, (PFBYTE)(pb+ptr), 4);
                if (!(*pi_out))
                    return(FALSE);   /* did not find route */
                tc_mv4((PFBYTE)(pb+ptr), (*pi_out)->addr.my_ip_addr, 4);  
                                    /* NOTE: insert IP address interface pkt   */
                                    /*       sending out on                    */
                *(pb+2) += (byte)4;
                break;

            case TIMESTAMP_OPTION:
                break;
        }
        if ( (*pb & 0x1f) == NOP_OPTION)
        {
            option_len -= 1;
            pb += 1;                /* point to next option */
DEBUG_ERROR("after NOP - option len = ", EBS_INT1,
                option_len, 0);
        }
        else
        {
            option_len -= *(pb+1);
            pb += *(pb+1);              /* point to next option */
DEBUG_ERROR("after OPTION - option len = ", EBS_INT1,
                option_len, 0);
        }
    }

    return(TRUE);
}

/* ********************************************************************   */
/* forward an IP packet which was sent to us but we are not the final     */
/* destination                                                            */
void route_pkg(PIFACE pi, DCU msg, PIPPKT pip)  /*__fn__*/
{
PIFACE pi_out;
byte   send_ip_addr[IP_ALEN];
int    header_len_bytes;
int    header_len_words;

#if (!INCLUDE_KEEP_STATS)
    ARGSUSED_PVOID(pi)
#endif

    header_len_words = (pip->ip_verlen&0x0f) << 1;
    header_len_bytes = header_len_words << 1;

    /* never route a packet which is ethernet broadcast   */
    if (IS_ETH_ADDR_EQUAL(LL_DEST_ADDR(msg, pi), broadaddr))
    {
        os_free_packet(msg);        /* Free the input packet */
        return;
    }

#if (INCLUDE_NAT)
/* TBD   */
    if (is_address_nat_internal(DCUTOIPPKT(msg)->ip_src))
    {
/*      DEBUG_ERROR("route_pkg: call nat_modify_outgoing", NOVAR, 0, 0);   */
        nat_modify_outgoing(msg);
    }
    else if (is_address_nat_unique(DCUTOIPPKT(msg)->ip_dest))
    {
/*      DEBUG_ERROR("route_pkg: call nat_modify_incoming", NOVAR, 0, 0);   */
        nat_modify_incoming(msg);
    }
    else
    {
        DEBUG_ERROR("route_pkg: NAT is on but not sure how to modify",
            NOVAR, 0, 0);
        os_free_packet(msg);
        return;
    }
#endif

    /* get interface from routing table for output;   */
    pi_out = rt_get_iface(pip->ip_dest, (PFBYTE)send_ip_addr, 
                          (PANYPORT)0, (RTIP_BOOLEAN *)0);

    /* if cannot find a next hop or ttl has expired (NOTE: a packet   */
    /* coming in with TTL=1 will be dropped, normally a packet with   */
    /* TTL=0 will not be received)                                    */
    if ( !pi_out || (pip->ip_ttl <= 1) )
    {
        /* send ICMP TIME_EXCEEDED message   */
        if (pip->ip_ttl <= 0)
        {
            DEBUG_ERROR("ROUTER: time limit exceeded, i.e. ip_ttl ", NOVAR, 0, 0);
            tc_icmp_send(msg, ICMP_T_TIME_EXCEEDED, ICMP_C_TEXP_TTL, 0);
        }

        INCR_SNMP(IpOutNoRoutes)
        os_free_packet(msg);
        return;
    }

    /* decrement time to live   */
    pip->ip_ttl--;

    /* process router options   */
    if (header_len_bytes > IP_HLEN_BYTES)
    {
        /* process any IP options that are meant for a router   */
        if (!process_router_options(&pi_out, send_ip_addr, pip, 
                                    header_len_bytes))
        {
            INCR_SNMP(IpOutNoRoutes)

            /* drop the packet since could not follow the strict routing   */
            /* requirements                                                */
            os_free_packet(msg);
            return;
        }
    }

    /* replace source ethernet address with our own                      */
    /* NOTE: make sure this is done after processing the options since   */
    /*       the output interface might be change by SOURCE ROUTE option */
    SETUP_LL_HDR_SRC(pi_out, msg, pi_out->addr.my_hw_addr)

    /* we have changed packet due by changing TTL and possibly due to options   */
    /* specified so if the packet has an IP checksum (it is optional),          */
    /* recalculate it                                                           */
    if (pip->ip_cksum)
    {
        pip->ip_cksum = 0;
        pip->ip_cksum = tc_ip_chksum((PFWORD) &pip->ip_verlen, header_len_words);
    }

/*  DEBUG_ERROR("ROUTE PKG: pi = ", EBS_INT1, pi_out->ctrl.index, 0);            */
/*  DEBUG_ERROR("ROUTE PKG: dest addr = ", IPADDR, DCUTOIPPKT(msg)->ip_dest, 0); */

    /* NOTE: at this point we definatly have a pi since returned above   */
    /*       if did not and router option routine returns FALSE if       */
    /*       fails                                                       */
    /* Send the packet on.  Discard it when through.                     */
    /* Do not signal.                                                    */
#if (FRAG_ROUTER && INCLUDE_FRAG)
    if ( (DCUTOPACKET(msg)->length - sizeof(struct _ether)) > 
         pi_out->addr.mtu )
    {
        tc_send_router_frags(pi_out, msg, send_ip_addr);
    }
    else
#endif
    {
        tc_netwrite(pi_out, (PANYPORT)0, msg, send_ip_addr, NO_DCU_FLAGS, 
                        (word)(40*ks_ticks_p_sec()));
    }

    UPDATE_INFO(pi, ip_routed, 1)
    INCR_SNMP(IpForwDatagrams)
}

#if (FRAG_ROUTER && INCLUDE_FRAG)
/* ********************************************************************   */
void tc_send_router_frags(PIFACE pi_out, DCU orig_msg, PFBYTE send_ip_addr)
{
int    len;
PIPPKT pip;
int    hdr_len;  /* len of IP and ETHER header in orig msg */
DCU    msg;
int    maxdlen;     /* amt of data (including protocol header) to  */
                    /* send in each frag    */
int    iplen;

    pip = DCUTOIPPKT(orig_msg);
    hdr_len = IP_HLEN(pip) + ETH_HLEN_BYTES;
    maxdlen = (pi_out->addr.mtu - IP_HLEN(pip)) & ~7; 

#if (1)
    if ( (msg = ip_alloc_packet(pi_out, (PANYPORT)0, maxdlen+hdr_len, 0, 
                                FRAG_ALLOC)) == (DCU)0 )
#else
    if ( (msg = ip_alloc_packet(pi_out, (PANYPORT)0, MAX_PACKETSIZE, 0, 
                                FRAG_ALLOC)) == (DCU)0 )
#endif
    {
        DEBUG_ERROR("ERROR(rtip) - tc_send_router_frags - ip_alloc_packet failed", NOVAR, 0, 0);
        goto exit_proc;
    }

    /* copy the ethernet header, IP header and options to the first msg   */
    tc_movebytes(DCUTODATA(msg), DCUTODATA(orig_msg), hdr_len);

    DCUTOPACKET(msg)->ip_option_len = DCUTOPACKET(orig_msg)->ip_option_len;

    len = DCUTOPACKET(orig_msg)->length - hdr_len;
    msg = ipf_create_pkt(pi_out, (PANYPORT)0, msg, len, 0);
    if (!msg)
        goto exit_proc;

    /* fill in the data into the fragments, pass in buffer           */
    /* this includes protocol header and data (but not IP header and */
    /* options)                                                      */
    msg = ipf_fill_pkt(pi_out, msg, 
                       (PFBYTE)DCUTOIPPKT(orig_msg)+IP_HLEN(pip), len, 0);
    if (!msg)
        goto exit_proc;

    /* set up length for tc_netwrite()   */
    iplen = IP_HLEN(pip);
    DCUTOPACKET(msg)->length = maxdlen + iplen + ETH_HLEN_BYTES;
    /* send all the fragments; will free packets after sent   */
    ipf_netwrite(pi_out, (PANYPORT)0, msg, send_ip_addr, NO_DCU_FLAGS, 
                 0);
exit_proc:
    os_free_packet(orig_msg);                /* Free the input message */
}
#endif      /* FRAG_ROUTER & INCLUDE_FRAG */
#endif      /* INCLUDE_ROUTER */


