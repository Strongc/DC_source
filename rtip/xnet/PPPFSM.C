/*                                                                      */
/* PPPFSM.C -- PPP Finite State Machine                                 */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Jan 91  Bill_Simpson@um.cc.umich.edu                                */
/*      Computer Systems Consulting Services                            */
/*                                                                      */
/*  Acknowledgements and correction history may be found in PPP.C       */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

#if (INCLUDE_PPP)

#define DISPLAY_TIMEOUT 0   /* display when resend packets due to timeout */
#define DEBUG_TRAFFIC   0

/* ********************************************************************   */
static int fsm_sendtermreq (PFSMS fsm_p);
static int fsm_sendtermack (PFSMS fsm_p, byte id);
static void fsm_timeout (void KS_FAR *vp);
static void fsm_opening (PFSMS fsm_p);

/***********************************************************************       */
/* Convert header in host form (cnf) to network form (msg) for outgoing pkt    */
void htoncnf(PCONFIG_HDR cnf, DCU msg)
{
PCONFIG_HDR cnfb;

    /* Load header with proper values   */
    cnfb = DCUTOCNFPKT(msg);   /* get dest to store config header */
    cnfb = (PCONFIG_HDR)((PFBYTE)cnfb + PPP_HDR_LEN);
    cnfb->ccode = cnf->ccode;
    cnfb->id = cnf->id;
    cnfb->len = hs2net(cnf->len - PPP_HDR_LEN);
}

/* Extract Config pkt header (i.e. type of cmd and len) from incoming packet   */
int ntohcnf(PCONFIG_HDR cnf, DCU msg)
{
PCONFIG_HDR cnfb;

    if (!cnf)
        return -1;

    if (DCUTOPACKET(msg)->length < IN_PKT_HDR_LEN)
        return -1;

    cnfb       = DCUTOCNFPKT(msg);   /* get config header to extract */
    cnf->ccode = cnfb->ccode;
    cnf->id    = cnfb->id;
    cnf->len   = net2hs(cnfb->len);

    return 0;
}

/**************************************   */
/* Extract configuration option header    */
int ntohopt(POPTION_HDR opt, PFBYTE pb)
{
    if (!opt)
        return -1;

    opt->type = pb[0];
    opt->len = pb[1];
    return 0;
}


/***********************************************************************  */
void fsm_no_action(PFSMS fsm_p)
{
    DEBUG_LOG("fsm_no_action()", LEVEL_3, NOVAR, 0, 0);
    ARGSUSED_PVOID(fsm_p);
}

int fsm_no_check(PFSMS fsm_p, PCONFIG_HDR hdr, DCU bp)
{
    DEBUG_LOG("fsm_no_check()", LEVEL_3, NOVAR, 0, 0);
    ARGSUSED_PVOID(fsm_p);
    ARGSUSED_PVOID(hdr);
    ARGSUSED_PVOID(bp);
    return 0;
}

/***************************************************************************  */
/* called at initialization when a passive open is done                       */
int fsm_passive(PFSMS fsm_p)
{
    fsm_p->flags = (byte)(fsm_p->flags & ~FSM_ACTIVE);
    fsm_p->flags = (byte)(fsm_p->flags | FSM_PASSIVE);

    if ( fsm_p->ppp_state < fsmLISTEN )   /* if closed */
    {
        DEBUG_LOG("fsm_passive - state = LISTEN", LEVEL_3, NOVAR, 0, 0);
        PPP_TRANS_STATE(fsm_p, fsmLISTEN);
    }

    return 0;
}


/* called at initialization when a passive open is done   */
int fsm_active(PFSMS fsm_p)
{
    fsm_p->flags &= ~FSM_PASSIVE;
    fsm_p->flags |= FSM_ACTIVE;

    if ( fsm_p->ppp_state < fsmLISTEN )   /* if closed */
    {
        DEBUG_LOG("fsm_active - state = LISTEN", LEVEL_3, NOVAR, 0, 0);
        PPP_TRANS_STATE(fsm_p, fsmLISTEN);
    }
    return 0;
}



