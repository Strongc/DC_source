/*                                                                        */
/* ROUTE.C - Interface functions                                          */
/*                                                                        */
/* EBS - RTIP                                                             */
/*                                                                        */
/* Copyright Peter Van Oudenaren , 1993                                   */
/* All rights reserved.                                                   */
/* This code may not be redistributed in source or linkable object form   */
/* without the consent of its author.                                     */
/*                                                                        */
/* Module description:                                                    */
/*   This module provides routines to control the routing table.          */
/*   The routing table is used to determine which interface               */
/*   and IP address to send a packet out on.                              */
/*   drivers and all the routines which access/modify the routing table.  */

#define DIAG_SECTION_KERNEL DIAG_SECTION_IP

#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_RT     0
#define DEBUG_USECNT 0          /* debug info for routing table use count */

/* ********************************************************************   */
#if (INCLUDE_ROUTING_TABLE)
static RTIP_BOOLEAN rt_match(PFCBYTE dest_ip_addr, PROUTE prt);
#endif

/* ********************************************************************   */
/* ********************************************************************   */
/* ROUTING TABLE ROUTINES                                                 */
/* ********************************************************************   */

#if (INCLUDE_ROUTING_TABLE)
/* ********************************************************************   */
/* rt_get_output() - get interface and IP address to send pkt to          */
/*                                                                        */
/*   Gets interface and IP address from routing table for an IP address.  */
/*   If the entry is for a gateway, sets send_addr to the gateway         */
/*   address from the routing table.  If the entry is not for a gateway,  */
/*   sets send_addr to dest_ip_addr.                                      */
/*                                                                        */
/*   Returns interface to send to or -1 if no entry found.  Also sets     */
/*   send_addr to IP address to send to.                                  */

int rt_get_output(PFBYTE dest_ip_addr, PFBYTE send_addr, PANYPORT port, RTIP_BOOLEAN *is_gw)  /*__fn__*/
{
PROUTE prt;
int    iface_no;
int    device_off;
PIFACE pi;
    
    if (is_gw)
        *is_gw = FALSE;

    /* check if sending to itself; if so then send to loopback interface   */
    if ( ((pi=tc_get_local_pi(dest_ip_addr)) != (PIFACE)0) ||
         (dest_ip_addr[0] == IP_LOOP_ADDR) )
    {
        /* the device is not in loopback mode, so send to LOOPBACK device   */
        /* if it is open                                                    */
        device_off = tc_find_device_minor(LOOP_DEVICE, -1);
        pi = tc_ino2_device(device_off);
        if ( (device_off < 0) || !pi )
            return(-1);
        if (send_addr)
            tc_mv4(send_addr, dest_ip_addr, IP_ALEN);
        return(pi->ctrl.index);
    }
    prt = rt_get(dest_ip_addr);
    if (prt)
    {
        /* save info from the entry into send_addr                    */
        /* NOTE: do not need to claim semaphore here since usecnt     */
        /*       is incremented, therefore, entry cannot be deleted   */
        /*       or modified (except ttl) until usecnt is decremented */
        /*       by rt_free                                           */

        /* set address to send to, i.e. if gateway entry, get from     */
        /* gateway entry in table, if not gateway send to dest address */
        /* connected to                                                */
        if (send_addr)
        {
            if (prt->rt_flags & RT_GW)
                tc_mv4(send_addr, prt->rt_gw, IP_ALEN);     /* gateway */
            else
                tc_mv4(send_addr, dest_ip_addr, IP_ALEN);       /* local */
        }

        iface_no = prt->rt_iface;

        /* get interface before decrement usecnt   */
#        if (DEBUG_RT)
            DEBUG_LOG("rt_get_output() - interface to send to = ", LEVEL_3, 
                EBS_INT1, iface_no, 0);
#        endif

        if (is_gw && (prt->rt_flags & RT_GW))
        {
            *is_gw = TRUE;
#if (INCLUDE_BUILD_STATIC_ROUTES)
            if (tc_cmp4(prt->rt_dest, RT_DEFAULT, IP_ALEN))
            {
                rt_add(dest_ip_addr, ip_ffaddr, prt->rt_gw, prt->rt_metric, 
                       prt->rt_iface, prt->rt_ttl, prt->rt_tag, SNMP_LOCAL);
            }
#endif
        }

#if (DEBUG_USECNT)
        DEBUG_ERROR("rt_get_output: port = ", EBS_INT1, port, 0);
#endif

#if (INCLUDE_RT_LOCK)
        /* either save prt for freeing later or free it now   */
        if (port && (port->anyport_flags |= LOCK_RT))
        {
#if (DEBUG_USECNT)
            DEBUG_ERROR("rt_get_output: port->prt = ", DINT1, prt, 0);
#endif
            port->prt = prt;
        }
        else
            rt_free(prt);
#endif

        return(iface_no);
    }

    return(-1);
}


