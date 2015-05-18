/*                                                                      */
/* LOOP.C - LOOP BACK device driver interface                           */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#undef DIAG_SECTION_KERNEL 
#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_LOOP 0

#if (INCLUDE_LOOP)
/* ********************************************************************   */
#if (DECLARING_DATA || BUILD_NEW_BINARY)

RTIP_BOOLEAN loop_open(PIFACE pi);
void    loop_close(PIFACE pi);
int     loop_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN loop_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN loop_statistics(PIFACE  pi);

/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
PIFACE        KS_FAR loop_pi;
unsigned long KS_FAR loop_packets_in;
unsigned long KS_FAR loop_packets_out;
unsigned long KS_FAR loop_bytes_in;
unsigned long KS_FAR loop_bytes_out;
unsigned long KS_FAR loop_errors_in;
unsigned long KS_FAR loop_errors_out;

/* used to register LOOPBACK init fnc   */
INIT_FNCS KS_FAR loop_fnc;  
    
EDEVTABLE KS_FAR loop_device = 
{
     loop_open, loop_close, loop_xmit, NULLP_FUNC,
     NULLP_FUNC, loop_statistics, NULLP_FUNC, 
     LOOP_DEVICE, "LOOPBACK", MINOR_0, LOOP_IFACE, 
     SNMP_DEVICE_INFO(CFG_OID_LOOP, CFG_SPEED_LOOP)
     CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS, 
     CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT, 
     IOADD(0), EN(0), EN(0)
};

#endif  /* DECLARING_DATA|| BUILD_NEW_BINARY */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
/* Pointer to the interface structure. We look at this inside the xmit routine   */
extern PIFACE        KS_FAR loop_pi;
extern unsigned long KS_FAR loop_packets_in;
extern unsigned long KS_FAR loop_packets_out;
extern unsigned long KS_FAR loop_bytes_in;
extern unsigned long KS_FAR loop_bytes_out;
extern unsigned long KS_FAR loop_errors_in;
extern unsigned long KS_FAR loop_errors_out;
extern EDEVTABLE KS_FAR loop_device;
#endif

KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR phony_en_addr[ETH_ALEN];

/* ********************************************************************    */
/* init_loopback - initialize the data structures for the loop back driver */
/*               - this function needs to be registered before calling     */
/*                 xn_rtip_init                                            */
/* ********************************************************************    */
void init_loopback(void)
{
    loop_pi = 0;
    loop_packets_in = 0L;
    loop_packets_out = 0L;
    loop_bytes_in = 0L;
    loop_bytes_out = 0L;
    loop_errors_in = 0L;
    loop_errors_out = 0L;
}

/* ********************************************************************    */
/* open the loop back driver interface.                                    */
/*                                                                         */
/* This routine opens a loop back device driver                            */
/*                                                                         */
/* The address of this function must be placed into the "devices" table in */
/* iface.c either at compile time or before a device open is called.       */
/*                                                                         */
RTIP_BOOLEAN loop_open(PIFACE pi)                            /*__fn_*/
{
    /* say interface structure so xmit knows where to find it   */
    loop_pi = pi;

    /* Now put in a dummy ethernet address        */
    tc_movebytes(pi->addr.my_hw_addr, phony_en_addr, ETH_ALEN); /* Get the ethernet address */

    /* clear statistic information   */
    loop_packets_in = 0L;
    loop_packets_out = 0L;
    loop_bytes_in =  0L;
    loop_bytes_out = 0L;
    loop_errors_in = 0L;
    loop_errors_out = 0L;

    return(TRUE);
}

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

void loop_close(PIFACE pi)                                /*__fn__*/
{
    ARGSUSED_PVOID( pi );  /* keep compiler happy */
}

/* ********************************************************************      */
/* Transmit. a packet over the packet driver interface.                      */
/*                                                                           */
/* This routine is called when a packet needs sending. The packet contains a */
/* full ethernet frame to be transmitted. The length of the packet is        */
/* provided.                                                                 */
/*                                                                           */
/* Returns 0 if successful or errno if unsuccessful                          */
/*                                                                           */
int loop_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
DCU lb_msg;
int length;
#if (DEBUG_LOOP)
    DEBUG_ERROR("loop_xmit: packet,len = ", DINT2, DCUTODATA(msg), length);
    DEBUG_ERROR("loop_xmit: PACKET = ", PKT, DCUTODATA(msg), length);