/***********************************************************************  */
/* Send a PPP negotiation packet to the remote host                       */
/* This routine is not called for IP packets                              */
/* msg - pkt without PPP header                                           */
/* frees packets                                                          */
/*                                                                        */
/* Returns errno value where 0 means success                              */
/*                                                                        */
int fsm_send(PFSMS fsm_p, byte ccode, byte id, DCU msg)
{
PPPPS ppp_p = fsm_p->ppp_p;
PIFACE pi = ppp_p->iface;
struct config_hdr hdr;

    switch (hdr.ccode = ccode) 
    {
    case CONFIG_REQ:
    case TERM_REQ:
    case ECHO_REQ:
        /* Save ID field for match against replies from remote host   */
        fsm_p->lastid = ppp_p->id;
        /* fallthru   */

    case PROT_REJ:
    case DISCARD_REQ:
    case ID_LCP:
        /* Use a unique ID field value   */
        hdr.id = ppp_p->id++;
        break;

    case CONFIG_ACK:
    case CONFIG_NAK:
    case CONFIG_REJ:
    case TERM_ACK:
    case CODE_REJ:
    case ECHO_REPLY:
        /* Use ID sent by remote host   */
        hdr.id = id;
        break;

    default:
        /* we're in trouble   */
        os_free_packet(msg);
        DEBUG_ERROR("PPP: Send with bogus code", NOVAR, 0, 0);
        return -1;
    };

    /* Write header to packet data   */
    hdr.len = (word)DCUTOPACKET(msg)->length;
    htoncnf(&hdr, msg);

    ppp_p->OutNCP[fsm_p->pdc->fsmi]++;

    DEBUG_LOG("OUTPUT PKT = ", LEVEL_3, NOVAR, 0, 0);
    DEBUG_LOG("PACKET = ", LEVEL_3, PKT, (PFBYTE)DCUTOETHERPKT(msg), 
        DCUTOPACKET(msg)->length);

    /* let ppp_xmit know what kind of packet is being sent    */
    DCUTOPACKET(msg)->protocol = fsm_p->pdc->protocol;

    /* queue packet for output                                             */
    /* don't keep after send (DCUTOCONTROL(msg).dcu_flags is 0 from alloc) */
    return(tc_interface_send(pi, msg));
}


/***********************************************************************  */
/* Send a configuration request                                           */
int fsm_sendreq(PFSMS fsm_p)
{
DCU msg;
int ret_val;

    DEBUG_LOG("fsm_sendreq()", LEVEL_3, NOVAR, 0, 0);
#if (DEBUG_TRAFFIC)
    DEBUG_ERROR("fsm_sendreq(): retry: ", EBS_INT1, fsm_p->retry, 0);
#endif

    if (fsm_p->retry <= 0)
    {
        DEBUG_ERROR("fsm_sendreq: retry counter expired", NOVAR, 0, 0);
        return -1;
    }

    fsm_p->retry--;
    fsm_timer(fsm_p);

    msg = (*fsm_p->pdc->makereq)(fsm_p);
    if (msg)
    {
        ret_val = fsm_send(fsm_p, CONFIG_REQ, 0, msg);
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */
        return(ret_val);
    }
    DEBUG_ERROR("fsm_sendreq: makereq failed", NOVAR, 0, 0);
    return(-1);
}


