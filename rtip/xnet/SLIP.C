/*                                                                      */
/* SLIP.C - Slip device driver interface                                */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#undef DIAG_SECTION_KERNEL
#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP

#include "sock.h"
#include "rtip.h"
#if (INCLUDE_CSLIP)
#include "slhc.h"
#endif

#if (INCLUDE_SLIP || INCLUDE_CSLIP) 

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
RTIP_BOOLEAN slip_open(PIFACE pi);
void         slip_close(PIFACE pi);
int          slip_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN slip_statistics(PIFACE  pi);

EDEVTABLE KS_FAR slip_device = 
{
    slip_open, slip_close, slip_xmit, rs232_xmit_done,
    NULLP_FUNC, slip_statistics, NULLP_FUNC, 
    SLIP_DEVICE, "SLIP-UART", MINOR_0, RS232_IFACE, 
    SNMP_DEVICE_INFO(CFG_OID_SLIP, 0)
    CFG_RS232_MAX_MTU, CFG_RS232_MAX_MSS, 
    CFG_RS232_MAX_WIN_IN, CFG_RS232_MAX_WIN_OUT, 
    IOADD(DFLT_PORT), EN(96), EN('N')
};

#endif

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
extern EDEVTABLE KS_FAR slip_device;
#endif


/* ********************************************************************           */
/* Pointer to the interface structure. We look at this inside the receive routine */
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR phony_en_addr[ETH_ALEN];

extern RS232_IF_INFO KS_FAR rs232_if_info_arry[CFG_NUM_RS232];

/* ********************************************************************   */
RTIP_BOOLEAN alloc_msg_in(PRS232_IF_INFO pif_info);
RTIP_BOOLEAN rs232_queue(unsigned char c, PIFACE pi);  
RTIP_BOOLEAN rs232_send(PIFACE pi);
RTIP_BOOLEAN rs232_init(PIFACE pi);
void    rs232_close(PIFACE pi);
int     decode_cslip(PRS232_IF_INFO pif_info, DCU msg);

/* ********************************************************************      */
/* SLIP OPEN                                                                 */
/* ********************************************************************      */
/* open the slip driver interface.                                           */
/*                                                                           */
/* This routine opens a slip device driver and copies a fake                 */
/* ethernet address into the interface structure. If the open is successful  */
/* it returns TRUE otherwise FALSE.                                          */
/*                                                                           */
/* The address of this function must be placed into the "devices" table in   */
/* iface.c either at compile time or before a device open is called.         */
/*                                                                           */
RTIP_BOOLEAN slip_open(PIFACE pi)                            /*__fn_*/
{
PRS232_IF_INFO pif_info;

    if (!rs232_init(pi))
    {
        set_errno(EDEVOPENFAIL);
        return(FALSE);
    }

    /* Now put in a dummy ethernet address        */
    tc_movebytes(pi->addr.my_hw_addr, phony_en_addr, 6); /* Get the ethernet address */

    /* clear statistics; NOTE: minor_number was setup before this routine   */
    /* was called                                                           */
    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];
    tc_memset((PFBYTE)&(pif_info->stats), 0, sizeof(struct rs232_statistics));

#if (INCLUDE_CSLIP)
    pif_info->cslip_slhcp = slhc_init(pi, CFG_CSLIP_SLOTS, CFG_CSLIP_SLOTS);
#endif

    return(TRUE);
}

/* ********************************************************************    */
/* SLIP CLOSE                                                              */
/* ********************************************************************    */
/* close the packet driver interface.                                      */
/*                                                                         */
/* This routine is called when the device interface is no longer needed    */
/* it should stop the driver from delivering packets to the upper levels   */
/* and shut off packet delivery to the network.                            */
/*                                                                         */
/* The address of this function must be placed into the "devices" table in */
/* iface.c either at compile time or before a device open is called.       */
/*                                                                         */
/*                                                                         */
/*                                                                         */

void slip_close(PIFACE pi)                                /*__fn__*/
{

#if (INCLUDE_CSLIP)
    slhc_free(((PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number])->cslip_slhcp);
#endif

    /* The interface structure is not used   */
    rs232_close(pi);

}

/* ********************************************************************   */
/* SLIP XMIT                                                              */
/* ********************************************************************   */

