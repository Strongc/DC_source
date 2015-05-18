/*                                                                      */
/* IGMP.C - IGMP functions                                              */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/* Module description:                                                  */
/*   This module provides suppport for the IGMP protocol.  It supports  */
/*   host side IGMP only.  RTIP is not a multicast router.              */

#define DIAG_SECTION_KERNEL DIAG_SECTION_IGMP


#include "sock.h"
#include "rtip.h"

#define DISPLAY_IGMP_TIMER 0
#define IGMPV2_DEBUG 0

void igmp_reset_timer(PIFACE pi, PFBYTE mcast_ip_addr);
#if (INCLUDE_IGMP_V2)
void igmp_start_timers(PIFACE pi, DCU msg);
#else
void igmp_start_timers(PIFACE pi);
#endif
#if (INCLUDE_IGMP_V2)
void igmp_leave_group(PIFACE pi, PFBYTE mcast_ip_addr);
#endif


#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ip_igmp_all_hosts[IP_ALEN];
#endif

#if (INCLUDE_IGMP_V2)
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ip_igmp_all_routers[IP_ALEN];
#endif


#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)


/* ********************************************************************   */
/* IGMP SEND REPORT                                                       */
/* ********************************************************************   */

/* ********************************************************************   */
/* igmp_setup_ip_header() - set up IP header for an IGMP message          */
/*                                                                        */
/*   Sets up IP header for an IGMP message.  The parameter pip            */
/*   points to the IP header.  The multicast address to send to           */
/*   is dest_ip.  The message will be sent on interface pi.               */
/*                                                                        */
/*   Returns nothing.                                                     */
/*                                                                        */

void igmp_setup_ip_header(DCU msg, PIFACE pi, PFBYTE dest_ip)
{
PIPPKT pip;

    /* set up ethernet header   */
    SETUP_LL_HDR(msg, pi, EIP_68K, FALSE, 0)

#if (INCLUDE_IPV4)
    /*  setup IP header   */
    pip = DCUTOIPPKT(msg);
    pip->ip_proto = PROTIGMP;
    tc_mv4(pip->ip_dest, dest_ip, IP_ALEN);
    tc_mv4(pip->ip_src, pi->addr.my_ip_addr, IP_ALEN);
#if (INCLUDE_IGMP_V2)
    setup_ipv4_header(msg, (PANYPORT)0, IGMP_TLEN_BYTES-ETH_HLEN_BYTES + 4, 0,   /* 4 reqrd for Router Alert */
              FALSE, 0, TRUE);                   /* will set ttl to 1 */
#else
    setup_ipv4_header(msg, (PANYPORT)0, IGMP_TLEN_BYTES-ETH_HLEN_BYTES, 0,
              FALSE, 0, TRUE);                   /* will set ttl to 1 */
#endif
#endif
}

/* ********************************************************************   */
/* igmp_send_report() - send an IGMP report                               */
/*                                                                        */
/*    Sends an IGMP report to multicast address ip_addr over interface    */
/*    pi.                                                                 */
/*                                                                        */
/*    NOTE: the send could fail but this routine does not check for       */
/*          success/failure                                               */
/*                                                                        */
/*    Returns nothing.                                                    */
/*                                                                        */

