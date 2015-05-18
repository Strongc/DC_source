/*                                                                       */
/*  PPPIPCP.C   -- negotiate IP parameters                               */
/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */
/*                                                                       */
/*  Jan 91  Bill_Simpson@um.cc.umich.edu                                 */
/*      Computer Systems Consulting Services                             */
/*                                                                       */
/*  Acknowledgements and correction history may be found in PPP.C        */
/*                                                                       */

#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP


#include "sock.h"
#include "rtip.h"

#define PEER_POOL 1

#if (INCLUDE_PPP)
#if (INCLUDE_PPP_DNS)
#include "socket.h"
#endif

/* ********************************************************************   */
/* LOCAL FUNCTIONS                                                        */
static void ipcp_option(DCU msg, PIPCPVALUES value_p, byte o_type,
                        byte o_length, PFBYTE copy_pb);
static void ipcp_makeoptions(DCU msg, PIPCPVALUES value_p, word negotiating);

#if (INCLUDE_PPP_DNS)
int check_nak_addr(PFBYTE ip_temp, PFBYTE want_addr, PFBYTE work_addr, RTIP_BOOLEAN will_negotiate);
int check_req_addr(PFBYTE other, PFBYTE want_addr, PFBYTE work_addr,
                   RTIP_BOOLEAN will_negotiate);
#endif
int ipcp_address_check(PFSMS fsm_p, RTIP_BOOLEAN request, 
                       PFBYTE ip_temp, int option_result,
                       PFBYTE local_want_addr,
                       PFBYTE local_work_addr,
                       PFBYTE remote_want_addr,
                       PFBYTE remote_work_addr,
                       PFBYTE other_addr,
                       RTIP_BOOLEAN will_negotiate);
static int ipcp_check (PFBYTE pb, PFSMS fsm_p, PIPCPSIDES side_p, 
                       POPTION_HDR option_p, RTIP_BOOLEAN request);

#if (PEER_POOL)
static void ipcp_addr_idle(int iface, PFBYTE addr, PFBYTE nextaddr);
void ipcp_poolnext(int iface, PIPCPS ipcp_p, PFBYTE nextaddr);
#endif

/* ********************************************************************   */
/* These defaults are defined in the PPP RFCs                             */
KS_EXTERN_GLOBAL_CONSTANT struct ipcp_value_s KS_FAR ipcp_default; 

/* Options to accept in NAK(local) or REQ(remote)   */
KS_EXTERN_GLOBAL_CONSTANT word KS_FAR ipcp_negotiate;

KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ipcp_option_length[IPCP_OPTION_LIMIT+1];

PPP_EXTERN_GLOBAL_CONSTANT struct fsm_constant_s KS_FAR ipcp_constants;  
#if (INCLUDE_PPP_DNS)
extern dword KS_FAR server_ip_table[CFG_MAX_DNS_SRV]; /* name servers */
#endif

/* ********************************************************************    */
/*          E V E N T   P R O C E S S I N G                                */
/* ********************************************************************    */

/* ********************************************************************    */
/* FORMAT OPTIONS                                                          */
/* ********************************************************************    */
static void ipcp_option(DCU msg, PIPCPVALUES value_p, byte o_type,
                        byte o_length, PFBYTE copy_pb)
{
PFBYTE cp;
register int toss = o_length - OPTION_HDR_LEN;

    if ( (DCUTOPACKET(msg)->length + o_length) > MAX_PACKETSIZE )
    {
        DEBUG_ERROR("ipcp_option: not enough room for option", NOVAR, 0, 0);
        return; 
    }

    /* point to end of packet so can append option   */
    cp = DCUTODATA(msg) + DCUTOPACKET(msg)->length;

    /* put on option header   */
#if (INCLUDE_PPP_DNS)
    if (o_type ==  IPCP_DNS_PRIMARY_SH)
        *cp++ = IPCP_DNS_PRIMARY;         
    else if (o_type ==  IPCP_DNS_SECOND_SH)
        *cp++ = IPCP_DNS_SECOND;         
    else
#endif
        *cp++ = o_type;         
    *cp++ = o_length;

    switch ( o_type ) 
    {
#if (INCLUDE_PPP_IPCP_ADDRESSES)
        case IPCP_ADDRESSES:
            tc_mv4(cp+4, value_p->other, IP_ALEN);
            toss -= IP_ALEN;
#endif
        case IPCP_ADDRESS:
            tc_mv4(cp, value_p->address, IP_ALEN);
            DEBUG_LOG("ipcp_option - ADDRESS - sending", 
                LEVEL_3, IPADDR, value_p->address, 0);
            toss -= IP_ALEN;
            break;

#if (INCLUDE_PPP_VANJC)
        case IPCP_COMPRESS:
            *((PFWORD)cp) = hs2net(PPP_COMPR_PROTOCOL);
            cp += 2;
            toss -= 2;
            *cp++ = (byte)(value_p->slots - 1);
            *cp++ = value_p->slot_compress;
            toss -= 2;
            break;
#endif

#if (INCLUDE_PPP_DNS)
        case IPCP_DNS_PRIMARY_SH:
            tc_mv4(cp, value_p->dns1_addr, IP_ALEN);
            DEBUG_LOG("ipcp_option - ADDRESS - sending", 
                LEVEL_3, IPADDR, value_p->dns1_addr, 0);
            toss -= IP_ALEN;
            break;
#endif

#if (INCLUDE_PPP_DNS)
        case IPCP_DNS_SECOND_SH:
            tc_mv4(cp, value_p->dns2_addr, IP_ALEN);
            DEBUG_LOG("ipcp_option - ADDRESS - sending", 
                LEVEL_3, IPADDR, value_p->dns2_addr, 0);
            toss -= IP_ALEN;
            break;
#endif

        default:
            break;
    };

    /* if padding data specified and there is room for it, copy the padding   */
    /* data to the packet                                                     */
    if (copy_pb)
    {
        cp += ipcp_option_length[o_type] - OPTION_HDR_LEN;
        while (toss--> 0) 
            *cp++ = *copy_pb++;
    }
    DCUTOPACKET(msg)->length += o_length;
}


/* ********************************************************************   */
RTIP_BOOLEAN ipcp_option_supported(byte o_type)
{
#if (!INCLUDE_PPP_VANJC)
    if (o_type == IPCP_COMPRESS)
        return(FALSE);
#endif

    if (o_type <= IPCP_OPTION_LIMIT)
        return(TRUE);
    return(FALSE);
}

/* ********************************************************************   */
/* Build a list of options                                                */
static void ipcp_makeoptions(DCU msg, PIPCPVALUES value_p, word negotiating)
{
byte o_type;

    DEBUG_LOG("ipcp_makeoptions()", LEVEL_3, NOVAR, 0, 0);

    for ( o_type = 1; o_type <= IPCP_OPTION_LIMIT; o_type++ ) 
    {
        if (negotiating & (1 << o_type)) 
        {
            ipcp_option(msg, value_p, o_type, ipcp_option_length[o_type], 
                (PFBYTE)0);
        }
    }
}


/* ********************************************************************    */
/* FORMAT REQUEST                                                          */
/* ********************************************************************    */
/* Build a request to send to remote host                                  */
DCU ipcp_makereq(PFSMS fsm_p)
{
PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;
DCU msg;

    DEBUG_LOG("ipcp_makereq()", LEVEL_3, NOVAR, 0, 0);

    /* allocate a packet used to build options up in   */
    msg = os_alloc_packet(MAX_PACKETSIZE, IPCP_MR_ALLOC);
    if (!msg)
        return(msg);  

    /* leave room for ppp and config header headers   */
    DCUTOPACKET(msg)->length = OUT_PKT_HDR_LEN;

    ipcp_makeoptions(msg, (PIPCPVALUES)&(ipcp_p->local_entry.work), 
                     ipcp_p->local_entry.work.negotiate);
    return(msg);
}

