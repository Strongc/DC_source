/*                                                                               */
/* ARP.C - Address Resolution Protocol                                           */
/*                                                                               */
/* EBS - RTIP                                                                    */
/*                                                                               */
/* Copyright Peter Van Oudenaren , 1993                                          */
/* All rights reserved.                                                          */
/* This code may not be redistributed in source or linkable object form          */
/* without the consent of its author.                                            */
/*                                                                               */
/*  Module description:                                                          */
/*  This module provides the address resolution protocol. Packets addressed to   */
/*  an IP address are sent to this module via tc_netwrite(). It looks up the     */
/*  IP address up in the interface's local arp cache and resolves the ethernet   */
/*  address and queues the packet for transmition. If the address is not in      */
/*  the cache an address resolution request is broadcast and the packet is       */
/*  queued for writing when the request is resolved.                             */
/*  This module also contains code to reply to ARP requests for this IP address. */
/*  If INCLUDE_RARP (rtip.h) is non-zero the functionality needed to implement   */
/*  xn_rarp() (rtipapi.c) is compiled in.                                        */
/*                                                                               */
/*  if INCLUDE_RARP_SRV (xnconf.h) is non-zero a primitive RARP server is        */
/*  compiled in. The rarp server is hard wired to certain ethernet addresses.    */
/*  The IP to ethernet table must be loaded for your local network or you must   */
/*  write  a module to load it from disk.                                        */
/*                                                                               */

#define DIAG_SECTION_KERNEL DIAG_SECTION_ARP


/*  Functions in this module                                                                         */
/* tc_arp_remote     - Arp a remote host                                                             */
/* tc_arp_interpret  - Process an incoming arp packet                                                */
/* tc_arp_timeout    - called periodically (1/sec) mark entries stale as                             */
/*                     appropriate, retry unanswered requests or time them out.                      */
/* tc_arp_cachelook  - (static) Search the arp cache for an IP to ENET map.                          */
/* tc_arp_cachealloc - (static) Grab an unused or recycle an in-use cache entry                      */
/* tc_arp_just_update_cache - Update existing cache entry                                            */
/* tc_arp_add_entry_to_cache - (static) Add an entry to cache when I am the target for an ARP packet */
/*                     they are put in the ether output queue.                                       */
/* tc_arp_qsend      - (static) the packet send routine for                */
/*                     if the resolved address is a host                   */
/* tc_arp_qpurge     - (static) A host arp request timed out. Purge all    */
/*                     output packets waiting for the request to complete. */
/* tc_do_arp_send    - (static) Broadcast an arp request                   */
/* tc_rarp_send      - Send a RARP request (see xn_rarp())                 */
/* tc_rarp_reply     - Reply to a rarp request. (unsupported)              */
/* tc_rarp_lookup    - Map an ethernet to an IP address for rarp server    */
/*                     (unsupported)                                       */

#include "rtip.h"
#include "sock.h"
#include "rtipext.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_ARP 0
#define DISPLAY_ARP_RETRIES 1   /* display messages thru DEBUG_ERROR */

#if (INCLUDE_IPV4)

/* ********************************************************************   */
/* Protoypes of private functions for arp                                 */
#if (INCLUDE_ARP)
static int tc_arp_cachelook(PFBYTE ipn);
static int tc_arp_cachealloc(void);
static void tc_arp_just_update_cache(PFBYTE packet, PFBYTE ipn, PFBYTE hrdn,
                              PIFACE pi, int entry_num,
                              dword timeout);
static int tc_arp_add_entry_to_cache(PFBYTE packet, PFBYTE ipn, PFBYTE hrdn, 
                                     PIFACE pi);
static void tc_arp_qsend(word index);
static void tc_arp_qpurge(word index);
#endif
#if (INCLUDE_RARP_SRV)
void tc_rarp_reply(PIFACE pi, DCU msg, PFBYTE ip_address);
PFBYTE tc_rarp_lookup(PFBYTE ether_address);
#endif