/***********************************************************************  */
/* Send a termination request                                             */
static int fsm_sendtermreq(PFSMS fsm_p)
{
int ret_val;
DCU msg;

    DEBUG_LOG("fsm_sendtermreq()", LEVEL_3, NOVAR, 0, 0);
#if (DEBUG_TRAFFIC)
    DEBUG_ERROR("fsm_sendtermreq(): retry = ", EBS_INT1, fsm_p->retry, 0);
#endif

    if (fsm_p->retry <= 0)
        return -1;

    fsm_p->retry--;
    fsm_timer(fsm_p);

    /* allocate a packet used to send terminate request   */
    msg = os_alloc_packet(OUT_PKT_HDR_LEN, FSM_TR_ALLOC);
    if (!msg)
    {
        DEBUG_ERROR("fsm_sendtermreq: out of packets", NOVAR, 0, 0);
        return(-1);
    }

    DCUTOPACKET(msg)->length = OUT_PKT_HDR_LEN;  /* leave room for config header */

    ret_val = fsm_send(fsm_p, TERM_REQ, 0, msg);
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */
    return(ret_val);
}


/***********************************************************************  */
/* Send Terminate Ack                                                     */
static int fsm_sendtermack(PFSMS fsm_p, byte id)
{
DCU msg;
int ret_val;

    DEBUG_LOG("fsm_sendtermack()", LEVEL_3, NOVAR, 0, 0);

#if (DEBUG_TRAFFIC)
    DEBUG_ERROR("fsm_sendtermack()", NOVAR, 0, 0);
#endif

    /* allocate a packet used to send terminate request   */
    msg = os_alloc_packet(OUT_PKT_HDR_LEN, FSM_TA_ALLOC);
    if (!msg)
        return(-1);

    DCUTOPACKET(msg)->length = OUT_PKT_HDR_LEN;  /* leave room for headers */

    ret_val = fsm_send(fsm_p, TERM_ACK, id, msg);
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */

    return(ret_val);
}


/***********************************************************************  */
/* Reset state machine; i.e. state, retry info, config parameter          */
void fsm_reset(PFSMS fsm_p)
{
byte new_state;

    DEBUG_LOG("fsm_reset()", LEVEL_3, NOVAR, 0, 0);
#if (DEBUG_TRAFFIC)
    DEBUG_ERROR("fsm_reset()", NOVAR, 0, 0);
#endif

    /* if ACTIVE or PASSIVE go to listen state   */
    new_state = (byte)((fsm_p->flags & (FSM_ACTIVE | FSM_PASSIVE)) ?
                          fsmLISTEN : fsmCLOSED);
    PPP_TRANS_STATE(fsm_p, new_state);

    /* set the retry counters back to initial values   */
    fsm_p->retry = fsm_p->try_req;
    fsm_p->retry_nak = fsm_p->try_nak;

    (*fsm_p->pdc->reset)(fsm_p);
}


/***********************************************************************  */
/* Configuration negotiation complete                                     */
static void fsm_opening(PFSMS fsm_p)
{
    DEBUG_LOG("Opened - fsm_opening() - state = OPENED", LEVEL_3, NOVAR, 0, 0);
#if (DEBUG_TRAFFIC)
    DEBUG_ERROR("fsm_opening called", NOVAR, 0, 0);
#endif

    ebs_stop_timer(&(fsm_p->timer_info));

    PPP_TRANS_STATE(fsm_p, fsmOPENED);
    (*fsm_p->pdc->opening)(fsm_p);
}


/***********************************************************************   */
/*          E V E N T   P R O C E S S I N G                                */
/***********************************************************************   */

