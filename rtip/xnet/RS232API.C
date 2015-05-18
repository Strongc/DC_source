
/*                                                                      */
/* RS232API.C - API functions                                           */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*                                                                      */
/*  Module description:                                                 */
/*  This module provides the application programmers interface layer    */
/*  for the RS232 implementation.                                       */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_API

#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

#if (INCLUDE_MODEM)
    #include "uart.h"
#endif

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define TEST_MODEM  0   /* set to 1 if using direct modem to modem connection */

/* ********************************************************************      */
/* LOCAL FUNCTIONS                                                           */

void    initrawdatabuf(PRS232_IF_INFO pif_info);
#if (INCLUDE_MODEM)
int     send_modem_command(PFCCHAR str, PIFACE pi);
int     wait_modem_response(PFCCHAR str, word timeout, PRS232_IF_INFO pif_info);
int     wait_modem_response_list(PAL_COMMAND waitlist, word timeout, PRS232_IF_INFO pif_info);
#endif

/* ********************************************************************      */
/* EXTERNAL FUNCTIONS                                                        */
#if (INCLUDE_MODEM)
PUART_INFO get_uinfo_struct(int minor_number);
#endif

#if (INCLUDE_MODEM)
/* ********************************************************************   */
/* MODEM STUFF                                                            */
/* ********************************************************************   */

/* ********************************************************************   */
void initrawdatabuf(PRS232_IF_INFO pif_info)
{
    if (pif_info->raw_mode)
    {
        /* initialize raw data buffer   */
        tc_memset((PFBYTE)pif_info->raw_mode_buffer, 0, MAX_PACKETSIZE);
    }
    pif_info->raw_mode_index = 0;
}

/* ********************************************************************   */
/* returns number of bytes sent                                           */
int send_modem_command(PFCCHAR str, PIFACE pi)
{
    return(uart_send(pi->minor_number, (PFBYTE)str, tc_strlen(str)));
}

/* ********************************************************************   */
/* wait for response str,                                                 */
/* Returns 0 if expected string is received, -1 if not found              */
int wait_modem_response(PFCCHAR str, word timeout, PRS232_IF_INFO pif_info)
{
dword n, num_loops;
word delay_ticks;

    /* set-up to check for string every quarter second   */
    delay_ticks = (word)(ks_ticks_p_sec() >> 2);
    num_loops = (dword)timeout * 4ul;

    for (n=0; (n<num_loops) || (timeout == RTIP_INF); n++)
    {
        ks_sleep(delay_ticks);
        if (tc_strstr((PFCHAR)pif_info->raw_mode_buffer, str))
            return(0);
    }
    return(-1);
}

/* ********************************************************************      */
/* wait for response str,                                                    */
/* Returns offset of WAITLIST command in script if expected string is 
   received, -1 if not found              */
int wait_modem_response_list(PAL_COMMAND waitlist, word timeout, PRS232_IF_INFO pif_info)
{
int n, num_loops, offset;
word delay_ticks;
PAL_COMMAND loopvar;

    /* set-up to check for string every quarter second     */
    delay_ticks = (word)(ks_ticks_p_sec() >> 2);
    num_loops = timeout * 4;

    for (n=0; (n<num_loops) || (timeout == RTIP_INF); n++)
    {
        ks_sleep(delay_ticks);
        for (offset = 0, loopvar=waitlist; 
             (loopvar->command==AL_WAITLIST) || (loopvar->command==AL_BRKFND); 
             loopvar++, offset++)
        {
            if (tc_stristr((PFCHAR)pif_info->raw_mode_buffer, 
                           loopvar->str_arg))
                return(offset);
        }
    }
    return(-1);
}

/* ********************************************************************   */
/* buffer_char() - save an input char from the modem                      */
/*                                                                        */
/*    Called by application to save characters from the modem             */
/*    while in raw mode.  Characters are tossed if the buffer             */
/*    (of size MAX_PACKETSIZE) is full.                                   */
/*                                                                        */
/*    Returns nothing                                                     */

void buffer_char(char chr, PRS232_IF_INFO pif_info)
{
    if (pif_info->raw_mode_index < MAX_PACKETSIZE && pif_info->raw_mode)
    {
        pif_info->raw_mode_buffer[pif_info->raw_mode_index] = chr;
        pif_info->raw_mode_buffer[pif_info->raw_mode_index+1] = '\0';
        pif_info->raw_mode_index++;
    }
}

