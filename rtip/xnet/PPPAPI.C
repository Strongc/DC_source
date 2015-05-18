/*                                                                      */
/* PPPAPI.C -- PPP related API commands                                 */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*  This module provides the application programmers interface layer    */
/*  for the PPP implementation.                                         */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

#if (INCLUDE_PPP)

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DISPLAY_TRK_SUCCESS 1   /* display TRK packets if open succeeded */
#define DISPLAY_PPP_CLOSE   0

#define DEBUG_PAP 1

/***************************************************************************  */
int  set_magic(int iface_no, int host, dword magic_seed);
#if (INCLUDE_CHAP)
void init_secrets_table(void);
#endif

/***************************************************************************  */
extern EDEVTABLE KS_FAR ppp_device;
#if (INCLUDE_CHAP)
extern struct _chap_secrets KS_FAR chap_machine_secrets[CFG_CHAP_SECRETS];
extern char KS_FAR local_host[CFG_CHAP_NAMELEN+1];
#endif
#if (INCLUDE_TRK_PPP)
extern int KS_FAR trk_ppp_off;
#endif

/***************************************************************************  */
/* EXTERNAL FUNCTIONS                                                         */
RTIP_BOOLEAN ppp_xmit_escape(PRS232_IF_INFO pif_info);
void         ppp_give_string(PRS232_IF_INFO pif_info, PFBYTE buffer, int n);


/***************************************************************************  */
static void KS_FAR *get_pdv(int iface_no, int fsmi)             
{                                                   
PIFACE pi;                                      
PPPPS ppp_p;
                                                    
    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi || !pi->edv)
        return((void KS_FAR *)0);

    ppp_p = pi->edv;
    return(ppp_p->fsm[fsmi].pdv);
}

struct lcp_side_s KS_FAR *get_side_p_lcp(int iface_no, int host)            
{                                                   
struct lcp_s KS_FAR *lcp_p;

    lcp_p = (struct lcp_s KS_FAR *)get_pdv(iface_no, Lcp);                                              
    if (lcp_p == 0)
        return(0);      /* FAILURE */

    if (host == LOCAL_HOST)
        return((struct lcp_side_s KS_FAR *)&lcp_p->local_entry);        
    else if (host == REMOTE_HOST)
        return((struct lcp_side_s KS_FAR *)&lcp_p->remote_entry);       
    else
        return((struct lcp_side_s KS_FAR *)0);
}


static struct ipcp_side_s KS_FAR *get_side_p_ipcp(int iface_no, int host)           
{                                                   
struct ipcp_s KS_FAR *ipcp_p;

    ipcp_p = (struct ipcp_s KS_FAR *)get_pdv(iface_no, IPcp);                                               
    if (ipcp_p == 0)
        return(0);      /* FAILURE */

    if (host == LOCAL_HOST)
        return((struct ipcp_side_s KS_FAR *)&ipcp_p->local_entry);      
    else if (host == REMOTE_HOST)
        return((struct ipcp_side_s KS_FAR *)&ipcp_p->remote_entry);     
    else
        return((struct ipcp_side_s KS_FAR *)0);
}


/***************************************************************************  */
/* GLOBAL FUNCTIONS                                                           */

/***********************************************************************   */
/* SET/RESET LCP and IPCP ON NEGOTIATION OPTIONS                           */
/***********************************************************************   */

/* ********************************************************************              */
/* xn_lcp_set_option() -  Set or reset a LCP option as negotiable                    */
/*                                                                                   */
/* Summary:                                                                          */
/*   #include "ppp.h"                                                                */
/*                                                                                   */
/*   int xn_lcp_set_option(iface_no, host, option, turn_on)                          */
/*      int iface_no        - Interface number (see rtip.h)                          */
/*      int host            - Set negotiation flag for local or remote host          */
/*                            (LOCAL_HOST or REMOTE_HOST)                            */
/*      int option          - Option to set or reset                                 */
/*                            (LCP_MRU, LCP_ACCM, LCP_AUTHENT,                       */
/*                             LCP_PFC, LCP_ACFC, LCP_MAGIC)                         */
/*      RTIP_BOOLEAN turn_on        - Turn option on if TRUE, otherwise, turn it off */
/*                                                                                   */
/* Description:                                                                      */
/*                                                                                   */
/*   This function establishes whether one of the following LCP options is           */
/*   negotiable if requested by the remote host: Maximum-Receive-Unit,               */
/*   Async-Control-Character-Map, Authentication-Type,                               */
/*   Protocol-Field-Compression, Address-and-Control-Field-Compression and           */
/*   Magic-Number.                                                                   */
/*                                                                                   */
/*   In order to accept the specified option sent from the remote host in a          */
/*   LCP Configuration-NAK, call this routine with host set to LOCAL_HOST and        */
/*   turn_on set to TRUE.  In order to reject the option sent from the               */
/*   remote host in a Configuration-NAK, call this routine with host                 */
/*   set to REMOTE_HOST and turn_on set to FALSE.                                    */
/*                                                                                   */
/*   In order to accept a LCP Configuration-Request for the option from the          */
/*   remote host for the specified option, call this routine with host set to        */
/*   remote host and turn_on set to TRUE.  In order to reject the option             */
/*   sent in a LCP Configuration-Request, call this routine with host                */
/*   set to REMOTE_HOST and turn_on set to FALSE.                                    */
/*                                                                                   */
/*   The default for both LOCAL_HOST and REMOTE_HOST is all supported                */
/*   options are accepted.                                                           */
/*                                                                                   */
/*   For more details see PPP User's Manual.                                         */
/*                                                                                   */
/* Returns:                                                                          */
/*   Zero is returned on success. Otherwise -1                                       */
/*                                                                                   */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by       */
/*   this function.  For error values returned by this function see                  */
/*   PPP User's Manual.                                                              */
/*                                                                                   */

int xn_lcp_set_option(int iface_no, int host, int option, RTIP_BOOLEAN turn_on)             
{                                                   
struct lcp_side_s KS_FAR *side_p;                       

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    side_p = get_side_p_lcp(iface_no, host);

    if (side_p)
    {
        if (turn_on)
        {
            side_p->will_negotiate |= (word)(1 << option);              
            return(0);
        }
        else 
        {
            side_p->will_negotiate &= (word)(~(1 << option));               
            return(0);
        }
    }
        
    return(-1);
}

/* ********************************************************************              */
/* xn_ipcp_set_option() -  Set or reset a IPCP option as negotiable                  */
/*                                                                                   */
/* Summary:                                                                          */
/*   #include "ppp.h"                                                                */
/*                                                                                   */
/*   int xn_ipcp_set_option(iface_no, host, option, turn_on)                         */
/*      int iface_no        - Interface number (see rtip.h)                          */
/*      int host            - Set negotiation flag for local or remote host          */
/*                            (LOCAL_HOST or REMOTE_HOST)                            */
/*      int option          - Option to set or reset                                 */
/*                            (IPCP_ADDRESS, IPCP_ADDRESSES, IPCP_COMPRESS)          */
/*      RTIP_BOOLEAN turn_on        - Turn option on if TRUE, otherwise, turn it off */
/*                                                                                   */
/* Description:                                                                      */
/*                                                                                   */
/*   This function establishes whether one of the following IPCP options is          */
/*   negotiable if requested by the remote host: IP_Address,                         */
/*   IP-Addresses and Compression-Type.                                              */
/*                                                                                   */
/*   In order to accept the specified option sent from the remote host in a          */
/*   IPCP Configuration-NAK, call this routine with host set to LOCAL_HOST and       */
/*   turn_on set to TRUE.  In order to reject the option sent from the               */
/*   remote host in a Configuration-NAK, call this routine with host                 */
/*   set to REMOTE_HOST and turn_on set to FALSE.                                    */
/*                                                                                   */
/*   In order to accept a IPCP Configuration-Request for the option from the         */
/*   remote host for the specified option, call this routine with host set to        */
/*   REMOTE_HOST and turn_on set to TRUE.  In order to reject the option             */
/*   sent in a IPCP Configuration-Request, call this routine with host               */
/*   set to REMOTE_HOST and turn_on set to FALSE.                                    */
/*                                                                                   */
/*   The default for both LOCAL_HOST and REMOTE_HOST is all supported                */
/*   options are accepted.                                                           */
/*                                                                                   */
/*   For more details see PPP User's Manual.                                         */
/*                                                                                   */
/* Returns:                                                                          */
/*   Zero is returned on success. Otherwise -1                                       */
/*                                                                                   */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by       */
/*   this function.  For error values returned by this function see                  */
/*   PPP User's Manual.                                                              */
/*                                                                                   */