/* ********************************************************************   */
/* rt_get() - get entry in routing table for an IP address                */
/*                                                                        */
/*   Get and return a routing table entry which matches an IP address.    */
/*                                                                        */
/*   Returns the routing table entry which matches the IP address         */
/*                                                                        */

PROUTE rt_get(PFCBYTE dest_ip_addr)                      /*__fn__*/
{
int    i;
PROUTE prt;
PROUTE gw_prt;
PROUTE net_prt;
byte   net_masklen;
byte   gw_masklen;

    OS_CLAIM_TABLE(RT_GET_TABLE_CLAIM)

    gw_prt = 0;
    gw_masklen = 0;
    net_prt = 0;
    net_masklen = 0;
    prt = 0;    /* keep compiler happy */
    for (i=0; i < CFG_RTSIZE; i++)
    {
        prt = (PROUTE)(&(rt_table[i]));

        /* if entry is not valid, continue   */
#if (INCLUDE_RIP)
        if ( RT_FREE(prt) || (prt->rt_flags & RT_DEL) || 
             (prt->rt_metric == RTM_INF) )
#else
        if ( RT_FREE(prt) || (prt->rt_flags & RT_DEL) )
#endif
        {
            prt = (PROUTE)0;
            continue;       
        }

        /* see if the current entry is a match but the matches will   */
        /* be prioritized where hosts, then networks then gateways    */
        /* are the priorities                                         */
        if ( !tc_cmp4(prt->rt_dest, RT_DEFAULT, IP_ALEN) && /* if not default gw */
             rt_match(dest_ip_addr, prt) )
        {
            /* found a match;                             */
            /* if host entry (highest priority) then done */
            if ( tc_cmp4(prt->rt_mask, ip_ffaddr, IP_ALEN) &&
                 !(prt->rt_flags & RT_GW) )
            {
#                if (DEBUG_RT)
                    DEBUG_LOG("rt_get - found non-gw interface,metric = ", 
                        LEVEL_3, EBS_INT2, prt->rt_iface, prt->rt_metric);                  
#                endif
                break;
            }

            /* if network entry, save it in case do not find host entry   */
            else if (!(prt->rt_flags & RT_GW))  /* if not gateway */
            {
                if (prt->rt_masklen > net_masklen)
                {
                    net_prt = prt;
                    net_masklen = prt->rt_masklen;
                }
#                if (DEBUG_RT)
                    DEBUG_LOG("did not break-found network entry", LEVEL_3, NOVAR, 0, 0);
#                endif
            }

            /* gateway entry, save if in case do not find host or network   */
            /* entry                                                        */
            else
            {
                if (prt->rt_masklen > gw_masklen)
                {
                    gw_prt = prt;
                    gw_masklen = prt->rt_masklen;
                }
#                if (DEBUG_RT)
                    DEBUG_LOG("did not break-found gw entry", LEVEL_3, NOVAR, 0, 0);
#                endif
            }
        }
        prt = (PROUTE)0;
    }

    /* get network entry if did not find host entry   */
    if (!prt) 
    {
        /* get network entry if did not find host entry     */
        if (net_prt)
            prt = net_prt;

        /* get gatway if did not find host or network entry     */
        else if (gw_prt)
            prt = gw_prt;

        /* get default gateway if did not find one     */
        else
            prt = (PROUTE)root_rt_default;  /* may be null also. */
    }

    if (prt)
    {
#if (INCLUDE_RT_LOCK)
        prt->rt_usecnt++;
#        if (DEBUG_USECNT)
            DEBUG_ERROR("rt_get: use cnt incr:", EBS_INT2, prt->rt_usecnt, prt);
#        endif
#endif
#        if (DEBUG_RT)
            DEBUG_LOG("rt_get - found interface = ", LEVEL_3, EBS_INT1, prt->rt_iface, 0);
#        endif
    }
    else
    {
#        if (DEBUG_RT)
            DEBUG_ERROR("rt_get failed", NOVAR, 0, 0);
#        endif
    }

    OS_RELEASE_TABLE()

    return(prt);
}

/* ********************************************************************   */
/* rt_match() - determines if IP address matches a routing table entry    */
/*                                                                        */
/*   Determines if IP address when masked by the mask in the routing      */
/*   table entry matches the routing table entry.                         */
/*                                                                        */
/*   Returns TRUE if entry matches, FALSE if it does not                  */
/*                                                                        */

