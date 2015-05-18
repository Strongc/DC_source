/*                                                                                                                 */
/* IFACE.C - Interface functions                                                                                   */
/*                                                                                                                 */
/* EBS - RTIP                                                                                                      */
/*                                                                                                                 */
/* Copyright Peter Van Oudenaren , 1993                                                                            */
/* All rights reserved.                                                                                            */
/* This code may not be redistributed in source or linkable object form                                            */
/* without the consent of its author.                                                                              */
/*                                                                                                                 */
/* Module description:                                                                                             */
/*   This module provides a portable interface layer to the device                                                 */
/*   drivers and all the routines which access/modify the routing table.                                           */
/*   The implementor must bind the device which adds an entry to the device table.                                 */
/*   All RTIP device accesses are done through routines in                                                         */
/*   the device table. The driver is never directly called.                                                        */
/*                                                                                                                 */

/*  Functions in this module                                                   */
/*  tc_ino2_iface      - Given an interface number returns the associated      */
/*                       interface structure.                                  */
/*  tc_interface_init  - One time strtup of an interface                       */
/*  tc_interface_open  - Open an interface.                                    */
/*  tc_interface_stats - Return driver statistics for an interface             */
/*  tc_interface_close - Close an interface.                                   */
/*  tc_netwrite        - Resolve an IP address to an ethernet address and sent */
/*  tc_interface_send  - Queue a packet on output list                         */
/*  tc_process_output  - Manage the output list                                */

#define DIAG_SECTION_KERNEL DIAG_SECTION_IP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"
/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_LOOP          0
#define DEBUG_PKT_SENT      0   /* show pkt sent to tc_do_interface_write */
#define DEBUG_NETWRITE      0
#define DISPLAY_OUTPUT_PKT  0   /* if set, interface_write will call */
                                /* DEBUG_LOG to print output pkt   */
#define DEBUG_OUTPUT_LIST   0
#define MEASURE_MAX_OUT_QUE 1
#if (MEASURE_MAX_OUT_QUE)
long max_out_queue = 0;
#endif
#define DISPLAY_XMIT_TIMER_DECR 0

/* ********************************************************************   */
/* MANAGE XMIT MACROS                                                     */
/* ********************************************************************   */
#if (INCLUDE_XMIT_QUE)
#define SEND_IN_PROGRESS(PI)   ((PI)->xmit_started > 0)
#define ROOM_IN_QUE(PI)        ((PI)->xmit_started >= (PI)->xmit_que_depth)
#define TRANSMIT_COMPLETE(PI)  ((PI)->xmit_done_counter && (PI)->ctrl.list_xmit)
#define CAN_START_NEW_XMIT(PI) (ROOM_IN_QUE((PI)) && (PI)->ctrl.list_output) 
#else
#define SEND_IN_PROGRESS(PI)   ((PI)->xmit_dcu)
#define TRANSMIT_COMPLETE(PI)  ((PI)->xmit_done_counter && (PI)->xmit_dcu)
#define CAN_START_NEW_XMIT(PI) (!(PI)->xmit_dcu && (PI)->ctrl.list_output)
#endif

/* start xmit completion timer (add 1 to ensure at least   */
/* one second has passed before decrementing the           */
/* timer)                                                  */
#define START_XMIT_TIMER(pi)                                \
    pi->xmit_done_timer = (CFG_ETHER_XMIT_TIMER + 1);       \
    if (pi->pdev && (pi->pdev->iface_type == RS232_IFACE))  \
        pi->xmit_done_timer = (CFG_RS232_XMIT_TIMER + 1);

/* all packets have been output   */
#if (INCLUDE_XMIT_QUE)
#define ALL_XMIT_DONE(PI) ( ((PI)->xmit_started == 0) && !(PI)->ctrl.list_output )      
#else
#define ALL_XMIT_DONE(PI) (!(PI)->xmit_dcu && !(PI)->ctrl.list_output)      
#endif

#if (INCLUDE_XMIT_QUE)
#define XMIT_COMPLETE_NO_ERROR(STATUS) \
    ((STATUS & REPORT_XMIT_DONE) <= 0 || (STATUS) & REPORT_XMIT_DONE)
#else
#define XMIT_COMPLETE_NO_ERROR(STATUS) \
    ((STATUS) == 0 || (STATUS) == REPORT_XMIT_DONE)
#endif

/* ********************************************************************   */
/* LOCAL FUNCTIONS                                                        */
/* ********************************************************************   */
RTIP_BOOLEAN tc_process_output(PIFACE pi);
void         process_one_xmit_done(PIFACE pi, DCU msg);
void         finish_interface_close(PIFACE pi);
int          loop_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN _tc_interface_mcast(PIFACE pi, PFBYTE mclist, int nipaddrs, int add);
RTIP_BOOLEAN interface_mcast_entry(PIFACE pi, PFCBYTE mcast_ip_addr, int op);
RTIP_BOOLEAN setup_mcast(PIFACE pi);
RTIP_BOOLEAN save_mcast_entry(PIFACE pi, PFCBYTE mcast_ip_addr, 
                         PFBYTE mcast_eth_addr);
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
void igmp_send_report(PIFACE pi, PFBYTE ip_addr);
#endif
#if (INCLUDE_IGMP_V2)
void igmp_leave_group(PIFACE pi, PFBYTE mcast_ip_addr);
#endif

/* ********************************************************************   */
/* INTERFACE UTILITY ROUTINES                                             */
/* ********************************************************************   */

/* ********************************************************************   */
/* tc_ino2_iface() - return interface structure for the interface number  */
/*                                                                        */
/*   Convert an interface number to a structure.  The interface must      */
/*   be opened.                                                           */
/*   The parameter do_set_errno is set to DONT_SET_ERRNO or SET_ERRNO.    */
/*   (i.e. if the interface is closed, do not set errno value OR          */
/*         set errno to EIFACECLOSED if the interface is closed)          */
/*                                                                        */
/*   Called by lots of routines throughout the package                    */
/*                                                                        */
/*   Returns interface structure if successful or 0 if not.               */
/*   If errno is set sets errno if an error was detected.                 */
/*                                                                        */

PIFACE tc_ino2_iface(int iface_no, RTIP_BOOLEAN do_set_errno)   /*__fn__*/
{
PIFACE pi;
int    err;

    /* check for valid interface number   */
    if (iface_no < 0 || iface_no >= CFG_NIFACES)
    {
        err = EBADIFACE;
        goto iface_error_exit;
    }
    else
    {
        pi = (PIFACE) &ifaces[iface_no];
        if (pi->open_count <= 0)
        {
            err = EIFACECLOSED;
            goto iface_error_exit;
        }
    }
    return(pi);

iface_error_exit:
    if (do_set_errno)
        set_errno(err);
    return((PIFACE)0);
}

/* ********************************************************************   */
/* tc_ino2_device() - return interface structure for the device number    */
/*                                                                        */
/*   Convert an device offset to an interface structure if there is one.  */
/*   The device offset is the offset in the device table.                 */
/*   NOTE: there is one only if the device is open.                       */
/*                                                                        */
/*   Returns interface structure if successful or 0 if not.               */
/*   If errno is set sets errno if an error was detected.                 */
/*                                                                        */

PIFACE tc_ino2_device(int device_off)      /*__fn__*/
{
PIFACE pi;
int iface_no;

    /* check for valid device number   */
    if (device_off >= 0 || device_off < valid_device_entries)
    {
        LOOP_THRU_IFACES(iface_no)
        {
            PI_FROM_OFF(pi, iface_no)
/*          OS_CLAIM_IFACE(pi, DEVICE_CLAIM_IFACE)   */
            if (pi->open_count) /* if valid, i.e. open */
            {
                if ( (devices_ptr[device_off].device_id == pi->pdev->device_id) &&
                     (devices_ptr[device_off].minor_number == pi->pdev->minor_number) )
                {
/*                  OS_RELEASE_IFACE(pi)   */
                    return(pi);
                }
            }
/*          OS_RELEASE_IFACE(pi)   */
        }
    }
    return((PIFACE)0);
}


/* ********************************************************************   */
/* tc_device_id() - returns device id given the interface number          */
/*                                                                        */
/*   Convert an interface number to a device id.                          */
/*                                                                        */
/*   Returns interface type if successful or -1 if not.                   */
/*                                                                        */

int tc_device_id(int iface_no)   /*__fn__*/
{
PIFACE pi;

    PI_FROM_OFF(pi, iface_no)
    if (!pi)
        return(-1);

    return(pi->pdev->device_id);
}

/* ********************************************************************   */
/* tc_iface_type() - returns interface type given the interface number    */
/*                                                                        */
/*   Convert an interface number to a interface type.                     */
/*                                                                        */
/*   Called by lots of routines throughout the package                    */
/*                                                                        */
/*   Returns interface type if successful or -1 if not.                   */
/*                                                                        */

int tc_iface_type(int iface_no)   /*__fn__*/
{
PIFACE pi;

    PI_FROM_OFF(pi, iface_no)
    if (!pi)
        return(-1);

    return(pi->pdev->iface_type);
}

/* ********************************************************************   */
/* tc_find_device_minor() - find device table entry                       */
/*                                                                        */
/*   Given the device id and the minor number, searches and returns the   */
/*   offset of the specified device in the device table.  A minor number  */
/*   of -1 means match any minor number.  The device does not need to     */
/*   be open.                                                             */
/*                                                                        */
/*   Returns offset in device table or -1 if device not found             */

int tc_find_device_minor(int device_id, int minor_number)   /*__fn__*/
{
int device_off;

    for (device_off = 0; ; device_off++)
    {
        if (devices_ptr[device_off].open == NULLP_FUNC) /* if end of table */
            return(-1);

        if ( (devices_ptr[device_off].device_id == device_id) &&
             ((devices_ptr[device_off].minor_number == minor_number) ||
              (minor_number == -1)) )
            break;
    }
    return(device_off);
}

/* get interface number for local interface with IP address ip_addr   */
int get_local_ifaceno(PFBYTE ip_addr)
{
int    iface_no;
PIFACE pi;

    LOOP_THRU_IFACES(iface_no)
    {
        pi = tc_ino2_iface(iface_no, DONT_SET_ERRNO);
        if (pi && (pi->addr.iface_flags & IP_ADDR_VALID) &&
            (pi->pdev->iface_type != LOOP_IFACE))
        {
            if (tc_cmp4(pi->addr.my_ip_addr, ip_addr, IP_ALEN))
                return(iface_no);
        }
    }
    return(-1);
}


#if (INCLUDE_SNMP)
/* ********************************************************************   */
/* tc_find_iface_minor() - find device table entry                        */
/*                                                                        */
/*   Given the device id and the minor number, searches and returns the   */
/*   offset of the specified device in the device table.                  */
/*                                                                        */
/*   Returns offset in device table or -1 if device not found             */

int tc_find_iface_minor(int iface_type, int minor_number)   /*__fn__*/
{
int device_off;

    for (device_off = 0; ; device_off++)
    {
        if (devices_ptr[device_off].open == NULLP_FUNC) /* if end of table */
            return(-1);

        if ( (devices_ptr[device_off].iface_type == iface_type) &&
             (devices_ptr[device_off].minor_number == minor_number) )
            break;
    }
    return(device_off);
}
#endif