int xn_ipcp_set_option(int iface_no, int host, int option, RTIP_BOOLEAN turn_on)            
{                                                   
struct ipcp_side_s KS_FAR *side_p;                      

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    side_p = get_side_p_ipcp(iface_no, host);

    if (side_p)
    {
        if (turn_on)
        {
            side_p->will_negotiate |= (word)(1 << option);              
            return(0);
        }
        else
        {
            side_p->will_negotiate &= (word)(~(1 << option));               
            return(0);
        }
    }
        
    return(-1);
}


/***********************************************************************   */
/* SET NEGOTIATION VALUES WANTED;                                          */
/*     These macro will set the option value wanted.  They will turn       */
/*     want.negotiate on if value is not default or off it it matches.     */
/***********************************************************************   */


/* ********************************************************************        */
/* xn_lcp_want_accm - Set Character Map to negotiate                           */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_lcp_want_accm(iface_no, host, opt_val)                             */
/*      int iface_no        - Interface number (see rtip.h)                    */
/*      int host            - Set negotiation flag for local or remote host    */
/*                            (LOCAL_HOST or REMOTE_HOST)                      */
/*      dword opt_val       - Async control character map                      */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function establishes the LPC Character Map option should be          */
/*   requested and Character Map to request.                                   */
/*                                                                             */
/*   In order to request a LCP Async-Control Character-Map,                    */
/*   call this routine with host set to LOCAL_HOST.  If value specifed is      */
/*   different than the default map specified in RFC 1172 then the             */
/*   ACCM option will be requested.  The ACCM option will not be requested     */
/*   if the map is the same as the default.  If the remote host                */
/*   rejects the ACCM option requested in the Configuration-Request message,   */
/*   the option will be dropped.                                               */
/*                                                                             */
/*   In order to request an alternative LCP Async-Control Character-Map        */
/*   in response to receiving a request from the remote host, or to            */
/*   suggest ACCM in a CONFIG-NAK if the REQUEST does not contain the option,  */
/*   call this routine with host set to REMOTE_HOST.  If value specifed is     */
/*   different than the default map specified in RFC 1172 then the alternative */
/*   ACCM option will be requested.  The option                                */
/*   will be dropped after CFG_LCP_NAK_TRY retries if the                      */
/*   Configuration-Request received in response to the Configuration-Nak       */
/*   does not request the option.                                              */
/*                                                                             */
/*   The default is to not request this option.                                */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_lcp_want_accm(int iface_no, int host, dword opt_val)         
{                                                   
struct lcp_side_s KS_FAR *side_p;                       

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif
                                        
    side_p = get_side_p_lcp(iface_no, host);
    if (side_p)
    {
        side_p->want.accm = opt_val;                    
        if ( side_p->want.accm != CFG_LCP_ACCM_DFLT )   
            side_p->want.negotiate |= LCP_N_ACCM;       
        else                                            
            side_p->want.negotiate &= ~LCP_N_ACCM;  
        return(0);
    }
    return(-1);
}

/* ********************************************************************        */
/* xn_lcp_want_pap - Set authentication protocol to negotiate to PAP           */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_lcp_want_pap(iface_no, host)                                       */
/*      int iface_no        - Interface number (see rtip.h)                    */
/*      int host            - Set negotiation flag for local or remote host    */
/*                            (LOCAL_HOST or REMOTE_HOST)                      */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function establishes LCP Authentication should be requested and      */
/*   Password Authentication Protocol, PAP, should be the protocol to          */
/*   request.                                                                  */
/*                                                                             */
/*   In order to mandate that the remote host must authenticate itself         */
/*   where PAP is an acceptable protocol to authenticate itself with,          */
/*   call this routine with host set to LOCAL_HOST.  In order to setup         */
/*   a user account, the function xn_pap_add_user() needs to be called.        */
/*   If CHAP is also mandated, CHAP will be negotiated first.  If              */
/*   CHAP is NAKed, PAP will be requested.                                     */
/*                                                                             */
/*   In order to request PAP in response to a request from the remote host     */
/*   for an unknown protocol or if CHAP is requested and not configured,       */
/*   call this routine with host set to REMOTE_HOST.  In order to set          */
/*   logon username and password, the function xn_pap_user() needs to          */
/*   be called.                                                                */
/*                                                                             */
/*   Internally, this routine sets the negotiation flag as well as the         */
/*   authentication value to PAP.                                              */
/*                                                                             */
/*   The LCP Configuration-Request message will contain the PAP option if      */
/*   this function is called with host set to LOCAL_HOST.  The link will       */
/*   not become established if the remote host rejects the LCP Configuration   */
/*   Request for PAP or the user name and password in the PAP                  */
/*   Configuration-Request message received from the remote host during        */
/*   the authentication phase is invalid.                                      */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_lcp_want_pap(int iface_no, int host)                     
{                                                   
struct lcp_side_s KS_FAR *side_p;                       

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    side_p = get_side_p_lcp(iface_no, host);
    if (side_p)
    {
#if (!PRECEDENCE_PAP)
        /* give precedence to CHAP; if remote requests PAP then   */
        /* we will accept PAP                                     */
        if (side_p->want.authentication != PPP_CHAP_PROTOCOL)
#endif
        {
            side_p->want.negotiate |= LCP_N_AUTHENT;        
            side_p->want.authentication = PPP_PAP_PROTOCOL;
        }
        side_p->will_negotiate |= LCP_WANT_PAP;
        return(0);
    }
    return(-1);
}

#if (INCLUDE_CHAP)
/* ********************************************************************           */
/* xn_lcp_want_chap - Set authentication protocol to negotiate to CHAP            */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "ppp.h"                                                             */
/*                                                                                */
/*   int xn_lcp_want_chap(iface_no, host)                                         */
/*      int iface_no        - Interface number (see rtip.h)                       */
/*      int host            - Set negotiation flag for local or remote host       */
/*                            (LOCAL_HOST, REMOTE_HOST)                           */
/*                                                                                */
/* Description:                                                                   */
/*                                                                                */
/*   This function establishes LCP Authentication should be requested and         */
/*   Challange-Handshake, CHAP, should be the protocol to request.                */
/*                                                                                */
/*   Call this routine with host set to LOCAL_HOST, to mandate that the           */
/*   remote host must authenticate itself where CHAP is the authentication        */
/*   protocol. If CHAP is NAKed and PAP is also configured, a new request will be */
/*   sent with PAP requested. In order to setup a secret, xn_chap_add_secret      */
/*   needs to be called.  In order to setup the local machine name,               */
/*   xn_chap_machine_name needs to be called.                                     */
/*                                                                                */
/*   Call this routine with host set to REMOTE_HOST to accept CHAP if it is       */
/*   requested by the remote host or suggest CHAP in a Configuration-Nak          */
/*   in response to a request from the remote host for an unknown protocol        */
/*   or in response to a Password Authenticaion Protocol (PAP) request where      */
/*   CHAP is not configured.  In order to setup a secret, xn_chap_add_secret      */
/*   needs to be called.  In order to setup the local machine name,               */
/*   xn_chap_machine_name needs to be called.                                     */
/*                                                                                */
/*   Internally, this routine sets the negotiation flag as well as the            */
/*   authentication value to PAP.                                                 */
/*                                                                                */
/*   The LCP Configuration-Request message will contain the PAP option if         */
/*   this function is called with host set to LOCAL_HOST.  The link will          */
/*   not become established if the remote host rejects the LCP Configuration      */
/*   Request for PAP or the user name and password in the PAP                     */
/*   Configuration-Request message received from the remote host during           */
/*   the authentication phase is invalid.                                         */
/*                                                                                */
/*   For more details see PPP User's Manual.                                      */
/*                                                                                */
/* Returns:                                                                       */
/*   Zero is returned on success. Otherwise -1                                    */
/*                                                                                */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by    */
/*   this function.  For error values returned by this function see               */
/*   PPP User's Manual.                                                           */
/*                                                                                */

int xn_lcp_want_chap(int iface_no, int host)                        
{                                                   
struct lcp_side_s KS_FAR *side_p; 

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    side_p = get_side_p_lcp(iface_no, host);
    if (side_p)
    {
#if (PRECEDENCE_PAP)
        /* give precedence to CHAP; if remote requests PAP then   */
        /* we will accept PAP                                     */
        if (side_p->want.authentication != PPP_PAP_PROTOCOL)
#endif
        {
            side_p->want.negotiate |= LCP_N_AUTHENT;        
            side_p->want.authentication = PPP_CHAP_PROTOCOL;
        }
        side_p->want.chap_algorithm = MD5_ALGORITHM;
        side_p->will_negotiate |= LCP_WANT_CHAP_MD5;
        return(0);
    }
    return(-1);
}
#endif