#if (INCLUDE_ARP)
/* ********************************************************************   */
/* return 0 upon success, ERRNO upon error                                */
int tc_arp_remote(PIFACE pi, PANYPORT port, DCU msg, PFBYTE ipn, int flags, word wait_count) /*__fn__*/
{
int     found;
int     ret_val;
int     on_list_no;
RTIP_BOOLEAN wait_for_arp;
RTIP_BOOLEAN on_arp_list;

#if (DEBUG_ARP)
    DEBUG_ERROR("tc_arp_remote entered ", IPADDR, ipn, 0);
#endif
    wait_for_arp = FALSE;           /* If true block & wait for resolution  */
    on_list_no = 0;

    /* For recovering if a send times out             */
    /* What list should we remove the message from    */
    on_arp_list = FALSE;

    ret_val = 0;

    /* **************************************************   */
#if (INCLUDE_SOCK_ARP_CACHE)
    /* If cacheing see if the last entry we used in the arp table
       for this socket matches the IPN we are sending. If so
       check to be sure the IPN matches the arp table entry and
       if so send to that ethernet address and go to post processing */
#if (INCLUDE_MEASURE_PERFORMANCE)
if (do_sock_arp_cache)
{
#endif
    OS_CLAIM_TABLE(ARP_SOCK_ARP_TABLE_CLAIM)
    if (port && port->port_cache_flags & SOCK_ARP_CACHE_VALID)
    {
        found = port->cached_arp_index;
        if (tc_arpcache[found].arpc_state == ARPC_STATE_RESOLVED
            && tc_cmp4(ipn, tc_arpcache[found].arpc_ip_addr, 4))
        {
#if (DEBUG_ARP)
            DEBUG_ERROR("tc_arp_remote: entry found, so send on interface ",
                pi->ctrl.index, 0);
#endif
            /* They match so send the packet                                        */
            /* ******************************************************************** */
            /* Only send if interface is open                                       */
            if (pi->pdev)
            {
                /* copy ethernet addr from cache to output msg   */
                SETUP_LL_HDR_DEST(pi, msg, 
                                  tc_arpcache[found].arpc_hw_addr)
                /* Send the packet, i.e. put it on output exchange   */
                DEBUG_LOG("tc_arp_remote - cache hit call tc_interface_send", LEVEL_3, NOVAR, 0, 0);
                ret_val = tc_do_interface_send(pi, msg);
                OS_RELEASE_TABLE()

                wait_for_arp = FALSE;       /* Don't wait for resolution  */
                /* We've sent the packet now wait for completion if required   */
                goto post_interface_send;   /* Go to normal post processing */
            }
        }
        /* Cache doesn't match or device is closed so invalidate   */
        port->port_cache_flags &= ~SOCK_ARP_CACHE_VALID;
    }       
    OS_RELEASE_TABLE()
#if (INCLUDE_MEASURE_PERFORMANCE)
}
#endif
    /* Otherwise we fall through to normal processing   */
#endif

    /* ********************************************************************   */
    /* proceed to the arp layer                                               */
    /* ********************************************************************   */
    /* We'll be searching the cache. Don't change it on us                    */
    OS_CLAIM_TABLE(ARP_SEARCH_TABLE_CLAIM)

    /* ********************************************************************   */
    /* See if the address is already resolved, i.e IP->ethernet address       */
    /* is already in the cache                                                */
    found = tc_arp_cachelook(ipn);
    if (found >= 0)
    {
        /* found arp entry, queue it if pending   */
        if (tc_arpcache[found].arpc_state == ARPC_STATE_PENDING)
        {
#if (DEBUG_ARP)
            DEBUG_ERROR("tc_arp_remote: PENDING, add to cache entry", NOVAR,
                0, 0);
#endif
            DEBUG_LOG("tc_arp_remote - pending:send to arpcache exchg", LEVEL_3, NOVAR, 0, 0);
            /* save interface for when address resolved an put msg arp cache   */
            DCUTOCONTROL(msg).pi = pi;  
            os_sndx_arpcache_list((word)found, msg);

            OS_RELEASE_TABLE()

            wait_for_arp = TRUE;
            on_arp_list = TRUE;
            on_list_no = found;
        }

        /* found arp entry and not pending, therefore, send pkt   */
        else
        {
#if (DEBUG_ARP)
            DEBUG_ERROR("tc_arp_remote: Found, resolved", NOVAR, 0, 0);
#endif
            DEBUG_LOG("tc_arp_remote - not pending:send pkt", LEVEL_3, NOVAR, 0, 0);

            /* copy link layert addr from cache to output msg   */
            SETUP_LL_HDR_DEST(pi, msg, 
                              tc_arpcache[found].arpc_hw_addr)
#if (INCLUDE_SOCK_ARP_CACHE)
            /* If cacheing and we got a hit in the arp cache then
               save away the index so we can bypass all of this 
               next time */
            if (port)
            {
                port->port_cache_flags |= SOCK_ARP_CACHE_VALID;
                port->cached_arp_index = found;
            }
#endif
            /* Send the packet, i.e. put it on output exchange   */
            DEBUG_LOG("tc_arp_remote - not pending call tc_interface_send", LEVEL_3, NOVAR, 0, 0);
            OS_RELEASE_TABLE()

            INCR_INFO(pi, xmit_ucast)
            ret_val = tc_do_interface_send(pi, msg);
        }
    }

    /* if no entry in cache   */
    else
    {
        found = tc_arp_cachealloc();
        if (found >= 0)
        {
            /* Set up the cache table entry               */
            /* ip address, state, timeout and retry count */
            tc_movebytes((PFBYTE)tc_arpcache[found].arpc_ip_addr,(PFBYTE)ipn, 
                         IP_ALEN);
            tc_arpcache[found].arpc_state = ARPC_STATE_PENDING;
            DEBUG_LOG("netwrite - set state, index", LEVEL_3, EBS_INT2, 
                     ARPC_STATE_PENDING, found);
                                        
            /* double the retry timer on the first send so the timer doesn't   */
            /* retry it too quickly                                            */
            tc_arpcache[found].arpc_ttl = ARPC_REQ_TIMEOUT*2;
            tc_arpcache[found].arpc_nretries = ARPC_MAX_RETRIES;
            tc_arpcache[found].pi = pi;  /* save for retries */

            DEBUG_LOG("Arp send", LEVEL_3, NOVAR, 0, 0);

            /* save interface for when address resolved and queue the    */
            /* message in arp cache to be sent later when                */
            /* address is resolved                                       */
            DCUTOCONTROL(msg).pi = pi;
            os_sndx_arpcache_list((word)found, msg);

            OS_RELEASE_TABLE()

            on_arp_list = TRUE;
            wait_for_arp = TRUE;
            on_list_no = found;
            DEBUG_LOG("netwrite - sent to arpcache, index =", LEVEL_3, EBS_INT1, found, 0);

            tc_do_arp_send(pi, ipn, (PFBYTE)broadaddr, ARPREQ_68K);
        }

        /*****************************************************************   */
        /* if arp cache alloc failed, set errno value                        */
        else
        {
            OS_RELEASE_TABLE()

            FREE_DCU(msg, flags)
            ret_val = EARPFULL;
        }
    }

#if (INCLUDE_SOCK_ARP_CACHE)
post_interface_send:
#endif

    /* ********************************************************************   */
    if (flags & PKT_FLAG_SIGNAL)
    {
        /* Wait for the packet to be send.   */
        DEBUG_LOG("netwrite - block waiting for sent signal", LEVEL_3, NOVAR, 0, 0);
        if (OS_TEST_SENT_SIGNAL(port, wait_count))
        {
            ret_val = port->ctrl.sent_status;   /* Were awake  */
                                                /* use send status   */
        }
        else
        {
            if (wait_for_arp)
            {
                /* Wait Timed out. remove the packet from the arp cache   */
                /* if its still there.                                    */
                OS_CLAIM_TABLE(ARP_REMOVE_TABLE_CLAIM)

                /* make sure the signal is cleared. - If a signal was sent
                    between the time we timed out and when we claimed the
                    arp cache this will clear it. */
                OS_CLEAR_SENT_SIGNAL(port);

                /* Remove the packet from the arp cache.
                    if it is there */
                if (on_arp_list)
                    os_rmvx_arpcache_list((word)on_list_no, msg);
                OS_RELEASE_TABLE()
            }
            ret_val = ETIMEDOUT;
        }
    }
    return(ret_val);
}
#endif /* INCLUDE_ARP */