/* ********************************************************************   */
/* tc_interface_info() - returns interface information                    */
/*                                                                        */
/*   Returns information about an opened interface                        */
/*                                                                        */
/*   The macro xn_iface_info() is defined to be tc_iface_info()           */
/*                                                                        */
/*   Returns 0 if successful or -1 if not.                                */
/*   If errno is set sets errno if an error was detected.                 */
/*                                                                        */

int tc_interface_info(int iface_no, PIFACE_INFO pi_info)   /*__fn__*/
{
PIFACE pi;

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
        return(-1);

    pi_info->device_id = tc_device_id(iface_no);
    tc_mv4(pi_info->my_ip_address, pi->addr.my_ip_addr, IP_ALEN);
    tc_movebytes(pi_info->my_ethernet_address, pi->addr.my_hw_addr, ETH_ALEN);
#if (INCLUDE_BOOTP)
    tc_mv4(pi_info->bootp_ip_address, pi->addr.bootp_ip_addr, IP_ALEN);
    tc_movebytes(pi_info->bootp_file_name, pi->addr.bootp_file_name,
        BOOTP_FILE_LEN);
#endif
    tc_mv4(pi_info->ip_mask, pi->addr.my_ip_mask, IP_ALEN);
    tc_mv4(pi_info->his_ip_address, pi->addr.his_ip_addr, IP_ALEN);
    pi_info->mtu = (int)(pi->addr.mtu);
    pi_info->remote_mtu = (int)(pi->addr.remote_mtu);

    /* PCI discovers during open   */
    pi_info->irq    = pi->irq_val;
    pi_info->ioaddr = pi->io_address;
    return(0);
}

/* ********************************************************************   */
/* ROUTER ROUTINES                                                        */
/* ********************************************************************   */

#if (INCLUDE_ROUTER)
#if (!INCLUDE_ROUTING_TABLE)
error: INCLUDE_ROUTING_TABLE must be set if INCLUDE_ROUTER is set
#endif

/* ********************************************************************   */
/* tc_get_route() - return interface for router at IP addr                */
/*                                                                        */
/*   Returns interface structure associated with a gateway ip address.    */
/*                                                                        */
PIFACE tc_get_route(PFBYTE ip_addr)     /*__fn__*/
{
int    i;
PROUTE prt;
PIFACE pi = (PIFACE)0;

    OS_CLAIM_TABLE(RT_GETR_TABLE_CLAIM)

    for (i=0; i < CFG_RTSIZE; i++)
    {
        prt = (PROUTE)(&(rt_table[i]));

        /* if entry is not valid, continue   */
        if ( RT_FREE(prt) || (prt->rt_flags & RT_DEL) )
        {
            continue;       
        }

        /* see if the current entry is a match and a gateway   */
        if ( tc_cmp4(prt->rt_gw, ip_addr, IP_ALEN) &&   
             (prt->rt_flags | RT_GW) )
        {
#            if (DEBUG_RT)
                DEBUG_LOG("rt_get - found non-gw interface,metric = ", LEVEL_3, EBS_INT2, 
                    prt->rt_iface, prt->rt_metric);                 
#            endif
            pi = tc_ino2_iface(prt->rt_iface, DONT_SET_ERRNO);
            if (pi)
                break;
        }
    }

    OS_RELEASE_TABLE()
    return(pi);
}
#endif      /* INCLUDE_ROUTER */

/* ********************************************************************   */
/* tc_get_nxt_bc_iface() - return first interface for broadcast           */
/*                                                                        */
/*   Returns first interface to send broadcast package to.  The first     */
/*   possible match is *iface. It then updates *iface to the next iface   */
/*   number.  An interface is a valid broadcast interface if it is open,  */
/*   it has its ip and mask set and it is not a loopback interface.       */
/*                                                                        */
PIFACE tc_get_nxt_bc_iface(int iface)   /*__fn__*/
{
PIFACE pi;
int    iface_off;

    for (iface_off = iface; iface_off < CFG_NIFACES; iface_off++)
    {
        /* get interface, don't care if open since valid flags will only   */
        /* be set if open and don't want tc_ino2_iface to report error     */
        PI_FROM_OFF(pi, iface_off)

        /* xn_set_ip must have been called prior to this routine   */
        if (pi->addr.iface_flags != IP_ADDR_VALID)
            pi = (PIFACE)0;

#if (INCLUDE_PPP)
        else if (pi->pdev->device_id == PPP_DEVICE)
        {
            /* state must be opened   */
            if (CHECK_PPP_NOT_OPEN(pi))
                pi = (PIFACE)0;
        }
#endif

        /* if iface is valid broadcast    */
        else if (pi->pdev->device_id == LOOP_DEVICE)
        {
            pi = (PIFACE)0;
        }

        if (pi)
            return(pi);
    }

    return((PIFACE)0);
}

/* ********************************************************************               */
/* tc_get_src_ip() - get local IP address for interface associated with an IP address */
/*                                                                                    */
/*   Get local IP address for interface associated with an dest_ip_addr and           */
/*   store it in src_ip_addr                                                          */
/*                                                                                    */
/*   Returns interface structure (PIFACE) if an interface is found or 0               */
/*   if one is not found                                                              */
/*                                                                                    */

PIFACE tc_get_src_ip(PFBYTE src_ip_addr, PFBYTE dest_ip_addr)  /*__fn__*/
{
PIFACE pi;
byte   temp_ip_addr[IP_ALEN];

    /* get interface will send to   */
    pi = rt_get_iface(dest_ip_addr, (PFBYTE)temp_ip_addr, (PANYPORT)0, (RTIP_BOOLEAN *)0);
    if (pi)
    {  
        /* use my ip address for that interface will send to   */
        tc_mv4(src_ip_addr, (pi->addr.my_ip_addr), IP_ALEN);
                                                 /* fixed value                  */
                                                 /* NOTE: ip_dest set from input */
                                                 /*       msg during listen or   */
                                                 /*       by connect             */
    }
    return(pi);     /* 0 if failure */
}

/* ********************************************************************   */
/* INTERFACE API ROUTINES                                                 */
/* ********************************************************************   */

/* ********************************************************************    */
/* tc_interface_open() - open an interface                                 */
/*                                                                         */
/*   Returns offset into interface table (>=0) if successful, -1 if not    */
/*   NOTE: CSLIP, SLIP and PPP all use RS232, therefore, minor_number must */
/*         be offset in RS232 structure                                    */
/*   Sets errno if an error was detected                                   */
/*                                                                         */

int tc_interface_open(int device_id, int minor_number,  /*__fn__*/
                      IOADDRESS io_address,             /*__fn__*/
                      int  irq_val,                     /*__fn__*/
                      word mem_address)                 /*__fn__*/
{
PIFACE pi, pi_test;
int    ret_val;
int    device_off;      /* offset in device table */
int    iface_no;        /* ofset in interface table */
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
int    i;
#endif

    DEBUG_LOG("tc_interface_open - device_id, minor_number = ", LEVEL_3, EBS_INT2, 
        device_id, minor_number);

    /* **************************************************   */
    /* find a matching entry in interface table             */
    pi = (PIFACE)0;
    LOOP_THRU_IFACES(iface_no)
    {
        PI_FROM_OFF(pi_test, iface_no)
        if (pi_test)
        {
            OS_CLAIM_IFACE(pi_test, OPEN1_CLAIM_IFACE)
            /* check if matches, NOTE: if open_count, then pdev is set up   */
            if (pi_test->open_count &&
                (pi_test->pdev->device_id == device_id) && 
                (pi_test->minor_number == minor_number) )
            {
                pi = pi_test;           
                BREAK_IF_LOOP   /* break with interface still claimed */
            }
            else
            {
                OS_RELEASE_IFACE(pi_test)
            }
        }
    }

    /* **************************************************         */
    /* if no matching entry in interface table, find a free entry */
    if (!pi)
    {
        LOOP_THRU_IFACES(iface_no)
        {
            PI_FROM_OFF(pi_test, iface_no)
            if (pi_test)
            {
                OS_CLAIM_IFACE(pi_test, OPEN1_CLAIM_IFACE)
                if (pi_test->open_count == 0)
                {
                    pi = pi_test;
                    BREAK_IF_LOOP
                }
                else
                {
                    OS_RELEASE_IFACE(pi_test)
                }
            }
        }
    }

    if (!pi)
    {
        DEBUG_ERROR("tc_interface_open() - no interface struct; iface_no = ", EBS_INT1, 
            iface_no, 0);
        return(set_errno(EIFACEFULL));
    }

    /* **************************************************             */
    /* at this point, an entry was found and the interface is claimed */
    /* find device_id in device table                                 */
    device_off = tc_find_device_minor(device_id, minor_number);
    if (device_off < 0)
    {
        OS_RELEASE_IFACE(pi_test);    
        return(set_errno(EBADDEVICE));
    }

    /* **************************************************   */
    /* Set up the addresses of the io routines              */
    pi->pdev = (PDEVTABLE) &devices_ptr[device_off];  /* IO routines, types etc */

/* DEBUG_ERROR("OPEN DEVICE: ", STR1, pi->pdev->device_name, 0);   */


    /* **************************************************   */
    if (pi->open_count && pi->pdev && 
        (pi->pdev->iface_type != RS232_IFACE))  /* already done by xn_attach */
    {
        pi->open_count++;
        ret_val = pi->ctrl.index;        
        goto done;
    }

    /* **************************************************                      */
    /* set values based on parameters to xn_interface_open_config if they are  */
    /* not set to the illegal values (0, -1, 0) otherwise                      */
    /* set them to default values from the device table                        */
    if (pi->pdev->iface_type == ETHER_IFACE)
    {
        if (io_address == 0)
            pi->io_address = (IOADDRESS)(pi->pdev->default1.io_address);
        else
            pi->io_address = io_address;

        if (irq_val == -1)
            pi->irq_val = (word)pi->pdev->default3.irq;
        else
            pi->irq_val = irq_val;
        
#if (INCLUDE_SMC8XXX || INCLUDE_3C503 || INCLUDE_CS89X0)
        if (mem_address == 0)
            pi->mem_address = pi->pdev->default2.mem_address;
        else
            pi->mem_address = mem_address;
#else
        ARGSUSED_INT(mem_address)
#endif
    }

    /* **************************************************   */
    pi->max_output_que = -1;    /* number of packets which can be queued
                                   on output list; no limit (default) */
    pi->xmit_que_depth = 1;     /* depth of driver output que; can be */
                                /* set higher by device open   */
#if (INCLUDE_XMIT_QUE)
    pi->xmit_started = 0;       /* xmits in progress */
#endif
    pi->no_output_que  = 0;     /* nothing queued yet */

    /* for the interface, set the offset of this interface from the   */
    /* first entry in the table of the same type; used for access     */
    /* to data structures for devices which have multiple entries     */
    /* in the device table                                            */
    pi->minor_number = minor_number;

    /* **************************************************           */
    /* set the addr structure to all 0's, i.e.                      */
    /*    pi->addr->dhcp_status = DHCP_EXTEND_IDLE;                 */
    /*    pi->addr->orig_lease_time = 0;                            */
    /*   Set the local interface ip address to all 0's. for now.    */
    /*   This must be change by set ip address or bootp             */
    /*    tc_mv4(pi->addr.my_ip_addr, ip_nulladdr, IP_ALEN);        */
    /*   Set the remote interface ip address to all 0's. for now.   */
    /*   This must be change by attach if dedicated line            */
    /*    tc_mv4(pi->addr.his_ip_addr, ip_nulladdr, IP_ALEN);       */
    /*    pi->addr.iface_flags = 0;                                 */
    /*    pi->addr.remote_mtu = 0;                                  */
    tc_memset((PFBYTE)&pi->addr, 0, sizeof(struct _iface_ip));

    /* Set up a default class 'C' mask   */
    pi->addr.my_ip_mask[0] = 0xff;
    pi->addr.my_ip_mask[1] = 0xff;
    pi->addr.my_ip_mask[2] = 0xff;
    pi->addr.my_ip_mask[3] = 0x00;

    /* setup MTU based upon device table default;                            */
    /* xn_pkt_data_max will use this value etc as well as                    */
    /* TCP will instruct the other side not to send packets larger than this */
    /* (i.e. mss in sync message)                                            */
    pi->addr.mtu = devices_ptr[device_off].mtu; 
    pi->addr.max_mss_out = devices_ptr[device_off].max_mss_out; 

    /* **************************************************   */
    /* clear multicast info                                 */
    pi->mcast.lenmclist = 0;
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
    for (i=0; i < CFG_MCLISTSIZE; i++)
    {
        pi->mcast.report_timer[i] = 0;
#if (INCLUDE_IGMP_V2)
        pi->mcast.last_host_toreply[i] = FALSE;
#endif
    }
#endif

#if (INCLUDE_IGMP_V2)
    pi->mcast.mcast_querier = IGMP_V2; /* assume querier running version 2 at start */
#endif

    /* **************************************************        */
    /* reset values for tracking output packets done by IP layer */
    pi->xmit_done_counter = 0;
    pi->xmit_done_timer = 0;
    pi->ctrl.list_output = 0;
#if (INCLUDE_XMIT_QUE)
    pi->ctrl.list_xmit = 0;
#else
    pi->xmit_dcu = (DCU)0;
#endif

    /* **************************************************                   */
    /* Now for the board level open. And get the ethernet address into the  */
    /* interface structure                                                  */
    if (!(*(pi->pdev->open))(pi))
    {
        DEBUG_ERROR("tc_interface_open - device open failed, errno = ", 
            EBS_INT1, xn_getlasterror(), 0);
        pi->pdev = (PDEVTABLE)0;
        ret_val =  -1;
    }
    else
    {
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
        if (CFG_MCLISTSIZE > 0)
        {
            /* IGMP hosts must join the ALL HOSTS GROUP (224.0.0.1)   */
            if (!interface_mcast_entry(pi, ip_igmp_all_hosts, ADD_ENTRY))
            {
                OS_RELEASE_IFACE(pi_test)
                pi->pdev = (PDEVTABLE)0;
                return(-1);
            }
        }
#endif

        pi->open_count++;

#if (INCLUDE_SNMP)
        /* keep snmp statistics   */
        pi->lastchange = calc_sys_up_time();
#        if (INCLUDE_SNMP)
            SEND_SNMP_TRAP(TRAP_LINK_UP, 0, (POID)0, (PAPI_REQ_DESC)0);
#        endif
#endif
        ret_val = pi->ctrl.index;        
    }

    /* **************************************************   */
#if (INCLUDE_PCMCIA)
    /* NOTE: check after device open since for PCI cards, irq number is   */
    /*       discovered by the open routine                               */
    if (pi->irq_val == MGMT_INTERRUPT)
    {
        DEBUG_ERROR("tc_interface_open - possible irq conflict with PCMCIA INTERRUPT (see MGMT_INTERRUPT): ",
            EBS_INT1, irq_val, 0);
    }
#endif

/* **************************************************   */
done:

   OS_RELEASE_IFACE(pi_test)
   return(ret_val);
}