/* Process incoming packet (LCP or IPCP)   */
/* Frees packet when done                  */
void fsm_proc(PIFACE pi, PFSMS fsm_p, DCU msg)
{
struct config_hdr hdr;
RTIP_BOOLEAN freed = FALSE;
byte new_state;

    if ( ntohcnf(&hdr, msg) == -1 )
    {
        DEBUG_LOG("short configuration packet", LEVEL_3, NOVAR, 0, 0);
    }

    DEBUG_LOG("fsm_proc() entered - msg hdr.ccode, state", LEVEL_3, EBS_INT2,  
        hdr.ccode, fsm_p->ppp_state);

    /* Trim off padding   */
    if ((int)hdr.len < DCUTOPACKET(msg)->length)
        DCUTOPACKET(msg)->length = hdr.len;

    switch(hdr.ccode) 
    {
    /* **************************************************   */
    case CONFIG_REQ:
        switch(fsm_p->ppp_state) 
        {
        case fsmOPENED:     /* Unexpected event? */
            (*fsm_p->pdc->closing)(fsm_p);
            fsm_reset(fsm_p);
            /* fallthru   */
        case fsmLISTEN:
            (*fsm_p->pdc->starting)(fsm_p);
#if (DEBUG_REQ)
            DEBUG_ERROR("listen->send request", NOVAR, 0, 0);
#endif
            fsm_sendreq(fsm_p);
            /* fallthru   */
        case fsmREQ_Sent:
        case fsmACK_Sent:   /* Unexpected event? */
            /* process the request - if CONFIG_ACK response sent, transition   */
            /* to ACK_Sent; if CONFIG_NAK or CONFIG_REJ sent, transition       */
            /* to REQ_Sent                                                     */
            new_state = (byte)(((*fsm_p->pdc->request)(fsm_p, &hdr, msg) == 0)
                ? fsmACK_Sent : fsmREQ_Sent);
            PPP_TRANS_STATE(fsm_p, new_state);
            DEBUG_LOG("fsm_proc - CONFIG_REQ rcvd - set state to ", LEVEL_3, EBS_INT1, fsm_p->ppp_state, 0);
            break;

        case fsmACK_Rcvd:
            if ((*fsm_p->pdc->request)(fsm_p, &hdr, msg) == 0) 
            {
                fsm_opening(fsm_p);
            } 
            else 
            {
                /* give peer time to respond   */
                fsm_timer(fsm_p);
            }
            break;

        case fsmCLOSED:
            /* Don't accept any connections   */
            fsm_sendtermack(fsm_p, hdr.id);
            /* fallthru   */
        case fsmTERM_Sent:
            /* We are attempting to close connection;            */
            /* wait for timeout to resend a Terminate Request    */
            break;
        };
        break;

    /* **************************************************   */
    case CONFIG_ACK:
        switch(fsm_p->ppp_state) 
        {
        case fsmREQ_Sent:
            if ((*fsm_p->pdc->ack)(fsm_p, &hdr, msg) == 0) 
            {
                PPP_TRANS_STATE(fsm_p, fsmACK_Rcvd);
                DEBUG_LOG("fsm_proc - CONFIG_ACK rcvd - set state to ", LEVEL_3, 
                    EBS_INT1, fsm_p->ppp_state, 0);
            }
            break;

        case fsmACK_Sent:
            if ((*fsm_p->pdc->ack)(fsm_p, &hdr, msg) == 0) 
            {
                fsm_opening(fsm_p);
            }
            break;

        case fsmOPENED:     /* Unexpected event? */
            (*fsm_p->pdc->closing)(fsm_p);
            (*fsm_p->pdc->starting)(fsm_p);
            fsm_reset(fsm_p);
            /* fallthru   */
        case fsmACK_Rcvd:   /* Unexpected event? */
#if (DEBUG_REQ)
            DEBUG_ERROR("OPENED or ACK_RCVD: send REQ", NOVAR, 0, 0);
#endif
            fsm_sendreq(fsm_p);
            PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
            DEBUG_LOG("fsm_proc - CONFIG_ACK rcvd, state = ACK_rcvd - set state to ", LEVEL_3, EBS_INT1, fsm_p->ppp_state, 0);
            break;

        case fsmCLOSED:
        case fsmLISTEN:
            /* Out of Sync; kill the remote   */
            fsm_sendtermack(fsm_p, hdr.id);
            /* fallthru   */
        case fsmTERM_Sent:
            /* We are attempting to close connection;            */
            /* wait for timeout to resend a Terminate Request    */
            break;
        };
        break;

    /* **************************************************   */
    case CONFIG_NAK:
        switch(fsm_p->ppp_state) 
        {
        case fsmREQ_Sent:
        case fsmACK_Sent:
            /* Update our config request to reflect NAKed options   */
            if ((*fsm_p->pdc->nak)(fsm_p, &hdr, msg) == 0) 
            {
                /* Send updated config request   */
                fsm_sendreq(fsm_p);
            }
            break;

        case fsmOPENED:     /* Unexpected event? */
            (*fsm_p->pdc->closing)(fsm_p);
            (*fsm_p->pdc->starting)(fsm_p);
            fsm_reset(fsm_p);
            /* fallthru   */
        case fsmACK_Rcvd:   /* Unexpected event? */
            fsm_sendreq(fsm_p);
            PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
            DEBUG_LOG("fsm_proc - CONFIG_NAK rcvd - set state to ", 
                LEVEL_3, EBS_INT1, fsm_p->ppp_state, 0);
            break;

        case fsmCLOSED:
        case fsmLISTEN:
            /* Out of Sync; kill the remote   */
            fsm_sendtermack(fsm_p, hdr.id);
            /* fallthru   */
        case fsmTERM_Sent:
            /* We are attempting to close connection;            */
            /* wait for timeout to resend a Terminate Request    */
            break;
        };
        break;

    /* **************************************************   */
    case CONFIG_REJ:
        switch(fsm_p->ppp_state) 
        {
        case fsmREQ_Sent:
        case fsmACK_Sent:
            if((*fsm_p->pdc->reject)(fsm_p, &hdr, msg) == 0) 
            {
                fsm_sendreq(fsm_p);
            }
            break;

        case fsmOPENED:     /* Unexpected event? */
            (*fsm_p->pdc->closing)(fsm_p);
            (*fsm_p->pdc->starting)(fsm_p);
            fsm_reset(fsm_p);
            /* fallthru   */
        case fsmACK_Rcvd:   /* Unexpected event? */
            fsm_sendreq(fsm_p);
            PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
            DEBUG_LOG("fsm_proc - CONFIG_REJ rcvd - set state to ", LEVEL_3, 
                EBS_INT1, fsm_p->ppp_state, 0);
            break;

        case fsmCLOSED:
        case fsmLISTEN:
            /* Out of Sync; kill the remote   */
            fsm_sendtermack(fsm_p, hdr.id);
            /* fallthru   */
        case fsmTERM_Sent:
            /* We are attempting to close connection;            */
            /* wait for timeout to resend a Terminate Request    */
            break;
        };
        break;

    /* **************************************************   */
    case TERM_REQ:
        DEBUG_LOG("Peer requested Termination", LEVEL_3, NOVAR, 0, 0);

        switch(fsm_p->ppp_state) 
        {
        case fsmOPENED:
            fsm_sendtermack(fsm_p, hdr.id);
            (*fsm_p->pdc->closing)(fsm_p);
            (*fsm_p->pdc->stopping)(fsm_p);
            fsm_reset(fsm_p);
#if (DEBUG_SIGNAL)
            DEBUG_ERROR("fsm_proc: got TERM_REQ: set signal", NOVAR, 0, 0);
            ppp_signal_set();
#endif
            /* signal xn_ppp_wait_down that connect is terminating   */
            pi->ctrl.signal_status = PPP_SIGNAL_FAIL;   
            OS_SET_PPP_SIGNAL(fsm_p->ppp_p->iface);

            /* callback to application   */
            DEBUG_ERROR("fsm_proc: CALL RS232_CONNECTION_LOST: TERM REQ received", NOVAR, 0, 0);
            CB_RS232_CONNECTION_LOST(fsm_p->ppp_p->iface->ctrl.index);
            break;

        case fsmACK_Rcvd:
        case fsmACK_Sent:
            PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
            DEBUG_LOG("fsm_proc - TERM_REQ rcvd - set state to ", LEVEL_3, 
                EBS_INT1, fsm_p->ppp_state, 0);
            /* fallthru   */
        case fsmREQ_Sent:
        case fsmTERM_Sent:
            /* waiting for timeout   */
            /* fallthru              */
        case fsmCLOSED:
        case fsmLISTEN:
            /* Unexpected, but make them happy   */
            fsm_sendtermack(fsm_p, hdr.id);
            break;
        };
        break;

    /* **************************************************   */
    case TERM_ACK:
        switch(fsm_p->ppp_state) 
        {
        case fsmTERM_Sent:
            ebs_stop_timer(&(fsm_p->timer_info));

            DEBUG_LOG("Terminated", LEVEL_3, NOVAR, 0, 0);
            (*fsm_p->pdc->stopping)(fsm_p);
            fsm_reset(fsm_p);   /* sets state to LISTEN or CLOSED */
            break;

        case fsmOPENED:
            /* Remote host has abruptly closed connection   */
            DEBUG_LOG("Terminated unexpectly", LEVEL_3, NOVAR, 0, 0);
            (*fsm_p->pdc->closing)(fsm_p);
            fsm_reset(fsm_p);
            if ( fsm_sendreq(fsm_p) == 0 ) 
            {
                PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
                DEBUG_LOG("fsm_proc - TERM_ACK rcvd - set state to ", 
                    LEVEL_3, EBS_INT1, fsm_p->ppp_state, 0);
            }
            break;

        case fsmACK_Sent:
        case fsmACK_Rcvd:
            PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
            DEBUG_LOG("fsm_proc - TERM_ACK rcvd - set state to ", 
                LEVEL_3, EBS_INT1, fsm_p->ppp_state, 0);
            /* fallthru   */
        case fsmREQ_Sent:
            /* waiting for timeout   */
            /* fallthru              */
        case fsmCLOSED:
        case fsmLISTEN:
            /* Unexpected, but no action needed   */
            break;
        };
        break;

    /* **************************************************   */
    case CODE_REJ:
        (*fsm_p->pdc->stopping)(fsm_p);
        fsm_reset(fsm_p);
        break;

    /* **************************************************   */
    case PROT_REJ:
        break;

    /* **************************************************   */
    case ECHO_REQ:
        switch(fsm_p->ppp_state) 
        {
        case fsmOPENED:
            if (proc_lcp_echo_request(fsm_p, msg, hdr.id))
                freed = TRUE;
            break;

        case fsmCLOSED:
        case fsmLISTEN:
            /* Out of Sync; kill the remote   */
            fsm_sendtermack(fsm_p, hdr.id);
            /* fallthru   */
        case fsmREQ_Sent:
        case fsmACK_Rcvd:
        case fsmACK_Sent:
        case fsmTERM_Sent:
            /* ignore   */
            break;
        };
        break;

    /* **************************************************   */
    case ECHO_REPLY:
        proc_lcp_echo_reply(pi, msg);
        freed = TRUE;

    case DISCARD_REQ:
    case QUALITY_REPORT:
        break;

    /* **************************************************   */
    default:
        htoncnf( &hdr, msg );   /* put header in beginning of pkt */
                                    /* tbd - probably not necessary   */

        freed = TRUE;
        fsm_send( fsm_p, CODE_REJ, hdr.id, msg );

        switch(fsm_p->ppp_state) 
        {
        case fsmREQ_Sent:
        case fsmACK_Rcvd:
        case fsmACK_Sent:
        case fsmOPENED:
            PPP_TRANS_STATE(fsm_p, fsmLISTEN);
            DEBUG_LOG("fsm_proc - default rcvd - set state to ", 
                LEVEL_3, EBS_INT1, fsm_p->ppp_state, 0);
            break;

        case fsmCLOSED:
        case fsmLISTEN:
        case fsmTERM_Sent:
            /* no change   */
            break;
        };
        break;
    }

    /* ******            */
    /* NOTE: free packet */
    if (!freed)     /* if not freed by send */
        os_free_packet(msg);
}