/* ********************************************************************   */
/* ARP INTERPRET                                                          */
/* ********************************************************************   */

/* ********************************************************************        */
/*  Interpret an incoming ARP packet. Called from the interface input routine. */
/*                                                                             */
void tc_arp_interpret(PIFACE pi, DCU msg)                      /*__fn__*/
{
PARPPKT pa;
#if (INCLUDE_ARP)
int found;
#endif
#if (INCLUDE_ARP || INCLUDE_ETH_BUT_NOARP)
RTIP_BOOLEAN entry_updated;
#endif
#if (INCLUDE_ROUTER && INCLUDE_PROXY_ARP && INCLUDE_ARP)
PIFACE proxy_pi;
PIFACE pi_out;
#endif
    /* Look at the packet       */
    pa = DCUTOARPPKT(msg);
    DEBUG_LOG("tc_arp_interpret - Arp In, iface = ", LEVEL_3, EBS_INT1,
        pi->ctrl.index, 0);

    DEBUG_LOG("  pt ip_eth_dest", LEVEL_3, IPADDR, (PFBYTE)(pa->ar_tpa), 0);
    DEBUG_LOG("  pt ip_src", LEVEL_3, IPADDR, (PFBYTE)(pa->ar_spa), 0);

    /* ********************************************************************   */
    /* ARP REQUEST OR RESPONSE                                                */
    /* ********************************************************************   */
#if (INCLUDE_ARP || INCLUDE_ETH_BUT_NOARP)
    if ( ((pa->ar_opcode == ARPREQ_68K) || (pa->ar_opcode == ARPREP_68K)) &&
/* tbd -tok hw type is 6                                       */
/*        (pa->ar_hw_type == HTYPE_68K) &&   // check h/w type */
          (pa->ar_hw_len == ETH_ALEN) &&     /* optional check for # of bytes in ethernet address */
          (pa->ar_pr_type == ARPPRO_68K) &&  /* check for protocol type */
          (pa->ar_pr_len == 4) )             /* optional check for # of bytes in an IP address */
    {
        entry_updated = FALSE;  /* set to true if existing entry in cache  */
                                /* is superseded   */
#if (INCLUDE_ARP)
        /* see if entry for sender's IP address already exists   */
        found = tc_arp_cachelook(pa->ar_spa);
        if (found >= 0 ) 
        {
#if (DEBUG_ARP)
            DEBUG_ERROR("tc_arp_interpret: Arp Entry Before", ARPC_ENTRY, found,0);
#endif
            OS_CLAIM_TABLE(ARP_INTERPRET_TABLE_CLAIM)
            /* entry for sender already exitst, update entry to RESOLVED   */
            tc_arp_just_update_cache(DCUTODATA(msg), (PFBYTE)pa->ar_spa, 
                                     (PFBYTE)pa->ar_sha, 
                                     pi, found, arpc_res_tmeout);  
#if (DEBUG_ARP)
            DEBUG_ERROR("tc_arp_interpret: Arp Entry Update", ARPC_ENTRY, found,0);
#endif
            OS_RELEASE_TABLE() 
            entry_updated = TRUE;
        }
#endif /* INCLUDE_ARP */

        /* ********************************************************************   */
        /* ARP REQUEST OR RESPONSE                                                */
        /* check if IP addresses is destined for us or we are a proxy arp         */
        /* ********************************************************************   */
#if (INCLUDE_ROUTER && INCLUDE_PROXY_ARP)
        proxy_pi = rt_get_iface(pa->ar_tpa, 0, 0, 0);

        pi_out = tc_get_local_pi((PFBYTE)pa->ar_tpa);
        if (pi_out ||
            (proxy_pi != 0 && pi != proxy_pi) )
        {
            if (pi_out)
                pi = pi_out;
#else
        if ((pi=tc_get_local_pi((PFBYTE)pa->ar_tpa)) != 0)
        { 
#endif
#if (INCLUDE_ARP)
            if (entry_updated == FALSE) 
            {
                OS_CLAIM_TABLE(ARP_ADDIN_TABLE_CLAIM)
                /* since I am the target add an entry   */
                found = tc_arp_add_entry_to_cache(DCUTODATA(msg),
                                                  (PFBYTE)pa->ar_spa, 
                                                  (PFBYTE)pa->ar_sha, pi); 
#if (DEBUG_ARP)
                DEBUG_ERROR("tc_arp_interpret: Arp Entry Add", ARPC_ENTRY, found,0);
#endif
                OS_RELEASE_TABLE()                       
            }
#endif  /* INCLUDE_ARP */

            /* ********************************************************************   */
            /* ARP REQUEST                                                            */
            /* ********************************************************************   */
            /* if this is a request I must reply                                      */
            if (pa->ar_opcode == ARPREQ_68K)  
            {
                tc_do_arp_send(pi, pa->ar_spa, LL_SRC_ADDR(msg, pi),
                                ARPREP_68K);
            }
#if (INCLUDE_ARP)
            /* ********************************************************************   */
            /* ARP REQUEST OR REPLY                                                   */
            /* ********************************************************************   */
            /* if resolved a cache entry, send any packets enqueued on this           */
            /* request                                                                */
            if (found >= 0) 
            {
                OS_CLAIM_TABLE(ARP_REPLY_TABLE_CLAIM)
                tc_arp_qsend((word)found);  
                OS_RELEASE_TABLE()          
            }                               
#endif  /* INCLUDE_ARP */
        }
    }
#endif          /* INCLUDE_ARP */

/* ********************************************************************   */
#if (INCLUDE_RARP)
    /* check for RARP reply   */
#if (INCLUDE_ARP)
    else 
#endif
    if ( pa->ar_opcode==RARPR_68K &&
         tc_comparen((PFBYTE)pa->ar_tha,(PFBYTE)pi->addr.my_hw_addr,ETH_ALEN) )
    {
        /* If setting IP address successful, signal xn_rarp that we got a    */
        /* reply; otherwise, let it timeout                                  */
        /* NOTE: set_ip could set error but thats ok since initiated         */
        /*       by an api call anyway                                       */
        if (!set_ip(pi->ctrl.index, (PFBYTE)pa->ar_tpa, (PFCBYTE)0))
        {
            OS_SET_RARP_SIGNAL(pi);
        }
    }
#endif

#if (INCLUDE_RARP_SRV)
    /* In the unlikely event that you need a rarp server, here it is.   */
    else if (pa->ar_opcode==RARPQ_68K)
    {
    PFBYTE p;
        /* Look up the senders hardware address in our data base. If found   */
        /* we have is IP address.                                            */
        p = tc_rarp_lookup((PFBYTE)pa->ar_sha);
        if (p)
        {
            /* Send his ip address back to him.   */
            tc_rarp_reply(pi, msg, p);
            return;
        }
    }
#endif

    /* Send the packet buffer back to the free buffer exchange   */
    os_free_packet(msg);               /* Free the input packet */
    return;
}

/* ********************************************************************   */
/* RELEASE MESSAGE                                                        */
/* ********************************************************************   */

/* ********************************************************************   */
/* tc_release_message - Release a packet that was queued for sending      */
/*                                                                        */
/*  A packet was either sent or the send timed out. If the caller         */
/*  requested we signal it, we place the status in the port structure     */
/*  and signal the caller.                                                */
/*                                                                        */
/*  If the caller wants the message freed the message is freed            */
/*                                                                        */
/*  This routine is called by process output and arp purge routines.      */
/*                                                                        */
/*  The arp semaphore must be claimed when this routine is called         */
/*  since it coordinates with the cleanup code in tc_netwrite.            */

void tc_release_message(DCU msg, int errno_val)  /*__fn__*/
{
PANYPORT port;
int  dcu_flags;

    /* get info for possible signal below: do this before freeing the
       packet */
    dcu_flags = DCUTOCONTROL(msg).dcu_flags;
    port = (PANYPORT)0;
    if (dcu_flags & PKT_FLAG_SIGNAL)
        port = DCUTOCONTROL(msg).port;

    /* Release the packet-unless it is flagged as a keeper      */
    if (!(dcu_flags & PKT_FLAG_KEEP))
    {
        DEBUG_LOG("tc_release_message() - threw DCU away", LEVEL_3, NOVAR, 0, 0);
        os_free_packet(msg);    /* Release message frees the packet */
    }

    /* If the caller wants notification the packet was sent let'm know     */
    /* do signal here: do not do this before freeing the packet or
       might wake-up task which will free the packet and then
       the dcu_flags might change */
    if (port)
    {
        /* Tell the port if the send worked   */
        port->ctrl.rtip_errno = errno_val;
        /* Signal TRUE if errno is 0 (no error) else False   */
        os_set_sent_signal(port, (RTIP_BOOLEAN) (errno_val == 0));
    }
}

#if (INCLUDE_ARP)
/* ********************************************************************   */
/* ARP ROUTINES                                                           */
/* ********************************************************************   */

/* ********************************************************************   */
/* tc_arp_purge_timeout() - delete entry in arp cache                     */
/*                                                                        */
/* Process an entry in the arp cache that needs to be deleted due to      */
/* timeout, snmp request etc                                              */
void tc_arp_purge_timeout(int i)
{
    tc_arpcache[i].arpc_state = ARPC_STATE_FREE;
    tc_arp_qpurge((word)i); /* purge the queue for this entry */
}

/* ********************************************************************   */
/*  This routine is called periodically (default == 1 second).            */
/*  It scans the arp cache looking for requests that have timed out       */
/*  or have become stale.                                                 */
/*                                                                        */
/*  Scan the arp cache. If ttl expires fire off                           */
/*  a retry, cancel an ARP_PENDING state or mark an entry stale.          */
/*  Flush queues of pending packets if there is no hope that they         */
/*  will be resolved.                                                     */
/*                                                                        */
/*  Called from the backstop routine in tc_tcpip_main() every second      */
/*                                                                        */
/*  Returns nothing                                                       */
/*                                                                        */

void tc_arp_timeout(dword gran)                                   /*__fn__*/
{
int i;

/*    DEBUG_LOG("\ntc_arp_timeout() entered", LEVEL_3, NOVAR, 0, 0);   */

    OS_CLAIM_TABLE(ARP_TMEOUT_TABLE_CLAIM)

    for (i=0; i < CFG_ARPCLEN; i++)   /* loop thru arp table */
    {
        switch(tc_arpcache[i].arpc_state)
        {
        case ARPC_STATE_FREE:
            continue;

        case ARPC_STATE_PENDING:
            /* decrement timeout/retry counter   */
            if (tc_arpcache[i].arpc_ttl != ARP_TTL_INF)
            {
                if (tc_arpcache[i].arpc_ttl >= gran)
                    tc_arpcache[i].arpc_ttl -= gran;
                else
                    tc_arpcache[i].arpc_ttl = 0;

                DEBUG_LOG("tc_arp_timeout - arp/gw index = ", LEVEL_3, EBS_INT1, i, 0);

                /* if timeout   */
                if (!tc_arpcache[i].arpc_ttl)
                {
                    DEBUG_LOG("tc_arp_timeout - timeout, check for retry index = ", LEVEL_3, EBS_INT1, i, 0);
                    if (tc_arpcache[i].arpc_nretries)
                    {
                        tc_arpcache[i].arpc_nretries -= 1;

#if (DISPLAY_ARP_RETRIES)
                        DEBUG_ERROR("tc_arp_timeout - retries left = ", 
                            EBS_INT1, tc_arpcache[i].arpc_nretries, 0);
                        DEBUG_ERROR("tc_arp_timeout - IP addr arping ", 
                            IPADDR, tc_arpcache[i].arpc_ip_addr, 0);
#else
                        DEBUG_LOG("tc_arp_timeout - retries left = ", 
                            LEVEL_3, EBS_INT1, tc_arpcache[i].arpc_nretries, 0);
                        DEBUG_LOG("tc_arp_timeout - IP addr arping ", 
                            LEVEL_3, IPADDR, tc_arpcache[i].arpc_ip_addr, 0);
#endif

                        /* Retries remaining. Try again   */
                        tc_arpcache[i].arpc_ttl = ARPC_REQ_TIMEOUT;
                        tc_do_arp_send(tc_arpcache[i].pi, 
                                    tc_arpcache[i].arpc_ip_addr,
                                    (PFBYTE)broadaddr, ARPREQ_68K);
                    }

                    else
                    {
                        /* No retries remaining. purge output queues   */
                        tc_arp_purge_timeout(i);
                    }
                }       /* arpc_ttl != INF */
            }
            break;
        case ARPC_STATE_RESOLVED:
            tc_arpcache[i].arpc_ttl -= gran;
            if (!tc_arpcache[i].arpc_ttl)
                 tc_arpcache[i].arpc_state = ARPC_STATE_FREE;
            break;
        default:
            DEBUG_ERROR("tc_arp_to - unknown arpc_state: ", EBS_INT1, 
                tc_arpcache[i].arpc_state, 0);
            break;
        }   /* end of switch */
    }
    OS_RELEASE_TABLE()
}

/* ********************************************************************   */
/* tc_arp_cachelook() - find entry in arp cache                           */
/*                                                                        */
/*  Search the cache for the entries which are not free with ip address   */
/*  at ipn.                                                               */
/*                                                                        */
/*  The cache should be locked                                            */
/*                                                                        */
/*  Returns the index in the arp cache. Or returns -1 if not found.       */
/*                                                                        */

static int tc_arp_cachelook(PFBYTE ipn)           /*__fn__*/
{
int i,found;

    found = -1;

    DEBUG_LOG("tc_arp_cachloop - looking for ", LEVEL_3, IPADDR, ipn, 0);

    /* linear search to see if we already have this entry   */
    for (i=0; found<0 && i<CFG_ARPCLEN; i++) 
    {
        if ( (tc_arpcache[i].arpc_state != ARPC_STATE_FREE) &&
             (tc_cmp4(ipn, tc_arpcache[i].arpc_ip_addr, 4)) )
            found = i;
    }
    return(found);
}

/* ********************************************************************    */
/* tc_arp_cachealloc() - allocate a arp cache entry                        */
/*                                                                         */
/* Search the cache for a free entry or one that may be reused. Return the */
/*  index or -1 if not found.                                              */
/*                                                                         */
/* The cache should be locked                                              */
/*                                                                         */
/* Returns the index in the arp cache. Or returns -1 if not found.         */
/*                                                                         */

static int tc_arp_cachealloc(void)                          /*__fn__*/
{
int   i,found;
dword timer;

    timer = arpc_res_tmeout;
    found = -1;

    for (i=0; i<CFG_ARPCLEN; i++) 
    {
        /* FREE is 0. (the initial value)   */
        if (tc_arpcache[i].arpc_state == ARPC_STATE_FREE)
        {   
            found = i;   
            break; 
        }

        /* only recycle entries in the resolved state. not pending    */
        /* take the oldest resolved entry.                            */
        if (tc_arpcache[i].arpc_state == ARPC_STATE_RESOLVED)
        {
            if ( (tc_arpcache[i].arpc_ttl != ARP_TTL_INF) &&
                 (tc_arpcache[i].arpc_ttl <= timer) )
            {
                timer = tc_arpcache[i].arpc_ttl;
                found = i;
            }
        }
    }

    if (found >= 0)
    {
        tc_memset(&(tc_arpcache[found]), 0, sizeof(struct _arpcache));
    }
    return(found);
}


/* ********************************************************************    */
/* tc_arp_just_update_cache() -  update an arp cache entry                 */
/*                                                                         */
/*  We just received an ARP packet from a host which exists in our cache,  */
/*  so need to update ethernet address for that host in our cache          */
/*                                                                         */
/*  The cache should be locked                                             */
/*                                                                         */
/*      Returns nothing                                                    */
/*                                                                         */

static void tc_arp_just_update_cache(PFBYTE packet, PFBYTE ipn, PFBYTE hrdn,    /*__fn__*/
                              PIFACE pi, int entry_num,                 /*__fn__*/
                              dword timeout)                            /*__fn__*/
{
    DEBUG_LOG("tc_arp_just_update_cache - cache entry", LEVEL_3, 
        IPADDR, ipn, 0);
#if (!INCLUDE_TOKEN)
    ARGSUSED_PVOID(packet);
#endif

    /* do the update to the cache   */
    tc_movebytes((PFBYTE)tc_arpcache[entry_num].arpc_hw_addr, (PFBYTE)hrdn,
         ETH_ALEN);
    tc_mv4(tc_arpcache[entry_num].arpc_ip_addr, ipn, 4);
    tc_arpcache[entry_num].arpc_ttl = timeout;

    if (pi && !tc_arpcache[entry_num].pi)
        tc_arpcache[entry_num].pi = pi;  /* save for SNMP */

    /* Now mark the state RESOLVED   */
    tc_arpcache[entry_num].arpc_state = ARPC_STATE_RESOLVED;
}

/* ********************************************************************   */
/* tc_arp_add_entry_to_cache() -  add a new entry to arp cache            */
/*                                                                        */
/*  We just received an ARP packet and we are the target protocol address */
/*  so we need to add the senders protocol and ethernet address to our    */
/*  cache if it does not exist there yet                                  */
/*                                                                        */
/*                                                                        */
/*  The cache should be locked                                            */
/*                                                                        */
/*  Returns the index of the new entry if it was successfully added to    */
/*  cache otherwise it returns -1                                         */
/*                                                                        */

static int tc_arp_add_entry_to_cache(PFBYTE packet, PFBYTE ipn, PFBYTE hrdn, /*__fn__*/
                                     PIFACE pi) /*__fn__*/
{
int found;

    DEBUG_LOG("tc_arp_update - add cache entry", LEVEL_3, IPADDR, ipn, 0);

    /* Allocate a cache entry   */
    found = tc_arp_cachealloc();

    if (found >= 0)
    {
        tc_arp_just_update_cache(packet, ipn, hrdn, pi, found, 
            arpc_res_tmeout);

        if (pi && !tc_arpcache[found].pi)
            tc_arpcache[found].pi = pi;  /* save for SNMP */

        /* Now mark the state RESOLVED   */
        tc_arpcache[found].arpc_state = ARPC_STATE_RESOLVED;

    }
    return(found);
}

/* ********************************************************************   */
/* tc_arp_qsend() - send packets on arp queue entry                       */
/*                                                                        */
/*   Sends any packets enqueued on this request by setting each packets   */
/*   destination hardware address and putting it on the output queue.     */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */
static void tc_arp_qsend(word index)                       /*__fn__*/
{
DCU    msg;
PIFACE pi;

    do 
    {
        msg = os_rcvx_arpcache_list(index);
        if (msg)
        {
            pi = DCUTOCONTROL(msg).pi;
            SETUP_LL_HDR_DEST(pi ,msg, 
                              (PFBYTE)tc_arpcache[index].arpc_hw_addr)
            /* Call interface send. And give the result to release message.
               if there is someone waiting on the packet release message
               will wake him up */
            INCR_INFO(pi, xmit_ucast)
            tc_do_interface_send(pi, msg);
        }
    } while (msg);
}

/* ********************************************************************   */
/* tc_arp_qpurge() - purge packets on arp cache entry                     */
/*                                                                        */
/*   Purges all packets enqueued on this request by returning each packet */
/*   to the free packet exchange. (called when an arp request times out)  */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */
static void tc_arp_qpurge(word index)                             /*__fn__*/
{
DCU msg;
#if (INCLUDE_TCP)
PTCPPORT port;
#endif

    do 
    {
        msg = os_rcvx_arpcache_list(index);
        if (msg)
        {
#if (INCLUDE_TCP)
            /* set up condition such that tc_tcp_timeout will detect      */
            /* the failure (same condition as if it had given up retries) */
            port = (PTCPPORT)(DCUTOCONTROL(msg).port);
            if (port)
            {
                if (port->ap.port_type == TCPPORTTYPE)
                {
                    DEBUG_ERROR("tc_arp_qpurge - giveup retry", NOVAR, 0, 0);
                    port->ap.port_flags |= RETRANS_EN;
                    port->ap.port_flags |= RETRANS_RUNNING;
                    port->sincetime = CFG_RETRANS_TMO + 1;
                    port->lasttime = port->rto + 1;
                    DEBUG_LOG("tc_tcp_force_timeout - port_flags = ", LEVEL_3, EBS_INT1, 
                            port->ap.port_flags, 0);
                }
            }
#endif

            DEBUG_LOG("tc_arp_qpurge - purge arp cache - rqst timed out", LEVEL_3, NOVAR, 0, 0);
            tc_release_message(msg, ETIMEDOUT);
        }
    } while (msg);
}


/* ********************************************************************   */
/* removes references to port in the arp cache; called when a port        */
/* is being freed                                                         */
void tc_arp_closeport(PANYPORT port)                            /*__fn__*/
{
int index;
DCU msg;

/*    DEBUG_LOG("tc_arp_closeport() entered", LEVEL_3, NOVAR, 0, 0);   */
    OS_CLAIM_TABLE(ARP_TMEOUT_TABLE_CLAIM)

    for (index=0; index < CFG_ARPCLEN; index++)   /* loop thru arp table */
    {
        if ( (tc_arpcache[index].arpc_state == ARPC_STATE_PENDING) ||
             (tc_arpcache[index].arpc_state == ARPC_STATE_RESOLVED) )
        {
            OS_ENTER_CRITICAL(ARPCP_CLAIM_CRITICAL)
            msg = (DCU)(tc_arpcache[index].ctrl.msg_list);
            while (msg)
            {
                if (port == DCUTOCONTROL(msg).port)
                    DCUTOCONTROL(msg).port = (PANYPORT)0;
                msg = (DCU)
                    os_list_next_entry_off(tc_arpcache[index].ctrl.msg_list, 
                                       (POS_LIST)msg, PACKET_OFFSET);
            }
            OS_EXIT_CRITICAL();
        }
    }
    OS_RELEASE_TABLE()
}
#endif  /* INCLUDE_ARP */

#if (INCLUDE_ARP || INCLUDE_ETH_BUT_NOARP)
/* ********************************************************************   */
/* tc_do_arp_send() - send an arp request                                 */
/*                                                                        */
/*   Sends an arp request trying to resolve the IP address ipn.           */
/*   If sending an arp reply, msg contains the arp request                */
/*                                                                        */
/*  Returns errno value where 0 means success                             */
/*                                                                        */
int tc_do_arp_send(PIFACE pi, PFBYTE ipn, PFBYTE lln, word type)   /*__fn__*/
{
DCU     msg_out;
PARPPKT pa;
int     tlen;
#if (INCLUDE_TOKEN || INCLUDE_802_2)
RTIP_BOOLEAN is_802_2;
#endif
    DEBUG_LOG("tc_do_arp_send entered", LEVEL_3, NOVAR, 0, 0);

#if (INCLUDE_802_2)
    is_802_2 = pi->iface_flags & IS_802_2;
#endif

    TOTAL_LL_HLEN_SIZE(tlen, is_802_2, pi, 0)
    tlen += sizeof(*pa);

    /* grab a buffer to send the arp request/reply out on    */
    /* (will be freed when sent)                             */
    msg_out = ip_alloc_packet(pi, (PANYPORT)0, tlen, 0, ARP_SEND_ALLOC);
    if (!msg_out)
    {
        DEBUG_LOG("ERROR(rtip) : tc_do_arp_send - os_alloc_packet failed", LEVEL_3, NOVAR, 0, 0);
        return(ENOPKTS);
    }

    pa = DCUTOARPPKT(msg_out);
        pa->ar_hw_type = HTYPE_68K;                  /* Type, (redundant)  */
    pa->ar_pr_type = ARPPRO_68K;                 /* byte swapped reply opcode  */
    pa->ar_hw_len = ETH_ALEN;
    pa->ar_pr_len = IP_ALEN;
    pa->ar_opcode = type;                       /* byte swapped reply opcode  */
    tc_movebytes((PFBYTE)pa->ar_sha, (PFBYTE) pi->addr.my_hw_addr, ETH_ALEN);
                                                      /* My hw address    */
    tc_mv4(pa->ar_spa, pi->addr.my_ip_addr, 4); 
                                                     /* My IP address    */
    tc_movebytes((PFBYTE)pa->ar_tha, (PFBYTE)lln, LL_ALEN);  
                                                     /* His hw address    */
    tc_mv4(pa->ar_tpa, ipn,4);                       /* His IP address  */

    /* NOTE: arp packet must be setup before link layer header for   */
    /*       token ring due to setup_rif_802_2_header checking       */
    /*       the type of packet, i.e. checks if ARP request          */
    SETUP_LL_HDR(msg_out, pi, EARP_68K, is_802_2, 
                 sizeof(*pa) + LLC_SNAP_HLEN_BYTES)
    SETUP_LL_HDR_DEST(pi, msg_out, (PFBYTE)lln)

    /* Enqueue the reply for sending over the wire    */
    DCUTOPACKET(msg_out)->length = tlen;
    return(tc_interface_send(pi, msg_out));
}
#endif /* INCLUDE_ARP || INCLUDE_ETH_BUT_NOARP */

/* ********************************************************************   */
/* ARP API FUNCTIONS                                                      */
/* ********************************************************************   */

#if (INCLUDE_ARP)
/* ********************************************************************      */
/* tc_arp_send() - Sends a gratuitous arp                                    */
/*                                                                           */
/*   Broadcasts an ARP request to all hosts on the network using its own     */
/*   IP address as the sender and target protocol address in order to force  */
/*   all other hosts on the network to update the entry in their ARP cache   */
/*   for this IP address.                                                    */
/*                                                                           */
/* Returns:                                                                  */
/*   0 if successful or -1 if an error was detected                          */
/*                                                                           */

int tc_arp_send(int interface_no)
{
PIFACE pi;
int    ret_val;
      
    RTIP_API_ENTER(API_XN_ARP_SEND)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_ARP_SEND)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    pi = tc_ino2_iface(interface_no, SET_ERRNO);
    if (!pi)
    {
        RTIP_API_EXIT(API_XN_ARP_SEND)
        return(-1);
    }

    DEBUG_LOG("Arp send", LEVEL_3, NOVAR, 0, 0);
    ret_val = tc_do_arp_send(pi, pi->addr.my_ip_addr, (PFBYTE)broadaddr, 
                             ARPREQ_68K);
    RTIP_API_EXIT(API_XN_ARP_SEND)
    return(ret_val);

}
#endif /* INCLUDE_ARP */

