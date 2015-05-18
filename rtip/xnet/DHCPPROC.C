/*

   DHCPPROC.C - DHCP client internal functions

   EBS - RTIP

   Copyright Peter Van Oudenaren , 1996
   All rights reserved.
   This code may not be redistributed in source or linkable object form
   without the consent of its author.

    Module description:
    This module contains functions internal to those in DHCPAPI.C
*/

#define DIAG_SECTION_KERNEL DIAG_SECTION_DHCP

#include "sock.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#include "rtipext.h"
#endif

#if (INCLUDE_DHCP_CLI || INCLUDE_DHCP_SRV)

#include "dhcp.h"
#include "dhcpcapi.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DISPLAY_LEASE_EXPIRE 1  /* display msg if lease expires */
#define DISPLAY_LEASE_EXTEND 1  /* display msg if lease is extended */
#define DISPLAY_NEW_LEASE    1  /* display msg if new lease obtained due */
                                /* to old lease expiring and renew+rebind   */
                                /* failed                                   */

#define DEBUG_DHCP           0
#define DEBUG_RENEW          1
#define DEBUG_RENEW_STATES   1
#define DEBUG_SOCKET         0

/* ********************************************************************    */
/* DEFINES                                                                 */
/* ********************************************************************    */
/* minimum lease time to request due to limitations in retry logic;        */
/* i.e. if the lease time is too short than there will not be enough       */
/*      rebind time; the rebind time starts at lease_time/8 and            */
/*      needs at least ((CFG_DHCP_RETRIES+1) * CFG_DHCP_TIMEOUT) seconds   */
/*      to accomplish the rebind                                           */
/* NOTE: it is set to CFG_DHCP_RETRIES+1 since that is the number of times */
/*      the request is tried                                               */
/* UNITS: seconds                                                          */
#define DHCP_MIN_LEASE (((CFG_DHCP_RETRIES+1) * CFG_DHCP_TIMEOUT) << 3)

#define broadcast_addr ip_ffaddr

#define NO_TAG 0

#if (INCLUDE_DHCP_CLI)
extern DHCP_callback_fp KS_FAR choose_dhcp_offer;
KS_EXTERN_GLOBAL_CONSTANT DHCP_param KS_FAR std_param_lst[DHCP_STD_PARAMS];
extern EBS_TIMER KS_FAR dhcp_timer_info;        /* timer information */
static void dhcp_timeout(void KS_FAR *vp);

#endif      /* DHCP_CLI */

/************************************************************   */
/********************* FUNCTION BODIES **********************   */
/************************************************************   */

#if (INCLUDE_DHCP_CLI)
/* ********************************************************************   */
/* DHCP CLIENT INTERNAL FUNCTIONS                                         */
/* ********************************************************************   */

void init_dhcp(void)
{
#if (INCLUDE_DHCP_RENEW_NO_BLOCK)
PIFACE pi;
int    iface_no;
#endif

    choose_dhcp_offer = (DHCP_callback_fp)(0);

#if (INCLUDE_DHCP_RENEW_NO_BLOCK)
    LOOP_THRU_IFACES(iface_no)
    {
        pi = tc_ino2_iface(iface_no, DONT_SET_ERRNO);
        if (pi)
        {
            pi->addr.dhcp_extend_status = DHCP_EXTEND_IDLE;
            pi->addr.orig_lease_time = 0;
        }
    }
#endif

    /* set up 1 second timer to renew leases   */
    dhcp_timer_info.func = dhcp_timeout;    /* routine to execute if timeout */
    dhcp_timer_info.arg = 0;                /* dummy arg list - not used */
    ebs_set_timer(&dhcp_timer_info, 1, TRUE);
    // XFK 11-06-2007: MPC TTP 1072 indicates that dhcp_timeout() and RTIPDHCPClient::Run() 
    // should not both run at the same time, or sockets will be closed without notifying listeners 
    // (thus blocking sockets forever), in the rare case of a DHCP lease timeout. 
    // The chosen solution is not to start the DHCP timer (dhcp_timer_info) at all.
    //ebs_start_timer(&dhcp_timer_info); 
}

/* ********************************************************************   */
/* EXTEND LEASE SUPPORT ROUTINES                                          */
/* ********************************************************************   */
int extend_dhcp_lease_start(PFDHCP_session session, dword lease_time)
{
dword ciaddr;
int   dhcpc_sock;

#if (DEBUG_LEASE_STATE)
    DEBUG_ERROR("extend_dhcp_lease_start: lease time passed to extend lease = ", 
        DINT1, lease_time, 0);
#endif
    DEBUG_LOG("extend_dhcp_lease_start: lease time passed to extend lease = ", 
        LEVEL_2, DINT1, lease_time, 0);
    tc_mv4(&ciaddr, session->client_ip, IP_ALEN);
    format_dhcp_extend(&(session->outmsg), ciaddr, lease_time);
    dhcpc_sock = open_dhcp_socket(session->iface_no);

    return(dhcpc_sock);
}

void process_extend_results(PFDHCP_session session, int dhcpc_sock)
{
PIFACE pi;

    /* return params   */
    get_dhcp_op(&(session->replies[0]), IP_LEASE, sizeof(dword), 
        (void*)&(session->lease_time));
    session->lease_time = ntohl(session->lease_time);
#if (DEBUG_RENEW_STATES)
    DEBUG_ERROR("process_extend_results: session->lease_time = ",
        DINT1, session->lease_time, 0);
#endif

    closesocket(dhcpc_sock);

    pi =  tc_get_local_pi(session->client_ip);
    if (pi && (pi->open_count > 0))
    {
        /* Set the renew and rebind lease times again                       */
        /* Update DHCP table info in iface. For lease expiration times, use */
        /* the defaults (.5,.875) unless the DHCP server gave               */
        /* us renew times                                                   */
        OS_CLAIM_IFACE(pi, DHCP_EXTEND_CLAIM_IFACE)
        pi->addr.lease_time = pi->addr.orig_lease_time = session->lease_time;
#if (DEBUG_RENEW_STATES)
        DEBUG_ERROR("process_extend_results: set lease_time to ", DINT1,
            pi->addr.lease_time, 0);
#endif
        if(get_dhcp_op(&(session->replies[0]), RENEWAL_TIME,
            sizeof(dword), (void*)&pi->addr.renew_lease_time) < 0)
        {
            pi->addr.renew_lease_time = session->lease_time >> 1;
        }
        else
        {
            pi->addr.renew_lease_time = ntohl(pi->addr.renew_lease_time);
        }
        if (get_dhcp_op(&(session->replies[0]), REBINDING_TIME,
            sizeof(dword), (void*)&pi->addr.rebind_lease_time) <0)
        {
            pi->addr.rebind_lease_time = session->lease_time >>3;
        }
        else
        {
            pi->addr.rebind_lease_time = pi->addr.orig_lease_time-
            ntohl(pi->addr.rebind_lease_time);
        }
        
        OS_RELEASE_IFACE(pi)
    }
}

#if (INCLUDE_DHCP_RENEW_NO_BLOCK)
int rebind_renew_dhcp_lease_start(PIFACE pi, PFDHCP_session session)
{
int dhcpc_sock;

    /* format request and get a socket   */
    dhcpc_sock = extend_dhcp_lease_start(session, pi->addr.orig_lease_time);
#if (DEBUG_SOCKET)
    DEBUG_ERROR("lease_start: socket = ", EBS_INT1, dhcpc_sock, 0);
#endif

    session->timeout_ticks = CFG_DHCP_TIMEOUT*ks_ticks_p_sec();
    session->retries = 0;
    session->start_time = ks_get_ticks();
    session->rindex = 0;

    if (send_dhcp_packet(&(session->outmsg), dhcpc_sock, 
                         pi->addr.dhcp_session.server_ip) < 0)
        return(-1);     /* errno set by sendto */
    return(dhcpc_sock);
}
#endif

#if (INCLUDE_DHCP_RENEW_NO_BLOCK)
/* ********************************************************************   */
/* EXTEND LEASE TIMEOUT ROUTINES                                          */
/* ********************************************************************   */

