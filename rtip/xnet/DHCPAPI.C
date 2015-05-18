/*
    DHCPAPI.C - DHCP client API functions

    EBS - RTIP

    Copyright Peter Van Oudenaren , 1996
    All rights reserved.
    This code may not be redistributed in source or linkable object form
    without the consent of its author.

    Module description:
    This module provides functions necessary to create a DHCP client
    application.
*/

#define DIAG_SECTION_KERNEL DIAG_SECTION_DHCP


#include "sock.h"
#include "rtip.h"
#include "rtipapi.h"
#include "rtipext.h"

#if (INCLUDE_DHCP_CLI)
#include "dhcp.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_RENEW        1
#define DEBUG_ADDRESS      0

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */
#define broadcast_addr ip_ffaddr

/* ********************************************************************   */
/* EXTERNALS                                                              */
/* ********************************************************************   */
KS_EXTERN_GLOBAL_CONSTANT DHCP_conf KS_FAR dhcp_default_conf;
extern DHCP_callback_fp KS_FAR choose_dhcp_offer;

/* ********************************************************************   */
/* FUNCTIONS                                                              */
/* basic API functions                                                    */
/* ********************************************************************   */


/* ********************************************************************   */
void xn_init_dhcp_conf(PFDHCP_conf conf)
{
    /* set up call-back routine to default   */
    choose_dhcp_offer = (DHCP_callback_fp)(0);

    /* set conf to default   */
    tc_movebytes((PFBYTE)conf, &dhcp_default_conf, sizeof(struct DHCP_conf));
/*  DEBUG_LOG("request std params = ", LEVEL_2, DINT1, (dword)(conf->request_std_params), 0);   */
/*  DEBUG_LOG("apply std params = ", LEVEL_2, DINT1, (dword)(conf->apply_std_params), 0);       */
}