#if (INCLUDE_ARP || INCLUDE_ETH_BUT_NOARP)
/* ********************************************************************   */
/* tc_arp_add() - Add an entry to the arp cache                           */
/*                                                                        */
/*   Adds an entry to ARP cache.  If an entry already exists for          */
/*   the specified IP address, the entry is updated with                  */
/*   the specified parameter information.                                 */
/*                                                                        */
/*   If time_to_live parameter is ARP_TTL_INF, the entry never expires.   */
/*                                                                        */
/* Returns:                                                               */
/*   0 if successful or -1 if an error was detected                       */
/*                                                                        */

int tc_arp_add(PFBYTE ipn, PFBYTE ethn, dword time_to_live)
{
int found;
int ret_val;
PIFACE pi;

    RTIP_API_ENTER(API_XN_ARP_ADD)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_ARP_ADD)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    ret_val = 0;
    pi = rt_get_iface(ipn, (PFBYTE)0, (PANYPORT)0, (RTIP_BOOLEAN *)0);

    /* We'll be searching the cache. Don't change it on us   */
    OS_CLAIM_TABLE(ARP_ADD_TABLE_CLAIM)

#if (INCLUDE_ARP)
    found = tc_arp_cachelook(ipn);
    if (found < 0)
    {
        /* Allocate a cache entry   */
        found = tc_arp_cachealloc();
    }

    if (found >= 0)
    {
        /* Set up the cache table entry   */
        tc_arp_just_update_cache((PFBYTE)0, ipn, ethn, pi, found, 
                                 time_to_live);

        tc_arpcache[found].arpc_nretries = ARPC_MAX_RETRIES;
        tc_arpcache[found].pi = pi;  /* save for retries */
    }
    else
    {
        ret_val = -1;
        set_errno(ETABLEFULL);
    }