/* ********************************************************************      */
/* xn_scriptlogin() -  Login to remote server using script                   */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   int xn_scriptlogin(iface_no, script, num_commands)                      */
/*      int iface_no                 - interface number                      */
/*      PAL_COMMAND script           - instructions for login procedure      */
/*      int num_commands             - number of commands in script          */
/*      PFCHAR term_str              - termination string for sending        */
/*                                     commands                              */
/*                                                                           */
/* Description:                                                              */
/*   Automates login to remote server via Hayes compatible modem using a     */
/*   script.    For information on scripting language see RTIP Reference     */
/*   Manual.                                                                 */
/*                                                                           */
/*  For more details see the RTIP Manual.                                    */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*   NOTE: returns status of last modem response expected                    */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_scriptlogin(int iface_no, PAL_COMMAND script,  /*__fn__*/
                   int num_commands, PFCHAR term_str)
{
int n,l;
int result;
PIFACE pi;
DCU msg;
PFCHAR send_str;
PRS232_IF_INFO pif_info;

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
    {
        DEBUG_ERROR("xn_scriptlogin - interface is not open", NOVAR, 0, 0);
        return(set_errno(EBADIFACE));
    }

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];

    msg = os_alloc_packet(MAX_PACKETSIZE, MODEM_ALLOC);
    if (!msg)
    {
        DEBUG_ERROR("xn_scriptlogin - out of DCUs", NOVAR, 0, 0);
        return(set_errno(ENOPKTS));
    }
    send_str = (PFCHAR)DCUTODATA(msg);

    rs232_raw_mode(pi, TRUE);
    result = 0;     /* assume success */
    initrawdatabuf(pif_info);
    for (n = 0; n < num_commands; n++)
    {
        if (script[n].command == AL_SEND)
        {
            tc_strcpy(send_str, script[n].str_arg);
            tc_strcat(send_str, term_str);
            DEBUG_ERROR("modem send: ", STR1, send_str, 0);
            initrawdatabuf(pif_info);
            if (!send_modem_command(send_str, pi))
            {
                os_free_packet(msg);
                return(set_errno(EMODEMSENDFAILED));
            }
        }
        else if (script[n].command == AL_WAIT)
        {
            result = wait_modem_response(script[n].str_arg,
                                         script[n].num_arg, pif_info);
        }
        else if (script[n].command == AL_WAITLIST)
        {
            result = wait_modem_response_list(&script[n], script[n].num_arg,
                                              pif_info);
            if ((result >= 0) && (script[n+result+1].command == AL_BRKFND))
                n = n + result + 1;     /* point to AL_BRKFND */
            else
            {
                while ( (script[n].command == AL_WAITLIST) ||
                        (script[n].command == AL_BRKFND) )
                    n++;
            }
            /* Decrement by one, so that the CURRENT item is the LAST item in the
               waitlist; NOTE: n will be incremented by for loop */
            n--;
        }
        else if (script[n].command == AL_SLEEP)
        {
            ks_sleep((word)(script[n].num_arg*ks_ticks_p_sec()));
        }
        else if (script[n].command == AL_PRINT)
        {
            if (tc_strcmp(script[n].str_arg, "rawdata"))
            {
                DEBUG_ERROR((PFCHAR)pif_info->raw_mode_buffer, NOVAR, 0, 0);
            }
            else
            {
                DEBUG_ERROR(script[n].str_arg, NOVAR, 0, 0);
            }
        }
        else if (script[n].command == AL_END)
        {
            break;
        }
        else if (script[n].command == AL_ENDERR)
        {
            result = -1;
            break;
        }
        else if ( (script[n].command == AL_BRKERR) ||
                  (script[n].command == AL_BRKFND) )
        {
            /* if error occured in script, goto label specified by    */
            /* break command in script OR                             */
            /* if found in WAITLIST and there is a BRKFND after found */
            /* string, then goto label; NOTE: for this case WAITLIST  */
            /* left n pointing to BRKFND command                      */
            if ( ((script[n].command == AL_BRKERR) && (result < 0)) ||
                 ((script[n].command == AL_BRKFND) && (result >= 0)) ) 
            {
                for (l = 0; l < num_commands; l++)
                {
                    if (script[l].command == AL_LABEL)
                    {
                        /* if label we are looking for, set command   */
                        /* working on to the offset of the label      */
                        if (tc_strcmp(script[l].str_arg, 
                                      script[n].str_arg) == 0)
                        {
                            n = l-1;        /* do the goto to the LABEL */
                            break;
                        }       
                    }
                }
                if (l == num_commands)
                {
                    DEBUG_ERROR("xn_scriptlogin: no label found, looking for:", 
                        STR1, script[n].str_arg, 0);
                    result = -1;
                    break;
                }
            }   /* end of BRKERR or BRKFND */
        }       /* end of if ... else BRKERR or BRKFND */
    }           /* end of loop thru script commands */
    rs232_raw_mode(pi, FALSE);
    os_free_packet(msg);
    if (result < 0)
        return(set_errno(EMODEMBADRESP));
    return(0);
}