/* ********************************************************************   */
/* escape chars from pif_info->packet_out to pif_info->out_buf_esc_in     */
/* returns TRUE if xmit complete or FALSE if xmit error or xmit not done  */
RTIP_BOOLEAN slip_xmit_escape(PRS232_IF_INFO pif_info)
{
    while (pif_info->packet_out_left)
    {
        if ( (pif_info->nout_esc > CFG_RS232_XMBUFSIZE) ||
             (pif_info->out_buf_esc_base+CFG_RS232_XMBUFSIZE <=     
              pif_info->out_buf_esc_in+1) )         /* end of buf */
                                                    /* where write char;   */
                                                    /* +1 incase esc char  */
        {
            rs232_xmit_uart(pif_info);
            uart_setouttrigger(pif_info, 0);
            return(FALSE);
        }
        if (*pif_info->packet_out == SLIP_END)
        {
            *pif_info->out_buf_esc_in++ = SLIP_ESC; 
            pif_info->nout_esc++;
            *pif_info->out_buf_esc_in++ = SLIP_ESC_END; 
        }
        else if (*pif_info->packet_out == SLIP_ESC)
        {
            *pif_info->out_buf_esc_in++ = SLIP_ESC; 
            pif_info->nout_esc++;
            *pif_info->out_buf_esc_in++ = SLIP_ESC_ESC; 
        }
        else
        {
            *pif_info->out_buf_esc_in++ = *pif_info->packet_out; 
        }
        pif_info->nout_esc++;

        pif_info->packet_out++;
        pif_info->packet_out_left--;
    }
    *pif_info->out_buf_esc_in = SLIP_END; 
    pif_info->nout_esc++;

    if (!rs232_xmit_uart(pif_info))
        return(FALSE);

    /* success   */
    return(TRUE);
}

/* ********************************************************************      */
/* Transmit. a packet over the packet driver interface.                      */
/*                                                                           */
/* This routine is called when a packet needs sending. The packet contains a */
/* full ethernet frame to be transmitted. The length of the packet is        */
/* provided.                                                                 */
/*                                                                           */
/* The address of this function must be placed into the "devices" table in   */
/* iface.c either at compile time or before a device open is called.         */
/*                                                                           */
/* Returns 0 upon success and errno if error                                 */

int slip_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
#if (INCLUDE_CSLIP)
int type = 0;       /* initialize to keep compiler happy */
#endif
PRS232_IF_INFO pif_info;
int  minor_number;

    if (DCUTOPACKET(msg)->length == 0)
    {
        DEBUG_ERROR("slip_xmit: length = ", EBS_INT1, 
            DCUTOPACKET(msg)->length, 0);
        return(REPORT_XMIT_DONE);
    }

    minor_number = pi->minor_number;
    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[minor_number];

    DEBUG_LOG("PACKET = ", LEVEL_3, PKT, 
        DCUTODATA(msg), DCUTOPACKET(msg)->length);

#if (INCLUDE_CSLIP && INCLUDE_CSLIP_XMIT) /* If user requested CSLIP, */
    if (pi->pdev->device_id == CSLIP_DEVICE)
    {
        /* NOTE: this has to be done after compressing in case compression   */
        /*       allocates a new packet                                      */
        type = slhc_compress(pi, pif_info->cslip_slhcp, &msg, TRUE);
    }

    /* initialize xmit buffers (for escaped buffer and pointers to packet   */
    /* which is being sent)                                                 */
    /* NOTE: must be done after compression in case alloced a new           */
    /*       message                                                        */
    rs232_xmit_init(pif_info, msg, ETH_HLEN_BYTES);

    if (pi->pdev->device_id == CSLIP_DEVICE)
        *pif_info->packet_out = (byte)(*pif_info->packet_out | type);   
#else
    /* initialize xmit buffers (for escaped buffer and pointers to packet   */
    /* which is being sent)                                                 */
    rs232_xmit_init(pif_info, msg, ETH_HLEN_BYTES);
#endif

    /* Send an end character so other side flushes   */
    *pif_info->out_buf_esc_in++ = SLIP_END; 
    pif_info->nout_esc++;

    /* escape buffer and start transmitter   */
    if (rs232_xmit_done(pi, msg, TRUE))
        return(REPORT_XMIT_DONE);
    else
        return(0);
}