/* ********************************************************************    */
/* This routine calls the driver statistics function. See the demo program */
/* for an example of its use.                                              */
/*                                                                         */

RTIP_BOOLEAN tc_interface_stats(int iface_no)                      /*__fn__*/
{
PIFACE pi;
RTIP_BOOLEAN ret_val;

    pi = (PIFACE) tc_ino2_iface(iface_no, DONT_SET_ERRNO);

    OS_CLAIM_IFACE(pi, STATS_CLAIM_IFACE)
    ret_val = FALSE;
    if (pi && pi->pdev->statistics)
    {
        ret_val = pi->pdev->statistics(pi);
    }
    OS_RELEASE_IFACE(pi)
    return(ret_val);
}

/* ********************************************************************   */
/* tc_interface_close() - closes an interface                             */
/*                                                                        */
/*   Close down an interface. Releases all resources.                     */
/*                                                                        */
/*   Returns 0 if successful, -1 if not                                   */
/*   Sets errno if an error was detected                                  */
/*                                                                        */

int tc_interface_close(int iface_no)                      /*__fn__*/
{
PIFACE pi;
int    ret_val;

    DEBUG_LOG("tc_interface_close() - start", LEVEL_3, NOVAR, 0, 0);

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    PI_FROM_OFF(pi, iface_no)
    if (!pi)
    {
        set_errno(EBADIFACE);
        return(-1);
    }

    /* if device is not open, return   */
    if (!pi->open_count)
        return(0);

    /* Signal IP layer via sending it a DCU with flag set in dcu_flags   */
    /* to perform an event.  In this case, the event is to perform       */
    /* the interface close.  This will cause the routine                 */
    /* finish_interface_close() to be called by the IP layer.            */
    ret_val = signal_ip_via_dcu(pi, CLOSE_IFACE);
    if (ret_val)
        return(set_errno(ret_val));

    DEBUG_LOG("tc_interface_close() - end", LEVEL_3, NOVAR, 0, 0);
    return(0);
}

/* ********************************************************************   */
/* signal_ip_via_dcu - Signal IP layer to perform event                   */
/*                                                                        */
/*   Signal IP layer via sending it a DCU with flag set in dcu_flags      */
/*   to perform an event.                                                 */
/*   Possible values for flag are:                                        */
/*      CLOSE_IFACE - finish closing an interface)                        */
/*      MODEM_DROP  - interrupt indicated modem drop                      */
/*                                                                        */
/*   Returns 0 if successful, errno if not successful                     */
/*   Sets errno if an error was detected                                  */
/*                                                                        */
int signal_ip_via_dcu(PIFACE pi, int flag)                      /*__fn__*/
{
DCU    msg;

    DEBUG_LOG("signal_ip_via_dcu() - flag - ", LEVEL_3, EBS_INT1, flag, 0);

    /* just set flag so close; will be done by IP layer              */
    /* NOTE: need to do this so do not close interface while packets */
    /*       are on the output list                                  */
    /* NOTE: msg will be freed by the IP layer                       */
    msg = os_alloc_packet(0, CLOSE_IFACE_ALLOC);
    if (!msg)
        return(ENOPKTS);

    /* tell IP layer to close the interface   */
    DCUTOCONTROL(msg).dcu_flags = flag;

    /* claim interface structure                                            */
    /* NOTE: does not claim udp or tcp semaphores because this is a drastic */
    /*       close and another task might have the semaphore claimed        */
    OS_CLAIM_IFACE(pi, CLOSE_CLAIM_IFACE)

    if (flag == CLOSE_IFACE)
    {
        /* set flag so xn_rtip_exit knows to wait until close done   */
        pi->iface_flags |= CLOSE_IN_PROGRESS;   
    }

#if (MEASURE_MAX_OUT_QUE)
    if (pi->no_output_que > max_out_queue)
        max_out_queue = pi->no_output_que;
#endif

    if (flag == CLOSE_IFACE)
    {
        /* add to end of output list (don't check limit since don't want   */
        /* to loose the close event)                                       */
        pi->no_output_que++;

#if (DEBUG_OUTPUT_LIST)
        DEBUG_ERROR("SIGNAL_IP_VIA_DCU: ADD TO OUTPUT QUE", 
            EBS_INT1, pi->no_output_que, 0);
#endif
        pi->ctrl.list_output = 
            os_list_add_rear_off(pi->ctrl.list_output, (POS_LIST)msg, 
                                 OUTPUT_LIST_OFFSET);
        if (!SEND_IN_PROGRESS(pi))
            ks_invoke_output(pi, 1);
    }
    else    /* MODEM DROP */
    {
        /* Send the message directly to the IP exchange   */
        /* NOTE: called from ISR                          */
        OS_SNDX_IP_EXCHG(pi, msg);
    }

    OS_RELEASE_IFACE(pi)

    DEBUG_LOG("signal_ip_via_dcu() - end", LEVEL_3, NOVAR, 0, 0);
    return(0);
}

/* ********************************************************************   */
/* finish_interface_close() - closes an interface                         */
/*                                                                        */
/*   Close down an interface. Releases all resources.                     */
/*   Called when interface closed and output queue empties.               */
/*   ASSUMPTION: called with interface semaphore claimed                  */
/*                                                                        */
/*   Returns 0 if successful, -1 if not                                   */
/*   Sets errno if an error was detected                                  */
/*                                                                        */

void finish_interface_close(PIFACE pi)
{
DCU msg;
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */

DEBUG_ERROR("finish_interface_close called", NOVAR, 0, 0);

    pi->open_count--;

    /* Release buffers if no longer open by anyone   */
    if (pi->open_count == 0)
    {
#if (INCLUDE_SNMP)
        /* keep SNMP statistics   */
        pi->lastchange = calc_sys_up_time();
#endif

        /* delete all entries in routing table for this interface   */
        rt_del_iface(pi->ctrl.index);

        /* Close down the device driver   */
        if (pi->pdev->close)
            pi->pdev->close(pi);
        pi->pdev = (PDEVTABLE)0;

        /* free all DCUs on the IP exchange   */
        OS_CLEAR_IP_EXCHG(pi);

        /* free all DCUs on the output list   */
        KS_TL_SPLX(sp)
        while (pi->ctrl.list_output)
        {
            msg = (DCU)(pi->ctrl.list_output);
            pi->ctrl.list_output = 
                os_list_remove_off(pi->ctrl.list_output, (POS_LIST)msg,
                                   OUTPUT_LIST_OFFSET);
            KS_TL_SPL(sp)
            os_free_packet(msg);
            KS_TL_SPLX(sp)
        }

#if (INCLUDE_XMIT_QUE)
        /* free all DCUs on the xmit list   */
        KS_TL_SPLX(sp)
        while (pi->ctrl.list_xmit)
        {
            msg = (DCU)(pi->ctrl.list_xmit);
            pi->ctrl.list_xmit = 
                os_list_remove_off(pi->ctrl.list_xmit, (POS_LIST)msg,
                                   XMIT_LIST_OFFSET);
            KS_TL_SPL(sp)
            os_free_packet(msg);
            KS_TL_SPLX(sp)
        }
#endif

        /* free all DCUs on the input list           */
        /* NOTE: loop above left interrupts disabled */
        while (pi->ctrl.list_input)
        {
            msg = (DCU)(pi->ctrl.list_input);
            pi->ctrl.list_input = 
                os_list_remove_off(pi->ctrl.list_input, (POS_LIST)msg,
                                   PACKET_OFFSET);
            KS_TL_SPL(sp)
            os_free_packet(msg);
            KS_TL_SPLX(sp)
        }
        KS_TL_SPL(sp)
    }
}

