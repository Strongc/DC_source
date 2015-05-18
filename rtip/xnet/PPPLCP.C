/*                                                                       */
/*  PPPLCP.C    -- negotiate data link options                           */
/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */
/*  Jan 91  Bill_Simpson@um.cc.umich.edu                                 */
/*      Computer Systems Consulting Services                             */
/*                                                                       */
/*  Acknowledgements and correction history may be found in PPP.C        */
/*                                                                       */

#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP


#include "sock.h"
#include "rtip.h"

#if (INCLUDE_PPP)

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_AUTH   0

/***********************************************************************  */
/* local functions                                                        */

static void lcp_option(DCU msg, PLCPVALUES value_p, byte o_type,
                       byte o_length, PFBYTE copy_pb );
static void lcp_makeoptions(DCU msg, PLCPVALUES value_p, word negotiating);
DCU lcp_makereq(PFSMS fsm_p);

static int lcp_check(PFBYTE pb, PLCPS lcp_p, struct lcp_side_s KS_FAR *side_p, 
                     POPTION_HDR option_p, RTIP_BOOLEAN request);
static byte get_option_len(byte o_type, word auth_type);
static void lcp_setup_next_auth(struct lcp_side_s KS_FAR *side_p);
static RTIP_BOOLEAN check_lcp_auth_want(struct lcp_side_s KS_FAR *side_p, PFBYTE pb);

/***********************************************************************  */
/* default values for LCP options                                         */
/* these defaults are defined in the PPP RFCs, and must not be changed    */
PPP_EXTERN_GLOBAL_CONSTANT struct lcp_value_s KS_FAR lcp_default;  
KS_EXTERN_GLOBAL_CONSTANT word KS_FAR lcp_negotiate;

KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR lcp_option_length[LCP_OPTION_LIMIT+1];

/* LCP constants - contains values as well as routines to call when   */
/*                 events occur                                       */
PPP_EXTERN_GLOBAL_CONSTANT struct fsm_constant_s KS_FAR lcp_constants; 


/***********************************************************************   */
/* EVENT PROCESSING                                                        */
/***********************************************************************   */

/* REQUEST uses LOCAL=remote must authenticate with   */

static void lcp_option(DCU msg, PLCPVALUES value_p, byte o_type, 
                       byte o_length, PFBYTE copy_pb)
{
PFBYTE cp;
int toss;

    toss = o_length - OPTION_HDR_LEN;

    /* if not enough room to put the option   */
    if ( (DCUTOPACKET(msg)->length + o_length) > MAX_PACKETSIZE )
    {
        DEBUG_ERROR("lcp_option: not enough room in DCU for option", NOVAR,
            0, 0);
        return;   
    }

    /* point to end of packet so can append option   */
    cp = DCUTODATA(msg) + DCUTOPACKET(msg)->length;

    /* set up option header    */
    *cp++ = o_type;
    *cp++ = o_length;

    switch (o_type) 
    {
    case LCP_MRU:
        *((PFWORD)cp) = hs2net(value_p->mru);
        DEBUG_LOG("lcp_option - MRU ", LEVEL_3, EBS_INT1, value_p->mru, 0);
        toss -= 2;
        break;

    case LCP_ACCM:
        LONG_2_WARRAY((PFWORD)cp, hl2net(value_p->accm));
        DEBUG_LOG("lcp_option - ACCM ", LEVEL_3, EBS_INT1, value_p->accm, 0);
        toss -= 4;
        break;

    case LCP_AUTHENT:
        *((PFWORD)cp) = hs2net(value_p->authentication);
        cp += 2;
        DEBUG_LOG("lcp_option - AUTHENT ", LEVEL_3, EBS_INT1, 
            value_p->authentication, 0);
        toss -= 2;
#if (INCLUDE_CHAP)
        if (value_p->authentication == PPP_CHAP_PROTOCOL)
        {
            *cp = (byte)(value_p->chap_algorithm);
            toss--;
        }
#endif
        break;

    case LCP_MAGIC:
        DEBUG_LOG("lcp_option - MAGIC ", LEVEL_3, DINT1, 
            value_p->magic_number, 0);
        if (value_p->magic_number)
        {
            LONG_2_WARRAY((PFWORD)cp, hl2net(value_p->magic_number));
            toss -= 4;
        }
        else
            o_length = 0;
        break;

    case LCP_PFC:
        DEBUG_LOG("lcp_option - PFC ", LEVEL_3, NOVAR, 0, 0);
        break;

    case LCP_ACFC:
        DEBUG_LOG("lcp_option - ACFC", LEVEL_3, NOVAR, 0, 0);
        break;

    case LCP_ENCRYPT:       /* not implemented */
    case LCP_QUALITY:       /* not implemented */
    default:
        DEBUG_LOG("lcp_option - ENCRYPT or QUALITY", LEVEL_3, NOVAR, 0, 0);
        break;
    };

    /* if padding data specifed and there is room for it, copy specified    */
    /* padding to packet                                                    */
    if (o_length && copy_pb)
    {
        cp += o_length - OPTION_HDR_LEN;    
                                /* point past bytes added above   */
        while ( toss-- > 0 ) 
            *cp++ = *copy_pb++;
    }

    DCUTOPACKET(msg)->length += o_length;
}


/***********************************************************************  */
/* Build a list of options - tacking them onto the end of msg             */
static void lcp_makeoptions(DCU msg, PLCPVALUES value_p, word negotiating)
{
register byte o_type;

    DEBUG_LOG("lcp_makeoptions(): negotiating = ", LEVEL_3, 
        EBS_INT1, negotiating, 0);

    for ( o_type = 1; o_type <= LCP_OPTION_LIMIT; o_type++ ) 
    {
        if (negotiating & (1 << o_type)) 
        {
            lcp_option(msg, value_p, o_type, 
                       get_option_len(o_type, value_p->authentication), 
                       (PFBYTE)0);
        }
    }
}