void igmp_send_report(PIFACE pi, PFBYTE ip_addr)
{
DCU msg;
PIGMPPKT igmp;

    /* never send a report to loop back    */
    if (!pi || !pi->pdev || pi->pdev->device_id == LOOP_DEVICE)
        return;

    /* never send report for ALL HOSTS GROUP   */
    if (tc_cmp4(ip_addr, (PFBYTE)ip_igmp_all_hosts, IP_ALEN))
        return;

    DEBUG_LOG("send report - IP addr = ", LEVEL_3, IPADDR, ip_addr, 0);
#if (IGMPV2_DEBUG)
    DEBUG_ERROR("send report", NOVAR, 0,0);
#endif
    if ( (msg = ip_alloc_packet(pi, (PANYPORT)0, IGMP_TLEN_BYTES, 0,
                IGMP_ALLOC)) == (DCU)0 )
    {
    DEBUG_ERROR("ERROR(rtip) - tc_tcp_reset() - os_alloc_packet failed", 
            NOVAR, 0, 0);
    return;
    }

#if (IGMPV2_DEBUG)
    DEBUG_ERROR("set up header",NOVAR,0,0);
#endif
    igmp_setup_ip_header(msg, pi, ip_addr);

    igmp = DCUTOIGMPPKT(msg);

#if (INCLUDE_IGMP_V2)
    if (pi->mcast.mcast_querier == IGMP_V1) /* router is Version 1 */
        igmp->igmp_type = IGMP_REPORT1;             /* report must be Version 1 format */
    else
        igmp->igmp_type = IGMP_REPORT2;        /* always set to 0 for reports */
    igmp->igmp_max_resp_time = 0;
#else
    igmp->igmp_vertype = (1<<4) | (IGMP_REPORT);
    igmp->igmp_unused = 0;
#endif
    tc_mv4(igmp->igmp_ip_addr, ip_addr, IP_ALEN);

    /* compute checksum   */
    igmp->igmp_chk = 0;
#if (INCLUDE_IGMP_V2)
    igmp->igmp_chk = tc_ip_chksum((PFWORD)&igmp->igmp_type, IGMP_HLEN);
#else
    igmp->igmp_chk = tc_ip_chksum((PFWORD)&igmp->igmp_vertype, IGMP_HLEN);
#endif


    /* Call netwrite to send it through - netwrite will discard it   */
#if (INCLUDE_IGMP_V2)
    DCUTOPACKET(msg)->length = IGMP_TLEN_BYTES + 4; /* need to account for IP Router Alert */
#else
    DCUTOPACKET(msg)->length = IGMP_TLEN_BYTES;
#endif
#if (IGMPV2_DEBUG)
    DEBUG_ERROR("call netwrite",NOVAR,0,0); /* required for IGMP V2 */
#endif
    tc_netwrite(pi, (PANYPORT)0, msg, (PFBYTE)ip_addr, NO_DCU_FLAGS, 0);
}

#if (INCLUDE_IGMP_V2)
/* ********************************************************************   */
/* igmp_leave_group() - send an IGMP Leave Group Message                  */
/*                                                                        */
/*    Sends an IGMP Leave Group Message to all-routers Multicast group    */
/*    for group mcast_ip_addr on interface pi.                            */
/*                                                                        */
/*    NOTE: the send could fail but this routine does not check for       */
/*          success/failure                                               */
/*                                                                        */
/*    Returns nothing.                                                    */
/*                                                                        */

void igmp_leave_group(PIFACE pi, PFBYTE mcast_ip_addr)
{
DCU msg;
PIGMPPKT igmp;

    if ( (msg = ip_alloc_packet(pi, (PANYPORT)0, IGMP_TLEN_BYTES, 0,
                IGMP_ALLOC)) == (DCU)0 )
    {
        DEBUG_ERROR("ERROR(rtip) - tc_tcp_reset() - os_alloc_packet failed", 
            NOVAR, 0, 0);
        return;
    }

    igmp_setup_ip_header(msg, pi, (PFBYTE)ip_igmp_all_routers);

    igmp = DCUTOIGMPPKT(msg);


    igmp->igmp_type = IGMP_LEAVE;
    igmp->igmp_max_resp_time = 0;
    tc_mv4(igmp->igmp_ip_addr, mcast_ip_addr, IP_ALEN);

    /* compute checksum   */
    igmp->igmp_chk = 0;
    igmp->igmp_chk = tc_ip_chksum((PFWORD)&igmp->igmp_type, IGMP_HLEN);


    /* Call netwrite to send it through - netwrite will discard it   */
    DCUTOPACKET(msg)->length = IGMP_TLEN_BYTES + 4; /* reqrd for Router Alert option */
    tc_netwrite(pi, (PANYPORT)0, msg, (PFBYTE)ip_igmp_all_routers, NO_DCU_FLAGS, 0);

}