/* wait for output list to empty on all interfaces or timeout   */
RTIP_BOOLEAN wait_pkts_output(RTIP_BOOLEAN disable_output, word timeout)
{
long    tme;
int     iface_no;
PIFACE  pi;
RTIP_BOOLEAN ret_val;

    tme = (long)timeout;
    ret_val = TRUE;

    for (iface_no = 0; iface_no < CFG_NIFACES; iface_no++)
    {
        PI_FROM_OFF(pi, iface_no)
        if (!pi->open_count)    /* if valid, i.e. open */
            continue;

        /* disable any further queing of output packets   */
        if (disable_output)
        {
            DEBUG_ERROR("wait_pkts_output: disable que", NOVAR, 0, 0);
            pi->max_output_que = -2;
        }

        /* wait until all DCUs on the output list are sent or timeout   */
        while (!ALL_XMIT_DONE(pi) && (tme > 0))
        {
            ks_sleep(2);
            tme -= 2;
        }

        if (pi->ctrl.list_output)
            ret_val = FALSE;
    }
    return(ret_val);
}

/* ********************************************************************   */
/* INTERFACE MULTICAST                                                    */
/* ********************************************************************   */

/* ********************************************************************   */
/* tc_interface_mcast() - set-up/add/delete a multicast list              */
/*                                                                        */
/*   This function sets up, adds to, deletes from or clears the           */
/*   multicast list for an interface.  It also sets up the device         */
/*   driver for that interface to accept packets sent to any              */
/*   of the multicast addresses in the list.                              */
/*                                                                        */
/*   Returns TRUE if successful, FALSE if an error is detected            */
/*                                                                        */

RTIP_BOOLEAN tc_interface_mcast(int iface_no, PFBYTE mclist, int nipaddrs, int add) /*__fn__*/
{
PIFACE pi;
RTIP_BOOLEAN ret_val;

    /* if default interface specifed and the default interface was setup   */
    /* by xn_interface_opt, then use the default                           */
    if ( (iface_no == -1) && (default_mcast_iface >= 0) )
        iface_no = default_mcast_iface;

    pi = (PIFACE) tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
        return(FALSE);
    OS_CLAIM_IFACE(pi, MCAST1_CLAIM_IFACE)

    ret_val = _tc_interface_mcast(pi, mclist, nipaddrs, add);

    OS_RELEASE_IFACE(pi)
    return(ret_val);
}

/* ********************************************************************   */
/* _tc_interface_mcast() - set-up/add/delete a multicast list             */
/*                                                                        */
/*   This function sets up, adds to, deletes from or clears the           */
/*   multicast list for an interface.  It also sets up the device         */
/*   driver for that interface to accept packets sent to any              */
/*   of the multicast addresses in the list.                              */
/*                                                                        */
/*   Returns TRUE if successful, FALSE if an error is detected            */
/*                                                                        */

RTIP_BOOLEAN _tc_interface_mcast(PIFACE pi, PFBYTE mclist, int nipaddrs, int add) /*__fn__*/
{
PFBYTE pmcaddr;
PFBYTE pipaddr;
int i;

    /* save multicast info in interface structure   */
    switch (add)
    {
    case SET_ENTRY:
        if (nipaddrs > CFG_MCLISTSIZE)      /* Make sure we can fit it */
        {
            set_errno(EMCASTFULL);
            return(FALSE);
        }

        pmcaddr = (PFBYTE) &pi->mcast.mclist[0];
        pipaddr = mclist;
        pi->mcast.lenmclist = 0;
        for (i = 0; i < nipaddrs ; i++)
        {
            ETHER_MAP_IP_MULTICAST(pipaddr, pmcaddr);
            if (!save_mcast_entry(pi, pipaddr, pmcaddr))
            {
                pi->mcast.lenmclist = 0;
                set_errno(EFAULT);
                return(FALSE);
            }
            pmcaddr += 6;
            pipaddr += IP_ALEN;
        }
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
        /* IGMP hosts must join the ALL HOSTS GROUP (224.0.0.1)   */
        if ( (pi->mcast.lenmclist+1) > CFG_MCLISTSIZE )
        {
            pi->mcast.lenmclist = 0;
            set_errno(EMCASTFULL);
            return(FALSE);
        }
        if (!interface_mcast_entry(pi, ip_igmp_all_hosts, ADD_ENTRY))
        {
            pi->mcast.lenmclist = 0;
            return(FALSE);
        }
#endif
        break;

    case ADD_ENTRY:
    case DELETE_ENTRY:
        pipaddr = mclist;
        for (i = 0; i < nipaddrs ; i++)
        {
            /* perform specified operation on the address;    */
            if (!interface_mcast_entry(pi, pipaddr, add))
                return(FALSE);
            pipaddr += IP_ALEN;
        }
        break;          

    case CLEAR_ENTRY:
        pi->mcast.lenmclist = 0;
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
        if (!interface_mcast_entry(pi, ip_igmp_all_hosts, ADD_ENTRY))
        {
            pi->mcast.lenmclist = 0;
            return(FALSE);
        }
#endif
        break;

    default:
        DEBUG_ERROR("_tc_interface_mcast: illegal parameter add = ",
            EBS_INT1, add, 0);
    }

    /* set up the device driver to accepts multicast addresses in the list   */
    /* NOTE: always setting up driver even if list has not changed           */
    return(setup_mcast(pi));
}

/* ********************************************************************   */
/* interface_mcast_entry() - add/delete a multicast list                  */
/*                                                                        */
/*   This function adds to or deletes entries from the                    */
/*   multicast list for an interface.                                     */
/*                                                                        */
/*   Returns TRUE if successful, FALSE if an error is detected            */
/*                                                                        */

RTIP_BOOLEAN interface_mcast_entry(PIFACE pi, PFCBYTE mcast_ip_addr, int op)
{
int i, j;
byte mcast_eth_addr[ETH_ALEN];

    ETHER_MAP_IP_MULTICAST(mcast_ip_addr, mcast_eth_addr);

    /* find the multicast IP address   */
    for (i=0; i<pi->mcast.lenmclist; i++)
    {
        if (tc_comparen(&(pi->mcast.mclist_ip[i*IP_ALEN]), mcast_ip_addr, 
                        IP_ALEN))
        {
            /* found the multicast address in the table; if requested to add,    */
            /* it is an error                                                    */
            if (op == ADD_ENTRY)
            {
                pi->mcast.mcast_cnt[i]++;
                return(TRUE);
            }

            /* delete: pointing to entry to delete; remove the current    */
            /*         entry by ripple copying the rest of the            */
            /*         table up 1 entry                                   */
            else
            {
                /* if another user is using the multicast address   */
                /* don't delete it from the table                   */
                pi->mcast.mcast_cnt[i]--;
                if (pi->mcast.mcast_cnt[i] > 0)
                    return(TRUE);
                
#if (INCLUDE_IGMP_V2)
                if (pi->mcast.mcast_querier == IGMP_V2 &&
                    pi->mcast.last_host_toreply[i])       /*if last host to reply to a query for */
                {
                    OS_RELEASE_IFACE(pi)
                    if (pi->mcast.report_timer[i])      /* don't want a report to be sent after */
                        pi->mcast.report_timer[i] = 0;  /* leaving the group */
                    igmp_leave_group(pi, (PFBYTE)mcast_ip_addr); /*this group, let querier know                          */
                    OS_CLAIM_IFACE(pi, MCAST4_CLAIM_IFACE)
                }
#endif
                for (j=i; j<(pi->mcast.lenmclist-1); j++)
                {
                    tc_movebytes(&(pi->mcast.mclist[j*ETH_ALEN]), 
                                 &(pi->mcast.mclist[(j+1)*ETH_ALEN]), 
                                 ETH_ALEN);
                    tc_mv4(&(pi->mcast.mclist_ip[j*IP_ALEN]), 
                           &(pi->mcast.mclist_ip[(j+1)*IP_ALEN]), IP_ALEN);
                    pi->mcast.mcast_cnt[j] = pi->mcast.mcast_cnt[j+1];
#                    if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
                        pi->mcast.report_timer[j] = 
                            pi->mcast.report_timer[j+1];
#                    endif
                }
                pi->mcast.lenmclist--;
                return(TRUE);       /* success */
            }
        }           /* end of if found entry in table */
    }

    /* NOT FOUND IN TABLE                                             */
    /* add: if here then IP address was not in the list; if is within */
    /*      the table it will be the first free entry                 */
    if (op == ADD_ENTRY)
    {
        /* make entry will fit   */
        if ((pi->mcast.lenmclist+1) > CFG_MCLISTSIZE)
        {
            set_errno(EMCASTFULL);
            return(FALSE);
        }

        if (!save_mcast_entry(pi, mcast_ip_addr, mcast_eth_addr))
        {
            set_errno(EFAULT);
            return(FALSE);
        }
        return(TRUE);       /* success */
    }

    /* delete: entry not found, return error   */
    set_errno(EMCASTNOTFOUND);
    return(FALSE);
}

/* ********************************************************************   */
/* setup_mcast - set up multicast in device driver                        */
/*                                                                        */
/*    Call the device driver routine (pointed to in device table)         */
/*    to set up driver for multicast.  The multicast information          */
/*    is stored in the device table.                                      */
/*                                                                        */
/*    The interface semaphore is claimed when this routine is called.     */
/*                                                                        */
/*    Returns TRUE if successful, FALSE if failure                        */
/*                                                                        */

RTIP_BOOLEAN setup_mcast(PIFACE pi)
{
RTIP_BOOLEAN ret_val;

    /* set up driver for multicast; if no routine in the device table   */
    /* then the driver does not need to be setup to accept multicast    */
    ret_val = TRUE;         
    if (pi->pdev && pi->pdev->setmcast)
    {
        ret_val = pi->pdev->setmcast(pi);
    }
    return(ret_val);
}

/* ********************************************************************   */
/* save_mcast_entry() - save multicast entry information                  */
/*                                                                        */
/*   Saves multicast information in next available entry in multicast     */
/*   table for the specified interface.  A check must be made             */
/*   for room in table before this routine is called.  The address        */
/*   is checked to see if it is a legal multicast address.                */
/*                                                                        */
/*   Interface semaphore must be claimed when this routine is called.     */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