/***********************************************************************  */
/* Build a request to send to remote host                                 */
/* NOTE: caller must free request when done with it                       */
DCU lcp_makereq(PFSMS fsm_p)
{
PLCPS lcp_p = (PLCPS)fsm_p->pdv;
DCU msg;

    DEBUG_LOG("lcp_makereq()", LEVEL_3, NOVAR, 0, 0);

    /* allocate a packet used to build options up in                       */
    /* NOTE: don't know size of packet before building packet so alloc max */
    msg = os_alloc_packet(MAX_PACKETSIZE, LCP_MR_ALLOC);
    if (!msg)
        return(msg);

    /* leave room for ppp and config header headers   */
    DCUTOPACKET(msg)->length = OUT_PKT_HDR_LEN;
    lcp_makeoptions(msg, &(lcp_p->local_entry.work), 
                    lcp_p->local_entry.work.negotiate);
    return(msg);
}


/***********************************************************************  */
/* Check the options, updating the working values.  Called in response
 * to receiveing a request (side_p=remote_entry) or a NAK(side_p=local_entry);
 * Returns -1 if ran out of data, ACK/NAK/REJ as appropriate.
*/
static int lcp_check(PFBYTE pb, PLCPS lcp_p, struct lcp_side_s KS_FAR *side_p, 
                     POPTION_HDR option_p, RTIP_BOOLEAN request)
{
int toss = option_p->len - OPTION_HDR_LEN;
int option_result = CONFIG_ACK;     /* Assume good values */

    /* if invalid option or the option is not negotiable    */
    if ( (option_p->type <= 0) || (option_p->type > LCP_OPTION_LIMIT) ||
         !(side_p->will_negotiate & (1 << option_p->type))) 
    {
        DEBUG_LOG("invalid option or not neg", LEVEL_3, EBS_INT1, 
            side_p->will_negotiate, 0);

        /* don't return yet since need to save opt value for REJECT   */
        /* reply                                                      */
        option_result = CONFIG_REJ;
    }

    switch (option_p->type) 
    {
    case LCP_MRU:
        side_p->work.mru = net2hs(*((PFWORD)pb));
        DEBUG_LOG("lcp_check - MRU", LEVEL_3, EBS_INT1, side_p->work.mru, 0);
        toss -= 2;

        if (option_result == CONFIG_REJ)
            break;

        /* Check if new value is appropriate   */
        if (side_p->work.mru < CFG_LCP_MRU_LO) 
        {
            side_p->work.mru = CFG_LCP_MRU_LO;
            option_result = CONFIG_NAK;
        } 
        else if (side_p->work.mru > CFG_LCP_MRU_HI) 
        {
            side_p->work.mru = CFG_LCP_MRU_HI;
            option_result = CONFIG_NAK;
        }

        /* if remote side wants higher value then we             */
        /* do then use value from remote unless it is negotiable */
        if (side_p->want.negotiate & LCP_N_MRU) 
        {
            /* only accept if it is negotiable or the our value is less;   */
            /* NAK if it is not negotiable and value requested is more     */
            if ( !(side_p->will_negotiate & LCP_MRU_NEGOTIABLE) &&
                 side_p->work.mru > side_p->want.mru ) 
            {
                side_p->work.mru = side_p->want.mru;    /* set back to orig */
                option_result = CONFIG_NAK;
            }

            /* if request and not negotiable; try NAK value at least once   */
            if (request && !(side_p->will_negotiate & LCP_MRU_NAK_TRIED))
            {
                side_p->work.mru = side_p->want.mru;    /* set back to orig */
                option_result = CONFIG_NAK;
                side_p->will_negotiate |= LCP_MRU_NAK_TRIED;
            }
        }
        break;

    case LCP_ACCM:
        side_p->work.accm = net2hl(WARRAY_2_LONG((PFWORD)pb));
        DEBUG_LOG("lcp_check - ACCM", LEVEL_3, DINT1, side_p->work.accm, 0);
        toss -= 4;

        if (option_result == CONFIG_REJ)
            break;

        /* Remote host may ask to escape more control     */
        /* characters than we require, but must escape    */
        /* at least the control chars that we require.    */
        if ( (!request || (side_p->want.negotiate & LCP_N_ACCM)) &&
             side_p->work.accm != (side_p->work.accm | side_p->want.accm) ) 
        {
            side_p->work.accm |= side_p->want.accm;
            option_result = CONFIG_NAK;
        }
        break;

    case LCP_AUTHENT:
        side_p->work.authentication = net2hs(*((PFWORD)pb));
        DEBUG_LOG("lcp_check - AUTHENT", LEVEL_3, EBS_INT1, 
            side_p->work.authentication, 0);
        toss -= 2;

        if (option_result == CONFIG_REJ)
            break;

        /* Check if new value remote side wants is appropriate:
           REMOTE: processing request, protocol we login with;
           LOCAL:  process nak, what remote wants to login with */
        switch ( side_p->work.authentication ) 
        {
#if (INCLUDE_CHAP)
        case PPP_CHAP_PROTOCOL:
            if (*(pb+2) != MD5_ALGORITHM) 
            {
                DEBUG_ERROR("CHAP request rcvd but algorithm = ",
                    EBS_INT1, *(pb+2), 0);
            }
#endif
#if (INCLUDE_PAP)
        case PPP_PAP_PROTOCOL:
            if (request)
            {
                /* if remote requested protocol we are willing to   */
                /* do then accept it                                */
                if (check_lcp_auth_want(side_p, pb))
                    break;
            }
                      
            /* our REQUEST uses REMOTE=local must authenticate with;   */
            /* NAK from our request; we'll accept what they want us    */
            /* to login with since it is PAP or CHAP                   */
            else
            {
                if ( (side_p->work.authentication == PPP_CHAP_PROTOCOL) &&
                     (side_p->will_negotiate & LCP_WANT_CHAP_MD5) &&
                     (*(pb+2) == MD5_ALGORITHM) )
                    break;
                if ( (side_p->work.authentication == PPP_PAP_PROTOCOL) &&
                     (side_p->will_negotiate & LCP_WANT_PAP) )
                    break;
            }
#endif
        default:
            if (request)
            {
                /* setup next authentication protocol remote must   */
                /* login with to try                                */
                lcp_setup_next_auth(side_p);
                if (check_lcp_auth_want(side_p, pb))
                    break;
            }

            /* No - PAP and CHAP is the only one presently supported;
                if an auth protocol has been setup then NAK with
                the protocol; otherwise if no protocol is set
                up then REJECT */
            /* NOTE: option_result does not matter for NAK unless   */
            /*       it returns -1; i.e. we always respond to a     */
            /*       NAK with a new request; what matters is the    */
            /*       working authentication value                   */
            if (side_p->want.authentication)
            {
                side_p->work.authentication = side_p->want.authentication;
                option_result = CONFIG_NAK;
            }
            else
            {
                option_result = CONFIG_REJ;
            }
        }   /* end of auth switch */
        break;

    case LCP_MAGIC:
        side_p->work.magic_number = net2hl(WARRAY_2_LONG((PFWORD)pb));
        DEBUG_LOG("lcp_check - MAGIC", LEVEL_3, DINT1, side_p->work.magic_number, 0);
        toss -= 4;

        if (option_result == CONFIG_REJ)
            break;

        /* if we did not request MAGIC, then it is not a loop-back link;   */
        /* so accept the request from the remote host                      */
        if (side_p->work.magic_number == 0L)
            break;
            
        /* Ensure that magic numbers are different   */
        if (lcp_p->remote_entry.work.magic_number ==    /* last magic # we sent */
            lcp_p->local_entry.work.magic_number)       /* magic # sent */
        {
            /* send NAK with a new magic numer   */
            side_p->work.magic_number += ks_get_ticks();
            option_result = CONFIG_NAK;
        }
        break;

    case LCP_PFC:
        DEBUG_LOG("lcp_check - PFC", LEVEL_3, NOVAR, 0, 0);
        break;

    case LCP_ACFC:
        DEBUG_LOG("lcp_check - ACFC", LEVEL_3, NOVAR, 0, 0);
        break;

    case LCP_ENCRYPT:       /* not implemented */
    case LCP_QUALITY:       /* not implemented */
    default:
        DEBUG_LOG("lcp_check - ENCRYPT or QUALITY", LEVEL_3, NOVAR, 0, 0);
        option_result = CONFIG_REJ;
        break;
    };

    /* if read more bytes then LCP option header specified   */
    if ( toss < 0 )
        return -1;

    return (option_result);
}