/* ********************************************************************           */
/* xn_autologin() -  automated login to remote server via Hayes modem             */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "rtipapi.h"                                                         */
/*                                                                                */
/*   int xn_autologin(iface_no, setupstr, phonenum, username, passwd, term_str)   */
/*      int iface_no        - interface number                                    */
/*      PFCHAR setupstr     - modem initialization string. "ATM1L1", for example. */
/*      PFCHAR phonenum     - the phone number to dial                            */
/*      PFCHAR username     - the username to enter at login: prompt              */
/*      PFCHAR passwd       - the password to enter at password: prompt           */
/*      PFCHAR term_str     - termination string for sending commands             */
/*                                                                                */
/* Description:                                                                   */
/*   Automates login to remote server via Hayes compatible modem.                 */
/*                                                                                */
/*   For more details see the RTIP Manual.                                        */
/*                                                                                */
/* Returns:                                                                       */
/*   Zero is returned on success. Otherwise -1                                    */
/*                                                                                */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set       */
/*   by this function.  For error values returned by this function see            */
/*   RTIP Reference Manual.                                                       */
/*                                                                                */

int xn_autologin(int iface_no, PFCHAR setupstr, PFCHAR phonenum,
                 PFCHAR username, PFCHAR passwd, PFCHAR term_str)
{
char _setupstr[100];
char _phonenum[40];
char _username[40];
char _passwd[40];

    /* check if username/password is required   */
    if (username && tc_strlen(username))
    {
        /* connect to modem where username/password is required   */

        /* modify fields of script for specific info for this connect   */
        tc_strcpy(_setupstr, setupstr);
        default_script_login_ptr[1].str_arg = _setupstr;

        tc_strcpy(_phonenum, "ATDT");
        tc_strcat(_phonenum, phonenum);
        default_script_login_ptr[4].str_arg = _phonenum;

        tc_strcpy(_username, username);
        default_script_login_ptr[14].str_arg = _username;

        tc_strcpy(_passwd, passwd);
        default_script_login_ptr[17].str_arg = _passwd;

        return xn_scriptlogin(iface_no, default_script_login_ptr, 37, 
                              term_str);
    }

    /* connect to modem where username/password is not required   */
    /* modify fields of script for specific info for this connect */
    tc_strcpy(_setupstr, setupstr);
    default_script_ptr[1].str_arg = _setupstr;

    tc_strcpy(_phonenum, "ATDT");
    tc_strcat(_phonenum, phonenum);
    default_script_ptr[4].str_arg = _phonenum;

    return xn_scriptlogin(iface_no, default_script_ptr, 24, term_str);
}

/* ********************************************************************      */
/* xn_autoanswer() -  waits for remote login via modem and answers           */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   int xn_autoanswer(iface_no, setupstr, timeout)                          */
/*      int iface_no            - interface number                           */
/*      PFCHAR setupstr         - modem initialization string.               */
/*      word timeout            - number of seconds to wait for ring         */
/*      PFCHAR term_str         - termination string for sending commands    */
/*                                                                           */
/* Description:                                                              */
/*   Waits for a remote modem to dial in and answers it.                     */
/*                                                                           */
/*  For more details see the RTIP Manual.                                    */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_autoanswer(int iface_no, PFCHAR setupstr, word timeout, PFCHAR term_str)
{
int n;
PIFACE pi;
DCU msg;
PFCHAR send_str;
PRS232_IF_INFO pif_info;

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
    {
        DEBUG_ERROR("xn_autoanswer - interface is not open", NOVAR, 0, 0);
        return(set_errno(EBADIFACE));       
    }

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];

    msg = os_alloc_packet(sizeof(setupstr)+sizeof(term_str), MODEM_ANS_ALLOC);
    if (!msg)
    {
        DEBUG_ERROR("xn_autoanswer - out of DCUs", NOVAR, 0, 0);
        return(set_errno(ENOPKTS));
    }
    send_str = (PFCHAR)DCUTODATA(msg);
    
    rs232_raw_mode(pi, TRUE);
    DEBUG_ERROR("Initializing modem . . .", NOVAR, 0, 0);
    tc_strcpy(send_str, setupstr);
    tc_strcat(send_str, term_str);
    send_modem_command(send_str, pi);

    /* free msg (send_str is no longer needed)   */
    os_free_packet(msg);

