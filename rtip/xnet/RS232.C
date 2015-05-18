/*
 * RS232.C                                                  Version 1.0
 *
 * This module contains the interface between PPP/SLIP and the
 * uart driver.
 *
 * Copyright Peter Van Oudenaren , 1993
 * All rights reserved.
 * This code may not be redistributed in source or linkable object form
 * without the consent of its author.
 *
 * Authors: Frank Rybicki and Bill Schubert
 * Rewritten: Ralph Moore 6/7/94 for use with SLIP
 *
 * ********************************************************************
 * The device driver to support SLIP/PPP consists of 3 layers:
 *   PPP/SLIP - interfaces directly with the RS232 layers.
 *   RS232    - interface between PPP and uart.
 *   UART     - interface between RS232 layer and hardware.
 *              The UART layer is broken into the UART layer (uart.c)
 *              and the UART Porting layer (uartport.c)
 *
 * When transmitting the data is treated as follows:
 *   The packet needs to be escaped from the DCU to the escape buffer
 *   The characters from the escape buffer are put in the circular
 *      output buffer
 *   Characters from the circular output buffer are transmitted to
 *      the UART hardware
 *
 * When transmitting a packet the following code is executed:
 *   slip_xmit/ppp_xmit - calls rs232_xmit_done 
 *   rs232_xmit_done    - calls SLIP/PPP layer (slip_xmit_escape
 *                        or ppp_xmit_escape) to transfer characters
 *                        from the DCU to the escape buffer escaping
 *                        them when necessary
 *                      - calls rs232_xmit_uart
 *                      - returns TRUE if xmit done (i.e. written to
 *                        circular buffer or error or returns
 *                        FALSE if xmit not done due to circular 
 *                        buffer being full.
 *   rs232_xmit_uart    - calls uart_send
 *                        returns false upon hardware error, otherwise
 *                        returns TRUE
 *   uart_send          - copies as many characters from escape buffer
 *                        to the circular buffer as possible.  
 *                      - If there is not enough room in the circular
 *                        buffer sets up condition so when there is
 *                        enough room, the IP task (tc_process_output)
 *                        will be woken up (by calling ks_invoke_output).  
 *                      - starts the transmitter
 *                      - Returns number of bytes written to circular
 *                        buffer.
 *   tc_process_output  - If a transmit is in progress, calls rs232_xmit_done.  
 *                        If rs232_xmit_done returns the xmit completed, 
 *                        a new packet is transmitted by calling slip_xmit/
 *                        ppp_xmit.  If it has not completed, there is
 *                        no further processing to do at that time.
 *
 *                                          -----------
 *                                          |          \
 *            slip_xmit/ppp_xmit   tc_process_output    |
 *                        \         /                   |     
 *                       rs232_xmit_done                | not called
 *                            |                         | directly but
 *                slip_xmit_escape/ppp_xmit_escape      | invoked via
 *                           |                          | uart_interrupt
 *                      rs232_xmit_uart                 |    calling
 *                           |                          | ks_invoke_output
 *                       uart_send                      | when output circular
 *                           \--------------------------  buffer is empty enough
 *
 * NOTE: drivers which do not need to use a circular buffer need
 *       to call ks_invoke_output when the packet has been transmitted
 *
 * NOTE: drivers which need to use the circular buffer can be implemented
 *       by modifying the UART porting layer (uartport.c)
 * NOTE: drivers which do not need to use a circular buffer can
 *       be implemented by rewritting the routines in the UART
 *       layer (uart.c)
 * ********************************************************************/

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

#include "sock.h"  
#include "rtip.h"  
#include "rtipext.h"  

#define PPP_ORIG 1

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_RS232    0
#define DEBUG_PPP_XMIT 0

#if (DEBUG_RS232)
long toss_no_msg_in = 0;
long toss_no_pb_in = 0;
long toss_ether = 0;
long bytes_in = 0;
#endif

/* ********************************************************************   */
#if (INCLUDE_SLIP || INCLUDE_CSLIP)
RTIP_BOOLEAN slip_xmit_escape(PRS232_IF_INFO pif_info);
void    slip_give_string(PRS232_IF_INFO pif_info, PFBYTE buffer, int n);
#endif
void display_rs232(int minor_number);