/***********************************************************************  */
/* lcp_setup_next_auth() - cycle between authentication protocols         */
/*                                                                        */
/*   sets side_p->want.authentication to the authentication               */
/*   protocol to try next                                                 */

static void lcp_setup_next_auth(struct lcp_side_s KS_FAR *side_p)
{
    DEBUG_LOG("lcp_setup_next_auth called: will_neg", LEVEL_3, EBS_INT1,
        side_p->will_negotiate, 0);

    /* UNACCEPTABLE AUTH                                       */
    /* cycle between NAKing with CHAP(MD5) if requested by API */
    /* and PAP if PAP was requested by API                     */

    /* if not MD5, first NAK with MD5   */
    if ( (side_p->will_negotiate & LCP_WANT_CHAP_MD5) &&
         !(side_p->will_negotiate & LCP_CHAP_MD5_NAK_TRIED) )
    {
        DEBUG_LOG("lcp_setup_next_auth: set to CHAP", LEVEL_3, NOVAR, 0, 0);

        side_p->want.authentication = PPP_CHAP_PROTOCOL;

        /* give PAP a chance next unacceptable request   */
        if (side_p->will_negotiate & LCP_WANT_PAP) 
        {
            DEBUG_LOG("lcp_setup_next_auth: set CHAP tried flag", LEVEL_3, 
                NOVAR, 0, 0);
            side_p->will_negotiate |= LCP_CHAP_MD5_NAK_TRIED;
        }
    }
    else if (side_p->will_negotiate & LCP_WANT_PAP)
    {
        DEBUG_LOG("lcp_setup_next_auth: set to PAP", LEVEL_3, 
            NOVAR, 0, 0);
        side_p->want.authentication = PPP_PAP_PROTOCOL;

        /* give CHAP another chance next unacceptable request   */
        side_p->will_negotiate &= (word)~LCP_CHAP_MD5_NAK_TRIED;
    }
}

/***********************************************************************  */
/* check_lcp_auth_want - check if requested auth is what we want          */
/*                                                                        */
/*   Checks if requested authentication type is what we want and          */
/*   it is one we support.                                                */
/*                                                                        */
/*   Returns TRUE if it is what we want; FALSE if it is not               */
/*                                                                        */
static RTIP_BOOLEAN check_lcp_auth_want(struct lcp_side_s KS_FAR *side_p, PFBYTE pb)
{
#if (OLD)
    /* if they requested PAP or CHAP, MD5   */
    if ( (side_p->work.authentication == PPP_PAP_PROTOCOL) ||
        ((side_p->work.authentication == PPP_CHAP_PROTOCOL) &&
        (*(pb+2) == MD5_ALGORITHM)) )
    {
        /* if they requested what we want to use to login in with   */
        /* then accept it                                           */
        if (side_p->work.authentication == 
            side_p->want.authentication)
        {
            return(TRUE);
        }
    }
#else
    /* if they requested PAP or CHAP, MD5   */
    if (side_p->work.authentication == PPP_PAP_PROTOCOL)
    {
        if (side_p->will_negotiate & LCP_WANT_PAP)
            return(TRUE);
    }
    else if ( (side_p->work.authentication == PPP_CHAP_PROTOCOL) &&
              (*(pb+2) == MD5_ALGORITHM) )
    {
        if (side_p->will_negotiate & LCP_WANT_CHAP_MD5)
            return(TRUE);
    }
#endif
    return(FALSE);
}