/* ********************************************************************              */
/* xn_lcp_want_option - Set/Reset LCP negotiation for various options                */
/*                                                                                   */
/* Summary:                                                                          */
/*   #include "ppp.h"                                                                */
/*                                                                                   */
/*   int xn_lcp_want_option(iface_no, host, option, turn_on)                         */
/*      int iface_no        - Interface number (see rtip.h)                          */
/*      int host            - Set negotiation flag for local or remote host          */
/*                            (LOCAL_HOST or REMOTE_HOST)                            */
/*      int option          - Option to negotiate                                    */
/*                            (LCP_ACFC, LCP_MAGIC (off only), LCP_PFC)              */
/*      RTIP_BOOLEAN turn_on        - Turn option on if TRUE, otherwise, turn it off */
/*                                                                                   */
/* Description:                                                                      */
/*                                                                                   */
/*   This function establishes whether to request the following LCP options:         */
/*   Address-and-Control-Field-Compression, Protocol-Field-Compression and           */
/*   Magic Number (off only).                                                        */
/*                                                                                   */
/*   In order to request Address-and-Control-Field-Compression or Protocol-          */
/*   Field-Compression options, call this function with the option set               */
/*   appropriately, turn_on set to TRUE and host set to LOCAL_HOST.  The             */
/*   option will be requested in the Configuration-Request message sent              */
/*   to the remote host.  The option will be dropped if it is rejected               */
/*   by the remote host.                                                             */
/*                                                                                   */
/*   In order to suggest an option (Address-and-Control-Field-Compression or         */
/*   Protocol-Field-Compression) if it is not requested by the                       */
/*   remote host, call this function with the option set appropriately,              */
/*   turn_on set to TRUE and host set to REMOTE_HOST.  The option will be            */
/*   suggested in a Configuration-Nak sent to the remote host.  The option           */
/*   will be dropped after CFG_LCP_NAK_TRY retries if the                            */
/*   Configuration-Request received in response to the Configuration-Nak             */
/*   does not request the option.                                                    */
/*                                                                                   */
/*   The default value is not to request the option.                                 */
/*                                                                                   */
/*   For more details see PPP User's Manual.                                         */
/*                                                                                   */
/* Returns:                                                                          */
/*   Zero is returned on success. Otherwise -1                                       */
/*                                                                                   */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by       */
/*   this function.  For error values returned by this function see                  */
/*   PPP User's Manual.                                                              */
/*                                                                                   */

int xn_lcp_want_option(int iface_no, int host, int option, RTIP_BOOLEAN turn_on)
{                                                   
struct lcp_side_s KS_FAR *side_p;                       

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    if ( (option != LCP_ACFC) && (option != LCP_MAGIC) && 
         (option != LCP_PFC) )
        return(-1);

    if ( (option == LCP_MAGIC) && turn_on )
        return(-1);

    side_p = get_side_p_lcp(iface_no, host);
    if (side_p)
    {
        side_p->want.negotiate |= (word)(1 << option);      
        return(0);
    }
    return(-1);
}


/* ********************************************************************        */
/* xn_lcp_want_magic - Set magic number to negotiate                           */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_lcp_want_magic(iface_no, local_seed, remote_seed)                  */
/*      int iface_no        - Interface number (see rtip.h)                    */
/*      dword local_seed    - Seed to use for random number generator          */
/*                            for calculating magic number when                */
/*                            sending requests                                 */
/*      dword remote_seed   - Seed to use for random number generator          */
/*                            for calculating magic number when                */
/*                            sending naks when loop-back link is detected     */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function establishes the LCP Magic-Number option should be           */
/*   requested and the value to request.                                       */
/*                                                                             */
/*   The default value is not to request the Magic-Number option.              */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_lcp_want_magic(int iface_no, dword local_seed, dword remote_seed)        
{                                                   

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    if (set_magic(iface_no, LOCAL_HOST, local_seed) < 0)
        return(-1);
    if (set_magic(iface_no, REMOTE_HOST, remote_seed) < 0)
        return(-1);

    return(0);
}

int set_magic(int iface_no, int host, dword magic_seed)
{
struct lcp_side_s KS_FAR *side_p;                       

    side_p = get_side_p_lcp(iface_no, host);
    if (side_p)
    {
        side_p->want.magic_number = magic_seed;         
        side_p->want.negotiate |= LCP_N_MAGIC;
        return(0);
    }
    return(-1);
}