RTIP_BOOLEAN save_mcast_entry(PIFACE pi, PFCBYTE mcast_ip_addr, 
                         PFBYTE mcast_eth_addr)
{
int    off;
PFBYTE pmcaddr;
byte   ill_mcast[IP_ALEN];
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
dword  mcast_addr_conc, my_ip_addr_conc;
int    i;
#endif

    /* illegal mcast addr is {0xe0, 0, 0, 0}   */
    tc_memset(ill_mcast, 0, IP_ALEN);
    ill_mcast[0] = 0xe0;

    if ( IP_ISMCAST_ADDR(mcast_ip_addr[0]) && 
         !tc_cmp4(mcast_ip_addr, ill_mcast, IP_ALEN) )
    {
        off = pi->mcast.lenmclist;
        pmcaddr = (PFBYTE) &(pi->mcast.mclist[off*ETH_ALEN]);

        /* set up a new entry at the end of the list   */
        tc_movebytes(pmcaddr, (PFBYTE)mcast_eth_addr, ETH_ALEN);
        tc_mv4(&(pi->mcast.mclist_ip[off*IP_ALEN]), mcast_ip_addr, IP_ALEN);
        pi->mcast.mcast_cnt[off] = 1;
        pi->mcast.lenmclist++;

#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
        pi->mcast.report_timer[off] = 0;
        OS_RELEASE_IFACE(pi);
#if (INCLUDE_IGMP_V2)
        if (!tc_cmp4((PFBYTE)mcast_ip_addr, (PFBYTE)ip_igmp_all_hosts, IP_ALEN))
        {
            pi->mcast.last_host_toreply[off] = TRUE;  /* last one to send a report */
        }
#endif
        igmp_send_report(pi, (PFBYTE)&(pi->mcast.mclist_ip[off*IP_ALEN]));

        /* repeat the sending of the report after a short delay in case this   */
        /* initial unsolicited membership report gets lost or damamged         */
        mcast_addr_conc = 0;
        my_ip_addr_conc = 0;
        for (i = 0; i < IP_ALEN; i++)
            mcast_addr_conc = (mcast_addr_conc << 8) | 
                              ((unsigned long) (* (mcast_ip_addr+i)));
        for (i = 0; i < IP_ALEN; i++)
            my_ip_addr_conc = (my_ip_addr_conc << 8) | 
                              ((unsigned long) pi->addr.my_ip_addr[i]);
        pi->mcast.report_timer[off] = ((mcast_addr_conc + my_ip_addr_conc) % 
                                      CFG_IGMP_MAX_DELAY) + 1;                

        OS_CLAIM_IFACE(pi, MCAST3_CLAIM_IFACE)
#endif
        return(TRUE);
    }
    return(FALSE);
}

/* ********************************************************************   */
/* MULTICAST LOOPBACK                                                     */
/* ********************************************************************   */
/* mcast_write_loop() - send mcast packet loopback                        */
/*                                                                        */
/*   If the destination address of the packet msg is a multicast address  */
/*   we are listening to, then the packet is sent to the loopback         */
/*   interface.  The loopback transmit will make a copy of the packet     */
/*   and send it to the IP exchange.                                      */
/*                                                                        */
/*   Returns 0 if successful or errno if failure                          */
/*                                                                        */

int mcast_write_loop(DCU msg)           /*__fn__*/
{
PIPPKT pip;
PIFACE pi;
int    iface_no;
#if (INCLUDE_LOOP)
int    status;
#endif
int    ret_status;

    pip = DCUTOIPPKT(msg);
    ret_status = 0;

    /* loop thru interfaces looking for both an open loopback interface   */
    /* and a matching multicast address                                   */
    LOOP_THRU_IFACES(iface_no)
    {
        PI_FROM_OFF(pi, iface_no)
        OS_CLAIM_IFACE(pi, MCAST2_CLAIM_IFACE)  
        if (pi && pi->open_count)   /* if valid, i.e. open */
        {
            if ( pi->pdev && (pi->pdev->device_id == LOOP_DEVICE) &&
                 is_local_mcast(pi, pip->ip_dest) )
            {
#if (INCLUDE_LOOP)
                status = loop_xmit(pi, msg);
                if (status)
                {
                    DEBUG_ERROR("ERROR(RTIP): mcast_write_loop - transmit error",
                        NOVAR, 0, 0);
                    UPDATE_INFO(pi, xmit_errors, 1)
                    ret_status = status;
                }
#else
                ret_status = EIFACECLOSED;
#endif
            }
        }           /* end of if device open */
        OS_RELEASE_IFACE(pi);
    }               /* end of loop thru interfaces */
    return(ret_status);
}

/* ********************************************************************   */
/* INTERFACE WRITE                                                        */
/* ********************************************************************   */

/* ********************************************************************   */
/* tc_interface_send() - Send a packet                                    */
/*                                                                        */
/*  This routine is called to send a packet. It queues the packet on      */
/*  the output list associated with the interface.  The packet will       */
/*  be sent by the IP layer.                                              */
/*                                                                        */
/*  If PKT_FLAG_KEEP is not set, msg will be freed (either by this        */
/*  routine or after packet has been sent)                                */
/*                                                                        */
/*  Returns 0 if success or errno value if failure                        */

#if (DEBUG_OUTPUT_LIST)
void break_out_que(void)
{
DEBUG_ERROR("ticks = ", DINT1, ks_get_ticks(), 0);
    display_packet_info(TRUE);
        display_packet_info(TRUE);
        display_packet_lowwater();
        display_sem_info();
        display_xmit_info();
}
#endif

int tc_interface_send(PIFACE pi, DCU msg)      /*__fn__*/
{
int ret_val;
int len;
PFBYTE data_ptr;

    ret_val = 0;        /* assume success */

    OS_CLAIM_IFACE(pi, SEND_CLAIM_IFACE)

#if (DEBUG_SEND_NO_BLOCK)
    DEBUG_ERROR("tc_interface_send: add DCU,data to output list", DINT2, 
        msg, DCUTODATA(msg));
#endif

    /* add to end of output list   */
    if ( (pi->max_output_que == -1) ||              /* no limit or */
         (pi->no_output_que < pi->max_output_que) ) /* que not full */
    {
        pi->no_output_que++;
#if (DEBUG_OUTPUT_LIST)
        DEBUG_ERROR("ADD TO OUTPUT QUE", EBS_INT1, pi->no_output_que, 0);
        if (pi->no_output_que > 4)
          break_out_que();
#endif

#if (MEASURE_MAX_OUT_QUE)
        if (pi->no_output_que > max_out_queue)
            max_out_queue = pi->no_output_que;
#endif
        DCUTOCONTROL(msg).dcu_flags |= PKT_SEND_IN_PROG;

        /* zero fill ethernet packets which are smaller than minimum size   */
        /* NOTE: this assumes allocated packets are never smaller           */
        /*       than ETHER_MIN_LEN                                         */
        len = DCUTOPACKET(msg)->length;
        if ( (len < ETHER_MIN_LEN) && (pi->pdev->iface_type == ETHER_IFACE) )
        {
            data_ptr = DCUTODATA(msg);
            tc_memset(data_ptr+len, 0, ETHER_MIN_LEN-len);
        }

        pi->ctrl.list_output = 
            os_list_add_rear_off(pi->ctrl.list_output, (POS_LIST)msg, 
                                 OUTPUT_LIST_OFFSET);

        /* if not already sending, start up the send    */
        if (!SEND_IN_PROGRESS(pi))
        {
            tc_process_output(pi);
            /*OS_SET_IP_SIGNAL(pi);   */
        }
    }
    else
    {
        FREE_DCU(msg, DCUTOCONTROL(msg).dcu_flags)
/*      ret_val = EOUTPUTFULL;   */
        DEBUG_ERROR("tc_interface_send: xmit dropped,out list full: num, max = ", 
            EBS_INT2, pi->no_output_que, pi->max_output_que);
    }

    OS_RELEASE_IFACE(pi)

    return(ret_val);
}

/* ********************************************************************        */
/* tc_do_interface_send() - Send a packet and enable TCP retransmission        */
/*                                                                             */
/*  This routine is called to send a packet.  If the package is TCP, then      */
/*  arp must have been resolved, therefore, enable TCP timeout retransmission. */
/*                                                                             */
/*  Returns 0 if success or errno value if failure                             */
/*                                                                             */
int tc_do_interface_send(PIFACE pi, DCU msg)      /*__fn__*/
{
#if (INCLUDE_TCP)
PTCPPORT port;
#if (RTIP_VERSION >= 26)
int eth_type;
#endif
#if (INCLUDE_802_2)
PSNAP     psnap;
#endif

#if (DEBUG_PKT_SENT)
    DEBUG_ERROR("tc_do_interface_send: send out of interface ",
        EBS_INT1, pi->ctrl.index, 0);
#endif

    /* if TCP packet, set sincetime and retrans_en so timeout knowns when    */
    /* last tcp packet was sent                                              */
    /* NOTE: if arp not resolved then timeout will not retry TCP packets     */
#if (RTIP_VERSION >= 26)
    eth_type = DCUTOETHERPKT(msg)->eth_type;
#if (INCLUDE_802_2)
    if (msg_is_802_2(msg))
    {
        psnap = DCUTOSNAP8022(msg);
        eth_type = psnap->snap_type;
    }
#endif
    if ( (eth_type == EIP_68K) &&
         (DCUTOIPPKT(msg)->ip_proto == PROTTCP) )
#else
    if ( (DCUTOETHERPKT(msg)->eth_type == EIP_68K) &&
         (DCUTOIPPKT(msg)->ip_proto == PROTTCP) )
#endif
    {
        DEBUG_LOG("tc_do_interface_send - send TCP pkt", LEVEL_3, NOVAR, 0, 0);
        port = (PTCPPORT)(DCUTOCONTROL(msg).port);

        /* set time last TCP pkt sent (used for timeout) unless specified   */
        /* not to for reset pkts and window probes; do not retrans          */
        /* reset pkts since if get pkts with unknown port will send reset,  */
        /* therefore, if sincetime is cleared for these, it could delay     */
        /* transitioning out of TWAIT or LAST (due to timers expiring)      */
        /* indefinitely                                                     */
        DEBUG_LOG("tc_do_interface_send - tcp flags", LEVEL_3, EBS_INT1,
            DCUTOTCPPKT(msg)->tcp_flags, 0);

        /* arp resolved, therefore, retransmit if timeout   */
        if (port)
            TCP_SETUP_RETRANS(port)
    }
    else
    {
        DEBUG_LOG("tc_do_interface_send - sent non TCP pkt", LEVEL_3, NOVAR, 0, 0);
    }
#endif

#if (INCLUDE_PPP)
    DCUTOPACKET(msg)->protocol = PPP_IP_PROTOCOL;
#endif

    /* send it to output exchange   */
#if (DEBUG_PKT_SENT)
    DEBUG_ERROR("call tc_do_interface_send ", IPADDR, DCUTOIPPKT(msg)->ip_dest, 0);
    DEBUG_ERROR("PKT SENT: ", PKT, DCUTODATA(msg), DCUTOPACKET(msg)->length);
#endif
    return(tc_interface_send(pi, msg));
}