#endif

    length = DCUTOPACKET(msg)->length;

    /* allocate DCU for msg;                                                  */
    /* For non loopback case, msg will be queued and freed when               */
    /* sent out only if PKT_FLAG_KEEP is not set.                             */
    /* But for loopback, copy of msg will not go thru the output queue, but   */
    /* will go straight to IP list which will free it                         */
    /* after interpreting it.  Therefore, copy it to                          */
    /* a new allocated DCU and the new DCU will be freed after                */
    /* it is processed on the input side.  The origional DCU                  */
    /* will be freed according to dcu_flags.  The origional DCU               */
    /* is on the output list, therefore, it will signal according             */
    /* to the flags in the origional DCU, therefore, signal flag              */
    /* does not have to be set in the new DCU.                                */
    /* IN OTHER WORDS, the packet could have been allocated by the            */
    /* application who would be the only task which should free the           */
    /* packet and the IP task will always free an incoming packet when        */
    /* done, therefore, allocate a new one                                    */
    /* NOTE: the new packet has flags 0, i.e. do not keep, do not signal      */
    /* NOTE: netwrite() will call tc_release_message for the origional packet */
    /*       which will signal and free it if requested                       */
    lb_msg = os_alloc_packet(length, DRIVER_ALLOC); 

    if (!lb_msg)
    {
        DEBUG_LOG("loop_xmit - os_alloc_packet failed", LEVEL_3, NOVAR, 0, 0);
        return(ENOPKTS);
    }

    /* copy msg to lbmsg which was just allocated (set flags to no keep,   */
    /* no signal)                                                          */
    COPY_DCU_FLAGS(lb_msg, msg)

#if (DEBUG_LOOP)
    DEBUG_ERROR("loop_xmit: new pkt = ", PKT, DCUTODATA(lb_msg), length);
    DEBUG_ERROR("           addr = ", DINT1, DCUTODATA(lb_msg), 0);
    DEBUG_ERROR("           length = ", EBS_INT1, length, 0);
#endif

    /* set up phoney dest addr to make sure it is not broadcast; this   */
    /* is done so icmp will not drop the packet                         */
    SETUP_LL_HDR_DEST(pi, lb_msg, phony_en_addr)

    /* send packet to ip exchange                       */
    /* NOTE: interface number is put in DCU by ip tasks */
    OS_SNDX_IP_EXCHG(pi, lb_msg);
    DEBUG_LOG("loop_xmit - sent lb to ip ex", LEVEL_3, NOVAR, 0, 0);

    /* ******            */
    /* update statistics */
    loop_packets_out += 1;
    loop_bytes_out += length;

    loop_packets_in += 1;
    loop_bytes_in += length;

#if (1)     /* tbd */
    /* signal IP layer that send is done                              */
    /* this is exactly ks_invoke_output except ks_invoke_output needs */
    /* to work from an interrupt service routine                      */
    ks_disable();           /* Disable interrupts  */
    pi->xmit_done_counter++;
    ks_enable();            /* enable interrupts */
    OS_SET_IP_SIGNAL(pi);

    /* ******   */
    return (0);
#else
    return(REPORT_XMIT_DONE);           /* return xmit done */
#endif

}


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

RTIP_BOOLEAN loop_statistics(PIFACE pi)                       /*__fn__*/
{
#if (!INCLUDE_KEEP_STATS)
    ARGSUSED_PVOID(pi)
#endif

    UPDATE_SET_INFO(pi, interface_packets_in, loop_packets_in)
    UPDATE_SET_INFO(pi, interface_packets_out, loop_packets_out)
    UPDATE_SET_INFO(pi, interface_bytes_in, loop_bytes_in)
    UPDATE_SET_INFO(pi, interface_bytes_out, loop_bytes_out)
    UPDATE_SET_INFO(pi, interface_errors_in, loop_errors_in)
    UPDATE_SET_INFO(pi, interface_errors_out, loop_errors_out)
    UPDATE_SET_INFO(pi, interface_packets_lost, 0L)
    return(TRUE);
}

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_loop(int minor_number)
{
    return(xn_device_table_add(loop_device.device_id, 
                        minor_number, 
                        loop_device.iface_type,
                        loop_device.device_name,
                        SNMP_DEVICE_INFO(loop_device.media_mib, 
                                         loop_device.speed)                         
                        (DEV_OPEN)loop_device.open,
                        (DEV_CLOSE)loop_device.close,
                        (DEV_XMIT)loop_device.xmit,
                        (DEV_XMIT_DONE)loop_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)loop_device.proc_interrupts,
                        (DEV_STATS)loop_device.statistics,
                        (DEV_SETMCAST)loop_device.setmcast));
}
#endif      /* DECLARING_DATA */
#endif      /* INCLUDE_LOOP */