#endif  /* INCLUDE_ARP */

#if (INCLUDE_ETH_BUT_NOARP)
    ARGSUSED_INT(time_to_live)
    for (found=0; found < CFG_NUM_IP2ETH_ADDR; found++)
    {
        if (tc_cmp4(ip2eth_table[found].ip_addr, ip_nulladdr, IP_ALEN))
        {
            tc_mv4(ip2eth_table[found].ip_addr, ipn, IP_ALEN);
            tc_movebytes(ip2eth_table[found].hw_addr, ethn, ETH_ALEN);
            break;
        }
    }
    if (found >= CFG_NUM_IP2ETH_ADDR)
    {
        ret_val = -1;
        set_errno(ETABLEFULL);
    }
#endif      /* INCLUDE_ETH_BUT_NOARP */

    OS_RELEASE_TABLE()
    RTIP_API_EXIT(API_XN_ARP_ADD)
    return(ret_val);
}

/* ********************************************************************   */
/* tc_arp_del() - Delete an entry from the arp cache                      */
/*                                                                        */
/*   Delete an entry from the ARP cache.                                  */
/*                                                                        */
/* Returns:                                                               */
/*   0 if successful or -1 if an error was detected                       */
/*                                                                        */

int tc_arp_del(PFBYTE ipn)
{
int found;
int ret_val;

    RTIP_API_ENTER(API_XN_ARP_DEL)

    ret_val = 0;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_ARP_DEL)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    /* We'll be searching the cache. Don't change it on us   */
    OS_CLAIM_TABLE(ARP_DEL_TABLE_CLAIM)