/* ********************************************************************        */
/* xn_lcp_want_mru - Set mru value to negotiate                                */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_lcp_want_mru(iface_no, host, opt_val, negotiable)                  */
/*      int iface_no        - Interface number (see rtip.h)                    */
/*      int host            - Set negotiation flag for local or remote host    */
/*                            (LOCAL_HOST or REMOTE_HOST)                      */
/*      dword opt_val       - MRU value                                        */
/*      RTIP_BOOLEAN negotiable - set to TRUE if the MRU is negotiable, i.e.   */
/*                            if will accept what remote host says the MRU     */
/*                            is; set to FALSE if the MRU is not negotiable    */
/*                            i.e. if will accept what the remote host         */
/*                            says only if the MRU value is less than          */
/*                            or equal to the requested value                  */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function establishes the LCP Maximum-Receive-Unit option (maximum    */
/*   segment size) should be requested and the value to request.               */
/*                                                                             */
/*   In order to request a LCP Maximum-Receive-Unit (maximum segment size),    */
/*   call this routine with host set to LOCAL_HOST.  If value specifed is      */
/*   different than the default map specified in RFC 1172 then the             */
/*   MRU option will be requested.  The MRU option will not be requested       */
/*   if the map is the same as the default.  The option will be dropped if     */
/*   the remote host rejects the MRU option requested.                         */
/*                                                                             */
/*   In order to request a an alternative LCP Maximum-Receive-Unit             */
/*   (maximum segment size) if the MRU is not requested by the remote          */
/*   host or the MRU does not match the desired value, call this               */
/*   routine with host set to REMOTE_HOST.  If value specifed is               */
/*   different than the default map specified in RFC 1172 then the             */
/*   alternative MRU option will be requested.  The option                     */
/*   will be dropped after CFG_LCP_NAK_TRY retries if the                      */
/*   Configuration-Request received in response to the Configuration-Nak       */
/*   does not request the option.                                              */
/*                                                                             */
/*   The default is to not request the MRU option.                             */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_lcp_want_mru(int iface_no, int host, int opt_val, RTIP_BOOLEAN negotiable)           
{                                                   
struct lcp_side_s KS_FAR *side_p;                       

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    if (opt_val >= CFG_LCP_MRU_LO && opt_val <= CFG_LCP_MRU_HI) 
    {                                                   
        side_p = get_side_p_lcp(iface_no, host);
        if (side_p)
        {
            side_p->want.mru = (word)opt_val;                   
            if ( side_p->want.mru != CFG_LCP_MRU_DFLT )     
                side_p->want.negotiate |= LCP_N_MRU;        
            else                                            
                side_p->want.negotiate &= ~LCP_N_MRU;       
            if (negotiable)
                side_p->will_negotiate |= LCP_MRU_NEGOTIABLE;
            return(0);
        }  
    }
    return(-1);
}


/***********************************************************************   */
/* SET IP ADDRESS FOR IPCP                                                 */
/***********************************************************************   */

/* ********************************************************************           */
/* xn_ipcp_want_compress - Set IPCP negotiation for compression                   */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "ppp.h"                                                             */
/*                                                                                */
/*   int xn_ipcp_want_compress(iface_no, host, num_slots)                         */
/*      int iface_no        - Interface number (see rtip.h)                       */
/*      int host            - Set negotiation flag for local or remote host       */
/*                            (LOCAL_HOST or REMOTE_HOST)                         */
/*      int num_slots       - Number of slots to send in request                  */
/*                                                                                */
/* Description:                                                                   */
/*                                                                                */
/*   This function establishes whether to request the following IPCP options:     */
/*   Van Jacobson Compression.                                                    */
/*                                                                                */
/*   In order to request the Van Jacobson Compression, call this function         */
/*   with host set to LOCAL_HOST.  The option will be requested in the            */
/*   Configuration-Request message sent to the remote host.  The option will      */
/*   be dropped if it is rejected by the remote host.                             */
/*                                                                                */
/*   In order to suggest an option if it is not requested by the                  */
/*   remote host, call this function with host set to REMOTE_HOST.  The option    */
/*   will be suggested in a Configuration-Nak sent to the remote host.            */
/*   The option will be dropped if the Configuration-Request received in response */
/*   to the Configuration-Nak does not request the option.                        */
/*                                                                                */
/*   The default value is not to request the option.                              */
/*                                                                                */
/*   For more details see PPP User's Manual.                                      */
/*                                                                                */
/* Returns:                                                                       */
/*   Zero is returned on success. Otherwise -1                                    */
/*                                                                                */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by    */
/*   this function.  For error values returned by this function see               */
/*   PPP User's Manual.                                                           */
/*                                                                                */

int xn_ipcp_want_compress(int iface_no, int host, int num_slots)  
{                                                   
struct ipcp_side_s KS_FAR *side_p;                      

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    side_p = get_side_p_ipcp(iface_no, host);
    if (side_p)
    {
        if (host == LOCAL_HOST) 
        {
            side_p->want.slots = (byte)num_slots;   /* tbd: range checking??? */
            side_p->want.slot_compress = 1;
        }   
        side_p->want.negotiate |= (word)(1 << IPCP_COMPRESS);       
        side_p->will_negotiate |= IPCP_N_COMPRESS;
        return(0);
    }
    return(-1);
}

/* ********************************************************************          */
/* xn_ipcp_want_address - Set IP address to negotiate for IPCP                   */
/*                                                                               */
/* Summary:                                                                      */
/*   #include "ppp.h"                                                            */
/*                                                                               */
/*   int xn_ipcp_want_address(iface_no, host, ip_address, negotiable)            */
/*      int iface_no        - Interface number (see rtip.h)                      */
/*      int host            - Set negotiation flag for local or remote host      */
/*                            (LOCAL_HOST or REMOTE_HOST)                        */
/*      PFCBYTE ip_address  - IP address to negotiate                            */
/*      RTIP_BOOLEAN negotiable - set to TRUE if the address is negotiable, i.e. */
/*                            if will accept what remote host says the address   */
/*                            is; set to FALSE if the address is not negotiable  */
/*                            i.e. the remote host must accept this address      */
/*                                                                               */
/* Description:                                                                  */
/*                                                                               */
/*   This function establishes the IPCP IP address option should be requested    */
/*   and the IP address to request.                                              */
/*                                                                               */
/*   In order to request a local IP address, call this routine with host set     */
/*   to LOCAL_HOST.  The option will be dropped if the remote host rejects       */
/*   the IPCP address requested.  If negotiable is set, an address suggested     */
/*   by the remote host in a NAK message will be used.  If negotiable is         */
/*   set to FALSE, a NAK message with a different IP address will be             */
/*   rejected.                                                                   */
/*                                                                               */
/*   In order to request an alternative address for the remote host or           */
/*   to suggest an address if the remote host does not request the option,       */
/*   call this routine with host set to REMOTE_HOST.  If negotiable is           */
/*   set to TRUE, the address sent in a REQUEST will be used (if it is           */
/*   not 0).  If negotiable is set to FALSE, a NAK message will be               */
/*   sent if the address in the message does not match the address               */
/*   specified by calling xn_ipcp_want_address.  In either case, the address     */
/*   specifed by xn_ipcp_want_address will be sent in a NAK message is a         */
/*   REQUEST is received with a 0 address.                                       */
/*                                                                               */
/*   Care should be taken to ensure this address is acceptable to the            */
/*   remote host if negotiation parameter is FALSE, otherwise, negotiation       */
/*   will never converge.                                                        */
/*                                                                               */
/*   The default is to not request the IPCP option.                              */
/*                                                                               */
/*   In order to request the an address from the remote host, set address to     */
/*   all 0s.                                                                     */
/*                                                                               */
/*   If this routine (and xn_want_ipcp_pool for REMOTE_HOST only) are not        */
/*   called, the IP addresses specified in xn_set_ip() and xn_attach()           */
/*   local and remote addresses respectively, will be sent to the otherside      */
/*   if a message is received requesting these addresses, i.e. if a null         */
/*   address is received.                                                        */
/*                                                                               */
/*   In order to extract the IP addresses negotiated, xn_interface_info() can    */
/*   be called after xn_lcp_open().                                              */
/*                                                                               */
/*   For more details see PPP User's Manual.                                     */
/*                                                                               */
/* Returns:                                                                      */
/*   Zero is returned on success. Otherwise -1                                   */
/*                                                                               */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by   */
/*   this function.  For error values returned by this function see              */
/*   PPP User's Manual.                                                          */
/*                                                                               */

int xn_ipcp_want_address(int iface_no, int host, PFCBYTE ip_address, RTIP_BOOLEAN negotiable)
{
struct ipcp_side_s KS_FAR *side_p;                      
struct ipcp_s KS_FAR *ipcp_p;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    ipcp_p = (struct ipcp_s KS_FAR *)get_pdv(iface_no, IPcp);                                               
    side_p = get_side_p_ipcp(iface_no, host);

    if (side_p && ipcp_p)
    {
        if (host == REMOTE_HOST)
        {
            /* set pool to one entry   */
            tc_mv4(ipcp_p->peer_min,  ip_address, IP_ALEN);
            tc_mv4(ipcp_p->peer_curr, ip_address, IP_ALEN);
            tc_mv4(ipcp_p->peer_max,  ip_address, IP_ALEN);
        }
        else
            tc_mv4(side_p->want.address, ip_address, IP_ALEN);

        /* set flag so will negotiate -                              */
        /* NOTE: if rejects ADDRESS, ADDRESSES option will be tried  */
        side_p->want.negotiate |= IPCP_N_ADDRESS;

        if (negotiable)
            side_p->will_negotiate |= IPCP_ADDRESS_NEGOTIABLE;

        return 0;
    }
    DEBUG_LOG("after xn_ipcp_want_address - side_p.want.address = ", LEVEL_3, IPADDR,
            side_p->want.address, 0);

    return(-1);
}

/* ********************************************************************        */
/* xn_ipcp_want_pool - Set IP address pool for REMOTE host addresses           */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_ipcp_want_pool(iface_no, ip_address_min, ip_address_max)           */
/*      int iface_no          - Interface number (see rtip.h)                  */
/*      PFBYTE ip_address_min - minimum address in IP address pool             */
/*      PFBYTE ip_address_max - minimum address in IP address pool             */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function establishes the IPCP IP address pool for assignment         */
/*   to remote hosts.                                                          */
/*                                                                             */
/*   In order to request a pool for the remote host (to provide an             */
/*   an address if the remote host requests the option with a 0 address,       */
/*   call this routine with the address range of the pool).                    */
/*   NOTE: negotiable is always set to TRUE, which force acceptance of         */
/*         an address requested by the REMOTE host.                            */
/*   NOTE: calling xn_ipcp_want_address with remote host set to REMOTE_HOST    */
/*         will override this routine.                                         */
/*                                                                             */
/*   The default is to not have an address pool.                               */
/*                                                                             */
/*   If this routine and xn_want_ipcp_address (REMOTE_HOST) are not called,    */
/*   the IP addresses specified in xn_attach(), will be sent to the otherside  */
/*   if a message is received requesting these addresses, i.e. if a null       */
/*   address is received.                                                      */
/*                                                                             */
/*   In order to extract the IP addresses negotiated, xn_interface_info() can  */
/*   be called after xn_lcp_open().                                            */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_ipcp_want_pool(int iface_no, PFBYTE ip_address_min, PFBYTE ip_address_max)
{
struct ipcp_side_s KS_FAR *side_p;                      
struct ipcp_s KS_FAR *ipcp_p;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    ipcp_p = (struct ipcp_s KS_FAR *)get_pdv(iface_no, IPcp);                                               
    side_p = get_side_p_ipcp(iface_no, REMOTE_HOST);

    if (ipcp_p && side_p)
    {
        tc_mv4(ipcp_p->peer_min,  ip_address_min, IP_ALEN);
        tc_mv4(ipcp_p->peer_curr, ip_address_min, IP_ALEN);
        tc_mv4(ipcp_p->peer_max,  ip_address_max, IP_ALEN);

        /* set flag so will negotiate    */
        side_p->want.negotiate |= IPCP_N_ADDRESS;

        /* make the address negotiable   */
        side_p->will_negotiate |= IPCP_ADDRESS_NEGOTIABLE;

        return 0;
    }
    DEBUG_LOG("after xn_ipcp_want_pool - pool min = ", LEVEL_3, IPADDR,
            ipcp_p->peer_min, 0);

    return(-1);
}

#if (INCLUDE_PPP_DNS)
/* ********************************************************************             */
/* xn_ipcp_want_dns_address - Set DNS addresses to negotiate for IPCP               */
/*                                                                                  */
/* Summary:                                                                         */
/*   #include "ppp.h"                                                               */
/*                                                                                  */
/*   int xn_ipcp_want_dns_address(iface_no, host, dns1_addr, dns2_addr, negotiable) */
/*      int iface_no        - Interface number (see rtip.h)                         */
/*      int host            - Set negotiation flag for local or remote host         */
/*                            (LOCAL_HOST or REMOTE_HOST)                           */
/*      PFCBYTE dns1_addr   - DNS primary address to negotiate.  Can                */
/*                            be an IP address or ip_nulladdr if                    */
/*                            requesting address from remote host.                  */
/*                            Cannot be 0.                                          */
/*      PFCBYTE dns2_addr   - DNS secondary address to negotiate.  Can              */
/*                            be an IP address, ip_nulladdr if                      */
/*                            requesting address from remote host or                */
/*                            0 if don't want to start negotiating                  */
/*                            secondary DNS address.                                */
/*      RTIP_BOOLEAN negotiable - set to TRUE if the address is negotiable, i.e.    */
/*                            if will accept what remote host says the address      */
/*                            is; set to FALSE if the address is not negotiable     */
/*                            i.e. the remote host must accept to this address      */
/*                                                                                  */
/* Description:                                                                     */
/*                                                                                  */
/*   This function establishes the IPCP DNS address option should be requested      */
/*   and the DNS addresses to request.                                              */
/*                                                                                  */
/*   In order to request local DNS addresses, call this routine with host set       */
/*   to LOCAL_HOST.  The option will be dropped if the remote host rejects          */
/*   the IPCP address requested.  If negotiable is set, an address suggested        */
/*   by the remote host in a NAK message will be used.  If negotiable is            */
/*   set to FALSE, a NAK message with a different DNS addresses will be             */
/*   rejected.                                                                      */
/*                                                                                  */
/*   In order to request an alternative address for the remote host or              */
/*   to suggest an address if the remote host does not request the option,          */
/*   call this routine with host set to REMOTE_HOST.  If negotiable is              */
/*   set to TRUE, the address sent in a REQUEST will be used (if it is              */
/*   not 0).  If negotiable is set to FALSE, a NAK message will be                  */
/*   sent if the address in the message does not match the address                  */
/*   specified by calling xn_ipcp_want_dns_address.  In either case, the address    */
/*   specifed by xn_ipcp_want_dns_address will be sent in a NAK message is a        */
/*   REQUEST is received with a 0 address.                                          */
/*                                                                                  */
/*   Care should be taken to ensure this address is acceptable to the               */
/*   remote host if negotiation parameter is FALSE, otherwise, negotiation          */
/*   will never converge.                                                           */
/*                                                                                  */
/*   The default is to not request the IPCP option.                                 */
/*                                                                                  */
/*   In order to request the an address from the remote host, set address to        */
/*   all 0s.                                                                        */
/*                                                                                  */
/*   If this routine is not called, the DNS addresseses specified in                */
/*   xn_set_server_list() will be sent to the otherside                             */
/*   if a message is received requesting these addresses, i.e. if a null            */
/*   address is received.                                                           */
/*                                                                                  */
/*   For more details see PPP User's Manual.                                        */
/*                                                                                  */
/* Returns:                                                                         */
/*   Zero is returned on success. Otherwise -1                                      */
/*                                                                                  */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by      */
/*   this function.  For error values returned by this function see                 */
/*   PPP User's Manual.                                                             */
/*                                                                                  */

int xn_ipcp_want_dns_address(int iface_no, int host, PFCBYTE dns1_addr, 
                             PFCBYTE dns2_addr, RTIP_BOOLEAN negotiable)
{
struct ipcp_side_s KS_FAR *side_p;                      
struct ipcp_s KS_FAR *ipcp_p;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    ipcp_p = (struct ipcp_s KS_FAR *)get_pdv(iface_no, IPcp);                                               
    side_p = get_side_p_ipcp(iface_no, host);

    if (side_p && ipcp_p)
    {
        if (!dns1_addr)
            return(set_errno(EFAULT));
        tc_mv4(side_p->want.dns1_addr, dns1_addr, IP_ALEN);

        /* set flag so will negotiate -                              */
        /* NOTE: if rejects ADDRESS, ADDRESSES option will be tried  */
        side_p->want.negotiate |= IPCP_N_DNS_PRIMARY;

        if (dns2_addr)
        {
            tc_mv4(side_p->want.dns2_addr, dns2_addr, IP_ALEN);

            /* set flag so will negotiate -                              */
            /* NOTE: if rejects ADDRESS, ADDRESSES option will be tried  */
            side_p->want.negotiate |= IPCP_N_DNS_SECOND;
        }

        if (negotiable)
            side_p->will_negotiate |= IPCP_DNS_NEGOTIABLE;

        return 0;
    }
    DEBUG_LOG("after xn_ipcp_want_dns_address - side_p.want.address = ", LEVEL_3, IPADDR,
            side_p->want.address, 0);

    return(-1);
}
#endif      /* INCLUDE_PPP_DNS */


/***********************************************************************   */
/* SET PAP USER/PASSWORD                                                   */
/***********************************************************************   */

/* ********************************************************************        */
/* xn_pap_user() -  Set info for access to remote host for PAP                 */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_pap_user(iface_no, user_name, pass_word)                           */
/*      int iface_no        - Interface number (see rtip.h)                    */
/*      PFCHAR user_name    - Username                                         */
/*      PFCHAR pass_word    - Password                                         */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function establishs a PAP username and password to be used to logon  */
/*   to a remote host using password authentication phase.                     */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_pap_user(int iface_no, PFCHAR user_name, PFCHAR pass_word)
{
PIFACE pi;
PFSMS fsm_p;
PPAPS pap_p;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

#if (DEBUG_PAP)
    DEBUG_ERROR("xn_pap_user (log into remote): ", STR2, user_name, pass_word);
#endif
    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi || !pi->edv)
        return(-1);

    fsm_p = (PFSMS)&(pi->edv->fsm[Pap]);
    pap_p = (PPAPS)fsm_p->pdv;

    pap_p->username[0] = NULLCHAR;
    pap_p->password[0] = NULLCHAR;

    tc_strncpy(pap_p->username, user_name, CFG_PAP_NAMELEN);
    if (pass_word) 
    {
        tc_strncpy(pap_p->password, pass_word, CFG_PAP_PWDLEN);
    } 
#if (INCLUDE_PAP_SRV && !PAP_FILE)
    else 
    {
        pap_pwdlookup(pap_p);
    }
#endif /* INCLUDE_PAP_SRV && !PAP_FILE */
    return 0;
}