/* returns: 0 -  in progress; i.e. either not a timeout so          */
/*               waiting for more replies or a timeout and sending  */
/*               again                                              */
/*         -1 -  failure, i.e. no replies                           */
/*         >0 -  done and all responses within time frame are in    */
int rebind_renew_dhcp_lease_recv(PIFACE pi, PFDHCP_session session)
{
int   validity;
int   result;
dword elap_time_tics, curr_time;
byte  msg_type;

    do
    {
#if (DEBUG_SOCKET)
        DEBUG_ERROR("rebind_renew_dhcp_lease_recv: socket = ", EBS_INT1,
            session->dhcpc_sock, 0);
#endif
        result = recv_dhcp_packet(&(session->replies[session->rindex]), 
                                  session->dhcpc_sock, 0);
        if (result == -1)
            return result;      /* errno set by recv */
        else if (result == 1)   /* we got a reply */
        {
#if (DEBUG_RENEW_STATES)
            DEBUG_ERROR("rebind_renew_dhcp_lease_recv: got a reply", 
                NOVAR, 0, 0);
#endif
            DEBUG_LOG("got a reply", LEVEL_2, NOVAR, 0, 0);

            /* check if reply is valid. If so, keep it.   */
            validity = 
                is_dhcp_response_valid(&(session->outmsg), 
                                       &(session->replies[session->rindex]));
            DEBUG_LOG("validity = ", LEVEL_2, DINT1, (dword)validity, 0);
            if (validity != DHCP_INVALID)
            {
#if (DEBUG_RENEW_STATES)
                DEBUG_ERROR("rebind_renew_dhcp_lease_recv: reply is VALID", 
                    NOVAR, 0, 0);
#endif
                DEBUG_LOG("reply is valid", LEVEL_2, NOVAR, 0, 0);
                get_dhcp_op(&(session->replies[session->rindex]), 
                            DHCP_MSG_TYPE, sizeof(byte),
                            (PFVOID)&msg_type);

                /* message type is ok   */
                if (msg_type == DHCPACK || msg_type == DHCPNAK)
                {
                    session->rindex++;
#if (DEBUG_RENEW_STATES)
                    DEBUG_ERROR("rebind_renew_dhcp_lease_recv: update rindex",
                        EBS_INT1, session->rindex, 0);
#endif
                    DEBUG_LOG("rindex inc", LEVEL_2, NOVAR, 0, 0);
                }
            }
        }
    }
    while (result == 1);

    /* get elapsed time (taking into account wrap) and check for timeout   */
    /* tbd: check for wrap                                                 */
    curr_time = ks_get_ticks();
    if (curr_time < session->start_time)  /* if wrap */
        elap_time_tics = DHCP_INF_LEASE - session->start_time + curr_time;
    else
        elap_time_tics = curr_time - session->start_time;

#if (DEBUG_RENEW_STATES)
    DEBUG_ERROR("rebind_renew_dhcp_lease_recv: elap, timeout", DINT2,
        elap_time_tics, CFG_DHCP_TIMEOUT*ks_ticks_p_sec());
    if (elap_time_tics > (dword)(CFG_DHCP_TIMEOUT*ks_ticks_p_sec()))
    {
        DEBUG_ERROR("                            : TIMEOUT", NOVAR, 0, 0);
    }
    else
    {
        DEBUG_ERROR("                            : NOT TIMEOUT", NOVAR, 0, 0);
    }
#endif

    if (elap_time_tics > CFG_DHCP_TIMEOUT*ks_ticks_p_sec())
    {
        if (session->rindex > 0)
        {
#if (DEBUG_RENEW_STATES)
            DEBUG_ERROR("rebind_renew_dhcp_lease_recv: TIMEOUT and DONE: rindex = ",
                EBS_INT1, session->rindex, 0);
#endif
            return(session->rindex);
        }

        /* timeout   */
        session->retries++;
        if (session->retries > CFG_DHCP_RETRIES)
        {
#if (DEBUG_RENEW_STATES)
            DEBUG_ERROR("rebind_renew_dhcp_lease_recv: TIMEOUT and RETRIES EXCEEDED->failure",
                NOVAR, 0, 0);
#endif
            return(-1);
        }

#if (DEBUG_RENEW_STATES)
        DEBUG_ERROR("rebind_renew_dhcp_lease_recv: times up: rindex = ",
            EBS_INT1, session->rindex, 0);
#endif
        /* timeout: if haven't gotten any replyies, send again   */
        if (session->rindex <= 0)
        {
#if (DEBUG_RENEW_STATES)
            DEBUG_ERROR("rebind_renew_dhcp_lease_recv: timeout, no replies: send again",
                NOVAR, 0, 0);
#endif
            if (send_dhcp_packet(&(session->outmsg), session->dhcpc_sock, 
                                 pi->addr.dhcp_session.server_ip) < 0)
                return(-1);     /* errno set by sendto */
            session->timeout_ticks = CFG_DHCP_TIMEOUT*ks_ticks_p_sec();
            session->retries = CFG_DHCP_RETRIES;
            session->start_time = ks_get_ticks();
        }
        else
        {
            /* timeout: we have gotten some replies so return # of replies   */
            return(session->rindex);
        }
    }
    return(0);  /* return in progress; i.e. either not a timeout so */
                /* waiting for more replies or a timeout and sending    */
                /* again                                                */
            
}
#endif  /* INCLUDE_DHCP_RENEW_NO_BLOCK */

#if (INCLUDE_DHCP_RENEW_NO_BLOCK)
/* ********************************************************************   */
void dhcp_set_conf(PIFACE pi, PFDHCP_cparam cplist, PFDHCP_conf conf)
{
    xn_init_dhcp_conf(conf);

    if ( (pi->addr.iface_flags & DHCP_CLI_DDNS) &&
         xn_has_dns() )
    {
        cplist[0].id = HOST_NAME_OP;
        cplist[0].cpdata = xn_get_domain_name();
        cplist[0].len = tc_strlen((PFCCHAR)cplist[0].cpdata); 
        cplist[0].next = (void *)0;
        conf->cplist = cplist;
        conf->cplist_entries = 1;

        conf->request_std_params = FALSE;
        conf->apply_std_params = TRUE;
        conf->vs_size = 0;
    }

    conf->request_std_params = FALSE;
    conf->plist_entries = 0;
    conf->vs_size = 0;
}

/* ********************************************************************          */
/* dhcp_timeout - renew IP leases                                                */
/*                                                                               */
/*   This routine is called once per second by the timer task.                   */
/*   It renew IP address leases.  This routine cannot block                      */
/*   since it is the timer task.                                                 */
/*                                                                               */
/*   The client will automatically renew (referred to as renewing lease)         */
/*   its lease.  The routine dhcp_timeout is called once per second to           */
/*   perform the renew.  It performs the same functionality as                   */
/*   xn_extend_dhcp_lease except it cannot block since it is                     */
/*   the timer task.  It also tracks when to perform and retry the renew.        */
/*   The first renew is attempted when one half of the lease time has expired.   */
/*   It will send the renew request directly to the DHCP server it got the       */
/*   origional lease from.  It will retry the renew at one half intervals        */
/*   until it is time to rebind (see below) or it is successful.  For example,   */
/*   if the lease time is 60 minutes it will try to renew at 30 minutes,         */
/*   15 minutes, 7 1/2 minutes etc.                                              */
/*                                                                               */
/*   When 87.5% of the lease time has expired, the client will extend the        */
/*   lease by performing a rebind.  It will broadcast the extend                 */
/*   request.  It will continully retry at one half intervals until              */
/*   only one minute is left.  Then it will give up trying.                      */
/*                                                                               */
/*   When the lease expires (i.e. extending the lease was unsuccessful),         */
/*   the IP address and all sockets bound to that IP address are invalidated.    */
/*                                                                               */
/*   Because dhcp_timeout cannot block, this routine must return whenever        */
/*   it needs to wait.  Therefore, for each interface, state information         */
/*   must be kept.  There are 2 states, IDLE and INPROG.  The routine            */
/*   is in the IDLE state when it is not attempting an extend.  When             */
/*   the IDLE state detects it is time to extend, it sends request               */
/*   and transitions to the IDLE state.  The idle state check for                */
/*   responses, if it is time to retry and for timeout.                          */
/*                                                                               */
/*   There are 2 separate retry situations.  When it is time to extend,          */
/*   if now responses are received within CFG_DHCP_TIMEOUT seconds (default      */
/*   is 4), the extend will be retried.  It will retry CFG_DHCP_RETRIES          */
/*   times.  This retry is done since UDP is an unreliable transport             */
/*   protocol.  The second retry (which is described above) retries              */
/*   the extend at half the lease interval.  The lease time                      */
/*   is much larger than the retry time.  In fact we limit how small             */
/*   the lease time can be by DHCP_MIN_LEASE.  This retry is done                */
/*   for the case the DHCP server is off line for a period of time.              */
/*                                                                               */
/*   Renew information is kept in the interface structure as follows:            */
/*   in interface structure (in struct iface_ip) keep following information:     */
/*       orig_lease_time - amount of lease time given by server                  */
/*       lease_time      - amount of lease time left                             */
/*       renew_lease_time- time left before renew (typically .5 x orig)          */
/*       rebind_lease_time-time left before rebind (typically .875 x orig)       */
/*       dhcp_server_ip_addr[IP_ALEN] - DHCP server IP address                   */
/*                address used to send renew request; set by xn_dhcp             */
/*      dhcp_extend_status - status of extend lease                              */
/*      dhcp_session;      - session used to extend lease                        */
/*                                                                               */
/*   Returns nothing                                                             */
/*                                                                               */