#if (INCLUDE_ARP)
    found = tc_arp_cachelook(ipn);
    if (found < 0)
    {
        set_errno(EFAULT);
        ret_val = -1;
    }
#endif

#if (INCLUDE_ETH_BUT_NOARP)
    for (found=0; found < CFG_NUM_IP2ETH_ADDR; found++)
    {
        if (tc_cmp4(ip2eth_table[found].ip_addr, ipn, IP_ALEN))
        {
            tc_mv4(ip2eth_table[found].ip_addr, ip_nulladdr, IP_ALEN);
            break;
        }
    }
    if (found >= CFG_NUM_IP2ETH_ADDR)
    {
        ret_val = -1;
        set_errno(EFAULT);
    }
#endif

    OS_RELEASE_TABLE()
    RTIP_API_EXIT(API_XN_ARP_DEL)
    return(ret_val);
}

#endif  /* INCLUDE_ARP || INCLUDE_ETH_BUT_NOARP */


/* ********************************************************************   */
/* RARP FUNCTIONS                                                         */
/* ********************************************************************   */

#if (INCLUDE_RARP)
/* ********************************************************************   */
/* tc_rarp_send() - send a RARP request                                   */
/*                                                                        */
/*   Formats and sends a RARP request on interface pi.                    */
/*                                                                        */
/*   Return 0 if successful, ERRNO if not                                 */
/*                                                                        */