/***********************************************************************   */
/* TIMER STUFF                                                             */
/***********************************************************************   */

/***********************************************************************  */
/* Set a timer in case an expected event does not occur                   */
void fsm_timer(PFSMS fsm_p)
{
    DEBUG_LOG("fsm_timer() - start timer", LEVEL_3, NOVAR, 0, 0);

    ebs_start_timer( &(fsm_p->timer_info) );
}

/***********************************************************************  */
/* Timeout while waiting for reply from remote host                       */
static void fsm_timeout(void KS_FAR *vp)
{
PFSMS fsm_p = (PFSMS)vp;
PPPPS ppp_p = fsm_p->ppp_p;
PIFACE pi =   ppp_p->iface;

    DEBUG_LOG( "Timeout - fsm_timeout() entered", LEVEL_3, NOVAR, 0, 0);

    switch(fsm_p->ppp_state) 
    {
    case fsmREQ_Sent:
    case fsmACK_Rcvd:
    case fsmACK_Sent:
        if (fsm_p->retry > 0) 
        {
#if (DISPLAY_TIMEOUT)
            DEBUG_ERROR("fsm_timeout - resend - phase, state = ", 
                EBS_INT2, ppp_p->ppp_phase, fsm_p->ppp_state);
#endif
            DEBUG_LOG("        - resend - state = ", LEVEL_3, EBS_INT1, fsm_p->ppp_state, 0);
            fsm_sendreq(fsm_p);
            if (fsm_p->ppp_state != fsmACK_Sent)    /* spr - added to fix bug */
                PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
        } 
        else 
        {
            DEBUG_LOG("Request retry exceeded", LEVEL_3, NOVAR, 0, 0);
            ebs_stop_timer(&(fsm_p->timer_info));
            fsm_reset(fsm_p);
        }
        break;

    case fsmTERM_Sent:
        if (fsm_p->retry > 0) 
        {
            DEBUG_LOG("        - resend - state = ", LEVEL_3, EBS_INT1, 
                fsm_p->ppp_state, 0);
            fsm_sendtermreq(fsm_p);
        }
        else 
        {
            DEBUG_LOG("Terminate retry exceeded", LEVEL_3, NOVAR, 0, 0);
            (*fsm_p->pdc->stopping)(fsm_p);
            fsm_reset(fsm_p);

            /* must not be connected so signal xn_lcp_open   */
#if (DEBUG_SIGNAL)
            DEBUG_ERROR("fsm_timeout: set signal", NOVAR, 0, 0);
            ppp_signal_set();
#endif
            pi->ctrl.signal_status = PPP_SIGNAL_FAIL;   
            OS_SET_PPP_SIGNAL(pi);
        }
        break;

    case fsmCLOSED:
    case fsmLISTEN:
    case fsmOPENED:
        /* nothing to do   */
        break;
    }
}