/* ********************************************************************        */
/*  tc_netwrite() - Send a packet to a remote IP address                       */
/*                                                                             */
/*  Send the IP packet at msg to the host at ipn. If the host's                */
/*  address is not yet resolved send out an arp request and enqueue            */
/*  the packet for sending when the address is resolved. If already            */
/*  resolved send the packet. If requested to block until sent, we             */
/*  set flags in the packet that tell the IP task and timer tasks              */
/*  to signal us when the packet was sent or when we time out.                 */
/*  If the user requested it, we keep the packet. Otherwise we discard         */
/*  the packet.  NOTE: this will work with a 0 passed for port but since       */
/*  there is not port signal, signalling when packet is sent out cannot        */
/*  and will not be done.                                                      */
/*                                                                             */
/*  Assumes DCUTOCONTROL(msg).pi is set up before calling this routine.        */
/*                                                                             */
/*  Returns 0 if the packet was sent or it was enqueued. Returns nonzero       */
/*  errno if the ARP request was not sent or signal was specified and we know  */
/*  it was not sent within wait_count                                          */
/*                                                                             */
/*  The process is locked while we scan the arp cache                          */

int tc_netwrite(PIFACE pi, PANYPORT port, DCU msg, PFBYTE ipn, int flags, word wait_count) /*__fn__*/
{
#if (INCLUDE_ETH_BUT_NOARP)
int     found;
#endif
int     ret_val;
RTIP_BOOLEAN is_bc;
RTIP_BOOLEAN is_net_bc;
RTIP_BOOLEAN is_mc;
RTIP_BOOLEAN do_arp;
PIPPKT  pip;
int     iface;
int     rval;
DCU     msg2;
PIFACE  pi2;

    DEBUG_LOG("tc_netwrite() entered, iface = ", LEVEL_3, EBS_INT1, pi->ctrl.index, 0);
#if (DEBUG_NETWRITE)
    DEBUG_ERROR("tc_netwrite() entered, iface = ", EBS_INT1, pi->ctrl.index, 0);
    DEBUG_ERROR("                       dest addr = ", IPADDR, ipn, 0);

#endif

#if (!INCLUDE_ARP)
    ARGSUSED_INT(wait_count);
#endif
   
    /* Check for bogus flag values   */
    if (!port)
        flags &= ~PKT_FLAG_SIGNAL;

    /* Store the flags (discard or not) in the msg   */
    DCUTOCONTROL(msg).dcu_flags = flags;    

    /* Store the port structure (may be zero) in the msg   */
    DCUTOCONTROL(msg).port = (PANYPORT) port;
    
    /* Bind the sent signal if needed   */
    if (flags & PKT_FLAG_SIGNAL)
    {
        OS_BIND_SENT_SIGNAL((PANYPORT)port);
        OS_CLEAR_SENT_SIGNAL(port);
    }

    /* **************************************************   */
    do_arp = TRUE;
    ret_val = 0;                    /* success */
    is_bc = FALSE;          /* later we set if broadcast  */
    is_mc = FALSE;          /* later we set if mcast */
    pip = DCUTOIPPKT(msg);

    /* **************************************************                   */
    /* Check if it is a broadcast. If so use the ethernet broadcast address */
    /* and send the msg to all interfaces execpt loopback                   */
    /* check the first byte before calling compare                          */
    /* NOTE: even though an interface was passed in, the interface number   */
    /*       was only determined it was not used for anything (UDP only)    */
    if ( (ipn[0] == ip_broadaddr[0]) &&           /* 0xff */
          tc_cmp4(ipn, ip_broadaddr, 4))
    {
        is_bc = TRUE;
    }

    /* Check if it is a net-directed broadcast. If so use the ethernet    */
    /* broadcast address and send to the specified interface              */
    is_net_bc = is_net_broadcast(pi, ipn);

    if (is_net_bc || is_bc)
        SETUP_LL_HDR_DEST(pi, msg, broadaddr)

    /* Check if it is a multicast. If so convert the IP address to    */
    /* an ethernet multicast address                                  */
    /* and send the msg to all interfaces execpt loopback             */
    if (IP_ISMCAST_ADDR(ipn[0]))
    {
        is_mc = TRUE;
        ETHER_MAP_IP_MULTICAST((PFBYTE) &ipn[0], 
                               (PFBYTE)&(LL_DEST_ADDR(msg, pi))[0]);

        /* send the packet loopback also if we are listening of the   */
        /* same address we are sending to and the option is enabled   */
        if ( (pip->ip_proto != PROTIGMP) && 
             port && (port->options & SO_MCAST_LOOP) ) 
        {
            if (mcast_write_loop(msg))
            {
                DEBUG_ERROR("tc_netwrite - sending to loopback mcast failed",
                    NOVAR, 0, 0);
            }
        }
    }

    /* **************************************************   */
    /* check if legal to do broadcast or multicast          */
    if (is_bc || is_net_bc || is_mc)
    {
#if (DEBUG_NETWRITE)
        DEBUG_ERROR("tc_netwrite: sending bc, netbc, mc ", EBS_INT2,
            is_bc, is_net_bc);
#endif
        do_arp = FALSE;

        /* double check the packet is UDP or IGMP   */
        if ( (pip->ip_proto != PROTUDP) &&
             (pip->ip_proto != PROTIGMP) )
        {
            FREE_DCU(msg, flags)
            return(EADDRNOTAVAIL);
        }

#        if (INCLUDE_KEEP_STATS)
            INCR_INFO(pi, xmit_nucast)
#        endif
    }

    /* **************************************************          */
    /* if interface has closed, error                              */
    /* NOTE: we do not want to return here in case it is broadcast */
    if (!pi->pdev && !is_bc)
    {
        FREE_DCU(msg, flags)
        return(EIFACECLOSED);
    }

#if (INCLUDE_PPP || INCLUDE_SLIP || INCLUDE_CSLIP)
    /* **************************************************   */
    /* If driver is CSLIP, SLIP or PPP do not ARP           */
    else if (pi->pdev->iface_type == RS232_IFACE)
    {
#if (DEBUG_NETWRITE)
        DEBUG_ERROR("tc_netwrite: sending RS232", NOVAR, 0, 0);
#endif
#        if (INCLUDE_PPP)
            /* check if PPP is open                                     */
            /* NOTE: this is done here where the IP packet is queued on */
            /*       the output list instead of when the IP packet is   */
            /*       sent; if you wait until the packet is sent, the    */
            /*       PPP connection could have started the close after  */
            /*       the IP packet sent (by send etc) but before it is  */
            /*       output by the IP layer                             */
            if (pi->pdev->device_id == PPP_DEVICE)  
            {
                ret_val = CHECK_PPP_NOT_OPEN(pi);
                if (ret_val)
                {
                    FREE_DCU(msg, flags)
                    return(ret_val);
                }
            }
#        endif

        INCR_INFO(pi, xmit_ucast)
        do_arp = FALSE;
    }
#endif

    /* **************************************************            */
    /* if multicast or broadcast, don't need to arp                  */
    /* broadcast: need to send to every interface (except loop back) */
    /* multicast: only send to one interface which is specified by   */
    /*            routing table                                      */
    if (!do_arp)
    {
        /* for multicast, rs232 or net-directed broadcast, send to    */
        /* the interface specified                                    */
        if (!is_bc)
        {
            return(tc_do_interface_send(pi, msg));
        }

        /* DHCP which know interface will set broadcast_pi so that   */
        /* request is sent on the specified interface only           */
        else if (port && ((PUDPPORT)port)->broadcast_pi)
        {
            return(tc_do_interface_send(((PUDPPORT)port)->broadcast_pi, msg));
        }

        /* if should broadcast on only one network   */
        else if ( port && 
                  (port->port_flags & PORT_BOUND) &&
                  !(port->port_flags & PORT_WILD) )
        {
            /* based upon IP address bound to, get the interface to send   */
            /* out on                                                      */
            iface = get_local_ifaceno(((PUDPPORT)port)->ip_connection.ip_src);
            if (iface >= 0)
            {
                pi = tc_ino2_iface(iface, DONT_SET_ERRNO);
                if (pi)
                    return(tc_do_interface_send(pi, msg));
            }
        }

        /* send the packet out on all the interfaces (except LOOPBACK)   */
        iface = 0;
        while ( (pi = tc_get_nxt_bc_iface(iface)) != 0 )
        {
            /* get next interface to try in next loop   */
            iface = pi->ctrl.index + 1;

            /* set up the correct IP address for the interface going out   */
            /* on                                                          */
            if (port && port->port_flags & PORT_WILD) 
                tc_mv4(pip->ip_src, pi->addr.my_ip_addr, 4);

            /* see if this is the last interface to send to (i.e.   */
            /* pi2 will be 0 if it is)                              */
            pi2 = tc_get_nxt_bc_iface(iface);

            /* if don't want to keep or want to signal, and         */
            /* it is not the last interface, then copy the DCU and  */
            /* send the copy which will not signal and will be      */
            /* discarded                                            */
            if ( (!(DCUTOCONTROL(msg).dcu_flags & PKT_FLAG_KEEP) || 
                   (DCUTOCONTROL(msg).dcu_flags & PKT_FLAG_SIGNAL)) && 
                 pi2 )  
            {
                msg2 = os_alloc_packet(DCUTOPACKET(msg)->length, CAST_ALLOC); 
                    if (!msg2)
                {
                    DEBUG_LOG("tc_netwrite - os_alloc_packet failed", 
                        LEVEL_3, NOVAR, 0, 0);
                    rval = ENOPKTS;
                }
                else
                {
                    /* copy msg to lbmsg which was just allocated (set flags to no keep,   */
                    /* no signal)                                                          */
                    COPY_DCU_FLAGS(msg2, msg)

                    rval = tc_do_interface_send(pi, msg2);
                }
            }
            else
            {
                /* (keep and don't signal) or last time thru loop   */
                rval = tc_do_interface_send(pi, msg);
            }

            /* if error, continue to broadcast but return errno   */
            if (rval)          
                ret_val = rval;
        }   /* end of while */
        return(ret_val);
    }       /* end of if !do_arp */

    /* **************************************************                 */
    /* Check if it is a loopback.  If so the ethernet address will not be */
    /* used since it will not be sent over the network but will be sent   */
    /* to itself, therefore, clear the ethernet address and put the msg   */
    /* on the output exchange                                             */
    else if ( ret_val == 0 &&
              ((pi->pdev->iface_type == LOOP_IFACE) ||
               (ipn[0] == IP_LOOP_ADDR) || 
               tc_get_local_pi(ipn)) )
    {
        DEBUG_LOG("tc_netwrite() - sending loopback msg", LEVEL_3, NOVAR, 0, 0);
        SETUP_LL_HDR_DEST(pi, msg, broadaddr)
        INCR_INFO(pi, xmit_ucast)
        return(tc_do_interface_send(pi, msg));
    }

    /* **************************************************               */
    /* NEED ETHER ADDRESS - CHECK CONFIG BEFORE PROCEEDING TO ARP LAYER */
    /* **************************************************               */
    DEBUG_LOG("tc_netwrite() - sending packet msg - not lb,slip or bc", 
        LEVEL_3, NOVAR, 0, 0);

#if (INCLUDE_ETH_BUT_NOARP)
    /* we need a hardware address but they are in a table and we   */
    /* do not need to arp                                          */
    for (found=0; found < CFG_NUM_IP2ETH_ADDR; found++)
    {
        if (tc_cmp4(ipn, ip2eth_table[found].ip_addr, 4))
        {
            pi = DCUTOCONTROL(msg).pi;
            SETUP_LL_HDR_DEST(pi, msg, ip2eth_table[found].hw_addr);
            return(tc_do_interface_send(pi, msg));
        }
    }
#endif

#if (!INCLUDE_ARP)
    FREE_DCU(msg, flags)
    return(EWOULDARP);
#else
#if (DEBUG_NETWRITE)
    DEBUG_ERROR("tc_netwrite: call tc_arp_remote for ", IPADDR, ipn, 0);
#endif
    /* ARP remote host   */
    return(tc_arp_remote(pi, port, msg, ipn, flags, wait_count));
#endif
}