/* ********************************************************************   */
RTIP_BOOLEAN rs232_init(PIFACE pi)
{
PRS232_IF_INFO pif_info;
RTIP_BOOLEAN ret_val;

    /* get free info structure for this interface   */
    if (pi->minor_number >= CFG_NUM_RS232)  /* range 0..CFG_NUM_RS232-1 */
    {
        DEBUG_ERROR("rs232_init to large a interface offset = ", 
            EBS_INT1, pi->minor_number, 0);
        set_errno(EIFACEOPENFAIL);
        return(FALSE);
    }
    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];

    if (pif_info->comm_port > CFG_COMM_NUM) /* range 1..CFG_COMM_NUM */
    {
        set_errno(EBADCOMMNUM);
        return(FALSE);
    }
    pif_info->index = pi->minor_number;
    pif_info->rs232_pi = pi;

    /* ******                                                          */
    /* for RS232, set up defaults here                                 */
    /* use values from parameters passed to xn_attach or device table  */
    /* as defaults; pass them to open                                  */
    if (pif_info->comm_port == -1) 
        pif_info->comm_port = (word)pi->pdev->default1.comm_port; 
    if (pif_info->baud_rate == 0)
        pif_info->baud_rate = pi->pdev->default2.baud_rate; 
    if (pif_info->handshake_type == '\0')
        pif_info->handshake_type = pi->pdev->default3.handshake_type;

    if ( (pi->pdev->device_id == SLIP_DEVICE) ||
         (pi->pdev->device_id == CSLIP_DEVICE) )
        pif_info->rs232_end = SLIP_END;
#if (INCLUDE_PPP)
    else
        pif_info->rs232_end = HDLC_FLAG;
#endif

    /* ******                                          */
    /* initialize global DCU for framing input         */
    /* just clear msg_in the rest is done by msg_alloc */
    pif_info->msg_in = (DCU)0;

    ret_val = uart_init(pi->minor_number, 
                        pi,
                        pif_info->handshake_type,
                        pif_info->baud_rate,
                        pif_info->comm_port,
                        pif_info->rs232_end,
                        pif_info->input_buffer,
                        pif_info->input_buffer_len,
                        pif_info->output_buffer,
                        pif_info->output_buffer_len
                        );

#if (!PPP_ORIG)
    /* let uart_send know where data to be output is   */
    pif_info->out_buf_esc_in = pif_info->out_buf_esc_out = 
        pif_info->out_buf_esc_base;
    pif_info->nout_esc = 0;
#endif

    /* initialize for escaping and sending output   */
    return(ret_val);
}

/* ********************************************************************   */
/* TRANSMIT SUPPORT ROUTINES                                              */
/* ********************************************************************   */
/* Called to perform initialization to transmit each new packet           */
/* offset is offset in packet to start sending                            */
void rs232_xmit_init(PRS232_IF_INFO pif_info, DCU msg, int offset)
{
PFBYTE packet;
int length;

    packet = DCUTODATA(msg) + offset;
    length = DCUTOPACKET(msg)->length - offset;

#if (PPP_ORIG)
    /* let uart_send know where data to be output is   */
    pif_info->out_buf_esc_in = pif_info->out_buf_esc_out = 
        pif_info->out_buf_esc_base;
    pif_info->nout_esc = 0;
#endif

    /* let xxx_xmit_escape know where data to be output is   */
    pif_info->packet_out = packet;
    pif_info->packet_out_left = length;
}