#endif

/* ********************************************************************   */
/* IGMP INTERPRET                                                         */
/* ********************************************************************   */

/* ********************************************************************   */
/* tc_igmp_interpret() - process IGMP input packets                       */
/*                                                                        */
/*    Processes IGMP input packets (QUERY and REPORTS) input on           */
/*    interface pi.  The parmater msg contains the input packet.          */
/*    If a report comes in any running timers for the report are          */
/*    stopped (the timer was started when a query comes in; when          */
/*    the timer expires the report is sent).                              */
#if (INCLUDE_IGMP_V2)
/*    If the packet is a query, a timer is started as long as a timer   */
/*    is not already running or if a timer is running but the requested */
/*    max response time is less than the remaining time on the timer.   */
/*    When the timer expires, a report is sent as long as a report for  */
/*    the same multicast address has not been received.                 */
#else
/*    If the packet is a query, a timer is started as long as a timer   */
/*    is not already running. When the timer expires, a report is sent  */
/*    as long as a report for the same multicast address has not been   */
/*    received.                                                         */
#endif
/*    NOTE: queries are sent by a multicast router; reports are sent      */
/*          by other hosts on the same network in response to query.      */
/*    NOTE: The multicast router does not need to know which or how       */
/*          many hosts are listening on a multicast address. The router   */
/*          only needs to know if any hosts are listening on the address. */
/*    NOTE: the packet is freed                                           */
/*                                                                        */
/*    Returns nothing                                                     */

void tc_igmp_interpret(PIFACE pi, DCU msg)                      /*__fn__*/
{
PIGMPPKT igmp;
PIPPKT   pip;

    igmp = DCUTOIGMPPKT(msg);
    pip  = DCUTOIPPKT(msg);

    /* verify checksum   */
#if (INCLUDE_IGMP_V2)
    if (tc_ip_chksum((PFWORD)&igmp->igmp_type, IGMP_HLEN))
#else
    if (tc_ip_chksum((PFWORD)&igmp->igmp_vertype, IGMP_HLEN))
#endif
    {
    DEBUG_ERROR("tc_igmp_interpret - IP Checksum Error", NOVAR, 0, 0);
        os_free_packet(msg);
        return;
    }

    /* silently dicard message that has a multicast addresses in   */
    /* its source IP address                                       */
    if (!IP_ISMCAST_ADDR(pip->ip_src[0]))
    {
#if (INCLUDE_IGMP_V2)
        switch (igmp->igmp_type)
#else
        switch (igmp->igmp_vertype & 0x0f)
#endif
        {
            case IGMP_QUERY:
                DEBUG_LOG("tc_igmp_interpret - query", LEVEL_3, NOVAR, 0, 0);
#if (INCLUDE_IGMP_V2)
#if (IGMPV2_DEBUG)
                DEBUG_ERROR("query recv'd",NOVAR,0,0);
#endif
                if ((tc_cmp4(igmp->igmp_ip_addr, ip_nulladdr, 4)) || 
                    (IP_ISMCAST_ADDR(igmp->igmp_ip_addr[0])))
#if (IGMPV2_DEBUG)
                    DEBUG_ERROR("going to start timers",NOVAR,0,0);
#endif
                    igmp_start_timers(pi, msg);
#else
                if (tc_cmp4(pip->ip_dest, ip_igmp_all_hosts, 4))
                    igmp_start_timers(pi);
#endif
                break;
#if (INCLUDE_IGMP_V2)
            case IGMP_REPORT1:
            case IGMP_REPORT2:
#else
            case IGMP_REPORT:
#endif
                DEBUG_LOG("tc_igmp_interpret - report", LEVEL_3, NOVAR, 0, 0);
#if (INCLUDE_IGMP)
            /* this check required for version 1                      */
            /* but specifically not supposed to be here for version 2 */
                if (tc_cmp4(pip->ip_dest, igmp->igmp_ip_addr, 4))
#endif
                    igmp_reset_timer(pi, igmp->igmp_ip_addr);
                break;
            default:
                break;          /* silently discard any others */
        }
    }
    os_free_packet(msg);
}