/* ********************************************************************   */
int xn_dhcp(int iface_no, PFDHCP_session session, PFDHCP_conf conf)
{
int dhcpc_sock;
int num_offers;
int num_acknaks;
int chosen_dhcp_offer = 0;
byte msg_type;
int n;
RTIP_BOOLEAN cold_dhcp = FALSE;  /* cold means you want IP address */
PIFACE pi;

    DEBUG_LOG("xn_dhcp", LEVEL_2, NOVAR, 0, 0);

/*  DEBUG_LOG("request std params = ", LEVEL_2, DINT1, (dword)(conf->request_std_params), 0);   */
/*  DEBUG_LOG("apply std params = ", LEVEL_2, DINT1, (dword)(conf->apply_std_params), 0);       */

    if (!allow_udp_broadcasts)
    {
        DEBUG_ERROR("WARNING: dhcp will not work with broadcast disabled",
            NOVAR, 0, 0);
    }

    /* NOTE: cold_dhcp=TRUE means do not know IP address, therefore, send   */
    /*       discover then request                                          */
    /* NOTE: cold_dhcp=FALSE means know IP address, therefore, just send    */
    /*       an inform (which is actually a request)                        */

    /* set null address to the interface so nxt_bc_iface will work   */
    set_ip(iface_no, (PFBYTE) ip_nulladdr, (PFCBYTE) 0);

    if (tc_cmp4(conf->client_ip_addr, ip_nulladdr, IP_ALEN))
        cold_dhcp = TRUE;

    session->iface_no = iface_no;

start_dhcp_stuff:

    /* initialize packets:   */
    init_dhcp_packet(&(session->outmsg));
    for (n=0; n<CFG_DHCP_OFFERS; n++)
        init_dhcp_packet(&(session->replies[n]));

    if (session->packet_style != DHCP_RFC &&      /* if style not already */
        session->packet_style != DHCP_MICROSOFT)    /* assigned, use default */
        session->packet_style = DHCP_DEF_PKT_STYLE;

    /* allocate a socket and bind to it   */
    dhcpc_sock = open_dhcp_socket(iface_no);

    if (dhcpc_sock < 0)
    {
        DEBUG_ERROR("xn_dhcp failed - errno = ",EBS_INT1,xn_getlasterror(),0);
        return -1;
    }

    if (cold_dhcp == TRUE)
    {
        DEBUG_LOG("COLD DHCP", LEVEL_2, NOVAR, 0, 0);
        if (format_dhcp_discover(&(session->outmsg), iface_no, conf) < 0)
            return(-1);     /* NOTE: errno already set */

        /* send out the DHCPDISCOVER, waits for replies, and retries   */
        /* if necessary                                                */
        num_offers = do_dhcp_packet(&(session->outmsg), session->replies,
                                    dhcpc_sock, broadcast_addr,
                                    CFG_DHCP_OFFERS, CFG_DHCP_TIMEOUT,
                                    CFG_DHCP_RETRIES, DHCPOFFER_MC);
        if (num_offers == 0)
        {
            DEBUG_ERROR("Server not responding to DHCPDISCOVER.", NOVAR, 0, 0);
            closesocket(dhcpc_sock);
            return set_errno(EDHCPSERVNORESP);
        }
        if (num_offers == -1)
        {
            DEBUG_ERROR("DHCP receive error.", NOVAR, 0, 0);
            closesocket(dhcpc_sock);
            return -1;
        }

        /* select a DHCPOFFER using user callback or default routine   */
        if (choose_dhcp_offer != (DHCP_callback_fp)(0))
            chosen_dhcp_offer = (*choose_dhcp_offer)(session->replies, num_offers,
                conf);
        else
            chosen_dhcp_offer = dhcp_choose_offer(session->replies, num_offers,
                conf);
        DEBUG_LOG("chosen_dhcp_offer = ", LEVEL_2, DINT1,
            (dword)chosen_dhcp_offer, 0);
        if (chosen_dhcp_offer == -1)
        {
            DEBUG_ERROR("No DHCPOFFERs meet user criterion.", NOVAR, 0, 0);
            closesocket(dhcpc_sock);
            return -1;
        }

        /* extract server id from chosen offer   */
        get_dhcp_op(&(session->replies[chosen_dhcp_offer]), SERVER_ID,
            sizeof(dword), (void*)session->server_ip);
    }
    else  /* "WARM" DHCP - send an inform */
    {
        DEBUG_LOG("WARM DHCP", LEVEL_2, NOVAR, 0, 0);
    }

    /* loop thru styles until we get an ACK   */
    for (n=0; n<DHCP_REQ_STYLES; n++)
    {
        /* format DHCPREQUEST message   */
        if (cold_dhcp == TRUE)
        {
            format_dhcp_request(&(session->outmsg),
                &(session->replies[chosen_dhcp_offer]),
                session->packet_style);
        }
        else
        {
            if (format_dhcp_inform(&(session->outmsg), iface_no, conf) < 0)
                return(-1);     /* errno already set */
        }

        /* send out DHCPREQUEST and wait for a DHCPACK -- we just want one.   */
        num_acknaks = do_dhcp_packet(&(session->outmsg), session->replies,
                                     dhcpc_sock, broadcast_addr, 1,
                                     CFG_DHCP_TIMEOUT, CFG_DHCP_RETRIES,
                                     DHCPACK_MC|DHCPNAK_MC);
        if (num_acknaks == 0)
        {
            DEBUG_ERROR("Server not responding to DHCPREQUEST: cold_dhcp = ",
                EBS_INT1, cold_dhcp, 0);

            if (cold_dhcp == FALSE)
            {
                DEBUG_ERROR("Trying cold DHCP . . .", NOVAR, 0, 0);
                closesocket(dhcpc_sock);
                cold_dhcp = TRUE;
                goto start_dhcp_stuff;
            }
            else
            {
                closesocket(dhcpc_sock);
                return set_errno(EDHCPSERVNORESP);
            }
        }
        if (num_acknaks == -1)
        {
            DEBUG_ERROR("DHCP receive error.", NOVAR, 0, 0);
            closesocket(dhcpc_sock);
            return -1;
        }

        /* get message type of response   */
        get_dhcp_op(&(session->replies[0]), DHCP_MSG_TYPE, sizeof(byte),
                    (void*)&msg_type);

        if (msg_type == DHCPNAK)
        {   /* try a different style request */
            if (session->packet_style == DHCP_MICROSOFT)
                session->packet_style = DHCP_RFC;
            else
                session->packet_style = DHCP_MICROSOFT;
        }
        else
        {
            break;
        }
    }       /* loop thru styles */

    /* return parameters from DHCPACK.   */
    tc_mv4((PFBYTE)session->client_ip,
           (PFBYTE)&(session->replies[0].yiaddr), 4);

    session->lease_time = (dword)-1;	/*This is OK: -1 = 0xFFFFFFFF specifies an infinite lease time */
															/*and shouldn't cause any problems - JLA IO Technologies 2005-09-22*/
    get_dhcp_op(&(session->replies[0]), IP_LEASE, sizeof(dword),
                (void*)&(session->lease_time));
    session->lease_time = ntohl(session->lease_time);
#if (DEBUG_ADDRESS)
        DEBUG_ERROR("xn_dhcp: set client_ip from yiaddr: ",
            IPADDR, session->client_ip, 0);
#endif

    /* extract the parameters from the reply and store them in    */
    /* session->params structure                                  */
    get_dhcp_params(&(session->replies[0]), &session->params);
#if (DEBUG_RENEW)
    DEBUG_ERROR("xn_dhcp: lease time = ", DINT1, session->lease_time, 0);
#endif

    /* set up IP address and apply MASK                                 */
    /* NOTE: mask is applied by xn_dhcp since it need to call xn_set_ip */
    /*       regardless                                                 */
    if ( (session->params.subnet_mask) && conf->apply_std_params)
    {
        DEBUG_LOG("subnet_mask = ", LEVEL_2, IPADDR,
            session->params.subnet_mask, 0);
#if (DEBUG_ADDRESS)
        DEBUG_ERROR("xn_dhcp: call set_ip address: ",
            IPADDR, session->client_ip, 0);
#endif
        if (set_ip(iface_no, (PFBYTE)session->client_ip,
                   (PFCBYTE)session->params.subnet_mask) < 0)
        {
            return(-1);
        }
        CB_DHCP_NEW_IP(iface_no);
    }
    else
    {
        DEBUG_LOG("subnet_mask = [not returned]", LEVEL_2, NOVAR, 0, 0);
#if (DEBUG_ADDRESS)
        DEBUG_ERROR("xn_dhcp: call set_ip address: ",
            IPADDR, session->client_ip, 0);
#endif
        if (set_ip(iface_no, (PFBYTE)session->client_ip, (PFCBYTE)0) < 0)
        {
            return(-1);
        }
        CB_DHCP_NEW_IP(iface_no);
    }

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
        return(-1);

    /* Update DHCP table info in iface. For lease expiration times, use   */
    /* the defaults (.5,.875) unless the server says otherwise            */
    tc_mv4(pi->addr.dhcp_server_ip_addr, session->server_ip,4);
    pi->addr.lease_time = pi->addr.orig_lease_time = session->lease_time;

#if (INCLUDE_DHCP_RENEW_NO_BLOCK)
    pi->addr.dhcp_extend_status = DHCP_EXTEND_IDLE;
#endif

#if (DEBUG_RENEW)
    DEBUG_ERROR("xn_dhcp: lease time = ", DINT1, pi->addr.lease_time, 0);
#endif
    if(get_dhcp_op(&(session->replies[chosen_dhcp_offer]), RENEWAL_TIME,
            sizeof(dword), (void*)&pi->addr.renew_lease_time) <0)
    {
        pi->addr.renew_lease_time = session->lease_time >> 1;
    }
    else
    {
        pi->addr.renew_lease_time = ntohl(pi->addr.renew_lease_time);
    }

    if(get_dhcp_op(&(session->replies[chosen_dhcp_offer]), REBINDING_TIME,
            sizeof(dword), (void*)&pi->addr.rebind_lease_time) <0)
    {
        pi->addr.rebind_lease_time = session->lease_time >> 3;
    }
    else
    {
        pi->addr.rebind_lease_time = pi->addr.orig_lease_time-
            ntohl(pi->addr.rebind_lease_time);
    }
#if (DEBUG_RENEW)
    DEBUG_ERROR("xn_dhcp: renew, rebind lease time = ", DINT1,
        pi->addr.renew_lease_time, pi->addr.rebind_lease_time);
#endif


    /* apply ops if told to   */
    if (conf->apply_std_params)
    {
        DEBUG_LOG("applying standard params", LEVEL_2, NOVAR, 0, 0);
        apply_dhcp_std_ops(session, iface_no);
    }
    else
    {
        DEBUG_LOG("not applying standard params", LEVEL_2, NOVAR, 0, 0);
    }
    closesocket(dhcpc_sock);
    return 0;
}