/* ********************************************************************   */
/* slip_give_string                                                       */
/* ********************************************************************   */
/* Process a SLIP packet which was input over RS232 port                  */
/* NOTE: frees input packet when done                                     */

/* =============================================================================   */
void slip_give_string(PRS232_IF_INFO pif_info, PFBYTE buffer, int n)
{
int i;
byte chr, in_chr;
#if (INCLUDE_CSLIP)
byte type;
#endif
RTIP_BOOLEAN toss_chars = FALSE;
RTIP_BOOLEAN esc_chars = FALSE;
RTIP_BOOLEAN resynch_vars, send_it;
int length = 0;
PFBYTE pb_in = (PFBYTE)0;
DCU msg = (DCU)0;

    /* Resyncronize state variables to start   */
    resynch_vars = TRUE;

    for (i=0; i<n; i++)
    {
        /* Allocate a buffer and reset state if needed      */
        if (!pif_info->msg_in)
        {
            alloc_msg_in(pif_info);
            resynch_vars = TRUE;
        }

        /* Resync local copy of variables if new packet or first time in   */
        if (resynch_vars)
        {
            resynch_vars = FALSE;
            /* fast copies of pinfo state info   */
            length = pif_info->length;
            pb_in = pif_info->pb_in;
            esc_chars =     pif_info->esc_chars;
            toss_chars =    pif_info->toss_chars;
            msg = pif_info->msg_in;
        }

        in_chr = chr = *buffer++;
        if (!toss_chars)
        {
            if (chr == SLIP_ESC)
            {
                esc_chars = TRUE;
                continue;
            }
            if (esc_chars)
            {
                esc_chars = FALSE;
                if (chr == SLIP_ESC_END)        chr = SLIP_END;
                else if (chr == SLIP_ESC_ESC)   chr = SLIP_ESC;
            }
            *pb_in++ = chr;
            length++;
        }
        if (in_chr != SLIP_END)
        {
            if (length > MAX_PACKETSIZE)    /* Overflow condition */
                toss_chars = TRUE;
        }
        else
        {
            /* Got a frame seperator. decompress if need and enque it   */
            length--;   /* subtract the end char */
            /* Tossing or empty packet ??   */
            if (toss_chars || length == ETH_HLEN_BYTES)
                os_free_packet(msg);
            else
            {
                /* Submit the packet   */
                send_it = TRUE;
                pif_info->stats.bytes_in += (length-ETH_HLEN_BYTES);
                DCUTOPACKET(msg)->length = (word) length;
                DCUTOCONTROL(msg).dcu_flags = NO_DCU_FLAGS;   /* do not keep, do not signal */

                /* Send the packet to IP.   */
                ((PETHER)(DCUTODATA(msg)))->eth_type = EIP_68K;
                tc_movebytes(((PETHER)(DCUTODATA(msg)))->eth_dest, 
                             phony_en_addr, ETH_ALEN); /* Get the ethernet address */

                /* Send ether packet off to the IP dispatcher to handle   */
                /* NOTE: interface number is put in DCU by ip tasks       */
                DEBUG_LOG("slip_interpret - packet sent to ip exc", LEVEL_3, NOVAR, 0, 0);

#if (INCLUDE_CSLIP)
                /* set up pointer to IP packet (done by tc_ip_interpret but   */
                /* this code executes before tc_ip_interpret)                 */
                DCUSETUPIPARP(msg, 0)

                /* Decompress the packet, if it is CSLIP   */
                type = (byte)((DCUTOIPPKT(msg)->ip_verlen) & 0xf0);  /* Mask out low bits (length) */
                switch(type)
                {
                case SL_TYPE_IP: /* Do nothing; this is not a CSLIP packet. */
                    break;
                case SL_TYPE_UNCOMPRESSED_TCP: /* This is an uncompressed packet, so we need to remember it. */
                    slhc_remember(pif_info->cslip_slhcp,msg);
                    break;
                default:
                    if (!(type & SL_TYPE_COMPRESSED_TCP)) /* If this bit isn't set, we have error... */
                    {
                        /* Error... this isn't an ip packet at all...   */
                        DEBUG_ERROR("decode_cslip: Invalid packet type rcvd, type=",EBS_INT1,type,0);
                        send_it = FALSE;
                        break;
                    }  /*But if the compressed bit is set, then continue:  (no break) */
                case SL_TYPE_COMPRESSED_TCP:
                    /* Compressed; decompress it.   */
                    slhc_uncompress(pif_info->cslip_slhcp, msg);
                    break;
                }
#endif
                if (send_it)
                {
                    pif_info->stats.packets_in++;
                    OS_SNDX_IP_EXCHG(pif_info->rs232_pi, msg);
                }
                else
                    os_free_packet(msg);
            }
            /* Null the input message so we re-allocate and resynch on 
               the next char */
            msg = pif_info->msg_in = (DCU)0;
        }
    }
    /* We're leaving so update the pif_info structure   */
    pif_info->pb_in = pb_in;
    pif_info->toss_chars = toss_chars;
    pif_info->esc_chars =  esc_chars;
    pif_info->length = length;
}

