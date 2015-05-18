/*                                                                      */
/* NAME2IP.C - xn_name2ip_addr utility function                         */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Ger 'Insh_Allah' Hobbelt, 2000                             */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DNS

#include    "sock.h"

RTIP_BOOLEAN ip_str_to_bin(PFBYTE address, PFCCHAR string);

/*
 * <name> may be valid decimal dotted IP number of DNS resolvable name.
 *
 * Return converted IP number in 'ip_addr' and TRUE as function result.
 *
 * Return 'ip_ffaddr'/'ip_nulladdr' (255.255.255.255/0.0.0.0) in ip_addr and FALSE 
 * as function result on error!
 *
 * NOTE: Errors are:
 * - IP numbers 255.255.255.255 and 0.0.0.0 and anything that leads to these two
 * - 'name' cannot be resolved by DNS for any reason
 * - 'name' is not a decimal dotted number that matches IP address specifications 
 *          (format: ddd.ddd.ddd.ddd) where 'ddd' may be any decimal number 
 *          from 0..255
 *
 * Examples of unaccepted (--> 'faulty') inputs:
 *
 *   xn_name2ip_addr(xxxx, "255.255.255.255")           --> FALSE [255.255.255.255]
 *   xn_name2ip_addr(xxxx, "0.0.0.0")                   --> FALSE [0.0.0.0]
 *   xn_name2ip_addr(xxxx, "1000.0.0.0")                --> FALSE [255.255.255.255]
 *   xn_name2ip_addr(xxxx, "totally.unknown.dns.entry") --> FALSE [255.255.255.255]
 */
int SOCKAPI_ xn_name2ip_addr(PFBYTE ip_addr, PFCCHAR name)
{
    int ret = FALSE;

    /* resolve server name: may be dotted IP number or DNS resolvable name: try dotted IP number *FIRST* for speed!   */
    if (!ip_str_to_bin(ip_addr, name))
    {
#if (INCLUDE_DNS != 0)
        PFHOSTENT hostresult;

        /* check if the DNS has already been properly set up!   */
        if (name[0] != '\0' && xn_has_dns())
        {
            hostresult = gethostbyname((PFCHAR)name);
            if (hostresult)
            {
                tc_mv4(ip_addr, (PFBYTE)&(hostresult->ip_addr.s_un.s_un_b.s_b1), IP_ALEN);
                ret = TRUE;
            }
        }
#endif
    }
    else
    {
        ret = TRUE;
    }

    /*
     * Yes, next might look like too much of a good thing, but we don't accept 
     * IPs 0.0.0.0 or 255.255.255.255.
     * And it would be kinda sad if we reported 'OK' while the input resolved 
     * to either of 'em.
     */

    if (ret)
        ret = !tc_cmp4(ip_addr, ip_ffaddr, IP_ALEN);  /* successful decode; 255.255.255.255 however? :-( */
    else
        tc_mv4(ip_addr, (PFBYTE)ip_ffaddr, IP_ALEN);

    if (ret)
        ret = !tc_cmp4(ip_addr, ip_nulladdr, IP_ALEN);

    return ret;
}