/* ********************************************************************   */
/* IGMP TIMER                                                             */
/* ********************************************************************   */

/* ********************************************************************   */
/* igmp_timer() - IGMP timer                                              */
/*                                                                        */
/*   IGMP timer routine: called every second to decrement IGMP            */
/*   timers and to send a IGMP report for every timer which has           */
/*   expired.                                                             */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void igmp_timer(void)
{
int iface_no;
int i;
PIFACE pi;

#if (DISPLAY_IGMP_TIMER)
    DEBUG_LOG("igmp_timer enter", LEVEL_3, NOVAR, 0, 0);
#endif
    for (iface_no = 0; iface_no < CFG_NIFACES; iface_no++)
    {
        pi = (PIFACE) tc_ino2_iface(iface_no, DONT_SET_ERRNO);
        if (!pi || !pi->open_count)
            continue;

        OS_CLAIM_IFACE(pi, IGMP_TIMER_CLAIM_IFACE)

        if (pi->open_count <= 0)
        {
            OS_RELEASE_IFACE(pi)
            continue;
        }

#if (INCLUDE_IGMP_V2)
    if (pi->mcast.router1_present_tmr > 0)          /* if any time left on router V1 */
        {                                                                                       /* present then decr and check */
            if (--pi->mcast.router1_present_tmr <= 0)
                pi->mcast.mcast_querier = IGMP_V2;  /* haven't heard V1 queries in quite a while */
        }
#endif


        for (i=0; i<pi->mcast.lenmclist; i++)
        {
            /* decrement timer by 1 second;    */
            if (pi->mcast.report_timer[i] > 0)
            {
                DEBUG_LOG("igmp_timer - len, report_timer ", LEVEL_3, EBS_INT2, 
                    pi->mcast.lenmclist, pi->mcast.report_timer);
                pi->mcast.report_timer[i]--;

                /* if timer expired, send report   */
                if (pi->mcast.report_timer[i] <= 0)
                {
                    DEBUG_LOG("tc_igmp_timer - timer expired - send report", 
                        LEVEL_3, NOVAR, 0, 0);
                OS_RELEASE_IFACE(pi)
#if (INCLUDE_IGMP_V2)
#if (INCLUDE_RUN_TIME_CONFIG)
                if (!tc_cmp4((PFBYTE)&(pi->mcast.mclist_ip[i*IP_ALEN]), 
                             (PFBYTE)ip_igmp_all_hosts, 
                             IP_ALEN))
#else
                if (!tc_cmp4((PFBYTE)&(pi->mcast.mclist_ip[i][0]), 
                             (PFBYTE)ip_igmp_all_hosts, 
                             IP_ALEN))
#endif
                {
                    pi->mcast.last_host_toreply[i] = TRUE; /* last one to send a report */
                }

#if (IGMPV2_DEBUG)
                DEBUG_ERROR("timer expired i =",EBS_INT1,i,0);
#endif
#endif

#if (INCLUDE_RUN_TIME_CONFIG)
                igmp_send_report(pi, (PFBYTE)
                                     &(pi->mcast.mclist_ip[i*IP_ALEN]));
#else
                igmp_send_report(pi, (PFBYTE)
                                     &(pi->mcast.mclist_ip[i][0]));
#endif
                OS_CLAIM_IFACE(pi, IGMP_TIMER_CLAIM_IFACE)
                    pi->mcast.report_timer[i] = 0;
                }
            }
        }       /* end of loop thru mcast addrs */

    OS_RELEASE_IFACE(pi)

    }               /* end of loop thru ifaces */
#if (DISPLAY_IGMP_TIMER)
    DEBUG_LOG("igmp_timer exit", LEVEL_3, NOVAR, 0, 0);
#endif
}

