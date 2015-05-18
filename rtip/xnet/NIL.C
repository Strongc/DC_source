/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"

#if (INCLUDE_NIL)

typdef struct nil_softc
{
    PIFACE iface;
    struct ether_statistics stats;
}

void nil_interrupt(int minor_no);
RTIP_BOOLEAN nil_open(PIFACE pi);
void         nil_close(PIFACE pi);
int          nil_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN nil_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN nil_statistics(PIFACE  pi);
RTIP_BOOLEAN nil_setmcast(PIFACE pi);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
NIL_SOFTC KS_FAR nilsoftc[CFG_NUM_NE2000];

EDEVTABLE KS_FAR nil_device =
{
     nil_open, nil_close, nil_xmit, nil_xmit_done,
     NULLP_FUNC, nil_statistics, nil_setmcast,
     NE2000_DEVICE, "NE2000", MINOR_0, ETHER_IFACE,
     SNMP_DEVICE_INFO(CFG_OID_NE2000, CFG_SPEED_NE2000)
     CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS, 
     CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT, 
     IOADD(0x300), EN(0x0), EN(5)
};
#endif  /* DECLARING_DATA */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
extern NILL_SOFTC KS_FAR nilsoftc[CFG_NUM_NIL];
extern EDEVTABLE KS_FAR nil_device;
#endif

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */

/* ********************************************************************   */
/* nil_close() - close a device                                           */
/*                                                                        */
/* Perform device driver specific processing to disable sending and       */
/* receiving packets.                                                     */
/*                                                                        */
/* Inputs:                                                                */
/*   pi - interface structure of the device to close                      */
/*                                                                        */
/* Returns: nothing                                                       */
/*                                                                        */

void nil_close(PIFACE pi)                     /*__fn__*/
{
    ARGSUSED_PVOID(pi);
}

/* ********************************************************************   */
/* nil_open() - open a device                                             */
/*                                                                        */
/* Perform device driver specific processing to enable sending and        */
/* receiving packets.  Also probes for the device.                        */
/*                                                                        */
/* Inputs:                                                                */
/*   pi - interface structure of the device to open                       */
/*                                                                        */
/* Returns: TRUE if successful, FALSE if failure                          */
/*                                                                        */

RTIP_BOOLEAN nil_open(PIFACE pi)
{
byte tbd;
int  irq_tbd;
int  i;

    sc->iface = pi;
    pi->driver_stats.ether_stats = (PETHER_STATS)&(softc->stats);

    pi->io_address = tbd;
    pi->irq_val    = tbd;

    /* set up local ethernet address   */
    for (i = 0; i < ETH_ALEN; i++)
        pi->addr.my_hw_addr[i] = tbd;

    /* hook the interrupt   */
    ks_hook_interrupt(irq_tbd, (PFVOID) pi, 
                      (RTIPINTFN_POINTER)nil_interrupt,
                      (RTIPINTFN_POINTER)nil_pre_interrupt,
                      pi->minor_number);
    return(TRUE);
}

/* ********************************************************************   */
/* INTERRUPT routines                                                     */
/* ********************************************************************   */
void nil_pre_interrupt(int minor_no)
{
PNE2000_SOFTC sc;

#if (CFG_NUM_NE2000 == 1)
    ARGSUSED_INT(minor_no)
#endif

    sc = off_to_softc(minor_no);

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(sc->ia_irq)
}

/* ********************************************************************      */
/* nil_interrupt() - process driver interrupt                                */
/*                                                                           */
/* Processing routine for device driver interrupt.  It should process        */
/* transmit complete interrupts, input interrupts as well as any             */
/* indicating errors.                                                        */
/*                                                                           */
/* If an input packet arrives it is sent to the IP exchange (which           */
/* will signal the IP task to wake up).                                      */
/*                                                                           */
/* If an output xmit completed, it will wake up the IP tasks to              */
/* process the output list.                                                  */
/*                                                                           */
/* Input:                                                                    */
/*   minor_number - minor number of device driver which caused the interrupt */
/*                                                                           */
/* Returns: nothing                                                          */
/*                                                                           */