/* ********************************************************************   */
/* calls uart_send to send bytes in following buffer:                     */
/*   pif_info->out_buf_esc_out = buffer to send                           */
/*   pif_info->nout_esc        = amount to send                           */
/* returns FALSE if error and we reset buffers                            */
RTIP_BOOLEAN rs232_xmit_uart(PRS232_IF_INFO pif_info)
{
int n_sent;
RTIP_BOOLEAN ret_val;

#if (DEBUG_RS232)
    DEBUG_ERROR("rs232_xmit_uart called", NOVAR, 0, 0);
#endif
    ret_val = TRUE;

    n_sent = uart_send(pif_info->index, pif_info->out_buf_esc_out, 
                       pif_info->nout_esc);
#if (DEBUG_RS232)
    DEBUG_ERROR("rs232_xmit_uart: sent bytes: ", EBS_INT1, n_sent, 0);
#endif
    if (n_sent < 0)
    {
        pif_info->stats.errors_out += 1;
        ret_val = FALSE;        /* error */
    }
    else
    {
        pif_info->out_buf_esc_out += n_sent;
        pif_info->nout_esc    -= n_sent;
    }

/*HERE                                 */
    /* if done or error, reset buffers */
    if (pif_info->out_buf_esc_out >= pif_info->out_buf_esc_in)
    {
        /* reset esc buffer pointer since all chars in esc buffer   */
        /* were copied to circular buffer                           */
        pif_info->out_buf_esc_in = pif_info->out_buf_esc_out = 
            pif_info->out_buf_esc_base;
        pif_info->nout_esc = 0;
    }

    return(ret_val);
}

/* ********************************************************************   */
int escape_chars(PIFACE pi, PRS232_IF_INFO pif_info)
{
#if (INCLUDE_SLIP || INCLUDE_CSLIP)
    if ( (pi->pdev->device_id == SLIP_DEVICE) ||
         (pi->pdev->device_id == CSLIP_DEVICE) )
    {
        if (!slip_xmit_escape(pif_info))
            return(FALSE);      /* xmit not done due to error or buffer full */
    }
#endif
#if (INCLUDE_PPP)
    if (pi->pdev->device_id == PPP_DEVICE)
    {
        if (ppp_escape_routine)
        {
            if (!ppp_escape_routine(pif_info))
            {
                return(FALSE);      /* xmit not done due to error or buffer full */
            }
        }
        else
        {
            DEBUG_ERROR("escape_chars: ppp_escape_routine not set up",
                NOVAR, 0, 0);
            return(TRUE);   /* xmit done */
        }
    }
#endif
#if (!PPP_ORIG || 1)
    return(TRUE);       /* spr - patch */
#endif
}

/* ********************************************************************   */
/* escapes characters, starts transmitter                                 */
/* returns TRUE if xmit done or error; xmit_done means all chars          */
/*              are in the circular buffer                                */
/*         FALSE if xmit not done due to cirular buffer being full.       */
RTIP_BOOLEAN rs232_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PRS232_IF_INFO pif_info;
int  minor_number;

    ARGSUSED_INT(success);

    minor_number = pi->minor_number;

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[minor_number];

    escape_chars(pi, pif_info);

    /* if there are unsent chars in escaped buffer then send them;   */
    /* if uart_send fails or esc buffer does not empty (which would  */
    /* be due to circular buffer filling up) then we return          */
    if (pif_info->out_buf_esc_out < pif_info->out_buf_esc_in)
    {
        if (!rs232_xmit_uart(pif_info))
        {
            return(FALSE);      /* error */
        }
    }

    /* if more data in DCU which has not been escaped, escape them   */
    /* and call uart send                                            */
    if (pif_info->packet_out_left)
    {
        escape_chars(pi, pif_info);
    }

    /* if not all chars have been escaped ||                            */
    /* not everything written from esc buffer to circular buffer        */
    /* out_buf_esc_base = base of esc buffer                            */
    /* out_buf_esc_out  = point to nxt char to send thru uart           */
    /* out_buf_esc_in   = pointer to nxt char to put in circular buffer */
    /*                    (points to base of esc buffer when empty)     */
    if ( pif_info->packet_out_left ||
         (pif_info->out_buf_esc_out < pif_info->out_buf_esc_in) )
    {
#if (DEBUG_PPP_XMIT)
        DEBUG_ERROR("rs232_xmit_done: NOT all queued",
            NOVAR, 0, 0);
#endif
        return(FALSE);      /* xmit not done */
    }

    /* send must be done; at least all the chars are in the circular   */
    /* output buffer so let xmit done know the xfer has completed      */
    pif_info->stats.bytes_out += DCUTOPACKET(msg)->length;