/* ********************************************************************   */
/* igmp_start_timers() - starts report timers for an interface            */
/*                                                                        */
/*    Starts IGMP report timers for all multicast addresses listening     */
/*    to on an interface.  If a timer is already running, it is not       */
#if (INCLUDE_IGMP_V2)
/*    restarted unless the max response time is less than the time    */
/*    remaining on the timer.                                         */
#else
/*    restarted.   */
#endif
/*                     */
/*    Returns nothing. */
/*                     */

#if (INCLUDE_IGMP_V2)
void igmp_start_timers(PIFACE pi, DCU msg)
#else
void igmp_start_timers(PIFACE pi)
#endif
{
int i;
#if (INCLUDE_IGMP_V2)
PIGMPPKT igmp;
#endif

#if (INCLUDE_IGMP_V2)
#if (IGMPV2_DEBUG)
    DEBUG_ERROR("I'm at Start_Timers",NOVAR,0,0);
#endif
    
    igmp = DCUTOIGMPPKT(msg);

    if (igmp->igmp_max_resp_time == 0)
    {
        pi->mcast.mcast_querier = IGMP_V1; /*this query is IGMP_V1 style */
        pi->mcast.router1_present_tmr =  VERS1_ROUTER_PRESENT_TMO; /*can't send V2 msgs while positive */
#if (IGMPV2_DEBUG)
        DEBUG_ERROR("My querier is V1", NOVAR,0,0);
#endif
    }
#endif

    OS_CLAIM_IFACE(pi, IGMP_STIMER_CLAIM_IFACE)

#if (INCLUDE_IGMP_V2)
    if ( (pi->mcast.mcast_querier == IGMP_V2) &&      /* check for group */
         (!tc_cmp4((PFBYTE)&(igmp->igmp_ip_addr),     /* specific query */
                     (PFBYTE)ip_nulladdr, IP_ALEN))     )
    {
#endif
    for (i=0; i<pi->mcast.lenmclist; i++)
    {
#if (INCLUDE_IGMP_V2)
#if (INCLUDE_RUN_TIME_CONFIG)
        if  (tc_cmp4((PFBYTE)&(pi->mcast.mclist_ip[i*IP_ALEN]), 
                     (PFBYTE)igmp->igmp_ip_addr, IP_ALEN)) 
#else
        if  (tc_cmp4((PFBYTE)&(pi->mcast.mclist_ip[i][0]), 
                     (PFBYTE)igmp->igmp_ip_addr, IP_ALEN)) 
#endif
#else
        /* start timer; if timer is already running do not restart it   */
        if (pi->mcast.report_timer[i] <= 0)
#endif
        {
#if (INCLUDE_IGMP_V2)
#if (IGMPV2_DEBUG)
                DEBUG_ERROR("timer V2 = ",DINT1,pi->mcast.report_timer[i],0);
                DEBUG_ERROR("i = ",EBS_INT1,i,0);
#endif
                if ( (pi->mcast.report_timer[i] <= 0) ||    /* timer not running or */
                     ( (pi->mcast.report_timer[i] > 0) &&  /* timer is running but max resp time */
                       (igmp->igmp_max_resp_time < (pi->mcast.report_timer[i]*10)))) /* less than time left */
                {
                    pi->mcast.report_timer[i] = 
                        (((int)((ks_get_ticks()*10) % igmp->igmp_max_resp_time))/10) + 1;
#if (IGMPV2_DEBUG)
                    DEBUG_ERROR("Timer started V2 =",DINT1,pi->mcast.report_timer[i],0);
#endif
                }
            break;    
            }
        }
    }
        
    else   /* version 1 or 2 general query */
    {
#if (IGMPV2_DEBUG)
        DEBUG_ERROR("I'm V1 or V2 genrl query",NOVAR,0,0);
#endif
        for (i=0; i<pi->mcast.lenmclist; i++)
        {
#endif
            /* never start timer for ALL HOSTS GROUP   */
#if (INCLUDE_RUN_TIME_CONFIG)
            if (!tc_cmp4((PFBYTE)&(pi->mcast.mclist_ip[i*IP_ALEN]), 
                     (PFBYTE)ip_igmp_all_hosts, IP_ALEN))
#else
            if (!tc_cmp4((PFBYTE)&(pi->mcast.mclist_ip[i][0]), 
                     (PFBYTE)ip_igmp_all_hosts, IP_ALEN))
#endif
            {
#if (INCLUDE_IGMP_V2)
                if (pi->mcast.mcast_querier == IGMP_V1)     /* version 1 query */
                {
#if (IGMPV2_DEBUG)
                    DEBUG_ERROR("timer V1 = ",DINT1,pi->mcast.report_timer[i],0);
                    DEBUG_ERROR("i = ",EBS_INT1,i,0);
#endif
                    if (pi->mcast.report_timer[i] <= 0)
                    {
#endif
                        pi->mcast.report_timer[i] = 
                        ((int)(ks_get_ticks() % CFG_IGMP_MAX_DELAY)) + 1;
#if (INCLUDE_IGMP_V2)
#if (IGMPV2_DEBUG)
                        DEBUG_ERROR("V1 timer started = ",DINT1,pi->mcast.report_timer[i],0);
                        DEBUG_ERROR("i = ",EBS_INT1,i,0);
#endif
#endif
                    }
                }
#if (INCLUDE_IGMP_V2)
                else  /* version 2 general query */
                {
#if (IGMPV2_DEBUG)
                    DEBUG_ERROR("timer V2 general = ",DINT1,pi->mcast.report_timer[i],0);
                    DEBUG_ERROR("i = ",EBS_INT1,i,0);
#endif
                    if ( (pi->mcast.report_timer[i] <= 0) ||    /* timer not running or */
                         ( (pi->mcast.report_timer[i] > 0) &&  /* timer is running but max resp time */
                           (igmp->igmp_max_resp_time < (pi->mcast.report_timer[i]*10)))) /* less than time left */
                    {
                        pi->mcast.report_timer[i] = 
                            (((int)((ks_get_ticks()*10) % igmp->igmp_max_resp_time))/10) + 1;
#if (IGMPV2_DEBUG)
                        DEBUG_ERROR("Timer V2 genrl started =",DINT1,pi->mcast.report_timer[i],0);
                        DEBUG_ERROR("i = ",EBS_INT1,i,0);
#endif
                    }
                }
            }
        }

#endif
    }
    OS_RELEASE_IFACE(pi);
}