void nil_interrupt(int minor_no)
{
#define XMIT_INT_TBD 1
#define RCV_INT_TBD  2
RTIP_BOOLEAN int_type_tbd;
int     len_tbd;
PIFACE  pi_tbd;
DCU     msg;

    /* get pi structure based upon minor number   */
    ARGSUSED_INT(minor_no);

    /* if receive interrupt   */
    if (int_type_tbd == RCV_INT_TBD)
    {
        /* allocate a DCU to store input packet in                */
        /* NOTE: if size is not know, ETHERSIZE plus any room for */
        /*       crc etc should be allocated                      */
        msg = os_alloc_packet_input(len_tbd, DRIVER_ALLOC);
        if (!msg)
        {
            DEBUG_ERROR("nil_interrupt: out of DCUs", NOVAR, 0, 0);
        }
        else
        {
            /* download the packet into DCUTODATA(msg)   */

            /* set up length of packet; MUST be set to actual size   */
            /* not including crc etc even if allocated a larger      */
            /* packet above                                          */
            DCUTOPACKET(msg)->length = len_tbd;

            /* signal IP layer that a packet is on its exchange   */
            ks_invoke_input(pi_tbd, msg);   
        }
    }

    /* if xmit done interrupt   */
    if (int_type_tbd == XMIT_INT_TBD)
    {
        /* signal IP task that xmit has completed   */
        ks_invoke_output(pi_tbd, 1); 
    }
    DRIVER_MASK_ISR_ON(sc->ia_irq);
}

/* ********************************************************************   */
/* nil_xmit_done() - process a completed transmit                         */
/*                                                                        */
/* This routine is called as a result of the transmit complete            */
/* interrupt occuring (see ks_invoke_output).                             */
/*                                                                        */
/* Inputs:                                                                */
/*   pi     - interface structure                                         */
/*   DCU    - packet transmitting                                         */
/*   status - TRUE indicates the xmit completed successfully, FALSE       */
/*            indicates it did not (possible errors include               */
/*            timeout etc)                                                */
/*                                                                        */
/* Returns: TRUE if xmit done or error                                    */
/*          FALSE if xmit not done; if it is not done when the            */
/*                next xmit interrupt occurs, nil_xmit_done will          */
/*                be called again                                         */
/*                                                                        */

RTIP_BOOLEAN nil_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
RTIP_BOOLEAN done_tbd;
int     errno_tbd;

    ARGSUSED_INT(success);
    ARGSUSED_PVOID(msg);

    if (done_tbd)
        return(TRUE);

    pi->xmit_status = errno_tbd;
    return(FALSE);
}

/* ********************************************************************   */
/* nil_xmit() - transmit a packet                                         */
/*                                                                        */
/* Starts transmitting packet msg.  nil_xmit_done will be called          */
/* when transmit complete interrupt occurs.                               */
/*                                                                        */
/* Inputs:                                                                */
/*   pi  - interface structure of the device to open                      */
/*   msg - packet to transmit where                                       */
/*         DCUTOPACKET(msg)->length - length of packet                    */
/*         DCUTODATA(msg)           - packet                              */
/*                                                                        */
/* Returns: REPORT_XMIT_DONE if xmit is done                              */
/*          0 if the transmit is started but not done                     */
/*          errno if an error occured                                     */
/*                                                                        */

int nil_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
    ARGSUSED_PVOID(pi);
    ARGSUSED_PVOID(msg);
    return(0);
}

/* ********************************************************************   */
RTIP_BOOLEAN nil_setmcast(PIFACE pi)
{
    return(TRUE);
}

/* ********************************************************************   */
/* nil_statistics() - update statistics                                   */
/*                                                                        */
/* Update statistics in interface structure which are kept in             */
/* a structure specific to this device driver                             */
/*                                                                        */
/* Returns: TRUE if successful FALSE if error                             */
/*                                                                        */

RTIP_BOOLEAN nil_statistics(PIFACE pi)                       /*__fn__*/
{
   UPDATE_SET_INFO(pi,interface_packets_in, 0)
   UPDATE_SET_INFO(pi,interface_packets_out, 0)
   UPDATE_SET_INFO(pi,interface_bytes_in, 0)
   UPDATE_SET_INFO(pi,interface_bytes_out, 0)
   UPDATE_SET_INFO(pi,interface_errors_in, 0)
   UPDATE_SET_INFO(pi,interface_errors_out, 0)
   UPDATE_SET_INFO(pi,interface_packets_lost, 0)
   return(TRUE);
}

int xn_bind_nil(int minor_number)
{
    return(xn_device_table_add(nil_device.device_id,
                        minor_number,
                        nil_device.iface_type,
                        nil_device.device_name,
                        SNMP_DEVICE_INFO(nil_device.media_mib,
                                         nil_device.speed)
                        (DEV_OPEN)nil_device.open,
                        (DEV_CLOSE)nil_device.close,
                        (DEV_XMIT)nil_device.xmit,
                        (DEV_XMIT_DONE)nil_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)nil_device.proc_interrupts,
                        (DEV_STATS)nil_device.statistics,
                        (DEV_SETMCAST)nil_device.setmcast));
}
#endif      /* DECLARING_DATA */
#endif      /* INCLUDE_NIL */