/***********************************************************************   */
/*          I N I T I A L I Z A T I O N                                    */
/***********************************************************************   */
/* Start FSM (after open event, and physical line up)                      */
void fsm_start(PFSMS fsm_p)
{
    if (!fsm_p->pdv)
        return;

    DEBUG_LOG("fsm_start called - flags = ", LEVEL_3, EBS_INT1, fsm_p->flags, 0);

    if ( !(fsm_p->flags & (FSM_ACTIVE | FSM_PASSIVE)) )
        return;

    switch ( fsm_p->ppp_state ) 
    {
    case fsmCLOSED:
    case fsmLISTEN:
    case fsmTERM_Sent:
        (*fsm_p->pdc->starting)(fsm_p);
        fsm_reset(fsm_p);

        if ( fsm_p->flags & FSM_ACTIVE )
        {
            PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
            DEBUG_LOG("fsm_start() - set state = ", LEVEL_3, 
                EBS_INT1, fsm_p->ppp_state, 0);
            fsm_sendreq(fsm_p);
        }
        break;
    default:
        /* already started   */
        break;
    };
}


/***********************************************************************  */
/* Physical Line Down Event                                               */
void fsm_down(PFSMS fsm_p)
{
    if (!fsm_p->pdv)
        return;

    DEBUG_LOG("Down", LEVEL_3, NOVAR, 0, 0);

    switch ( fsm_p->ppp_state ) 
    {
    case fsmREQ_Sent:
    case fsmACK_Rcvd:
    case fsmACK_Sent:
        ebs_stop_timer(&(fsm_p->timer_info));
        fsm_reset(fsm_p);
        break;

    case fsmOPENED:
        (*fsm_p->pdc->closing)(fsm_p);
        /* fallthru   */
    case fsmTERM_Sent:
        fsm_reset(fsm_p);
        break;

    case fsmCLOSED:
    case fsmLISTEN:
        /* nothing to do   */
        break;
    };
}