/* ********************************************************************          */
/* SLIP STATISTICS                                                               */
/* ********************************************************************          */
/* Statistic. return statistics about the device interface                       */
/*                                                                               */
/* This routine is called by user code that wishes to inspect driver statistics. */
/* We call this routine in the demo program. It is not absolutely necessary      */
/* to implement such a function (Leave it empty.), but it is a handy debugging   */
/* tool.                                                                         */
/*                                                                               */
/* The address of this function must be placed into the "devices" table in       */
/* iface.c either at compile time or before a device open is called.             */
/*                                                                               */
/*                                                                               */
/* Non packet drivers should behave the same way.                                */
/*                                                                               */

RTIP_BOOLEAN slip_statistics(PIFACE pi)                       /*__fn__*/
{
#if (INCLUDE_KEEP_STATS)
PRS232_IF_INFO pif_info;
PRS232_STATS p;

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];
    p = (PRS232_STATS) &(pif_info->stats);

    UPDATE_SET_INFO(pi,interface_packets_in, p->packets_in)
    UPDATE_SET_INFO(pi, interface_packets_in, p->packets_in)
    UPDATE_SET_INFO(pi, interface_packets_out, p->packets_out)
    UPDATE_SET_INFO(pi, interface_bytes_in, p->bytes_in)
    UPDATE_SET_INFO(pi, interface_bytes_out, p->bytes_out)
    UPDATE_SET_INFO(pi, interface_errors_in, p->errors_in)
    UPDATE_SET_INFO(pi, interface_errors_out, p->errors_out)
    UPDATE_SET_INFO(pi, interface_packets_lost, p->packets_lost)
#else
    ARGSUSED_PVOID(pi)
#endif
    return(TRUE);
}

/* ********************************************************************   */
#if (INCLUDE_SLIP)
int xn_bind_slip(int minor_number)
{
    return(xn_device_table_add(slip_device.device_id, 
                        minor_number, 
                        slip_device.iface_type,
                        slip_device.device_name,
                        SNMP_DEVICE_INFO(slip_device.media_mib, 
                                         slip_device.speed)                         
                        (DEV_OPEN)slip_device.open,
                        (DEV_CLOSE)slip_device.close,
                        (DEV_XMIT)slip_device.xmit,
                        (DEV_XMIT_DONE)slip_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)slip_device.proc_interrupts,
                        (DEV_STATS)slip_device.statistics,
                        (DEV_SETMCAST)slip_device.setmcast));
}
#endif

#if (INCLUDE_CSLIP)
int xn_bind_cslip(int minor_number)
{
    return(xn_device_table_add(CSLIP_DEVICE,
                        minor_number, 
                        slip_device.iface_type,
                        "CSLIP-UART",
                        SNMP_DEVICE_INFO(slip_device.media_mib, 
                                         slip_device.speed)                         
/* TBD                  SNMP_DEVICE_INFO(CFG_OID_CSLIP, 0)   */
                        (DEV_OPEN)slip_device.open,
                        (DEV_CLOSE)slip_device.close,
                        (DEV_XMIT)slip_device.xmit,
                        (DEV_XMIT_DONE)slip_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)slip_device.proc_interrupts,
                        (DEV_STATS)slip_device.statistics,
                        (DEV_SETMCAST)slip_device.setmcast));
}
#endif

#endif      /* DECLARING_DATA */
#endif  /* end of #if (INCLUDE_SLIP) */