int tc_rarp_send(PIFACE pi)            /*__fn__*/
{
DCU     msg;
PARPPKT pa;

    /* grab a buffer to send the arp request out on   */
    msg = ip_alloc_packet(pi, (PANYPORT)0, sizeof(*pa)+ETH_HLEN_BYTES, 0,
                          RARP_SEND_ALLOC);
    if (!msg)
    {
        DEBUG_LOG("ERROR(rtip) : tc_rarp_send - ip_alloc_packet failed", LEVEL_3, NOVAR, 0, 0);
        return(ENOPKTS);
    }

    pa = DCUTOARPPKT(msg);

    SETUP_LL_HDR_DEST(pi, msg, broadaddr)
                                                  /* hw place to send to    */
    SETUP_LL_HDR(msg, pi, ERARP_68K, FALSE, 0)

    pa->ar_hw_type = HTYPE_68K;                  /* Type, (redundant)  */
    pa->ar_pr_type = ARPPRO_68K;                 /* byte swapped protocol */
    pa->ar_hw_len = ETH_ALEN;
    pa->ar_pr_len = IP_ALEN;
    pa->ar_opcode =  RARPQ_68K;                  /* byte swapped opcode  */
    tc_movebytes((PFBYTE)pa->ar_sha, (PFBYTE)pi->addr.my_hw_addr, ETH_ALEN);
                                                 /* My hw address    */
    tc_movebytes((PFBYTE)pa->ar_tha, (PFBYTE)pi->addr.my_hw_addr, ETH_ALEN);
                                                 /* My hw address    */

    /* Enqueue the reply for sending over the wire    */
    DCUTOPACKET(msg)->length = sizeof(*pa)+ETH_HLEN_BYTES;
    return(tc_interface_send(pi, msg));
}
#endif      /* INCLUDE_RARP */