#if (INCLUDE_PAP_SRV && !PAP_FILE)
/* ********************************************************************        */
/* xn_pap_add_user() -  Add info to database for access by remote host for PAP */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_pap_add_user(user_name, pass_word)                                 */
/*      PFCHAR user_name    - Username                                         */
/*      PFCHAR pass_word    - Password                                         */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function establishs an account for a remote host to logon to this    */
/*   machine using Password Authentication Protocol, PAP, during the           */
/*   authentication phase.                                                     */
/*                                                                             */
/*   The accounts are saved in the array pap_users (see pppuser.c) and are     */
/*   cleared by xn_ppp_init().                                                 */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_pap_add_user(PFCHAR user_name, PFCHAR pass_word)
{

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

#if (DEBUG_PAP)
    DEBUG_ERROR("xn_pap_add_user (remote must login): ", STR2, user_name, pass_word);
#endif

    return(pap_add_user(user_name, pass_word));
}
#endif  /* INCLUDE_PAP_SRV && !PAP_FILE */


#if (INCLUDE_CHAP)
/* ********************************************************************        */
/* xn_chap_clear_secret() - Remove all entries from CHAP secret table          */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_chap_clear_secret(void)                                            */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function removes all entries from the CHAP secret table.             */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_chap_clear_secret(void)
{
#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    init_secrets_table();
    return(0);
}

/* ********************************************************************            */
/* xn_chap_add_secret() -  Add info to database for access by remote host for CHAP */
/*                                                                                 */
/* Summary:                                                                        */
/*   #include "ppp.h"                                                              */
/*                                                                                 */
/*   int xn_chap_add_secret(PFCHAR machine, PFBYTE secret, int secret_len)         */
/*      PFCHAR machine  - machine name                                             */
/*      PFBYTE secret   - secret to use for challenging remote host machine        */
/*      int secret_len  - length of the secret                                     */
/*                                                                                 */
/* Description:                                                                    */
/*                                                                                 */
/*   This function establishs the secret to use for challenging or when            */
/*   being challenged by a remote host machine using Challenge-Handshake           */
/*   Authentication Protocol.                                                      */
/*   A CHAP challenge is sent during the authentication phase and                  */
/*   at random intervals after the connection is established.                      */
/*                                                                                 */
/*   The accounts are saved in the array chap_machine_secrets and are              */
/*   cleared by xn_ppp_init().                                                     */
/*                                                                                 */
/*   For more details see PPP User's Manual.                                       */
/*                                                                                 */
/* Returns:                                                                        */
/*   Zero is returned on success. Otherwise -1                                     */
/*                                                                                 */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by     */
/*   this function.  For error values returned by this function see                */
/*   PPP User's Manual.                                                            */
/*                                                                                 */