/* ********************************************************************   */
/* ********************************************************************   */
/* MANAGE OUTPUT DCU LIST                                                 */
/* ********************************************************************   */
/* ********************************************************************   */

/* ********************************************************************   */
/* START XMIT                                                             */
/* ********************************************************************   */
/* if msg is 0, then no more msgs to xmit                                 */
/*  ASSUMPTIONS: interface semaphore is claimed when called               */
/*               interrupts are enabled                                   */
int start_one_driver_xmit(PIFACE pi, DCU msg)
{
#if (INCLUDE_XMIT_QUE)
int status;
int done_status;
#endif
int ret_val;

#if (!INCLUDE_XMIT_QUE)
    pi->xmit_dcu = msg;
#else

    if (msg)
    {
        /* add msg to xmit list   */
        pi->ctrl.list_xmit = 
            os_list_add_rear_off(pi->ctrl.list_xmit, (POS_LIST)msg, 
                                 XMIT_LIST_OFFSET);
        pi->xmit_started++;
    }
#endif

#    if (DEBUG_SEND_NO_BLOCK)
        DEBUG_ERROR("tc_ip_interpret: start send: msg, list_output after remove", 
            DINT2, msg, pi->ctrl.list_output);
#    endif

    pi->xmit_status = 0;                  

    /* start xmit completion timer   */
    START_XMIT_TIMER(pi)

    /* DRIVER XFER THE PACKET                      */
    /* (0 = success; REPORT_XMIT_DONE = xmit done) */
    ret_val = pi->xmit_status = _interface_write(pi, msg);

#if (!INCLUDE_XMIT_QUE)
    if (pi->xmit_status != 0)       /* if send complete or error */
    {
        if (pi->xmit_status == REPORT_XMIT_DONE)    /* if send complete */
            pi->xmit_status = 0;                    /* success */
        KS_TL_DISABLE()         /* Disable interrupts for xmit_done */
        pi->xmit_done_counter++;
        KS_TL_ENABLE()              /* enable interrupts */
    }
#else
    /* if status says queue but don't start then can xfer another           */
    /* return this status and start_one_driver_xmit() will be called again; */
    /* when no more DCUs are available to transmit, then call               */
    /* driver with 0 DCU which means for the driver to start the xmit       */

    done_status = pi->xmit_status & REPORT_XMIT_DONE;
    status = pi->xmit_status & ~(REPORT_XMIT_DONE | REPORT_XMIT_MORE);

    if (done_status || status > 0)      /* if send complete or error */
    {
        KS_TL_DISABLE()         /* Disable interrupts for xmit_done */
        status = -status;       /* make it positive */
        pi->xmit_done_counter += status;
        KS_TL_ENABLE()              /* enable interrupts */
    }
#endif
    return(ret_val);
}

/* ********************************************************************   */
/*  ASSUMPTIONS: interface semaphore is claimed when called               */
/*               interrupts are enabled                                   */
/*  Returns TRUE if processed output, FALSE if not                        */
RTIP_BOOLEAN start_driver_xmit(PIFACE pi)
{
int status;
RTIP_BOOLEAN processed_output;
DCU msg;
int     dcu_flags;

    processed_output = FALSE;
#if (INCLUDE_XMIT_QUE)
    status = REPORT_XMIT_MORE;
    while (status & REPORT_XMIT_MORE)
#endif
    {
        processed_output = TRUE;

        /* remove msg from output list   */
        msg = (DCU)pi->ctrl.list_output;
        if (msg)
        {
            pi->ctrl.list_output = 
                    os_list_remove_off(pi->ctrl.list_output, (POS_LIST)msg,
                               OUTPUT_LIST_OFFSET);
            pi->no_output_que--;

#if (DEBUG_OUTPUT_LIST)
            DEBUG_ERROR("REMOVE OUTPUT QUE", EBS_INT1, pi->no_output_que, 0);
#endif
        }

        /* **************************************************           */
        /* if DCU is not a packet to be xmitted but is a DCU used       */
        /* to signal the IP layer to do something special               */
        /* if this DCU specified closing the interface, then close it   */
        /* NOTE: this was a special packet queued by xn_interface_close */
        /*       so that closing the interface can be done in sync with */
        /*       the output queue; i.e. so the interface is not         */
        /*       closed while there are packets on the output queue     */
        dcu_flags = 0;
        if (msg)
            dcu_flags = DCUTOCONTROL(msg).dcu_flags;

        if (msg && (dcu_flags & CLOSE_IFACE))
        {
            os_free_packet(msg);
#if (!INCLUDE_XMIT_QUE)
            /* clear xmit in progress in case a callback needs   */
            /* to check for it                                   */
            pi->xmit_dcu = (DCU)0;
#endif
            /* reset flag so xn_rtip_exit knows not to wait until close done   */
            pi->iface_flags &= ~CLOSE_IN_PROGRESS;

            /* finish the close (and reset CLOSE_IFACE flag)   */
            finish_interface_close(pi);         /* NOTE: releases and */
                                                /* claims iface sem   */
        }
        /* **************************************************   */
        else    /* not a "special DCU", i.e. MODEM_DROP etc */
        {
            /* NOTE: 0 msg tells driver to xmit any msgs queued   */
            status = start_one_driver_xmit(pi, msg);
        }
        /* **************************************************   */
    }

    if (status) status = status;
    
    return(processed_output);
}

/* ********************************************************************   */
/* XMIT DONE                                                              */
/* ********************************************************************   */
/* pi->xmit_done_counter specifies how many DCUs completed XMIT           */
/*  ASSUMPTIONS: interface semaphore is claimed when called               */
/*               interrupts are enabled                                   */
void process_xmit_done(PIFACE pi)
{
#if (INCLUDE_XMIT_QUE)
int i;
int temp;
#endif

#if (INCLUDE_XMIT_QUE)
    KS_TL_DISABLE()         /* Disable interrupts for xmit_done */
    temp = pi->xmit_done_counter;
    KS_TL_ENABLE()          /* Disable interrupts for xmit_done */
    for (i=0; i < temp; i++)
    {
        process_one_xmit_done(pi, (DCU)(pi->ctrl.list_xmit));
    }
    pi->xmit_done_counter -= temp;
#else
    process_one_xmit_done(pi, pi->xmit_dcu);
#endif
}

/* process one DCU which has been completed its xmit        */
/*  ASSUMPTIONS: interface semaphore is claimed when called */
/*               interrupts are enabled                     */
void process_one_xmit_done(PIFACE pi, DCU msg)
{
RTIP_BOOLEAN xmit_finished;
RTIP_BOOLEAN succ;
#if (INCLUDE_XMIT_QUE)
int status;
#endif

#    if (DEBUG_SEND_NO_BLOCK)
#        if (INCLUDE_XMIT_QUE)
        DEBUG_ERROR("tc_ip_interpret: send done, xmit_done_counter", DINT2, 
            pi->ctrl.list_xmit, pi->xmit_done_counter);
#        else
        DEBUG_ERROR("tc_ip_interpret: send done, xmit_done_counter", DINT2, 
            pi->xmit_dcu, pi->xmit_done_counter);
#        endif
#    endif


    /* call device driver xmit complete routine                  */
    /* NOTE: for ethernet this occurs when packet has been sent; */
    /*       for RS232 this occurs when there is more room in    */
    /*       the circular output buffer                          */
    xmit_finished = TRUE;
    succ = TRUE;
#if (INCLUDE_XMIT_QUE)
    status = pi->xmit_status & ~REPORT_XMIT_DONE;
    if (status > 0 || pi->xmit_status & REPORT_XMIT_DONE)
#else
    if (pi->xmit_status)
#endif
    {
        succ = FALSE;
    }

    if (pi->pdev->xmit_done)
    {
        xmit_finished = pi->pdev->xmit_done(pi, msg, succ); 
    }

    /* if xmit done or error   */
    if (xmit_finished)      
    {
        /* stop xmit completion timer   */
        pi->xmit_done_timer = 0;

#if (INCLUDE_XMIT_QUE)
        KS_TL_DISABLE()         /* Disable interrupts for xmit_done */
        /* remove xmitted msg from xmit list   */
        msg = (DCU)(pi->ctrl.list_xmit);
            pi->ctrl.list_xmit = 
                os_list_remove_off(pi->ctrl.list_xmit, (POS_LIST)msg,
                               XMIT_LIST_OFFSET);
        pi->xmit_started--;

        /* if another xmit has already been started, then   */
        /* start the xmit completion timer for it           */
        /* NOTE: we only keep one timer instead of one per  */
        /*       DCU, so if they are timing out, they could */
        /*       actually take longer to time out           */
        if (pi->ctrl.list_xmit)
        {
            START_XMIT_TIMER(pi)
        }
        KS_TL_ENABLE()          /* Disable interrupts for xmit_done */
#else
        msg = pi->xmit_dcu;
        pi->xmit_dcu = (DCU)0;
#endif

        DCUTOCONTROL(msg).dcu_flags &= ~PKT_SEND_IN_PROG;
        if (DCUTOCONTROL(msg).dcu_flags & PKT_FREE)
        {
#            if (DEBUG_SEND_NO_BLOCK)
                DEBUG_ERROR("tc_ip_interpret: free DCU, data", DINT2, 
                    msg, DCUTODATA(msg));
#            endif
            os_free_packet(msg);        /* Free message */
        }

        else
        {
            /* Give the result to release message.
               - If there is someone waiting on the packet 
                 tc_release message will wake him up;
               - It will free the packet if not PKT_FLAG_KEEP */
            tc_release_message(msg, pi->xmit_status); 
        }

#if (DEBUG_SEND_NO_BLOCK)
    DEBUG_ERROR("tc_ip_interpret: xmit done - reset xmit dcu", 
        NOVAR, 0, 0);
#endif
    }
}