/* ********************************************************************   */
/* RARP SERVER FUNCTIONS                                                  */
/* ********************************************************************   */

#if (INCLUDE_RARP_SRV)

/* ********************************************************************    */
/* tc_rarp_reply() - reply to a RARP request                               */
/*                                                                         */
/*   Sends a reply to a RARP request.                                      */
/*   NOTE: you need to fill in the table that feeds tc_rarp_lookup(), i.e. */
/*         rt                                                              */
/*                                                                         */
/*   Returns nothing                                                       */

void tc_rarp_reply(PIFACE pi, DCU msg, PFBYTE ip_address)    /*__fn__*/
{
PARPPKT pa;

    pa = DCUTOARPPKT(msg);

    /* Return the ether net packet by copying the source into the dest   */
    SETUP_LL_HDR_DEST(pi, msg, LL_SRC_ADDR(msg, pi))
                                                    /* hw place to send to    */
    SETUP_LL_HDR(msg, pi, ERARP_68K, FALSE, 0)

    /* Put the RARPER's IP address in the target protocol address   */
    /* The hardware address is already in target.                   */
    tc_mv4(pa->ar_tpa, ip_address,IP_ALEN);

    /* Supply my ip and ethernet address   */
    tc_mv4(pa->ar_spa, pi->addr.my_ip_addr,IP_ALEN);
    tc_movebytes((PFBYTE)pa->ar_sha, (PFBYTE)pi->addr.my_hw_addr ,ETH_ALEN);
                                                   /* My hw address    */

    /* Reload the packet. This is redundant since the packet was set up by the   */
    /* sender, but it documents the packet structure so we do it.                */
    pa->ar_hw_type = HTYPE_68K;                  /* Type, (redundant)  */
    pa->ar_pr_type = ARPPRO_68K;                 /* byte swapped protocol */
    pa->ar_hw_len  = ETH_ALEN;
    pa->ar_pr_len  = IP_ALEN;

    /* Set the opcode to reply   */
    pa->ar_opcode=  RARPR_68K;                  /* byte swapped opcode  */

    /* Enqueue the reply for sending over the wire    */
    DCUTOPACKET(msg)->length = sizeof(*pa)+ETH_HLEN_BYTES;
    tc_interface_send(pi, msg);
}

/* ********************************************************************   */
/* tc_rarp_lookup() - look for a match in the rarp table                  */
/*                                                                        */
/*  Given an ethernet address look up its IP address in a table.          */
/*  We only have two stations "scott" and "rachel" so we return           */
/*  either of their addresses if a match occurs.                          */
/*                                                                        */
/*  If you generalize this the rarp server will be fully functional       */
/*                                                                        */

PFBYTE tc_rarp_lookup(PFBYTE ether_address)                   /*__fn__*/
{
dword *p;
int i;
    
    for (i = 0; ; i++)
    {
        /* Look for null termination   */
        p = (dword *) rt[i].ip_addr;
        if (!*p)
            break;
        if (tc_comparen(ether_address,(PFBYTE)rt[i].ether_addr,ETH_ALEN))
            return((PFBYTE ) rt[i].ip_addr);
    }
    return((PFBYTE ) 0);
}
#endif  /* INCLUDE_RARP_SRV */

#endif  /* INCLUDE_IPV4 */