int xn_chap_add_secret(PFCHAR machine, PFBYTE secret, int secret_len)
{
#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    return(chap_add_secret(machine, secret, secret_len));
}

/* ********************************************************************        */
/* xn_chap_machine_name() -  Set machine name for sending CHAP messages        */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_chap_machine_name(PFCHAR machine)                                  */
/*      PFCHAR machine  - local machine name                                   */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function establishs the local machine name to send                   */
/*   in CHAP messages.                                                         */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_chap_machine_name(PFCHAR machine)
{
#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    if (tc_strlen(machine) > CFG_CHAP_NAMELEN)
        return(set_errno(EFAULT));

    tc_strcpy(local_host, machine);

    return 0;
}


#endif      /* INCLUDE_CHAP */



/***********************************************************************   */
/* OPEN PROTOCOLS                                                          */
/***********************************************************************   */

/* TBD   */
#if (INCLUDE_RUN_TIME_CONFIG)
void ppp_config(PIFACE pi)
{
PFSMS fsm_p;
PPPPS ppp_p;

    ppp_p = pi->edv;
    fsm_p = &(ppp_p->fsm[Lcp]);
    fsm_p->pdc->try_req = (byte)CFG_LCP_REQ_TRY;
    fsm_p->pdc->try_nak = (byte)CFG_LCP_NAK_TRY;
    fsm_p->pdc->try_terminate = (byte)CFG_LCP_TERM_TRY;
    fsm_p->pdc->timeout = CFG_LCP_TIMEOUT;

    fsm_p = &(ppp_p->fsm[IPcp]);
    fsm_p->pdc->try_req = (byte)CFG_IPCP_REQ_TRY;
    fsm_p->pdc->try_nak = (byte)CFG_IPCP_NAK_TRY;
    fsm_p->pdc->try_terminate = (byte)CFG_IPCP_TERM_TRY;
    fsm_p->pdc->timeout = CFG_IPCP_TIMEOUT;

    fsm_p = &(ppp_p->fsm[Pap]);
    fsm_p->pdc->try_req = (byte)CFG_PAP_REQ_TRY;
    fsm_p->pdc->try_nak = (byte)CFG_PAP_FAIL_MAX;
    fsm_p->pdc->timeout = CFG_PAP_TIMEOUT;

    fsm_p = &(ppp_p->fsm[Chap]);
    fsm_p->pdc->try_req = (byte)CFG_CHAP_REQ_TRY;
    fsm_p->pdc->timeout = CFG_CHAP_TIMEOUT;
}
#endif

/* ********************************************************************        */
/* xn_ppp_init() - Set PPP options to their default values                     */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_ppp_init(iface_no, RTIP_BOOLEAN async_link)                        */
/*      int iface_no        - Interface number (see rtip.h)                    */
/*      RTIP_BOOLEAN async_link  - set to TRUE if asynchronous link; FALSE     */
/*                            for all others                                   */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function sets all PPP options to their default values.  This         */
/*   includes whether to request or negotiate options, option values,          */
/*   PAP user accounts, login information, etc.                                */
/*                                                                             */
/*   NOTE: the parameter async_link affects the default value for ACCM         */
/*         where async links use 0xffffffff as default and all others          */
/*         use 0                                                               */
/*                                                                             */
/*   xn_interface_open() needs to be called prior to calling this routine.     */
/*   All other PPP API calls to set negotiation options, open IPCP etc should  */
/*   not be called until after this routine has been called. In order          */
/*   to guarentee this routine completes before the remote side initiates      */
/*   another LCP open, this routine needs be called before xn_ppp_close().     */
/*   The routine does not modify the state machine.                            */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_ppp_init(int iface_no, RTIP_BOOLEAN async_link)
{
PIFACE pi;
PPPPS ppp_p;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi || !pi->edv)
        return(-1);

    ppp_p = pi->edv;
    ppp_init(ppp_p, async_link);

    return(0);
}

/* ********************************************************************        */
/* xn_lcp_echo() - send LCP ECHO to remote and wait for response               */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_lcp_echo(iface_no, type, wait_cnt)                                 */
/*          int iface_no        - Interface number (see rtip.h)                */
/*          int wait_cnt        - Time to wait in ticks for echo response      */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function tests the connection via ECHO REQUEST and                   */
/*   ECHO REPLY LCP commands.  It sends an ECHO REQUEST to the                 */
/*   remote host and waits up to wait_cnt ticks for the ECHO RESPONSE.         */
/*   It waits on the PING exchange.                                            */
/*                                                                             */
/*   All other PPP API calls to set negotiation options, open IPCP etc. as     */
/*   well as xn_interface_open() and xn_ppp_init() need to be done prior to    */
/*   calling xn_lcp_echo().                                                    */
/*                                                                             */
/*   Echo requests and replys use the magic number option if it has            */
/*   been negotiated in the LCP phase.  Using the magic number option          */
/*   is recomended since it will prevent loopback connections from             */
/*   succeeding.                                                               */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_lcp_echo(int iface_no, int wait_cnt, int size)
{
PIFACE pi;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi || !pi->edv)
        return(-1);

    return(ppp_echo(pi, (word)wait_cnt, size));
}

/* ********************************************************************             */
/* xn_ppp_poll_state() - Open PPP connection                                        */
/*                                                                                  */
/* Summary:                                                                         */
/*   #include "ppp.h"                                                               */
/*                                                                                  */
/*   int xn_ppp_poll_state(iface_no, poll_close, poll_open)                         */
/*          int iface_no        - Interface number (see rtip.h)                     */
/*          RTIP_BOOLEAN poll_close - Specifies if should check for previous PPP    */
/*                                close operation has completed                     */
/*          RTIP_BOOLEAN poll_open   - Specifies if should check if PPP connection  */
/*                                is established                                    */
/*                                                                                  */
/* Description:                                                                     */
/*                                                                                  */
/*   This function checks the status of a PPP connection.                           */
/*   It is useful in conjunction with xn_lcp_open() for cases where                 */
/*   xn_lcp_open() should not block.                                                */
/*                                                                                  */
/*   Usage to connect on interface iface_no would be:                               */
/*      if (xn_ppp_poll_state(iface_no, TRUE, FALSE))                               */
/*      {                                                                           */
/*         xn_lcp_open(iface_no, 0, 0);                                             */
/*         open_started = TRUE;                                                     */
/*      }                                                                           */
/*           ...                                                                    */
/*      if (open_started)                                                           */
/*      {                                                                           */
/*         if (xn_ppp_poll_state(iface_no, FALSE, TRUE))                            */
/*            ...                                                                   */
/*      }                                                                           */
/*                                                                                  */
/*   For more details see PPP User's Manual.                                        */
/*                                                                                  */
/* Returns:                                                                         */
/*   One is returned on ready. Zero is returned on not ready.                       */
/*   -1 is returned upon error.                                                     */
/*                                                                                  */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by      */
/*   this function.  For error values returned by this function see                 */
/*   PPP User's Manual.                                                             */
/*                                                                                  */

int xn_ppp_poll_state(int iface_no, RTIP_BOOLEAN poll_close, RTIP_BOOLEAN poll_open)
{
PIFACE pi;
PFSMS  fsm_p;

    DEBUG_LOG("xn_ppp_poll_state called", LEVEL_3, NOVAR, 0, 0);

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi || !pi->edv)
        return(-1);


    /* need to wait for prev close to finish   */
    if (poll_close)
    {
        fsm_p = (PFSMS)&(pi->edv->fsm[IPcp]);
        if (fsm_p->ppp_state == fsmTERM_Sent)
            return(0);

        fsm_p = (PFSMS)&(pi->edv->fsm[Lcp]);
        if (fsm_p->ppp_state == fsmTERM_Sent)
            return(0);
    }

    if (poll_open)
    {
        fsm_p = (PFSMS)&(pi->edv->fsm[IPcp]);
        if (fsm_p->ppp_state != fsmOPENED)
        {        
            return(0);
        }
    }
#if (INCLUDE_TRK_PPP && DISPLAY_TRK_SUCCESS)
    display_track_ppp_info();