/***********************************************************************  */
/* Check Link Control options requested by the remote host                */
/* returns -1 on error                                                    */
/* returns 0 on CONFIG_ACK response sent                                  */
/* returns 1 on CONFIG_NAK or CONFIG_REJ response sent                    */
int lcp_request(PFSMS fsm_p, PCONFIG_HDR config_hdr, DCU msg)
{
PLCPS lcp_p = (PLCPS)fsm_p->pdv;
int signed_length;
DCU reply_msg;                  /* reply packet */
PFBYTE reply_pb;
int reply_result = CONFIG_ACK;  /* reply to request */
word desired;                   /* desired to negotiate */
struct option_hdr option;       /* option header storage */
int option_result;              /* option reply */
PFBYTE pb;                      /* pointer to data area */
PFBYTE pb_pad;                  /* pointer to data area (padding area) */
struct lcp_side_s KS_FAR *remote_p = &(lcp_p->remote_entry);    
struct lcp_side_s KS_FAR *local_p = &(lcp_p->local_entry);  
RTIP_BOOLEAN magic_in_resp = FALSE;

    DEBUG_LOG("lcp_request()", LEVEL_3, NOVAR, 0, 0);
    lcp_p->remote_entry.work.negotiate = FALSE; /* clear flags */

    /* point to option area of the packet   */
    pb = DCUTODATA(msg) + IN_PKT_HDR_LEN;

    /* get a DCU to send reply in    */
    reply_msg = os_alloc_packet(MAX_PACKETSIZE, LCP_REPLY_ALLOC);
    if (!reply_msg)
    {
        return -1;  /* tbd */
    }
    DCUTOPACKET(reply_msg)->length = OUT_PKT_HDR_LEN;  
                                    /* leave room for PPP and config header   */

    /* get length of options from input packet   */
    signed_length = config_hdr->len - IN_PKT_HDR_LEN;

    /* Process each option requested by remote host   */
    while (signed_length > 0  &&  ntohopt(&option, pb) != -1) 
    {
        pb += OPTION_HDR_LEN;

        if ((signed_length -= option.len) < 0) 
        {
            DEBUG_ERROR("LCP REQ: bad header length", NOVAR, 0, 0);
            os_free_packet(reply_msg);
            return -1;
        }

        /* check the request   */
        if ( ( option_result = lcp_check( pb, lcp_p, remote_p,
                                          &option, TRUE ) ) == -1 ) 
        {
            DEBUG_ERROR("LCP REQ: ran out of data", NOVAR, 0, 0);
            os_free_packet(reply_msg);
            return -1;
        }

        /* set pb_pad to point to padding area (if any) and   */
        /* set pb to point to next option                     */
        /* NOTE: header already accounted for                 */
        pb_pad = pb;
        pb += option.len - OPTION_HDR_LEN;

        /* check if overran packet length as specified by DCU   */
        if ( pb > (DCUTODATA(msg)+DCUTOPACKET(msg)->length) )
        {
            os_free_packet(reply_msg);
            DEBUG_ERROR("lcp_request - overran pkt length", NOVAR, 0, 0);
            return -1;
        }

        if (option_result < reply_result) 
        {
            continue;
        } 
        else if (option_result > reply_result) 
        {
            /* Discard current list of replies leaving room for headers   */
            DCUTOPACKET(reply_msg)->length = OUT_PKT_HDR_LEN;  
            reply_result = option_result;
            magic_in_resp = FALSE;
        }

        /* remember that we processed option   */
        if ( option_result != CONFIG_REJ && option.type <= LCP_OPTION_LIMIT ) 
        {                                                 
            lcp_p->remote_entry.work.negotiate |= (word)(1 << option.type);
        }

        /* put option into reply   */
        if ( (option.type > 0) && (option.type <= LCP_OPTION_LIMIT) )
        {
            pb_pad += get_option_len(option.type, 
                                     lcp_p->remote_entry.work.authentication) 
                      - OPTION_HDR_LEN;    

            /* Add option response to the return list   */
            DEBUG_LOG("lcp_request - add option to return list", LEVEL_3, NOVAR, 0, 0);

            /* remember that MAGIC is in reply   */
            if (option.type == LCP_MAGIC)
                magic_in_resp = TRUE;

            lcp_option(reply_msg, &(lcp_p->remote_entry.work), option.type, 
                       get_option_len(option.type, lcp_p->remote_entry.work.authentication), 
                       pb_pad);
        }

        /* for illegal options, when putting option into reply, do a   */
        /* straight copy                                               */
        else
        {
            /* if not enough room to put the option, don't do it   */
            if ( (DCUTOPACKET(reply_msg)->length + option.len) <= 
                  MAX_PACKETSIZE )
            {
                reply_pb = DCUTODATA(reply_msg) + DCUTOPACKET(reply_msg)->length;
                tc_movebytes(reply_pb, pb - option.len, option.len);
                DCUTOPACKET(reply_msg)->length += (word)option.len;
            }
        }
    }           /* done processing all options */

    /* Now check for any missing options which are desired   */
    desired = (word)(lcp_p->remote_entry.want.negotiate & 
                     ~lcp_p->remote_entry.work.negotiate);

    /* do not suggest magic if remote does not request it;        */
    /* magic is different than the other options; the NAK is used */
    /* if receive a request which is loopback                     */
    desired &= ~(1 << LCP_MAGIC);

    /* do not suggest authentication if remote does not request it;   */
    desired &= ~(1 << LCP_AUTHENT);

    if (fsm_p->retry_nak > 0 && desired != 0) 
    {
        DEBUG_LOG("lcp_request - missing option-remote_entry.want/word.negotiate = ", LEVEL_3, EBS_INT2, 
            lcp_p->remote_entry.want.negotiate, lcp_p->remote_entry.work.negotiate);

        switch (reply_result) 
        {
        case CONFIG_ACK:
            DCUTOPACKET(reply_msg)->length = OUT_PKT_HDR_LEN;   
                    /* leave room for config header   */
            reply_result = CONFIG_NAK;
            DEBUG_LOG("lcp_request - CONFIG_ACK: missing -> NAK it", LEVEL_3, NOVAR, 0, 0);
            /* fallthru   */
        case CONFIG_NAK:
            lcp_makeoptions(reply_msg, &(lcp_p->remote_entry.want), desired);
            fsm_p->retry_nak--;
            DEBUG_LOG("lcp_request - CONFIG_NAK: missing", LEVEL_3, NOVAR, 0, 0);
            break;
        case CONFIG_REJ:
            DEBUG_LOG("lcp_request - CONFIG_REJ: missing", LEVEL_3, NOVAR, 0, 0);
            /* do nothing   */
            break;
        };
    } 
    else if ( reply_result == CONFIG_NAK ) 
    {
        /* if too many NAKs, reject instead   */
        if ( fsm_p->retry_nak > 0 )
            fsm_p->retry_nak--;
        else
        {
            /* do not reject magic number since if the REJECT is looped-back   */
            /* it will be interpreted that the link is not loop-back           */
            /* NOTE: requirements are to not reject if local is sending        */
            /*       request with magic                                        */
            if ( !(magic_in_resp && local_p->work.negotiate & LCP_N_MAGIC) ) 
            {
                reply_result = CONFIG_REJ;
            }
        }
    }

    /* Send ACK/NAK/REJ to remote host   */
    fsm_send(fsm_p, (byte)reply_result, config_hdr->id, reply_msg);
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */
    return (reply_result != CONFIG_ACK);
}