static void dhcp_timeout(void KS_FAR *vp)
{
DHCP_conf conf;
int       iface_no,n;
dword     temp;
PIFACE    pi;
int       result;
#if (INCLUDE_DHCP_NEW_LEASE)
DHCP_session dhcp_session;
dword        orig_lease;
#endif
DHCP_cparam cplist[1];

    ARGSUSED_PVOID(vp)

    LOOP_THRU_IFACES(iface_no)
    {
        pi = tc_ino2_iface(iface_no, DONT_SET_ERRNO);

        if (!pi || (pi->open_count <= 0))
            CONTINUE_IF_LOOP

        /* Only execute if we have a lease time and it is not infinite   */
        if ((pi->addr.orig_lease_time > 0) &&
            (pi->addr.orig_lease_time != DHCP_INF_LEASE))
        {
            pi->addr.lease_time--;

            /* if lease time has run out invalidate address and   */
            /* all sockets bound to it                            */
            if (pi->addr.lease_time == 1)
            {
#if (DISPLAY_LEASE_EXPIRE || DEBUG_RENEW_STATES)
                DEBUG_ERROR("dhcp_timeout: lease ran out; invalidate", NOVAR, 0, 0);
#endif
#if (INCLUDE_DHCP_NEW_LEASE)
                orig_lease = pi->addr.orig_lease_time;
#endif
                pi->addr.orig_lease_time = 0;
                pi->addr.dhcp_extend_status = DHCP_EXTEND_IDLE;
                closesocket(pi->addr.dhcp_session.dhcpc_sock);

#            if (INCLUDE_TCP)
                tcp_invalidate_sockets(pi->addr.my_ip_addr);
#            endif
#            if (INCLUDE_UDP)
                udp_invalidate_sockets(pi->addr.my_ip_addr);
#            endif

                /* reset values for next lease   */
                tc_mv4(pi->addr.my_ip_addr, ip_nulladdr, IP_ALEN);

#if (INCLUDE_DHCP_NEW_LEASE)
                /* try to get a new lease   */

                /* initialize config info for renew                 */
                /* if DDNS is being used, register our DNS address  */
                dhcp_set_conf(pi, cplist, &conf);

                /* use same lease time as origionally allocated   */
                conf.lease_time = orig_lease;
                result = xn_dhcp(iface_no, &dhcp_session, &conf);
                if (result < 0)
                {
                    DEBUG_ERROR("dhcp_timeout: NEW LEASE failed: iface", 
                        EBS_INT1, 0, 0);
                    CB_DHCP_NO_IP(iface_no);
                }
#if (DISPLAY_NEW_LEASE || DEBUG_RENEW_STATES)
                else
                {
                    DEBUG_ERROR("dhcp_timeout: NEW LEASE obtained: ", IPADDR,
                        pi->addr.my_ip_addr, 0);
                }
#endif
#else
                CB_DHCP_NO_IP(iface_no);
                    
#endif      /* INCLUDE_DHCP_NEW_LEASE */
                CONTINUE_IF_LOOP
            }   
        }

        /* perform equivalent functionality as xn_extend_dhcp_lease but   */
        /* since cannot block in timer task the functionality is          */
        /* done here but as a state machine                               */
        switch(pi->addr.dhcp_extend_status)
        {
        case DHCP_EXTEND_IDLE:

            /* if time to renew the lease   */
            if ((pi->addr.lease_time < pi->addr.renew_lease_time) ||
                (pi->addr.lease_time < pi->addr.rebind_lease_time))
            {
                /* initialize session data and packet packets:      */
                /* initialize config info for renew                 */
                /* if DDNS is being used, register our DNS address  */
                dhcp_set_conf(pi, cplist, &conf);

                conf.lease_time = pi->addr.dhcp_session.lease_time = 
                    pi->addr.orig_lease_time;
                tc_mv4(pi->addr.dhcp_session.client_ip,
                       pi->addr.my_ip_addr,IP_ALEN);

#if (DEBUG_RENEW_STATES)
                DEBUG_ERROR("dhcp_timeout: time to renew: set lease_time to ", DINT1,
                    pi->addr.orig_lease_time, 0);
#endif

                /* if doing a renew send directly to server; if doing a   */
                /* rebind broadcast the request                           */
                if (pi->addr.renew_lease_time > 0)
                    tc_mv4(pi->addr.dhcp_session.server_ip,
                           pi->addr.dhcp_server_ip_addr, IP_ALEN);
                else
                    tc_mv4(pi->addr.dhcp_session.server_ip,
                           broadcast_addr, IP_ALEN);

                init_dhcp_packet(&(pi->addr.dhcp_session.outmsg));
                for (n = 0; n < CFG_DHCP_OFFERS; n++)
                    init_dhcp_packet(&(pi->addr.dhcp_session.replies[n]));
                if (format_dhcp_discinf(&(pi->addr.dhcp_session.outmsg),
                                        iface_no, &conf, DHCPREQUEST)<0) 
                {
                    break;
                }
#if (DEBUG_RENEW)
                DEBUG_ERROR("dhcp_timeout: renew to address ", IPADDR,
                    pi->addr.dhcp_session.server_ip, 0);
                DEBUG_ERROR("              requested lease time = ",
                    DINT1, pi->addr.orig_lease_time, 0);
#endif
                pi->addr.dhcp_session.dhcpc_sock = 
                    rebind_renew_dhcp_lease_start(pi, &(pi->addr.dhcp_session));
                pi->addr.dhcp_extend_status = DHCP_EXTEND_INPROG;
                break;
            }
            break;

        case DHCP_EXTEND_INPROG:
            result = rebind_renew_dhcp_lease_recv(pi, &(pi->addr.dhcp_session));
            if (result > 0)
            {
                /* responses are in, now process them   */
                /* NOTE: closes socket                  */
                process_extend_results(&(pi->addr.dhcp_session), 
                                       pi->addr.dhcp_session.dhcpc_sock);
                pi->addr.dhcp_extend_status = DHCP_EXTEND_IDLE;
#if (DISPLAY_LEASE_EXTEND || DEBUG_RENEW)
                DEBUG_ERROR("dhcp_timeout: LEASE EXTENDED: ", IPADDR,
                    pi->addr.my_ip_addr, 0);
#endif
            }

            /* if failure, i.e. timeout and all retries have been done   */
            /* then set up for the next rebind/renew at 1/2 the          */
            /* interval                                                  */
            if (result < 0)
            {
#if (DEBUG_RENEW)
                DEBUG_ERROR("dhcp_timeout: renew FAILED ", IPADDR,
                    pi->addr.dhcp_session.server_ip, 0);
#endif

                pi->addr.dhcp_extend_status = DHCP_EXTEND_IDLE;
                closesocket(pi->addr.dhcp_session.dhcpc_sock);

                /* extend lease failed   */
                OS_CLAIM_IFACE(pi, DHCP_TO_CLAIM_IFACE)

                /* if did a renew   */
                if (pi->addr.renew_lease_time > 0)
                {
#if (DEBUG_RENEW)
                    DEBUG_ERROR("dhcp_timeout: renew>0 ", DINT1,
                        pi->addr.renew_lease_time, 0);
#endif
                    /* do retry of renew unless we are close to the   */
                    /* rebind time in which case force the rebind     */
                    /* the next time                                  */
                    /* temp = (renew-rebind)/2                        */
                    temp = (pi->addr.renew_lease_time -
                            pi->addr.rebind_lease_time) >> 1;
                    if (temp >= ONE_MINUTE)
                    {
#if (DEBUG_RENEW)
                        DEBUG_ERROR("dhcp_timeout: renew>0, temp>59 ", DINT2,
                            pi->addr.renew_lease_time, temp);
#endif
                        /* do another renew at half the interval   */
                        /* renew = (renew-rebind)/2 + rebind       */
                        pi->addr.renew_lease_time = 
                            temp + pi->addr.rebind_lease_time;
                    }
                    else
                    {
#if (DEBUG_RENEW)
                        DEBUG_ERROR("dhcp_timeout: renew>0, temp<=59=>set renew to 0 ", 
                            DINT2, pi->addr.renew_lease_time, temp);
#endif
                        /* next time do a rebind   */
                        pi->addr.renew_lease_time = 0;
                    }
                }
                /* if did a rebind   */
                else
                {
#if (DEBUG_RENEW)
                    DEBUG_ERROR("dhcp_timeout: renew<=0: SET BROADCAST ", DINT1,
                        pi->addr.renew_lease_time, 0);
#endif
                    temp = pi->addr.rebind_lease_time >> 1;
                    if (temp >= ONE_MINUTE)
                    {
                        /* retry the rebind at half the time interval   */
                        pi->addr.rebind_lease_time = temp;
                    }
                    else
                    {
#if (DEBUG_RENEW)
                        DEBUG_ERROR("dhcp_timeout: renew<=0: < ONE_MINUTE, give up ", DINT1,
                            pi->addr.renew_lease_time, 0);
#endif
                        /* give up on the rebind and let the timer   */
                        /* expire                                    */
                        pi->addr.rebind_lease_time = 0;
                    }
                }
                OS_RELEASE_IFACE(pi)
            }
            break;

        default:
            DEBUG_ERROR("dhcp_timeout: illegal state = ", EBS_INT1,
                pi->addr.dhcp_extend_status, 0);
        }   /* end of switch */

    }       /* end of loop */

    /* restart the one second timer   */
    ebs_start_timer(&dhcp_timer_info); 
}


#else   /* INCLUDE_DHCP_RENEW_NO_BLOCK */

