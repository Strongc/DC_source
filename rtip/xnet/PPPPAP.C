/*                                                                       */
/*  PPPPAP.C    -- Password Authentication Protocol for PPP              */
/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */
/*  This implementation of PPP is declared to be in the public domain.   */
/*                                                                       */
/*  Jan 91  Bill_Simpson@um.cc.umich.edu                                 */
/*      Computer Systems Consulting Services                             */
/*                                                                       */
/*  Acknowledgements and correction history may be found in PPP.C        */
/*                                                                       */

#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP


#include "sock.h"
#include "rtip.h"

#if (INCLUDE_PPP && INCLUDE_PAP)

/***********************************************************************  */
/* LOCAL FUNCTIONS                                                        */

static int pap_verify(PFCHAR username, PFCHAR password);
static void pap_shutdown(PFSMS fsm_p);
static void pap_opening(PFSMS fsm_p, int flag);
static void pap_timeout(void KS_FAR *vp);

/***********************************************************************  */
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR welcome_msg[9];
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR invalid_msg[30];

KS_EXTERN_GLOBAL_CONSTANT struct fsm_constant_s KS_FAR pap_constants;

#if (INCLUDE_PAP_SRV && !PAP_FILE)
/***************************************************************************  */
/* Verify user and password sent by remote host                               */
static int pap_verify(PFCHAR username, PFCHAR password)
{
int privs;
char path[CFG_USER_REC_LEN];
int anony = 0;      /* used to set anonymous login */

    /* Verify the password associated with username;
       Use same login as FTP server */
    privs = userlogin(username, password, (PFCHAR)path, CFG_USER_REC_LEN, &anony);

    /* Check privs for this user   */
    if (privs == -1) 
    {
        DEBUG_LOG("ERROR: pap_verify - privs = -1", LEVEL_3, NOVAR, 0, 0);
        return -1;
    }

    if ((privs & PPP_ACCESS_PRIV) == 0) 
    {
        DEBUG_LOG("ERROR: pap_verify - privs & PPP_ACCESS_PRIV = 0", LEVEL_3, NOVAR, 0, 0);
        return -1;
    }
    return 0;
}
#endif /* INCLUDE_PAP_SRV && !PAP_FILE */


/***************************************************************************  */
/* Build a PAP request to send to remote host                                 */
DCU pap_makereq(PFSMS fsm_p)
{                     
PPAPS pap_p = (PPAPS)fsm_p->pdv;
DCU req_msg;
PFBYTE cp;
word total_len;
word len_user, len_pwd;

    DEBUG_LOG("pap_makereq()", LEVEL_3, NOVAR, 0, 0);

    /* get length of data area needed for packet and check if it is   */
    /* too large;                                                     */
    /* NOTE: zero length is ok                                        */
    len_user = (word)tc_strlen(pap_p->username);
    len_pwd = (word)tc_strlen(pap_p->password);
    total_len = (word)(2 + len_user + len_pwd + OUT_PKT_HDR_LEN);
    if ((total_len+1) > MAX_PACKETSIZE) /* make sure '\0' put after  */
                                        /* password will not         */
                                        /* overwrite anything        */
                                        /* (\0 written by strcpy but */
                                        /* is not part of packet)    */
                                        /* overwrite anything        */
    {
        DEBUG_LOG("username/password to long" , LEVEL_3, NOVAR, 0, 0);
        return((DCU)0);
    }

    /* Get buffer for authenticate request packet   */
    if ((req_msg = os_alloc_packet(total_len, PAP_MR_ALLOC)) == (DCU)0)
        return(req_msg);

    /* Load user id and password for authenticate packet            */
    /* NOTE: copies '\0' after password but there is plenty of room */
    cp = DCUTODATA(req_msg);

    /* leave room for ppp and config header headers   */
    cp += OUT_PKT_HDR_LEN;  

    *cp++ = (byte)len_user;
    if ( tc_strlen(pap_p->username) > 0 )
        tc_strcpy((PFCHAR)cp, pap_p->username);

    cp += len_user;
    *cp++ = (byte)len_pwd;
    if ( tc_strlen(pap_p->password) > 0 )
        tc_strcpy((PFCHAR)cp, pap_p->password);

    DCUTOPACKET(req_msg)->length = total_len;
    return(req_msg);
}