/***********************************************************************  */
/* Process configuration ACK sent by remote host                          */
int lcp_ack(PFSMS fsm_p, PCONFIG_HDR config_hdr, DCU msg)
{
int error = FALSE;
PFBYTE pb, req_pb;
int signed_length;
DCU req_msg;

    DEBUG_LOG("lcp_ack()", LEVEL_3, NOVAR, 0, 0);

    /* point to options area in packet received   */
    pb = DCUTODATA(msg);
    pb += IN_PKT_HDR_LEN;

    /* ID field must match last request we sent   */
    if (config_hdr->id != fsm_p->lastid) 
    {
        DEBUG_ERROR("LCP ACK: wrong ID: recved, sent", EBS_INT2, 
            config_hdr->id, fsm_p->lastid);
        return -1;
    }

    /* Get a copy of last request we sent   */
    req_msg = lcp_makereq(fsm_p);
    if (!req_msg)
    {
        DEBUG_ERROR("LCP ACK: out of packets", NOVAR, 0, 0);
        return -1;
    }

    /* Overall buffer length should match         */
    /* NOTE: length includes configuration header */
    if ((int)config_hdr->len != DCUTOPACKET(req_msg)->length - PPP_HDR_LEN) 
    {
        DEBUG_ERROR("LCP ACK: buffer length mismatch", EBS_INT2,
            config_hdr->len, DCUTOPACKET(req_msg)->length); 
        error = TRUE;
    } 
    else 
    {
        /* point to options area of last request we sent   */
        req_pb = DCUTODATA(req_msg) + OUT_PKT_HDR_LEN;

        /* get length of options of ACK   */
        signed_length = config_hdr->len - IN_PKT_HDR_LEN;

        /* Each byte should match   */
        while (signed_length--) 
        {
            if (*pb++ != *req_pb++) 
            {
                DEBUG_ERROR("LCP ACK: data mismatch", NOVAR, 0, 0);
                error = TRUE;
                break;
            }
        }
    }
    os_free_packet(req_msg);

    if (error) 
        return -1;

    DEBUG_LOG("LCP ACK: valid", LEVEL_3, NOVAR, 0, 0);
    return 0;
}


/***********************************************************************  */
/* Process configuration NAK sent by remote host                          */
int lcp_nak(PFSMS fsm_p, PCONFIG_HDR config_hdr, DCU msg)
{
PLCPS lcp_p = (PLCPS)fsm_p->pdv;
struct lcp_side_s KS_FAR *local_p = &(lcp_p->local_entry);
int signed_length;
struct option_hdr option;
int last_option = 0;
int result;
PFBYTE pb;

    DEBUG_LOG("lcp_nak() - NAK received", LEVEL_3, NOVAR, 0, 0);

    /* ID field must match last request we sent   */
    if (config_hdr->id != fsm_p->lastid) 
    {
        DEBUG_ERROR("LCP NAK: wrong ID", NOVAR, 0, 0);
        return -1;
    }

    /* First, process in order.  Then, process extra "important" options   */
    pb = DCUTODATA(msg);
    pb += IN_PKT_HDR_LEN;

    /* get length of options in input packet   */
    signed_length = config_hdr->len - IN_PKT_HDR_LEN;

    while (signed_length > 0  &&  ntohopt(&option, pb) != -1) 
    {
        pb += OPTION_HDR_LEN;

        if ((signed_length -= option.len) < 0) 
        {
            DEBUG_ERROR("LCP NAK: bad header length", NOVAR, 0, 0);
            return -1;
        }
        if ( (option.type <= 0) || (option.type > LCP_OPTION_LIMIT) ) 
        {
            DEBUG_ERROR("LCP NAK: option out of range", NOVAR, 0, 0);
        } 

        /* if out of order or is not being negotiated (i.e. did not send    */
        /* the option in last msg)                                          */
        else if ( option.type < last_option ||
                  !(local_p->work.negotiate & (1 << option.type)) ) 
        {
            if (local_p->work.negotiate & (1 << option.type)) 
            {
                DEBUG_ERROR("LCP NAK: option out of order", NOVAR, 0, 0);
                return -1;      /* was requested */
            }

            /* option was NAKed, turn on negotiation if it is negotiable   */
            if ( local_p->will_negotiate & (1 << option.type) ) 
            {
                local_p->work.negotiate |= (word)(1 << option.type);
                last_option = LCP_OPTION_LIMIT + 1;
                DEBUG_LOG("lcp_nak - did not send option - turn on neg for option", LEVEL_3, EBS_INT2, 
                    option.type, local_p->work.negotiate);
            }
        } 
        else 
        {
            last_option = option.type;
        }

        /* check the nak                                              */
        /* check and update the working values but if value of option */
        /* sent is invalid, set working value to an acceptable value  */
        if ( ( result = lcp_check(pb, lcp_p, local_p, &option, FALSE) ) == -1 ) 
        {
            DEBUG_ERROR("LCP NAK: ran out of data", NOVAR, 0, 0);
            return(result);
        }

        /* point past the current option; amount to bypass is based   */
        /* upon option length specified in packet                     */
        pb += option.len - OPTION_HDR_LEN;

        /* check if overran packet length as specified by DCU   */
        if ( pb > (DCUTODATA(msg) + DCUTOPACKET(msg)->length) )
        {
            return -1;
        }
    }
    return 0;
}