/* 1 second timer routine - renews leases which   */
static void dhcp_timeout(void KS_FAR *vp)
{
DHCP_session session;
DHCP_conf conf;
int iface_no,n;
dword temp;
PIFACE pi;

    ARGSUSED_PVOID(vp)

    LOOP_THRU_IFACES(iface_no)
    {
        pi = tc_ino2_iface(iface_no, DONT_SET_ERRNO);
        if (pi && (pi->open_count > 0))
        {
            /* Only execute if we have a lease time and it is not infinite   */
            if ((pi->addr.orig_lease_time >0) &&
                (pi->addr.orig_lease_time != 0xffffffff))
            {
                pi->addr.lease_time--;

                /* if time to renew the lease   */
                if ((pi->addr.lease_time < pi->addr.renew_lease_time) ||
                    (pi->addr.lease_time < pi->addr.rebind_lease_time))
                {
                    /* initialize session data and packet packets:   */
                    xn_init_dhcp_conf(&conf);
                    conf.lease_time = session.lease_time = pi->addr.orig_lease_time;
                    tc_mv4(session.client_ip,pi->addr.my_ip_addr,IP_ALEN);

                    /* if doing a renew send directly to server; if doing a   */
                    /* rebind broadcast the request                           */
                    if (pi->addr.renew_lease_time > 0)
                        tc_mv4(session.server_ip,pi->addr.dhcp_server_ip_addr, IP_ALEN);
                    else
                        tc_mv4(session.server_ip,broadcast_addr, IP_ALEN);

                    /* initialize config info for renew   */
                    conf.request_std_params = FALSE;
                    conf.plist_entries = 0;
                    conf.vs_size = 0;

                    init_dhcp_packet(&(session.outmsg));
                    for (n=0; n<CFG_DHCP_OFFERS; n++)
                        init_dhcp_packet(&(session.replies[n]));
                    if (format_dhcp_discinf(&(session.outmsg),iface_no,
                                        &conf,DHCPREQUEST)<0) 
                        break;
#if (DEBUG_RENEW)
                    DEBUG_ERROR("dhcp_timeout: renew to address ", IPADDR,
                        session.server_ip, 0);
                    DEBUG_ERROR("              requested lease time = ",
                        DINT1, pi->addr.orig_lease_time, 0);
#endif

                    if (xn_extend_dhcp_lease(&session,session.lease_time)<0)
                    {
#if (DEBUG_RENEW)
                        DEBUG_ERROR("dhcp_timeout: renew FAILED ", IPADDR,
                            session.server_ip, 0);
#endif

                        /* extend lease failed   */
                        OS_CLAIM_IFACE(pi, DHCP_TO_CLAIM_IFACE)

                        /* if did a renew   */
                        if (pi->addr.renew_lease_time > 0)
                        {
#if (DEBUG_RENEW)
                            DEBUG_ERROR("dhcp_timeout: renew>0 ", DINT1,
                                pi->addr.renew_lease_time, 0);
#endif
                            /* do retry of renew unless we are close to the   */
                            /* rebind time in which case force the rebind     */
                            /* the next time                                  */
                            /* temp = (renew-rebind)/2                        */
                            temp = (pi->addr.renew_lease_time -
                                    pi->addr.rebind_lease_time) >> 1;
                            if (temp >= ONE_MINUTE)
                            {
#if (DEBUG_RENEW)
                                DEBUG_ERROR("dhcp_timeout: renew>0, temp>59 ", DINT2,
                                    pi->addr.renew_lease_time, temp);
#endif
                                /* do another renew at half the interval   */
                                /* renew = (renew-rebind)/2 + rebind       */
                                pi->addr.renew_lease_time = 
                                    temp + pi->addr.rebind_lease_time;
                            }
                            else
                            {
#if (DEBUG_RENEW)
                                DEBUG_ERROR("dhcp_timeout: renew>0, temp<=59=>set renew to 0 ", 
                                    DINT2, pi->addr.renew_lease_time, temp);
#endif
                                /* next time do a rebind   */
                                pi->addr.renew_lease_time = 0;
                            }
                        }
                        else
                        {
#if (DEBUG_RENEW)
                            DEBUG_ERROR("dhcp_timeout: renew<=0: SET BROADCAST ", DINT1,
                                pi->addr.renew_lease_time, 0);
#endif              
                            temp = pi->addr.rebind_lease_time >> 1;
                            if (temp >= ONE_MINUTE)
                            {
                                /* retry the rebind at half the time interval   */
                                pi->addr.rebind_lease_time = temp;
                            }
                            else
                            {
                                /* give up on the rebind and let the timer   */
                                /* expire                                    */
                                pi->addr.rebind_lease_time = 0;
                            }
                        }
                        OS_RELEASE_IFACE(pi)
                    }
                }

                /* if lease time has run out invalidate address and   */
                /* all sockets bound to it                            */
                if (pi->addr.lease_time == 1)
                {
                    tc_mv4(pi->addr.my_ip_addr, ip_nulladdr, IP_ALEN);
                    pi->addr.orig_lease_time = 0;

#                    if (INCLUDE_TCP)
                        tcp_invalidate_sockets(pi->addr.my_ip_addr);
#                    endif
#                    if (INCLUDE_UDP)
                        udp_invalidate_sockets(pi->addr.my_ip_addr);
#                    endif
                }
            }   /* of of if lease time which is not infinate */
        }       /* end of if pi and pi is open */
    }           /* end of loo thru ifaces */

    /* restart the one second timer   */
    ebs_start_timer(&dhcp_timer_info); 
}
#endif      /* INCLUDE_DHCP_RENEW_NO_BLOCK */