#endif
    return(1);
}


/* ********************************************************************        */
/* xn_lcp_open() - Open PPP connection                                         */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_lcp_open(iface_no, type, close_wait_cnt, open_wait_cnt)            */
/*          int iface_no        - Interface number (see rtip.h)                */
/*          int type            - Specifies whether to do active or            */
/*                                passive open (ACTIVE or PASSIVE)             */
/*          long close_wait_cnt - Time to wait in ticks for previous PPP       */
/*                                close operation to complete                  */
/*          long open_wait_cnt  - Time to wait in ticks for PPP connection     */
/*                                to be established                            */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function establishes a PPP connection.  It waits for the             */
/*   connection to become established or until a timeout occurs.  It           */
/*   waits on the PPP signal.                                                  */
/*                                                                             */
/*   The connection is open after LCP, authentication (if negotiated) and      */
/*   IPCP negotiation has completed.                                           */
/*                                                                             */
/*   In order to wait for the remote side to start the LCP negotiation,        */
/*   this routine needs be called with type set to PASSIVE.  In order          */
/*   to start the negotiation immediately, this routine needs to be called     */
/*   with type set to PASSIVE.  Both the local and remote hosts can            */
/*   do an active open.                                                        */
/*                                                                             */
/*   All other PPP API calls to set negotiation options, open IPCP etc. as     */
/*   well as xn_interface_open() and xn_ppp_init() need to be done prior to    */
/*   calling xn_lcp_open().                                                    */
/*                                                                             */
/*   In order to extract the IP addresses negotiated, xn_interface_info() can  */
/*   be called after xn_lcp_open().                                            */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_lcp_open(int iface_no, int type, long close_wait_cnt, long open_wait_cnt)
{
PIFACE pi;
PFSMS  fsm_p;
int    i;
int    ticks_sec;
PPPPS  ppp_p;
dword  start_time, curr_time;
int    elap_time_tics; 

    DEBUG_LOG("xn_lcp_open called", LEVEL_3, NOVAR, 0, 0);

#if (INCLUDE_TRK_PPP)
    trk_ppp_off = 0;
#endif

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi || !pi->edv)
        return(-1);

    start_time = ks_get_ticks();    

    /* need to wait for prev close to finish   */
    ticks_sec = ks_ticks_p_sec();
    fsm_p = (PFSMS)&(pi->edv->fsm[IPcp]);
    if (close_wait_cnt)
    {
        for (i=0; i < close_wait_cnt; i+= ticks_sec)
        {
            if (fsm_p->ppp_state != fsmTERM_Sent)
                break;
            ks_sleep(ks_ticks_p_sec());
        }
        if (i >= close_wait_cnt)
            return(set_errno(EPPPNOTCLOSED));
    }

    /* set IPCP state to closed since it will check the IPcp state   */
    /* when a signal is received                                     */
    PPP_TRANS_STATE(fsm_p, fsmCLOSED);

    /* need to wait for prev close to finish but only wait a limited time   */
    fsm_p = (PFSMS)&(pi->edv->fsm[Lcp]);
    if (close_wait_cnt)
    {
        for (i=0; i < close_wait_cnt; i+= ticks_sec)
        {
            if (fsm_p->ppp_state != fsmTERM_Sent)
                break;
            ks_sleep(ks_ticks_p_sec());
        }
        if (i >= close_wait_cnt)
            return(set_errno(EPPPNOTCLOSED));
    }

    /* if OPENED, issue a down event as suggested in RFC 1331 under Open   */
    /* Event description                                                   */
    /* NOTE: possible states coming into this routine are OPEN, CLOSE,     */
    /*       LISTEN and TermSent                                           */
    if (fsm_p->ppp_state == fsmOPENED)
    {
        /* do LCP Down Event which will perform the following:              */
        /*   - stop timers, fsm_reset (sets state to closed, reinitializes  */
        /*     retry values, calls lcp_reset() which set negotiation values */
        /*     back , calls lcp_closing()                                   */
        /*   - lcp_closing() performs down event for IPCP (fsm_down()) and  */
        /*     down event for PAP (pap_down())                              */
        fsm_down(fsm_p);
    }

    /* set Phase to allow input/output of PPP packets to begin   */
    ppp_p = pi->edv;       
    PPP_TRANS_PHASE(ppp_p, pppOPEN);

    /* save type of open done; set state to listen and for active open send config req   */
    if (type == ACTIVE)
        fsm_active(fsm_p);
    else
        fsm_passive(fsm_p);

    OS_CLEAR_PPP_SIGNAL(pi);

    /* prepare to begin negotiation; send opening request if active   */
    /* open                                                           */
    fsm_start(fsm_p);

    /* wait for IPCP to signal its IPCP config exchange is done or timeout    */
    /* NOTE: - when LCP negotioation is done, if PAP authentication was       */
    /*         negotiated, PAP authentication is done automatically (see      */
    /*         pap_opening, pap_local and pap_remote);                        */
    /*       - if PAP authentication was not negotiated, IPCP config exchange */
    /*         is started (see ppp_ready)                                     */
    /*       - if PAP authentication was not negotiated or when it is done,   */
    /*         IPCP config exchange is automatically started (see ppp_ready)  */
    /*       - when IPCP config exchange has completed, it will signal        */
    /* NOTE: currently there is no status returned when setting signal; the   */
    /*       signal is only set if successful                                 */
    /* tbd - check fsm_start for IPCP config - will it always start neg       */
    /*       if not, need to signal there (ppp_ready)                         */
    /* tbd - different errno if state not right - could be due to interface   */
    /*       close being done                                                 */
    if (open_wait_cnt)
    {
        fsm_p = (PFSMS)&(pi->edv->fsm[IPcp]);
        for (;;)
        {
            if (!OS_TEST_PPP_SIGNAL(pi, (word)open_wait_cnt) ||
                (fsm_p->ppp_state != fsmOPENED))
            {        
                /* SIGNALLED BUT STATE IS --NOT-- OPEN                  */
                /* if succeded then the other side must have started    */
                /* over since the state is not OPENED; so go wait again */
                /* but do not wait longer than open_wait_cnt            */
                if (pi->ctrl.signal_status == PPP_SIGNAL_SUCCESS)
                {
                    /* get elapsed time since started open                           */
                    /* if wrap, elap_time = 0xffffffff - ctx->start_time + curr_time */
                    curr_time = ks_get_ticks();
                    if (curr_time < start_time)  /* if wrap */
                        elap_time_tics = (int)(0xfffffffful - 
                                            start_time + curr_time);
                    else
                        elap_time_tics = (int)(curr_time - start_time);

                    /* check for timeout                */
                    /* NOTE: comparison is done in msec */
                    if (elap_time_tics < open_wait_cnt)
                    {
                        DEBUG_ERROR("xn_lcp_open - signalled but start over",
                            NOVAR, 0, 0);  
                        open_wait_cnt -= elap_time_tics;
                        continue;
                    }
                }

                /* do LCP Down Event which will perform the following:                 */
                /*   - stop timers, fsm_reset (sets state to closed, reinitializes     */
                /*     retry values, calls lcp_reset() which set negotiation values    */
                /*     back , calls lcp_closing()                                      */
                /*   - lcp_closing() performs down event for IPCP (fsm_down()) and     */
                /*     down event for PAP (pap_down())                                 */
                DEBUG_ERROR("PPP open timed out - IPCP state (5=OPENED) = ", EBS_INT1, 
                    fsm_p->ppp_state, 0);
                fsm_p = (PFSMS)&(pi->edv->fsm[Lcp]);
                DEBUG_ERROR("PPP open timed out - LCP state (5=OPENED) = ", EBS_INT1, 
                    fsm_p->ppp_state, 0);
                fsm_down(fsm_p);
                PPP_TRANS_STATE(fsm_p, fsmCLOSED);
#        if (INCLUDE_TRK_PPP)
                display_track_ppp_info();
#        endif
                return(set_errno(EPPPTIMEDOUT));
            }
            break;
        }
    }   /* if open_wait_cnt */

    DEBUG_LOG("PPP opened successfully", LEVEL_3, NOVAR, 0, 0);

#if (INCLUDE_TRK_PPP && DISPLAY_TRK_SUCCESS)
    if (open_wait_cnt)
    {
        DEBUG_ERROR("xn_lcp_open: PPP open succeeded", NOVAR, 0, 0);
        display_track_ppp_info();
    }
    else
    {
        DEBUG_ERROR("xn_lcp_open: PPP open succeeded BUT POLLING mode", 
            NOVAR, 0, 0);
    }
#endif
    return 0;
}