/***************************************************************************  */
/* abandon PAP attempt; shutdown LCP layer                                    */
static void pap_shutdown(PFSMS fsm_p)
{
PPPPS ppp_p = fsm_p->ppp_p;

    DEBUG_LOG("pap_shutdown()", LEVEL_3, NOVAR, 0, 0);

    DEBUG_LOG("Failed; close connection", LEVEL_3, NOVAR, 0, 0);

    /* close the connection; possibly send term req depending upon state   */
    fsm_close( &(ppp_p->fsm[Lcp]) );
}


/* Configuration negotiation complete   */
static void pap_opening(PFSMS fsm_p, int flag)
{
register PPPPS ppp_p = fsm_p->ppp_p;

    DEBUG_LOG("pap_opening", LEVEL_3, NOVAR, 0, 0);

    ebs_stop_timer(&(fsm_p->timer_info));

    if ( !((fsm_p->flags &= (word) ~flag) & (PPP_AP_LOCAL | PPP_AP_REMOTE)) ) 
    {
        PPP_TRANS_STATE(fsm_p, fsmOPENED);
    }
    ppp_p->flags &= (word) ~flag;

    /* PAP done; either start IPCP or signal done   */
    ppp_ready(ppp_p);
}


#if (INCLUDE_PAP_SRV && !PAP_FILE)
/***************************************************************************  */
/* Check request from remote host                                             */
static int pap_request(PFSMS fsm_p, PCONFIG_HDR hdr, DCU msg)
{
DCU reply_msg;
int result;
PFCCHAR message;
int mess_length;
PFBYTE username = (PFBYTE)0;
int userlen;
PFBYTE password = (PFBYTE)0;
int passwordlen;
PFBYTE pb;                  /* pointer to data area */
PFBYTE reply_pb;

    DEBUG_LOG("pap_request()", LEVEL_3, NOVAR, 0, 0);

    /* point to option area of the packet   */
    pb = DCUTODATA(msg) + IN_PKT_HDR_LEN;

    /* get the length of the 2 strings   */
    userlen = *pb++;                /* get len and point to username or */
                                    /* next len of password (if userlen is 0)   */
    passwordlen = *(pb+userlen);    /* get len of password */
    
    /* Extract userID/password sent by remote host   */
    if (userlen) 
    {
        username = pb;
        pb += userlen;
        *pb = NULLCHAR;     /* put end of string after username (overwrites */
                            /* len of password   */
    }

    pb++;               /* point to password string; was pointing to length */

    if (passwordlen) 
    {
        password = pb;
        pb += passwordlen;
        *pb = NULLCHAR;
    }

    /* verify attempted logon from remote host   */
    if (pap_verify((PFCHAR)username, (PFCHAR)password) == 0) 
    {
        tc_strcpy(fsm_p->ppp_p->peername, (PFCHAR)username);
        result = CONFIG_ACK;
        message = welcome_msg;
        DEBUG_LOG("pap_request - success - send back CONFIG_ACK", LEVEL_3, NOVAR, 0, 0);
    } 
    else 
    {
        result = CONFIG_NAK;
        message = invalid_msg;
        DEBUG_LOG("pap_request - fail - send back CONFIG_NAK", LEVEL_3, NOVAR, 0, 0);
    }

    /* the space at the beginning of the message is crucial   */
    /* it is replaced with the length of the message          */
    mess_length = tc_strlen(message);
    reply_msg = os_alloc_packet(OUT_PKT_HDR_LEN + mess_length, 
                                PAP_REPLY_ALLOC);
    if (reply_msg)
    {
        reply_pb = DCUTODATA(reply_msg) + OUT_PKT_HDR_LEN;
        tc_strncpy((PFCHAR)reply_pb, message, mess_length);
        *reply_pb = (byte)(mess_length - 1);   /* -1 for space at beginning */

        DCUTOPACKET(reply_msg)->length = (word)(OUT_PKT_HDR_LEN +
                                                (word)mess_length);  
        fsm_send(fsm_p, (byte)result, hdr->id, reply_msg);
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */
    }
    /* tbd - what to do if alloc fails   */

    /* if remote login attempt fails more than allowed, shutdown LCP   */
    if (result == CONFIG_NAK) 
    {
        if ( fsm_p->retry_nak > 0 ) 
        {
            fsm_p->retry_nak--;
        } 
        else 
        {
            /* we failed to authenticate ourselves   */
#if (INCLUDE_POSTMESSAGE)
            PostMessage(fsm_p->ppp_p->hwnd, fsm_p->ppp_p->iface->ctrl.index, PPP_PAP_FAIL, 0);
#endif
            pap_shutdown(fsm_p);
        }
    }
    return (result != CONFIG_ACK);
}
#endif /* INCLUDE_PAP_SRV && !PAP_FILE */