static RTIP_BOOLEAN rt_match(PFCBYTE dest_ip_addr, PROUTE prt)   /*__fn__*/
{
#if (LONG_W_ALLIGN)
int i;
#endif

#    if (DEBUG_RT)
        DEBUG_LOG("rt_match: dest_ip_addr = ", LEVEL_3, IPADDR, dest_ip_addr, 0);
        DEBUG_LOG("rt_match: prt->rt_dest = ", LEVEL_3, IPADDR, prt->rt_dest, 0);
        DEBUG_LOG("rt_match: prt->rt_mask = ", LEVEL_3, IPADDR, prt->rt_mask, 0);
        DEBUG_LOG("rt_match: prt->rt_iface = ", LEVEL_3, EBS_INT1, prt->rt_iface, 0);
#    endif

#if (LONG_W_ALLIGN)
    for (i=3; i>= 0; i--)
    {
        if ( (prt->rt_mask[i] & dest_ip_addr[i]) !=  
             (prt->rt_mask[i] & prt->rt_dest[i]) )
        {
#            if (DEBUG_RT)
                DEBUG_LOG("rt_match: it did not match", LEVEL_3, NOVAR, 0, 0);
#            endif
            return(FALSE);
        }
    }
#    if (DEBUG_RT)
        DEBUG_LOG("rt_match: it matched", LEVEL_3, NOVAR, 0, 0);
#    endif
    return(TRUE);
#else
    return( (RTIP_BOOLEAN)((*(dword*)prt->rt_mask & *(dword*)dest_ip_addr) ==
                      (*(dword*)prt->rt_mask & *(dword*)prt->rt_dest)) );
#endif
}
#endif      /* INCLUDE_ROUTING_TABLE */


/* ********************************************************************     */
/* rt_add() - adds an entry to routing table                                */
/*                                                                          */
/*   NOTE: if entry is not a gateway (gw = 0), gw will be set to the local  */
/*         IP address of the outgoing interface                             */
/*   NOTE: for default entry, net should be ip_nulladdr or RT_DEFAULT       */
/*                                                                          */
/*   Returns 0 if entry added, -1 if not added due to no entry available.   */
/*   Sets errno upon error.                                                 */
/*                                                                          */


int rt_add(PFCBYTE dest, PFCBYTE mask, PFCBYTE gw, dword metric,   /*__fn__*/
               int iface, int ttl, int tag, int snmp_proto)              /*__fn__*/
{
#if (INCLUDE_ROUTING_TABLE)
int     i,j;
byte    m,masklen;
PROUTE  prt, prt_free;
PIFACE  pi;
RTIP_BOOLEAN gw_match;

#if (!INCLUDE_SNMP)
    ARGSUSED_INT(snmp_proto)
#endif

    /* check if interface exists and is open   */
    if ((pi = tc_ino2_iface(iface, SET_ERRNO)) == 0)  /* will set errno */
        return(-1);

    /* calculate number of significant 1 bits in mask (used for best   */
    /* fit match)                                                      */
    masklen = 0;
    for (j=0; j<4; j++)
    {
        for (m=1; m!=0; m <<= 1)
        {
            if (m & mask[j])
                masklen++;
        }
    }

    OS_CLAIM_TABLE(RT_ADD_TABLE_CLAIM)

    /* check if entry already exists and find a free entry    */
    /* NOTE: an entry is free if ttl (time to live) is 0      */
    prt_free = (PROUTE)0;
    for (i=0; i < CFG_RTSIZE; i++)
    {
        prt = (PROUTE)(&(rt_table[i]));

        /* first compare gateway           */
        gw_match = TRUE;
        if ( (prt->rt_flags & RT_GW) && gw)
        {
            /* both are gateways   */
            if (!tc_cmp4(prt->rt_gw, gw, IP_ALEN) )
                gw_match = FALSE;
        }
        else if ((prt->rt_flags & RT_GW) || gw)
        {
            /* one is a gateway but the other isn't   */
            gw_match = FALSE;
        }

        /* if entry already exists     */
        if (!RT_FREE(prt))
        {
            if ( tc_cmp4((prt->rt_dest), dest, IP_ALEN) &&
                 (prt->rt_masklen == masklen) &&
                 gw_match )
            {
                prt->rt_flags &= ~RT_DEL;
                prt->rt_opencnt++;
                OS_RELEASE_TABLE()
                return(0);
            }
        }

        if (RT_FREE(prt) && !prt_free)
        {
            prt_free = prt;
        }
    }

    /* insert new info in entry   */
    if (prt_free)
    {
        DEBUG_LOG("rt_add - added entry for addr = ", LEVEL_3, 
            IPADDR, dest, 0);
        tc_mv4((prt_free->rt_dest),  dest,  IP_ALEN);
        tc_mv4((prt_free->rt_mask), mask, IP_ALEN);
        prt_free->rt_iface  = iface;
        metric = (metric == RT_USEIFACEMETRIC)? pi->addr.metric : metric;
        metric = (metric > RTM_INF)? RTM_INF : metric;
        prt_free->rt_metric = metric; 
        prt_free->rt_flags  = RT_CHANGED;
#if (INCLUDE_RT_TTL)
        prt_free->rt_ttl    = ttl;
#else
        ARGSUSED_INT(ttl);
        prt_free->rt_flags |= RT_INUSE;
#endif
        prt_free->rt_masklen = masklen;
        prt_free->rt_opencnt = 1;

        if (gw)
        {
            prt_free->rt_flags  |= RT_GW;
            tc_mv4((prt_free->rt_gw), gw, IP_ALEN);

            /* if default gateway, add to list of default gateways   */
            if (tc_cmp4(dest, (RT_DEFAULT), IP_ALEN))
            {
                root_rt_default = 
                    os_list_add_rear_off(root_rt_default,
                                        (POS_LIST)prt_free, ROUTE_OFFSET);
            }

        }
        else
        {
            tc_mv4((prt_free->rt_gw), pi->addr.my_ip_addr, IP_ALEN);
        }
        prt_free->rt_tag = tag;
#if (INCLUDE_RT_LOCK)
        prt_free->rt_usecnt = 0;
#if (DEBUG_USECNT)
        DEBUG_ERROR("rt_add: use cnt clear:", DINT2, 
            prt_free->rt_usecnt, prt_free);
#endif
#endif

#if (INCLUDE_SNMP)
        /* keep statistics for SNMP   */
        prt_free->snmp_proto = snmp_proto;
        prt_free->route_age = calc_sys_up_time();
#endif

        OS_RELEASE_TABLE()
#if (INCLUDE_UDP_IFACE_CACHE && INCLUDE_SYNCHRONIZE_UDP_IFACE_CACHE && INCLUDE_UDP)
        /* Since rotues may have changed clear all sockets that may be
           caching a route */
        clear_udp_iface_cache();
#endif

#if (INCLUDE_RIP && !INCLUDE_RIP_LISTEN_ONLY)
        _rip_trigger_update();
#endif
        return(0);
    }       /* end of if adding a new entry */

    OS_RELEASE_TABLE()
#if (INCLUDE_UDP_IFACE_CACHE && INCLUDE_SYNCHRONIZE_UDP_IFACE_CACHE && INCLUDE_UDP)
    /* Since rotues may have changed clear all sockets that may be
       caching a route */
    clear_udp_iface_cache();
#endif
    return(set_errno(ERTFULL));

#else
    ARGSUSED_INT(iface);
    ARGSUSED_INT(ttl);
    ARGSUSED_PVOID(dest);
    ARGSUSED_PVOID(mask);
    ARGSUSED_INT(tag);
    ARGSUSED_INT(snmp_proto);

    /* if there is a gateway, save the address   */
    if (gw) 
    {
        tc_mv4(default_gateway_address, gw, IP_ALEN);
    }
    return(0);
#endif
}