/* ********************************************************************        */
/* xn_ipcp_open() - Enables IPCP Protocol                                      */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_ipcp_open(iface_no)                                                */
/*      int iface_no        - Interface number (see rtip.h)                    */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function enables IPCP protocol negotiation to be performed which     */
/*   will enable sending and receiving IP packets after the  PPP               */
/*   connection is established.                                                */
/*                                                                             */
/*   IPCP negotiation is started after LCP negotiation and authentication      */
/*   has completed (see xn_lcp_open.)                                          */
/*                                                                             */
/*   This function must be called prior to xn_lcp_open() but after             */
/*   xn_interface_open().                                                      */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_ipcp_open(int iface_no)
{
PIFACE pi;
PFSMS fsm_p;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi || !pi->edv)
        return(-1);

    fsm_p = (PFSMS)&(pi->edv->fsm[IPcp]);

    fsm_active(fsm_p);

    /* if the PPP we belong to is ready   */
    if ( fsm_p->ppp_p->ppp_phase == pppREADY ) 
    {
        fsm_start(fsm_p);
    }
    return 0;
}


/* ********************************************************************        */
/* xn_ppp_close() - Close PPP connection but not the interface                 */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_ppp_close(iface_no, wait)                                          */
/*      int     iface_no        - Interface number (see rtip.h)                */
/*      RTIP_BOOLEAN wait                                                      */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function closes a PPP connection.                                    */
/*                                                                             */
/*   This function has no affected on any of the options or values to be       */
/*   negotiated and does not close the interface.  It will only shut down      */
/*   the PPP connection by initiating a TERM/ACK handshake.                    */
/*                                                                             */
/*   The parameter wait specifies the number of seconds                        */
/*   until the ACK of the TERM is received.                                    */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_ppp_close(int iface_no, int wait)
{
PIFACE pi;
PFSMS fsm_p;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
        return(-1);

    DEBUG_LOG("before xn_ppp_close - his_ip_addr = ", LEVEL_3, IPADDR,
           pi->addr.his_ip_addr, 0);

    /* initiate protocol close; possibly send a term req depending upon state   */
    /* but do not wait for terminate ack to be received - subsequent open       */
    /* will wait for it                                                         */
    /* sets state to CLOSED                                                     */
    proc_fsm_close(pi, Lcp);
    proc_fsm_close(pi, IPcp);

    if (wait)
    {
        /* need to wait for close to finish otherwise if the interface    */
        /* is closed before closing the interface                         */
        /* or the response might come in while the interface is closed    */
        /* leaving the state as fsmTERM_Sent which will cause xn_lcp_open */
        /* to hang                                                        */
        fsm_p = (PFSMS)&(pi->edv->fsm[Lcp]);
        while (fsm_p->ppp_state == fsmTERM_Sent)
        {
#if (INCLUDE_XMIT_QUE)
            DEBUG_ERROR("Lcp stat = TERM_Sent, xmit dcu = ", DINT1,
                pi->ctrl.list_xmit, 0);
#else
            DEBUG_ERROR("Lcp stat = TERM_Sent, xmit_dcu = ", DINT1,
                pi->xmit_dcu, 0);
#endif
            DEBUG_ERROR("xn_ppp_close: wait for LCP to close", NOVAR, 0, 0);
            ks_sleep(ks_ticks_p_sec());
        }

        fsm_p = (PFSMS)&(pi->edv->fsm[IPcp]);
        while (fsm_p->ppp_state == fsmTERM_Sent)
        {
#if (DISPLAY_PPP_CLOSE)
#if (INCLUDE_XMIT_QUE)
            DEBUG_ERROR("IPcp stat = TERM_Sent, xmit dcu = ", DINT1,
                pi->ctrl.list_xmit, 0);
#else
            DEBUG_ERROR("IPcp stat = TERM_Sent, xmit_dcu = ", DINT1,
                pi->xmit_dcu, 0);
#endif
            DEBUG_ERROR("xn_ppp_close: wait for IPcp to close", NOVAR, 0, 0);
#endif
            ks_sleep(ks_ticks_p_sec());
        }
    }

    DEBUG_LOG("after xn_ppp_close - his_ip_addr = ", LEVEL_3, IPADDR,
           pi->addr.his_ip_addr, 0);
DEBUG_ERROR("xn_ppp_close: done", NOVAR, 0, 0);

    return 0;
}

/* ********************************************************************        */
/* xn_ppp_down() - Perform a down event on a PPP connection                    */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_ppp_down(iface_no)                                                 */
/*      int iface_no        - Interface number (see rtip.h)                    */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function downs a PPP connection, i.e. it resets the state            */
/*   information in preparation for the next open.                             */
/*                                                                             */
/*   This function has no affected on any of the options or values to be       */
/*   negotiated and does not close the interface.  It will only shuts down     */
/*   the PPP connection by setting the connection state to CLOSED or           */
/*   LISTEN.  This routine should be called if xn_lcp_open fails due           */
/*   to the previous close not completing or the close handshake cannot        */
/*   be done (see xn_ppp_close) due to a line drop.                            */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success. Otherwise -1                                 */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_ppp_down(int iface_no)
{
PIFACE pi;
PFSMS fsm_p;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
        return(-1);

    DEBUG_LOG("before ppp_down - his_ip_addr = ", LEVEL_3, IPADDR,
           pi->addr.his_ip_addr, 0);

    /* initiate down event   */
    fsm_p = (PFSMS)&(pi->edv->fsm[IPcp]);
    fsm_down(fsm_p);

    fsm_p = (PFSMS)&(pi->edv->fsm[Lcp]);
    fsm_down(fsm_p);

    return 0;
}

/* ********************************************************************        */
/* xn_ppp_wait_down() - Waits for PPP connection to go down                    */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ppp.h"                                                          */
/*                                                                             */
/*   int xn_ppp_wait_down(iface_no, wait_cnt)                                  */
/*      int iface_no    - Interface number (see rtip.h)                        */
/*      int wait_cnt    - Time to wait in ticks for the down event             */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/*   This function waits for a PPP connection to go down (via receipt          */
/*   of a TERMINATE REQUEST from remote host or modem drop).                   */
/*                                                                             */
/*   This function has no affected on any of the options or values to be       */
/*   negotiated and does not close the interface.                              */
/*                                                                             */
/*   For more details see PPP User's Manual.                                   */
/*                                                                             */
/* Returns:                                                                    */
/*   Zero is returned on success (PPP connection is down). Otherwise -1        */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   PPP User's Manual.                                                        */
/*                                                                             */

int xn_ppp_wait_down(int iface_no, int wait_cnt)
{
PIFACE pi;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
        return(-1);

    DEBUG_LOG("before ppp_down - his_ip_addr = ", LEVEL_3, IPADDR,
           pi->addr.his_ip_addr, 0);

    if (!OS_TEST_PPP_SIGNAL(pi, (word)wait_cnt))
    {
        /* TIMEOUT; PPP is still connected   */
        return(-1);     
    }
    return 0;           /* PPP is NOT connected */
}

/* ********************************************************************   */
int xn_bind_ppp(int minor_number)
{
    ppp_escape_routine = ppp_xmit_escape;
    ppp_give_string_routine = ppp_give_string;

    return(xn_device_table_add(ppp_device.device_id, 
                        minor_number, 
                        ppp_device.iface_type,
                        ppp_device.device_name,
                        SNMP_DEVICE_INFO(ppp_device.media_mib, 
                                         ppp_device.speed)                          
                        (DEV_OPEN)ppp_device.open,
                        (DEV_CLOSE)ppp_device.close,
                        (DEV_XMIT)ppp_device.xmit,
                        (DEV_XMIT_DONE)ppp_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)ppp_device.proc_interrupts,
                        (DEV_STATS)ppp_device.statistics,
                        (DEV_SETMCAST)ppp_device.setmcast));
}

#if (INCLUDE_PPPOE)
int xn_bind_pppoe(int minor_number)
{
    ppp_escape_routine = ppp_xmit_escape;
    ppp_give_string_routine = ppp_give_string;

    return(xn_device_table_add(PPPOE_DEVICE,
                        minor_number, 
                        ppp_device.iface_type,
                        "PPPOED_DEVICE",
                        SNMP_DEVICE_INFO(ppp_device.media_mib, 
                                         ppp_device.speed)                          
                        (DEV_OPEN)ppp_device.open,
                        (DEV_CLOSE)ppp_device.close,
                        (DEV_XMIT)ppp_device.xmit,
                        (DEV_XMIT_DONE)ppp_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)ppp_device.proc_interrupts,
                        (DEV_STATS)ppp_device.statistics,
                        (DEV_SETMCAST)ppp_device.setmcast));
}
#endif

#endif