#if (DEBUG_PPP_XMIT)
    DEBUG_ERROR("rs232_xmit_done: all queued - return TRUE",
        NOVAR, 0, 0);
#endif
    return(TRUE);       /* send done */
}

#if (INCLUDE_MODEM)
/* ********************************************************************   */
/* RAW MODE                                                               */
/* ********************************************************************   */
void rs232_raw_mode(PIFACE pi, RTIP_BOOLEAN raw_on)
{
PRS232_IF_INFO pif_info;
DCU msg;

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];

    if (pif_info && raw_on != pif_info->raw_mode)
    {
        if (raw_on)
        {
            DEBUG_ASSERT(!pif_info->raw_mode_buffer_dcu,
                "Warning: msg not null, re-entering raw mode",NOVAR,0,0);
            if (!pif_info->raw_mode_buffer_dcu)
            {
                msg=os_alloc_packet(MAX_PACKETSIZE, RS232_RAW_ALLOC);
        
                if (!msg)
                {
                    DEBUG_ERROR("rs232_raw_mode() - out of DCUs", NOVAR, 0, 0);
                    return;
                }
        
                pif_info->raw_mode_buffer = (PFCHAR)DCUTODATA(msg);
                pif_info->raw_mode_buffer_dcu = msg;
                pif_info->raw_mode_index=0;
            }
        }
        else
        {
            if (pif_info->raw_mode_buffer_dcu)
            {
                os_free_packet(pif_info->raw_mode_buffer_dcu);
            }
            pif_info->raw_mode_buffer_dcu = 0;
            pif_info->raw_mode_buffer = 0;
        }

        pif_info->raw_mode = raw_on;

        /* Handle raw mode processing   */
        uart_raw_mode(pi->minor_number, raw_on);
    }
}
#endif


/* ********************************************************************   */
/* CLOSE INTERFACE                                                        */
/* ********************************************************************   */
void rs232_close(PIFACE pi)
{
PRS232_IF_INFO pif_info;

    uart_close(pi->minor_number);
    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];
    if (pif_info->msg_in)
        os_free_packet(pif_info->msg_in);
    pif_info->msg_in = 0;
}

/* ********************************************************************   */
/* ERROR LOGGING                                                          */
/* ********************************************************************   */
/* Called back from uart driver.                                          */
/*   Inputs:                                                              */
/*      channel: uart number 0 to (CFG_NUM_RS232 - 1)                     */
/*  error :                                                               */
/*      0 == overrun                                                      */
/*      1 == parity                                                       */
/*      2 == framing                                                      */
/*      3 == dropped a character. Out of input buffer space               */
/*                                                                        */
/* NOTE: this can be called from interrupt service routine                */

void rs232_error(int minor_number, int error)
{
PRS232_IF_INFO pif_info;      

    DEBUG_ERROR("rs232_error: ", EBS_INT1, error, 0);
    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[minor_number];
    if (error == 0)
        pif_info->stats.overrun_errors_in++;
    else if (error == 1)
        pif_info->stats.parity_errors_in++;
    else if (error == 2)
        pif_info->stats.framing_errors_in++;
}
    
/* ********************************************************************      */
/* EVENT SIGNALING                                                           */
/* ********************************************************************      */
/* Called back from uart driver.                                             */
/*   Inputs:                                                                 */
/*      minor device number 0 to (CFG_NUM_RS232 - 1)                         */
/*  event :                                                                  */
/*      1 == data received                                                   */
/*      2 == buffered data has been send                                     */
/*      3 == modem line drop occured                                         */
/*                                                                           */
/* NOTE: this can be called from interrupt service routine                   */