/***********************************************************************  */
/* Process configuration reject sent by remote host                       */
int lcp_reject(PFSMS fsm_p, PCONFIG_HDR config_hdr, DCU msg)
{
PLCPS lcp_p = (PLCPS)fsm_p->pdv;
struct lcp_side_s KS_FAR *local_p = &(lcp_p->local_entry);
int signed_length;
struct option_hdr option;
int last_option = 0;
PFBYTE pb;

    DEBUG_LOG("lcp_reject()", LEVEL_3, NOVAR, 0, 0);

    /* ID field must match last request we sent   */
    if (config_hdr->id != fsm_p->lastid) 
    {
        DEBUG_ERROR("LCP REJ: wrong ID", NOVAR, 0, 0);
        return -1;
    }

    /* point to options area of packet   */
    pb = DCUTODATA(msg) + IN_PKT_HDR_LEN;

    /* get length of options   */
    signed_length = config_hdr->len - IN_PKT_HDR_LEN;

    /* Process in order, checking for errors       */
    /* loop thru options and get the option header */
    while (signed_length > 0  &&  ntohopt(&option, pb) != -1) 
    {
        /* decrement loop variable by size of option specified in header   */
        if ((signed_length -= option.len) < 0) 
        {
            DEBUG_ERROR("LCP REJ: bad header length", NOVAR, 0, 0);
            return -1;
        }

        if ( (option.type <= 0) || (option.type > LCP_OPTION_LIMIT) ) 
        {
            DEBUG_ERROR("LCP REJ: option out of range", NOVAR, 0, 0);
        }
 
        else if ( option.type < last_option || 
                  !(local_p->work.negotiate & (1 << option.type))) 
        {
            DEBUG_ERROR("LCP REJ: option out of order", NOVAR, 0, 0);
            return -1;
        }

        /* move pointer past data area of option and check to make     */
        /* sure did not overrun packet                                 */
        /* NOTE: currently it points beginning of header of the option */
        pb += option.len;   /* NOTE: len include header */
        if (pb > (DCUTODATA(msg) + DCUTOPACKET(msg)->length))
        {
            DEBUG_ERROR("LCP REJ: ran out of data", NOVAR, 0, 0);
            return -1;
        }
        last_option = option.type;

        if ( option.type <= LCP_OPTION_LIMIT ) 
        {
            /* if AUTHEN was requested, it is unacceptable to reject it;   */
            /* all others are OK and negotiation will be dropped for them  */
            /* NOTE: for magic numbers receipt of a reject indicates       */
            /*       the connection is not loop-back                       */
            if (option.type != LCP_AUTHENT) 
                local_p->work.negotiate &= (word)(~(1 << option.type));
        }
    }
    return 0;
}

/***********************************************************************   */
/* ECHO EVENT PROCESSING                                                   */
/***********************************************************************   */
/* signal xn_ppp_echo that a reply came in                                 */
void proc_lcp_echo_reply(PIFACE pi, DCU msg)
{
PANYPORT port;

    port = pi->ctrl.echo_port;
    if (port)
        OS_SNDX_PING_EXCHG(port, msg);
}

/* process an incoming echo request                                  */
/* returns TRUE if response sent (i.e. msg freed)                    */
/* NOTE: input packet has room for ethernet header but output packet */
/*       is formatted as an PPP packet                               */
RTIP_BOOLEAN proc_lcp_echo_request(PFSMS fsm_p, DCU msg, byte id)
{
PFBYTE pb;
PLCPS  lcp_p = (PLCPS)(fsm_p->pdv);
int diff;

    pb = DCUTODATA(msg);

    /* find the difference of where the output packet starts and the   */
    /* input packet starts                                             */
    diff = OUT_PKT_HDR_LEN - IN_PKT_HDR_LEN;

    /* convert input packet to an output packet   */
    /* move pb to pb+diff                         */
    tc_memmove(pb+diff, pb, DCUTOPACKET(msg)->length);

    DCUTOPACKET(msg)->length += diff;

    /* point to magic number option   */
    pb += OUT_PKT_HDR_LEN;

    /* check magic number to make sure not loopback             */
    /* if magic number matches one we would send in a request,  */
    /* then loopback so drop it                                 */
    if ( *(PFDWORD)pb == 0 ||       /* no magic number in request */
         hl2net(lcp_p->local_entry.work.magic_number) != *(PFDWORD)pb )
    {
        /* set magic number to local number which was negotiated   */
        LONG_2_WARRAY((PFWORD)pb, 
                      hl2net(lcp_p->local_entry.work.magic_number));

        /* send ECHO REPLY   */
        fsm_send(fsm_p, ECHO_REPLY, id, msg);
        return(TRUE);
    }
    return(FALSE);
}

/* ********************************************************************   */
/* DISCARD, ECHO SEND                                                     */
/* ********************************************************************   */
/* tbd - never called; needs api call; should msg be freed after sent?    */
/* NOTE: msg is packet to sent; must have room for config header, option  */
/*       header and magic number before the data                          */
int ppp_discard(PIFACE pi, DCU msg)
{
PPPPS ppp_p = pi->edv;

    return fsm_send(&(ppp_p->fsm[Lcp]), DISCARD_REQ, 0, msg);
}

void free_port(PANYPORT port)
{
#if (INCLUDE_UDP)
    /* NOTE: do not need to deference port in arp table since      */
    /*       port structure is not passed to tc_netwrite()         */
    port->list_type = FREE_LIST; 
    os_free_udpport((PUDPPORT)port);
#elif (INCLUDE_TCP)
    free_tcpport_avail((PTCPPORT)port, FALSE);
#endif
}