/* ********************************************************************   */
int xn_extend_dhcp_lease(PFDHCP_session session, dword lease_time)
{
int dhcpc_sock;
int num_acknaks;

    DEBUG_LOG("lease time passed to extend lease = ", LEVEL_2, DINT1,
        lease_time, 0);

    dhcpc_sock = extend_dhcp_lease_start(session, lease_time);

    /* send DHCP packet and wait for response; does retries if no   */
    /* response                                                     */
#if (DHCP_RENEW_RFC_NETADDR)
    num_acknaks = do_dhcp_packet(&(session->outmsg), session->replies,
        dhcpc_sock, session->server_ip, 1, CFG_DHCP_TIMEOUT,
        CFG_DHCP_RETRIES, DHCPACK_MC|DHCPNAK_MC);
#else
    num_acknaks = do_dhcp_packet(&(session->outmsg), session->replies,
        dhcpc_sock, broadcast_addr, 1, CFG_DHCP_TIMEOUT,
        CFG_DHCP_RETRIES, DHCPACK_MC|DHCPNAK_MC);
#endif

    if (num_acknaks == -1)
    {
        DEBUG_ERROR("DHCP receive error.", NOVAR, 0, 0);
        closesocket(dhcpc_sock);
        return -1;
    }

    if (num_acknaks == 0)
    {
        DEBUG_ERROR("Server not responding to DHCPREQUEST.", NOVAR, 0, 0);
        closesocket(dhcpc_sock);
        return set_errno(EDHCPSERVNORESP);
    }

    /* responses are in, now process them   */
    process_extend_results(session, dhcpc_sock);

    return 0;
}