/* ********************************************************************   */
/* MAIN OUTPUT PROCESS ROUTINE - check_process_output()                   */
/* ********************************************************************   */

/* ********************************************************************   */
/* tc_process_output() - Process output queue                             */
/*                                                                        */
/*  This routine is called by the IP layer to send packets on the         */
/*  output queue (packets were queued by tc_interface_send).              */
/*  If a previous transmit completed or a transmit is not in progress,    */
/*  and there is a packet on the output queue, it starts transmitting     */
/*  a packet from the output queue.                                       */
/*                                                                        */
/*  ASSUMPTIONS: interface semaphore is claimed when called               */
/*               interrupts are enabled                                   */
/*                                                                        */
/*  Returns TRUE if there was any processing to do; FALSE if it did       */
/*  not do anything                                                       */

RTIP_BOOLEAN tc_process_output(PIFACE pi)
{
RTIP_BOOLEAN processed_output = FALSE;

    /* **************************************************                  */
    /* PROCESS OUTPUT FROM IP LAYER -                                      */
    /*   tc_interface_send - puts packet on output list and signals        */
    /*                       IP layer if xmit not in progress              */
    /*   DRIVER interrupt - signals IP tasks when output complete          */
    /*   IP layer - starts output of all packets (calls _interface_write); */
    /*              processes completion of outputs                        */
    /*                                                                     */
    /* THE FOLLOWING FLAGS ETC ARE USED:                                   */
#if (INCLUDE_XMIT_QUE)
    /*   pi->ctrl.list_xmit: xmit list of packet which are in progress   */
    /*                  of being xmitted                                 */
    /*                  (protected by IFACE SEMAPHORE)                   */
#else
    /*   pi->xmit_dcu:  packet which is being sent (but not completed)   */
    /*                  (protected by IFACE SEMAPHORE)                   */
#endif
    /*   pi->xmit_done: flag set by device driver to let IP layer know     */
    /*                  it was signalled due to send complete;             */
    /*                  (protected by disabling interrupts)                */
    /*   msg->dcu_flags: flags which keeps track of if a packet was freed  */
    /*                  but has not been returned to the free list since   */
    /*                  output has not completed                           */
    /*   pi->ctrl.list_output, dcu->ctrl.list3(OUTPUT_LIST_OFFSET): output */
    /*                  list of packets waiting to be sent                 */
    /*                  (protected by IFACE SEMAPHORE)                     */
    /* **************************************************                  */
    KS_TL_DISABLE()         /* Disable interrupts since checking xmit_done */

    /* if signal was due to xmit complete or         */
    /*    signal was due to need to start a new xmit */
    while ( TRANSMIT_COMPLETE(pi) || CAN_START_NEW_XMIT(pi) )
    {
        /* **************************************************              */
        /* if previous send of a DCU is done or timed out;                 */
        /* NOTE: IP task was signalled from the driver interrupt routine   */
        /*       when send completed                                       */
        /* NOTE: for rs232 this means there is room in the circular buffer */
        if (TRANSMIT_COMPLETE(pi))
        {
            pi->xmit_done_counter--;

            KS_TL_ENABLE()          /* enable interrupts */
            processed_output = TRUE;

            /* calls device driver done routine and release message   */
            process_xmit_done(pi);
        }
        else
        {
            KS_TL_ENABLE()          /* enable interrupts */
        }

        /* **************************************************               */
        /* start sending another packet if one is not in progress and the   */
        /* output list is not empty                                         */
        /* NOTE: the IP task could have been signalled to start a send when */
        /* one was not in progress; or it could have been signalled when    */
        /* a send completed                                                 */
        if (CAN_START_NEW_XMIT(pi) && !SEND_IN_PROGRESS(pi))
        {
            processed_output = start_driver_xmit(pi);
        }                       /* end of if CAN_START_NEW_XMIT */
        KS_TL_DISABLE()         /* Disable interrupts  */
    }       /* end of while */

    KS_TL_ENABLE()          /* enable interrupts */

    return(processed_output);
}

/* ********************************************************************   */
/* check if need to process output                                        */
RTIP_BOOLEAN check_process_output(PIFACE pi)
{
RTIP_BOOLEAN processed_output = TRUE;

#if (!INCLUDE_XMIT_QUE)
    OS_CLAIM_IFACE(pi, IP_SEND_CLAIM_IFACE)
    OS_IFACE_SIGNAL_CLEAR(OS_HNDL_TO_SIGNAL(IF_EX_IP), pi);
#endif

    KS_TL_DISABLE()

    /* First check if there is anything to do before incurring subroutine
       overhead */
    /* if signal was due to xmit complete or         */
    /*    signal was due to need to start a new xmit */
    if (TRANSMIT_COMPLETE(pi) || CAN_START_NEW_XMIT(pi))
    {
        KS_TL_ENABLE()
#if (INCLUDE_XMIT_QUE)
        OS_CLAIM_IFACE(pi, IP_SEND_CLAIM_IFACE)
#endif

#if (DEBUG_IP_INTERPRET || DEBUG_OUT_PKTS)
        DEBUG_ERROR("ip_interpret: process output queue: xmit_done, output que: ", 
            DINT2, pi->xmit_done, pi->ctrl.list_output);
#endif
        /* process output queue (i.e. start xmits etc)   */
        processed_output = tc_process_output(pi);

#if (INCLUDE_XMIT_QUE)
        OS_RELEASE_IFACE(pi)
#endif
    }
    else
    {
        KS_TL_ENABLE()
        processed_output = FALSE;
    }

#if (!INCLUDE_XMIT_QUE)
    OS_RELEASE_IFACE(pi)
#endif
    return(processed_output);
}

/* ********************************************************************               */
/* _interface_write() - call device driver to send packet                             */
/*                                                                                    */
/*   Calls appropriate device driver specified in device table to send                */
/*   a message.                                                                       */
/*                                                                                    */
/*   Returns:                                                                         */
/*      0 =  one packet started, i.e. success                                         */
/*      < 0 = number of packets started to xmit, i.e. |return value| = number started */
/*      REPORT_XMIT_DONE means xfer complete                                          */
/*      > 0 = ERRNO                                                                   */

int _interface_write(PIFACE pi, DCU msg)              /*__fn__*/
{
#if (RTIP_VERSION <= 25)
#if (INCLUDE_LOOP)
PETHER pe;
PIPPKT pip;
#endif
#endif
int    status;

    if (!pi->open_count)
    {
        DEBUG_LOG("error, interface not open, interface = ", LEVEL_3, EBS_INT1, 
            pi->ctrl.index, 0);
        return(EIFACECLOSED);
    }

#if (DISPLAY_OUTPUT_PKT)
    if (msg)
    {
        DEBUG_LOG("interface_write: OUTPUT PACKET = ", LEVEL_3, PKT, 
            (PFBYTE)DCUTOETHERPKT(msg), DCUTOPACKET(msg)->length);
    }
#endif

    /* **************************************************             */
    /* if sending to loop back address, do not send it out but        */
    /* send it back to itself, i.e. put it on IP exchange (i.e. input */
    /* queue)                                                         */
    /* NOTE: sending to a local IP address will call loop_xmit below  */
    /*       (pi->pdev->xmit)                                         */
#if (RTIP_VERSION <= 25)
#if (INCLUDE_LOOP)
    if (msg)
    {
        pip = DCUTOIPPKT(msg);
        pe = DCUTOETHERPKT(msg);
    }
    if (msg && (pi->pdev->iface_type != RS232_IFACE))
    {
        if ( (pe->eth_type == EIP_68K) && (pip->ip_dest[0] == IP_LOOP_ADDR))
        {
#            if (DEBUG_LOOP)
                DEBUG_ERROR("iface - loop pkt", PKT, 
                    DCUTODATA(msg), DCUTOPACKET(msg)->length);
                DEBUG_ERROR("iface - size = ", EBS_INT1, 
                    DCUTOPACKET(msg)->length, 0);
#            endif
            DEBUG_LOG("iface - sending loop back", LEVEL_3, NOVAR, 0, 0);
            DEBUG_LOG("interface_write, ip type =", LEVEL_3, EBS_INT1, 
                pip->ip_proto, 0);
            status = loop_xmit(pi, msg);
            if (status)
            {
                DEBUG_ERROR("ERROR(RTIP): _interface_write() - transmit error",
                    NOVAR, 0, 0);
                UPDATE_INFO(pi, xmit_errors, 1)
                return(status);
            }
            else
            {
                UPDATE_INFO(pi, xmit_packets, 1)
                UPDATE_INFO(pi, xmit_bytes, DCUTOPACKET(msg)->length)
                return(0);
            }
        }
    }
#endif
#endif

    /* **************************************************                */
    /* Call device driver to send the packet. Retry once on failure      */
    DEBUG_LOG("interface_write - sending pkt", LEVEL_3, NOVAR, 0, 0);

    status = pi->pdev->xmit(pi, msg);

    if (msg)
    {

        if (XMIT_COMPLETE_NO_ERROR(status))
        {
            UPDATE_INFO(pi, xmit_packets, 1)
            UPDATE_INFO(pi, xmit_bytes, DCUTOPACKET(msg)->length)
        }
        else
        {
            DEBUG_LOG("error : interface_write - xmit error", LEVEL_3, NOVAR, 0, 0);
            UPDATE_INFO(pi, xmit_errors, 1)
        }
    }
    return(status); 
}

/* ********************************************************************   */
/* xmit_done_timer - process any device driver xmits which have timed out */
/*                                                                        */
/*   Decrements timer for any xmit in progress for all interfaces.  If    */
/*   a xmit has timed out it signals the IP layer with the error          */
/*   so the next xmit can be started.                                     */
/*                                                                        */
/*   Returns nothing                                                      */

void xmit_done_timer(void)
{
PIFACE pi;
int iface_no;

    LOOP_THRU_IFACES(iface_no)
    {
        PI_FROM_OFF(pi, iface_no)
#if (INCLUDE_XMIT_QUE)
        if (pi && pi->ctrl.list_xmit)
#else
        if (pi && pi->xmit_dcu)
#endif
        {
            OS_CLAIM_IFACE(pi, XMIT_DONE_IFACE)
            if (pi->xmit_done_timer == 1)
            {
                DEBUG_ERROR("xmit_done_timer: xmit timed out", NOVAR, 0, 0);
                pi->xmit_done_timer = 0;
                pi->xmit_status = ENETDOWN; /* pass errno back to IP layer */

#if (INCLUDE_TRK_PKTS )
                display_sem_info();
#endif

                /* timed out   */
                ks_invoke_output(pi, 1);
            }
            else if (pi->xmit_done_timer > 1)
            {                                                      
                if (pi->xmit_done_timer <= CFG_ETHER_XMIT_TIMER)
                {
#if (DISPLAY_XMIT_TIMER_DECR)
                    DEBUG_ERROR("xmit_done_timer: xmit timer decr", EBS_INT2,
                        pi->xmit_done_timer, ks_get_ticks());
#endif
                }
                pi->xmit_done_timer--;
            }

            OS_RELEASE_IFACE(pi)
        }
    }
}