#if (INCLUDE_ROUTING_TABLE)
/* ********************************************************************   */
/* rt_add_from_iface() - adds an entry to routing table for the interface */
/*                                                                        */
/*   Adds entry to routing table for the interface.  If the interface     */
/*   is a ethernet interface, it adds a network entry (non-host).  If it  */
/*   is slip or loopback, it adds a host entry.                           */
/*                                                                        */
/*   NOTE: xn_set_ip must have been called prior to this routine          */
/*                                                                        */
/*   Returns 0 if entry added, -1 if not added due to no entry available. */
/*   Sets errno upon error.                                               */
/*                                                                        */
int rt_add_from_iface(PIFACE pi)       /*__fn__*/
{
byte net_addr[IP_ALEN];
int  i;
int  ret_val;

    if (!pi->pdev)
    {
        DEBUG_ERROR("rt_add_from_iface: pdev not set up", NOVAR, 0, 0);
        return(-1);
    }

    /* for ethernet interface and rs232 where remote address is unknown,    */
    /* set non-host (i.e. network) entry                                    */
    /*    net    = my_ip_addr & my_ip_mask                                  */
    /*    mask   = interface mask                                           */
    /*    gw     = 0                                                        */
    /*    metric = default for this iface                                   */
    /*    ttl    = no timeout                                               */
    if ( (pi->pdev->iface_type == ETHER_IFACE) ||
         ((pi->pdev->iface_type == RS232_IFACE) && 
           tc_cmp4(pi->addr.his_ip_addr, ip_nulladdr, IP_ALEN)) )
    {
        for (i=3; i>= 0; i--)
            net_addr[i] = (byte)(pi->addr.my_ip_mask[i] & pi->addr.my_ip_addr[i]);

        return(rt_add(net_addr, pi->addr.my_ip_mask, 0, 
                     pi->addr.metric, pi->ctrl.index, RT_INF, 0, SNMP_LOCAL));  /* INCLUDE_RIP */
    }

    /* for loopback and slip/ppp interfaces, set host entry              */
    /*    net    = his_ip_addr (CSLIP/SLIP/PPP), my_ip_addr (LOOP BACK)  */
    /*    mask   = all fs (CSLIP/SLIP), ff000000 (LOOP BACK)             */
    /*    gw     = 0                                                     */
    /*    metric = iface cost                                            */
    /*    ttl    = no timeout                                            */
    else if (pi->pdev->iface_type == LOOP_IFACE)
    {
        if (pi->addr.my_ip_addr[0] == 0x7f)
        {
            return(rt_add(pi->addr.my_ip_addr, (PFCBYTE)ip_lbmask, 0,
                          pi->addr.metric, pi->ctrl.index, RT_INF, 0, SNMP_LOCAL));  /* INCLUDE_RIP */
        }
        else
        {
            ret_val = rt_add(pi->addr.my_ip_addr, (PFCBYTE)ip_ffaddr, 0,
                             pi->addr.metric, pi->ctrl.index, RT_INF, 0, SNMP_LOCAL); /* INCLUDE_RIP */
            if (ret_val < 0)
                return(ret_val);
            net_addr[0] = 0x7f;
            net_addr[1] = 0x00;
            net_addr[2] = 0x00;
            net_addr[3] = 0x01;
            return(rt_add(net_addr, (PFCBYTE)ip_lbmask, 0, pi->addr.metric,
                          pi->ctrl.index, RT_INF, 0, SNMP_LOCAL)); /* INCLUDE_RIP */
        }
    }
    else if (pi->pdev->iface_type == RS232_IFACE) 
        return(rt_add(pi->addr.his_ip_addr, (PFCBYTE)ip_ffaddr, 0,
                     pi->addr.metric, pi->ctrl.index, RT_INF, 0, SNMP_LOCAL));  /* INCLUDE_RIP */

    else
    {
        DEBUG_ERROR("rt_add_from_iface() - unknown device", NOVAR, 0, 0);
        return(-1);     /* internal error */
    }
}