/***********************************************************************  */
/* Close the connection - possibly sending a term request                 */
void fsm_close(PFSMS fsm_p)
{
    if (!fsm_p->pdv)
        return;

    DEBUG_LOG("fsm_close - state = ", LEVEL_3, EBS_INT1, fsm_p->ppp_state, 0);

    switch (fsm_p->ppp_state) 
    {
    case fsmOPENED:
        (*fsm_p->pdc->closing)(fsm_p);
        /* fallthru   */
    case fsmACK_Sent:
        PPP_TRANS_STATE(fsm_p, fsmTERM_Sent);
        DEBUG_LOG("fsm_close() - set state = ", LEVEL_3, 
            EBS_INT1, fsm_p->ppp_state, 0);
        fsm_p->retry = fsm_p->try_terminate;
        fsm_sendtermreq(fsm_p);
        break;

    case fsmREQ_Sent:
    case fsmACK_Rcvd:
        /* simply wait for REQ timeout to expire   */
        fsm_p->retry = 0;
        PPP_TRANS_STATE(fsm_p, fsmTERM_Sent);
        DEBUG_LOG("fsm_close() - set state = ", LEVEL_3, EBS_INT1, 
            fsm_p->ppp_state, 0);
        break;

    case fsmLISTEN:
        PPP_TRANS_STATE(fsm_p, fsmCLOSED);
        DEBUG_LOG("fsm_close() - set state = ", LEVEL_3, EBS_INT1, 
            fsm_p->ppp_state, 0);
        break;

    case fsmTERM_Sent:
    case fsmCLOSED:
        /* nothing to do   */
        break;
    };
}