/* send an echo request and wait and process result                  */
/* must have room for config header, option header and magic number  */
/* before the data                                                   */
/* NOTE: must call xn_lcp_want_magic with local seed                 */
int ppp_echo(PIFACE pi, word wait_count, int size)
{
DCU    msg;
PLCPS  lcp_p;
PFSMS  fsm_p;
PFBYTE pb;
int    errnum;
int    i;
PANYPORT  port;
                                                    
    if (!pi->edv)
        return(-1);

    fsm_p = (PFSMS)&(pi->edv->fsm[Lcp]);

#if (INCLUDE_UDP)
    port = (PANYPORT)os_alloc_udpport();
#elif (INCLUDE_TCP)
    port = (PANYPORT)os_alloc_tcpport();
#else
#error: INCLUDE_TCP or INCLUDE_UDP has to be on for PING
#endif

    if (!port)
    {
        return(set_errno(EMFILE));
    }

    /* save the port structure so when the reply comes comes in we know   */
    /* what port to signal                                                */
    /* NOTE: you can only do one echo per interface at a time             */
    pi->ctrl.echo_port = port;

    /* allocate a packet used to build options up in                       */
    /* NOTE: don't know size of packet before building packet so alloc max */
    msg = os_alloc_packet(MAX_PACKETSIZE, ECHO_ALLOC);
    if (!msg)
    {
        free_port(port);
        return(set_errno(ENOPKTS));
    }

    lcp_p = (PLCPS)fsm_p->pdv;

    /* +4 for magic number   */
    DCUTOPACKET(msg)->length = OUT_PKT_HDR_LEN + size + 4;

    pb = DCUTODATA(msg) + OUT_PKT_HDR_LEN;  /* point to magic number */
    LONG_2_WARRAY((PFWORD)pb, 
                  hl2net(lcp_p->local_entry.work.magic_number));

    pb += 4;        /* skip past magic number option and point to data */

    tc_memset(pb, 0x55, size);

    /* bind exchange to this application task so IP task knows who to send   */
    /* response to                                                           */
    /* NOTE: this code may be called only once at a time per interface       */
    OS_BIND_PING_EXCHG(port);

    /* clear out any residual packets   */
    OS_CLEAR_PING_EXCHG(port);

    /* send the LCP ECHO command   */
    fsm_send(fsm_p, ECHO_REQ, 0, msg);

    /* assume success   */
    errnum = 0;

    /* wait for response or timeout   */
    msg = OS_RCVX_PING_EXCHG(port, (word)wait_count);
    if (!msg)
        errnum = ETIMEDOUT;
    else
    {
        /* check if id matches request?   */
        pb = DCUTODATA(msg) + 1;    /* point to id */
        if (*pb != fsm_p->lastid) 
        {
            DEBUG_ERROR("ppp_echo: id does not match: exp, act",
                EBS_INT2, fsm_p->lastid, *pb);
            errnum = EBADRESP;
        }

        /* check magic number to make sure not loopback   */
        pb = DCUTODATA(msg) + IN_PKT_HDR_LEN;   /* point to Magic-Number */

        /* if magic number is not the remote's negotiated number, then    */
        /* possibly loopback or link misconfigured for communication      */
        /* with a different peer                                          */
        if ( hl2net(lcp_p->remote_entry.work.magic_number) != *(PFDWORD)pb )
        {
            DEBUG_ERROR("ppp_echo: magic number does not match: exp, act",
                DINT2, hl2net(lcp_p->remote_entry.work.magic_number), 
                *(PFDWORD)pb);
            errnum = EBADRESP;
        }

        /* check if data matches   */
        pb += 4;    /* point to data */
        for (i=0; i<size; i++)
        {
            if (*pb != 0x55)
            {
                DEBUG_ERROR("ppp_echo: data does not match: offset, act",
                    EBS_INT2, i, *pb);
                errnum = EBADRESP;
            }
            pb++;
        }
    }

    if (msg)
        os_free_packet(msg);        

    free_port(port);

    if (errnum)
        return(set_errno(errnum));
    return(0);
}

/***********************************************************************   */
/*          I N I T I A L I Z A T I O N                                    */
/***********************************************************************   */

/* Check for PPP Network-Layer Protocol Phase                    */
/* i.e. if no authentication protocol, start IPCP negotiation or */
/*      signal xn_lcp_open                                       */
/* called when PAP, CHAP and LCP is opening                      */
void ppp_ready(PPPPS ppp_p)
{
PFSMS fsm_p;

    ppp_p->id = 1;

    /* check if authentication is necessary   */
    if ( !(ppp_p->flags & (PPP_AP_LOCAL | PPP_AP_REMOTE)) ) 
    {
        DEBUG_LOG("ppp_ready - no authentication protocol pending", 
            LEVEL_3 , NOVAR, 0, 0);

        /* no pending authentication   */
        PPP_TRANS_PHASE(ppp_p, pppREADY);
#if (DEBUG_PHASE)
        DEBUG_ERROR("ppp_ready: go to READY", NOVAR, 0, 0);
#endif

        /* if PAP authentication is done, start IPCP config exchange   */
        /* NOTE: even if passive do not need to wait for other side    */
        /*       to start negotiation                                  */
        fsm_p = (PFSMS)&(ppp_p->fsm[IPcp]);
        if ( fsm_p->flags & (FSM_ACTIVE | FSM_PASSIVE) )
        {
#if (DEBUG_PHASE)
            DEBUG_ERROR("ppp_ready: start IPCP", NOVAR, 0, 0);
#endif
            DEBUG_LOG("ppp_ready - start IPCP", LEVEL_3, NOVAR, 0, 0);
            fsm_active(fsm_p);  /* set to active open */
            fsm_start(fsm_p);
            return;
        }
        else
        {
            DEBUG_LOG("ppp_ready - signal open", LEVEL_3 , NOVAR, 0, 0);
#if (DEBUG_SIGNAL)
            DEBUG_ERROR("ppp_ready: set signal", NOVAR, 0, 0);
            ppp_signal_set();
#endif
            /* if no authentication pending and no IPCP exchange pending   */
            /* LCP OPEN IS DONE, therefore, signal dolcp_open              */
            ppp_p->iface->ctrl.signal_status = PPP_SIGNAL_SUCCESS;  
            OS_SET_PPP_SIGNAL(ppp_p->iface);
        }
    }
}

/***************************************************************************  */
/* Reset configuration options before request                                 */
void lcp_reset(PFSMS fsm_p)
{
PLCPS lcp_p = (PLCPS)fsm_p->pdv;

    DEBUG_LOG("lcp_reset()", LEVEL_3, NOVAR, 0, 0);

    /* if option but no seed then just want to accept magic request   */
    /* from remote but will not request it ourselves                  */
    if ( (lcp_p->local_entry.want.negotiate & LCP_N_MAGIC) &&
         lcp_p->local_entry.want.magic_number )
    {
        /* the magic number is just a seed at this point so randomize it   */
        if (lcp_p->local_entry.want.magic_number)
            lcp_p->local_entry.want.magic_number += ks_get_ticks();
    }

    ASSIGN( lcp_p->local_entry.work, lcp_p->local_entry.want );

    lcp_p->remote_entry.work.negotiate = FALSE;
}


/***********************************************************************  */
/* Prepare to begin configuration exchange                                */
void lcp_starting(PFSMS fsm_p)
{
    DEBUG_LOG("lcp_starting() - set phase to LCP", LEVEL_3, NOVAR, 0, 0);

    PPP_TRANS_PHASE(fsm_p->ppp_p, pppLCP);
#if (DEBUG_PHASE)
    DEBUG_ERROR("lcp_starting: go to LCP", NOVAR, 0, 0);
#endif
}