/* ********************************************************************   */
int xn_dhcp_release(PFDHCP_session session)
{
int dhcpc_sock;
PIFACE pi;

    format_dhcp_decrel(&(session->outmsg), session->server_ip,
        session->client_ip, DHCPRELEASE);
    dhcpc_sock = open_dhcp_socket(session->iface_no);
    if (send_dhcp_packet(&(session->outmsg), dhcpc_sock,
                         session->server_ip) < 0)
    {
        return(-1);
    }

    pi =  tc_get_local_pi(session->client_ip);
    if (pi && (pi->open_count > 0))
    {
        OS_CLAIM_IFACE(pi, DHCP_RELEASE_CLAIM_IFACE)
        pi->addr.orig_lease_time = 0;

        /* invalidate all UDP and TCP sockets bound to address just   */
        /* released                                                   */
#    if (INCLUDE_TCP)
        tcp_invalidate_sockets(pi->addr.my_ip_addr);
#    endif
#    if (INCLUDE_UDP)
        udp_invalidate_sockets(pi->addr.my_ip_addr);
#    endif
        tc_mv4(pi->addr.my_ip_addr, ip_nulladdr, IP_ALEN);
        OS_RELEASE_IFACE(pi)
    }

    return(closesocket(dhcpc_sock));
}

/* ********************************************************************   */
int xn_dhcp_decline(PFDHCP_session session)
{
int dhcpc_sock;

    format_dhcp_decrel(&(session->outmsg), session->server_ip,
                       session->client_ip, DHCPDECLINE);
    dhcpc_sock = open_dhcp_socket(session->iface_no);
    if (send_dhcp_packet(&(session->outmsg), dhcpc_sock,
                         session->server_ip) < 0)
    {
        return(-1);
    }
    return(closesocket(dhcpc_sock));
}

/* ********************************************************************   */
/* API functions for advanced use of DHCP:                                */
/* ********************************************************************   */

/* ********************************************************************   */
void xn_set_dhcp_callback(DHCP_callback_fp fp)
{
    choose_dhcp_offer = fp;
}

/* ********************************************************************   */
int xn_get_dhcp_op(PFDHCP_session session, byte op_id, byte bufsize, void *vpdest)
{
    return get_dhcp_op(&(session->replies[0]), op_id, bufsize, vpdest);
}

/* ********************************************************************   */
void  xn_set_dhcp_conf_op(PFDHCP_conf conf,PFDHCP_cparam cpdata)
{
    conf->cplist_entries +=1;
    cpdata->next = (void *)conf->cplist;
    conf->cplist = cpdata;
}

#endif      /* INCLUDE_DHCP_CLI */

/* END OF DHCPAPI.C   */



