/*                                                                       */
/*  PPPCHAP.C   -- Challenge Handshake Authentication Protocol for PPP   */
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
#include "rtipext.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_MD5  0
#define DEBUG_CHAP 0

#if (INCLUDE_PPP && INCLUDE_CHAP)

/***********************************************************************  */
/* LOCAL FUNCTIONS                                                        */

static void chap_shutdown(PFSMS fsm_p);
static void chap_opening(PFSMS fsm_p, int flag);
void        chap_set_timer(PFSMS fsm_p, int timeout);
static void chap_timeout(void KS_FAR *vp);
int         get_secret(PFCHAR hostname, PFBYTE secret, int *secret_len);
void        init_secrets_table(void);
int         calc_host_len(DCU msg);

/***********************************************************************  */

KS_EXTERN_GLOBAL_CONSTANT char KS_FAR welcome_msg[9];
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR invalid_chap_msg[27];

extern char KS_FAR local_host[CFG_CHAP_NAMELEN+1];

PPP_EXTERN_GLOBAL_CONSTANT struct fsm_constant_s KS_FAR chap_constants;  
extern struct _chap_secrets KS_FAR chap_machine_secrets[CFG_CHAP_SECRETS];


/***************************************************************************  */
/* Build a request (challenge) to send to remote host                         */
DCU chap_makereq(PFSMS fsm_p)
{                     
PCHAPS chap_p = (PCHAPS)fsm_p->pdv;
DCU req_msg;
PFBYTE cp;
word total_len;

    DEBUG_LOG("chap_makereq()", LEVEL_3, NOVAR, 0, 0);

    /* get random length for challenge value; it is at least as   */
    /* large as MD_5 response                                     */
    CB_CHAP_GET_RANDOM_VALUE(&chap_p->md5_value_len, chap_p->md5_value);

    if ( !tc_strlen(local_host) )
    {
        DEBUG_ERROR("chap_makereq: NULL machinename" , NOVAR, 0, 0);
        return (DCU)0;
    }
#if (DEBUG_MD5)
        DEBUG_ERROR("makereq: value = ", PKT, 
            chap_p->md5_value, chap_p->md5_value_len);
#endif

    /* get length of data area needed for packet and check if it is   */
    /* too large (+1 for Value-Size field, ie. chap->md5_value_len)   */
    total_len = (word)(1 + tc_strlen(local_host) + chap_p->md5_value_len + 
                       OUT_PKT_HDR_LEN);
    if ((total_len+1) > MAX_PACKETSIZE) /* make sure '\0' put after  */
                                        /* password will not         */
                                        /* overwrite anything        */
                                        /* (\0 written by strcpy but */
                                        /* is not part of packet)    */
    {
        DEBUG_LOG("chap_makereq: value/name to long" , LEVEL_3, NOVAR, 0, 0);
        return((DCU)0);
    }

    /* Get buffer for authenticate request packet   */
    if ((req_msg = os_alloc_packet(total_len, CHAP_MR_ALLOC)) == (DCU)0)
        return(req_msg);

    /* Load value and name for authenticate packet   */
    cp = DCUTODATA(req_msg);

    /* leave room for ppp and config header headers   */
    cp += OUT_PKT_HDR_LEN;  

    *cp++ = chap_p->md5_value_len;

    if ( chap_p->md5_value_len > 0 )
        tc_movebytes(cp, chap_p->md5_value, chap_p->md5_value_len);

    cp += chap_p->md5_value_len;
    if (local_host[0] != NULLCHAR)
    {
        *cp = NULLCHAR;     /* put eos for next strcpy */
        tc_strcpy((PFCHAR)cp, local_host);
    }

    DCUTOPACKET(req_msg)->length = total_len;   /* pass length to fsm_send */
    return(req_msg);
}


/***************************************************************************  */
/* abandon CHAP attempt; shutdown LCP layer                                   */
static void chap_shutdown(PFSMS fsm_p)
{
PPPPS ppp_p = fsm_p->ppp_p;

    DEBUG_LOG("chap_shutdown()", LEVEL_3, NOVAR, 0, 0);

    DEBUG_LOG("Failed; close connection", LEVEL_3, NOVAR, 0, 0);

    /* close the connection; possibly send term req depending upon state   */
    fsm_close( &(ppp_p->fsm[Lcp]) );
}