/* ********************************************************************        */
/* rt_del_from_iface() - deletes an entry from routing table for the interface */
/*                                                                             */
/*   Deletes an entry from routing table for the interface.  If the interface  */
/*   is a ethernet interface, it deletes a network entry (non-host).  If it    */
/*   is slip or loopback, it deletes a host entry.                             */
/*                                                                             */
/*   NOTE: xn_set_ip must have been called prior to this routine               */
/*                                                                             */
/*   Returns 0 if entry deleted, -1 if not deleted due to no entry available.  */
/*   Sets errno upon error.                                                    */
/*                                                                             */
int rt_del_from_iface(PIFACE pi)       /*__fn__*/
{
byte net_addr[IP_ALEN];
int  i;

    /* for ethernet interface and rs232 where remote address is unknown,    */
    /* delete non-host (i.e. network) entry                                 */
    /*    net    = my_ip_addr & my_ip_mask                                  */
    /*    mask   = interface mask                                           */
    if ( (pi->pdev->iface_type == ETHER_IFACE) ||
         ((pi->pdev->iface_type == RS232_IFACE) && 
           tc_cmp4(pi->addr.his_ip_addr, ip_nulladdr, IP_ALEN)) )
    {
        for (i=3; i>= 0; i--)
            net_addr[i] = (byte)(pi->addr.my_ip_mask[i] & pi->addr.my_ip_addr[i]);

        return(rt_del(net_addr, pi->addr.my_ip_mask));
    }

    /* for loopback and slip interfaces, delete host entry       */
    /*    net    = my_ip_addr                                    */
    /*    mask   = all fs (CSLIP/SLIP/PPP), ff000000 (LOOP BACK) */
    else if (pi->pdev->iface_type == LOOP_IFACE)
        return(rt_del(pi->addr.my_ip_addr, (PFBYTE)ip_lbmask));

    else if (pi->pdev->iface_type == RS232_IFACE)
        return(rt_del(pi->addr.his_ip_addr, (PFBYTE)ip_ffaddr));

    else
    {
        DEBUG_ERROR("rt_del_from_iface() - unknown device", NOVAR, 0, 0);
        return(-1);     /* internal error */
    }
}

/* ********************************************************************   */
/* rt_del() - delete an entry from routing table                          */
/*                                                                        */
/*   Deletes entry which has net for network address and mask for network */
/*   mask.  If net is all 0's (RT_DEFAULT) it refers to the default entry */
/*   but the mask still has to match.  An entry is deleted by setting its */
/*   time to live (rt_ttl) to 0.                                          */
/*                                                                        */
/*   Returns 0 if found an entry to delete or -1 if no entry was found.   */
/*                                                                        */
int rt_del(PFBYTE net, PFCBYTE mask)              /*__fn__*/
{
int    i;
PROUTE prt;

    OS_CLAIM_TABLE(RT_DEL_TABLE_CLAIM)

    for (i=0; i < CFG_RTSIZE; i++)
    {
        prt = (PROUTE)(&(rt_table[i]));
        
        /* both net and mask must match    */
        if ( (tc_cmp4(net, prt->rt_dest, IP_ALEN)) &&
             (tc_cmp4(mask, prt->rt_mask, IP_ALEN)) ) 
        {
            /* make sure the entry has not already been deleted   */
            if ( !(prt->rt_flags & RT_DEL) && !RT_FREE(prt) )
            {
                if (prt->rt_opencnt > 1)
                {
                    prt->rt_opencnt--;
                    OS_RELEASE_TABLE()
                    return(0);
                }

                /* mark entry invalid but not for the case where          */
                /* it is in use; if it is in use mark it needs to be      */
                /* deleted when not in use and no more accesses to it are */
                /* allowed                                                */
#if (INCLUDE_RT_LOCK)
                if (prt->rt_usecnt)
                    prt->rt_flags |= RT_DEL;  
                else
                {
                    RT_SET_FREE(prt);

                    /* if this is the default entry, delete it from list   */
                    if (tc_cmp4(net, (RT_DEFAULT), IP_ALEN))
                    {
                        root_rt_default =
                            os_list_remove_off(root_rt_default, (POS_LIST)prt, 
                                               ROUTE_OFFSET);
                    }
                }
#else
                RT_SET_FREE(prt);

                /* if this is the default entry, delete it from list   */
                if (tc_cmp4(net, (RT_DEFAULT), IP_ALEN))
                {
                    root_rt_default =
                        os_list_remove_off(root_rt_default, (POS_LIST)prt, 
                                       ROUTE_OFFSET);
                }
#endif
                OS_RELEASE_TABLE()
#if (INCLUDE_UDP_IFACE_CACHE && INCLUDE_SYNCHRONIZE_UDP_IFACE_CACHE && INCLUDE_UDP)
                /* Since rotues may have changed clear all sockets that may be
                   caching a route */
                clear_udp_iface_cache();
#endif
                return(0);
            }
        }
    }

    OS_RELEASE_TABLE()
#if (INCLUDE_UDP_IFACE_CACHE && INCLUDE_SYNCHRONIZE_UDP_IFACE_CACHE && INCLUDE_UDP)
    /* Since rotues may have changed clear all sockets that may be
       caching a route */
    clear_udp_iface_cache();
#endif
    return(-1);
}