/***********************************************************************  */
/* Initialize the fsm for this protocol
 * Called from protocol _init
 */
void fsm_init(PFSMS fsm_p)
{
PTIMER t;

    DEBUG_LOG("fsm_init()", LEVEL_3, NOVAR, 0, 0);

    fsm_p->try_req = fsm_p->pdc->try_req;
    fsm_p->try_nak = fsm_p->pdc->try_nak;
    fsm_p->try_terminate = fsm_p->pdc->try_terminate;

    /* set retry counters and state to LISTEN or CLOSED            */
    /* fsm_reset(fsm_p);                                           */
    /* do the following instead of calling reset since do not want */
    /* to change the state during init                             */
    /* set the retry counters back to initial values               */
    fsm_p->retry = fsm_p->try_req;
    fsm_p->retry_nak = fsm_p->try_nak;

    /* reset config info for specific protocol   */
    (*fsm_p->pdc->reset)(fsm_p);

    /* Initialize timer   */
    t = (PTIMER)&(fsm_p->timer_info);
    t->func = fsm_timeout;    /* save routine to execute if timeout */
    t->arg = (void KS_FAR *)fsm_p;
    ebs_set_timer(t, fsm_p->pdc->timeout, TRUE);
    fsm_timer(fsm_p);
    ebs_stop_timer(t);
}


void fsm_free(PFSMS fsm_p)
{
    if (fsm_p->pdv) 
    {
        (*fsm_p->pdc->free)(fsm_p);   /* call the protocol free routine */
        fsm_p->pdv = 0;
    }
}

#endif






