/* Configuration negotiation complete   */
static void chap_opening(PFSMS fsm_p, int flag)
{
register PPPPS ppp_p = fsm_p->ppp_p;

    DEBUG_LOG("chap_opening", LEVEL_3, NOVAR, 0, 0);

/*  ebs_stop_timer(&(fsm_p->timer_info));   */

    if ( !((fsm_p->flags &= (word) ~flag) & (PPP_AP_LOCAL | PPP_AP_REMOTE)) ) 
    {
        PPP_TRANS_STATE(fsm_p, fsmOPENED);
    }
    ppp_p->flags &= (word) ~flag;

#if (DEBUG_CHAP)
    DEBUG_ERROR("chap_opening: call ppp_ready", NOVAR, 0, 0);
#endif

    /* CHAP done; either start IPCP or signal done   */
    ppp_ready(ppp_p);
}


/* ************************************************************************   */
/* INPUT PACKET PROCESSING                                                    */
/* ************************************************************************   */

/***************************************************************************  */
/* Send response to a challenge request from remote host; we will
   send a CHALL_RESP with the calculated value */
static int chap_challenge(PFSMS fsm_p, PCONFIG_HDR config_hdr, DCU msg)
{
DCU           reply_msg;
PFBYTE        value;
int           valuelen;
PFBYTE        pb;                   /* pointer to data area */
PFBYTE        reply_pb;
MD5_CTX       mdContext;        
unsigned char digest[MD5_DIGEST_LENGTH];
PFCHAR        hostname; 
int           hostlen;
byte          secret[CFG_MAXSECRETLEN];
int           secret_len;
word          len;

    DEBUG_LOG("chap_challenge()", LEVEL_3, NOVAR, 0, 0);

    /* point to option area of the packet   */
    pb = DCUTODATA(msg) + IN_PKT_HDR_LEN;

    /* get the value   */
    valuelen = *pb;     /* get len and point to username or */
                        /* next len of password (if userlen is 0)   */
    value = pb+1;       /* point to value field of message */

    hostname = (PFCHAR)(pb + valuelen + 1); /* point to machine name of remote host  */

    /* calc length of host name based upon total length   */
    hostlen = calc_host_len(msg);

    /* null terminal the host name (NOTE: the packet is plenty big enough   */
    /* to add a \0 at the end)                                              */
    *(hostname+hostlen) = NULLCHAR;

    if (get_secret(hostname, secret, &secret_len) == 0) 
    {
        MD5_INIT(&mdContext);
        MD5_UPDATE(&mdContext, &(config_hdr->id), 1);
        MD5_UPDATE(&mdContext, secret, secret_len);
        MD5_UPDATE(&mdContext, value, valuelen);
        MD5_FINAL(digest, &mdContext); 

#if (DEBUG_MD5)
        DEBUG_ERROR("value = ", PKT, value, valuelen);
        DEBUG_ERROR("secret = ", PKT, secret, secret_len);
#if (INCLUDE_SSL_MD5)
        DEBUG_ERROR("calc MD5 = ", PKT, digest, MD5_DIGEST_LENGTH);
#else
        DEBUG_ERROR("calc MD5 = ", PKT, mdContext.digest, MD5_SIZE);
#endif
#endif
        len = (word)(OUT_PKT_HDR_LEN + tc_strlen(local_host) + 
                     MD5_DIGEST_LENGTH + 1);

        reply_msg = os_alloc_packet(len, CHAP_REPLYC_ALLOC);
        if (reply_msg)
        {
            reply_pb = DCUTODATA(reply_msg) + OUT_PKT_HDR_LEN;
            *reply_pb = MD5_DIGEST_LENGTH;      /* fill in value_size: md_5 is always 16 */
#if (INCLUDE_SSL_MD5)
            tc_movebytes((PFBYTE)reply_pb+1, (PFBYTE)digest, 
                         MD5_DIGEST_LENGTH);
#else
            tc_movebytes((PFBYTE)reply_pb+1, (PFBYTE)mdContext.digest, 
                         MD5_SIZE);
#endif
            tc_strcpy((PFCHAR)reply_pb+MD5_DIGEST_LENGTH+1, local_host);

            DCUTOPACKET(reply_msg)->length = len;
            fsm_send(fsm_p, CONFIG_RESP, config_hdr->id, reply_msg);
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */
        }
        else
        {
            DEBUG_ERROR("chap_challenge: ran out of DCUs", NOVAR, 0, 0);
        }
    }
    else
    {
        DEBUG_ERROR("chap_challenge: hostname not in secret table:",
            STR1, hostname, 0);
        return (-1);
    }
    return (0);
}