/* ********************************************************************       */
/* rt_del_iface() - delete entries from routing table for an interface        */
/*                                                                            */
/*   Deletes all entries for an interface.                                    */
/*   If net is all 0's (RT_DEFAULT) it refers to the default entry            */
/*   but the mask still has to match.  An entry is deleted by setting its     */
/*   time to live (rt_ttl) to 0.                                              */
/*                                                                            */
/*   Returns TRUE if found an entry to delete or FALSE if no entry was found. */
/*                                                                            */
void rt_del_iface(int iface_no)              /*__fn__*/
{
int    i;
PROUTE prt;

    OS_CLAIM_TABLE(RT_DIFACE_TABLE_CLAIM)

    for (i=0; i < CFG_RTSIZE; i++)
    {
        prt = (PROUTE)(&(rt_table[i]));

        if (!RT_FREE(prt) && (prt->rt_iface == iface_no))
        {
            if (prt->rt_opencnt > 1)
            {
                prt->rt_opencnt--;
                OS_RELEASE_TABLE()
                return;
            }

            /* mark entry invalid but not for the case where          */
            /* it is in use; if it is in use mark it needs to be      */
            /* deleted when not in use and no more accesses to it are */
            /* allowed                                                */
#if (INCLUDE_RT_LOCK)
            if (prt->rt_usecnt)
                prt->rt_flags |= RT_DEL;  
            else
            {
                RT_SET_FREE(prt);

                /* if this is the default gateway entry, delete it from list   */
                if (tc_cmp4(prt->rt_dest, (RT_DEFAULT), IP_ALEN))
                {
                    root_rt_default =
                        os_list_remove_off(root_rt_default, (POS_LIST)prt, 
                                           ROUTE_OFFSET);
                }
            }
#else
            RT_SET_FREE(prt);

            if (tc_cmp4(prt->rt_dest, (RT_DEFAULT), IP_ALEN))
            {
                root_rt_default =
                    os_list_remove_off(root_rt_default, (POS_LIST)prt, 
                                       ROUTE_OFFSET);
            }
#endif
        }
    }
    OS_RELEASE_TABLE()
}


/* ********************************************************************   */
/* rt_redirect() - redirect a gateway entry                               */
/*                                                                        */
/*   Modifies a gateway entry to send to a different gateway as a         */
/*   result of an ICMP REDIRECT message                                   */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void rt_redirect(PIFACE pi, PFBYTE src_ip_addr, PFBYTE orig_dest_ip_addr, PFBYTE new_gw_addr)  /*__fn__*/
{
int i;
PROUTE prt;
RTIP_BOOLEAN error;

    error = FALSE;
    prt = rt_get(orig_dest_ip_addr);
    if (prt)
    {
        /* silently discard ICMP msg if source of the redirect is not   */
        /* the first-hop gateway for the specified destination          */
        if ( !(prt->rt_flags & RT_GW) ||    /* if not gateway */
             (!tc_cmp4(src_ip_addr, prt->rt_gw, IP_ALEN)) )  
            error = TRUE;

        /* silently discard ICMP msg if the new gateway address is   */
        /* not on the same net through which the redirect arrived    */
        for (i=3; i>= 0; i--)
        {
            if ( (pi->addr.my_ip_mask[i] & new_gw_addr[i]) !=  
                 (pi->addr.my_ip_mask[i] & pi->addr.my_ip_addr[i]) )
                error = TRUE;
        }

        if (!error)
        {
            /* PASSED ERROR TESTS       */
            /* if host entry, update it */
            if (tc_cmp4(prt->rt_mask, ip_ffaddr, IP_ALEN))
            {
                OS_CLAIM_TABLE(RT_REDIRECT_TABLE_CLAIM)
                tc_mv4((prt->rt_gw), new_gw_addr, IP_ALEN);
                OS_RELEASE_TABLE()
#if (INCLUDE_RT_LOCK)
                rt_free(prt);
#endif
                return;
            }
#if (INCLUDE_RT_LOCK)
            rt_free(prt);
#endif
        }
    }

    /* if here a host entry was not found, if an error was not detected   */
    /* then add a host entry to the routing table                         */
    if (!error)
        rt_add(orig_dest_ip_addr, (PFBYTE)ip_ffaddr, new_gw_addr,
               pi->addr.metric, pi->ctrl.index, RT_INF, 0, SNMP_ICMP); 
}