/* Check acknowledgement from remote host   */
static int pap_check(PFSMS fsm_p, PCONFIG_HDR hdr, DCU msg)
{
    DEBUG_LOG("pap_check()", LEVEL_3, NOVAR, 0, 0);

    ARGSUSED_PVOID(msg);

    /* ID field must match last request we sent   */
    if (hdr->id != fsm_p->lastid) 
    {
        DEBUG_LOG("PAP: wrong ID", LEVEL_3, NOVAR, 0, 0);
        return -1;
    }

    return(0);
}


/***********************************************************************   */
/*          E V E N T   P R O C E S S I N G                                */
/***********************************************************************   */

/* Process incoming packet then free it  */
void pap_proc(PFSMS fsm_p, DCU msg)
{
PPAPS pap_p = (PPAPS)fsm_p->pdv;
struct config_hdr hdr;

    if ( ntohcnf(&hdr, msg) == -1 )
    {
        DEBUG_ERROR("short authentication packet", NOVAR, 0, 0);
        os_free_packet(msg);
        return;
    }

    /* Trim off padding   */
    if ((int)hdr.len < DCUTOPACKET(msg)->length)
        DCUTOPACKET(msg)->length = hdr.len;

    switch(hdr.ccode) 
    {
#if (INCLUDE_PAP_SRV && !PAP_FILE)
    case CONFIG_REQ:
        if ( pap_request(fsm_p, &hdr, msg) == 0) 
        {
            pap_opening(fsm_p, PPP_AP_LOCAL);
        }
        break;
#endif

    case CONFIG_ACK:
        if (pap_check(fsm_p, &hdr, msg) == 0) 
        {
            pap_opening(fsm_p, PPP_AP_REMOTE);
        }
        break;

    case CONFIG_NAK:
        if (pap_check(fsm_p, &hdr, msg) == 0) 
        {
            ebs_stop_timer(&(fsm_p->timer_info));

            /* We must have sent a bad username or password   */
            pap_p->username[0] = NULLCHAR;
            pap_p->password[0] = NULLCHAR;

#if (DEBUG_SIGNAL)
            DEBUG_ERROR("pap_proc: set signal", NOVAR, 0, 0);
#endif
            /* signal xn_lcp_open which will fail since state is not open   */
            fsm_p->ppp_p->iface->ctrl.signal_status = PPP_SIGNAL_FAIL;
            OS_SET_PPP_SIGNAL(fsm_p->ppp_p->iface);
            DEBUG_ERROR("PAP login attempt was NAKed", NOVAR, 0, 0);
        }
        break;

    default:
        DEBUG_LOG("PPP/Pap Unknown packet type", LEVEL_3, NOVAR, 0, 0);
        break;
    }
    os_free_packet(msg);
}


/* Timeout while waiting for reply from remote host   */
static void pap_timeout(void KS_FAR *vp) 
{
PFSMS fsm_p = (PFSMS)vp;

    DEBUG_LOG("PAP Timeout" , LEVEL_3, NOVAR, 0, 0);

    if (fsm_p->retry > 0) 
    {
        if (fsm_p->ppp_state == fsmREQ_Sent)
        {
            fsm_sendreq(fsm_p);
        }
    }

    else 
    {
        /* remote failed to authenticate itself   */
        /* os_set_pap_signal(FALSE);              */
        DEBUG_LOG("pap_timeout: Request retry exceeded", LEVEL_3, NOVAR, 0, 0);

#if (INCLUDE_POSTMESSAGE)
        PostMessage(fsm_p->ppp_p->hwnd, fsm_p->ppp_p->iface->ctrl.index, PPP_PAP_FAIL, 0);
#endif

        pap_shutdown(fsm_p);
    }
}