/* ********************************************************************   */
/* igmp_reset_timer() - reset report timer                                */
/*                                                                        */
/*   Resets IGMP report timer for a multicast address on interface iface. */
/*                                                                        */
/*   Returns nothing.                                                     */
/*                                                                        */

void igmp_reset_timer(PIFACE pi, PFBYTE mcast_ip_addr)
{
int    i;

    OS_CLAIM_IFACE(pi, IGMP_RESET_CLAIM_IFACE)

    /* find the multicast IP address   */
    for (i=0; i<pi->mcast.lenmclist; i++)
    {
#if (INCLUDE_RUN_TIME_CONFIG)
        if (tc_comparen(&(pi->mcast.mclist_ip[i*IP_ALEN]), 
                        (PFBYTE)mcast_ip_addr, IP_ALEN))
#else
        if (tc_comparen(&(pi->mcast.mclist_ip[i][0]), (PFBYTE)mcast_ip_addr, IP_ALEN))
#endif
        {
            DEBUG_LOG("igmp_reset_timer - reset a timer", LEVEL_3, 
                NOVAR, 0, 0);
            pi->mcast.report_timer[i] = 0;

            DEBUG_LOG("igmp_reset_timer - reset a timer", LEVEL_3, 
                NOVAR, 0, 0);

#if (INCLUDE_IGMP_V2)
            pi->mcast.last_host_toreply[i] = FALSE; /* not the last one left */
                            /* since another host reported   */
#endif
        }
    }
    OS_RELEASE_IFACE(pi)
}

#endif