#if (INCLUDE_RT_LOCK)
/* ********************************************************************    */
/* rt_free() - decrements use count for route                              */
/*                                                                         */
/*   When entry no longer being used, rt_free() is called to decrement the */
/*   use count.  If an attempt was previously made to delete the entry and */
/*   the use count is now 0, the entry is set to invalid.                  */
/*                                                                         */
/*   Returns nothing.                                                      */
/*                                                                         */

void rt_free(PROUTE prt)               /*__fn__*/
{
    OS_CLAIM_TABLE(RT_FREE_TABLE_CLAIM)

    if (prt->rt_usecnt > 0)
        prt->rt_usecnt--;

#if (DEBUG_USECNT)
    DEBUG_ERROR("rt_free: use cnt decr:", DINT2, prt->rt_usecnt, prt);
#endif

    /* if an attempt was made to delete entry but someone was accessing   */
    /* entry and now noone is accessing this entry then delete it         */
    if ( (prt->rt_flags & RT_DEL) && (prt->rt_usecnt == 0) )
    {
        RT_SET_FREE(prt);

        /* if this is the default entry, remove it   */
        if (tc_cmp4(prt->rt_dest, (RT_DEFAULT), IP_ALEN))
        {
            root_rt_default =
                os_list_remove_off(root_rt_default, (POS_LIST)prt, 
                                   ROUTE_OFFSET);
        }
    }

    OS_RELEASE_TABLE()
}
#endif

#if (INCLUDE_RT_TTL)
/* ********************************************************************   */
/* rt_timer() - routing table time to live maintenance routine            */
/*                                                                        */
/*   Decrements time to live entries for all valid routing table entries  */
/*   until the time to live is infinite.  This routine is called every    */
/*   second.                                                              */
/*                                                                        */
/*   Returns nothing                                                      */

void rt_timer(void)                             /*__fn__*/
{
int    i;
PROUTE prt;

    OS_CLAIM_TABLE(RT_TIMER_TABLE_CLAIM)

    for (i=0; i < CFG_RTSIZE; i++)
    {
        prt = (PROUTE)(&(rt_table[i]));

        if ( !RT_FREE(prt) && (prt->rt_ttl != RT_INF) )
        {
            /* decrement time to live but not for the case where   */
            /* it is in use and the timer is about to expire       */
#if (INCLUDE_RT_LOCK)
            if ( !(prt->rt_usecnt) || (prt->rt_ttl > 1) )
#else
            if (prt->rt_ttl > 0) 
#endif
            {
                prt->rt_ttl--;  /* NOTE: if entry is 0 it is invalid */

#if (INCLUDE_RIP)
                /* This gives RIP a chance to tell neighboring routers that this   */
                /*  route has timed out                                            */
                if ( (prt->rt_ttl == 0) && (prt->rt_metric < RTM_INF) )
                {
                    prt->rt_ttl = RIP_GARBAGE_TTL;
                    prt->rt_metric = RTM_INF;
#if (!INCLUDE_RIP_LISTEN_ONLY)
                    prt->rt_flags |= RT_CHANGED;
                    _rip_trigger_update();
#endif
                }
#endif

                /* if this is the default entry, delete reference to it   */
                if ( (prt->rt_ttl == 0) && 
                     (tc_cmp4(prt->rt_dest, (RT_DEFAULT), IP_ALEN)) )
                {
                    root_rt_default = 
                        os_list_add_rear_off(root_rt_default,
                                             (POS_LIST)prt, ROUTE_OFFSET);
                }
            }
#if (INCLUDE_RT_LOCK)
            else
                prt->rt_flags |= RT_DEL;
#endif
        }
    }       

    OS_RELEASE_TABLE()
}
#endif  /* INCLUDE_RT_TTL */

#endif  /* INCLUDE_ROUTING_TABLE */

/* ********************************************************************     */
/* rt_init() - initializes routing table                                    */
/*                                                                          */
/*   Initializes routing table to all entries invalid.  Entries are invalid */
/*   if the time to live is 0.                                              */
/*                                                                          */
/*   Return nothing                                                         */
/*                                                                          */