/* Check response from remote host; i.e. verify the value to the 
   challenge is what we would expect; send a CONFIG_SUCC or CONFIG_FAIL
   in response */
static int chap_response(PFSMS fsm_p, PCONFIG_HDR config_hdr, DCU msg)
{
PCHAPS chap_p = (PCHAPS)fsm_p->pdv;
PFBYTE pb;
int hostlen;
DCU reply_msg;
PFBYTE reply_pb;
byte value_len;
MD5_CTX mdContext;
unsigned char digest[MD5_DIGEST_LENGTH];
byte secret[CFG_MAXSECRETLEN];
int secret_len;
PFCHAR hostname; 
word len;

    DEBUG_LOG("chap_response()", LEVEL_3, NOVAR, 0, 0);

    ARGSUSED_PVOID(msg);

    /* ID field must match last request we sent   */
    if (config_hdr->id != fsm_p->lastid) 
    {
        DEBUG_LOG("CHAP: wrong ID", LEVEL_3, NOVAR, 0, 0);
        return -1;
    }

    pb = DCUTODATA(msg) + IN_PKT_HDR_LEN;
    value_len = *pb;

    hostname = (PFCHAR)(pb + value_len + 1);    /* point to machine name of  */
                                                /* remote host     */

    /* calc length of host name based upon total length   */
    hostlen = calc_host_len(msg);

    /* null terminal the host name (NOTE: the packet is plenty big enough   */
    /* to add a \0 at the end)                                              */
    *(hostname+hostlen) = NULLCHAR;

    /* set up response   */
    len = (word)(OUT_PKT_HDR_LEN + tc_strlen(welcome_msg));
    reply_msg = os_alloc_packet(len, CHAP_REPLYR_ALLOC);
    if (reply_msg)
    {
        reply_pb = DCUTODATA(reply_msg) + OUT_PKT_HDR_LEN;

        /* calc exp value   */
        if (get_secret(hostname, secret, &secret_len) == 0) 
        {
            MD5_INIT(&mdContext);
            MD5_UPDATE(&mdContext, &(fsm_p->lastid), 1);
            MD5_UPDATE(&mdContext, secret, secret_len);
            MD5_UPDATE(&mdContext, chap_p->md5_value, chap_p->md5_value_len);
            MD5_FINAL(digest, &mdContext); 
#if (DEBUG_MD5)
            DEBUG_ERROR("value = ", PKT, chap_p->md5_value, chap_p->md5_value_len);
            DEBUG_ERROR("secret = ", PKT, secret, secret_len);
#if (INCLUDE_SSL_MD5)
            DEBUG_ERROR("calc MD5 = ", PKT, digest, MD5_DIGEST_LENGTH);
#else
            DEBUG_ERROR("calc MD5 = ", PKT, mdContext.digest, MD5_SIZE);
#endif
#endif

            /* compare exp value to actual value from MD5 algorithm   */
#if (INCLUDE_SSL_MD5)
            if ( (value_len == MD5_DIGEST_LENGTH) &&
                 tc_comparen(digest, pb+1, MD5_DIGEST_LENGTH) )
#else
            if ( (value_len == MD5_SIZE) &&
                 tc_comparen(mdContext.digest, pb+1, MD5_SIZE) )
#endif
            {
#if (DEBUG_CHAP)
                DEBUG_ERROR("send welcome msg", NOVAR, 0, 0);
#endif
                reply_pb = DCUTODATA(reply_msg) + OUT_PKT_HDR_LEN;
                tc_strcpy((PFCHAR)reply_pb, welcome_msg);

                DCUTOPACKET(reply_msg)->length = len;
                fsm_send(fsm_p, CONFIG_SUCC, config_hdr->id, reply_msg);
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */
                return(0);
            }
        }
        else
        {
            DEBUG_ERROR("secret not found for ", STR1, hostname, 0);
        }

#if (DEBUG_CHAP)
        DEBUG_ERROR("send invalid chap msg", NOVAR, 0, 0);
#endif
        tc_strcpy((PFCHAR)reply_pb, invalid_chap_msg);

        DCUTOPACKET(reply_msg)->length = (word)
            (OUT_PKT_HDR_LEN + tc_strlen(invalid_chap_msg));
        fsm_send(fsm_p, CONFIG_FAIL, config_hdr->id, reply_msg);
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */
        return(1);
    }
    else
    {
        DEBUG_ERROR("chap_response: os_alloc_packet failed", NOVAR, 0, 0);
    }
    return(0);
}