/* ********************************************************************    */
/* EXTRACT HEADER                                                          */
/* ********************************************************************    */
/* Extract configuration option header for IPCP packet                     */
int ipcp_ntohopt(POPTION_HDR opt, PFBYTE pb)
{
    if (!opt)
        return -1;

    opt->type = pb[0];
    opt->len = pb[1];
    if (opt->type == IPCP_DNS_PRIMARY)
        opt->type = IPCP_DNS_PRIMARY_SH;
    else if (opt->type == IPCP_DNS_SECOND)
        opt->type = IPCP_DNS_SECOND_SH;
    return 0;
}

/* ********************************************************************    */
/* PROCESS REQ and NAK                                                     */
/* ********************************************************************    */
/* ********************************************************************    */
/* Check the options, updating the working values.
 * Called for REQUEST and NAK packets.
 * Returns -1 if ran out of data, ACK/NAK/REJ as appropriate.
 */
static int ipcp_check(PFBYTE pb, PFSMS fsm_p, PIPCPSIDES side_p, 
                      POPTION_HDR option_p, RTIP_BOOLEAN request)
{
PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;
int toss = option_p->len - OPTION_HDR_LEN;
int option_result = CONFIG_ACK;     /* Assume good values */
byte ip_temp[IP_ALEN];
#if (INCLUDE_PPP_VANJC)
byte test;
#endif

    DEBUG_LOG("ipcp_check - remote.want.address = ", LEVEL_3, IPADDR, 
        ipcp_p->remote_entry.want.address, 0);

    /* if invalid option or the option is not negotiable                       */
    /* NOTE: for REQ: if reject it want to have work.address and work.other    */
    /*                updated so send back reject with same values as received */
    /*       for NAK: if reject it will just send REQ again with same          */
    /*                values sent origionally                                  */
    if ( option_p->type > IPCP_OPTION_LIMIT ||
         !(side_p->will_negotiate & (1 << option_p->type)) ) 
    {                          
        DEBUG_LOG("ipcp_check - reject since not negotiable or out of range", LEVEL_3, NOVAR, 0, 0);
        option_result = CONFIG_REJ;
        if ( !request )             /* if NAK */
            return(CONFIG_REJ);
    }

    switch (option_p->type) 
    {
#if (INCLUDE_PPP_IPCP_ADDRESSES)
        case IPCP_ADDRESSES:
            DEBUG_LOG("ipcp_check - IPCP_ADDRESSES = ", LEVEL_3, IPADDR, 
                pb+4, 0);
            tc_mv4(side_p->work.other, pb+IP_ALEN, IP_ALEN);
            toss -= IP_ALEN;

            DEBUG_LOG("ipcp_check - IPCP_ADDRESS = ", LEVEL_3, IPADDR, 
                pb, 0);
            /* set working addresses to requested/naked address   */
            /* side_p : REQ is remote, NAK is local               */
            tc_mv4(ip_temp, side_p->work.address, IP_ALEN);
            tc_mv4(side_p->work.address, pb, IP_ALEN);
            toss -= IP_ALEN;

            /* ******                                                                */
            /* if checking NAK from remote                                           */
            /* NOTE: the handling of a NAK from remote host is handled differently   */
            /*       for ADDRESS and ADDRESSES; ADDRESSES is done like the origional */
            /*       KA9Q code and never accecpts a NAK for the address (I think     */
            /*       this is wrong)-                                                 */
            /*       ADDRESS will accept the address in the NAK if                   */
            /*          - we sent an address of 0                                    */
            /*          - as long as the negotiation flag is set                     */
            if (!request)
            {
                DEBUG_LOG("ipcp_check - checking NAK", LEVEL_3, NOVAR, 0, 0);

                /* override any undesirable changes which were set when came
                   into this routine*/
                /* NOTE: for addresses, if NAK then only use addresses in      */
                /*       NAK if we sent a 0, i.e. requested address            */
                /*       (overrides only if address we want is not 0, in other */
                /*        words use address in NAK if address we want is 0)    */
                if (option_p->type == IPCP_ADDRESSES)
                {
                    if (!tc_cmp4(ipcp_p->remote_entry.want.address,
                                 (PFBYTE)ip_nulladdr, IP_ALEN)) 
                    {
                        tc_mv4(ipcp_p->local_entry.work.other, 
                               ipcp_p->remote_entry.want.address, IP_ALEN);
                    }
                    if (!tc_cmp4(ipcp_p->local_entry.want.address, 
                                 ip_nulladdr, IP_ALEN)) 
                    {
                        tc_mv4(ipcp_p->local_entry.work.address,
                               ipcp_p->local_entry.want.address, IP_ALEN);
                    }
                }

                /* for ADDRESS, if otherside NAKed our address, use the    */
                /* address they sent but if they naked our request of 0    */
                /* then they don't know address either                     */
                else if (!tc_cmp4(ipcp_p->local_entry.want.address, 
                                  ip_nulladdr, IP_ALEN)) 
                {
                    /* we know our address -                                */
                    /* if otherside NAKed with 0, they are requesting us to */
                    /* send the address so restore what we know (i.e. fix   */
                    /* back to values before this routine was called)       */
                    if (tc_cmp4(ipcp_p->local_entry.work.address, 
                                ip_nulladdr, IP_ALEN))
                    {
                        tc_mv4(ipcp_p->local_entry.work.address,
                            ipcp_p->local_entry.want.address, IP_ALEN);
                        DEBUG_LOG("ipcp_check - otherside NAKed our 0 with", LEVEL_3, 
                            IPADDR, ipcp_p->local_entry.want.address, 0);
                    }

                    /* they NAKed with an address and we did not REQ 0s; use   */
                    /* their address if the address itself is negotiable       */
                    /* (i.e. when xn_want_ipcp_address was called a negotiable */
                    /*       address was specified)                            */
                    else if (side_p->will_negotiate & IPCP_ADDRESS_NEGOTIABLE) 
                    {
                        tc_mv4(ipcp_p->local_entry.want.address,
                            ipcp_p->local_entry.work.address, IP_ALEN);
                        DEBUG_LOG("ipcp_check - otherside NAKed with (ours not 0)-its negotiable", 
                            LEVEL_3, IPADDR, ipcp_p->local_entry.work.address, 0);
                    }
                    else
                    {
                        DEBUG_LOG("ipcp_check - otherside NAKed with (ours not 0)-not negotiable=>REJECT", 
                            LEVEL_3, IPADDR, ipcp_p->local_entry.work.address, 0);
                        tc_mv4(side_p->work.address, ip_temp, IP_ALEN);
                        return(CONFIG_NAK);
                    }
                }
                break;
            }

            /* ******                                           */
            /* THE FOLLOWING CODE IS FOR NAK ONLY               */
            /* Ensure that source address field matches         */
            /* NOTE: this is really the remote address for REQ  */

            /* if rejected above return since work.address and work.other   */
            /* have been updated; i.e. reject needs to send back same       */
            /* addresses that were in the request                           */
            if (option_result == CONFIG_REJ)
            {
                DEBUG_LOG("ipcp_check - REJ - option result is reject", 
                    LEVEL_3, NOVAR, 0, 0);
                return(CONFIG_REJ);
            }

            /* ******                             */
            /* THE FOLLOWING CODE IS FOR REQ ONLY */
            DEBUG_LOG("ipcp_check - checking REQ", LEVEL_3, NOVAR, 0, 0);

            /* REQ: check if othersides IP (from source field) matches what         */
            /*      we want                                                         */
            /* NOTE: ipcp_p->remote_entry.work.address is source field from in pkt  */
            if (tc_cmp4(ipcp_p->remote_entry.work.address, 
                        ipcp_p->remote_entry.want.address, IP_ALEN)) 
            {
                DEBUG_LOG("ipcp_check - REQ - otherside IP matches what we want", 
                    LEVEL_3, IPADDR, ipcp_p->local_entry.work.address, 0);

                /* REQ: it matches, if both are null addresses, otherside is    */
                /*      requesting their IP address but we do not know it       */
                if (tc_cmp4(ipcp_p->remote_entry.want.address, 
                            ip_nulladdr, IP_ALEN)) 
                {
                    DEBUG_LOG("ipcp_check-1-reject since both are null addresses", LEVEL_3, NOVAR, 0, 0);
                    option_result = CONFIG_REJ;
                }
            } 

            /* REQ: othersides IP (from source field) does not match, accept   */
            /*      what remote wants if                                       */
            /*        - ours is null then we now know othersides IP address    */
            /*        - the address is negotiable and remote is not sending 0  */
            else if ( tc_cmp4(ipcp_p->remote_entry.want.address,
                              (PFBYTE)ip_nulladdr, IP_ALEN)       || 
                      (!tc_cmp4(ipcp_p->remote_entry.work.address, 
                                (PFBYTE)ip_nulladdr, IP_ALEN)     &&
                      (side_p->will_negotiate & IPCP_ADDRESS_NEGOTIABLE)) )
            {
                tc_mv4(ipcp_p->local_entry.work.other, 
                       ipcp_p->remote_entry.work.address, IP_ALEN);
                DEBUG_LOG("ipcp_check - REQ - IP no match - ours is null, save it", 
                    LEVEL_3, IPADDR, ipcp_p->remote_entry.work.address, 0);
            } 

            /* REQ: othersides IP (from source field) does not match and ours   */
            /*      is not null so NAK it with what we think it should be       */
            else 
            {
                DEBUG_LOG("ipcp_check - otherside IP does not match - NAK it", 
                    LEVEL_3, IPADDR, ipcp_p->local_entry.work.address, 0);
                option_result = CONFIG_NAK;
            }

            /* ******                                           */
            /* Ensure that destination address field matches    */
            /* NOTE: this is really the local address for REQ   */

            /* REQ: check if our IP (from dest field) matches what    */
            /*      we want                                           */
            if (option_p->type == IPCP_ADDRESSES)
            {
                DEBUG_LOG("ipcp_check - REQ - IPCP_ADDRESSES", 
                    LEVEL_3, IPADDR, ipcp_p->remote_entry.work.other, 0);
                /* NOTE: ipcp_p->remote_entry.work.other is dest field from in pkt    */
                if (tc_cmp4(ipcp_p->remote_entry.work.other, 
                            ipcp_p->local_entry.want.address, IP_ALEN)) 
                {
                    /* REQ: if our IP (from dest field) matches what we want and   */
                    /*      both are null, they are requesting our IP address but  */
                    /*      we do not know it, so rej packet                       */
                    if (tc_cmp4(ipcp_p->local_entry.want.address, 
                                (PFBYTE)ip_nulladdr, IP_ALEN)) 
                    {
                        /* don't know address either   */
                        DEBUG_LOG("ipcp_check-2-reject since both are null addresses", LEVEL_3, NOVAR, 0, 0);
                        option_result = CONFIG_REJ;
                    }
                } 

                /* REQ: our IP (from dest field) does not matches what we want   */
                /*      and we do not know our IP address then save our address  */
                else if (tc_cmp4(ipcp_p->local_entry.want.address, 
                                 (PFBYTE)ip_nulladdr, IP_ALEN)) 
                {
                    tc_mv4(ipcp_p->local_entry.work.address,
                           ipcp_p->remote_entry.work.other, IP_ALEN);
                } 

                /* REQ: our IP (from dest field) does not matches what we want,   */
                /*      NAK packet and set our IP address back to orig value      */
                /*      before packet came in                                     */
                else 
                {
                    option_result = CONFIG_NAK;
                }
            }

            /* if too many NAKs, reject instead   */
            if ( option_result == CONFIG_NAK ) 
            {
                if ( fsm_p->retry_nak > 0 )
                    fsm_p->retry_nak--;
                else
                {
                    DEBUG_LOG("ipcp_check - too many NAKs so REJECT", LEVEL_3, NOVAR, 0, 0);
                    DEBUG_ERROR("ipcp_check - too many NAKs so REJECT", NOVAR, 0, 0);
                    option_result = CONFIG_REJ;
                }
            }

            /* if NAK, set values back to origional (before this routine called)   */
            /* since NAK needs to put in values acceptable to us                   */
            if (option_result == CONFIG_NAK)
            {
                tc_mv4(ipcp_p->remote_entry.work.address, 
                       ipcp_p->remote_entry.want.address, IP_ALEN);
                if (option_p->type == IPCP_ADDRESSES)
                    tc_mv4(ipcp_p->remote_entry.work.other, 
                           ipcp_p->local_entry.want.address, IP_ALEN);
            }
            break;
#endif

        case IPCP_ADDRESS:
            /* set working addresses to requested/naked address   */
            /* side_p : REQ is remote, NAK is local               */
            tc_mv4(ip_temp, side_p->work.address, IP_ALEN);
            tc_mv4(side_p->work.address, pb, IP_ALEN);
            toss -= IP_ALEN;

            option_result = ipcp_address_check(fsm_p, request, ip_temp, 
                               option_result,
                               (PFBYTE)(ipcp_p->local_entry.want.address),
                               (PFBYTE)(ipcp_p->local_entry.work.address),
                               (PFBYTE)(ipcp_p->remote_entry.want.address),
                               (PFBYTE)(ipcp_p->remote_entry.work.address),
                               (PFBYTE)(ipcp_p->local_entry.work.other),
                               (RTIP_BOOLEAN)(side_p->will_negotiate & IPCP_ADDRESS_NEGOTIABLE));
            break;

#if (INCLUDE_PPP_VANJC)
        case IPCP_COMPRESS:
            side_p->work.compression = net2hs(*((PFWORD)pb));
            pb += 2;
            toss -= 2;

            /* Check if requested type is acceptable   */
            switch ( side_p->work.compression ) 
            {
                case PPP_COMPR_PROTOCOL:
                    if (option_result == CONFIG_REJ)
                        return(option_result);
                    test = *pb++;
                    if ( (side_p->work.slots = (byte)(test + 1)) < IPCP_SLOT_LO) 
                    {
                        side_p->work.slots = IPCP_SLOT_LO;
                        option_result = CONFIG_NAK;
                    } 
                    else if (side_p->work.slots > IPCP_SLOT_HI) 
                    {
                        side_p->work.slots = IPCP_SLOT_HI;
                        option_result = CONFIG_NAK;
                    }

                    test = *pb;
                    if ( (side_p->work.slot_compress = (byte)test) > 1 ) 
                    {
                        side_p->work.slot_compress = 1;
                        option_result = CONFIG_NAK;
                    }
                    else
                        side_p->work.slot_compress = test;
                    toss -= 2;

                    break;

                default:
                    if ( side_p->want.negotiate & IPCP_N_COMPRESS ) 
                    {
                        side_p->work.compression = side_p->want.compression;
                        side_p->work.slots = side_p->want.slots;
                        side_p->work.slot_compress = 
                            side_p->want.slot_compress;
                    } 
                    else 
                    {
                        side_p->work.compression = PPP_COMPR_PROTOCOL;
                        side_p->work.slots = IPCP_SLOT_DEFAULT;
                        side_p->work.slot_compress = IPCP_SLOT_COMPRESS;
                    }
                    option_result = CONFIG_NAK;
                    break;
            };
            break;
#endif

#if (INCLUDE_PPP_DNS)
        case IPCP_DNS_PRIMARY_SH:
            /* set working addresses to requested/naked address    */
            /* side_p : REQ is remote, NAK is local                */
            /* NOTE: save origional work value in case do not want */
            /*       to use value from request later on            */
            tc_mv4(ip_temp, side_p->work.dns1_addr, IP_ALEN);
            tc_mv4(side_p->work.dns1_addr, pb, IP_ALEN);
            toss -= IP_ALEN;

            option_result = ipcp_address_check(fsm_p, request, ip_temp, 
                               option_result,
                               (PFBYTE)(ipcp_p->local_entry.want.dns1_addr),
                               (PFBYTE)(ipcp_p->local_entry.work.dns1_addr),
                               (PFBYTE)(ipcp_p->remote_entry.want.dns1_addr),
                               (PFBYTE)(ipcp_p->remote_entry.work.dns1_addr),
                               (PFBYTE)0,
                               (RTIP_BOOLEAN)(side_p->will_negotiate & IPCP_DNS_NEGOTIABLE));
            break;

        case IPCP_DNS_SECOND_SH:
            DEBUG_LOG("ipcp_check - IPCP_DNS SECONDARY = ", LEVEL_3, IPADDR, 
                pb, 0);
            /* set working addresses to requested/naked address   */
            /* side_p : REQ is remote, NAK is local               */
            tc_mv4(ip_temp, side_p->work.dns2_addr, IP_ALEN);
            tc_mv4(side_p->work.dns2_addr, pb, IP_ALEN);
            toss -= IP_ALEN;

            option_result = ipcp_address_check(fsm_p, request, ip_temp, 
                               option_result,
                               (PFBYTE)(ipcp_p->local_entry.want.dns2_addr),
                               (PFBYTE)(ipcp_p->local_entry.work.dns2_addr),
                               (PFBYTE)(ipcp_p->remote_entry.want.dns2_addr),
                               (PFBYTE)(ipcp_p->remote_entry.work.dns2_addr),
                               (PFBYTE)0,
                               (RTIP_BOOLEAN)(side_p->will_negotiate & IPCP_DNS_NEGOTIABLE));
            break;
#endif      /* INCLUDE_PPP_DNS */

        default:
            DEBUG_LOG("ipcp_check-unknown option - reject", LEVEL_3, NOVAR, 0, 0);
            option_result = CONFIG_REJ;
            break;
    };      /* end of switch */

    /* if read more bytes than IPCP option header specified   */
    if (toss < 0)
        return(-1);

    DEBUG_LOG("after ipcp_check - remote_entry.want.dns1_addr = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.dns1_addr, 0);

    return (option_result);
}


/* ********************************************************************   */
/* check_nak_addr() - process a NAK for and IP address option             */
/*                                                                        */
/*   This routine checks it it wants to use the address in the NAK packet */
/*   for its subsequent REQUESTS.  When this routine is called, the       */
/*   address from the request was already saved in work_addr.  So, if     */
/*   the address in the request is acceptable, this routine does not      */
/*   need to do anything.                                                 */
/*                                                                        */
/*   Parameters:                                                          */
/*     ip_temp   : origional work_addr before NAK received                */
/*     want_addr : address wanted at beginning of negotiation             */
/*     work_addr : address to REQUEST which when this routine is called   */
/*                 is set to the address from the NAK.                    */
/*     will_negotiate: set to TRUE is address is negotiable               */
/*                                                                        */
/*   Returns: response to send to remote (CONFIG_REJ for example)         */
/*                                                                        */
int check_nak_addr(PFBYTE ip_temp, PFBYTE want_addr, PFBYTE work_addr, RTIP_BOOLEAN will_negotiate)
{
    DEBUG_LOG("check_nak_addr - checking NAK", LEVEL_3, NOVAR, 0, 0);

    /* override any undesirable changes which were set when came
       into this routine*/
    /* for DNS PRIMARY, if otherside NAKed our address, use the              */
    /* address they sent : this is already done before calling this          */
    /* routine by setting work.dns1_addr for example to the addresses        */
    /* in the NAK                                                            */
    /* But if they naked our request of 0, then this routine need to do some */
    /* some work, i.e. it means then they don't know address either          */
    if (!tc_cmp4(want_addr, ip_nulladdr, IP_ALEN)) 
    {
        /* we know our address -                                */
        /* if otherside NAKed with 0, they are requesting us to */
        /* send the address so restore what we know (i.e. fix   */
        /* back to values before this routine was called)       */
        if (tc_cmp4(work_addr, ip_nulladdr, IP_ALEN))
        {
            tc_mv4(work_addr, want_addr, IP_ALEN);
            DEBUG_LOG("check_nak_addr - otherside NAKed our 0 with", LEVEL_3, 
                IPADDR, want_addr, 0);
        }

        /* they NAKed with an address and we did not REQ 0s; use   */
        /* their address if the address itself is negotiable       */
        /* (i.e. when xn_want_ipcp_address was called a negotiable */
        /*       address was specified)                            */
        else if (will_negotiate) 
        {
            tc_mv4(want_addr, work_addr, IP_ALEN);
            DEBUG_LOG("check_nak_addr - otherside NAKed with (ours not 0)-its negotiable", 
                LEVEL_3, IPADDR, work_addr, 0);
        }
        else
        {
            DEBUG_LOG("check_nak_addr - otherside NAKed with (ours not 0)-not negotiable=>REJECT", 
                LEVEL_3, IPADDR, work_addr, 0);
            tc_mv4(work_addr, ip_temp, IP_ALEN);
            return(CONFIG_NAK);
        }
    }
    return(CONFIG_ACK);
}

/* ********************************************************************   */
/* process a REQUEST for an IP address                                    */
/*                                                                        */
/* returns response to send to remote (CONFIG_REJ for example)            */
/*                                                                        */
int check_req_addr(PFBYTE other, PFBYTE want_addr, PFBYTE work_addr,
                   RTIP_BOOLEAN will_negotiate)
{
int option_result = CONFIG_ACK;     /* Assume good values */

    /* REQ: check if othersides IP (from source field) matches what    */
    /*      we want                                                    */
    /* NOTE: work_addr is source field from in pkt                     */
    if (tc_cmp4(work_addr, want_addr, IP_ALEN)) 
    {
        DEBUG_LOG("check_req_addr - REQ - otherside IP matches what we want", 
            LEVEL_3, IPADDR, work_addr, 0);

        /* REQ: it matches, if both are null addresses, otherside is    */
        /*      requesting their IP address but we do not know it       */
        if (tc_cmp4(want_addr, ip_nulladdr, IP_ALEN)) 
        {
            DEBUG_LOG("check_req_addr-1-reject since both are null addresses", LEVEL_3, NOVAR, 0, 0);
            option_result = CONFIG_REJ;
        }
    } 

    /* REQ: othersides IP (from source field) does not match, accept   */
    /*      what remote wants if                                       */
    /*        - ours is null then we now know othersides IP address    */
    /*        - the address is negotiable and remote is not sending 0  */
    else if ( tc_cmp4(want_addr, (PFBYTE)ip_nulladdr, IP_ALEN) || 
                (!tc_cmp4(work_addr, (PFBYTE)ip_nulladdr, IP_ALEN) &&
                will_negotiate) )
    {
        if (other)
            tc_mv4(other, work_addr, IP_ALEN);
        DEBUG_LOG("check_req_addr - REQ - IP no match - ours is null, save it", 
            LEVEL_3, IPADDR, work_addr, 0);
    } 

    /* REQ: othersides IP (from source field) does not match and ours   */
    /*      is not null so NAK it with what we think it should be       */
    else 
    {
        DEBUG_LOG("check_req_addr - otherside IP does not match - NAK it", 
            LEVEL_3, IPADDR, work_addr, 0);
        option_result = CONFIG_NAK;
    }

    return(option_result);
}

/* ********************************************************************   */
/* process a REQUEST or NAK with IP address;                              */
/* if request is 1, we are process a CONFIG_REQ; if it is 0, we are       */
/* processing a CONFIG_NAK                                                */
/*                                                                        */
/* returns response to send to remote (CONFIG_REJ for example)            */
/*                                                                        */
int ipcp_address_check(PFSMS fsm_p, RTIP_BOOLEAN request, 
                       PFBYTE ip_temp, int option_result,
                       PFBYTE local_want_addr,
                       PFBYTE local_work_addr,
                       PFBYTE remote_want_addr,
                       PFBYTE remote_work_addr,
                       PFBYTE other_addr,
                       RTIP_BOOLEAN will_negotiate)
{
    /* if checking NAK from remote                          */
    /*       ADDRESS will accept the address in the NAK if  */
    /*          - we sent an address of 0                   */
    /*          - as long as the negotiation flag is set    */
    if (!request)
    {
        option_result = check_nak_addr(ip_temp, local_want_addr, 
                                       local_work_addr,
                                       will_negotiate);
        return(option_result);
    }

    /* ******                                           */
    /* THE FOLLOWING CODE IS FOR NAK ONLY               */
    /* Ensure that source dns1_addr field matches       */
    /* NOTE: this is really the remote address for REQ  */

    /* if rejected above return since work.address and work.other   */
    /* have been updated; i.e. reject needs to send back same       */
    /* addresses that were in the request                           */
    if (option_result == CONFIG_REJ)
    {
        DEBUG_LOG("ipcp_address_check - REJ - option result is reject", 
            LEVEL_3, NOVAR, 0, 0);
        return(CONFIG_REJ);
    }

    /* ******                             */
    /* THE FOLLOWING CODE IS FOR REQ ONLY */
    DEBUG_LOG("ipcp_address_check - checking REQ", LEVEL_3, NOVAR, 0, 0);
    option_result = check_req_addr(other_addr, remote_want_addr, 
                                   remote_work_addr,
                                   will_negotiate);

    /* ******                              */
    /* if too many NAKs, reject instead    */
    if ( option_result == CONFIG_NAK ) 
    {
        if ( fsm_p->retry_nak > 0 )
            fsm_p->retry_nak--;
        else
        {
            DEBUG_LOG("ipcp_address_check - too many NAKs so REJECT", LEVEL_3, NOVAR, 0, 0);
            DEBUG_ERROR("ipcp_address_check - too many NAKs so REJECT", NOVAR, 0, 0);
            option_result = CONFIG_REJ;
        }
    }

    /* if NAK, set values back to origional (before this routine called)   */
    /* since NAK needs to put in values acceptable to us                   */
    if (option_result == CONFIG_NAK)
    {
        tc_mv4(remote_work_addr, remote_want_addr, IP_ALEN);
    }
    return(option_result);
}

/* ********************************************************************   */
/* Check options requested by the remote host                             */
int ipcp_request(PFSMS fsm_p, PCONFIG_HDR config, DCU msg)
{
PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;
int signed_length;
DCU reply_msg;                      /* reply packet */
PFBYTE reply_pb;
byte reply_result = CONFIG_ACK;     /* cumulative reply to request - i.e.
                                       the worst reply so far as each
                                       option is processed */
word desired;                       /* desired to negotiate */
struct option_hdr option;           /* option header storage */
byte option_result;                 /* option reply */
PFBYTE pb;                          /* pointer to data area */
PFBYTE pb_pad;                      /* pointer to data area (padding area) */
int t;

    DEBUG_LOG("ipcp_request()", LEVEL_3, NOVAR, 0, 0);
    DEBUG_LOG("ipcp_request - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);

    ipcp_p->remote_entry.work.negotiate = FALSE;  /* clear flags */

    /* point to option area of the packet   */
    pb = DCUTODATA(msg) + IN_PKT_HDR_LEN;

    /* get a DCU to send reply in   */
    reply_msg = os_alloc_packet(MAX_PACKETSIZE, IPCP_REPLY_ALLOC);
    if (!reply_msg)
    {
        return -1;  /* tbd */
    }
    DCUTOPACKET(reply_msg)->length = OUT_PKT_HDR_LEN;  
                        /* leave room for config and PPP headers   */

    /* get length of options from input packet   */
    signed_length = config->len - IN_PKT_HDR_LEN;

    /* Process options requested by remote host   */
    while (signed_length > 0  && ipcp_ntohopt(&option, pb) != -1) 
    {
        pb += OPTION_HDR_LEN;

        if (signed_length < option.len) 
        {
            DEBUG_ERROR("IPCP REQ: bad header length", NOVAR, 0, 0);
            os_free_packet(reply_msg);
            return -1;
        }
        signed_length -= option.len;

        t = ipcp_check(pb, fsm_p, &(ipcp_p->remote_entry), &option, TRUE);
        option_result = (byte) t;

        if (t == -1)
        {
            DEBUG_ERROR("IPCP REQ: ran out of data", NOVAR, 0, 0);
            os_free_packet(reply_msg);
            return -1;
        }

        /* set bp_pad to point to padding area (if any) and   */
        /* set pb to point to next option                     */
        /* NOTE: header already accounted for                 */
        pb_pad = pb;
        pb += option.len - OPTION_HDR_LEN;

        /* check if overran packet length as specified by DCU   */
        if ( pb > (DCUTODATA(msg) + DCUTOPACKET(msg)->length) )
        {
            os_free_packet(reply_msg);
            return(-1);
        }
            
        /* if result of last option is not as bad as options processed   */
        /* thus far go to next option; i.e. NAK is not as bad as REJ     */
        /* and ACK is not as bad as NAK                                  */
        if (option_result < reply_result) 
        {
            continue;
        } 

        /* if result of last option is worse than as options processed   */
        /* thus far discard options in reply                             */
        else if ( option_result > reply_result ) 
        {
            /* Discard current list of replies   */
            DCUTOPACKET(reply_msg)->length = OUT_PKT_HDR_LEN;
            reply_result = option_result;
        }

        /* remember that we processed option   */
        if ( (option_result != CONFIG_REJ) && 
             (option.type <= IPCP_OPTION_LIMIT) ) 
        {
            ipcp_p->remote_entry.work.negotiate = 
                (word)(ipcp_p->remote_entry.work.negotiate |  
                       (1 << option.type));
        }

        /* Add option response to the return list   */
        /* put option into reply                    */
        if ( ipcp_option_supported(option.type) && 
             (option_result != CONFIG_REJ) )
        {
            pb_pad += ipcp_option_length[option.type] - OPTION_HDR_LEN;    

            ipcp_option(reply_msg, &(ipcp_p->remote_entry.work),
                option.type, option.len, pb_pad);
        }  

        /* for illegal options, when putting option into reply, do a   */
        /* straight copy                                               */
        else
        {
            /* if not enough room to put the option, don't do it   */
            if ( (DCUTOPACKET(reply_msg)->length + option.len) <= 
                 MAX_PACKETSIZE )
            {
                reply_pb = DCUTODATA(reply_msg) + 
                           DCUTOPACKET(reply_msg)->length;
                tc_movebytes(reply_pb, pb - option.len, option.len);
                DCUTOPACKET(reply_msg)->length += (word)option.len;
            }
        }
    }

    /* Now check for any missing options which are desired   */
    if ( (desired = (word)(ipcp_p->remote_entry.want.negotiate &
        ~ipcp_p->remote_entry.work.negotiate)) != 0 ) 
    {
        switch ( reply_result ) 
        {
            case CONFIG_ACK:
                DCUTOPACKET(reply_msg)->length = OUT_PKT_HDR_LEN;    
                /* leave room for config header   */
                reply_result = CONFIG_NAK;
                /* fallthru   */
            case CONFIG_NAK:
                ipcp_makeoptions(reply_msg, (PIPCPVALUES)&(ipcp_p->remote_entry.want), desired);
                fsm_p->retry_nak--;
                break;
            case CONFIG_REJ:
                /* do nothing   */
                break;
        };
    } 

    /* Send ACK/NAK/REJ to remote host   */
    fsm_send(fsm_p, reply_result, config->id, reply_msg);
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */
    return (reply_result != CONFIG_ACK);
}



/* ********************************************************************   */
/* Process configuration NAK sent by remote host                          */
int ipcp_nak(PFSMS fsm_p, PCONFIG_HDR config, DCU msg)
{
PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;
PIPCPSIDES local_p = (PIPCPSIDES)&(ipcp_p->local_entry);
int signed_length;
struct option_hdr option;
int last_option = 0;
int result;
PFBYTE pb;

    DEBUG_LOG("ipcp_nak()", LEVEL_3, NOVAR, 0, 0);
    DEBUG_LOG("ipcp_nak - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);

    /* ID field must match last request we sent   */
    if (config->id != fsm_p->lastid) 
    {
        DEBUG_ERROR("IPCP NAK: wrong ID", NOVAR, 0, 0);
        return -1;
    }

    /* point to option area of the input packet   */
    pb = DCUTODATA(msg);
    pb += IN_PKT_HDR_LEN;

    /* get length of options   */
    signed_length = config->len - IN_PKT_HDR_LEN;
    DEBUG_LOG("ipcp_nak - len of options = ", LEVEL_3, DINT1, signed_length, 0);

    /* First, process in order.  Then, process extra "important" options   */
    while (signed_length > 0 && ipcp_ntohopt(&option, pb) != -1) 
    {
        pb += OPTION_HDR_LEN;

        if (signed_length < option.len) 
        {
            DEBUG_ERROR("IPCP NAK: bad header length", NOVAR, 0, 0);
            return -1;
        }
        signed_length -= option.len;

        if ( option.type > IPCP_OPTION_LIMIT ) 
        {
            DEBUG_ERROR("IPCP NAK: option out of range", NOVAR, 0, 0);
        } 

        /* if options are not in ascending order or a NAK for this       */
        /* option is not being negotiated (i.e. was not sent in request) */
        else if ( option.type < last_option ||
            !(local_p->work.negotiate & (1 << option.type)) ) 
        {
            if (local_p->work.negotiate & (1 << option.type)) 
            {
                DEBUG_ERROR("IPCP NAK: option out of order", NOVAR, 0, 0);
                return -1;      /* was requested */
            }
            /* got a NAK so make this option negotiable so will send   */
            /* it back in the reply                                    */
            local_p->work.negotiate |= (word)(1 << option.type);
            last_option = IPCP_OPTION_LIMIT + 1;
        } 
        else 
        {
            last_option = option.type;
        }

        if ( (result = ipcp_check(pb, fsm_p, local_p, &option, FALSE)) == -1 ) 
        {
            DEBUG_ERROR("IPCP NAK: ran out of data", NOVAR, 0, 0);
            return(result);
        }

        /* point past the current option; amount to bypass is based   */
        /* upon option length specified in packet                     */
        pb += option.len - OPTION_HDR_LEN;

        /* check if overran packet length as specified by DCU   */
        DEBUG_LOG("ipcp_nak - check overrun:pb,msg+len", LEVEL_3, DINT2, 
            (dword)pb, (dword)(DCUTODATA(msg) + DCUTOPACKET(msg)->length));
        if ( pb > (DCUTODATA(msg) + DCUTOPACKET(msg)->length) )
        {
            DEBUG_LOG("ipcp_nak - overran packet length", LEVEL_3, EBS_INT1, 
                DCUTOPACKET(msg)->length, 0);
            return(-1);
        }

    }
    DEBUG_LOG("IPCP NAK: valid", LEVEL_3, NOVAR, 0, 0);
    return 0;
}


/* ********************************************************************    */
/* PROCESS ACK                                                             */
/* ********************************************************************    */
/* Process configuration ACK sent by remote host                           */
int ipcp_ack(PFSMS fsm_p, PCONFIG_HDR config, DCU msg)
{
DCU req_msg;
int error = FALSE;
PFBYTE pb, req_pb;
int signed_length;

    DEBUG_LOG("ipcp_ack() - process ack", LEVEL_3, NOVAR, 0, 0);

    /* point to options area in packet received   */
    pb = DCUTODATA(msg);
    pb += IN_PKT_HDR_LEN;

    /* ID field must match last request we sent   */
    if (config->id != fsm_p->lastid) 
    {
        DEBUG_ERROR("IPCP ACK: wrong ID", NOVAR, 0, 0);
        return -1;
    }

    /* Get a copy of last request we sent   */
    req_msg = ipcp_makereq(fsm_p);
    if (!req_msg)
    {
        DEBUG_ERROR("LCP ACK: out of packets", NOVAR, 0, 0);
        return -1;
    }

    /* Overall buffer length should match         */
    /* NOTE: length includes configuration header */
    if ((int)config->len != DCUTOPACKET(req_msg)->length - PPP_HDR_LEN) 
    {
        DEBUG_ERROR("IPCP ACK: buffer length mismatch", NOVAR, 0, 0);
        error = TRUE;
    } 
    else 
    {
        /* point to options area of last request we sent   */
        req_pb = DCUTODATA(req_msg) + OUT_PKT_HDR_LEN;

        /* get length of options of ACK   */
        signed_length = config->len - IN_PKT_HDR_LEN;

        /* Each byte should match   */
        while (signed_length--) 
        {
            DEBUG_LOG("IPCP ACK: check ACK data, exp,act = ", LEVEL_3, EBS_INT2,  
                *pb, *req_pb);
            if (*pb++ != *req_pb++) 
            {
                DEBUG_ERROR("IPCP ACK: data mismatch", NOVAR, 0, 0);
                error = TRUE;
                /* break;    tbd - debug purposes   */
            }
        }
    }
    os_free_packet(req_msg);

    if (error) 
    {
        return -1;
    }

    DEBUG_LOG("IPCP ACK: valid", LEVEL_3, NOVAR, 0, 0);
    return 0;
}

/* ********************************************************************    */
/* PROCESS REJECT                                                          */
/* ********************************************************************    */
/* Process configuration reject sent by remote host                        */
int ipcp_reject(PFSMS fsm_p, PCONFIG_HDR config, DCU msg)
{
PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;
PIPCPSIDES local_p = (PIPCPSIDES)&(ipcp_p->local_entry);
int signed_length;
struct option_hdr option;
int last_option = 0;
PFBYTE pb;

    DEBUG_LOG("ipcp_reject()", LEVEL_3, NOVAR, 0, 0);
    DEBUG_LOG("ipcp_reject - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);

    /* ID field must match last request we sent   */
    if (config->id != fsm_p->lastid) 
    {
        DEBUG_ERROR("IPCP REJ: wrong ID", NOVAR, 0, 0);
        return -1;
    }

    /* point to option area of the packet   */
    pb = DCUTODATA(msg) + IN_PKT_HDR_LEN;

    /* get length of options   */
    signed_length = config->len - IN_PKT_HDR_LEN;

    /* Process in order, checking for errors   */
    while (signed_length > 0 && ipcp_ntohopt(&option, pb) != -1) 
    {
        if (signed_length < option.len)  
        {
            DEBUG_ERROR("IPCP REJ: bad header length", NOVAR, 0, 0);
            return -1;
        }
        signed_length -= option.len;

        if ( option.type > IPCP_OPTION_LIMIT ) 
        {
            DEBUG_ERROR("IPCP REJ: option out of range", NOVAR, 0, 0);
        } 
        else if (option.type < last_option ||
            !(local_p->work.negotiate & (1 << option.type))) 
        {
            DEBUG_ERROR("IPCP REJ: option out of order or not negotiating", NOVAR, 0, 0);
            return -1;
        }

        /* move pointer past data area of option and check to make     */
        /* sure did not overrun packet                                 */
        /* NOTE: currently it points beginning of header of the option */
        pb += option.len;   /* NOTE: len include header */
        if (pb > (DCUTODATA(msg) + DCUTOPACKET(msg)->length))
        {
            DEBUG_ERROR("IPCP REJ: ran out of data", NOVAR, 0, 0);
            return -1;
        }

        last_option = option.type;

        /* tbd - need to check if option matches one sent in req   */

        /* drop the option since it was rejected   */
        if ( option.type <= IPCP_OPTION_LIMIT ) 
        {
            local_p->work.negotiate &= (word)(~(1 << option.type));

#if (INCLUDE_PPP_IPCP_ADDRESSES)
            /* if address was rejected try addresses   */
            if (option.type == IPCP_ADDRESS) 
            {
                local_p->work.negotiate |= (1 << IPCP_ADDRESSES);
                DEBUG_ERROR("reject: turned on ADDRESSES", NOVAR, 0, 0);
            }
#endif
        }
    }
    DEBUG_LOG("IPCP REJ: valid", LEVEL_3, NOVAR, 0, 0);
    return 0;
}


/* ********************************************************************    */
/*          I N I T I A L I Z A T I O N                                    */
/* ********************************************************************    */

/* Reset configuration options before request   */
void ipcp_reset(PFSMS fsm_p)
{
PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;

    DEBUG_LOG("ipcp_reset()", LEVEL_3, NOVAR, 0, 0);
    DEBUG_LOG("ipcp_reset - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);

    ASSIGN( ipcp_p->local_entry.work, ipcp_p->local_entry.want );
    tc_mv4(ipcp_p->local_entry.work.other, 
           ipcp_p->remote_entry.want.address, IP_ALEN);

    ipcp_p->remote_entry.work.negotiate = FALSE;

    DEBUG_LOG("after ipcp_reset - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);
}


/* ********************************************************************   */
/* After termination                                                      */
void ipcp_stopping(PFSMS fsm_p)
{
    ARGSUSED_PVOID(fsm_p);

    DEBUG_LOG("ipcp_stopping()", LEVEL_3, NOVAR, 0, 0);
}


/* ********************************************************************   */
/* Close IPCP                                                             */
void ipcp_closing(PFSMS fsm_p)
{
#if (INCLUDE_PPP_VANJC)
    PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;

    DEBUG_LOG("ipcp_closing - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);
#endif

    ARGSUSED_PVOID(fsm_p);

    /* free old slhc configuration, if any   */
#if (INCLUDE_PPP_VANJC)
    slhc_free( ipcp_p->slhcp );
    ipcp_p->slhcp = (PSLCOMPRESS)0;
#endif
}


/* ********************************************************************   */
/* configuration negotiation complete                                     */
void ipcp_opening(PFSMS fsm_p)
{
PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;
PIFACE pi =     fsm_p->ppp_p->iface;
#if (INCLUDE_PPP_VANJC)
    int rslots = 0;
    int tslots = 0;
#endif
#if (INCLUDE_PPP_DNS)
    dword srv_list[2];
    int srv_cnt;
#endif

    DEBUG_LOG("ipcp_opening - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);

    /* Set local and remote IP address to reflect negotiated option      */
    /* NOTE: this will override attach but only if attached with NULL    */
    /* NOTE: his_ip_addr needs to set by xn_attach() before set_ip()     */
    if ( !tc_cmp4(ipcp_p->remote_entry.work.address, 
                  pi->addr.his_ip_addr, IP_ALEN) ||
        !tc_cmp4(ipcp_p->local_entry.work.address,  
                 pi->addr.my_ip_addr,  IP_ALEN) )
    {
        /* make sure we received othersides IP address before we update     */
        /* NOTE: they might have sent IPCP REQ without ADDRESS or ADDRESSES */
        /*       option                                                     */
        if ( !tc_cmp4(ipcp_p->remote_entry.work.address, ip_nulladdr, IP_ALEN) )
        {
            /* if addr valid, set_ip was called previously, so delete   */
            /* any entries in routing table from the first call         */
            /* NOTE: this is normally done by set_ip() but it needs     */
            /*       to be done before modifying his_ip_addr in order   */
            /*       to delete the correct entry                        */
            if (pi->addr.iface_flags & IP_ADDR_VALID)
                rt_del_from_iface(pi);

            tc_mv4(pi->addr.his_ip_addr, ipcp_p->remote_entry.work.address, 
                   IP_ALEN);
        }

        /* make sure we have our IP address before we update; else just   */
        /* use one already set from set_ip()                              */
        if ( !tc_cmp4(ipcp_p->local_entry.work.address, ip_nulladdr, IP_ALEN) )
            set_ip(pi->ctrl.index, ipcp_p->local_entry.work.address, 
                   (PFCBYTE)0);   /* leave mask as is */
        else
            set_ip(pi->ctrl.index, pi->addr.my_ip_addr, (PFCBYTE)0);   /* leave mask as is */

    }
#if (INCLUDE_PPP_DNS)
    /* if DNS server was negotiated, then set up DNS servers   */
    srv_cnt = 0;
    srv_list[0] = 0;
    srv_list[1] = 0;
    if ( !tc_cmp4(ipcp_p->local_entry.work.dns1_addr, 
                  (PFBYTE)&(server_ip_table[0]), IP_ALEN) ||
         !tc_cmp4(ipcp_p->local_entry.work.dns2_addr, 
                  (PFBYTE)&(server_ip_table[1]), IP_ALEN) )
    {
        srv_cnt++;
        tc_mv4((PFBYTE)&(srv_list[0]),
               ipcp_p->local_entry.work.dns1_addr, IP_ALEN);
        if ( !tc_cmp4(ipcp_p->local_entry.work.dns2_addr, 
                      (PFBYTE)&(server_ip_table[1]), IP_ALEN) )
        {
            srv_cnt++;
            tc_mv4((PFBYTE)&(srv_list[1]),
                   ipcp_p->local_entry.work.dns2_addr, IP_ALEN);
        }

        DEBUG_ERROR("ADD DNS SERVER: ", IPADDR, (PFBYTE)&(srv_list[0]), 0);
        DEBUG_ASSERT(srv_cnt != 2, "ADD DNS SERVER: ", IPADDR, 
            (PFBYTE)&(srv_list[1]), 0);

        if (xn_set_server_list(srv_list, srv_cnt) != 0)
        {
            DEBUG_ERROR("ipcp_opening: xn_set_server_list failed", NOVAR, 0, 0);
        }
    }
#endif

    DEBUG_LOG("after ipcp_opening - his_ip_addr = ", LEVEL_3, IPADDR,
        pi->addr.his_ip_addr, 0);
    DEBUG_LOG("after ipcp_opening - my_ip_addr = ", LEVEL_3, IPADDR,
        pi->addr.my_ip_addr, 0);

#if (INCLUDE_PPP_VANJC)
    /* free old slhc configuration, if any   */
    slhc_free( ipcp_p->slhcp );
    ipcp_p->slhcp = (PSLCOMPRESS)0;

    if (ipcp_p->local_entry.work.negotiate & IPCP_N_COMPRESS) 
    {
        rslots = ipcp_p->local_entry.work.slots;
    }
    if (ipcp_p->remote_entry.work.negotiate & IPCP_N_COMPRESS) 
    {
        tslots = ipcp_p->remote_entry.work.slots;
    }

    if ( rslots != 0 || tslots != 0 ) 
    {
        ipcp_p->slhcp = slhc_init( pi, rslots, tslots );
    }
#endif

#if (DEBUG_SIGNAL)
    DEBUG_ERROR("ipcp_opening: set signal", NOVAR, 0, 0);
    ppp_signal_set();
#endif

    /* signal xn_lcp_open since pap authentication and ipcp config exchange   */
    /* is done                                                                */
    pi->ctrl.signal_status = PPP_SIGNAL_SUCCESS;
    OS_SET_PPP_SIGNAL(pi);
}


#if (PEER_POOL)
/* ********************************************************************   */
/* Check the address against all other assigned addresses                 */
static void ipcp_addr_idle(int iface, PFBYTE addr, PFBYTE nextaddr)
{
PIFACE pi;
int    iface_off;

    tc_mv4(nextaddr, addr, IP_ALEN);

    if (CFG_NIFACES > 1)
    {
        /* Check if peer IP address is already in use on another interface   */
        /* !!! need to look at *remote* address, not local!                  */
        for (iface_off = 0; iface_off < CFG_NIFACES; iface_off++) 
        {
            /* if same interface; remote address is recalculated, therefore,   */
            /* it is ok it if is the same                                      */
            if (iface_off == iface)
                continue;

            /* get interface structure; don't care if open and don't report   */
            /* any errors                                                     */
    /*      PI_FROM_OFF(pi, iface_off)                                        */
            pi = tc_ino2_iface(iface_off, DONT_SET_ERRNO);

            if (!pi || !(pi->addr.iface_flags & IP_ADDR_VALID))
                continue;

            if (tc_cmp4(pi->addr.his_ip_addr, addr, IP_ALEN))
            {
                tc_mv4(nextaddr, (PFBYTE)ip_nulladdr, IP_ALEN);
                return;
            }
        }       /* end of for loop */
    }
}

/* ********************************************************************   */
/* subtract 2 addresses and return the result as a dword                  */
dword ip_addr_sub(PFBYTE addr1, PFBYTE addr2)
{
dword temp1, temp2;

    tc_mv4((PFBYTE)&temp1, addr1, IP_ALEN);
    temp1 = net2hl(temp1);

    tc_mv4((PFBYTE)&temp2, addr2, IP_ALEN);
    temp2 = net2hl(temp2);

    return(temp1 - temp2);
}

/* add 1 to an IP address, the address addr is modified in place   */
void ip_addr_add_one(PFBYTE addr)
{
dword temp;

    tc_mv4((PFBYTE)&temp, addr, IP_ALEN);
    temp = net2hl(temp);
    temp++;
    temp = hl2net(temp);
    tc_mv4(addr, (PFBYTE)&temp, IP_ALEN);
}

/* check if the current address in within range of the pool; return   */
/* TRUE if it is                                                      */
RTIP_BOOLEAN check_range(PIPCPS ipcp_p)
{
dword ip_curr;
dword ip_min;
dword ip_max;

    tc_mv4((PFBYTE)&ip_curr, ipcp_p->peer_curr, IP_ALEN);
    ip_curr = net2hl(ip_curr);

    tc_mv4((PFBYTE)&ip_min, ipcp_p->peer_min, IP_ALEN);
    ip_min = net2hl(ip_min);

    tc_mv4((PFBYTE)&ip_max, ipcp_p->peer_max, IP_ALEN);
    ip_max = net2hl(ip_max);

    if (ip_curr < ip_min ||ip_curr > ip_max)
        return(FALSE);
    else
        return(TRUE);
}

/* Assign the next unused address from a pool to nextaddr; if
   all the addresses in the pool are being used, nextaddr
   will be set to ipnulladdr w*/
void ipcp_poolnext(int iface, PIPCPS ipcp_p, PFBYTE nextaddr)
{
dword i;

    /* get number of addresses in pool   */
    i = 1L + ip_addr_sub(ipcp_p->peer_max, ipcp_p->peer_min);

    /* clear return value   */
    tc_mv4(nextaddr, (PFBYTE)ip_nulladdr, IP_ALEN);

    /* loop until nextaddr (return value) is found, i.e. an unused   */
    /* address from the pool                                         */
    while ( i-- > 0  && tc_cmp4(nextaddr, (PFBYTE)ip_nulladdr, IP_ALEN) ) 
    {
        /* if address in not in range of pool, start again with min address   */
        if (!check_range(ipcp_p))
            tc_mv4(ipcp_p->peer_curr, ipcp_p->peer_min, IP_ALEN);

        /* set nextaddr back to 0 if address is in use   */
        ipcp_addr_idle(iface, ipcp_p->peer_curr, nextaddr);

        /* increment current address by 1   */
        ip_addr_add_one(ipcp_p->peer_curr);
    }
}

#endif

/* ********************************************************************   */
/* Prepare to begin configuration exchange                                */
void    ipcp_starting(PFSMS fsm_p)
{
PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;
#if (PEER_POOL)
    byte addr[IP_ALEN];
#endif

    DEBUG_LOG("ipcp_starting()", LEVEL_3, NOVAR, 0, 0);
    DEBUG_LOG("ipcp_starting - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);
    DEBUG_LOG("ipcp_starting - his_ip_addr = ", LEVEL_3, IPADDR,
        fsm_p->ppp_p->iface->addr.his_ip_addr, 0);

#if (PEER_POOL)
    /* If there is a POOL available, get next address from PPP pool;
       pool will be set by both xp_want_ipcp_pool and xn_want_ipcp_address */
    /*  if ( tc_cmp4(ipcp_p->remote_entry.want.address, ip_nulladdr, IP_ALEN) &&    */
    /*       !(tc_cmp4(ipcp_p->peer_min, ip_nulladdr, IP_ALEN)) )                   */
    if (!tc_cmp4(ipcp_p->peer_min, ip_nulladdr, IP_ALEN)) 
    {
        ipcp_poolnext(fsm_p->ppp_p->iface->ctrl.index, ipcp_p, (PFBYTE)addr);
        tc_mv4(ipcp_p->remote_entry.want.address, (PFBYTE)addr, IP_ALEN);
    }
#endif

    /* set local and remote address based upon set_ip and xn_attach          */
    /* only if xn_want_ipcp_address was not called to set up desired address */
    if (!(ipcp_p->local_entry.want.negotiate & IPCP_N_ADDRESS))
    {
        tc_mv4(ipcp_p->local_entry.want.address, 
               fsm_p->ppp_p->iface->addr.my_ip_addr, IP_ALEN);
    }
    if (!(ipcp_p->remote_entry.want.negotiate & IPCP_N_ADDRESS))
    {
        tc_mv4(ipcp_p->remote_entry.want.address, 
               fsm_p->ppp_p->iface->addr.his_ip_addr, IP_ALEN);
    }

#if (INCLUDE_PPP_DNS)
    /* set dns addresses based upon xn_set_server_list                      */
    /* only if xn_want_dns_address was not called to set up desired address */
    if (!(ipcp_p->local_entry.want.negotiate & IPCP_N_DNS_PRIMARY))
    {
        if (server_ip_table[0] != 0L)
        {
            tc_mv4(ipcp_p->local_entry.want.dns1_addr, 
                   (PFBYTE)&(server_ip_table[0]), IP_ALEN);
            if (server_ip_table[1] != 0L)
            {
                tc_mv4(ipcp_p->local_entry.want.dns2_addr, 
                       (PFBYTE)&(server_ip_table[1]), IP_ALEN);
            }
        }
    }
#endif

    DEBUG_LOG("after ipcp_starting - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);
    DEBUG_LOG("after ipcp_starting - his_ip_addr = ", LEVEL_3, IPADDR,
        fsm_p->ppp_p->iface->addr.his_ip_addr, 0);
}


/* ********************************************************************   */
void    ipcp_free(PFSMS fsm_p)
{
#if (INCLUDE_PPP_VANJC)
PIPCPS ipcp_p = (PIPCPS)fsm_p->pdv;

    DEBUG_LOG("ipcp_free - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);

    slhc_free( ipcp_p->slhcp );
#else
    ARGSUSED_PVOID(fsm_p);
#endif
}

/* Allocate configuration structure - called when interface is opened   */
RTIP_BOOLEAN ipcp_open(PPPPS ppp_p)
{
PFSMS fsm_p = &(ppp_p->fsm[IPcp]);
PIFACE pi;

    pi = ppp_p->iface;

    fsm_p->ppp_p = ppp_p;
    fsm_p->pdc = (PFSM_CONSTS)&ipcp_constants;
    fsm_p->pdv = os_alloc_ipcp_cb(pi);

    if (fsm_p->pdv)
        return(TRUE);
    else
        return(FALSE);
}

/* Initialize configuration structure - called when PPP is initialized
   (xn_ppp_init) */
void ipcp_init(PPPPS ppp_p)
{
PFSMS fsm_p = &(ppp_p->fsm[IPcp]);
PIPCPS ipcp_p;

    DEBUG_LOG("ipcp_init()", LEVEL_3, NOVAR, 0, 0);

    ipcp_p = (PIPCPS)fsm_p->pdv;

    /* Set option parameters to first request defaults   */
    ASSIGN( ipcp_p->local_entry.want, ipcp_default );
    ASSIGN( ipcp_p->local_entry.work, ipcp_default );
    ipcp_p->local_entry.will_negotiate = ipcp_negotiate;

    ASSIGN( ipcp_p->remote_entry.want, ipcp_default );
    ASSIGN( ipcp_p->remote_entry.work, ipcp_default );
    ipcp_p->remote_entry.will_negotiate = ipcp_negotiate;

    tc_mv4(ipcp_p->peer_min, ip_nulladdr, IP_ALEN);

    DEBUG_LOG("after ipcp_init 1 - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);

    fsm_init(fsm_p);
    DEBUG_LOG("after ipcp_init 2 - remote_entry.want.address = ", LEVEL_3, IPADDR,
        ipcp_p->remote_entry.want.address, 0);
}


#endif  /* INCLUDE_PPP */