#if (!TEST_MODEM)
    n = wait_modem_response(modem_ok, 10, pif_info);
    if (n < 0) 
    {
        DEBUG_ERROR("xn_autoanswer: modem not responding.", NOVAR, 0, 0);
        rs232_raw_mode(pi, FALSE);
        return(set_errno(EMODEMBADRESP));
    }

    DEBUG_ERROR("Waiting for ring . . .", NOVAR, 0, 0);
#endif

    initrawdatabuf(pif_info);

#if (!TEST_MODEM)
    n = wait_modem_response("RING", timeout, pif_info);
    if (n < 0)
    {
        DEBUG_ERROR("xn_autoanswer: no ring.", NOVAR, 0, 0);
        rs232_raw_mode(pi, FALSE);
        return (set_errno(EMODEMNORING));
    }

    DEBUG_ERROR("Ring detected", NOVAR, 0, 0);
#endif

    n = wait_modem_response("CONNECT", timeout, pif_info);
    if (n < 0)
    {
        DEBUG_ERROR("xn_autoanswer: no connect.", NOVAR, 0, 0);
        rs232_raw_mode(pi, FALSE);
        return(set_errno(EMODEMBADRESP));
    }

    DEBUG_ERROR("Answering . . .", NOVAR, 0, 0);
    rs232_raw_mode(pi, FALSE);
    return 0;
}

/* ********************************************************************      */
/* xn_hangup() - hangs up modem                                              */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   int xn_hangup(iface_no, term_str)                                       */
/*      int iface_no    - interface number                                   */
/*      PFCHAR term_str - termination string for sending commands            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  For more details see the RTIP Manual.                                    */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_hangup(int iface_no, PFCHAR term_str)    /* __fn __ */
{
PIFACE pi;
char hang_str[100];
PRS232_IF_INFO pif_info;
#if defined(MODEM_CONNECTED)
PUART_INFO uinfo;
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
    {
        DEBUG_ERROR("xn_hangup - interface is not open", NOVAR, 0, 0);
        return(set_errno(EBADIFACE));       
    }

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];

    ks_sleep((word)(3*ks_ticks_p_sec()));

    rs232_raw_mode(pi, TRUE);

    /* put modem off line   */
    if (!send_modem_command(modem_off_line, pi))
    {
        rs232_raw_mode(pi, FALSE);
        return(set_errno(EMODEMSENDFAILED));
    }

    /* hardwired to 3 seconds - should be a config?   */
    if (wait_modem_response(modem_ok, 3, pif_info) < 0)
    {
        DEBUG_ERROR("xn_hangup: no response to +++, probably already in command mode", NOVAR, 0, 0);
    }

    /* send the hangup command   */
    tc_strcpy(hang_str, modem_ath0);
    tc_strcat(hang_str, term_str);
    if (!send_modem_command(hang_str, pi))
    {
        rs232_raw_mode(pi, FALSE);
        return(set_errno(EMODEMSENDFAILED));
    }

    rs232_raw_mode(pi, FALSE);

#if defined(MODEM_CONNECTED)  /* [i_a] 16550 only now; maybe others later too... */

    /* MODEM_CONNECTED is a uart porting layer issue (see uartport.c)   */
    /* but it should be done here and it won't hurt anything if         */
    /* it is done for a port which doesn't need this flag               */
    uinfo = get_uinfo_struct(pi->minor_number);
    if (uinfo)
        uinfo->flags &= ~MODEM_CONNECTED;
#endif

    return 0;
}