/***********************************************************************   */
/*          E V E N T   P R O C E S S I N G                                */
/***********************************************************************   */

/* Process incoming packet then free it  */
void chap_proc(PFSMS fsm_p, DCU msg, byte ppp_phase)
{
struct config_hdr hdr;
int nxt_chal_time;

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
    case CONFIG_CHAL:
        /* got a challenge   */
        if ( chap_challenge(fsm_p, &hdr, msg) != 0) 
        {
            DEBUG_ERROR("chap_proc: chap_challenge failed", NOVAR, 0, 0);
        }
        break;

    case CONFIG_RESP:
        /* check response to our challange   */
        if (chap_response(fsm_p, &hdr, msg) == 0)   /* succeeded */
        {
            ebs_stop_timer(&(fsm_p->timer_info));

            /* if Authentication phase, then initial challenge;   */
            /* if Network phase, then CHAP was already opened     */
            if (ppp_phase == pppAP)
                chap_opening(fsm_p, PPP_AP_LOCAL);

            /* start timer again so can challenge remote host during   */
            /* network (pppREADY) phase; set to random interval then   */
            /* start the timer                                         */
            /* tbd - interval to challenge?                            */
            nxt_chal_time = (int)(ks_get_ticks() % 8 + 1);
#if (DEBUG_CHAP)
            DEBUG_ERROR("set timer to ", EBS_INT1, nxt_chal_time, 0);
#endif
            chap_set_timer(fsm_p, nxt_chal_time);
            fsm_p->retry = fsm_p->try_req;  /* reset retry counter */
            fsm_timer(fsm_p);

        }
        else        /* failed, i.e. response is invalid */
        {
#if (INCLUDE_POSTMESSAGE)
            PostMessage(fsm_p->ppp_p->hwnd, fsm_p->ppp_p->iface->ctrl.index, PPP_CHAP_FAIL, 0);
#endif
            chap_shutdown(fsm_p);
        }
        break;

    case CONFIG_SUCC:
        /* we responded ok to remotes challenge   */
        chap_opening(fsm_p, PPP_AP_REMOTE);
        break;

    case CONFIG_FAIL:
        DEBUG_ERROR("chap_proc: recvd FAILURE response to our CHALLENGE response",
            NOVAR, 0, 0);

#if (INCLUDE_POSTMESSAGE)
        /* we failed to authenticate ourselves   */
        PostMessage(fsm_p->ppp_p->hwnd, fsm_p->ppp_p->iface->ctrl.index, PPP_CHAP_FAIL, 0);
#endif

#if (!USE_DB_L0)
        /* display failure message from remote host   */
        {
        #define DISP_LEN 60
        int len;
        char disp[DISP_LEN+1];

        len = *(DCUTODATA(msg) + IN_PKT_HDR_LEN-1)-IN_PKT_HDR_LEN;  
        if (len > DISP_LEN)
            len = DISP_LEN;
        tc_strncpy(disp, (PFCHAR)(DCUTODATA(msg) + IN_PKT_HDR_LEN), len);
        disp[len] = '\0';
        DEBUG_ERROR("           message from remote: ", STR1, disp, len);
        }
#endif

        /* the remote host should bring down the connection   */
        break;

    default:
        DEBUG_LOG("PPP/Chap Unknown packet type", LEVEL_3, NOVAR, 0, 0);
        break;
    }
    os_free_packet(msg);
}

/* ***********************************************************************   */
/* TIMER STUFF                                                               */
/* ***********************************************************************   */

/* set the duration of the timeout timer   */
/* NOTE: does not start the timer          */
void chap_set_timer(PFSMS fsm_p, int timeout)
{
PTIMER t;

    t = (PTIMER)&(fsm_p->timer_info);
    ebs_set_timer(t, timeout, TRUE);
}


/* Timeout while waiting for reply to challenge from remote host, i.e. it
   is time to send another challenge */
