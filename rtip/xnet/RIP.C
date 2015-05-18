/*                                                                        */
/* RIP.C  - RIP source file                                               */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains source file for RIP                          */

#define DIAG_SECTION_KERNEL DIAG_SECTION_RIP

#include "sock.h"

#if (INCLUDE_RIP)

#include "rip.h"

/* *********************************************************************   */
void _rip_main_(void);

/* *********************************************************************   */
extern ROUTE KS_FAR rt_table[CFG_RTSIZE];   /* routing table */
extern byte         rip_buffer[RIP_MAX_PKTLEN];
extern byte         rip_version_running;
extern int          rip_udp_socket;
extern int          rip_secs_to_update;
extern int          rip_update_flag;
/* *********************************************************************   */
#define RT_INDEX(X) ((PROUTE)X - (PROUTE)rt_table)

/* *************************************************************************    */
/*  _rip_init_socket - initialize the RIP socket                                */
/*                                                                              */
/*      This function sets up a UDP socket for port 520.  The socket number     */
/*       is stored in the global rip_udp_socket.  Returns the contents of       */
/*       this variable (rip_udp_socket) (which is -1 if the open failed)        */
/*       If the socket is already initialized, it just returns rip_udp_socket   */
/*                                                                              */
/*      Returns rip_udp_socket if successful, -1 otherwise                      */
/*                                                                              */
/* *************************************************************************    */