/***********************************************************************  */
/* After termination                                                      */
void    lcp_stopping(PFSMS fsm_p)
{
PIFACE pi = fsm_p->ppp_p->iface;

    DEBUG_LOG("lcp_stopping()", LEVEL_3, NOVAR, 0, 0);
    ARGSUSED_PVOID(pi);
}


/***********************************************************************  */
/* Close higher levels in preparation for link shutdown                   */
void lcp_closing(PFSMS fsm_p)
{
PPPPS ppp_p = fsm_p->ppp_p;

    PPP_TRANS_PHASE(ppp_p, pppTERMINATE);
#if (DEBUG_PHASE)
        DEBUG_ERROR("lcp_closing: go to TERINATE", NOVAR, 0, 0);
#endif

    fsm_down( &(ppp_p->fsm[IPcp]) );
#if (INCLUDE_PAP)
    pap_down( &(ppp_p->fsm[Pap]) );
#endif
#if (INCLUDE_CHAP)
    chap_down( &(ppp_p->fsm[Chap]) );
#endif
}


#ifdef TURBOC_SWITCH_BUG
#pragma option -G-
#endif

/***********************************************************************  */
/* LCP configuration negotiation complete                                 */
void lcp_opening(PFSMS fsm_p)
{
PLCPS lcp_p = (PLCPS)fsm_p->pdv;
PIFACE pi =  fsm_p->ppp_p->iface;

    /* priority here is device table, then what was negotiated   */
    /* tbd: if it isn't negotiation shouln't it be default?      */
    if (pi->addr.mtu != lcp_p->local_entry.work.mru) 
    {
        /* Set new Max Transmission Unit for incoming packets; i.e.
           mss value we send in the sync message */
        pi->addr.mtu = lcp_p->local_entry.work.mru;
    }
    pi->addr.remote_mtu = lcp_p->remote_entry.work.mru;

    /* check for authentication   */
    PPP_TRANS_PHASE(fsm_p->ppp_p, pppAP);
#if (DEBUG_PHASE)
    DEBUG_ERROR("lcp opening: go to AUTHENTICATION", NOVAR, 0, 0);
#endif
    fsm_p->ppp_p->flags &= ~(PPP_AP_LOCAL | PPP_AP_REMOTE);
    fsm_p->ppp_p->peername[0] = NULLCHAR;

    /* LCP done: authenticate if negotiated   */
    if (lcp_p->local_entry.work.negotiate & LCP_N_AUTHENT) 
    {
        switch (lcp_p->local_entry.work.authentication) 
        {
#if (INCLUDE_PAP)
        case PPP_PAP_PROTOCOL:
            /* set to LISTEN; i.e. wait for remote host to   */
            /* log on                                        */
            pap_local(fsm_p->ppp_p);    /* init state machine */
            break;
#endif
#if (INCLUDE_CHAP)
        case PPP_CHAP_PROTOCOL:
            /* send challenge   */
            chap_local(fsm_p->ppp_p);   /* init state machine */
            break;
#endif
        };
    }

    if (lcp_p->remote_entry.work.negotiate & LCP_N_AUTHENT) 
    {
        switch (lcp_p->remote_entry.work.authentication) 
        {
#if (INCLUDE_PAP)
        case PPP_PAP_PROTOCOL:
            /* logon to remote host; i.e. send PAP request   */
            pap_remote(fsm_p->ppp_p);   /* init state machine */
            break;
#endif
#if (INCLUDE_CHAP)
        case PPP_CHAP_PROTOCOL:
            /* set up to wait for challange   */
            chap_remote(fsm_p->ppp_p);  /* init state machine */
            break;
#endif
        };
    }

    /* LCP open: if no authentication, start IPCP   */
    ppp_ready(fsm_p->ppp_p);
}

static byte get_option_len(byte o_type, word auth_type)
{
byte option_len;

    option_len = lcp_option_length[o_type];
    if ( (o_type == LCP_AUTHENT) && 
         (auth_type == PPP_CHAP_PROTOCOL) )
        option_len = (byte)(option_len + 1);        /* include algorithm */
    return(option_len);
}

#ifdef TURBOC_SWITCH_BUG
#pragma option -G
#endif


/***********************************************************************  */
void lcp_free(PFSMS fsm_p)
{
    /* nothing to do   */
    ARGSUSED_PVOID(fsm_p);
}


/* Allocate configuration structure; called by ppp_open   */
RTIP_BOOLEAN lcp_open(PPPPS ppp_p)
{
PFSMS fsm_p = &(ppp_p->fsm[Lcp]);
PIFACE pi;

    DEBUG_LOG("lcp_open()", LEVEL_3, NOVAR, 0, 0);

    pi = ppp_p->iface;

    fsm_p->ppp_p = ppp_p;
    fsm_p->pdc = (PFSM_CONSTS)&lcp_constants;
    fsm_p->pdv = os_alloc_lcp_cb(pi);
    if (fsm_p->pdv)
        return(TRUE);
    else
        return(FALSE);
}


/* Initialize configuration structure   */
void lcp_init(PPPPS ppp_p, RTIP_BOOLEAN async_link)
{
PFSMS fsm_p = &(ppp_p->fsm[Lcp]);
PLCPS lcp_p;

    DEBUG_LOG("lcp_init()", LEVEL_3, NOVAR, 0, 0);

    lcp_p = (PLCPS)(fsm_p->pdv);

    /* Set option parameters to defaults   */
    ASSIGN( lcp_p->local_entry.want, lcp_default );
    ASSIGN( lcp_p->local_entry.work, lcp_default );
    lcp_p->local_entry.will_negotiate = lcp_negotiate;

    ASSIGN( lcp_p->remote_entry.want, lcp_default );
    ASSIGN( lcp_p->remote_entry.work, lcp_default );
    lcp_p->remote_entry.will_negotiate = lcp_negotiate;

    /* for sync links, the default ACCM is 0   */
    if (!async_link)
    {
        lcp_p->local_entry.want.accm = 0;
        lcp_p->local_entry.work.accm = 0;

        lcp_p->remote_entry.want.accm = 0;
        lcp_p->remote_entry.work.accm = 0;
    }

    fsm_init(fsm_p);
}
#endif