void rs232_signal_event(int minor_device_number, int event)
{
PRS232_IF_INFO pif_info;      

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[minor_device_number];

    if (event == 1)
          ks_invoke_interrupt(pif_info->rs232_pi);

    else if (event == 2)
          ks_invoke_output(pif_info->rs232_pi, 1);

#if (INCLUDE_MODEM && INCLUDE_PPP)
    else if (event == 3)
    {
        if ( pif_info->rs232_pi->pdev &&
             (pif_info->rs232_pi->pdev->device_id == PPP_DEVICE) )
        {
             /* signal xn_ppp_wait_down that connect is terminating     */
            OS_SET_PPP_SIGNAL(pif_info->rs232_pi);

            /* signal IP layer to do callback to application      */
            /* (i.e. call rs232_connection_drop)                  */
            /* NOTE: can't do callback here since in interrupt    */
            /* service routine                                    */
            signal_ip_via_dcu(pif_info->rs232_pi, MODEM_DROP);
        }
    }
#endif
}

/* ********************************************************************   */
/* CALLBACK INPUT ROUTINES (called from uart input routine)               */
/* ********************************************************************   */
void rs232_give_string(int minor_number, PFBYTE buffer, int n)
{
PRS232_IF_INFO pif_info;
#if (INCLUDE_MODEM)
int i;
#endif

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[minor_number];
#if (INCLUDE_MODEM)
    /* Handle raw mode processing   */
    if (pif_info->raw_mode)
    {
        for (i=0; i<n; i++)
        {
            buffer_char(buffer[i],pif_info);    /* save char in buffer used by  */
                                                /* modem stuff   */
            CB_RAW_MODE_IN_CHAR(buffer[i]);     /* give the char to the app */
                                                /* via callback routine    */
        }
        return;
    }
#endif

#if (INCLUDE_PPP)
    if (pif_info->rs232_pi->pdev->device_id == PPP_DEVICE) 
    {
        if (ppp_give_string_routine)
            ppp_give_string_routine(pif_info, buffer, n);
        else
        {
            DEBUG_ERROR("rs232_give_string: ppp_give_string_routine not set up",
                NOVAR, 0, 0);
        }
        return;
    }
#endif

#if (INCLUDE_SLIP || INCLUDE_CSLIP)
    if ( (pif_info->rs232_pi->pdev->device_id == SLIP_DEVICE) ||
         (pif_info->rs232_pi->pdev->device_id == CSLIP_DEVICE) )
        slip_give_string(pif_info, buffer, n);
#endif
    return;
}

RTIP_BOOLEAN alloc_msg_in(PRS232_IF_INFO pif_info)
{
int is_ppp_device, length;
    
    is_ppp_device = (pif_info->rs232_pi->pdev->device_id == PPP_DEVICE);

    /* Allocate a message structure for putting data into.   */
    pif_info->msg_in = os_alloc_packet_input(MAX_PACKETSIZE,
                                             is_ppp_device ? PPP_ALLOC : 
                                                             SLIP_ALLOC);
    if (pif_info->msg_in)
    {
        /* PPP we start with len of 0 since there may be control information   */
        /* for slip we always count the ethernet pseudo header                 */
        if (is_ppp_device)
            length = 0;
        else
            length = ETH_HLEN_BYTES;    /* slip */
        DCUTOPACKET(pif_info->msg_in)->length = (word)length;       
        pif_info->pb_in = (PFBYTE) DCUTODATA(pif_info->msg_in);
        pif_info->pb_in += length;
        pif_info->length = length;
        pif_info->toss_chars = FALSE;
        pif_info->esc_chars = FALSE;
#if (INCLUDE_PPP)
        pif_info->in_header = TRUE;
        pif_info->calc_fcs = HDLC_FCS_START;    /* harmless even if slip */
#endif
        return(TRUE);
    }
    else
    {
#if (DEBUG_RS232)
        toss_no_msg_in++;
        DEBUG_ERROR("rs232_give_string: toss_no_msg_in,toss_no_pb_in = ", EBS_INT1,
            toss_no_msg_in, toss_no_pb_in);
        DEBUG_ERROR("rs232_give_string: toss_ether = ", EBS_INT1,
            toss_ether, 0);
#endif
        pif_info->pb_in = 0;
        pif_info->length = 0;
        pif_info->toss_chars = TRUE;
        return(FALSE);
    }
}

#endif  /* end of #if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP) */