/***********************************************************************   */
/*          I N I T I A L I Z A T I O N                                    */
/***********************************************************************   */

void pap_down(PFSMS fsm_p)
{
PPAPS pap_p = (PPAPS)fsm_p->pdv;

    if (!pap_p)
        return;

    DEBUG_LOG("Down", LEVEL_3, NOVAR, 0, 0);

    fsm_p->flags = FALSE;

    switch ( fsm_p->ppp_state ) 
    {
    case fsmREQ_Sent:
        ebs_stop_timer(&(fsm_p->timer_info));
        /* fallthru   */
    case fsmOPENED:
    case fsmLISTEN:
    case fsmTERM_Sent:
        PPP_TRANS_STATE(fsm_p, fsmCLOSED);
        break;

    case fsmCLOSED:
        /* Already closed; nothing to do   */
        break;
    };
}


void pap_free(PFSMS fsm_p)
{
PPAPS pap_p = (PPAPS)fsm_p->pdv;

    pap_p->username[0] = NULLCHAR;
    pap_p->password[0] = NULLCHAR;
}

/* Allocate configuration structure   */
RTIP_BOOLEAN pap_open(PPPPS ppp_p)
{
PFSMS fsm_p = &(ppp_p->fsm[Pap]);
PIFACE pi;

    DEBUG_LOG("pap_init()", LEVEL_3, NOVAR, 0, 0);

    pi = ppp_p->iface;
    fsm_p->ppp_p = ppp_p;
    fsm_p->pdc = (PFSM_CONSTS)&pap_constants;
    fsm_p->pdv = os_alloc_pap_cb(pi);
    if (fsm_p->pdv)
        return(TRUE);
    else
        return(FALSE);
}


/* Initialize configuration structure   */
void pap_init(PPPPS ppp_p)
{
PFSMS fsm_p = &(ppp_p->fsm[Pap]);
PTIMER t;

    DEBUG_LOG("pap_init()", LEVEL_3, NOVAR, 0, 0);

    fsm_p->try_req = fsm_p->pdc->try_req;
    fsm_p->try_nak = fsm_p->pdc->try_nak;
    fsm_p->try_terminate = fsm_p->pdc->try_terminate;

    PPP_TRANS_STATE(fsm_p, fsmCLOSED);
    fsm_p->retry = fsm_p->try_req;
    fsm_p->retry_nak = fsm_p->try_nak;

    /* Initialize timer   */
    t = &(fsm_p->timer_info);
    t->func = pap_timeout;     /* save routine to execute if timeout */
    t->arg = (void KS_FAR *)fsm_p;
    ebs_set_timer(t, fsm_p->pdc->timeout, TRUE);
    fsm_timer(fsm_p);
    ebs_stop_timer(t);

#    if (INCLUDE_PAP_SRV && !PAP_FILE)
        user_init();
#    endif

}


/* Initialize state machine for local   */
int pap_local(PPPPS ppp_p)
{
PFSMS fsm_p = &(ppp_p->fsm[Pap]);

    DEBUG_LOG("pap_local()", LEVEL_3, NOVAR, 0, 0);

    PPP_TRANS_STATE(fsm_p, fsmLISTEN);
    fsm_p->flags |= PPP_AP_LOCAL;
    ppp_p->flags |= PPP_AP_LOCAL;
    fsm_p->retry = fsm_p->try_req;
    return 0;
}


/* Initialize state machine for remote   */
int pap_remote(PPPPS ppp_p)
{
PFSMS fsm_p = (PFSMS)&(ppp_p->fsm[Pap]);

    DEBUG_LOG("pap_remote()", LEVEL_3, NOVAR, 0, 0);

    PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
    fsm_p->flags |= PPP_AP_REMOTE;
    ppp_p->flags |= PPP_AP_REMOTE;

    /* send pap request                                               */
    /* NOTE: dopap_user must have been called to set up user name and */
    /*       password to use to log onto remote machine               */
    fsm_sendreq(fsm_p);

    return 0;
}

#endif  /* INCLUDE_PPP and INCLUDE_PAP */