static dword prev_ticks = 0;
static void chap_timeout(void KS_FAR *vp) 
{
PFSMS fsm_p = (PFSMS)vp;
/* PCHAPS chap_p = (PCHAPS)fsm_p->pdv;   */
dword curr_ticks;

    DEBUG_LOG("CHAP Timeout" , LEVEL_3, NOVAR, 0, 0);
DEBUG_ERROR("CHAP Timeout: timer, timeout" , 
    DINT2, ks_get_ticks(), fsm_p->pdc->timeout);

    curr_ticks = ks_get_ticks();
    prev_ticks = ks_get_ticks();

    /* set the timer duration back to the configuration value; during   */
    /* the network phase it is set to a random number in order to       */
    /* send the next challange, i.e. the timer is used to send the      */
    /* next challenge as well as to do retries                          */
    chap_set_timer(fsm_p, fsm_p->pdc->timeout);

    if (fsm_p->retry > 0) 
    {
        /* state will be REQ_Sent during Authentication phase and   */
        /* it will be OPENED during Network phase                   */
        if ( (fsm_p->ppp_state == fsmREQ_Sent) || 
             (fsm_p->ppp_state == fsmOPENED) )
        {
            fsm_sendreq(fsm_p);
        }
    }
    else 
    {
        /* os_set_ppp_signal(FALSE);    */
        DEBUG_LOG("CHAP timeout: Request retry exceeded", LEVEL_3, NOVAR, 0, 0);
        chap_shutdown(fsm_p);
    }
}


/***********************************************************************   */
/*          I N I T I A L I Z A T I O N                                    */
/***********************************************************************   */