void rt_init(void)                         /*__fn__*/
{
#if (INCLUDE_ROUTING_TABLE)
int   i;

    for (i=0; i < CFG_RTSIZE; i++)
        RT_SET_FREE_INDEX(i);
    root_rt_default = (POS_LIST)0; 
#else
    tc_mv4(default_gateway_address, ip_nulladdr, IP_ALEN);
#endif
}

/* ********************************************************************      */
/* rt_get_iface_lock() - return interface based on IP addr and routing table */
/*                                                                           */
/*   Performs rt_get_iface (which gets iface structure and address to        */
/*   send to) as well as locks the routing table.                            */
/*                                                                           */
/*   Returns interface structure associated with ip address and sets         */
/*   send_addr to the address the packet should be sent to.                  */
/*                                                                           */
PIFACE rt_get_iface_lock(PFBYTE ip_addr, PFBYTE send_addr, PANYPORT port, 
                         RTIP_BOOLEAN *is_gw)    /*__fn__*/
{
PIFACE pi;

    /* tbd - check if flags are protected   */
    port->anyport_flags |= LOCK_RT;
    pi = rt_get_iface(ip_addr, send_addr, port, is_gw);
    port->anyport_flags &= ~LOCK_RT;

    return(pi);
}

/* ********************************************************************     */
/* rt_get_iface() - return interface based on IP addr and routing table     */
/*                                                                          */
/*   Returns interface structure associated with ip address.  If none       */
/*   founds returns the default gateway interface which currently is        */
/*   packet interface. If no default gateway found, returns 0.              */
/*                                                                          */
/*   If send_addr is not 0, the address to send to is returned in send_addr */
/*   (i.e. either gateway address from routing table or ip_addr passed      */
/*   to this routine.                                                       */
/*   If is_gw is not 0, this flag is set to TRUE if the matching routine    */
/*   table entry is a gateway.                                              */
/*                                                                          */
/*   NOTE: port can be 0                                                    */
/*                                                                          */
/*   Returns interface structure associated with ip address and sets        */
/*   send_addr to the address the packet should be sent to.                 */
/*                                                                          */

PIFACE rt_get_iface(PFBYTE ip_addr, PFBYTE send_addr, PANYPORT port, RTIP_BOOLEAN *is_gw)    /*__fn__*/
{
PIFACE pi;
#if (!INCLUDE_ROUTING_TABLE)
int i;
#else
int iface;
#endif

    /* check for broadcast; if not broadcast, search routing table      */
    /* NOTE: netwrite will still send to all interfaces except loopback */
    /*       still send to bc address and netwrite will send to all     */
    /*       interfaces for broadcast                                   */
    if ( (ip_addr[0] == ip_broadaddr[0]) &&           /* 0xff */
          tc_cmp4(ip_addr, ip_broadaddr, IP_ALEN) )
    {
        if (send_addr)
            tc_mv4(send_addr, ip_addr, IP_ALEN); 
        if ( port && ((PUDPPORT)port)->broadcast_pi && 
             (port->port_type == UDPPORTTYPE) )
            return(((PUDPPORT)port)->broadcast_pi);

        return(tc_get_nxt_bc_iface(0));
    }

        
#if (INCLUDE_ROUTING_TABLE)
    /* **************************************************   */
    iface = rt_get_output(ip_addr, send_addr, port, is_gw);

    DEBUG_LOG("rt_get_iface - interface = ", LEVEL_3, EBS_INT1, iface, 0);
    DEBUG_LOG("rt_get_iface - ip addr = ", LEVEL_3, IPADDR, ip_addr, 0);
    PI_FROM_OFF(pi, iface)
    return(pi);
#else       /* INCLUDE_ROUTING_TABLE */

    ARGSUSED_PVOID(port);

    if (is_gw)
        *is_gw = FALSE;

    /* return interface 0; if there is not routing table there   */
    /* should be only one interface                              */
    pi = (PIFACE) &ifaces[0];
    if ( !(pi->pdev) || !(pi->open_count) || 
         !(pi->addr.valid_flags & IP_ADDR_VALID) )
        return((PIFACE)0);

    /* if RS232 doesn't matter since used by ARP   */
    if (send_addr)
        tc_mv4(send_addr, ip_addr, IP_ALEN); 
    if (pi->pdev->iface_type != RS232_IFACE)
    {
        for (i=3; i>= 0; i--)
        {
            if ( (pi->addr.my_ip_mask[i] & ip_addr[i]) !=  
                 (pi->addr.my_ip_mask[i] & pi->addr.my_ip_addr[i]) )
            {
                if (tc_cmp4(default_gateway_address, ip_nulladdr, IP_ALEN))
                    pi = (PIFACE)0;
                else
                {
                    if (send_addr)
                    {
                        tc_mv4(send_addr, default_gateway_address, IP_ALEN);
                        if (is_gw)
                            *is_gw = TRUE;
                    }
                }
                break;
            }
        }
    }
    return(pi);

#endif      /* INCLUDE_ROUTING_TABLE */
}