int _rip_init_socket(void)
{
struct sockaddr_in sin;

    /* Listen on UDP port 520 for incoming RIP messages   */
    
    if (rip_udp_socket < 0)
    {

        if ((rip_udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            return (rip_udp_socket);
        }

        sin.sin_family = AF_INET;
#if (CFG_UNIX_COMPLIANT_SOCK_STRUCTS)
        sin.sin_addr.s_addr = INADDR_ANY;
#else
        sin.sin_addr = INADDR_ANY;
#endif
        sin.sin_port = htons(RIP_PORT);
    
        if (bind(rip_udp_socket,(PSOCKADDR)&sin,sizeof(sin)) < 0)
        {
            closesocket(rip_udp_socket);
            rip_udp_socket = -1;
            return (rip_udp_socket);
        }       
    }

    return (rip_udp_socket);
}

/* *************************************************************************    */
/*  _rip_daemon_cycle - process one RIP response or request                     */
/*                                                                              */
/*   This routine is non-reentrant; we only have 1 RIP task per system, so      */
/*    this is okay                                                              */
/*                                                                              */
/* *************************************************************************    */

int _rip_daemon_cycle(void)
{
struct rip_command rip_cmd;
    
    /* *********************************************************************   */
    /* NOTE: using rip_buffer makes RIP nonreentrant but that is ok            */
    rip_cmd.msg = (PRIPHDR) rip_buffer;  /* Global buffer for rip task */
    rip_cmd.src.sin_family = AF_INET;
    rip_cmd.srclen = sizeof(rip_cmd.src);

    rip_cmd.msglen = recvfrom (rip_udp_socket, (PFCHAR)rip_cmd.msg, RIP_MAX_PKTLEN, 0, (PSOCKADDR)&rip_cmd.src, &rip_cmd.srclen);

    if (rip_cmd.msglen > sizeof(RIPHDR))
    {
        if ((rip_cmd.msg->rp_version == 1) && (rip_cmd.msg->rp_res != 0))
            return(0);

        /* check version is legal    */
        if (!_rip_receive_okay(rip_cmd.msg->rp_version))
            return(0);

        switch (rip_cmd.msg->rp_command)
        {
#if (!INCLUDE_RIP_LISTEN_ONLY)
            case RIP_REQUEST:
                _rip_process_request(&rip_cmd);
                break;
#endif
    
            case RIP_RESPONSE:
                _rip_process_response(&rip_cmd);
                break;

            default:
                break;
        }
            
        return(0);
    }

    return(0);
}

/* *************************************************************************    */
/*  _rip_daemon - RIP daemon task                                               */
/*                                                                              */
/*      Initializes the RIP UDP socket (port 520) and continuously calls        */
/*       _rip_daemon_cycle to handle incoming RIP packets.                      */
/*                                                                              */
/*      Returns nothing                                                         */
/*                                                                              */
/* *************************************************************************    */

void _rip_daemon(void)
{
    if (_rip_init_socket() < 0)
        return;

    _rip_broadcast_request_all(rip_get_version());

    for (;;)
    {
        _rip_daemon_cycle();
    }   
}

/* *************************************************************************    */
/*  _rip_process_request - process a single RIP request                         */
/*                                                                              */
/*                                                                              */
/*      Returns nothing                                                         */
/*                                                                              */
/* *************************************************************************    */

#if (!INCLUDE_RIP_LISTEN_ONLY)
void _rip_process_request(PRIP_COMMAND prip)
{
PRIPDATA    rpdata;
PROUTE  prt;
int     bytes_left;
byte        version;

#if (DEBUG_RIP)
    DEBUG_ERROR("\nRIP: REQUEST received from ", IPADDR, (PFBYTE) &(prip->src.sin_addr), 0);
#endif

    version = prip->msg->rp_version;

    if (!_rip_send_okay(version))
    {
        DEBUG_ERROR("\nRIP: Packet dropped, wrong version ", IPADDR, (PFBYTE) &(prip->src.sin_addr), 0);
        return;
    }

    rpdata = (PRIPDATA) (prip->msg + 1);
    bytes_left = prip->msglen - sizeof(RIPHDR);

    /* *********************************************************************   */
    /* Process the routing entries in the RIP message one by one               */
    while (bytes_left >= sizeof(RIPDATA))
    {
#if (INCLUDE_RIPV1)
        if ( (version == 1) && ((rpdata->rp_tag != 0) || 
                 (byte_to_long((PFDWORD)rpdata->rp_mask) != 0) || 
                 (byte_to_long((PFDWORD)rpdata->rp_next) != 0)) )
        {
            return;     /* drop request */
        }
#endif

#if (INCLUDE_RIPV2)
        if (rpdata->rp_family == RIP_AF_AUTH)
        {
            if (version == 2)
            {
                if (!_rip_process_authentication(prip, (PRIPAUTH) rpdata))
                {
                    DEBUG_ERROR("\nRIP: Packet dropped, bad authentication ", IPADDR, (PFBYTE) &(prip->src.sin_addr), 0);
                    return;     /* drop request */
                }
            }
            bytes_left -= sizeof(RIPDATA);
            rpdata++;
            continue;       /* drop part of the request */
        }
#endif

        /* if family is 0 and metric is 16 (infinity), then this is a special   */
        /*  request for the entire routing table                                */

        if ((rpdata->rp_family == AF_UNSPEC) && 
            (byte_to_long(&rpdata->rp_metric) == htonl(RTM_INF)))
        {
#if (DEBUG_RIP)
            DEBUG_ERROR("\nRIP: Sending routing table (SOLICITED)", NOVAR, 0, 0);
#endif
            _rip_send_rt_table((PCSOCKADDR)&prip->src, prip->srclen, 0, version);
            return;
        }
        
        if (rpdata->rp_family != htons(AF_INET))
        {
            bytes_left -= sizeof(RIPDATA);
            rpdata++;
            DEBUG_ERROR("\nRIP: Route entry ignored: bad family", NOVAR, 0, 0);
            continue;
        }

        /* normal request processing involves looking up each entry in the     */
        /*  RIP message in our local routing table, replacing the metric field */
        /*  with our value for the metric if we find the address, 16 otherwise */

        prt = rt_get(rpdata->rp_addr);

        if (prt)
        {
#if (INCLUDE_RIPV2)
            if (version == 2)
            {
                rpdata->rp_tag = htons(prt->rt_tag);
                tc_mv4(rpdata->rp_mask, prt->rt_mask, IP_ALEN);

                /* Sending the next hop will only help if he's on the same    */
                /*  iface as the sender of this request                       */

                if (prt->rt_iface == 
                            rt_get_output((PFBYTE) &(prip->src.sin_addr), 0, 0, 0))
                    tc_mv4(rpdata->rp_next, prt->rt_gw, IP_ALEN);
            }
#endif
            long_to_bytes(&rpdata->rp_metric, htonl(prt->rt_metric));
        }
        else 
            long_to_bytes(&rpdata->rp_metric, htonl(RTM_INF));

        bytes_left -= sizeof(RIPDATA);
        rpdata++;
    }

    /* now we send the request back to the sender as a response   */

    if (_rip_init_socket() < 0)
        return;

#if (DEBUG_RIP)
    DEBUG_ERROR("\nRIP: Sending RESPONSE (SOLICITED)", NOVAR, 0, 0);
#endif

    prip->msg->rp_command = RIP_RESPONSE;
    if (sendto (rip_udp_socket, (PFCHAR)prip->msg, prip->msglen, 0, 
                (PCSOCKADDR)&prip->src, prip->srclen) < prip->msglen)
    {   
        DEBUG_ERROR("RIP: Send error", NOVAR, 0, 0);
    }
}
#endif

/* *************************************************************************    */
/*  _rip_process_response - process a single RIP response                       */
/*                                                                              */
/*                                                                              */
/*      Returns nothing                                                         */
/*                                                                              */
/* *************************************************************************    */

void _rip_process_response(PRIP_COMMAND prip)
{
PRIPDATA    rpdata;
PROUTE  prt;
int     bytes_left;
int     src_iface;
PFBYTE  src_addr;
PIFACE  pi;
dword   rp_metric;
byte        version;
int     rt_changed;

    src_addr = (PFBYTE) &(prip->src.sin_addr);

#if (DEBUG_RIP)
    DEBUG_ERROR("\nRIP: RESPONSE received from ", IPADDR, src_addr, 0);
#endif

    /* Ignore if this message is from us   */

    if (tc_get_local_pi(src_addr))
    {
        return;
    }   

    /*  Don't accept responses from routers we don't know about (or can't match   */
    /*  to an interface through subnet prefix)                                    */

    src_iface = rt_get_output(src_addr, 0, 0, 0);
    if (src_iface < 0)
    {
        DEBUG_ERROR("\nRIP: Packet dropped, bad source address", NOVAR,0,0);
        return;
    }
    pi = tc_ino2_iface(src_iface,DONT_SET_ERRNO);

    rpdata = (PRIPDATA) (prip->msg + 1);
    bytes_left = prip->msglen - sizeof(RIPHDR);
    version = prip->msg->rp_version;

#if (INCLUDE_RIPV1)
    /* *********************************************************************   */
    /* VERIFY RESPONSE                                                         */
    /* *********************************************************************   */
    /* This is a first pass on the packet: we check to make sure that no       */
    /*  reserved fields are non-zero.  If they are, discard this packet        */

    if (version == 1)
    {
        while (bytes_left >= sizeof(RIPDATA))
        {
            if ((rpdata->rp_tag != 0) || 
                 (byte_to_long((PFDWORD)rpdata->rp_mask) != 0) || 
                 (byte_to_long((PFDWORD)rpdata->rp_next) != 0))
            {
                return;
            }

            bytes_left -= sizeof(RIPDATA);
            rpdata++;
        }

        rpdata = (PRIPDATA) (prip->msg + 1);
        bytes_left = prip->msglen - sizeof(RIPHDR);
    }
#else /* INCLUDE_RIPV1 == 0 */

    if (version == 1)
        return;

#endif 

    /* *********************************************************************   */
    /* PROCESS RESPONSE                                                        */
    /* *********************************************************************   */

    /* Go through the route entries one at a time, updating our routing table   */
    /*  as needed.                                                              */
    rt_changed = 0;
    while (bytes_left >= sizeof(RIPDATA))
    {
#if (INCLUDE_RIPV2)
        if (rpdata->rp_family == RIP_AF_AUTH)
        {
            if (version == 2)
            {
                if (!_rip_process_authentication(prip, (PRIPAUTH) rpdata))
                {
                    DEBUG_ERROR("\nRIP: Packet dropped, bad authentication ", IPADDR, (PFBYTE) &(prip->src.sin_addr), 0);
                    return;
                }
            }
            bytes_left -= sizeof(RIPDATA);
            rpdata++;
            continue;       /* drop part of response */
        }
#endif

        /* Make sure the route destination is not a broadcast address and its   */
        /*  family is AF_INET                                                   */

        if ((rpdata->rp_addr[3] != 0xff) && (rpdata->rp_family == htons(AF_INET)))
        {
            /* If we're running RIP version 1, accept version 2 packets, but   */
            /* ignore all additional information                               */

#if (INCLUDE_RIPV1)
            if (rip_get_version() == 1)
            {
                rpdata->rp_tag = 0;
                long_to_bytes((PFDWORD)rpdata->rp_mask, 0);
                long_to_bytes((PFDWORD)rpdata->rp_next, 0);
            }
#endif 

            /* Convert metric to host byte order and update it to include the   */
            /* local network cost                                               */

            rp_metric = ntohl(byte_to_long(&(rpdata->rp_metric)));

            /* Don't even process routes whose metric is greater than infinity   */
    
            if (rp_metric > RTM_INF)
            {
                bytes_left -= sizeof(RIPDATA);
                rpdata++;
                continue;
            }

            /* Update the metric to include the interface it will be sent over   */
            
            rp_metric += pi->addr.metric;
            if (rp_metric > RTM_INF)
                rp_metric = RTM_INF;

            /* Copy the source ip address into the next hop field if next hop   */
            /*  is unspecified                                                  */

            if (byte_to_long((PFDWORD)rpdata->rp_next) == 0)
            {
                tc_mv4(rpdata->rp_next, src_addr, IP_ALEN);
            }

            /* Next hop MUST be on the same interface as the router sending   */
            /*  this packet                                                   */
            else 
            if (rt_get_output(rpdata->rp_next,0,0,0) != src_iface)
            {
                bytes_left -= sizeof(RIPDATA);
                rpdata++;
                DEBUG_ERROR("\nRIP: Route ignored, bad next hop", NOVAR,0,0);
                continue;
            }

            /* Find the destination ip address in our routing table   */

            prt = rt_get(rpdata->rp_addr);

            /* NEVER match to the default route   */

            if (prt && tc_cmp4(prt->rt_dest, RT_DEFAULT, IP_ALEN) &&
                !tc_cmp4(rpdata->rp_addr, RT_DEFAULT, IP_ALEN) )
            {
                prt = 0;
            }

            if (prt)
            {
                if (prt->rt_flags & RT_GW)
                {
                   OS_CLAIM_TABLE(RT_GET_TABLE_CLAIM)
                
                    /* If this response is from the current router for this    */
                    /*  destination, we always update the metric; otherwise,   */
                    /*  we only change the route if the new metric is smaller  */
                    /*  than the current one                                   */

                    if (tc_cmp4(rpdata->rp_next, prt->rt_gw, IP_ALEN))
                    {
#if (DEBUG_RIP)
                        DEBUG_ERROR("RIP: Updating RTE (new metric): ", NOVAR, 0, 0);
                        DEBUG_ERROR("", RT_ENTRY, RT_INDEX(prt), 0);
#endif
                        /* If the metric is infinity, invalidate but don't delete   */
                        /*  the entry so we can notify neighbors that this is a     */
                        /*  dead route  (trigger update)                            */

                        if (rp_metric == RTM_INF)
                            prt->rt_ttl = RIP_GARBAGE_TTL;
                        else
                            prt->rt_ttl = RIP_TTL;
            
                        if (rp_metric != prt->rt_metric)
                        {
                            prt->rt_flags |= RT_CHANGED;
                            prt->rt_metric = rp_metric;
                            rt_changed = 1;
                        }
                    }

                    /* if the new metric is as good as the old, then switch if   */
                    /*  there is a chance of the current route timing out soon   */
                    else 
                    if ((rp_metric < prt->rt_metric) || 
                        ((rp_metric == prt->rt_metric) && (prt->rt_ttl < RIP_TTL/2)))
                    {
#if (DEBUG_RIP)
                        DEBUG_ERROR("RIP: Updating RTE (new gateway): ", NOVAR, 0, 0);
                        DEBUG_ERROR("", RT_ENTRY, RT_INDEX(prt), 0);
#endif
                        tc_mv4(prt->rt_gw, rpdata->rp_next, IP_ALEN);
                        prt->rt_iface = src_iface;
                        prt->rt_flags |= RT_CHANGED;
                        prt->rt_metric = rp_metric;
                        prt->rt_ttl = RIP_TTL;
                        rt_changed = 1;
                    }

                    OS_RELEASE_TABLE()
                }
            }
            /* If rpdata->rp_addr was not in the routing table, we create   */
            /*  a new entry                                                 */
            else
            {
#if (INCLUDE_RIPV1)
                if (rip_get_version() == 1)
                {               
                    /* calculate the subnet mask from address type and our own   */
                    /*  network information                                      */

                    _rip_get_mask(rpdata->rp_mask, rpdata->rp_addr);
                }
#endif

                rt_add(rpdata->rp_addr, rpdata->rp_mask, rpdata->rp_next, 
                       (int)rp_metric, src_iface, RIP_TTL, 
                       ntohs(rpdata->rp_tag), 0);

#if (DEBUG_RIP)
                DEBUG_ERROR("RIP: Creating new RTE ", NOVAR, 0, 0);
                DEBUG_ERROR("       Address: ", IPADDR, rpdata->rp_addr, 0);
                DEBUG_ERROR("       Mask:    ", IPADDR, rpdata->rp_mask, 0);
                DEBUG_ERROR("       Gateway: ", IPADDR, rpdata->rp_next, 0);
                DEBUG_ERROR("       Metric:  ", EBS_INT1, (int)rp_metric, 0);
                DEBUG_ERROR("       TTL:     ", EBS_INT1, (int)RIP_TTL, 0);
#endif
            }
        }

        bytes_left -= sizeof(RIPDATA);
        rpdata++;
    }

#if (!INCLUDE_RIP_LISTEN_ONLY)
    if (rt_changed)
        _rip_trigger_update();
#endif
}

/* *************************************************************************    */
/*  _rip_get_mask - get net mask for given address                              */
/*                                                                              */
/*                                                                              */
/* *************************************************************************    */

#if (INCLUDE_RIPV1)
void _rip_get_mask(PFBYTE mask, PFBYTE addr)
{
int i,n;
byte netmask[] = {0xff,0x0,0x0,0x0};
PIFACE pi;
            
    /* For the default address, return mask of all 0's    */

    if (byte_to_long((PFDWORD)addr) == 0)
    {
        tc_memset(mask, 0, IP_ALEN);
        return;     
    }

    /* First, figure out the mask for this address.  If we can    */
    /*  match the net prefix to a net we are on, we can specify   */
    /*  the subnet mask, otherwise we just use the net mask       */
    /*  (class A, B, or C)                                        */

    if ((addr[0] & 0x80) == 0x80)
        netmask[1] = 0xff;

   if ((addr[0] & 0xc0) == 0xc0)
        netmask[2] = 0xff;

    tc_mv4(mask, netmask, IP_ALEN);
                    
    for (i = 0; i < CFG_NIFACES; i++)
    {
        pi = tc_ino2_iface(i,DONT_SET_ERRNO);
        if (pi)
        {
            OS_CLAIM_IFACE(pi,STATS_CLAIM_IFACE)

            /* Try to match the net number of the rip address to   */
            /* the ip address of this interface                    */

            for (n = 0; n < IP_ALEN; n++)
            {
                if ((addr[n] & netmask[n]) != 
                    (pi->addr.my_ip_addr[n] & netmask[n]))
                    break;
            }

            /* We found a match; use the subnet mask for this iface   */

            if (n == IP_ALEN)
            {
                tc_mv4(mask, (PFBYTE) pi->addr.my_ip_mask, IP_ALEN);
                OS_RELEASE_IFACE(pi)
                break;
            }

            OS_RELEASE_IFACE(pi)
        }
    }
}
#endif /* INCLUDE_RIPV1 */

/* *************************************************************************    */
/*  _rip_broadcast_request_all - send RIP request on all interfaces             */
/*                                                                              */
/*    Broadcast complete table request over all open interfaces                 */
/*                                                                              */
/*      Returns nothing                                                         */
/*                                                                              */
/* *************************************************************************    */

void _rip_broadcast_request_all(byte version)
{
int         i;
PIFACE  pi;

    if (!_rip_send_okay(version))
        return;

    for (i = 0; i < CFG_NIFACES; i++)
    {
        pi = tc_ino2_iface(i,DONT_SET_ERRNO);
        if (pi)
        {
            _rip_send_request_all(pi, version);
        }
    }
}

/* *************************************************************************    */
/*  _rip_send_request_all - send RIP request for complete routing table info    */
/*                                                                              */
/*    Called when an interface is initialized - broadcast a request for         */
/*     routing tables over the specified interface                              */
/*                                                                              */
/*      Returns nothing                                                         */
/*                                                                              */
/* *************************************************************************    */

void _rip_send_request_all(PIFACE pi, byte version)
{
DCU     ppkt;
int     msglen;
PRIPHDR rpmsg;
struct sockaddr_in sin;

    if (!_rip_send_okay(version))
        return;

    if (pi->pdev->iface_type == LOOP_IFACE)
        return;

    if (_rip_init_socket() < 0)
        return;

    msglen = rip_pkt_size(1);
    
    if ((ppkt = os_alloc_packet(msglen,RIP_ALLOC)) == 0)
        return;
    rpmsg = (PRIPHDR) ppkt->data;
    rip_format_header(rpmsg, RIP_REQUEST, version);

    /* For complete routing information requests, family is set to 0,   */
    /*  metric to RTM_INF (16, the infinite cost)                       */
    rip_format_route_entry(rpmsg, 0, 0, 0, 0, 0, 0, RTM_INF);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(RIP_PORT);
    tc_mv4((PFBYTE)&sin.sin_addr,pi->addr.my_net_bc_ip_addr,IP_ALEN);

#if (DEBUG_RIP)
    DEBUG_ERROR("RIP: Sending REQUEST ALL to: ", IPADDR, 
                    pi->addr.my_net_bc_ip_addr, 0);
#endif

    if (sendto(rip_udp_socket, (PFCHAR)rpmsg, msglen, 0, (PCSOCKADDR)&sin, 
              sizeof(sin)) < msglen)
    {
        DEBUG_ERROR("\nRIP: Send error", NOVAR, 0, 0);
    }             

    os_free_packet(ppkt);
}

/* *************************************************************************    */
/*  _rip_broadcast_rt_table - send routing table over all interfaces            */
/*                                                                              */
/*    This routine should be called every 30 seconds.  It sends the entire      */
/*     routing table over all interfaces using split horizon filtering.  If     */
/*     the update flag is set, then only routing table entries marked as        */
/*     changed will be included.                                                */
/*                                                                              */
/*      Returns nothing                                                         */
/*                                                                              */
/* *************************************************************************    */

#if (!INCLUDE_RIP_LISTEN_ONLY)
void _rip_broadcast_rt_table(int update, byte version)
{
int         i;
PIFACE  pi;
PROUTE  prt;
struct sockaddr_in sin;

    if (!_rip_send_okay(version))
        return;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(RIP_PORT);
                    
    /* For a routing table broadcast, cycle through all the interfaces and   */
    /*  do a send to each of their net broadcast addresses individually.     */
    /*  This is done so that _rip_send_rt_table will correctly filter out    */
    /*  the appropriate entries for each iface according to the split        */
    /*  horizon algorithm.                                                   */

    for (i = 0; i < CFG_NIFACES; i++)
    {
        pi = tc_ino2_iface(i,DONT_SET_ERRNO);
        if (pi)
        {
            tc_mv4((PFBYTE)&sin.sin_addr,pi->addr.my_net_bc_ip_addr,IP_ALEN);
            _rip_send_rt_table((PCSOCKADDR)&sin, sizeof(sin), update, version);
        }
    }

    /* Regardless of whether this was a complete table broadcast or just an   */
    /*  update, we've now eliminated the need for an update, so clear all the */
    /*  routing table entries' CHANGED flag                                   */

   OS_CLAIM_TABLE(RT_GET_TABLE_CLAIM)

    for (i=0; i < CFG_RTSIZE; i++)
    {
        prt = (PROUTE)(&(rt_table[i]));
        prt->rt_flags &= ~RT_CHANGED;
    }

    OS_RELEASE_TABLE()
}

/* *************************************************************************    */
/*  _rip_send_rt_table - send routing table to destination                      */
/*                                                                              */
/*  This sends the routing table (with split horizon filtering) to a            */
/*   specific destination.  This can be used to broadcast an update over        */
/*   a single interface by using the net bc address for that interface.         */
/*       If the update flag is set, then only routing table entries marked as   */
/*     changed will be included.                                                */
/*                                                                              */
/*      Returns nothing                                                         */
/*                                                                              */
/* *************************************************************************    */

void _rip_send_rt_table(PCSOCKADDR dest, int size, int update, byte version)
{
DCU     ppkt;
int     i;
int     dest_iface;
int     n_to_send;
PFBYTE  dest_addr;
PIFACE  pi;
PROUTE  prt;
PRIPHDR rpmsg;

    if (!_rip_send_okay(version))
        return;

    if (_rip_init_socket() < 0)
        return;

    /* If we can't find an interface to send this response over, we can't do   */
    /*  split horizon, so fail                                                 */

    dest_addr = (PFBYTE)&((PSOCKADDR_IN)dest)->sin_addr;    
    if ((dest_iface = rt_get_output(dest_addr, 0, 0, 0)) < 0)
        return;

    if ((ppkt = os_alloc_packet(RIP_MAX_PKTLEN,RIP_ALLOC)) == 0)
        return;
    rpmsg = (PRIPHDR) ppkt->data;

#if (DEBUG_RIP)
    DEBUG_ERROR("\nRIP: Sending RESPONSE (complete routing table) to ", IPADDR, dest_addr, 0);
#endif

    rip_format_header(rpmsg, RIP_RESPONSE, version);
    n_to_send = 0;

    /* Go through the routing table and fill the outgoing packet, entry by   */
    /*  entry, sending when necessary                                        */
    
    for (i=0; i < CFG_RTSIZE; i++)
    {
       OS_CLAIM_TABLE(RT_GET_TABLE_CLAIM)

        prt = (PROUTE)(&(rt_table[i]));

        /* if entry is not valid, continue   */
        if ( RT_FREE(prt) || (prt->rt_flags & RT_DEL) || 
                (update && !(prt->rt_flags & RT_CHANGED)) )
        {
            OS_RELEASE_TABLE()
            continue;       
        }
        
        /* check to see if this entry was created from information sent over   */
        /*  dest_iface; if so, we don't want to send it back over dest_iface   */
        /*  (this is split-horizon filtering); also check for loopback         */

        if ((pi = tc_ino2_iface(prt->rt_iface,DONT_SET_ERRNO)) == 0)
        {
            OS_RELEASE_TABLE()
            continue;
        }
        
        if ((prt->rt_iface == dest_iface) || (pi->pdev->iface_type == LOOP_IFACE))
        {
            OS_RELEASE_TABLE()
            continue;
        }

        /* Never put the gateway in the next hop field since this is only    */
        /*  helpful to nodes on the same interface as the next hop; if this  */
        /*  is the case, however, slip horizon prohibits our sending this    */
        /*  entry to the destination                                         */

        rip_format_route_entry(rpmsg, n_to_send, AF_INET, (word) prt->rt_tag, 
                            prt->rt_dest, prt->rt_mask, 0, prt->rt_metric);
        OS_RELEASE_TABLE()

        n_to_send++;

#if (DEBUG_RIP)
        DEBUG_ERROR("RIP: ", RT_ENTRY, RT_INDEX(prt), 0);
#endif
        /* We can only send 25 routing table entries per RIP message   */

        if (n_to_send >= RIP_MAX_ENTRIES)
        {
            if (sendto(rip_udp_socket, (PFCHAR)rpmsg, 
                       rip_pkt_size(RIP_MAX_ENTRIES), 0, dest, size) <
                           rip_pkt_size(RIP_MAX_ENTRIES))
            {
                DEBUG_ERROR("\nRIP: Send error", NOVAR, 0, 0);
            }
            n_to_send = 0;
        }
    }

    /* Send all remaining routing table entries   */

    if (n_to_send > 0)
    {
        if (sendto(rip_udp_socket, (PFCHAR)rpmsg, 
                   rip_pkt_size(n_to_send), 0, dest, size) <
                       (int)rip_pkt_size(n_to_send))
        {
            DEBUG_ERROR("\nRIP: Send error", NOVAR, 0, 0);
        }
    }

    os_free_packet(ppkt);
}

/* *************************************************************************    */
/* _rip_timer - Do one second RIP processing                                    */
/*                                                                              */
/*  This function checks to see if it either is time to broadcast a periodic    */
/*  update of the routing table, or if an update has been triggered, and if     */
/*  either is the case, the routing table is broadcast as RIP responses over    */
/*  all nonloopback interfaces.                                                 */
/*                                                                              */
/* THIS FUNCTION MUST BE CALLED ONCE A SECOND FOR RIP TO FUNCTION PROPERLY      */
/*                                                                              */
/* Returns:                                                                     */
/*                                                                              */
/*  Nothing                                                                     */
/*                                                                              */
/* *************************************************************************    */

void _rip_timer(void)
{
    if (rip_get_version() == 0)
        return;
    
    rip_secs_to_update--;

    if (rip_secs_to_update <= 0)
    {
        rip_secs_to_update = 26 + (int)(ks_get_ticks() & 0x07);
        _rip_broadcast_rt_table(0,rip_get_version());
        rip_update_flag = 0;
    }   
    else if (rip_update_flag)
    {
        _rip_broadcast_rt_table(1,rip_get_version());
        rip_update_flag = 0;
    }   
}

/* *************************************************************************   */
/* _rip_trigger_update - Signal the timer task to send an update over all      */
/*                      nonloopback interfaces                                 */
/*                                                                             */
/* This is called when the routing table is modified so that our neighbors     */
/* can be updated                                                              */
/*                                                                             */
/* Returns:                                                                    */
/*                                                                             */
/*  Nothing                                                                    */
/*                                                                             */
/*                                                                             */

void _rip_trigger_update(void)
{
    rip_update_flag = 1;
}

#endif /* INCLUDE_RIP_LISTEN_ONLY == 0 */

/* *************************************************************************      */
/*  rip_format_header() - Format a RIP message                                    */
/*                                                                                */
/* Parameters:                                                                    */
/*                                                                                */
/*      PRIPHDR msg  - Pointer to the start of the buffer to be formatted as a    */
/*                     RIP message; this buffer should be of size                 */
/*                          rip_pkt_size(num_rt_entries), where num_rt_entries is */
/*                          the total number of route entries to be put in the    */
/*                          message                                               */
/*      byte command - RIP_REQUEST or RIP_RESPONSE                                */
/*      byte version - RIP version to be used: 1 or 2                             */
/*                                                                                */
/* Returns:                                                                       */
/*                                                                                */
/*  Nothing.                                                                      */
/*                                                                                */
/* *************************************************************************      */

void rip_format_header(PRIPHDR msg, byte command, byte version)
{
    if (!msg)
        return;

    msg->rp_command = command;
    msg->rp_version = version;
    msg->rp_res = 0;
}

/* *************************************************************************    */
/*  rip_format_route_entry - Format an individual route entry                   */
/*                                                                              */
/* Parameters:                                                                  */
/*                                                                              */
/*      PRIPHDR msg  -  Pointer to start of the RIP packet                      */
/*      int index    -  Index into packet of the route entry we wish to format  */
/*      word format  -  Must be AF_INET for this RIP implementation             */
/*      word tag         -  Route tag (RIP version 2 only); Exact usage of this */
/*                          field is not specified by the RIP specification     */
/*      PFBYTE addr  -  IP address of the route destination                     */
/*      PFBYTE mask  -  Subnet mask for this destination (RIP version 2 only)   */
/*      PFBYTE gw    -  Next hop address for this route (RIP version 2 only)    */
/*      dword metric -  Total cost metric for this route (must be <=RTM_INF)    */
/*                                                                              */
/* Description:                                                                 */
/*                                                                              */
/*  This function formats a single route entry within a RIP packet.  It         */
/*      can be used in conjunction with rip_format_header and rip_send_request  */
/*      to obtain specific routing information from neighboring gateways.       */
/*                                                                              */
/* Returns:                                                                     */
/*                                                                              */
/*  Nothing                                                                     */
/*                                                                              */
/* *************************************************************************    */

void rip_format_route_entry(PRIPHDR msg, int index, word family, word tag, PFBYTE addr, PFBYTE mask, PFBYTE gw, dword metric)
{
PRIPDATA rpdata;

    if (!msg)
        return;

    rpdata = (PRIPDATA) (msg + 1);
    rpdata += index;

    tc_memset ((PFBYTE) rpdata, 0, sizeof(RIPDATA));

    rpdata->rp_family = htons(family);

    if (addr)
        tc_mv4(rpdata->rp_addr, addr, IP_ALEN);

#if (INCLUDE_RIPV2)
    if (msg->rp_version == 2)
    {
        rpdata->rp_tag = htons(tag);

        if (mask)
            tc_mv4(rpdata->rp_mask, mask, IP_ALEN);

        if (gw)
            tc_mv4(rpdata->rp_next, gw, IP_ALEN);
    }
#endif

    long_to_bytes(&rpdata->rp_metric, htonl(metric));
}

/* *************************************************************************                   */
/*  rip_rte - Return a route entry from within a RIP message                                   */
/*                                                                                             */
/* Parameters:                                                                                 */
/*                                                                                             */
/*      PRIPHDR msg -   Pointer to start of the RIP packet                                     */
/*      int index   -   Index into packet of the route entry we wish to                 access */
/*                                                                                             */
/* Returns:                                                                                    */
/*                                                                                             */
/*      The desired route entry structure.                                                     */
/*                                                                                             */
/* *************************************************************************                   */

PRIPDATA rip_rte(PRIPHDR msg, int index)
{
PRIPDATA rpdata;

    if (!msg)
        return (0);

    rpdata = (PRIPDATA) (msg + 1);
    rpdata += index;

    return (rpdata);
}

/* *************************************************************************    */
/* _rip_process_authentication - Process RIPv2 authentication information       */
/*                                                                              */
/*  The user must implement the processing of the given authentication          */
/*  information                                                                 */
/*                                                                              */
/* Returns:                                                                     */
/*                                                                              */
/*      TRUE if packet is authenticated and processing should continue          */
/*      FALSE if the supplied authentication is bad                             */
/*                                                                              */
/* *************************************************************************    */

#if (INCLUDE_RIPV2)
RTIP_BOOLEAN _rip_process_authentication(PRIP_COMMAND prip, PRIPAUTH rpauth)
{
    /* it is left up to the user to define authentication processing   */
    if (!prip || !rpauth)
        return (FALSE);

    return (TRUE);
}
#endif /* INCLUDE_RIPV2 */

/* *************************************************************************                                                                        */
/*  rip_daemon_start - Start the RIP daemon and send out a request all on                                                                           */
/*                     all open interfaces                                                                                                          */
/*                                                                                                                                                  */
/* Parameters:                                                                                                                                      */
/*                                                                                                                                                  */
/*      byte version - RIP version to use for periodic and triggered updates                                                                        */
/*                                                                                                                                                  */
/* Description:                                                                                                                                     */
/*                                                                                                                                                  */
/*      This function will start the RIP daemon running as a task using os_spawn_task if a  real-time kernel is running, or it will start it as a P */
/*                                                                                                                                                  */
/*      Note: this function should be called after the network stack has been                                                                       */
/*      fully initialized and all interfaces opened.                                                                                                */
/*                                                                                                                                                  */
/* Returns:                                                                                                                                         */
/*                                                                                                                                                  */
/*      Nothing.                                                                                                                                    */
/*                                                                                                                                                  */
/* *************************************************************************                                                                        */

void rip_daemon_start(byte version)
{
    rip_set_version(version);
    
    if (!os_spawn_task(TASKCLASS_RIP_DAEMON,_rip_main_, 0,0,0,0))
    {
        DEBUG_ERROR("rip_daemon_start - spawning of rip task failed",
            NOVAR, 0, 0);
    }
}

/* *************************************************************************    */
/*  _rip_main_ - stub function for _rip_daemon                                  */
/*                                                                              */
/* Returns:                                                                     */
/*      Nothing                                                                 */
/*                                                                              */
/* *************************************************************************    */

void _rip_main_(void)
{
    for(;;)
    {
        _rip_daemon();
        ks_sleep(ks_ticks_p_sec());
    }
}

/* *************************************************************************    */
/*  rip_fake_response - "fake" a response from the specified port               */
/*                                                                              */
/*  This function will invoke the processing of the given message (which        */
/*  must be formatted just as it would appear coming over the RIP socket)       */
/*  including updating the routing table.  This is for debugging purposes       */
/*  only.                                                                       */
/*                                                                              */
/* Returns:                                                                     */
/*      Nothing                                                                 */
/*                                                                              */
/* *************************************************************************    */

void rip_fake_response(PRIPHDR msg, word rt_entries, PFBYTE src_ip, word port)
{
struct rip_command rip_cmd;

    rip_cmd.msg = msg;
    rip_cmd.msglen = rip_pkt_size(rt_entries);
    rip_cmd.src.sin_family = AF_INET;
    rip_cmd.src.sin_port = htons(port);              
    tc_mv4((PFBYTE)&(rip_cmd.src.sin_addr), src_ip, IP_ALEN);
    rip_cmd.srclen = sizeof(rip_cmd.src);

    _rip_process_response((PRIP_COMMAND) &rip_cmd);
}

/* *************************************************************************         */
/*  rip_get_version - Return the preferred RIP version                               */
/*                                                                                   */
/* Returns:                                                                          */
/*                                                                                   */
/*      The current preferred RIP version, or 0 if RIP is not currently running      */
/*      (rip_daemon_start has not been invoked).                                     */
/*                                                                                   */
/* *************************************************************************         */

byte rip_get_version(void)
{
    return (rip_version_running);
}

/* *************************************************************************    */
/*  rip_set_version - Set the preferred RIP version                             */
/*                                                                              */
/*      byte version - RIP version to use for periodic and triggered updates    */
/*                                                                              */
/* Description:                                                                 */
/*                                                                              */
/*      Sets the current preferred RIP version.                                 */
/*                                                                              */
/* Returns:                                                                     */
/*                                                                              */
/*      Nothing                                                                 */
/*                                                                              */
/* *************************************************************************    */

void rip_set_version(byte version)
{
    if ((version == 0) || (version > 2))
        return;

#if (!INCLUDE_RIPV1)
    if (version == 1)
        return;
#endif

#if (!INCLUDE_RIPV2)
    if (version == 2)
        return;
#endif

    rip_version_running = version;
}

/* *************************************************************************         */
/*  rip_send_request - send a preformatted RIP request                               */
/*                                                                                   */
/*      PRIPHDR msg   - Pointer to start of fully formatted RIP request              */
/*      word len      - Number of route entries in the request                       */
/*      PFBYTE dst_ip - IP address to send request to; may be a net                  */
/*                    broadcast address                                              */
/*      word port     - Source port to use when sending the request                  */
/*      word timeout  - If RTIP_INF, rip_send_request will block forever             */
/*                    waiting for a response from dst_ip.  Otherwise,                */
/*                    this specifies a timeout in seconds                            */
/*                                                                                   */
/*  This function sends a RIP request message (already formatted) to a               */
/*  destination and optionally waits for a response.  The response packet            */
/*  is put into the same    buffer as the request.  The format of the packet         */
/*  is the same, except the blank   fields in the route entries are filled           */
/*  in with information from the responder’s    routing table.  Once a response      */
/*  has been received, the route entries can be     retrieved using rip_rte.         */
/*  If the destination address is a broadcast address, then only the first           */
/*  response is returned. If the port is set to 520, then this function will         */
/*  return without blocking and all responses will go directly to the RIP            */
/*  daemon.                                                                          */
/*                                                                                   */
/* Returns:                                                                          */
/*                                                                                   */
/*     Number of bytes received in response if successful                            */
/*      -1 if a failure occurred                                                     */
/*                                                                                   */
/* *************************************************************************         */

int rip_send_request(PRIPHDR msg, word rt_entries, PFBYTE dst_ip, word port, short timeout)
{
int sd;
int len;
unsigned long arg;
struct fd_set fread;
struct timeval t;
struct sockaddr_in sin;

    if (port != RIP_PORT)
    {
        if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
            return (-1);

        sin.sin_family = AF_INET;
#if (CFG_UNIX_COMPLIANT_SOCK_STRUCTS)
        sin.sin_addr.s_addr = INADDR_ANY;
#else
        sin.sin_addr = INADDR_ANY;
#endif
        sin.sin_port = htons(port);

        if (bind(sd,(PSOCKADDR)&sin,sizeof(sin)) < 0)
        {
            closesocket(sd);
            return (-1);
        }       

        arg = 1;   /* enable non-blocking mode */
        if (ioctlsocket(sd, FIONBIO, (unsigned long KS_FAR *)&arg) != 0) 
        {
            closesocket(sd);
            return (0);
        }
    }
    else
        sd = rip_udp_socket;

    len = rip_pkt_size(rt_entries);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(RIP_PORT);
    tc_mv4((PFBYTE)&(sin.sin_addr), dst_ip, IP_ALEN);
    if (sendto(sd, (PFCHAR)msg, len, 0, (PCSOCKADDR)&sin, sizeof(sin)) < len)
    {
        DEBUG_ERROR("\nRIP: Send error", NOVAR, 0, 0);
    }

    if (port == RIP_PORT)
        return (0);

    /* *********************************************************************   */
    FD_ZERO(&fread);
    FD_SET(sd,&fread);

    if (timeout >= 0)
    {
        t.tv_sec = (long) timeout;
        t.tv_usec = 0;
        
        select(1, &fread, 0, 0, (PCTIMEVAL)&t);
    }
    else
    {
        select(1, &fread, 0, 0, 0);
    }

    arg = sizeof(sin);

   len = recvfrom(sd, (PFCHAR)msg, len, 0, (PSOCKADDR)&sin, (PFINT)&arg);

    closesocket(sd);

    return (len);
}

/* *************************************************************************    */
/*  _rip_send_okay - check the version of an outgoing RIP message               */
/*                                                                              */
/*      Returns TRUE if this system is configured to send packets of this type  */
/*                                                                              */
/* *************************************************************************    */

RTIP_BOOLEAN _rip_send_okay(byte version)
{
    if ((version == 0) || (version > 2))
        return (FALSE);

#if (!INCLUDE_RIPV1)
    /* cannot send version 1 of not configured   */
    if (version == 1)
        return (FALSE);
#endif

#if (!INCLUDE_RIPV2)
    /* cannot send version 2 if not configured   */
    if (version == 2)
        return (FALSE);
#endif

    return (TRUE);
}

/* *************************************************************************    */
/*  _rip_receive_okay - check the version of an incoming RIP message            */
/*                                                                              */
/*      Returns TRUE if this system is configured to process the packet         */
/*                                                                              */
/* *************************************************************************    */

RTIP_BOOLEAN _rip_receive_okay(byte version)
{
    if ((version == 0) || (version > 2))
        return (FALSE);

#if (!INCLUDE_RIPV1)
    /* cannot accept version 1 if not configured            */
    /* NOTE: can accept version 2 whether configured or not */
    if (version == 1)
        return (FALSE);
#endif

    return (TRUE);
}

/* *************************************************************************    */
/*  _rip_init_data - initialize RIP-related global data                         */
/*                                                                              */
/*      Returns nothing                                                         */
/*                                                                              */
/* *************************************************************************    */

void _rip_init_data(void)
{
    tc_memset(rip_buffer, 0, RIP_MAX_PKTLEN);
    rip_version_running = 0;
    rip_udp_socket = -1;
    rip_secs_to_update = 5;
    rip_update_flag = 0;
}


#endif