void chap_down(PFSMS fsm_p)
{
PCHAPS chap_p = (PCHAPS)fsm_p->pdv;

    if (!chap_p)
        return;

    DEBUG_LOG("Down", LEVEL_3, NOVAR, 0, 0);

    ebs_stop_timer(&(fsm_p->timer_info));

    fsm_p->flags = FALSE;

    switch ( fsm_p->ppp_state ) 
    {
    case fsmREQ_Sent:
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


void chap_free(PFSMS fsm_p)
{
PCHAPS chap_p = (PCHAPS)fsm_p->pdv;

    chap_p->md5_value[0] = NULLCHAR;
    chap_p->md5_value_len = 0;
}

/* Allocate configuration structure   */
RTIP_BOOLEAN chap_open(PPPPS ppp_p)
{
PFSMS fsm_p = &(ppp_p->fsm[Chap]);
PIFACE pi;

    DEBUG_LOG("chap_open()", LEVEL_3, NOVAR, 0, 0);

    init_secrets_table();

    pi = ppp_p->iface;
    fsm_p->ppp_p = ppp_p;
    fsm_p->pdc = (PFSM_CONSTS)&chap_constants;
    fsm_p->pdv = os_alloc_chap_cb(pi);
    if (fsm_p->pdv)
        return(TRUE);
    else
        return(FALSE);
}


/* Initialize configuration structure   */
void chap_init(PPPPS ppp_p)
{
PFSMS fsm_p = &(ppp_p->fsm[Chap]);
PTIMER t;
int i;

    DEBUG_LOG("chap_init()", LEVEL_3, NOVAR, 0, 0);

    fsm_p->try_req = fsm_p->pdc->try_req;   
    fsm_p->try_nak = fsm_p->pdc->try_nak;
    fsm_p->try_terminate = fsm_p->pdc->try_terminate;

    PPP_TRANS_STATE(fsm_p, fsmCLOSED);
    fsm_p->retry = fsm_p->try_req;
    fsm_p->retry_nak = fsm_p->try_nak;

    /* Initialize timer   */
    t = &(fsm_p->timer_info);
    t->func = chap_timeout;        /* save routine to execute if timeout */
    t->arg = (void KS_FAR *)fsm_p;
    ebs_set_timer(t, fsm_p->pdc->timeout, TRUE);

    /* start then stop the timer??   */
/*  fsm_timer(fsm_p);                */
/*  ebs_stop_timer(t);               */

    for (i=0; i < CFG_CHAP_SECRETS; i++)
        chap_machine_secrets[i].name[0] = NULLCHAR;
}


/* Initialize state machine for local and send chap challenge   */
int chap_local(PPPPS ppp_p)
{
PFSMS fsm_p = &(ppp_p->fsm[Chap]);

    DEBUG_LOG("chap_local()", LEVEL_3, NOVAR, 0, 0);

    PPP_TRANS_STATE(fsm_p, fsmREQ_Sent);
    fsm_p->flags |= PPP_AP_LOCAL;
    ppp_p->flags |= PPP_AP_LOCAL;
    fsm_p->retry = fsm_p->try_req;

    /* send challenge   */
    fsm_sendreq(fsm_p);
    return 0;
}


/* Initialize state machine for remote   */
int chap_remote(PPPPS ppp_p)
{
PFSMS fsm_p = (PFSMS)&(ppp_p->fsm[Chap]);
/* PCHAPS chap_p = (PCHAPS)fsm_p->pdv;   */

    DEBUG_LOG("chap_remote()", LEVEL_3, NOVAR, 0, 0);

    PPP_TRANS_STATE(fsm_p, fsmLISTEN);
    fsm_p->flags |= PPP_AP_REMOTE;
    ppp_p->flags |= PPP_AP_REMOTE;

    return 0;
}


/* ********************************************************************   */
/* SECRETS FOR CHAP                                                       */
/* ********************************************************************   */
void init_secrets_table(void)
{
int i;

    for (i=0; i < CFG_CHAP_SECRETS; i++)
    {
        chap_machine_secrets[i].name[0] = NULLCHAR;
        chap_machine_secrets[i].secret_len = 0;
        chap_machine_secrets[i].secret[0] = NULLCHAR;
    }
}

/* given a hostname, copy its secret and its length into secret and   */
/* secret_len                                                         */
/* Returns 0 if successful, -1 if failure                             */
int get_secret(PFCHAR machine_name, PFBYTE secret, int *secret_len)
{
int i;

#if (CFG_CHAP_SINGLE_SECRET)
    i=0;
    if (chap_machine_secrets[0].name[0]) 
    {
#else
    for (i=0; i < CFG_CHAP_SECRETS; i++)
    {
        if (tc_strcmp(chap_machine_secrets[i].name, machine_name) == 0)
#endif
        {
            *secret_len = chap_machine_secrets[i].secret_len;
            tc_movebytes(secret, chap_machine_secrets[i].secret, *secret_len);
            return(0);
        }
    }
    return(-1);
}

/* Set secret/machine name - secret used for challenging remote host   */
int chap_add_secret(PFCHAR machine_name, PFBYTE secret, int secret_len)
{
int i;
#if (CFG_CHAP_SINGLE_SECRET)
char mn[2];
#endif

    if (!machine_name || !secret)
        return(set_errno(EFAULT));

#if (CFG_CHAP_SINGLE_SECRET)
    if (secret_len > CFG_MAXSECRETLEN)
        return(set_errno(EFAULT));

    /* just put a dummy value in for machine name if one is not   */
    /* specified                                                  */
    if (tc_strlen(machine_name) == 0)
    {
        machine_name = mn;
        machine_name[0] = '*';
        machine_name[1] = '\0';
    }
#endif
    
    if ( (secret_len > CFG_MAXSECRETLEN) || (tc_strlen(machine_name) == 0) )
        return(set_errno(EFAULT));

    for (i=0; i < CFG_CHAP_SECRETS; i++)
    {
        if (chap_machine_secrets[i].name[0] == NULLCHAR)
        {
            tc_strcpy(chap_machine_secrets[i].name, machine_name);
            tc_movebytes(chap_machine_secrets[i].secret, secret, secret_len);
            chap_machine_secrets[i].secret_len = secret_len;
            return(0);
        }
    }
    return(set_errno(EPPPFULL));
}

/* ********************************************************************   */
/* MISC                                                                   */
/* ********************************************************************   */

/* ********************************************************************        */
/* calc_host_len() - calculate length of host name in a challenge or response  */
/*                                                                             */
/*   Calculate length of host name in a challenge or response using the        */
/*   length field in the config header and the Value-Size in the body          */
/*   of the message                                                            */
/*                                                                             */
/*   Returns the length of the host name                                       */
/*                                                                             */
int calc_host_len(DCU msg)
{
PFBYTE pb;
int hostlen;
byte valuelen;

    /* point to option area of the packet   */
    pb = DCUTODATA(msg) + IN_PKT_HDR_LEN;

    /* get the length of value   */
    valuelen = *pb;     /* get len and point to username or */
                        /* next len of password (if userlen is 0)   */
    pb = DCUTODATA(msg) + 2;                /* point to len field  */
                                            /* (skip code + id)   */
    hostlen = net2hs(*((PFWORD)(pb)));      /* len is 2 bytes */
    hostlen = hostlen - IN_PKT_HDR_LEN - valuelen - 1;  /* -1 for value-size */
    return(hostlen);
}
#endif      /* INCLUDE_PPP and INCLUDE_CHAP */