/* ********************************************************************   */
/* MISC ROUTINES                                                          */
/* ********************************************************************   */
void apply_dhcp_std_ops(PFDHCP_session session, int iface_no)
{
PFDHCP_std_params p = &session->params;
PIFACE pi;
int i;
#if (INCLUDE_DNS)
dword srv_list[CFG_MAX_DNS_SRV];
#endif
dword dwtemp;
word  wtemp;
byte  btemp;
PFBYTE pbtemp;

    DEBUG_LOG("xn_apply_dhcp_std_ops", LEVEL_2, NOVAR, 0, 0);


    /* **************************************************               */
    /* MASK                                                             */
    /* NOTE: mask is applied by xn_dhcp since it need to call xn_set_ip */
    /*       regardless                                                 */

#if (INCLUDE_DNS)
    /* **************************************************   */
    /* DNS SERVER                                           */
    if (p->dns_server)
    {
        if (p->dns_server_len > (CFG_MAX_DNS_SRV<<2))
            p->dns_server_len = CFG_MAX_DNS_SRV<<2;
        for (i=0; i<(p->dns_server_len>>2); i++)
        {
            DEBUG_LOG("dns_server = ", LEVEL_2, IPADDR, 
                p->dns_server, 0);
            DEBUG_LOG("dns_server = ", LEVEL_2, IPADDR, 
                p->dns_server+4, 0);
            tc_mv4(&srv_list[i],(PFBYTE)(p->dns_server+i), 4);
        }

        /* set up DNS server list (convert length to # of servers)   */
        xn_set_server_list(srv_list, p->dns_server_len >> 2);
    }
    else
    {
        DEBUG_LOG("dns_server = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }
#endif

    /* **************************************************   */
    /* FORWARD IP PACKETS                                   */

    if (p->be_a_router)
    {
        tc_mv4((PFBYTE)&dwtemp,p->be_a_router,4);
        DEBUG_LOG("be_a_router=", LEVEL_2, DINT1, dwtemp, 0);
#        if (INCLUDE_ROUTER)
            ip_forwarding = dwtemp;
#        else
        {
            if (dwtemp)
            {
                DEBUG_ERROR("apply_dhcp_std_ops - be_a_router requested - routing disabled",
                    NOVAR, 0, 0);
            }
        }
#        endif
    }
    else
    {
        DEBUG_LOG("be_a_router = [not returned]", LEVEL_2, NOVAR, 0, 0);
        ip_forwarding = 1;
    }

    /* **************************************************               */
    /* MAX FRAG SIZE - MDRS                                             */
    /* we do not limit frag size (it is already limited by offset field */
    /* in fragement) but we save it (tbd)                               */
    if (p->mdrs)
    {
        tc_movebytes((PFBYTE)&wtemp,(PFBYTE)p->mdrs,2);
        DEBUG_LOG("mdrs = ", LEVEL_2, DINT1, wtemp, 0);
#if (INCLUDE_FRAG)
        max_frag_size = (dword)wtemp;
#else
        DEBUG_ERROR("apply_dhcp_std_ops - fragment size but frag turned off",
            NOVAR, 0, 0);
#endif
    }
    else
    {
        DEBUG_LOG("mdrs = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }

    /* **************************************************   */
    /* DOMAIN NAME - tbd - is this right                    */
    if (p->domain_name_len)
    {
        p->domain_name[p->domain_name_len] = '\0';
    }

    /* **************************************************   */
    /* IP TIME-TO-LIVE                                      */
    if (p->default_ip_ttl)
    {
        btemp = *(p->default_ip_ttl);
        DEBUG_LOG("default_ip_ttl = ", LEVEL_2, DINT1, (dword) btemp, 0);
        if ( (btemp >= 1) && (btemp <= 255) )
            CFG_IP_TTL = (dword) btemp;
        else
        {
            DEBUG_ERROR("apply_dhcp_std_ops - illegal CFG_IP_TTL = ",
                EBS_INT1, btemp, 0);
        }
    }
    else
    {
        DEBUG_LOG("default_ip_ttl = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }

    /* **************************************************   */
    /* MTU                                                  */
    if (p->mtu)
    {
        tc_mv4((PFBYTE)&dwtemp,(PFBYTE)p->mtu,4);
        DEBUG_LOG("mtu = ", LEVEL_2, DINT1, dwtemp, 0);
        pi = tc_ino2_iface(iface_no, DONT_SET_ERRNO);
        if (pi)
        {
            dwtemp = net2hl(dwtemp);
            if (dwtemp > 68)
                pi->addr.mtu = (word)dwtemp;
            else
            {
                DEBUG_ERROR("apply_dhcp_std_ops - illegal mtu = ",
                    EBS_INT1, dwtemp, 0);
            }
        }
        else
        {
            DEBUG_ERROR("apply_dhcp_std_ops - interface not open - iface_no = ",
                EBS_INT1, iface_no, 0);
        }
    }
    else
    {
        DEBUG_LOG("mtu = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }

    /* **************************************************   */
    /* STATIC ROUTE                                         */
    if (p->static_route)
    {
        if ( (p->static_route_len & 0x7) != 0 )
        {
            DEBUG_ERROR("static route info is not multiple of 8; it = ",
                EBS_INT1, p->static_route_len, 0);
        }
        else
        {
            pbtemp = (PFBYTE) p->static_route;
            for (i=0; i<p->static_route_len; i+=8)
            {
                DEBUG_LOG("static_route = ", LEVEL_2, IPADDR, 
                    (pbtemp+i), 0);
                DEBUG_LOG("static_route = ", LEVEL_2, IPADDR, 
                    0, (pbtemp+i+4));
                if (rt_add((PFBYTE)(pbtemp+i), (PFBYTE)ip_ffaddr, 
                           (PFBYTE)(pbtemp+i+4),
                           RT_USEIFACEMETRIC, iface_no, 
                           RT_INF, NO_TAG, SNMP_OTHER)) 
                {
                    DEBUG_ERROR("apply_dhcp_std_ops - rt_add failed", NOVAR,
                        0, 0);
                }
            }
        }
    }
    else
    {
        DEBUG_LOG("static_route = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }

    /* **************************************************   */
    /* STATIC ROUTE                                         */
    if (p->router_option)
    {
        if ( (p->router_option_len & 0x3) != 0 )
        {
            DEBUG_ERROR("router op info is not multiple of 4; it = ",
                EBS_INT1, p->router_option_len, 0);
        }
        else
        {
            pbtemp = (PFBYTE) p->router_option;
            for (i=0; i<p->router_option_len; i+=4)
            {
                DEBUG_LOG("router_option = ", LEVEL_2, IPADDR,
                    (pbtemp+i), 0);
                if (rt_add(RT_DEFAULT, (PFBYTE)ip_ffaddr, 
                           (PFBYTE)(pbtemp+i),
                           RT_USEIFACEMETRIC, iface_no, RT_INF, 
                           NO_TAG, SNMP_OTHER)) 
                {
                    DEBUG_ERROR("apply_dhcp_std_ops - rt_add failed", NOVAR,
                        0, 0);
                }
            }
        }
    }
    else
    {
        DEBUG_LOG("router_option = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }

    /* **************************************************   */
    /* CACHE TIMEOUT                                        */
    if (p->arp_cache_timeout) 
    {
        tc_mv4((PFBYTE)&dwtemp,(PFBYTE)p->arp_cache_timeout,4);
        DEBUG_LOG("arp_cache_timeout = ", LEVEL_2, DINT1, 
                  dwtemp, 0);
        arpc_res_tmeout = net2hl(dwtemp);
    }
    else
    {
        DEBUG_LOG("arp_cache_timeout = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }

    /* **************************************************   */
    /* TCP TIME-TO-LIVE - tbd                               */
#if (INCLUDE_TCP)
    if (p->tcp_default_ttl)
    {
        tc_mv4((PFBYTE)&dwtemp,(PFBYTE)p->tcp_default_ttl,4);
        DEBUG_LOG("tcp_default_ttl = ", LEVEL_2, DINT1, dwtemp, 0);
    }
    else
    {
        DEBUG_LOG("tcp_default_ttl = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }
#endif

    /* **************************************************   */
    /* KEEP-ALIVE INTERVAL                                  */
#if (INCLUDE_TCP)
    if (p->tcp_ka_interval)
    {
        tc_mv4((PFBYTE)&dwtemp,(PFBYTE)p->tcp_ka_interval,4);
        DEBUG_LOG("tcp_ka_interval = ", LEVEL_2, DINT1, dwtemp, 0);
        CFG_KA_INTERVAL = net2hl(dwtemp);
    }
    else
    {
        DEBUG_LOG("tcp_ka_interval = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }
#endif

    /* **************************************************   */
    /* KEEP-ALIVE GARBAGE BYTE - tbd                        */
    if (p->tcp_ka_garbage)
    {
        tc_mv4((PFBYTE)&dwtemp,(PFBYTE)p->tcp_ka_garbage,4);
        DEBUG_LOG("tcp_ka_garbage = ", LEVEL_2, DINT1, 
            dwtemp, 0);
#if (INCLUDE_TCP)
        ka_send_garbage = (RTIP_BOOLEAN)(dwtemp);
#endif
    }
    else
    {
        DEBUG_LOG("tcp_ka_garbage = [not returned]", LEVEL_2, NOVAR, 0, 0);
    }
}

/* extract the parameters from the reply and store them in params structure   */
void get_dhcp_params(PFDHCP_packet ack, PFDHCP_std_params params)
{
int index, len;
RTIP_BOOLEAN isthere;

    /* zero out all pointers   */
    tc_memset((PFBYTE)params, 0, sizeof(DHCP_std_params));

    isthere = is_dhcp_op_there(ack, SUBNET_MASK, &index, &len);
    if (isthere == TRUE)
        params->subnet_mask = (PFDWORD)&(ack->options[index]);

    isthere = is_dhcp_op_there(ack, NLSR, &index, &len);
    if (isthere == TRUE)
        params->be_a_router = (PFBYTE)&(ack->options[index]);

    isthere = is_dhcp_op_there(ack, DNS_OP, &index, &len);
    if (isthere == TRUE)
    {
        params->dns_server = (PFDWORD)&(ack->options[index]);
        params->dns_server_len = len;
    }

    params->domain_name_len = 0;
    isthere = is_dhcp_op_there(ack, DOMAIN_NAME, &index, &len);
    if (isthere == TRUE)
    {
        params->domain_name = (PFBYTE)&(ack->options[index]);
        params->domain_name_len = len;
    }

    isthere = is_dhcp_op_there(ack, MDRS, &index, &len);
    if (isthere == TRUE)
        params->mdrs = (PFWORD)&(ack->options[index]);

    isthere = is_dhcp_op_there(ack, DEFAULT_IP_TTL, &index, &len);
    if (isthere == TRUE)
        params->default_ip_ttl = (PFBYTE)&(ack->options[index]);

    isthere = is_dhcp_op_there(ack, INTERFACE_MTU, &index, &len);
    if (isthere == TRUE)
        params->mtu = (PFWORD)&(ack->options[index]);

    isthere = is_dhcp_op_there(ack, STATIC_ROUTE_OP, &index, &len);
    if (isthere == TRUE)
    {
        params->static_route_len = len;
        params->static_route = (PFDWORD)&(ack->options[index]);
    }

    isthere = is_dhcp_op_there(ack, ROUTER_OPTION, &index, &len);
    if (isthere == TRUE)
    {
        params->router_option_len = len;
        params->router_option = (PFDWORD)&(ack->options[index]);
    }

    isthere = is_dhcp_op_there(ack, ARP_CT_OP, &index, &len);
    if (isthere == TRUE)
        params->arp_cache_timeout = (PFDWORD)&(ack->options[index]);

    isthere = is_dhcp_op_there(ack, TCP_DEFAULT_TTL, &index, &len);
    if (isthere == TRUE)
        params->tcp_default_ttl = (PFBYTE)&(ack->options[index]);

    isthere = is_dhcp_op_there(ack, TCP_KA_INTERVAL, &index, &len);
    if (isthere == TRUE)
        params->tcp_ka_interval = (PFDWORD)&(ack->options[index]);

    isthere = is_dhcp_op_there(ack, TCP_KA_GARBAGE, &index, &len);
    if (isthere == TRUE)
        params->tcp_ka_garbage = (PFBYTE)&(ack->options[index]);

#if (INCLUDE_TFTP_CISCO)
    isthere = is_dhcp_op_there(ack, TFTP_ADDR_CISCO, &index, &len);
    if (isthere == TRUE)
        params->tftp_srv_ip = (PFDWORD)&(ack->options[index]);
#endif
}

int is_dhcp_response_valid(PFDHCP_packet client_pkt, PFDHCP_packet serv_resp)
{
byte msg_type;
int validity = DHCP_VALID;
int n;
int result;
dword dw;
byte buf[100];

    /* header check   */
    if (serv_resp->op != BOOTREPLY)
    {
        DEBUG_LOG("op not BOOTREPLY", LEVEL_2, NOVAR, 0, 0);
        return DHCP_INVALID;
    }

    if (serv_resp->xid != client_pkt->xid)
    {
        DEBUG_LOG("xid mismatch", LEVEL_2, NOVAR, 0, 0);
        return DHCP_INVALID;
    }
    if (serv_resp->secs != 0)
    {
        DEBUG_LOG("bad secs", LEVEL_2, NOVAR, 0, 0);
        return DHCP_INVALID;
    }
    if (serv_resp->flags != 0 && serv_resp->flags != client_pkt->flags)
    {
        DEBUG_LOG("bad flags", LEVEL_2, NOVAR, 0, 0);
        return DHCP_INVALID;
    }
    if (serv_resp->giaddr != 0)
    {
        if (serv_resp->flags != client_pkt->flags)
            validity = DHCP_ACCEPTABLE;
    }
    else
    {
        if (serv_resp->flags != 0)
            validity = DHCP_ACCEPTABLE;
    }

    for (n=0; n<DHCP_PKT_CHADDR_SIZE; n++)
    {
        if (serv_resp->chaddr[n] != client_pkt->chaddr[n])
        {
            DEBUG_LOG("chaddr mismatch", LEVEL_2, NOVAR, 0, 0);
            return DHCP_INVALID;
        }
    }

    /* options check   */
    DEBUG_LOG("evaluating:", LEVEL_2, NOVAR, 0, 0);
/*    dhcp_dump_packet_ops(serv_resp);   */
    result = get_dhcp_op(serv_resp, DHCP_MSG_TYPE, sizeof(byte), 
                         (PFVOID)&msg_type);
    if (result == -1)
    {
        DEBUG_LOG("missing DHCP_MSG_TYPE", LEVEL_2, NOVAR, 0, 0);
        return DHCP_INVALID;
    }

    if ((msg_type != DHCPOFFER) && (msg_type != DHCPACK) &&
        (msg_type != DHCPNAK))
    {
        DEBUG_LOG("DHCP_MSG_TYPE invalid", LEVEL_2, NOVAR, 0, 0);
        DEBUG_LOG("msg type = ", LEVEL_2, DINT1, (dword)msg_type, 0);
        return DHCP_INVALID;
    }

    if (get_dhcp_op(serv_resp, REQ_IP, sizeof(dword), (PFVOID)&dw) != -1)
    {
        DEBUG_LOG("REQ_IP present.", LEVEL_2, NOVAR, 0, 0);
        return DHCP_INVALID;
    }

    if (get_dhcp_op(serv_resp, PARAM_REQ_LST, 100, (PFVOID)buf) != -1)
    {
        DEBUG_LOG("PARAM_REQ_LST present", LEVEL_2, NOVAR, 0, 0);
        return DHCP_INVALID;
    }

    if (get_dhcp_op(serv_resp, CLIENT_ID, 100, (PFVOID)buf) != -1)
    {
        DEBUG_LOG("CLIENT_ID present", LEVEL_2, NOVAR, 0, 0);
        return DHCP_INVALID;
    }

    if (get_dhcp_op(serv_resp, CLASS_ID, 100, (PFVOID)buf) != -1)
    {
        DEBUG_LOG("CLASS_ID present", LEVEL_2, NOVAR, 0, 0);
        return DHCP_INVALID;
    }

    if (msg_type == DHCPOFFER)
    {
        if (get_dhcp_op(serv_resp, IP_LEASE, sizeof(dword), (PFVOID)&dw) == -1)
        {
            DEBUG_LOG("No IP_LEASE in DHCPOFFER", LEVEL_2, NOVAR, 0, 0);
            validity = DHCP_ACCEPTABLE;
        }
        if (get_dhcp_op(serv_resp, SERVER_ID, sizeof(dword), (PFVOID)&dw) == -1)
        {
            DEBUG_LOG("No SERVER_ID in DHCPOFFER", LEVEL_2, NOVAR, 0, 0);
            return DHCP_INVALID;
        }
    }

    if (msg_type == DHCPACK)
    {
        if (get_dhcp_op(serv_resp, IP_LEASE, sizeof(dword), (PFVOID)&dw) == -1)
        {
            DEBUG_LOG("No IP_LEASE in DHCPACK", LEVEL_2, NOVAR, 0, 0);
            validity = DHCP_ACCEPTABLE;
        }
    }

    if (msg_type == DHCPNAK)
    {
        if (get_dhcp_op(serv_resp, IP_LEASE, sizeof(dword), (PFVOID)&dw) != -1)
        {
            DEBUG_LOG("IP_LEASE in DHCPNAK", LEVEL_2, NOVAR, 0, 0);
            validity = DHCP_ACCEPTABLE;
        }
    }
    return validity;
}

/* returns offset of best offer in offers array                     */
/* best offer is based upon priorites setup in parameter to xn_dhcp */
int dhcp_choose_offer(PFDHCP_packet offers, int num_offers, PFDHCP_conf conf)
{
int score, maxscore = -1;
int imax = -1;    /* index pointing to offer with max score */
int n, m;    /* looping variables */
int blahint;

    DEBUG_LOG("in choose offer", LEVEL_2, NOVAR, 0, 0);
    DEBUG_LOG("num offers = ", LEVEL_2, DINT1, (dword)num_offers, 0);
    for (n=0; n<num_offers; n++)
    {
        score = 0;
        if (conf->request_std_params == TRUE)
        {
            for (m=0; m<DHCP_STD_PARAMS; m++)
            {
                if (is_dhcp_op_there(&(offers[n]), std_param_lst[m].id, &blahint,
                    &blahint) == TRUE)
                {
                    if (score != -1)
                        score += std_param_lst[m].prio;
                }
                else
                {
                    if (std_param_lst[m].prio == DHCP_MUST_HAVE)
                    {
                        score = -1; /* this offer disqualified */
                        DEBUG_LOG("dq std", LEVEL_2, NOVAR, 0, 0);
                    }
                }
            }
        }
        if (conf->plist_entries > 0)
        {
            for (m=0; m<conf->plist_entries; m++)
            {
                if (is_dhcp_op_there(&(offers[n]), conf->plist[m].id, &blahint,
                    &blahint) == TRUE)
                {
                    if (score != -1)
                        score += conf->plist[m].prio;
                }
                else
                {
                    if (conf->plist[m].prio == DHCP_MUST_HAVE)
                    {
                        score = -1; /* this offer disqualified */
                        DEBUG_LOG("dq op", LEVEL_2, NOVAR, 0, 0);
                    }
                }
            }
        }
        DEBUG_LOG("score = ", LEVEL_2, DINT1, (dword)score, 0);
        if (score > maxscore)
        {
            DEBUG_LOG("max score beaten", LEVEL_2, NOVAR, 0, 0);
            maxscore = score;
            imax = n;
        }
    }
    return imax; /* returns -1 if there are no acceptable offers */
}

/* ********************************************************************   */
/* DHCP CLIENT INTERNAL FUNCTIONS (socket, send, recv etc)                */
/* ********************************************************************   */
int open_dhcp_socket(int iface_no)
{
int dhcpc_sock;
struct sockaddr_in client_bindto_sin;
PIFACE pi;
PUDPPORT port;

    tc_mv4((PFBYTE)&client_bindto_sin.sin_addr, ip_nulladdr, 4);
    client_bindto_sin.sin_family = AF_INET;
    client_bindto_sin.sin_port = hs2net(DHCP_CLIENT_PORT);


    dhcpc_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (dhcpc_sock < 0)
    {
        return(dhcpc_sock);
    }

    if (dhcpc_sock >= 0)
    {
        if (!set_non_blocking_mode(dhcpc_sock))
        {
            DEBUG_ERROR("extend_dhcp_lease_start: set non-blocking failed",
                NOVAR, 0, 0);
        }
    }

    if (bind(dhcpc_sock, (PCSOCKADDR)&client_bindto_sin,  
             sizeof(struct sockaddr_in)) < 0)
    {
        DEBUG_ERROR("open_dhcp_socket failed", NOVAR, 0, 0);
        closesocket(dhcpc_sock);
        dhcpc_sock = -1;
    }

    /* make sure only broadcast to interface passed to xn_dhcp()   */
    pi = tc_ino2_iface(iface_no, DONT_SET_ERRNO);
    port = (PUDPPORT)(api_sock_to_port(dhcpc_sock));
    if (pi && port)
    {
        port->broadcast_pi = pi;
    }
    return dhcpc_sock;
}

/* waits timeout_in_ticks for packet to be available, then reads the packet   */
/*                                                                            */
/* returns 0 if timeout before packet is available                            */
/* returns -1 upon error                                                      */
/* returns 1 if packet is read                                                */
int recv_dhcp_packet(PFDHCP_packet recv_pkt, int dhcpc_sock,
                     long timeout_in_ticks)
{
struct timeval wait_time;
struct fd_set f_read;
struct sockaddr_in blah_sin;
int sinsize = sizeof(struct sockaddr_in);
int result;

    DEBUG_LOG("recv called", LEVEL_2, NOVAR, 0, 0);

    FD_ZERO((PFDSET)&f_read);
    FD_SET(dhcpc_sock, (PFDSET)&f_read);
    wait_time.tv_sec = timeout_in_ticks/ks_ticks_p_sec();
    wait_time.tv_usec = 0;

#ifdef USE_NORTEL_SELECT
    if (select(dhcpc_sock+1, (PFDSET)&f_read,(PFDSET)0,(PFDSET)0,
               (PCTIMEVAL)&wait_time) > 0)
#else
    if (select(1, (PFDSET)&f_read,(PFDSET)0,(PFDSET)0,
               (PCTIMEVAL)&wait_time) > 0)
#endif
    {
        /* got a packet   */
        result = recvfrom(dhcpc_sock, (PFCHAR)recv_pkt,
                          sizeof(DHCP_packet)-DHCP_PKT_EXTRA, 0,
                          (PSOCKADDR)&blah_sin, (PFINT)&sinsize);
        if (result == -1)
            return -1;
        else
        {
#            if (DHCP_DUMP_PACKETS == 1)
                DEBUG_LOG("MSG recvd", LEVEL_2, NOVAR, 0, 0);
                dhcp_dump_packet(recv_pkt);
#            endif
            return 1; /* 1 packet received */
        }
    }
    return 0; /* timed out */
}

/* returns -1 upon error;   */
int send_dhcp_packet(PFDHCP_packet send_pkt, int dhcpc_sock, PFCBYTE ip_addr)
{
struct sockaddr_in sin;

/*  #if (DHCP_DUMP_PACKETS == 1)   */
        DEBUG_LOG("MSG sent", LEVEL_2, NOVAR, 0, 0);
/*        dhcp_dump_packet(send_pkt);   */
/*  #endif                              */

    sin.sin_family = AF_INET;
    sin.sin_port = hs2net(DHCP_SERVER_PORT);
    tc_mv4( (PFBYTE)(&(sin.sin_addr)), (PFBYTE)(ip_addr), 4);
    DEBUG_LOG("calling sendto . . .", LEVEL_2, NOVAR, 0, 0);
    return( sendto(dhcpc_sock, (PFCHAR)send_pkt,
                   (sizeof(DHCP_packet)-DHCP_PKT_EXTRA), 0,
                   (PCSOCKADDR)&sin, sizeof(struct sockaddr_in)) );
}

/* send DHCP packet and wait for response; does retries if no   */
/* response                                                     */
/* returns -1 if error                                          */
int do_dhcp_packet(PFDHCP_packet outmsg, PFDHCP_packet replies, 
                   int dhcpc_sock, PFCBYTE ip_addr, int max_replies, 
                   int timeout, int max_retries, byte ok_msg_types)
{
int n;
dword start_time;
long timeout_ticks;
int rindex = 0;   /* index into replies */
int result;
int validity;
byte msg_type_stat[7]; /* tells if each message type is one we want */
byte msg_type = 0;

    /* fill in msg_type_stat   */
    for (n=0; n<7; n++)
        msg_type_stat[n] = (byte)((ok_msg_types>>n) & (0x01));

    /* retry max_retries times only if did NOT get any replies   */
    for (n=0; n<=max_retries && rindex==0; n++)
    {
#if (DEBUG_DHCP)
        DEBUG_ERROR("do_dhcp_packet: send pkt type, try = ", EBS_INT2, 
            outmsg->op, n);
#endif

        /* if less than one second left, don't bother to try anymore   */
        if (timeout <= 1)  
            return(rindex);     /* return 0 */

        if (send_dhcp_packet(outmsg, dhcpc_sock, ip_addr) < 0)
        {
#if (DEBUG_DHCP)
            DEBUG_ERROR("do_dhcp_packet: send failed: return -1", NOVAR, 0, 0);
#endif
            return(-1);     /* errno set by sendto */
        }

        /* convert timeout to ticks:   */
        timeout_ticks = timeout*ks_ticks_p_sec();

        while (timeout_ticks > 0 && rindex < max_replies)
        {
            start_time = ks_get_ticks();
            result = recv_dhcp_packet(&(replies[rindex]), dhcpc_sock,
                                      timeout_ticks);
            if (result == -1)
            {
#if (DEBUG_DHCP)
                DEBUG_ERROR("do_dhcp_packet: recv_dhcp_packet failed: return -1", NOVAR, 0, 0);
#endif
                return result;  /* errno set by recv */
            }

            timeout_ticks -= (long)(ks_get_ticks()-start_time);
            if (result == 1) /* we got a reply */
            {
                DEBUG_LOG("got a reply", LEVEL_2, NOVAR, 0, 0);

                /* check if reply is valid. If so, keep it.   */
                validity = is_dhcp_response_valid(outmsg, &(replies[rindex]));
                DEBUG_LOG("validity = ", LEVEL_2, DINT1, (dword)validity, 0);
                if (validity != DHCP_INVALID)
                {
                    DEBUG_LOG("reply is valid", LEVEL_2, NOVAR, 0, 0);
                    get_dhcp_op(&(replies[rindex]), DHCP_MSG_TYPE, sizeof(byte),
                        (PFVOID)&msg_type);
                    if (msg_type_stat[msg_type-1] == 1)  /* message type is ok */
                    {
                        rindex++;
                        DEBUG_LOG("rindex inc", LEVEL_2, NOVAR, 0, 0);
                    }
                }
            }
        }
    }
#if (DEBUG_DHCP)
    DEBUG_ERROR("do_dhcp_packet: done return replies = ", EBS_INT1, rindex, 0);
#endif
    return rindex;
}

/* ********************************************************************   */
/* FORMAT PACKETS                                                         */
/* ********************************************************************   */
/* format DHCP discover or DHCP request (specified by msg_type)           */
int format_dhcp_discinf(PFDHCP_packet packet, int iface_no, PFDHCP_conf conf, 
                        byte msg_type)
{
/* tbd - this assumes ethernet; there are not any types for RS232 in RFC   */
byte htype = ETHERNET_10MB;
byte hlen = ETH_ALEN;
int n;
byte client_id[DHCP_PKT_CHADDR_SIZE+1];
byte param_req_lst[NUM_DHCP_OPS];
dword lease_time;
PIFACE pi;

    /* TBD: use iface_no to detect net info; for now it's hardcoded   */
    /* set up packet header                                           */
    packet->op = BOOTREQUEST;
    packet->htype = htype;
    packet->hlen = hlen;
    packet->hops = 0;
    packet->xid = ks_get_ticks(); /* random number */
    packet->secs = 0;

    packet->flags = 0;

    tc_movebytes((PFBYTE)&(packet->yiaddr), (PFBYTE)ip_nulladdr, 
                 sizeof(dword));
    packet->siaddr = 0;
    packet->giaddr = 0;
    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
        return(-1);
    tc_memset((PFBYTE)packet->chaddr, 0, DHCP_PKT_CHADDR_SIZE);
    tc_movebytes((PFBYTE)packet->chaddr, (PFBYTE)pi->addr.my_hw_addr,
                 ETH_ALEN);

    /* set message type   */
    set_dhcp_op(packet, DHCP_MSG_TYPE, 1, (PFVOID)&msg_type);

    /* set client id; this option is needed for NT servers   */
    client_id[0] = packet->htype;
    tc_movebytes((PFBYTE)&(client_id[1]), (PFBYTE)(packet->chaddr),
                 ETH_ALEN);
    set_dhcp_op (packet, CLIENT_ID, ETH_ALEN + 1, client_id);

    if (conf->lease_time != 0)
    {
        /* set IP lease time in output packet; set it to a minimum   */
        /* value due to the way retry logic works (see dhcp_timeout) */
        lease_time = conf->lease_time;
        if (lease_time < DHCP_MIN_LEASE)
            lease_time = DHCP_MIN_LEASE;
        lease_time = hl2net(lease_time);
        set_dhcp_op(packet, IP_LEASE, sizeof(dword), (PFVOID)&(lease_time));

#if (DEBUG_RENEW || DEBUG_RENEW_STATES)
        DEBUG_ERROR("format_dhcp_discinf: request lease_time is ", DINT1,
            conf->lease_time, 0);
#endif
    }

    /* add options from standard parameter list   */
    if (conf->request_std_params == TRUE)
    {
        /* set param req list   */
        for (n=0; n<DHCP_STD_PARAMS; n++)
            param_req_lst[n] = std_param_lst[n].id;
        set_dhcp_op(packet, PARAM_REQ_LST, 
                    (byte)(DHCP_STD_PARAMS*sizeof(byte)),
                    param_req_lst);
    }

    /* add options from extra parameter list   */
    if (conf->plist_entries != 0 && (PFDSET)(conf->plist) != 0)
    {
        /* set param req list   */
        for (n=0; n<conf->plist_entries; n++)
            param_req_lst[n] = conf->plist[n].id;

        set_dhcp_op(packet, PARAM_REQ_LST, 
            (byte)(conf->plist_entries*sizeof(byte)),
            param_req_lst);
    }
    if (conf->vs_size != 0 && (PFDSET)(conf->vendor_specific) != 0)
    {
        /* set vendor specific information   */
        set_dhcp_op(packet, VENDOR_SPECIFIC, (byte)(conf->vs_size), 
                    conf->vendor_specific);
    }
    return(0);
}

int format_dhcp_discover(PFDHCP_packet packet, int iface_no, PFDHCP_conf conf)
{
PFDHCP_cparam next; /* point to next parameter entry */
PIFACE pi;

    /* set up header fields common to all dhcp client packets & null out   */
    /* options field                                                       */
    if (format_dhcp_discinf(packet, iface_no, conf, DHCPDISCOVER) < 0)
        return(-1);

    packet->ciaddr = 0;

    /* request a specific IP address   */
    if (*((dword*)(conf->req_ip_addr)) != 0)
        set_dhcp_op(packet, REQ_IP, sizeof(dword), (PFVOID)(conf->req_ip_addr));

    /* add options from extra parameter list     */
    next = conf->cplist;
    while ((PFDSET)(next) != 0)
    {
        /* HERE                                                        */
        /* if domain name is to be passed to server, save it so renews */
        /* and new address requests will pass the local DNS name       */
        if (next->id == HOST_NAME_OP)
        {
            pi = tc_ino2_iface(iface_no, SET_ERRNO);
            if (pi)
                pi->addr.iface_flags |= DHCP_CLI_DDNS;
            xn_set_domain_name(next->cpdata);   /* assumes null terminated */
        }
        /* set up param list     */
        set_dhcp_op(packet, next->id, next->len, next->cpdata);
        next = (PFDHCP_cparam)next->next; 
    }

    return(0);
}

int format_dhcp_inform(PFDHCP_packet packet, int iface_no, PFDHCP_conf conf)
{
    /* set up header fields common to all dhcp client packets & null out   */
    /* options field                                                       */
    if (format_dhcp_discinf(packet, iface_no, conf, DHCPREQUEST) < 0)
        return(-1);     /* errno already set */

    tc_mv4((PFBYTE)&(packet->ciaddr), (PFBYTE)(conf->client_ip_addr), 4);

    return(0);
}

void format_dhcp_request(PFDHCP_packet discover, PFDHCP_packet offer,
                         int style)
{
byte msg_type = DHCPREQUEST;
byte server_id[4];

    /* RFC 2131, section 4.4.1, table 5: transaction ID in request   */
    /* should be the same as in the offer                            */
    discover->xid = offer->xid;

    set_dhcp_op(discover, DHCP_MSG_TYPE, sizeof(byte), (PFVOID)&msg_type);

    get_dhcp_op(offer, SERVER_ID, sizeof(dword), (PFVOID)server_id);
    set_dhcp_op(discover, SERVER_ID, sizeof(dword), (PFVOID)server_id);

    if (style == DHCP_RFC)
        remove_dhcp_op(discover, REQ_IP);
    if (style == DHCP_MICROSOFT)
        set_dhcp_op(discover, REQ_IP, sizeof(dword), (PFVOID)&(offer->yiaddr));
}

void format_dhcp_extend(PFDHCP_packet prev_req, dword ciaddr, 
                        dword lease_time)
{
    DEBUG_LOG("lease time passed to format_extend = ", LEVEL_2, DINT1,
        lease_time, 0);
    prev_req->ciaddr = ciaddr;
    remove_dhcp_op(prev_req, SERVER_ID);
    remove_dhcp_op(prev_req, REQ_IP);
    lease_time = htonl(lease_time);
    set_dhcp_op(prev_req, IP_LEASE, sizeof(dword), (PFVOID)&lease_time);

}

void format_dhcp_decrel(PFDHCP_packet packet, PFBYTE server_ip,
                        PFBYTE client_ip, byte msg_type)
{
    packet->xid++;
    tc_mv4((PFBYTE)&(packet->ciaddr), client_ip, 4);
    remove_dhcp_op(packet, REQ_IP);
    remove_dhcp_op(packet, IP_LEASE);
    remove_dhcp_op(packet, CLASS_ID);
    remove_dhcp_op(packet, PARAM_REQ_LST);
    remove_dhcp_op(packet, MAX_DHCP_MSG_SIZE);
    remove_dhcp_op(packet, VENDOR_SPECIFIC);
    set_dhcp_op(packet, DHCP_MSG_TYPE, sizeof(byte), (PFVOID)&msg_type);
    set_dhcp_op(packet, SERVER_ID, 4, (PFVOID)server_ip);
}
#endif      /* DHCP CLIENT */

/* used by CLIENT and SERVER   */
void init_dhcp_packet(PFDHCP_packet packet)
{
    tc_memset((PFBYTE)packet, 0, 
              sizeof(DHCP_packet)-DHCP_PKT_OP_SIZE-DHCP_PKT_EXTRA);
    tc_memset((PFBYTE)(packet->options), END, DHCP_PKT_OP_SIZE);

    /* this magic four byte sequence must be there.   */
    packet->options[0] = 99;
    packet->options[1] = 130;
    packet->options[2] = 83;
    packet->options[3] = 99;

    packet->options_index = 4;
    packet->overload_type = NO_OVERLOAD;
}


/* ********************************************************************   */
/* GENERAL OPTION UTILITIES (SERVER AND CLIENT)                           */
/* ********************************************************************   */
/* sets pindex to offset of option field, returns length in plen          */
RTIP_BOOLEAN is_dhcp_op_there(PFDHCP_packet packet, byte op_id,
                         PFINT pindex, PFINT plen)
{
int index, op, len = 0;
RTIP_BOOLEAN opfound;

    index = 4;
    opfound = FALSE;

    do
    {
        op = packet->options[index];
        if (op == END || op == PAD)
        {
            break;
        }
        if ((word)op == (word)op_id)
        {
            opfound = TRUE;
            index++;        /* point to length field */
            len = packet->options[index];
            index++;        /* point to value field */
            break;
        }
        index++;
        len = packet->options[index];
        index += len;
        index++;
    } while (index < DHCP_PKT_OP_SIZE);

    if (opfound == TRUE)
    {
        *pindex = index;
        *plen = len;
    }
    else
    {
        *pindex = 0;
        *plen = 0;
    }
    return opfound;
}

int set_dhcp_op(PFDHCP_packet packet, byte op_id, byte len, PFVOID vpdata)
{
RTIP_BOOLEAN opfound;
PFBYTE data = (PFBYTE)vpdata;
int this_op_len, index;

    /* first check to see if the op is already here., i.e.   */
    /* make sure it is not in there twice                    */
    opfound = is_dhcp_op_there(packet, op_id, &index, &this_op_len);

    if (opfound == TRUE)
    {
        if (len > this_op_len) /* option enlarge */
        {
            /* create more space:   */
            tc_movebytes(&(packet->options[index+len-1]),
                         &(packet->options[index+this_op_len-1]),
                         DHCP_PKT_OP_SIZE-(index+len));
        }

        if (len < this_op_len)    /* option shrink */
        {
            tc_movebytes(&(packet->options[index+len-1]),
                         &(packet->options[index+this_op_len-1]),
                         DHCP_PKT_OP_SIZE-(index+this_op_len));
        }
        /* record length   */
        packet->options[index-1] = len;
        tc_movebytes(&(packet->options[index]), data, len);
        packet->options_index = (word)(packet->options_index+len-this_op_len);
    }
    else
    {
        if (packet->options_index+len <= DHCP_PKT_OP_SIZE)
        {
            packet->options[packet->options_index] = op_id;
            packet->options_index++;
            packet->options[packet->options_index] = len;
            packet->options_index++;
            tc_movebytes(&(packet->options[packet->options_index]),
                data, len);
            packet->options_index = (word)(packet->options_index + len);
        }
        else
        {
            return set_errno(EDHCPOPOVERFLOW);
        }
    }
    return 0;
}

/* if option is found, returns option value in parameter pvdest   */
/* if option not found or other error, returns -1 and sets errno  */
int get_dhcp_op(PFDHCP_packet packet, byte op_id, byte bufsize, PFVOID vpdest)
{
int index = 0, len = 0;
RTIP_BOOLEAN opfound;
PFBYTE dest = (PFBYTE)vpdest;

    opfound = is_dhcp_op_there(packet, op_id, (PFINT)&index, (PFINT)&len);

    if (opfound == TRUE)
    {
        if (len <= bufsize)
        {
            tc_movebytes(dest, &(packet->options[index]), len);
        }
        else
        {
            return set_errno(EDHCPOPOVERFLOW);
        }
    }
    else
    {
        return set_errno(EDHCPOPNOTFOUND);
    }
    return 0;
}

#if (INCLUDE_DHCP_CLI)
int remove_dhcp_op(PFDHCP_packet packet, byte op_id)
{
word index, len = 0;
byte op = 0;
RTIP_BOOLEAN opfound;

    index = 4;
    opfound = FALSE;

    do
    {
        op = packet->options[index];
        if (op == END || op == PAD)
        {
            break;
        }
        if (op == op_id)
        {
            opfound = TRUE;
            index++;
            len = packet->options[index];
            index++;
            break;
        }
        index++;
        len = packet->options[index];
        index += len;
        index++;
    } while (index < DHCP_PKT_OP_SIZE);

    if (opfound == TRUE)
    {
        /* move remaining ops on top of this op
           we must subtract 2 from the index to overwrite the op id and op
           length fields */
        tc_movebytes(&(packet->options[index-2]),
                     &(packet->options[index+len]), 
                     DHCP_PKT_OP_SIZE-(index+len));
        /* adjust op index   */
        packet->options_index = (word)(packet->options_index-len-2);
    }
    else
    {
        return set_errno(EDHCPOPNOTFOUND);
    }
    return 0;
}

#endif      /* INCLUDE_DHCP_CLI */
#endif      /* INCLUDE_DHCP_CLI || INCLUDE_DHCP_SRV */