#endif      /* end of INCLUDE_MODEM */

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
/* ********************************************************************            */
/* xn_attach() - Attach interface to a point-to-point remote host                  */
/*                                                                                 */
/* Summary:                                                                        */
/*   #include "rtipapi.h"                                                          */
/*                                                                                 */
/*   int xn_attach(device_id, minor_number, his_ip_address, port_com, baud_rate)   */
/*      int  device_id          - Device Type (see possible definitions in rtip.h) */
/*      int  minor_number       - Minor number for the device                      */
/*      PFBYTE his_ip_address   - The ip address of remote host. Four bytes        */
/*      int  port_com           - Comm number (CSLIP/SLIP/PPP only - supports      */
/*                                COM1-COM4)                                       */
/*      int baud_rate           - Baud rate (12=1200,24=2400,96=9600,              */
/*                                192=19200,384=38400,1152=115200)                 */
/*      char handshake_type     - N=NONE, R = RTS/CTS, D = DTR/DSR                 */
/*      PFBYTE input_buffer     - buffer used to queue input chars which           */
/*                                will be processes at task level.  If             */
/*                                none specified, a DCU will be allocated          */
/*                                by the uart driver.                              */
/*      int input_buffer_len    - size of input buffer                             */
/*      PFBYTE output_buffer    - buffer used to queue output chars which          */
/*                                will be sent from the interrupt level.  If       */
/*                                none specified, a DCU will be allocated by       */
/*                                the uart driver.                                 */
/*      int output_buffer_len   - size of output buffer                            */
/*                                                                                 */
/*                                                                                 */
/* Description:                                                                    */
/*                                                                                 */
/*   Opens interface to a dedicated host for a point-to-point interface.           */
/*   xn_attach() is equivalent to xn_interface_open() but is used for              */
/*   dedicated interfaces.                                                         */
/*                                                                                 */
/*   If his_ip_address is 0.0.0.0, then the remote IP address is unknown           */
/*   at this point and a network entry will be added to the routing                */
/*   table by xn_set_ip() instead of a host entry.  For a remote                   */
/*   address to match a host entry in the routing table it must                    */
/*   match the entry exactly whereas it will match the network entry               */
/*   after it is anded with the mask.  When PPP is opened (by calling              */
/*   xn_lcp_open), if the address is negotiated pi->addr.his_ip_address            */
/*   will be modified to reflect the negotiated IP address.                        */
/*                                                                                 */
/*   For more details see the RTIP Manual.                                         */
/*                                                                                 */
/* Returns:                                                                        */
/*   The interface number, 0-CFG_NIFACES-1 (i.e. offset into interface table,      */
/*   PIFACE), if the interface is opened successfully.                             */
/*   -1 if not successful                                                          */
/*                                                                                 */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set        */
/*   by this function.  For error values returned by this function see             */
/*   RTIP Reference Manual.                                                        */
/*                                                                                 */

int xn_attach(int device_id, int minor_number, PFBYTE his_ip_address, /*__fn__*/
              int comm_port, int baud_rate, char handshake_type,       /*__fn__*/
              PFBYTE input_buffer, int input_buffer_len,
              PFBYTE output_buffer, int output_buffer_len)                        /*__fn__*/
{
PIFACE pi;
int iface_no;
PRS232_IF_INFO pinfo;

    RTIP_API_ENTER(API_XN_ATTACH)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_ATTACH)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

#if (INCLUDE_ERROR_CHECKING)
    if ( (device_id != SLIP_DEVICE) && (device_id != PPP_DEVICE) &&
         (device_id != CSLIP_DEVICE) )
    {
        RTIP_API_EXIT(API_XN_ATTACH)
        return(set_errno(EINVAL));  
    }
#endif

    pinfo = (PRS232_IF_INFO)&rs232_if_info_arry[minor_number];
    pi = pinfo->rs232_pi;
    
    /* if already open   */
    if (pi)
    {
        OS_CLAIM_IFACE(pi, ATTACH_CLAIM_IFACE)
        /* check pi again in case it closed   */
        if (pinfo->rs232_pi && pinfo->rs232_pi->open_count)
        {
            pi->open_count++;
            OS_RELEASE_IFACE(pi)
            RTIP_API_EXIT(API_XN_ATTACH)
            return(pi->ctrl.index);
        }
        OS_RELEASE_IFACE(pi)
    }

    pinfo->index = minor_number;

    /* Handshake type (N)one (D)tr/Dsr (R)ts/Cts   */
    pinfo->handshake_type = handshake_type;

    /* save buffer info;   */
    pinfo->input_buffer = input_buffer;
    pinfo->input_buffer_len = input_buffer_len;
    pinfo->output_buffer = output_buffer;
    pinfo->output_buffer_len = output_buffer_len;

    /* save values; NOTE: values may be overwritten with values from   */
    /* device table                                                    */
    pinfo->comm_port = comm_port;
    pinfo->baud_rate =  baud_rate;
    pinfo->handshake_type = handshake_type;

    iface_no = tc_interface_open(device_id, minor_number, 0, -1, 0);
    if (iface_no == -1)
    {
        RTIP_API_EXIT(API_XN_ATTACH)
        return(-1);
    }
    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
        return(-1);   

    /* save his IP address                                      */
    /* NOTE: if the interface was opened, his ip address is set */
    tc_mv4(pi->addr.his_ip_addr, his_ip_address, IP_ALEN);

    tc_mv4(pi->addr.my_net_bc_ip_addr, his_ip_address, 
           IP_ALEN);

    RTIP_API_EXIT(API_XN_ATTACH)
    return(iface_no);
}
#endif  /* INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP */


