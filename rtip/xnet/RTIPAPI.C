/*                                                                      */
/* RTIPAPI.C - API functions                                            */
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
/*  for the udp/tcp implementation.                                     */
/*                                                                      */
/*  Functions in this module                                            */
/* xn_rtip_init         - Initialize the tcpip run time environment     */
/* xn_device_table_add  - Add device table entry                        */
/* xn_rtip_exit         - Perform cleanup before exiting                */
/* xn_rarp              - Determine the local IP address using RARP     */
/* xn_bootp             - Determine the local IP address using BOOTP    */
/* xn_ping              - "PING" a host and wait for it to reply        */
/* xn_set_ip            - Set the ports IP address                         */
/* xn_abort             - Abort a connection                               */
/* xn_pkt_data_max      - Returns the size of the data area in a           */
/*                        message packet                                   */
/* xn_pkt_data_size     - Returns the amount of valid data in a received   */
/*                        message packet                                   */
/* xn_pkt_data_pointer  - Returns a pointer to the first byte of the data  */
/*                        area of a message packet                         */
/* xn_pkt_recv          - wait for the system to deliver a message packet  */
/*                        from a port.                                     */
/* xn_pkt_send          - send a message packet through a port.            */
/* xn_interface_open    - Opens an interface for LOOKBACK or ETHERNET      */
/* xn_interface_open_config- Opens an interface for LOOKBACK or ETHERNET   */
/* xn_pkt_alloc         - Allocate a DCU                                   */
/* xn_pkt_free          - Frees a DCU                                      */
/* xn_ip_set_options    - Set IP options                                   */
/* xn_arp_send          - Sends a gratuitous arp                           */
/* xn_arp_add           - Add an entry to ARP cache                        */
/* xn_arp_del           - Delete an entry from the ARP cache               */

#define DIAG_SECTION_KERNEL DIAG_SECTION_API

#include "rtip.h"
#include "rtipext.h"
#if(INCLUDE_RIP)
    #include "rip.h"
#endif
#if (INCLUDE_VFS)
    #include "vfile.h"
#endif
#include "rtipext.h"

#include "rtos.h" /*OS*/ /* added */

/* ********************************************************************   */
#if (USE_PCVID_INPUT && USE_KEYSCAN)
extern byte   keyscan_on;
#endif

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_MALLOC       0
#define DEBUG_MALLOC_CALLS 0
#define DEBUG_LARGE_MALLOC 0

/* ********************************************************************   */
/* LOCAL FUNCTIONS                                                        */
int             rtip_init(RTIP_BOOLEAN restart);
int             init_data(void);
#if (INCLUDE_RUN_TIME_CONFIG)
RTIP_BOOLEAN    alloc_config_data(void);
void            init_config_data(void);
void            free_config_data(void);
#endif
int             _xn_rtip_exit(RTIP_BOOLEAN free_resources);
#if (INCLUDE_IPV4 && INCLUDE_PING)
DCU             do_ping(PANYPORT port, int sequence, int len, int ttl, PFBYTE host, unsigned int wait_count);
#endif
void            abort_all_sockets(RTIP_BOOLEAN tcp_send_reset);

static int poolregistered=0;	//JLA(IO) The same memory pool was registered each time RTIP was restarted,
								//Which made BGET buffer allocator go haywire.
								//Now the memory pool is only registered at the first call to init_data.
								//(init_data is called by rtip_init and rtip_restart

/* ********************************************************************   */
/* EXTERNAL FUNCTIONS                                                     */
#if (!BUILD_NEW_BINARY)
/* this should be done by application for binary version   */
void register_add_ons(void);
#endif

/* ********************************************************************      */
/* xn_rtip_init() -  Initialize TCP/IP runtime environment                   */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   int xn_rtip_init()                                                      */
/*                                                                           */
/* Description:                                                              */
/*   Performs the following:                                                 */
/*     Allocates memory for the system                                       */
/*     Creates signals semaphores and tasks required by the system           */
/*     Initializes RTIP data                                                 */
/*                                                                           */
/*   NOTE: call xn_rtip_restart if RTIP needs to be initialized again        */
/*   after xn_rtip_init has already been called.                             */
/*                                                                           */
/*   For more details see the RTIP Manual.                                   */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_rtip_init(void)                                         /*__fn__*/
{
    OS_DebugSendString("ENTERED xn_rtip_init\n"); /*OS*/
    /* if already initialized, do nothing; this is not considered an   */
    /* error                                                           */
    if (rtip_initialized)
        return(0);

    rtip_initialized = TRUE;

    /* initialize global data   */
    return(rtip_init(FALSE));
}

/* ********************************************************************              */
/* xn_device_table_add() -  Add device                                               */
/*                                                                                   */
/* Summary:                                                                          */
/*   #include "rtipapi.h"                                                            */
/*                                                                                   */
/*   void xn_device_table_add(device_id, minor_number, iface_type,                   */
/*                            device_name, dev_open, dev_close,                      */
/*                            dev_xmit, dev_xmit_done, dev_proc_interrupts,          */
/*                            dev_stats, dev_smcast)                                 */
/*     int device_id                              - device id                        */
/*     int minor_number                           - minor number (MINOR_0 etc)       */
/*     int iface_type                             - interface type (ETHER_IFACE etc) */
/*     PFCHAR device_name                         - device name                      */
/*     DEV_OPEN dev_open                          - open device routine              */
/*     DEV_CLOSE dev_close                        - close device routine             */
/*     DEV_XMIT dev_xmit                          - transmit device routine          */
/*     DEV_XMIT_DONE dev_xmit_done                - transmit complete device routine */
/*     DEV_PROCESS_INTERRUPTS dev_proc_interrupts - process interrupts routine       */
/*     DEV_STATS dev_stats                        - update device statistics routine */
/*     DEV_SETMCAST dev_smcast                    - setup multicast routine          */
/*                                                                                   */
/* Description:                                                                      */
/*   Performs the following:                                                         */
/*     Adds an entry to the device table for the device specified by                 */
/*     the parameters.                                                               */
/*     Must be called after xn_rtip_init()                                           */
/*                                                                                   */
/*   For more details see the RTIP Manual.                                           */
/*                                                                                   */
/* Returns:                                                                          */
/*   Returns 0 if successful, -1 if failure                                          */
/*                                                                                   */

int xn_device_table_add(int device_id, int minor_number, int iface_type,
                        PFCHAR device_name,
#if (INCLUDE_SNMP)
                        struct oid media_mib, dword speed,
#endif
                        DEV_OPEN dev_open,
                        DEV_CLOSE dev_close,
                        DEV_XMIT dev_xmit,
                        DEV_XMIT_DONE dev_xmit_done,
                        DEV_PROCESS_INTERRUPTS dev_proc_interrupts,
                        DEV_STATS dev_stats,
                        DEV_SETMCAST dev_smcast
                       )
{
int i;

    for (i = 0; i < total_devices; i++)
    {
        if (devices_ptr[i].device_id == 0)  /* invalid entry */
        {
            STRUCT_COPY(devices_ptr[i], default_device_entry);
            devices_ptr[i].device_id    = device_id;
            devices_ptr[i].minor_number = minor_number;
            devices_ptr[i].iface_type   = iface_type;
            tc_strcpy(devices_ptr[i].device_name, device_name);
#if (INCLUDE_SNMP)
            devices_ptr[i].media_mib    = media_mib;
            devices_ptr[i].speed        = speed;
#endif
            devices_ptr[i].open         = (DEVICE_OPEN)dev_open;
            devices_ptr[i].close        = (DEVICE_CLOSE)dev_close;
            devices_ptr[i].xmit         = (DEVICE_XMIT)dev_xmit;
            devices_ptr[i].xmit_done    = (DEVICE_XMIT_DONE)dev_xmit_done;
            devices_ptr[i].proc_interrupts =
                (DEVICE_PROCESS_INTERRUPTS)dev_proc_interrupts;
            devices_ptr[i].statistics   = (DEVICE_STATS)dev_stats;
            devices_ptr[i].setmcast     = (DEVICE_SETMCAST)dev_smcast;

            if (iface_type == RS232_IFACE)
            {
                devices_ptr[i].mtu = CFG_RS232_MAX_MTU;
                devices_ptr[i].max_mss_out = CFG_RS232_MAX_MSS;
                devices_ptr[i].window_size_in = CFG_RS232_MAX_WIN_IN;
                devices_ptr[i].window_size_out = CFG_RS232_MAX_WIN_OUT;
            }
            else
            {
                devices_ptr[i].mtu = CFG_ETHER_MAX_MTU;
                devices_ptr[i].max_mss_out = CFG_ETHER_MAX_MSS;
                devices_ptr[i].window_size_in = CFG_ETHER_MAX_WIN_IN;
                devices_ptr[i].window_size_out = CFG_ETHER_MAX_WIN_OUT;
            }

            valid_device_entries++;
            return(0);
        }
    }
    return(set_errno(ETABLEFULL));
}

/* ********************************************************************     */
/* xn_rtip_restart() -  Restarts RTIP                                       */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_rtip_restart()                                                  */
/*                                                                          */
/* Description:                                                             */
/*   Performs the following:                                                */
/*     Allocates memory for the system                                      */
/*     Initializes RTIP data                                                */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Zero is returned on success. Otherwise -1                              */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */

int xn_rtip_restart(void)                                           /*__fn__*/
{
int ret_val;

    /* if xn_rtip_init() and xn_rtip_exit() have not called, return error   */
    if (!rtip_initialized || !rtip_exited)
        return(set_errno(ENOTINITIALIZED));

    ret_val = rtip_init(TRUE);
    if (ret_val == 0)   /* success */
        rtip_exited = FALSE;

    return(ret_val);
}

/* ********************************************************************     */
/* rtip_init() -  Initialize TCP/IP runtime environment                     */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_rtip_init()                                                     */
/*                                                                          */
/* Description:                                                             */
/*   Performs the following:                                                */
/*     Allocates memory for the system                                      */
/*     Creates signals semaphores and tasks required by the system          */
/*     Initializes RTIP data                                                */
/*                                                                          */
/* Returns:                                                                 */
/*   Zero is returned on success. Otherwise -1                              */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int rtip_init(RTIP_BOOLEAN restart)                                   /*__fn__*/
{
    OS_DebugSendString("ENTERED rtip_init\n"); /*OS*/
#if (!BUILD_NEW_BINARY)
    /* register add-ons                                      */
    /* this should be done by application for binary version */
    register_add_ons();
#endif

#if (INCLUDE_RUN_TIME_CONFIG)
    if (!restart)
    {
        init_config_data();
        OS_DebugSendString("BEFORE alloc_config_data\n"); /*OS*/
        if (!alloc_config_data())
            return(set_errno(ERSCINITFAIL));
    }
#endif

    /* initialize global data - do this after init_config_data() so tables   */
    /* will be allocated                                                     */
    OS_DebugSendString("BEFORE init_data\n"); /*OS*/
    if (init_data() < 0)
        return(-1);

    /* MEMORY INITIALIZATION - initialize routing table, interface structure,   */
    /* create pools (put element of array into linked list) etc.                */
    OS_DebugSendString("BEFORE os_memory_init\n"); /*OS*/
    if (!os_memory_init(restart))
        return(-1);

    /* RESOURCE INITIALIZAION - spawn tasks, create semaphores and signals   */
    if (!restart)
    {
        OS_DebugSendString("BEFORE ks_resource_init\n"); /*OS*/
        if (!ks_resource_init())
        {
            DEBUG_ERROR("ERROR(RTIP): xn_rtip_init() - resource initialization failed",
                NOVAR, 0, 0);
            return(set_errno(ERSCINITFAIL));
        }
    }

#if (INCLUDE_DNS)
    OS_DebugSendString("BEFORE init_dns\n"); /*OS*/
    init_dns();
#endif

    return(0);
}

/* ********************************************************************      */
/* init_data() - initialize all RTIP data                                    */
/*                                                                           */
/*   Initializes all RTIP data.  Called by xn_rtip_init and xn_rtip_restart. */
/*                                                                           */
/*   NOTE: this routine is called before resources are initialized so        */
/*         it cannot use any resources (semaphores, signals etc)             */
/*                                                                           */
/*   Returns nothing                                                         */
/*                                                                           */
int init_data(void)     /*__fn__*/
{
int i;
P_INIT_FNCS p_init_fnc;

    OS_DebugSendString("ENTERED init_data\n"); /*OS*/


    /* **********************************************************    */
#if (INCLUDE_RARP)
    rarp_depth = 0;
#endif
#if (INCLUDE_BOOTP)
    bootp_depth = 0;
#endif

    /* **********************************************************    */
    /* IP DATA                                                       */
    ipsequence = 1;             /*  ip packet numbers start here  */
                                /*  and increment   */
    CFG_IP_TTL = _CFG_IP_TTL;   /*  time-to-live for IP packets */

#    if (INCLUDE_ROUTER)
        ip_forwarding = 1;      /* enable */
#    else
        ip_forwarding = 2;      /* disable */
#    endif

#if (INCLUDE_FRAG)
    max_frag_size = 0xffffU;
#endif

    /* **********************************************************    */
    /* Input ethernet Packet filtering                               */
    allow_udp_broadcasts = 1;

    /* **********************************************************    */
    /* ROUTING TABLE                                                 */
    rt_init();

    /* **********************************************************    */
    /* RIP                                                           */
#if (INCLUDE_RIP)
    _rip_init_data();
#endif

    /* **********************************************************    */
    /* OS DATA                                                       */
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
    root_dcu      = (POS_LIST)0;
    for (i = 0; i < CFG_NUM_FREELISTS; i++)
    {
        root_dcu_array[i] = 0;
        dcu_size_array[i] = 0;

    }
#if (INCLUDE_NO_DCU_BLOCK)
    ports_blocked_for_dcu = (POS_LIST)0;
#endif

    lowest_free_packets = 0;
    current_free_packets = 0;
#else   /* !INCLUDE_MALLOC_DCU_AS_NEEDED */
    current_alloc_packets = 0;
    highest_alloc_packets = 0;
#endif  /* !INCLUDE_MALLOC_DCU_AS_NEEDED */

#if (INCLUDE_UDP && !INCLUDE_MALLOC_PORTS)
    root_udp_port = 0;
#if (INCLUDE_UDP_PORT_CACHE)
    cached_udp_port = 0;
#endif

#endif

#if (INCLUDE_TCP && !INCLUDE_MALLOC_PORTS)
    root_tcp_port = 0;
#if (INCLUDE_TCP_PORT_CACHE)
    cached_tcp_port = 0;
#endif

#endif

    /* **********************************************************    */
    /* OS PORT DATA                                                  */
    static_iface_no = 0;
    static_interrupt_no = 0;

    /* **********************************************************    */
    /* OS INT DATA                                                   */
    in_irq = 0;

    /* **********************************************************    */
    /* TASK                                                          */
    /* **********************************************************    */
#if (INCLUDE_MALLOC_PORTS || INCLUDE_RUN_TIME_CONFIG)
    alloced_ports = (PANYPORT KS_FAR *)ks_malloc(TOTAL_PORTS,
                                                 sizeof(PANYPORT KS_FAR *),
                                                 ALLOCPORTS_MALLOC);
    if (!alloced_ports)
        return(FALSE);
#endif

    tc_memset((PFBYTE)alloced_ports, 0,
              TOTAL_PORTS*sizeof(PANYPORT KS_FAR *));

    /* **********************************************************    */
    /* TCP                                                           */
#if (INCLUDE_TCP)
    for (i=0; i < NUM_TCP_LISTS; i++)
        root_tcp_lists[i] = (POS_LIST)0;

    tcp_port_number = 0;
    default_tcp_options = IO_BLOCK_OPT | SO_NAGLE | SO_DELAYED_ACK;
                                        /* default is:                      */
                                        /*    - blocking mode               */
                                        /*    - keepalive disabled          */
                                        /*    - not linger on close         */
                                        /*    - read and write not shutdown */
                                        /*    - TCP copy mode               */
                                        /*    - do not reuse while TWAIT    */
                                        /*    - NAGLE algorithm enabled     */
                                        /*    - delayed ack enabled         */
                                        /*    - not streaming mode          */
#if (!INCLUDE_TCP_COPY)
    default_tcp_options |= SO_TCP_NO_COPY;
#endif          /* !INCLUDE_TCP_COPY */

#if (INCLUDE_TCP_OUT_OF_ORDER)
    free_ooo_lists = FALSE;
#endif
#if (INCLUDE_OOO_QUE_LIMIT)
    num_ooo_que = 0;     /* number of DCUs on the out of order list */
#endif
#endif      /* INCLUDE_TCP */

    /* **********************************************************    */
#if (INCLUDE_UDP)
    /* UDP    */
    for (i=0; i < NUM_UDP_LISTS; i++)
    {
        root_udp_lists[i] = (POS_LIST)0;
#if (INCLUDE_RAW)
        root_raw_lists[i] = (POS_LIST)0;
#endif
    }
    udp_port_number = 0;
#endif

#if (INCLUDE_UDP || INCLUDE_RAW)
    default_udp_options = IO_BLOCK_OPT | SO_UDPCKSUM_OUT | SO_UDPCKSUM_IN |
                          SO_MCAST_LOOP;
#endif

    /* **********************************************************    */
    /* ARP                                                           */
#if (INCLUDE_ARP)
    arpc_res_tmeout = ARPC_RES_TIMEOUT;
#else
    arpc_res_tmeout = 0;
#endif

    /* **********************************************************    */
    /* POLLOS.C                                                      */
    /* **********************************************************    */
#if (!INCLUDE_RUN_TIME_CONFIG)
    tc_memset((PFBYTE)ifaces, 0, sizeof(IFACE)*CFG_NIFACES);
#endif

    default_mcast_iface = -1;

    /* set up frequency of timer task   */
    timer_freq = CFG_TIMER_FREQ;

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
    tc_memset((PFBYTE)rs232_if_info_arry, 0,
            sizeof(rs232_if_info_arry));
#endif

#if (INCLUDE_PPP)
    ppp_escape_routine = 0;
    ppp_give_string_routine = 0;
#endif

    /* **********************************************************    */
    /* device table                                                  */
    total_devices = CFG_NUM_DEVICES;
    valid_device_entries = 0;
#if (BUILD_NEW_BINARY || INCLUDE_RUN_TIME_CONFIG)
    devices_ptr = (PDEVTABLE)ks_malloc(sizeof(struct _edevtable),
                                       total_devices, DEVTBL_MALLOC);
    if (!devices_ptr)
    {
        return(set_errno(EOUTAMEM));
    }
#else
    devices_ptr = devices;
#endif
    tc_memset(devices_ptr, 0, sizeof(EDEVTABLE) * total_devices);

    /* **********************************************************    */
    /* TABLES                                                        */
#    if (INCLUDE_FRAG)
       tc_memset((PFBYTE)frag_table, 0, sizeof(IP_FRAGLIST)*CFG_FRAG_TABLE_SIZE);
#    endif

#if (INCLUDE_ARP)
    /* clear the arpcache   */
    tc_memset((PFBYTE)tc_arpcache, 0, sizeof(ARPCACHE)*CFG_ARPCLEN);
#endif

#if (INCLUDE_ROUTING_TABLE)
    /* ROUTING TABLE   */
    tc_memset((PFBYTE)rt_table, 0, sizeof(ROUTE));
    root_rt_default = (POS_LIST)0;              /* default gateways */
#else
    tc_memset(default_gateway_address, 0, IP_ALEN);
#endif

#if (INCLUDE_ETH_BUT_NOARP)
    tc_memset((PFBYTE)ip2eth_table, 0, sizeof(IP2ETH)*CFG_NUM_IP2ETH_ADDR);
#endif

    /* **********************************************************    */
    /* TIMERS                                                        */
    ebs_one_sec_timers = (PTIMER)0;
    ebs_timers = (PTIMER)0;

    /* **********************************************************    */
    /* MALLOC CORE FOR BGET FOR FTP, WEB and TELNET Servers          */
#if (INCLUDE_BGET && INCLUDE_MALLOC)
	if (poolregistered == 0)				//JLA(IO), see declaration of poolregistered
	{
		poolregistered = 1;                 //JLA(IO)
	    bpool((PFVOID)context_core, CFG_CONTEXT_CORE);
	}
#endif

    /* **********************************************************     */
    /* INITIALIZE ADD-ONS (SNMP, WEB, FTP, TELNET, ETC)               */
    /* initialization routines were registered by application calling */
    /* xn_register_init_fnc()                                         */
    p_init_fnc = registered_fncs;
    while (p_init_fnc)
    {
        p_init_fnc->init_fnc();
        p_init_fnc = p_init_fnc->nxt_fnc;
    }

#if (INCLUDE_VFS)
    /* **********************************************************    */
    /* VFS                                                           */
    vf_api = NULL;
#endif

#if (INCLUDE_SSL)
    /* **********************************************************   */
    /* VSSL - Virtual Secure Socket Layer                           */
    vssl_api = NULL;
#endif

    return(0);      /* success */
}

#if (INCLUDE_RUN_TIME_CONFIG)
/* ********************************************************************   */
RTIP_BOOLEAN alloc_config_data(void)
{
int iface_no;
PIFACE pi;


#if (INCLUDE_ARP)
    /* ARP CACHE   */
    tc_arpcache = (PARPCACHE)0;
#endif

#if (INCLUDE_ETH_BUT_NOARP)
    /* IP to ethernet translation table;                                     */
    /* used for ethernet packets where translation is known by application,  */
    /* therefore, ARP is not necessary                                       */
    ip2eth_table = (PIP2ETH)0;
#endif

    frag_table = (PIP_FRAGLIST)0;

#if (INCLUDE_ROUTING_TABLE)
    /* ROUTING TABLE   */
    rt_table = (PROUTE)0;
#endif

    ifaces = (PIFACE)0;

    /* ********************************************************************   */
    /* TCP, UDP/RAW ports                                                     */
#if (!INCLUDE_MALLOC_PORTS)
#if (INCLUDE_TCP)
    tc_tcpp_pool = (PTCPPORT)ks_malloc(CFG_NTCPPORTS, sizeof(TCPPORT),
                                       TCPPORT_MALLOC);
    if (!tc_tcpp_pool)
        return(FALSE);
#endif      /* INCLUDE_TCP */

#if (INCLUDE_UDP)
    tc_udpp_pool = (PUDPPORT)ks_malloc(CFG_NUDPPORTS, sizeof(UDPPORT),
                                       UDPPORT_MALLOC);
    if (!tc_udpp_pool)
        return(FALSE);
#endif      /* INCLUDE_UDP */

#endif      /* !INCLUDE_MALLOC_PORTS */

    /* ********************************************************************   */
#if (INCLUDE_ARP && INCLUDE_RUN_TIME_CONFIG)
    /* ARP CACHE   */
    tc_arpcache = (PARPCACHE)ks_malloc(CFG_ARPCLEN, sizeof(ARPCACHE),
                                       UDPPORT_MALLOC);
    if (!tc_arpcache)
        return(FALSE);
#endif

#if (INCLUDE_ETH_BUT_NOARP)
    /* IP to ethernet translation table;                                     */
    /* used for ethernet packets where translation is known by application,  */
    /* therefore, ARP is not necessary                                       */
    ip2eth_table = (PIP2ETH)ks_malloc(CFG_NUM_IP2ETH_ADDR, sizeof(IP2ETH),
                                      IP2ETH_MALLOC);
    if (!ip2eth_table)
        return(FALSE);
#endif

    frag_table = (PIP_FRAGLIST)ks_malloc(CFG_FRAG_TABLE_SIZE,
                                         sizeof(IP_FRAGLIST),
                                         FRAGTABLE_MALLOC);
    if (!frag_table)
        return(FALSE);

#if (INCLUDE_ROUTING_TABLE && INCLUDE_RUN_TIME_CONFIG)
    /* ROUTING TABLE   */
    rt_table = (PROUTE)ks_malloc(CFG_RTSIZE, sizeof(ROUTE), RTTABLE_MALLOC);
    if (!rt_table)
        return(FALSE);
#endif

    ifaces = (PIFACE)ks_malloc(CFG_NIFACES, sizeof(IFACE), IFACES_MALLOC);
    if (!ifaces)
        return(FALSE);
    tc_memset((PFBYTE)ifaces, 0, sizeof(IFACE)*CFG_NIFACES);

    LOOP_THRU_IFACES(iface_no)
    {
        PI_FROM_OFF(pi, iface_no)

        pi->mcast.mclist_ip =
            (byte *)ks_malloc(CFG_MCLISTSIZE*IP_ALEN, sizeof(byte),
                              MCLISTIP_MALLOC);
        pi->mcast.mclist =
            (byte *)ks_malloc(CFG_MCLISTSIZE*ETH_ALEN, sizeof(byte),
                              MCLISTETH_MALLOC);
        pi->mcast.mcast_cnt =
            (int *)ks_malloc(CFG_MCLISTSIZE, sizeof(int), MCASTCNT_MALLOC);

        if (!pi->mcast.mclist_ip || !pi->mcast.mclist || !pi->mcast.mcast_cnt)
            return(FALSE);

#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
        pi->mcast.report_timer =
            (dword *)ks_malloc(CFG_MCLISTSIZE, sizeof(int), MCASTREPORT_MALLOC);
        if (!pi->mcast.report_timer)
            return(FALSE);
#endif
#if (INCLUDE_IGMP_V2)
        pi->mcast.last_host_toreply =
            (byte *)ks_malloc(CFG_MCLISTSIZE, sizeof(int), MCASTLHTR_MALLOC);
        if (!pi->mcast.last_host_toreply)
            return(FALSE);
#endif
    }

    return(TRUE);
}

void free_config_data()
{
int iface_no;
PIFACE pi;

    /* free from alloc_config_data   */
#if (INCLUDE_ARP)
    /* ARP CACHE   */
    if (tc_arpcache)
        ks_free((PFBYTE)tc_arpcache, CFG_ARPCLEN, sizeof(ARPCACHE));
#endif

#if (INCLUDE_ETH_BUT_NOARP)
    /* IP to ethernet translation table;                                     */
    /* used for ethernet packets where translation is known by application,  */
    /* therefore, ARP is not necessary                                       */
    if (ip2eth_table)
        ks_free((PFBYTE)ip2eth_table, CFG_NUM_IP2ETH_ADDR, sizeof(IP2ETH));
#endif

    if (frag_table)
        ks_free((PFBYTE)frag_table, CFG_FRAG_TABLE_SIZE, sizeof(IP_FRAGLIST));

#if (INCLUDE_ARP)
    /* ROUTING TABLE   */
    if (rt_table)
        ks_free((PFBYTE)rt_table, CFG_RTSIZE, sizeof(ROUTE));
#endif

#if (INCLUDE_MALLOC_PORTS || INCLUDE_RUN_TIME_CONFIG)
    if (alloced_ports)
        ks_free((PFBYTE)alloced_ports, TOTAL_PORTS, sizeof(PANYPORT KS_FAR *));
#endif

    if (ifaces)
    {
        LOOP_THRU_IFACES(iface_no)
        {
            PI_FROM_OFF(pi, iface_no)

            if (pi->mcast.mclist_ip)
                ks_free((PFBYTE)pi->mcast.mclist_ip, CFG_MCLISTSIZE*IP_ALEN, sizeof(byte));
            if (pi->mcast.mclist)
                ks_free((PFBYTE)pi->mcast.mclist, CFG_MCLISTSIZE*ETH_ALEN, sizeof(byte));
            if (pi->mcast.mcast_cnt)
                ks_free((PFBYTE)pi->mcast.mcast_cnt, CFG_MCLISTSIZE, sizeof(int));

#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
            if (pi->mcast.report_timer)
                ks_free((PFBYTE)pi->mcast.report_timer, CFG_MCLISTSIZE, sizeof(int));
#endif
#if (INCLUDE_IGMP_V2)
            if (pi->mcast.last_host_toreply)
                ks_free((PFBYTE)pi->mcast.last_host_toreply, CFG_MCLISTSIZE, sizeof(int));
#endif
        }

        ks_free((PFBYTE)ifaces, CFG_NIFACES, sizeof(IFACE));
    }
}

void init_config_data(void)
{
}
#endif

/* ********************************************************************    */
/* REGISTER INIT FNCS                                                      */
/* ********************************************************************    */
/* ********************************************************************    */
/* xn_register_init_fnc() -  Register initialization functions             */
/*                                                                         */
/* Summary:                                                                */
/*   #include "rtipapi.h"                                                  */
/*                                                                         */
/*   void xn_register_init_fnc(init_fnc, fnc)                              */
/*      P_INIT_FNCS init_fnc   - structure used internally                 */
/*                               does not need to be initialized by caller */
/*      REGISTER_PROC fnc      - function to be registered                 */
/*                                                                         */
/* Description:                                                            */
/*   Performs the following:                                               */
/*     Register initialization functions.  The functions registered        */
/*     will be called by xn_rtip_init.                                     */
/*                                                                         */
/*     Needs to be called before xn_rtip_init for each add-on which        */
/*     has an initialization routine.  The add-ons which require           */
/*     initialization routines include SNMP, DHCP client and Server,       */
/*     FTP server, WEB server and TELNET server.  The packet driver        */
/*     as well as LOOPBACK driver need to be registered as well.           */
/*                                                                         */
/*   For more details see the RTIP Manual.                                 */
/*                                                                         */
/* Returns:                                                                */
/*   Nothing                                                               */
/*                                                                         */

void xn_register_init_fnc(P_INIT_FNCS init_fnc, REGISTER_PROC fnc)
{
    init_fnc->init_fnc = fnc;

    /* put initialization routine at the beginning of the list   */
    init_fnc->nxt_fnc = registered_fncs;
    registered_fncs = init_fnc;
}

/* ********************************************************************     */
/* xn_rtip_exit() -  Restores environment                                   */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_rtip_exit()                                                     */
/*                                                                          */
/* Description:                                                             */
/*   Performs the following:                                                */
/*     Restores the interrupt vectors to state prior to opening interfaces. */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Zero is returned on success. Otherwise -1                              */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int xn_rtip_exit(void)                                            /*__fn__*/
{
    return(_xn_rtip_exit(FALSE));
}

int _xn_rtip_exit(RTIP_BOOLEAN free_resources)                    /*__fn__*/
{
int    iface_no;
PIFACE pi;
int    total_wait;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    ks_disable();
    ks_restore_interrupts();
    ks_enable();

#if (USE_KEYSCAN)
    keyscan_on = 0;
#endif

#if (INCLUDE_MALLOC_DCU_INIT || INCLUDE_MALLOC_PORTS || BUILD_NEW_BINARY)
    if (free_resources)
    {
        /* free dynamic memory   */
        os_memory_free();

#        if (INCLUDE_MALLOC_DCU_INIT && INCLUDE_TRK_PKTS)
            free_dcu_table();
#        endif
    }
#endif      /* INCLUDE_MALLOC_DCU_INIT || BUILD_NEW_BINARY */

    /* rtip is not initailized   */
    rtip_exited = TRUE;

    LOOP_THRU_IFACES(iface_no)
    {
        PI_FROM_OFF(pi, iface_no)

        total_wait = 0;

        /* wait until interfaces closes which were started are done   */
        OS_CLAIM_IFACE(pi, DEVICE_CLAIM_IFACE)
        while (pi->iface_flags & CLOSE_IN_PROGRESS)
        {
            OS_RELEASE_IFACE(pi)

            iface_no = 0;
            ks_sleep(2);

            /* wait at most 3 secs - tbd   */
            total_wait += 2;
            if (total_wait < (int)(ks_ticks_p_sec() * 3))
                continue;
            else
                break;
        }
        OS_RELEASE_IFACE(pi)
    }

#if (INCLUDE_RUN_TIME_CONFIG)
    if (free_resources)
    {
        /* must be done last   */
        free_config_data();
    }
#else
    ARGSUSED_INT(free_resources)
#endif

    return(0);
}

/* ********************************************************************        */
/* xn_rarp() -  Send a rarp request to look up my IP number                    */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "rtipapi.h"                                                      */
/*                                                                             */
/*   int xn_rarp(iface_no)                                                     */
/*      int  iface_no  - Interface number (see rtip.h)                         */
/*                                                                             */
/* Description:                                                                */
/*   In a rommed system it may not be practical to hard code the internet      */
/*   address for a machine. RARP will broadcast a RARP request which will      */
/*   prompt any RARP servers on the net to reply with the machines IP address. */
/*   By default this capability is not compiled in. If you require RARP you    */
/*   should set INCLUDE_RARP to 1 in the configuration header file.            */
/*                                                                             */
/*   An unsupported RARP server capability is provide with this package.       */
/*   you may modify it to work in your environment if you wish (see arp.c)     */
/*                                                                             */
/*   Note: xn_bootp() provides a similar/superior capability.                  */
/*                                                                             */
/*   For more details see the RTIP Manual.                                     */
/*                                                                             */

#if (INCLUDE_RARP)

int xn_rarp(int iface_no)                             /*__fn__*/
{
PIFACE pi;
int i;
int ret_val;

    RTIP_API_ENTER(API_XN_RARP)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_RARP)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
    {
        RTIP_API_EXIT(API_XN_RARP)
        return(-1);
    }

    /* xn_rarp is non-reentrant; check for reentry error   */
    if (rarp_depth)
    {
        RTIP_API_EXIT(API_XN_RARP)
        return(set_errno(ERENTRANT));
    }
    rarp_depth++;

    /* set null address to the interface so nxt_bc_iface will work   */
    set_ip(iface_no, (PFBYTE)ip_nulladdr, (PFCBYTE)0);

    for (i = 0; i < CFG_RARP_TRIES; i++)
    {
        /* bind the signal to the application task so ip task knows who   */
        /* to signal when response is in                                  */
        OS_BIND_RARP_SIGNAL(pi);

        /* send RARP msg and wait for it to be sent over wire   */
        ret_val = tc_rarp_send(pi);
        if (!ret_val)
        {
            if (OS_TEST_RARP_SIGNAL(pi, (word)(CFG_RARP_TMO*ks_ticks_p_sec())))
            {
                rarp_depth--;
                RTIP_API_EXIT(API_XN_RARP)
                return(0);
            }
            else
            {
                ret_val = ETIMEDOUT;
                DEBUG_ERROR("ERROR(RTIP): xn_rarp() - rarp timed out",
                    NOVAR, 0, 0);
            }
        }
    }

    DEBUG_ERROR("ERROR(RTIP): xn_rarp() - no response to rarp - quitting",
        NOVAR, 0, 0);
    rt_del_from_iface(pi);
    rarp_depth--;
    RTIP_API_EXIT(API_XN_RARP)
    return(set_errno(ret_val));
}
#endif /* INCLUDE_RARP */



#if (INCLUDE_BOOTP)
/* ********************************************************************      */
/* xn_bootp() - Call a bootp server to get this node's IP address            */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   int xn_bootp(iface_no)                                                  */
/*      int iface_no            - Interface number. (see rtip.h)             */
/*                                                                           */
/* Description:                                                              */
/*   This routine implements a Stanford BOOTP protocol client over UDP. It   */
/*   may be called after the interface has been initialized and the network  */
/*   daemons are running. It sets the interface's IP address and optionally  */
/*   its IP mask based on the return value. All returned bootp values are    */
/*   accessible if needed.                                                   */
/*                                                                           */
/*   This code will not be compiled if INCLUDE_BOOTP is not defined in       */
/*   xnconf.h                                                                */
/*                                                                           */
/*   For more details see the RTIP Manual.                                   */
/*                                                                           */
/* Returns:                                                                  */
/*   0 if a bootp server responded, -1 otherwise.                            */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_bootp(int iface_no)                         /*__fn__*/
{
PBOOTR   presults;      /* Parse results */
int      ret_val;

    ret_val = -1;

    /* Allocate a packet we can use to store the results of parsing             */
    /* a BOOTP response. For now the scope of this packet will be local to this */
    /* routine. If later it is convenient to make the scope global we can do    */
    /* that                                                                     */
    presults = (PBOOTR)ks_malloc(sizeof(*presults), 1, BOOTP_MALLOC);
    if (!presults)
    {
        set_errno(EOUTAMEM);
        goto ex_and_cl_tab;
    }

    tc_memset((PFBYTE)presults, 0, sizeof(*presults));

#if (INCLUDE_RUN_TIME_CONFIG)
    presults->gateways = ks_malloc(CFG_BOOTP_RTSIZE*IP_ALEN,
                                   sizeof(byte), BOOTP_RT1_MALLOC);
    presults->domservers = ks_malloc(CFG_MAX_DNS_SRV*IP_ALEN,
                                     sizeof(byte), BOOTP_DNS_MALLOC);
    presults->static_routes = (STATIC_ROUTE *)
        ks_malloc(CFG_BOOTP_RTSIZE*IP_ALEN,
                  sizeof(STATIC_ROUTE), BOOTP_RT2_MALLOC);
    if (!presults->gateways || !presults->domservers ||
        !presults->static_routes)
    {
        /* Free the storage for the results packet   */
        if (presults->gateways)
            ks_free(presults->gateways, CFG_BOOTP_RTSIZE*IP_ALEN, sizeof(byte));
        if (presults->domservers)
            ks_free(presults->domservers, CFG_MAX_DNS_SRV*IP_ALEN, sizeof(byte));
        if (presults->static_routes)
            ks_free((PFBYTE)presults->static_routes, CFG_BOOTP_RTSIZE*IP_ALEN,
                  sizeof(STATIC_ROUTE));
        set_errno(EOUTAMEM);
        goto ex_and_cl_tab;
    }
#endif

    ret_val = xn_bootp_res(iface_no, presults);

    if (presults)
        ks_free((PFBYTE)presults, 1, sizeof(*presults));

ex_and_cl_tab:
    return(ret_val);
}

/* ********************************************************************      */
/* xn_bootp_res() - Call a bootp server to get this node's IP address        */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   int xn_bootp_res(iface_no, presults)                                    */
/*      int    iface_no      - Interface number. (see rtip.h)                */
/*      PBOOTR presults      - Parse results                                 */
/*                                                                           */
/* Description:                                                              */
/*   This routine implements a Stanford BOOTP protocol client over UDP. It   */
/*   may be called after the interface has been initialized and the network  */
/*   daemons are running. It sets the interface's IP address and optionally  */
/*   its IP mask based on the return value. All returned bootp values are    */
/*   accessible if needed.                                                   */
/*                                                                           */
/*   This code will not be compiled if INCLUDE_BOOTP is not defined in       */
/*   xnconf.h                                                                */
/*                                                                           */
/*   For more details see the RTIP Manual.                                   */
/*                                                                           */
/* Returns:                                                                  */
/*   0 if a bootp server responded, -1 otherwise.                            */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_bootp_res(int iface_no, PBOOTR presults)               /*__fn__*/
{
int      socket_no;
PUDPPORT pport;
PBOOTPKT pb;
word     delay;
int      x;
int      ret_val, rval;
dword    bootp_xid;
PIFACE   pi;
struct sockaddr_in sin;
int     size;
DCU     msg;
#if (INCLUDE_DNS)
dword srv_list[CFG_MAX_DNS_SRV];
#endif
#if (!INCLUDE_PKT_API)
int     nread;
#endif

    RTIP_API_ENTER(API_XN_BOOTP)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_BOOTP)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
    {
        RTIP_API_EXIT(API_XN_BOOTP)
        return(-1);
    }

    /* xn_bootp is non-reentrant; check for reentry error   */
    if (bootp_depth)
    {
        RTIP_API_EXIT(API_XN_BOOTP)
        return(set_errno(ERENTRANT));
    }
    bootp_depth++;

    ret_val = -1;   /* failure */

    /* set null address to the interface so nxt_bc_iface() will work   */
    set_ip(iface_no, (PFBYTE)ip_nulladdr, (PFCBYTE)0);

    /* Grab a udp port.    */
    socket_no = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_no < 0)
    {
        goto ex_and_cl_tab;
    }

    /* get port structure   */
    pport = (PUDPPORT)sock_to_port(socket_no);
    if (pport == (PUDPPORT)0)
    {
        goto ex_and_cl_tab;  /* should never happen */
    }

    /* make sure only broadcast to interface passed to xn_dhcp()   */
    pport->broadcast_pi = pi;

    /* Set my port to BOOTP client (binds to wild since by ip is 0)   */
    sin.sin_family = AF_INET;
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS
    tc_mv4((PFBYTE)&sin.sin_addr, pi->addr.my_ip_addr, 4);
#else
    tc_mv4((PFBYTE)&sin.sin_addr.s_addr, pi->addr.my_ip_addr, 4);
#endif

    sin.sin_port = hs2net((word)IPPORT_BOOTPC);
    if (bind(socket_no, (PCSOCKADDR)&sin, sizeof(sin)) < 0)
        goto ex_and_cl_s_tab;   /* bind set errno */

    /* Set his port to BOOTP server. His ip addr to broadcast (ff,ff,ff,ff)   */
    /* Connect does no IO for UDP                                             */
    sin.sin_family = AF_INET;
    tc_mv4((PFBYTE)&sin.sin_addr, ip_broadaddr, 4);
    sin.sin_port = hs2net((word)IPPORT_BOOTPS);
    if (connect(socket_no, (PCSOCKADDR)&sin, sizeof(sin)) < 0)
    {
        goto ex_and_cl_s_tab;   /* bind set errno */
    }

    bootp_xid = ks_get_ticks();   /* get a unique transaction ID  */

    /* This should only happen when codeview is running   */
    if (!bootp_xid)
        bootp_xid = 1139;

    /* Start with a delay between 1 and 4 seconds   */
#if (INCLUDE_PKT_API)
    delay = (word) ((ks_get_ticks()) % (4*ks_ticks_p_sec()));
    if (!delay)
        delay = (word) (ks_ticks_p_sec()/2);
#else
    delay = (word)ks_get_ticks() % 4;
    if (!delay)
        delay = (word)4;
#endif

    for (x=0; x <= CFG_BOOTP_RETRIES; x++)
    {
        /* Grap a message to send bootp request in.   */
#if (INCLUDE_PKT_API)
        TOTAL_UDP_HLEN_SIZE(size, pport, pi, 0)
#else
        size = 0;
#endif      /* INCLUDE_PKT_API */

        size += sizeof(*pb);

        msg = xn_pkt_alloc(size, FALSE);
        if (!msg)
        {
            set_errno(ENOPKTS);
            goto ex_and_cl_s_tab;   /* bind set errno */
        }
#if (INCLUDE_PKT_API)
        pb = (PBOOTPKT)xn_pkt_data_pointer(socket_no, msg, OUTPUT_PKT);
#else
        pb = (PBOOTPKT)DCUTODATA(msg);
#endif

        tc_memset((PFBYTE)pb, 0, sizeof(*pb));
        pb->bp_op = BOOTREQUEST;
        pb->bp_htype = 1;     /* hardware type 1 is ethernet. (should come from iface) */
        pb->bp_hlen = sizeof(pport->ap.iface->addr.my_hw_addr); /* address length */
        pb->bp_xid = bootp_xid;
        pb->bp_secs = 1;

        pport->ap.iface = pi;       /* port iface to point to pi  */

        /* Copy my ethernet address to the boot structure   */
        tc_movebytes((PFBYTE)pb->bp_chaddr,
                     (PFBYTE)pport->ap.iface->addr.my_hw_addr,
                     sizeof(pport->ap.iface->addr.my_hw_addr) );

        /* **************************************************   */
        /* Send the message off                                 */
#if (INCLUDE_PKT_API)
        rval = xn_pkt_send(socket_no, msg, sizeof(*pb), RTIP_INF);
        os_free_packet(msg);

        /* if send failed   */
        if (rval)
        {
            ret_val = -1;
            goto ex_and_cl_s_tab;   /* xn_pkt_send set errno */
        }

#else
        rval = send(socket_no, (PFCCHAR)pb, sizeof(*pb), 0);
        if (rval != sizeof(*pb))
        {
            os_free_packet(msg);
        /* if send failed   */
            ret_val = -1;
            goto ex_and_cl_s_tab;   /* xn_pkt_send set errno */
        }
#endif
#if (INCLUDE_PKT_API)
        msg = xn_pkt_recv(socket_no, delay);
        if (msg)
#else
        nread = 0;
        if (do_read_select(socket_no, delay))
            nread = recv(socket_no, (PFCHAR)pb, size, 0);
        if (nread)
#endif

        /* If no reply. Adjust the wait count. Double until 60 seconds.    */
        /* Then use random values < 60 secs                                */
        {
            /* **************************************************   */
            /* process the reply                                    */
#if (INCLUDE_PKT_API)
            pb = (PBOOTPKT)xn_pkt_data_pointer(socket_no, msg, INPUT_PKT);
#endif

            if ( (pb->bp_xid==bootp_xid) && (pb->bp_op==BOOTREPLY) &&
                tc_comparen((PFBYTE)pb->bp_chaddr,
                            (PFBYTE)pport->ap.iface->addr.my_hw_addr,
                             sizeof(pport->ap.iface->addr.my_hw_addr) ) )
            {
                parse_bootpacket(pb, presults);
                os_free_packet(msg);    /* Free the input packet */
                ret_val = 0;
                break;                  /* break out of retry loop */
            }
        }       /* end of if read worked */
        os_free_packet(msg);        /* Free the input packet */

        /* In debug mode show that we're not succeeding   */
        DEBUG_ERROR("ERROR(RTIP): xn_bootp() - BOOTP timed out - Retry",
            NOVAR, 0, 0);
        delay <<= 1;
        if (delay > (word) (60*ks_ticks_p_sec()))
            delay = (word) (ks_get_ticks() % (60*ks_ticks_p_sec()));
    }       /* end of retry loop */

    /* **************************************************   */
    /* if succeeded, offload the results                    */
    if (!ret_val)
    {
        /* IP ADDRESS, MASK   */
        if (presults->have_mask)
            set_ip(iface_no,  presults->my_ip_address,
                   (PFCBYTE)presults->my_net_mask);
        else
            set_ip(iface_no,  presults->my_ip_address, (PFCBYTE)0);

        /* BOOTP SERVERS ADDRESS   */
        tc_mv4(pi->addr.bootp_ip_addr, presults->bootp_ip_address, 4);

        /* BOOTP FILENAME   */
        tc_movebytes(pi->addr.bootp_file_name, presults->bootp_file_name,
                     BOOTP_FILE_LEN);

        /* GATEWAYS                                                     */
        /* install one gateway as default                               */
        /* NOTE: mask does not matter for default; metric=1; ttl=RT_INF */
        if (presults->n_gateways)
        {
            for (x=0; x < presults->n_gateways; x++)
            {
                if (!rt_add(RT_DEFAULT, (PFBYTE)ip_ffaddr,
                            &(presults->gateways[x*IP_ALEN]),
                            RT_USEIFACEMETRIC,
                            pi->ctrl.index, RT_INF, 0, SNMP_OTHER))
                {
                    break;  /* only add one; there are in order of preference */
                }
            }
        }

#if (INCLUDE_DNS)
        /* DNS SERVERS   */
        if (presults->n_domservers)  /* Number of domservers in vendor specific */
        {
            if (presults->n_domservers > CFG_MAX_DNS_SRV)
                presults->n_domservers = CFG_MAX_DNS_SRV;
            for (x=0; x < presults->n_domservers; x++)
            {
                tc_mv4((PFBYTE)&srv_list[x],
                       &(presults->domservers[x*IP_ALEN]), 4);
            }
            xn_set_server_list(srv_list, presults->n_domservers);
        }
#endif
    }
    else
    {
        DEBUG_ERROR("ERROR(RTIP): xn_bootp() - BOOTP no response - quit trying",
            NOVAR, 0, 0);
    }

ex_and_cl_s_tab:
    /* Close the socket   */
    closesocket(socket_no);

ex_and_cl_tab:
    if (ret_val)
        rt_del_from_iface(pi); /*  clear the routing table */
    bootp_depth--;
    RTIP_API_EXIT(API_XN_BOOTP)
    return(ret_val);
}


/* ********************************************************************      */
/* Bootp routines : from the Clarkson 2.2 version of NCSA Telnet.            */
/* Thanks to Brad Clements for implementing this!                            */
/*                                                                           */
/* bootp routines - These routines are based on the stanford/clarkson        */
/*                  bootp code. Originally developed at Stanford University. */
/*                                                                           */
/* Bootp is a UDP based protocol that determines the clients IP address and  */
/* gateway information etc.                                                  */
/*                                                                           */
/* Bootp is a stand alone module that will only be linked in if xuxn_bootp   */
/* is called.                                                                */
/*                                                                           */

/* parse an incoming bootp packet    */
void parse_bootpacket(PBOOTPKT pb, PBOOTR presults)
{
int    x,len;
PFBYTE c;
    /* Save my ip address   */
    tc_mv4(presults->my_ip_address, pb->bp_yiaddr, 4);

    /* Save servers ip address   */
    tc_mv4(presults->bootp_ip_address, pb->bp_siaddr, 4);

    /* save file name   */
    tc_movebytes(presults->bootp_file_name, pb->bp_file, BOOTP_FILE_LEN);

    /* Save default gateway address. This is superceded if gateways are   */
    /* enclosed later                                                     */
    tc_mv4(presults->default_gateway, pb->bp_giaddr, 4);

    if (tc_cmp4((PFBYTE)pb->bp_vend, bootp_VM_RFC1048, 4))
    {
        c = pb->bp_vend + 4;
        while ((*c!=255) && ((c-pb->bp_vend)<64))
        {
            switch(*c)
            {
                case 0:        /* nop pad */
                    c++;
                    break;

                case 1:        /* subnet mask  */
                    len =* (c+1);
                    c += 2;
                    tc_mv4(presults->my_net_mask, c, 4);
                    c += len;
                    presults->have_mask = TRUE;
                    break;

                case 2:        /* time offset  */
                    c += *(c+1)+2;
                    break;

                case 3:        /* gateways  */
                    len =* (c+1);     /* len is number of gw * 4 */
                    presults->n_gateways = 0;      /* Number of gateways in vendor specific */
                    c += 2;
                    for (x=0; x < (len>>2); x++)
                    {
                        /* discard additional gateways   */
                        if (x < CFG_BOOTP_RTSIZE)
                        {
                            presults->n_gateways++;
                            tc_mv4(&(presults->gateways[x*IP_ALEN]), c, IP_ALEN);
                        }
                        DEBUG_ASSERT(x < CFG_BOOTP_RTSIZE,
                                     "parse_bootpacket - discarding gateway: ",
                                     IPADDR, c, 0);
                        c += 4;
                    }
                    break;

                case 4:        /* time servers  */
                case 5:        /* IEN=116 name server  */
                    c += *(c+1)+2;
                    break;

                case 6:        /* domain name server  */
                    len =* (c+1);     /* len is number of DNS * 4 */
                    presults->n_domservers = 0;  /* Number of domservers in vendor specific */
                    c += 2;
                    for (x=0; x < (len>>2); x++)
                    {
                        /* discard additional DNS servers   */
                        if (x < CFG_MAX_DNS_SRV)
                        {
                            presults->n_domservers++;
                            tc_mv4(&(presults->domservers[x*IP_ALEN]), c, 4);
                        }
                        DEBUG_ASSERT(x < CFG_MAX_DNS_SRV,
                                     "parse_bootpacket - discarding dns: ",
                                     IPADDR, c, 0);
                        c += 4;
                    }
                    break;

                case 7:        /* log server  */
                case 8:        /* cookie server */
                case 9:        /* lpr server  */
                case 10:       /* impress server */
                case 11:       /* rlp server  */
                    c += *(c+1)+2;
                    break;

                case 12:        /* client host name     */
                    len = *(c+1);
                    if (len <= CFG_DNS_NAME_LEN-1)
                    {
                        tc_movebytes(presults->my_host_name,c+2, len);
                        presults->my_host_name[len] = 0;
                    }
                    else
                        presults->my_host_name[0] = 0;
                    c += len + 2;
                    break;

                /* assigning default domain name   */
                case 15:        /* default domain name  */
                    len = *(c+1);
                    if (len > CFG_DNS_NAME_LEN-3)
                    {
                        DEBUG_ERROR("parse_bootpacket: CFG_DNS_NAME_LEN is too small",
                            NOVAR, 0, 0);
                    }
                    else
                    {
                        tc_movebytes(presults->my_domain_name, c+2, len);
                        presults->my_domain_name[len] = 0;
                    }
                    c += len + 2;
                    break;

                /* assigning static routes                              */
                /* each static route entry consists of (dest_ip, gw_ip) */
                case 33:
                    len = *(c+1);
                    presults->n_static_routes = len/8;
                    if (presults->n_static_routes > CFG_BOOTP_RTSIZE)
                    {
                        DEBUG_ERROR("parse_bootpacket: dropping static route",
                            NOVAR, 0, 0);
                    }
                    c += 2;
                    for (x=0; x < presults->n_static_routes; x++)
                    {
                        if (x < CFG_BOOTP_RTSIZE)
                        {
                            tc_memmove(presults->static_routes[x].dest_ip,
                                       c, 4);
                            tc_memmove(presults->static_routes[x].gw_ip,
                                       c+4, 4);
                        }
                        c += 8;
                    }
                    if (presults->n_static_routes > CFG_BOOTP_RTSIZE)
                    {
                        presults->n_static_routes = CFG_BOOTP_RTSIZE;
                    }
                    break;

                case 255:
                    break;

                default:
                    c += *(c+1)+2;
                    break;
            }                /* end switch */
        }                    /* end while    */
    }                        /* end if comparen */
}

#endif /* INCLUDE_BOOTP */



#if (INCLUDE_IPV4 && INCLUDE_PING)
/* ********************************************************************                */
/*  xn_ping() - "PING" a host and wait for it to reply                                 */
/*                                                                                     */
/* Summary:                                                                            */
/*   #include "rtipapi.h"                                                              */
/*                                                                                     */
/*   int xn_ping(sequence, len, host, route_type, route_info, wait_count)              */
/*     int sequence            - Integer sent to the host. Reply should contain this # */
/*     int len                 - Number of bytes to send in the packet (helps measure  */
/*                               throughput)                                           */
/*     int ttl                 - Time to live to use for IP header.  If 0,             */
/*                               the default value will be used.                       */
/*     PFBYTE host             - IP address of the host. 4 unsigned chars              */
/*     int route_type          - IP options (SO_RECORD_ROUTE_OPTION,                   */
/*                                           SO_LOOSE_ROUTE_OPTION,                    */
/*                                           SO_STRICT_ROUTE_OPTION)                   */
/*     PROUTE_INFO route_info  - specifies routing information, i.e.                   */
/*                               addresses for LOOSE and STRICT routes                 */
/*                               and size for all routing options;                     */
/*                               also returns routing information for all              */
/*                               routing options                                       */
/*     unsigned int wait_count - Wait this long for a reply                            */
/*     long *elapsed_msec      - Elapsed time for PING request/response                */
/*                                                                                     */
/* Description:                                                                        */
/*                                                                                     */
/*   This routine uses the ICMP layer to "ping" another network node. If the           */
/*   other node is up it will return the packet to this node. The user                 */
/*   supplies a sequence number which will be echoed back from the other               */
/*   node. The user also supplies the requested packet length.                         */
/*                                                                                     */
/*   For more details see the RTIP Manual.                                             */
/*                                                                                     */
/* Returns:                                                                            */
/*  n  - If the other side replied with the correct sequence number                    */
/*       within wait count ticks where n is the length of route                        */
/*       information (0-route_len)                                                     */
/*  -1 - Otherwise                                                                     */
/*                                                                                     */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set            */
/*   by this function.  For error values returned by this function see                 */
/*   RTIP Reference Manual.                                                            */
/*                                                                                     */

#if (RTIP_VERSION >= 26)
int _xn_ping(int sequence, int len, int ttl, PFBYTE host,   /* __fn__ */
            int route_type, PROUTE_INFO route_info, /* __fn__ */
            unsigned int wait_count,                /* __fn__ */
            long *elapsed_msec, RTIP_BOOLEAN send_802_2) /* __fn__ */
#else
int xn_ping(int sequence, int len, int ttl, PFBYTE host,    /* __fn__ */
            int route_type, PROUTE_INFO route_info, /* __fn__ */
            unsigned int wait_count,                /* __fn__ */
            long *elapsed_msec) /* __fn__ */
#endif
{
#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
PFBYTE    pb;
int       exp_route_type;
int       optionlen;
#endif
int       sock;
PANYPORT  port;
DCU       msg_reply;
PIPPKT    pip;
dword     start_time;
word      msec_p_tick;
int       orig_route_len;

    RTIP_API_ENTER(API_XN_PING)

#if (!INCLUDE_802_2)
    ARGSUSED_INT(send_802_2)
#endif

    start_time = ks_get_ticks();

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_PING)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

#if (INCLUDE_ERROR_CHECKING)
    if (len < 0)
        len = 0;
#endif

#if (INCLUDE_UDP)
    port = (PANYPORT)os_alloc_udpport();
#elif (INCLUDE_TCP)
    port = (PANYPORT)os_alloc_tcpport();
#else
#error: INCLUDE_TCP or INCLUDE_UDP has to be on for PING
#endif

    if (!port)
    {
        RTIP_API_EXIT(API_XN_PING)
        return(set_errno(EMFILE));
    }

    port->ip_ttl = -1;        /* set thru socket option (TCP, UDP and RAW) */
    sock = port->ctrl.index;

    if (route_type && route_info && route_info->route_addresses)
    {
#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
        optionlen = sizeof(ROUTE_INFO);
        if (route_type == SO_RECORD_ROUTE_OPTION)
            optionlen = sizeof(int);
        if ( xn_ip_set_option(sock, route_type, (PFCCHAR)route_info,
                              optionlen) )
        {
            DEBUG_ERROR("xn_ping_option: ip_set_option failed", NOVAR, 0, 0);
#if (INCLUDE_UDP)
            /* NOTE: do not need to deference port in arp table since   */
            /*       port structure is not passed to tc_netwrite()      */
            port->list_type = FREE_LIST;
            os_free_udpport((PUDPPORT)port);
#elif (INCLUDE_TCP)
            free_tcpport_avail((PTCPPORT)port, FALSE);
#endif
            RTIP_API_EXIT(API_XN_PING)
            return(-1);
        }

#else       /* INCLUDE_IPV4 && INCLUDE_IP_OPTIONS */
        DEBUG_ERROR("xn_ping: IP options not turned on", NOVAR, 0, 0);
#endif      /* INCLUDE_IPV4 && INCLUDE_IP_OPTIONS */
    }

#if (RTIP_VERSION >= 26)
#if (INCLUDE_802_2)
    /* perform setsockopt() to turn on 802_2   */
    if (send_802_2)
    {
        port->options |= SOO_802_2;

        /* -8 for LLC+SNAP headers   */
        pi->pdev->mtu -= 8;
        pi->pdev->max_mss -= 8;
    }
#endif
#endif

    msg_reply = do_ping(port, sequence, len, ttl, host, wait_count);

#if (INCLUDE_UDP)
    port->list_type = FREE_LIST;
    os_free_udpport((PUDPPORT)port);
#elif (INCLUDE_TCP)
    free_tcpport_avail((PTCPPORT)port, FALSE);
#endif

    if (!msg_reply)
    {
        RTIP_API_EXIT(API_XN_PING)
        return(-1);
    }

    if (route_info)
    {
        orig_route_len = route_info->route_len;
        route_info->route_len = 0;
        if (route_type)
        {
            pip = DCUTOIPPKT(msg_reply);
            if (IP_HLEN(pip) > IP_HLEN_BYTES)   /* if there are any IP options */
            {
#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
                /* copy the route information back to the user   */
                /* first check the option type in the reply      */
                exp_route_type = -1;
                if (route_type == SO_STRICT_ROUTE_OPTION)
                    exp_route_type = STRICT_ROUTE_OPTION;
                else if (route_type == SO_LOOSE_ROUTE_OPTION)
                    exp_route_type = LOOSE_ROUTE_OPTION;
                else if (route_type == SO_RECORD_ROUTE_OPTION)
                    exp_route_type = RECORD_ROUTE_OPTION;
                pb = (PFBYTE)(DCUTOIPPKT(msg_reply)) + IP_HLEN_BYTES;
                if (exp_route_type == *pb)
                {
                    route_info->route_len = *(pb+2) - 4;
                    /* protect against more info being returned than   */
                    /* space to pass to user (should never happen)     */
                    if (orig_route_len > route_info->route_len)
                        orig_route_len = route_info->route_len;

                    tc_movebytes(route_info->route_addresses, pb+3,
                                 orig_route_len);
                }
                else
                {
                    DEBUG_ERROR("xn_ping_option: exp route type != actual: exp, act = ",
                        EBS_INT2, exp_route_type, *pb);
                }
#endif              /* INCLUDE_IPV4 && INCLUDE_IP_OPTIONS */
            }
        }       /* if route_type */
    }           /* if route_info */

    os_free_packet(msg_reply);

    if (elapsed_msec)
    {
        msec_p_tick = ks_msec_p_tick();
        *elapsed_msec = (ks_get_ticks() - start_time)*msec_p_tick;
        if (*elapsed_msec <= 0)
            *elapsed_msec = (long)msec_p_tick;
    }

    RTIP_API_EXIT(API_XN_PING)
    if (route_info)
        return(route_info->route_len);
    else
        return(0);
}

DCU do_ping(PANYPORT port, int sequence, int len, int ttl, PFBYTE host, /* __fn__ */
            unsigned int wait_count)                                    /* __fn__ */
{
PICMPPKT  pic;
PIPPKT    pip;
PFWORD    pw;
PFBYTE    pb;
word      icmp_len;
DCU       msg;
PIFACE    pi;
int       status;
int       n_max;
byte      send_ip_addr[4];
word      t;
word      first_len;
word      ip_len;
int       size;
#if (RTIP_VERSION >= 26)
int       max_data_size;
#endif

    /* get the interface structure   */
    pi = rt_get_iface(host, (PFBYTE)send_ip_addr, (PANYPORT)0,
                      (RTIP_BOOLEAN *)0);
    if (!pi)
    {
        DEBUG_ERROR("do_ping: EADDRNOTAVAIL for ", IPADDR, host, 0);
        INCR_SNMP(IpOutNoRoutes)
        set_errno(EADDRNOTAVAIL);
        return((DCU)0);
    }
/* DEBUG_ERROR("do_ping: interface is ", STR1, pi->pdev->device_name, 0);   */

    /* get maximum data which can be sent without fragmentation   */
    TOTAL_IP_HLEN_SIZE(size, port, pi, 0)

#if (RTIP_VERSION >= 26)
    max_data_size = CFG_MAX_PACKETSIZE - size - ICMP_HLEN_BYTES;
#endif
    n_max = pi->addr.mtu - IP_HLEN_BYTES - ICMP_HLEN_BYTES;
#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
    n_max -= port->ip_option_len;
#endif
#if (RTIP_VERSION >= 26)
    if (n_max > max_data_size)
        n_max = max_data_size;
#endif

#if (!INCLUDE_FRAG)
    /* Xfer a whole packet's worth or based on mtu, whichever is less   */
    len = n_max > len ? len : n_max;
#endif

    /* calculate total length if in one packet                            */
    /* (Link Layer size + IP size,IP options) + ICMP header + data length */
    first_len = (word)(size + ICMP_HLEN_BYTES + (word)len);
    if (first_len > (word)CFG_MAX_PACKETSIZE)
        first_len = (word)CFG_MAX_PACKETSIZE;

    /* A little defensive code here. Free any packets already queued on the   */
    /* ping exchange. (should never happen, but if something goes wrong       */
    /* we don't want to use up our packet pool).                              */
    OS_CLEAR_PING_EXCHG(port);

    /* allocate packet; set up IP options; set up pointer to protocol header   */
    msg = ip_alloc_packet(pi, port, first_len, 0, PING_ALLOC);
    if (!msg)
    {
        set_errno(ENOPKTS);
        return((DCU)0);
    }

    pic = DCUTOICMPPKT(msg);
    pic->icmp_type = ICMP_T_ECHO_REQUEST;
    pic->icmp_code = 0;

    pw = (PFWORD)pic->_4bytes;
    *pw = 0;                            /* Identifier */
    pw++;
    *pw = hs2net((word)sequence);       /* sequence number */

    /* Fill in the ethernet header   */
    SETUP_LL_HDR(msg, pi, EIP_68K, port->options & SOO_802_2,
                 first_len - LL_HLEN_BYTES)

#if (INCLUDE_IPV4)
    /* **************************************************   */
    /* Fill in the IP header                                */
    pip = DCUTOIPPKT(msg);
    pip->ip_proto = PROTICMP;   /* ICMP */
    tc_mv4(pip->ip_src, pi->addr.my_ip_addr, 4);
    tc_mv4(pip->ip_dest, host, 4);

    /* set up header of first packet; sets up checksum   */
    TOTAL_LL_HLEN_SIZE(size, port->options & SOO_802_2, pi, 0)
    ip_len = (word)(first_len - size);
    setup_ipv4_header(msg, port, ip_len, 0,
                      (RTIP_BOOLEAN)(len > n_max ? TRUE : FALSE), ttl, TRUE);
#endif

    /* **************************************************   */
#if (INCLUDE_FRAG)
    if (len > n_max)
    {
        /* create rest of packet filling in IP header   */
        msg = ipf_create_pkt(pi, port, msg, len, ICMP_HLEN_BYTES);
        if (!msg)
        {
            return((DCU)0);
        }

        /* fill in the data into the IP packet                      */
        /* NOTE: since a buffer is not specified, ipf_fill_pkt will */
        /*       fill a data byte into the data area;               */
        msg = ipf_fill_pkt(pi, msg, (PFBYTE)0, len, ICMP_HLEN_BYTES);
        if (!msg)           /* should never happen */
            return((DCU)0);
    }
    else
    {
#endif
        /* Point to the icmp optional data area and create a pattern   */
        if (len)
        {
            pb = (PFBYTE) &pic->_4bytes[3];
            pb++;
            for (t = 0; t < (word)len; t++)
                *pb++ = 0x5e;
        }
#if (INCLUDE_FRAG)
    }
#endif

    /* prepare ICMP checksum   */
    pic = DCUTOICMPPKT(msg);
/*  icmp_len = (word)(ICMP_HLEN_BYTES + len + port->ip_option_len);   */
    icmp_len = (word)(ICMP_HLEN_BYTES + len);
    pic->icmp_chk = 0;
    pic->icmp_chk = ipf_icmp_chksum(msg, icmp_len);

    /* set up length for tc_netwrite()   */
    DCUTOPACKET(msg)->length = first_len;

    /* save sequence number of tc_icmp_interpret() can match reply with   */
    /* request                                                            */
    port->ping_sequence = sequence;

    /* bind exchange to this application task so IP task knows who to send   */
    /* response to                                                           */
    /* NOTE: this code may be called only once at a time per interface       */
    OS_BIND_PING_EXCHG(port);

    /* add port to interface list so tc_icmp_interpret() will know who   */
    /* to send response to                                               */
    OS_CLAIM_TABLE(PING_ADD_TABLE_CLAIM)
    pi->ctrl.root_ping_ports =
        os_list_add_rear_off(pi->ctrl.root_ping_ports,
                             (POS_LIST)port, ZERO_OFFSET);
    OS_RELEASE_TABLE()

    UPDATE_INFO(pi, icmp_ping_send, 1)
    INCR_SNMP(IcmpOutEchos)

    /* **************************************************                      */
    /* Send the packet. No port, no flags no timeout                           */
    /* **************************************************                      */
/*DEBUG_ERROR("do_ping: send packet out on ", STR1, pi->pdev->device_name, 0); */
#if (INCLUDE_FRAG)
    status = ipf_netwrite(pi, (PANYPORT)port, msg, send_ip_addr, NO_DCU_FLAGS, 0);
#else
    status = tc_netwrite(pi, (PANYPORT)port, msg, send_ip_addr, NO_DCU_FLAGS, 0);
#endif

    if (status)
    {
        OS_CLAIM_TABLE(PING_DEL_TABLE_CLAIM)
        pi->ctrl.root_ping_ports =
            os_list_remove_off(pi->ctrl.root_ping_ports, (POS_LIST)port,
                               ZERO_OFFSET);
        OS_RELEASE_TABLE()
        set_errno(status);
        return((DCU)0);
    }

    /* **************************************************   */
    /* Now wait for the reply                               */
    /* **************************************************   */
    msg = OS_RCVX_PING_EXCHG(port, (word)wait_count);

    /* remove port from ping list (it was used by tc_icmp_interpret())   */
    OS_CLAIM_TABLE(PING_DEL_TABLE_CLAIM)
    pi->ctrl.root_ping_ports =
        os_list_remove_off(pi->ctrl.root_ping_ports, (POS_LIST)port,
                           ZERO_OFFSET);
    OS_RELEASE_TABLE()

    if (!msg)
    {
        set_errno(ETIMEDOUT);
        return((DCU)0);
    }

    /* Check the source of the echo and the sequence number   */
    pip = DCUTOIPPKT(msg);
    pic = DCUTOICMPPKT(msg);
    pw = (PFWORD) pic->_4bytes;      /* Identifier */
    pw++;

    /* if sequence number or hosts IP address do not match then   */
    /* not the correct response                                   */
    if ( (net2hs(*pw) != (word)sequence)  ||
         !(tc_cmp4(pip->ip_src, host, 4) ||
           ((host[0] == 0x7f) && (pip->ip_src[0] == 0x7f))) )
    {
        os_free_packet(msg);                /* Free the input packet */
        set_errno(EBADRESP);
        return((DCU)0);
    }

    /* Ping worked. Free the packet and return the good news   */
    UPDATE_INFO(pi, icmp_ping_answered, 1)
    return(msg);                     /* success */
}
#endif      /* INCLUDE_IPV4 && INCLUDE_PING */


/* ********************************************************************       */
/* xn_set_ip() - Set the ip address for this interface.                       */
/*                                                                            */
/* Summary:                                                                   */
/*   #include "rtipapi.h"                                                     */
/*                                                                            */
/*   int xn_set_ip(iface_no, local_ip_address, ip_mask)                       */
/*      int iface_no               - Interface number. (see rtip.h)           */
/*      PFBYTE local_ip_address    - The ip address. Four bytes               */
/*      PFCBYTE ip_mask            - The network mask. Four bytes             */
/*                                   If 0, use default                        */
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   Before a network node can access IP services it must have an IP address. */
/*   There are three ways to establish the IP address. You may call           */
/*   xn_bootp(), xn_rarp() to get your address from a bootp or rarp           */
/*   server, or you may call this function directly to set the IP address.    */
/*                                                                            */
/*   Adds entry in routing table corresponding to interface.                  */
/*   The network mask is used to determine values of entries to be added      */
/*   to routing table, i.e. if host or network entry etc.                     */
/*   The default mask is 255.255.255.0.                                       */
/*                                                                            */
/*   xn_interface_open() or xn_attach() must be called prior to calling       */
/*   this routine.                                                            */
/*                                                                            */
/*   For more details see the RTIP Manual.                                    */
/*                                                                            */
/* Returns:                                                                   */
/*   0 if successful, -1 if not successful                                    */
/*                                                                            */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set   */
/*   by this function.  For error values returned by this function see        */
/*   RTIP Reference Manual.                                                   */
/*                                                                            */

int xn_set_ip(int iface_no, PFBYTE local_ip_address, PFCBYTE ip_mask)  /*__fn__*/
{
int ret_val;

    RTIP_API_ENTER(API_XN_SET_IP)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_SET_IP)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    ret_val = set_ip(iface_no, local_ip_address, ip_mask);
    RTIP_API_EXIT(API_XN_PING)
    return(ret_val);
}

/* ********************************************************************   */
/* set_ip() - Set the ip address for this interface.                      */
/*                                                                        */
/*    Performs the processing for xn_set_ip.  Called internally by        */
/*    the stack.  Needed by POLLOS to avoid calling poll_os_cycle         */
/*    (which is called by RTIP_API_ENTER).                                */
/*                                                                        */
/*    Returns 0 if successful, -1 if not successful                       */
/*                                                                        */
int set_ip(int iface_no, PFBYTE local_ip_address, PFCBYTE ip_mask)  /*__fn__*/
{
PIFACE pi;
int i;

    pi = tc_ino2_iface(iface_no, SET_ERRNO);
    if (!pi)
        return(-1);

#if (INCLUDE_ERROR_CHECKING)
    /* mask of all fs for ethernet is invalid   */
    if ( ((pi->pdev->iface_type == ETHER_IFACE) ||
          (pi->pdev->iface_type == TOKEN_IFACE)) &&
         ip_mask &&
         tc_cmp4(ip_mask, ip_ffaddr, 4) )
        return(set_errno(EBADMASK));
#endif

    /* if addr valid, xn_set_ip was called previously, so delete   */
    /* any entries in routing table from the first call            */
    if (pi->addr.iface_flags & IP_ADDR_VALID)
        rt_del_from_iface(pi);

    /* set IP address   */
    tc_mv4(pi->addr.my_ip_addr, local_ip_address, 4);

    /* set mask; if no mask value from previous xn_set_ip or if xn_set_ip   */
    /* has not been called previously, use default (CLASS C) which was      */
    /* set up by tc_interface_open()                                        */
    if (ip_mask)
        tc_mv4(pi->addr.my_ip_mask, ip_mask, 4);

    tc_mv4(pi->addr.my_net_bc_ip_addr, ip_nulladdr, IP_ALEN);
    pi->addr.metric = 0;        /* loopback stays at 0 */

    if (pi->pdev->iface_type == RS232_IFACE)
    {
        pi->addr.metric = 2;
    }

    /* calc our net-directed broadcast address (205.161.8.255 for example)   */
    if (pi->pdev->iface_type == ETHER_IFACE)
    {
        pi->addr.metric = 1;

        for (i=0; i<IP_ALEN; i++)
        {
            pi->addr.my_net_bc_ip_addr[i] =
                (byte)( (local_ip_address[i] & pi->addr.my_ip_mask[i]) |
                    (ip_broadaddr[i] & ~(pi->addr.my_ip_mask[i])) );
        }
    }

    pi->addr.iface_flags |= IP_ADDR_VALID;

    /* set_ip() is called with 0.0.0.0 for bootp and dhcp; don't add   */
    /* routing table entry for this case or it will think it is        */
    /* a default entry                                                 */
    if (!tc_cmp4(local_ip_address, ip_nulladdr, IP_ALEN))
        i = rt_add_from_iface(pi);
    else
        i = 0;

#if (INCLUDE_RIP)
    if (_rip_daemon_is_running())
        _rip_broadcast_request_all(rip_get_version());
#endif

    return(i);
}


#if (INCLUDE_KEEP_STATS)
/* ********************************************************************      */
/* xn_interface_stats() - Update interface statistics                        */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   int xn_interface_stats(interface_no)                                    */
/*      int interface_no - interface number returned by xn_interface_open or */
/*                         xn_attach                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Updates the statistics for the specified interface.                     */
/*                                                                           */
/*   For more details see the RTIP Manual.                                   */
/*                                                                           */
/* Returns:                                                                  */
/*   0 if successful or -1 if an error was detected                          */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_interface_stats(int interface_no)    /*__fn__*/
{
PIFACE pi;

    pi = (PIFACE) tc_ino2_iface(interface_no, SET_ERRNO);
    if (!pi || !pi->pdev)
    {
        return(-1);
    }

    if (!(*(pi->pdev->statistics))(pi))
    {
        DEBUG_ERROR("xn_interface_stats - device statistics routine failed",
            NOVAR, 0, 0);
        return(-1);
    }
    return(0);
}
#endif  /* INCLUDE_KEEP_STATS */


/* ********************************************************************     */
/* xn_interface_info() - returns interface information                      */
/*                                                                          */
/* Implemented as a macro in rtipapi.h                                      */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_interface_info(iface_no, pi_info)                               */
/*      int iface_no   - Interface number. (see rtip.h)                     */
/*      PIFACE_INFO    - Structure in which interface information is stored */
/*                       (see rtipapi.h)                                    */
/* Description:                                                             */
/*                                                                          */
/*   Saves information about an opened interface in the pi_info parameter.  */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Returns 0 if successful or -1 if not.                                  */
/*   If errno is set sets errno if an error was detected.                   */
/*                                                                          */

/* defined in rtipapi.h   */

#if (INCLUDE_KEEP_STATS)
RTIP_BOOLEAN xn_interface_statistics(int iface_no, PIFACE_STATS stats)  /*__fn__*/
{
PIFACE pi;

    if (!tc_interface_stats(iface_no))
        return(FALSE);

    pi = (PIFACE) tc_ino2_iface(iface_no, DONT_SET_ERRNO);
    if (pi)
    {
        STRUCT_COPY(stats, pi->stats);
        return(TRUE);
    }
    return(FALSE);
}

RTIP_BOOLEAN xn_interface_ethernet_statistics(int iface_no, PETHER_STATS stats)  /*__fn__*/
{
PIFACE pi;

    pi = (PIFACE) tc_ino2_iface(iface_no, DONT_SET_ERRNO);
    if (pi)
    {
        STRUCT_COPY(*stats, *(pi->driver_stats.ether_stats));
        return(TRUE);
    }
    return(FALSE);
}

RTIP_BOOLEAN xn_interface_rs232_statistics(int iface_no, PRS232_STATS stats)  /*__fn__*/
{
PIFACE pi;

    pi = (PIFACE) tc_ino2_iface(iface_no, DONT_SET_ERRNO);
    if (pi)
    {
        STRUCT_COPY(*stats, *(pi->driver_stats.rs232_stats));
        return(TRUE);
    }
    return(FALSE);
}
#endif  /* INCLUDE_KEEP_STATS */

/* ********************************************************************      */
/* xn_rt_add() - add an ip address to the routing table                      */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   xn_rt_add(dest, mask, gw, metric, iface, ttl)                           */
/*     PFCBYTE dest         - destination IP address                         */
/*     PFCBYTE mask         - network mask (possibly includes subnet mask)   */
/*     PFCBYTE gw           - if gateway, gateway IP address; if not gateway */
/*                            then set to 0                                  */
/*     int    metric        - number of hops; NOTE: set to RT_USEIFACEMETRIC */
/*                            to use default metric specified for the        */
/*                            interface                                      */
/*     int    iface_no      - Interface number                               */
/*     int    ttl           - time to live (RT_INF is live forever)          */
/*                                                                           */
/*   Install the IP address in the routing table                             */
/*                                                                           */
/*   For more details see the RTIP Manual.                                   */
/*                                                                           */
/* Returns:                                                                  */
/*   Returns 0 if added or -1 if the routing table is full                   */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_rt_add(PFCBYTE dest, PFCBYTE mask, PFCBYTE gw, dword metric,  /*__fn__*/
              int iface, int ttl)                                    /*__fn__*/
{
int ret_val;

    RTIP_API_ENTER(API_XN_RT_ADD)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_RT_ADD)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    ret_val = rt_add(dest, mask, gw, metric, iface, ttl, 0, SNMP_LOCAL);
    RTIP_API_EXIT(API_XN_RT_ADD)
    return(ret_val);
}

/* ********************************************************************     */
/* xn_rt_del() - delete an entry from routing table                         */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   xn_rt_del(dest, mask)                                                  */
/*     PFBYTE dest          - destination IP address                        */
/*     PFCBYTE mask         - network mask (possibly includes subnet mask)  */
/*                                                                          */
/*   Delete the IP address in the routing table                             */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Returns 0 if added or -1 if the routing table is not found             */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int xn_rt_del(PFBYTE dest, PFCBYTE mask)              /*__fn__*/
{
byte net_addr[IP_ALEN];
int i;
int ret_val;

    RTIP_API_ENTER(API_XN_RT_DEL)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_RT_DEL)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    for (i=3; i>= 0; i--)
        net_addr[i] = (byte)(dest[i] & mask[i]);

    ret_val = rt_del(net_addr, mask);
    RTIP_API_EXIT(API_XN_RT_DEL)
    return(ret_val);
}

/* ********************************************************************     */
/* xn_rt_cycle_gw() - Cycle default gateway                                 */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   xn_rt_cycle_gw(void)                                                   */
/*                                                                          */
/*   Use next default gateway in list of the multiple default gateways.     */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Returns 0 if successful or -1 upon error                               */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

#if (INCLUDE_ROUTING_TABLE)

int xn_rt_cycle_gw(void)                      /*__fn__*/
{
    RTIP_API_ENTER(API_XN_CYCLE_GW)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_CYCLE_GW)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    /* if default list is not empty and it has more than one entry   */
    if (root_rt_default && (root_rt_default->pnext != root_rt_default))
    {
        root_rt_default =
                os_list_next_entry_off(root_rt_default,
                                       root_rt_default, ROUTE_OFFSET);
    }

    RTIP_API_EXIT(API_XN_CYCLE_GW)
    return(0);
}
#endif

/* ********************************************************************              */
/* xn_abort() - Abort a socket and sever any connections                             */
/*                                                                                   */
/* Summary:                                                                          */
/*   #include "rtipapi.h"                                                            */
/*                                                                                   */
/*   int  xn_abort(socket)                                                           */
/*      int socket             - Socket returned by socket                           */
/*      RTIP_BOOLEAN tcp_send_reset - Specifies whether to send reset to remote host */
/*                                                                                   */
/* Description:                                                                      */
/*   This routine shuts down a TCP connection and frees all buffers associated       */
/*   with the connection.  All readers, writers, listeners and connectors are        */
/*   signaled and the socket is closed. TCP resources are freed                      */
/*   unconditionally.                                                                */
/*                                                                                   */
/*   This routine is for TCP only if a specific socket is aborted.  If               */
/*   all socket are to be aborted, then all TCP, UDP and RAW sockets                 */
/*   will be aborted.                                                                */
/*                                                                                   */
/*   For more details see the RTIP Manual.                                           */
/*                                                                                   */
/* Returns:                                                                          */
/*   0 if successful (valid TCP socket) or -1 if not successful.                     */
/*                                                                                   */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set          */
/*   by this function.  For error values returned by this function see               */
/*   RTIP Reference Manual.                                                          */
/*                                                                                   */

int xn_abort(int socket, RTIP_BOOLEAN tcp_send_reset)              /*__fn__*/
{
#if (INCLUDE_TCP)
PANYPORT pport;
#endif

    RTIP_API_ENTER(API_XN_ABORT)

    if (socket == -1)
    {
        abort_all_sockets(tcp_send_reset);
        RTIP_API_EXIT(API_XN_ABORT)
        return(0);      /* tbd: check for failures */
    }

#if (INCLUDE_TCP)
    pport = api_sock_to_port(socket);
    if (!pport)
    {
        RTIP_API_EXIT(API_XN_ABORT)
        return(-1);
    }

    if (pport->port_type == TCPPORTTYPE)
    {
        tc_tcp_abort((PTCPPORT) pport, tcp_send_reset);
        RTIP_API_EXIT(API_XN_ABORT)
        return(0);
    }

#else
    ARGSUSED_INT(tcp_send_reset);
    ARGSUSED_INT(socket);
#endif

    RTIP_API_EXIT(API_XN_ABORT)
    return(set_errno(EOPNOTSUPPORT));
}

#define CLOSE_CLAIM_SEMS 1
void abort_all_sockets(RTIP_BOOLEAN tcp_send_reset)     /* __fn__ */
{
#if (INCLUDE_UDP)
PUDPPORT uport;
#endif
#if (INCLUDE_TCP)
PTCPPORT tport;
#endif
int i;

#if (!INCLUDE_TCP)
    ARGSUSED_INT(tcp_send_reset)
#endif

    /* UDP PORTS                                                       */
    /* Manually add all entries to their lists. We can't call standard */
    /* calls because they use kernel services which aren't yet set up  */
#if (INCLUDE_UDP || INCLUDE_RAW)
#    if (CLOSE_CLAIM_SEMS)
        OS_CLAIM_UDP(CLOSE_CLAIM_UDP)
#    endif

    for (i=FIRST_UDP_PORT; i < TOTAL_UDP_PORTS; i++)
    {
        uport = (PUDPPORT)(alloced_ports[i]);
        if (!uport || !IS_UDP_PORT(uport))
            continue;

        udp_close(uport);
    }
#    if (CLOSE_CLAIM_SEMS)
        OS_RELEASE_UDP()
#    endif
#endif

    /* TCP PORTS   */
#if (INCLUDE_TCP)
#    if (CLOSE_CLAIM_SEMS)
        OS_CLAIM_TCP(ABORT_CLAIM_TCP)
#    endif

    for (i=FIRST_TCP_PORT; i < TOTAL_TCP_PORTS; i++)
    {
        tport = (PTCPPORT)(alloced_ports[i]);
        if (!tport || !IS_TCP_PORT(tport) || tport->state == TCP_S_FREE)
            continue;

        tport->ap.port_flags |= API_CLOSE_DONE;
        tcp_abort(tport, tcp_send_reset);
    }

#    if (CLOSE_CLAIM_SEMS)
        OS_RELEASE_TCP()
#    endif
#endif
}


/* ********************************************************************          */
/* xn_interface_open() - Opens an interface                                      */
/*                                                                               */
/* Implemented as a macro in rtipapi.h                                           */
/*                                                                               */
/* Summary:                                                                      */
/*   #include "rtipapi.h"                                                        */
/*                                                                               */
/*   int xn_interface_open(int device_id, int minor_number)                      */
/*     int  device_id  - Device Type (see possible definitions in rtipapi.h)     */
/*     int  minor_number - Minor number for the device                           */
/*                                                                               */
/* Description:                                                                  */
/*   This function opens an interface of type device_id and offset               */
/*   minor_number.  There must be enough driver structures to accomadate         */
/*   minor_number (see CFG_NUM_ED etc).  CSLIP, SLIP and PPP both use the        */
/*   rs232 driver, therefore, the total number of CSLIP, SLIP and PPP interfaces */
/*   cannot exceed CFG_NUM_RS232).                                               */
/*   xn_interface_open() is equivalent to xn_attach() but is used for            */
/*   non-dedicated interfaces.                                                   */
/*                                                                               */
/*   For more details see the RTIP Manual.                                       */
/*                                                                               */
/* Returns:                                                                      */
/*   The interface number, 0-CFG_NIFACES-1 (i.e. offset into interface table,    */
/*   PIFACE), if the interface is opened successfully.                           */
/*   -1 if the interface cannot be opened.                                       */
/*                                                                               */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set      */
/*   by this function.  For error values returned by this function see           */
/*   RTIP Reference Manual.                                                      */
/*                                                                               */


/* ********************************************************************          */
/* xn_interface_open_config() - Opens an interface                               */
/*                                                                               */
/* Summary:                                                                      */
/*   #include "rtipapi.h"                                                        */
/*                                                                               */
/*   int xn_interface_open_config(int device_id, int minor_number)               */
/*     int  device_id    - Device Type (see possible definitions in rtipapi.h)   */
/*     int  minor_number - Minor number for the device                           */
/*     IOADDRESS io_address - I/O address to use instead of address from         */
/*                         device table (0=use device table value)               */
/*     int  irq_val      - IRQ to use instead of IRQ from device table           */
/*                         (-1=use device table value)                           */
/*     word mem_address  - memory address to use instead of memory address       */
/*                         from device table (0=use device table value)          */
/*                                                                               */
/* Description:                                                                  */
/*   This function opens an interface of type device_id and offset               */
/*   minor_number.  There must be enough driver structures to accomadate         */
/*   minor_number (see CFG_NUM_ED etc).  CSLIP, SLIP and PPP both use the        */
/*   rs232 driver, therefore, the total number of CSLIP, SLIP and PPP interfaces */
/*   cannot exceed CFG_NUM_RS232).                                               */
/*                                                                               */
/*   xn_interface_open_config() is equivalent to xn_interface_open()             */
/*   except with additional parameters for I/O address, IRQ and                  */
/*   memory address.  If io_address is 0, the I/O address from the               */
/*   device table is used.  If irq_val is -1, the IRQ from the                   */
/*   device is used.  If mem_address is 0, the memory address from               */
/*   the device table is used for devices which use memory mapped I/O.           */
/*   xn_interface_open_config() is equivalent to xn_attach() but is used for     */
/*   non-dedicated interfaces.                                                   */
/*                                                                               */
/*   For more details see the RTIP Manual.                                       */
/*                                                                               */
/* Returns:                                                                      */
/*   The interface number, 0-CFG_NIFACES-1 (i.e. offset into interface table,    */
/*   PIFACE), if the interface is opened successfully.                           */
/*   -1 if the interface cannot be opened.                                       */
/*                                                                               */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set      */
/*   by this function.  For error values returned by this function see           */
/*   RTIP Reference Manual.                                                      */
/*                                                                               */
int xn_interface_open_config(int device_id, int minor_number,  /*__fn__*/
                             IOADDRESS io_address,             /*__fn__*/
                             int  irq_val,                     /*__fn__*/
                             word mem_address)                 /*__fn__*/
{
int i;
int ret_val;

    RTIP_API_ENTER(API_XN_INTERFACE_OPEN_CONFIG)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_INTERFACE_OPEN_CONFIG)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    /* check interface type; must be LOOP or ETHER for xn_interface_open   */
    /* NOTE: if do not find a match, tc_interface_open will fail           */
    for (i = 0; ; i++)
    {
        if (devices_ptr[i].open == NULLP_FUNC)  /* if end of table */
        {
            DEBUG_ERROR("xn_interface_open: trying to open a device which is not in device table",
                NOVAR, 0, 0);
            DEBUG_ERROR("                   device_id, minor_number = ",
                EBS_INT2, device_id, minor_number);
            RTIP_API_EXIT(API_XN_INTERFACE_OPEN_CONFIG)
            return(set_errno(EBADDEVICE));
        }
        if (devices_ptr[i].device_id == device_id)
        {
            if ( (devices_ptr[i].iface_type != ETHER_IFACE) &&
#if (INCLUDE_PPPOE)
                (device_id != PPPOE_DEVICE) &&
#endif
                 (devices_ptr[i].iface_type != LOOP_IFACE) )
            {
                RTIP_API_EXIT(API_XN_INTERFACE_OPEN_CONFIG)
                return(set_errno(EINVAL));
            }
            break;
        }
    }

    ret_val = tc_interface_open(device_id, minor_number, io_address, irq_val,
                                mem_address);
    RTIP_API_EXIT(API_XN_INTERFACE_OPEN_CONFIG)
    return(ret_val);
}



/* ********************************************************************     */
/* xn_interface_close() - Opens an interface                                */
/*                                                                          */
/* Implemented as a macro in rtipapi.h                                      */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_interface_close(int iface_no)                                   */
/*     int  iface_no  - Interface number                                    */
/*                                                                          */
/* Description:                                                             */
/*   This function close an interface.                                      */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   0 if the interface is closed successfully.                             */
/*   -1 if the interface cannot be closed.                                  */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

/* THIS FUNCTION IS DEFINED AS A MACRO (SEE RTIP.H)   */


/* ********************************************************************       */
/* xn_wait_pkts_output() - Waits until all packets are sent or timeout        */
/*                                                                            */
/* Summary:                                                                   */
/*   #include "rtip.h"                                                        */
/*                                                                            */
/*   RTIP_BOOLEAN wait_pkts_output(disable_output, timeout)                   */
/*     RTIP_BOOLEAN disable_output - if set, disables any queueing of packets */
/*                              for output                                    */
/*     word    timeout - number of ticks to wait for output lists to          */
/*                       empty                                                */
/*                                                                            */
/* Description:                                                               */
/*   This function waits until all the packets which are queued on            */
/*   output lists are sent.  This is done for all interfaces.                 */
/*                                                                            */
/*   This is useful to do before hanging up the modem or restarting           */
/*   RTIP.                                                                    */
/*                                                                            */
/*   For more details see the RTIP Manual.                                    */
/*                                                                            */
/* Returns:                                                                   */
/*   TRUE is all packets are sent; FALSE if not.                              */
/*                                                                            */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set   */
/*   by this function.  For error values returned by this function see        */
/*   RTIP Reference Manual.                                                   */
/*                                                                            */

/* THIS FUNCTION IS DEFINED AS A MACRO (SEE RTIPAPI.H)   */


/* ********************************************************************      */
/* xn_wait_tcp_output() - Waits for all TCP data to be acknowledged          */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   int xn_wait_tcp_output(int sec_wait)                                    */
/*     int  sec_wait  - Number of seconds to wait for output window to empty */
/*                                                                           */
/* Description:                                                              */
/*   Waits for all TCP data (queued on all sockets) to be sent               */
/*   and acknowledged (i.e. waits for output window to emtpy.                */
/*                                                                           */
/*   This is useful to call before closing an interface.                     */
/*                                                                           */
/*   For more details see the RTIP Manual.                                   */
/*                                                                           */
/* Returns:                                                                  */
/*   Returns TRUE if all data has been sent or FALSE if timeout              */
/*                                                                           */

RTIP_BOOLEAN xn_wait_tcp_output(int sec_wait)
{
#if (INCLUDE_TCP)
int i;
PTCPPORT port;
RTIP_BOOLEAN done;

    done = TRUE;
    for (i=0; i<sec_wait || (i==0 && sec_wait == 0); i++)
    {
        port = (PTCPPORT)(root_tcp_lists[ACTIVE_LIST]);
        done = TRUE;

        /* loop thru all the ports to see if there is any data in the   */
        /* output windows                                               */
        while (port)
        {
            if (port->out.contain)
            {
                done = FALSE;
                break;
            }
            port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[ACTIVE_LIST],
                                                    (POS_LIST)port, ZERO_OFFSET);
        }
        /* if everything has been sent, we are done and will return TRUE   */
        if (done || (sec_wait == 0))
            break;
        ks_sleep(ks_ticks_p_sec());
    }
    return(done);
#else
    ARGSUSED_INT(sec_wait)
    return(FALSE);
#endif
}

/* ********************************************************************     */
/* xn_interface_mcast() - Setup/Add/Delete a multicast list                 */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_interface_set_mcast(iface_no, mclist, mclistsize, op)           */
/*     int iface_no   - Interface number                                    */
/*     PFBYTE mclist  - List of multicast IP address                        */
/*     int mclistsize - Number of IP addresses in mclist                    */
/*     int op         - Operation to perform (SET_ENTRIES, ADD_ENTRIES,     */
/*                      DELETE_ENTRIES, CLEAR_ENTRIES)                      */
/*                                                                          */
/* Description:                                                             */
/*   This function sets up, adds to, deletes from or clears the             */
/*   multicast list for an interface.  It also sets up the device           */
/*   driver for that interface to accept packets sent to any                */
/*   of the multicast addresses in the list.                                */
/*                                                                          */
/*   A default interface may be setup by calling xn_interface_opt().  If    */
/*   the value of the parameter iface_no is -1, the default value will      */
/*   be used.                                                               */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Returns TRUE if successful or FALSE if failure                         */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

/* THIS FUNCTION IS DEFINED AS A MACRO (SEE RTIP.H)   */

/* ********************************************************************         */
/* xn_interface_opt() - Change an option associated with an interface           */
/*                                                                              */
/* Summary:                                                                     */
/*   #include "rtipapi.h"                                                       */
/*                                                                              */
/*   int xn_interface_opt(iface_no, option_name, option_value, optionlen)       */
/*     int iface_no              - interface number returned by                 */
/*                                 xn_interface_open or xn_attach               */
/*     int option_name           - options                                      */
/*     PFCCHAR option_value      - buffer which contains values for option      */
/*     int optionlen             - length of option_value                       */
/*                                                                              */
/* Description:                                                                 */
/*   Allows application set the following options associated with               */
/*   an interface:                                                              */
/*     IO_DEFAULT_MCAST  - default multicast interface                          */
/*     IO_MAX_OUTPUT_QUE - maximum number of packets which can be queued        */
/*                         on output list (-1 = no limit = default)             */
/*     IO_OUTPUT_WINDOW  - TCP output window size; does not affect sockets      */
/*                         already connected                                    */
/*     IO_INPUT_WINDOW   - TCP input window size; does not affect sockets       */
/*                         already connected                                    */
/*     IO_MTU            - MTU: Maximum Transfer Unit (maximum size of          */
/*                         packet, includes IP header)                          */
/*                         NOTE: TCP - changing MTU will not affect             */
/*                                     sockets which are already connected      */
/*                         NOTE: UDP - changing MTU will affect all             */
/*                                     further sends on this interface          */
/*     IO_802_2          - Packets sent over this interface should be           */
/*                         802.2 packets                                        */
/*                                                                              */
/*   For more details see the RTIP Manual.                                      */
/*                                                                              */
/* Returns:                                                                     */
/*   0 if option set successfully.                                              */
/*   -1 if an error occurred.                                                   */
/*                                                                              */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set     */
/*   by this function.  For error values returned by this function see          */
/*   RTIP Reference Manual.                                                     */
/*                                                                              */

int xn_interface_opt(int iface_no, int option_name, PFCCHAR option_value, /*__fn__*/
                     int optionlen)                                       /*__fn__*/
{
PIFACE pi;

    RTIP_API_ENTER(API_XN_INTERFACE_OPT)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_INTERFACE_OPT)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    switch (option_name)
    {
    case IO_DEFAULT_MCAST:
        if (optionlen != sizeof(int))
        {
            RTIP_API_EXIT(API_XN_INTERFACE_OPT)
            return(set_errno(EFAULT));
        }

        if (*((PFINT)option_value))
            default_mcast_iface = iface_no;
        else
            default_mcast_iface = -1;
        RTIP_API_EXIT(API_XN_INTERFACE_OPT)
        return(0);

    case IO_MAX_OUTPUT_QUE:
    case IO_OUTPUT_WINDOW:
    case IO_INPUT_WINDOW:
    case IO_MTU:
#if (INCLUDE_802_2)
    case IO_802_2:
#endif
        if (optionlen != sizeof(int))
            return(set_errno(EFAULT));

        pi = tc_ino2_iface(iface_no, SET_ERRNO);
        if (!pi)
        {
            RTIP_API_EXIT(API_XN_INTERFACE_OPT)
            return(-1);
        }

        if (*((PFINT)option_value) >= 0)
        {
            switch (option_name)
            {
            case IO_MAX_OUTPUT_QUE:
                pi->max_output_que = *((PFINT)option_value);
                break;
            case IO_INPUT_WINDOW:
                pi->pdev->window_size_in = (word)*((PFINT)option_value);
                break;
            case IO_OUTPUT_WINDOW:
                pi->pdev->window_size_out = (word)*((PFINT)option_value);
                break;
            case IO_MTU:
                pi->addr.mtu = pi->pdev->mtu = (word)*((PFINT)option_value);

                /* update the pi so new mtu value takes effect immediately   */
                pi->addr.mtu = (word)*((PFINT)option_value);
                break;
#if (INCLUDE_802_2)
            case IO_802_2:
                pi->iface_flags |= IS_802_2;
                break;
#endif
            }
        }

        RTIP_API_EXIT(API_XN_INTERFACE_OPT)
        return(0);
    }
    RTIP_API_EXIT(API_XN_INTERFACE_OPT)
    return(set_errno(ENOPROTOOPT));

}

/* ********************************************************************   */
/* PKT API                                                                */
/* ********************************************************************   */

/* ********************************************************************     */
/* xn_pkt_data_max() - Maximum number of bytes transferable per packet      */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_pkt_data_max(int socket, dest_ip_addr)                          */
/*      int socket          - Socket returned by socket                     */
/*      PFBYTE dest_ip_addr - Destination IP address - UDP only             */
/*                            (used to determine interface to get MTU)      */
/*                                                                          */
/* Description:                                                             */
/*   The high speed interface allows the user to copy data directly into    */
/*   a packet structure which may then be queued for sending. The packet    */
/*   structure is of fixed length and contains preamble information. This   */
/*   routine returns the number of bytes available for user data in the     */
/*   packet while xn_pkt_data_pointer() returns the address of that         */
/*   data area.  The number of bytes returned assumes no IP or protocol     */
/*   options.                                                               */
/*                                                                          */
/*   For TCP, the value is the maximum segment size sent in the             */
/*   initial sync message, therefore, this routine should not               */
/*   be called until the connection has been established.                   */
/*                                                                          */
/*   For UDP, the socket has to be connected or the destination address     */
/*   must be passed as the parameter dest_ip_addr.                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/*   For a socket, the return value is the same for all packets             */
/*   so the function return value may be saved once for a                   */
/*   given socket and need not be called for every packet.  The only        */
/*   exception for UDP where a different destination address may            */
/*   lead to a different data length (i.e. due to different MTU values      */
/*   for the different interfaces).                                         */
/*                                                                          */
/* Returns:                                                                 */
/*   An integer indicating how many bytes are available in the user area of */
/*   a packet or 0 if an error was detected.                                */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int xn_pkt_data_max(int socket, PFBYTE dest_ip_addr)
{
PANYPORT pport;
int      ret_val;
#if (INCLUDE_UDP || INCLUDE_RAW)
PFBYTE   to;
PIFACE   pi;
#if (RTIP_VERSION >= 26)
int      max_data_size;
#endif
#endif

    RTIP_API_ENTER(API_XN_PKT_DATA_MAX)

#if (!INCLUDE_UDP && !INCLUDE_RAW)
    ARGSUSED_PVOID(dest_ip_addr);
#endif

    pport = api_sock_to_port(socket);
    if (!pport)
    {
        RTIP_API_EXIT(API_XN_PKT_DATA_MAX)
        return(0);
    }

    /* ******   */
    switch (pport->port_type)
    {
#if (INCLUDE_UDP || INCLUDE_RAW)
    case UDPPORTTYPE:
    case RAWPORTTYPE:
        if (dest_ip_addr)
            pi = rt_get_iface(dest_ip_addr, (PFBYTE)0, (PANYPORT)pport,
                              (RTIP_BOOLEAN *)0);
        else
        {
            to = ((PUDPPORT)pport)->ip_connection.ip_dest;
            if (tc_cmp4(to, ip_nulladdr, 4))
            {
                RTIP_API_EXIT(API_XN_PKT_DATA_MAX)
                set_errno(ENOTCONN);
                return(0);
            }
            pi = rt_get_iface(to, (PFBYTE)0, (PANYPORT)pport,
                              (RTIP_BOOLEAN *)0);
        }

        if (!pi)
        {
            set_errno(EADDRNOTAVAIL);
            RTIP_API_EXIT(API_XN_PKT_DATA_MAX)
            return(0);
        }

#if (RTIP_VERSION >= 26)
        /* calculate data size based upon MTU and MAX_PACKETSIZE;   */
        /* the data size calculated based upon MTU but limited by   */
        /* calculation based upon MTU is returned                   */
#else
        /* calculate data size based upon MTU   */
#endif
        ret_val = pi->addr.mtu - IP_HLEN_BYTES - pport->ip_option_len;
#if (RTIP_VERSION >= 26)
        TOTAL_IP_HLEN_SIZE(max_data_size, pport, pi, 0)
        max_data_size = CFG_MAX_PACKETSIZE - max_data_size;
#endif
        if (pport->port_type == UDPPORTTYPE)
        {
            ret_val -= UDP_HLEN_BYTES;
#if (RTIP_VERSION >= 26)
            max_data_size -= UDP_HLEN_BYTES;
#endif
        }
#if (RTIP_VERSION >= 26)
        /* verify size will fit into our maximum packet size;  also         */
        /* just in case the remote did not adjust the MSS by the LLC,SNAP   */
        /* headers this will adjust the size downward to fix that           */
        /* NOTE: this assumes CFG_MAX_PACKETSIZE reflects maxmimum          */
        /*       ethernet size; otherwise have to determine what the        */
        /*       device driver is and compare to its maximum transport size */
        if (ret_val > max_data_size)
            ret_val = max_data_size;
#endif      /* RTIP_VERSION */

        return(ret_val);
#endif      /* UDP || RAW */

    /* ******   */
#if (INCLUDE_TCP)
    case TCPPORTTYPE:
        return(tcp_pkt_data_max((PTCPPORT)pport));
#endif
    }

    /* ******   */
    set_errno(EOPNOTSUPPORT);
    RTIP_API_EXIT(API_XN_PKT_DATA_MAX)
    return(0);
}


#if (INCLUDE_PKT_API || INCLUDE_UDP || INCLUDE_RAW)
/* ********************************************************************        */
/* xn_pkt_data_pointer() - Returns the address of the user area in a packet    */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "rtipapi.h"                                                      */
/*                                                                             */
/*   PFBYTE xn_pkt_data_pointer(socket, msg, type)                             */
/*     int socket   - Socket returned by socket                                */
/*     DCU  msg     - Message structure returned from either                   */
/*                    xn_pkt_recv or xn_pkt_alloc                              */
/*     int type     - type of packet (INPUT_PKT, OUTPUT_PKT)                   */
/*                                                                             */
/* Description:                                                                */
/*   The high speed interface allows the user to copy data directly into       */
/*   a packet structure which may then be queued for sending. The packet       */
/*   structure is of fixed length and contains preamble information.           */
/*   This routine returns a pointer to the user data in the packet.            */
/*                                                                             */
/*   For send operations this returns the offset of the user data area and     */
/*   xn_pkt_data_max() returns the number of bytes that may be copied          */
/*   to it.                                                                    */
/*                                                                             */
/*   For receive operations, this returns the offset of the user data area and */
/*   xn_pkt_data_size() returns the number of valid bytes in the user          */
/*   data area.                                                                */
/*                                                                             */
/*   If type is INPUT_PKT, it is assumed the packet is already formatted       */
/*   and a pointer to the data area taking into account options and            */
/*   fragmentation is returned.                                                */
/*                                                                             */
/*   If type is OUTPUT_PKT, it is assumed the packet has not been              */
/*   formatted and a pointer to the data area assuming no options and          */
/*   no fragmentation is returned.                                             */
/*                                                                             */
/*   If type is OUTPUT_FRAG_PKT, it is assumed the only                        */
/*   formatting which has been done is the fragment fields, therefore,         */
/*   the pointer to the data area returned takes into consideration            */
/*   whether the packet is fragmented.                                         */
/*                                                                             */
/*   LIMITATIONS: does not work for token ring packet or 802.2;                */
/*                use xn_pkt_data_pointer_iface() for token ring               */
/*                                                                             */
/*   For more details see the RTIP Manual.                                     */
/*                                                                             */
/* Returns:                                                                    */
/*                                                                             */
/*   A pointer to the user data area in the packet or 0 if error               */
/*                                                                             */

PFBYTE xn_pkt_data_pointer(int socket, DCU msg, int type)           /*__fn__*/
{
PANYPORT pport;
int      off_len;
PFBYTE   ret_val;

    RTIP_API_ENTER(API_XN_DATA_POINTER)

    ARGSUSED_INT(type); /* keep compiler happy (will be needed when options */
                        /* supported)   */

    pport = api_sock_to_port(socket);
    if (!pport)
    {
        RTIP_API_EXIT(API_XN_PKT_DATA_POINTER)
        return(0);
    }

    if (!msg)
    {
        set_errno(EFAULT);
        RTIP_API_EXIT(API_XN_PKT_DATA_POINTER)
        return((PFBYTE)0);
    }

    /* point to protocol area of packet   */
    if (type == INPUT_PKT)
    {
        ret_val = DCUTOPROTPKT(msg);
    }
    else
    {
        /* perform TOTAL_IP_HLEN_SIZE(off_len, pport, (PIFACE)0, 0)   */
        TOTAL_ETH_HLEN_SIZE(off_len, pport->options & SOO_802_2, msg)
        off_len += IP_HLEN_BYTES;
        off_len += pport->ip_option_len;

        ret_val = DCUTODATA(msg) + off_len;
    }

    /* if fragment, but not first fragment then the packet does not   */
    /* have a protocol header                                         */
    if ( (type == INPUT_PKT) || (type == OUTPUT_FRAG_PKT) )
    {
        if (is_frag_not_first(msg))
        {
            RTIP_API_EXIT(API_XN_PKT_DATA_POINTER)
            return(ret_val);
        }
    }

    /* ******   */
    switch (pport->port_type)
    {
#if (INCLUDE_UDP)
    case UDPPORTTYPE:
        ret_val += UDP_HLEN_BYTES;
        return(ret_val);
#endif

#if (INCLUDE_RAW)
    case RAWPORTTYPE:
        /* raw packets do not have a protocol header   */
        break;
#endif

#if (INCLUDE_TCP)
    case TCPPORTTYPE:
        ret_val += TCP_HLEN_BYTES;
        break;
#endif
    default:
        ret_val = 0;
        set_errno(EOPNOTSUPPORT);
    }

    RTIP_API_EXIT(API_XN_PKT_DATA_POINTER)
    return(ret_val);
}

#if (INCLUDE_TOKEN || INCLUDE_802_2)
/* ********************************************************************           */
/* xn_pkt_data_pointer_iface() - Returns the address of the user area in a packet */
/*                                                                                */
/* Implemented as a macro in rtipapi.h                                            */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "rtipapi.h"                                                         */
/*                                                                                */
/*   PFBYTE xn_pkt_data_pointer(iface_no, socket, msg, type)                      */
/*     int iface_no - interface number returned by xn_interface_open or           */
/*                    xn_attach                                                   */
/*     int socket   - Socket returned by socket                                   */
/*     DCU  msg     - Message structure returned from either                      */
/*                    xn_pkt_recv or xn_pkt_alloc                                 */
/*     int type     - type of packet (INPUT_PKT, OUTPUT_PKT)                      */
/*                                                                                */
/* Description:                                                                   */
/*   This function is the same as xn_pkt_data_pointer except the                  */
/*   interface number is also passed in as a parameter.  The iface_no             */
/*   parameter is needed if the interface might be token ring or                  */
/*   supports 802.2.                                                              */
/*                                                                                */
/*   For more details see the RTIP Manual.                                        */
/*                                                                                */
/* Returns:                                                                       */
/*                                                                                */
/*   A pointer to the user data area in the packet or 0 if error                  */
/*                                                                                */

PFBYTE xn_pkt_data_pointer_iface(int iface_no, int socket, DCU msg, int type)           /*__fn__*/
{
PANYPORT pport;
int      off_len;
PFBYTE   ret_val;
PIFACE   pi;

    RTIP_API_ENTER(API_XN_DATA_POINTER)

    ARGSUSED_INT(type); /* keep compiler happy (will be needed when options */
                        /* supported)   */

    pi = tc_ino2_iface(iface_no, SET_ERRNO);

    pport = api_sock_to_port(socket);
    if (!pi || !pport || !msg)
    {
        if (!msg)
        {
            set_errno(EFAULT);
        }
        RTIP_API_EXIT(API_XN_DATA_POINTER)
        return((PFBYTE)0);
    }

    /* point to protocol area of packet   */
    if (type == INPUT_PKT)
    {
        ret_val = DCUTOPROTPKT(msg);
    }
    else
    {
        TOTAL_IP_HLEN_SIZE(off_len, pport, pi, 0)
        ret_val = DCUTODATA(msg) + off_len;
    }

    /* if fragment, but not first fragment then the packet does not   */
    /* have a protocol header                                         */
    if ( (type == INPUT_PKT) || (type == OUTPUT_FRAG_PKT) )
    {
        if (is_frag_not_first(msg))
        {
            RTIP_API_EXIT(API_XN_DATA_POINTER)
            return(ret_val);
        }
    }

    /* ******   */
    switch (pport->port_type)
    {
#if (INCLUDE_UDP)
    case UDPPORTTYPE:
        ret_val += UDP_HLEN_BYTES;
        break;
#endif

#if (INCLUDE_RAW)
    case RAWPORTTYPE:
        /* raw packets do not have a protocol header   */
        break;
#endif

#if (INCLUDE_TCP)
    case TCPPORTTYPE:
        ret_val += TCP_HLEN_BYTES;
        break;
#endif
    default:
        set_errno(EOPNOTSUPPORT);
        ret_val = 0;
    }
    RTIP_API_EXIT(API_XN_DATA_POINTER)
    return(ret_val);
}
#endif

/* ********************************************************************     */
/* pkt_data_size() - Return count of valid user byte in a received packet   */
/*                                                                          */
/*   This function examines the packet structure and returns the number of  */
/*   valid user bytes in the user data area of the current msg only (i.e    */
/*   it does not count any fragments which are linked to msg.               */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/*   Returns the number of valid user bytes in the user data area.          */
/*                                                                          */

int pkt_data_size(DCU msg)      /*__fn__*/
{
word dlen;
#if (INCLUDE_TCP)
    PTCPPKT  pt;
    word     hlen;
#endif
PIPPKT  pip;

    pip = DCUTOIPPKT(msg);

    switch (pip->ip_proto)
    {
#if (INCLUDE_UDP)
    case PROTUDP:
        /* Get total packet length of current frag excluding IP header    */
        /* and IP options                                                 */
        dlen = net2hs(pip->ip_len);
        dlen = (word)(dlen - IP_HLEN(pip));  /* sub number of bytes in  */
                                             /* IP header   */

        /* sub out UDP header; UDP header is only on the first datagram   */
        /* received in a message; i.e. if not a frag packet or first      */
        /* fragmented packet, then it contains a UDP header               */
        if (!is_frag(msg) || !is_frag_not_first(msg))
            dlen = (word)(dlen - UDP_HLEN_BYTES);
        return(dlen);
#endif

#if (INCLUDE_TCP)
    case PROTTCP:
        pt = DCUTOTCPPKT(msg);

        /* Get total packet length excluding IP header and IP options   */
        dlen = net2hs(pip->ip_len);
        dlen = (word)(dlen - IP_HLEN(pip));  /* sub number of bytes in IP header */

        /* Tcp header length in bytes                                   */
        /* i.e. ((x&0xf0)>>4)<<2); i.e.                                 */
        /*   - mask out lower 4 bits (reserved)                         */
        /*   - shift upper 4 bits to lower bits                         */
        /*   - convert # words to # bytes                               */
        /* which is equivalent to ((x&0xf0)>>2)                         */
        /* sub out TCP header; TCP header is only on the first datagram */
        /* received in a message; i.e. if not a frag packet or first    */
        /* fragmented packet, then it contains a TCP header             */
        if (!is_frag(msg) || !is_frag_not_first(msg))
        {
            hlen = (word) ((pt->tcp_hlen & 0xf0)>>2);

            /* length in the data area of the TCP packet   */
            if (hlen > dlen)
                dlen = 0;
            else
                dlen = (word)(dlen - hlen);
        }
        return(dlen);

#endif
    default:
#if (INCLUDE_RAW)
        /* if none of the above, assume RAW socket;       */
        /* RAW Length it total length minus the IP header */
        dlen = net2hs(pip->ip_len);
        dlen = (word)(dlen - IP_HLEN(pip));  /* sub number of bytes  */
                                             /* in IP header   */
        return(dlen);
#else
        set_errno(EOPNOTSUPPORT);
        return(0);
#endif
    }
}

#if (INCLUDE_TCP || INCLUDE_RAW)
/* ********************************************************************       */
/* tcp_pkt_data_size() - Return count of valid user byte in a received packet */
/*                                                                            */
/*   This function examines the packet structure and returns the number of    */
/*   valid user bytes in the user data area.  If the parameter count_frags    */
/*   is TRUE, all fragments in the linked list are counted including the      */
/*   current msg.  If count_frags is FALSE, only the size of the current      */
/*   frag (msg) is counted.                                                   */
/*                                                                            */
/*   For more details see the RTIP Manual.                                    */
/*                                                                            */
/* Returns:                                                                   */
/*   The number of valid user bytes in the user data area.                    */

int tcp_pkt_data_size(DCU msg, RTIP_BOOLEAN count_frags)      /*__fn__*/
{
word dlen;

#if (INCLUDE_FRAG)
    if (count_frags && is_frag(msg))
    {
        dlen = 0;
        while (msg)
        {
            dlen += (word)pkt_data_size(msg);
            msg = DCUTOPACKET(msg)->frag_next;
        }
    }
    else
#endif
    {
        dlen = (word)pkt_data_size(msg);
    }
    return(dlen);
}
#endif /* INCLUDE_TCP || INCLUDE_RAW */


/* ********************************************************************       */
/* xn_pkt_data_size() - Return count of valid user byte in a received packet  */
/*                                                                            */
/* Summary:                                                                   */
/*   #include "rtipapi.h"                                                     */
/*                                                                            */
/*   int  xn_pkt_data_size(msg, count_frag)                                   */
/*     DCU msg            - Message handle returned by xn_pkt_recv()          */
/*     RTIP_BOOLEAN count_frag - specified whether should count any fragments */
/*                          which are attached to msg                         */
/*                                                                            */
/*                                                                            */
/* Description:                                                               */
/*   This function examines the packet structure and returns the number of    */
/*   valid user bytes in the user data area.  If the parameter count_frags    */
/*   is TRUE, all fragments in the linked list are counted including the      */
/*   current msg.  If count_frags is FALSE, only the size of the current      */
/*   frag (msg) is counted.                                                   */
/*                                                                            */
/*   For more details see the RTIP Manual.                                    */
/*                                                                            */
/* Returns:                                                                   */
/*   The number of valid user bytes in the user data area.                    */
/*                                                                            */

int xn_pkt_data_size(DCU msg, RTIP_BOOLEAN count_frags)      /*__fn__*/
{
#if (INCLUDE_UDP)
    PUDPPKT pu;
    word l;
#endif
PIPPKT  pip;
int    ret_val;

    RTIP_API_ENTER(API_XN_PKT_DATA_SIZE)

#if (!INCLUDE_FRAG && !INCLUDE_UDP)
    ARGSUSED_INT(count_frags);
#endif

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_PKT_DATA_SIZE)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    if (!msg)
    {
        RTIP_API_EXIT(API_XN_PKT_DATA_SIZE)
        return(set_errno(EFAULT));
    }

    pip = DCUTOIPPKT(msg);

    switch (pip->ip_proto)
    {
#if (INCLUDE_UDP)
    case PROTUDP:
        pu = DCUTOUDPPKT(msg);

        if (count_frags)
        {
            /* Udp Length is the size of the data plus the udp header length (8)         */
            /* of the whole packet, therefore, subtract the header from the total length */
            l = net2hs(pu->udp_len);
            if (l >= UDP_HLEN_BYTES)
                ret_val = l-UDP_HLEN_BYTES;
            else
                ret_val = 0;
        }
        else
        {
            ret_val = pkt_data_size(msg);
        }
        break;
#endif

#if (INCLUDE_RAW)
    default:
#endif
#if (INCLUDE_TCP || INCLUDE_RAW)
    case PROTTCP:
        ret_val = tcp_pkt_data_size(msg, count_frags);
        break;
#endif
#if (!INCLUDE_RAW)
    default:
        set_errno(EOPNOTSUPPORT);
        ret_val = 0;
#endif
    }       /* end of switch */
    RTIP_API_EXIT(API_XN_PKT_DATA_SIZE)
    return(ret_val);
}

#endif

#if (INCLUDE_PKT_API)

/* ********************************************************************       */
/* xn_pkt_recv() - Wait for the next packet at a port                         */
/*                                                                            */
/* Implemented as a macro in rtipapi.h                                        */
/*                                                                            */
/* Summary:                                                                   */
/*   #include "rtip.h"                                                        */
/*                                                                            */
/*   DCU xn_pkt_recv(socket, wait_count)                                      */
/*     int socket              - Socket returned in socket                    */
/*     unsigned int wait_count - Wait this many ticks for a message packet    */
/*                               0 == 's None, RTIP_INF =='s forever          */
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This routine waits at the port's input queue for the next packet to come */
/*   from the net. If a packet is already there it returns immediately. If no */
/*   packets arrive before wait_count expires it return (DCU) 0.              */
/*                                                                            */
/*   The aplication should use the functions xn_pkt_data_pointer() and        */
/*   xn_pkt_data_size() to access the data.                                   */
/*                                                                            */
/*   It is the application's job to release the packet. It must return it to  */
/*   the free pool by calling xn_pkt_free.                                    */
/*                                                                            */
/*   For more details see the RTIP Manual.                                    */
/*                                                                            */
/* Returns:                                                                   */
/*   A valid message pointer if data was received or zero on a timeout        */
/*   or error.                                                                */
/*                                                                            */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set   */
/*   by this function.  For error values returned by this function see        */
/*   RTIP Reference Manual.                                                   */
/*                                                                            */

/* THIS FUNCTION IS DEFINED AS A MACRO (SEE RTIP.H)   */


/* ********************************************************************       */
/* xn_pkt_recv_from() - Wait for the next packet at a port                    */
/*                                                                            */
/* Summary:                                                                   */
/*   #include "rtip.h"                                                        */
/*                                                                            */
/*   DCU xn_pkt_recv_from(socket, from, from_port, wait_count)                */
/*     int socket              - Socket returned in socket                    */
/*     PFBYTE from             - IP address of the target received from       */
/*     PFINT from_port         - Port number of the target received from      */
/*     unsigned int wait_count - Wait this many ticks for a message packet    */
/*                               0 == 's None, RTIP_INF =='s forever          */
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   This routine waits at the port's input queue for the next packet to come */
/*   from the net. If a packet is already there it returns immediately. If no */
/*   packets arrive before wait_count expires it return (DCU) 0.  It returns  */
/*   the address of the sender in from parameter.                             */
/*                                                                            */
/*   The application should use the functions xn_pkt_data_pointer() and       */
/*   xn_pkt_data_size() to access the data.                                   */
/*                                                                            */
/*   It is the application's job to release the packet. It must return it to  */
/*   the free pool by calling os_free_packet(msg).                            */
/*                                                                            */
/*   For more details see the RTIP Manual.                                    */
/*                                                                            */
/* Returns:                                                                   */
/*   A valid message pointer if data was received or zero on a timeout        */
/*   or error.                                                                */
/*                                                                            */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set   */
/*   by this function.  For error values returned by this function see        */
/*   RTIP Reference Manual.                                                   */
/*                                                                            */

DCU xn_pkt_recv_from(int socket, PFBYTE from, PFINT from_port, unsigned int wait_count)       /*__fn__*/
{
DCU msg;
PANYPORT pport;
#if (INCLUDE_UDP || INCLUDE_RAW)
PUDPPORT pu_port;
#endif

    RTIP_API_ENTER(API_XN_PKT_RECV)

#if (!INCLUDE_UDP && !INCLUDE_RAW)
    ARGSUSED_PVOID(from);
    ARGSUSED_PVOID(from_port);
#endif

    msg = (DCU) 0;

    pport = api_sock_to_port(socket);
    if (!pport)
    {
        RTIP_API_EXIT(API_XN_PKT_RECV)
        return((DCU)0);
    }

    /* check if operation is illegal due to receive shutdown   */
    if (pport->port_flags & READ_SHUTDOWN)
    {
        set_errno(ESHUTDOWN);
        RTIP_API_EXIT(API_XN_PKT_RECV)
        return((DCU)0);
    }

    /* ******   */
#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        /* pport and pu_port are same port just cast as different types   */
        pu_port = (PUDPPORT)pport;

        if (from && from_port)   /* if recvfrom */
        {
            if ( !(pu_port->ap.port_flags & PORT_BOUND) )
            {
                set_errno(EINVAL);
                RTIP_API_EXIT(API_XN_PKT_RECV)
                return((DCU)0);
            }
        }
        else                    /* if recv */
        {
            /* check if it is connected   */
            DEBUG_LOG("pkt_recv: ", LEVEL_3, IPADDR, pu_port->ip_connection.ip_dest, 0);
            if (tc_cmp4((pu_port->ip_connection.ip_dest), ip_nulladdr, 4))
            {
                set_errno(ENOTCONN);
                RTIP_API_EXIT(API_XN_PKT_RECV)
                return((DCU)0);
            }
        }

        /* if no block and packet is not available, return error              */
        /* NOTE: pport and pu_port are same port just cast as different types */
        if ( !(pport->options & IO_BLOCK_OPT) &&
             !os_udp_pkt_avail(pu_port) )
        {
            set_errno(EWOULDBLOCK);
            RTIP_API_EXIT(API_XN_PKT_RECV)
            return((DCU)0);
        }

        /* get UDP input packet (can block in none available)   */
        msg = os_rcvx_udpapp_exchg(pu_port, (word)wait_count, TRUE);
        if (!msg)
        {
            /* only an error if there is a wait count   */
            if (wait_count)
            {
                DEBUG_ERROR("os_rcvx_udpapp_exchg() timed out",
                    NOVAR, 0, 0);
                RTIP_API_EXIT(API_XN_PKT_RECV)
                set_errno(ETIMEDOUT);
            }
            RTIP_API_EXIT(API_XN_PKT_RECV)
            return((DCU)0);
        }

        if (from  && from_port)
            tc_udp_pk_peer_address(msg, from, (PFINT)from_port);
        RTIP_API_EXIT(API_XN_PKT_RECV)
        return(msg);
    }
    else
#endif

    /* ******   */
#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        if (pport->options & SO_TCP_NO_COPY)
        {
            tc_tcp_read((PTCPPORT)pport, (PFBYTE)0, 0, (DCU KS_FAR *)&msg,
                        0, (word)wait_count);
            RTIP_API_EXIT(API_XN_PKT_RECV)
            return(msg);
        }
    }
#endif

    /* ******   */
    set_errno(EOPNOTSUPPORT);
    RTIP_API_EXIT(API_XN_PKT_RECV)
    return(msg);
}


/* ********************************************************************           */
/* xn_pkt_send() - Send data directly to a protocol port (unbuffered)             */
/*                                                                                */
/* Implemented as a macro in rtip.h                                               */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "rtip.h"                                                            */
/*                                                                                */
/*   int xn_pkt_send(socket, msg, n, wait_count)                                  */
/*     int  socket             - Socket returned in socket                        */
/*     DCU  msg                - Message structure returned from  xn_pkt_alloc    */
/*     int  n                  - Reference bytes to send                          */
/*     unsigned int wait_count - In ticks.                                        */
/*                               TCP: Fail if all data in the packet              */
/*                               can't be deliver to the other side of            */
/*                               the socket in this many ticks.                   */
/*                               UDP: Fail if the packet can't be sent over       */
/*                               the interface in this many ticks. If wait_count  */
/*                               is zero the packet is queued for output and      */
/*                               success is returned immediately (asynchronous).  */
/*                                                                                */
/*                                                                                */
/* Description:                                                                   */
/*   See xn_pkt_send_to                                                           */
/*                                                                                */
/*   For more details see the RTIP Manual.                                        */
/*                                                                                */
/* Returns:                                                                       */
/*   0 if the packet was sent. -1 otherwise                                       */
/*                                                                                */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set       */
/*   by this function.  For error values returned by this function see            */
/*   RTIP Reference Manual.                                                       */
/*                                                                                */

/* THIS FUNCTION IS DEFINED AS A MACRO (SEE RTIP.H)   */



/* ********************************************************************            */
/* xn_pkt_send_to() - Send data directly to a protocol port (unbuffered)           */
/*                                                                                 */
/* Summary:                                                                        */
/*   #include "rtip.h"                                                             */
/*                                                                                 */
/*   int xn_pkt_send_to(socket, msg, n, to, to_port, wait_count)                   */
/*     int  socket             - Socket returned in socket                         */
/*     DCU  msg                - Message structure returned from  xn_pkt_alloc     */
/*     int  n                  - Reference bytes to send                           */
/*     PFBYTE to               - IP address of the target to send to (UDP only)    */
/*     int to_port             - Port number at the target to send to (UDP only)   */
/*     unsigned int wait_count - In ticks.                                         */
/*                               TCP: Fail if all data in the packet               */
/*                               can't be deliver to the other side of             */
/*                               the socket in this many ticks.                    */
/*                               UDP: Fail if the packet can't be sent over        */
/*                               the interface in this many ticks. If wait_count   */
/*                               is zero the packet is queued for output and       */
/*                               success is returned immediately (asynchronous).   */
/*                                                                                 */
/*                                                                                 */
/* Description:                                                                    */
/*   This function queues the packet on the port's output queue incurring          */
/*   minimal overhead. This is in contrast to xn_send_to() which copies            */
/*   data from the user's buffer before queueing.                                  */
/*   The packet must have been allocated by xn_pkt_alloc.                          */
/*                                                                                 */
/*   If the protocol is UDP and wait_count is 0 (asynchronous) the packet          */
/*   will be freed automatically after it is sent. Otherwise the user must         */
/*   free the packet after this routine returns by calling xn_pkt_free().          */
/*   The packet must have been bound and connected.                                */
/*                                                                                 */
/*   If the protocol is TCP the packet will be freed automatically after all       */
/*   the data has been acked.  NOTE: TCP will free the packet even if an error     */
/*   is detected.                                                                  */
/*                                                                                 */
/*   For TCP, the socket must have been bound.  For UDP it cannot be connected but */
/*   TCP ignores the destination ip address and port.                              */
/*                                                                                 */
/*    Note: For TCP wait_count determines how long to wait for each packet to be   */
/*          delivered, not the maximum time spent in the routine. The time         */
/*          spent inside the routine can be > wait_count. It is advisable to use   */
/*          xn_send() instead for TCP.                                             */
/*                                                                                 */
/*   For more details see the RTIP Manual.                                         */
/*                                                                                 */
/* Returns:                                                                        */
/*   0 if the packet was sent. -1 otherwise                                        */
/*                                                                                 */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set        */
/*   by this function.  For error values returned by this function see             */
/*   RTIP Reference Manual.                                                        */
/*                                                                                 */

int xn_pkt_send_to(int socket, DCU msg, int len,                     /*__fn__*/
                   PFBYTE to, int to_port, unsigned int wait_count)  /*__fn__*/
{
PANYPORT pport;
#if (INCLUDE_UDP || INCLUDE_RAW)
PIFACE   pi;
byte     send_to_addr[4];   /* could be actual ip dest or gateway ip dest */
RTIP_BOOLEAN  set_prot_hdr;
int      dcu_flags;
#endif
int      ret_val;

    RTIP_API_ENTER(API_XN_PKT_SEND)

#if (!INCLUDE_UDP && !INCLUDE_RAW)
    ARGSUSED_INT(to_port);
    ARGSUSED_PVOID(to);
#endif

    pport = api_sock_to_port(socket);
    if (!pport)
    {
        RTIP_API_EXIT(API_XN_PKT_SEND)
        return(-1);
    }

    /* check if operation is illegal due to send shutdown   */
    if (pport->port_flags & WRITE_SHUTDOWN)
    {
        RTIP_API_EXIT(API_XN_PKT_SEND)
        return(set_errno(ESHUTDOWN));
    }

    if (!msg)
    {
        RTIP_API_EXIT(API_XN_PKT_SEND)
        return(set_errno(EFAULT));
    }

    /* ******   */
#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        if (pport->port_type == UDPPORTTYPE)
            set_prot_hdr = TRUE;
        else
            set_prot_hdr = FALSE;

        if (wait_count)
            dcu_flags = PKT_FLAG_KEEP | PKT_FLAG_SIGNAL;
        else
            dcu_flags = PKT_FLAG_KEEP_ERROR;    /* free packet except upon error */

        /* if send, get IP addr and port from packet   */
        if (!to)
        {
            to_port = ((PUDPPORT)pport)->udp_connection.udp_dest;
            to = ((PUDPPORT)pport)->ip_connection.ip_dest;

            /* send must be connected   */
            if (tc_cmp4(to, ip_nulladdr, 4))
            {
                RTIP_API_EXIT(API_XN_PKT_SEND)
                return(set_errno(ENOTCONN));
            }
        }

        if (!is_frag(msg))
        {
            /* ******                              */
            /* get interface to send packet out on */
            if (tc_udp_get_iface((PUDPPORT)pport, to, (PIFACE KS_FAR *)&pi,
                                 send_to_addr) != 0)
            {
                RTIP_API_EXIT(API_XN_PKT_SEND)
                return(-1);
            }

            /* set up the ETHER, IP and UDP headers except for lengths   */
            /* and checksums                                             */
            /* NOTE: for raw will not write UDP header                   */
            if (tc_udp_set_header((PUDPPORT)pport, msg, to, (word)to_port,
                                  pi, set_prot_hdr, len, len))
            {
                RTIP_API_EXIT(API_XN_PKT_SEND)
                return(-1);
            }

            /* Call packet send; sets up IP and UDP lengths and IP and   */
            /* UDP checksum fields before sending                        */
            /* 0 parameters for to and to_port signify send, not sendto  */
            ret_val = tc_udp_pk_send(pi, (PFBYTE)send_to_addr,
                                  (PUDPPORT)pport,
                                  msg, (word)len, (word)len, (word)wait_count,
                                  dcu_flags);
            RTIP_API_EXIT(API_XN_PKT_SEND)
            return(ret_val);
        }
        else
        {
#if (INCLUDE_FRAG)
            pi = rt_get_iface(to, (PFBYTE)send_to_addr, (PANYPORT)0,
                              (RTIP_BOOLEAN *)0);
            if (!pi)
            {
                DEBUG_LOG("xn_pkt_send - rt_get_iface failed", LEVEL_3, NOVAR, 0, 0);
                INCR_SNMP(IpOutNoRoutes)
                RTIP_API_EXIT(API_XN_PKT_SEND)
                return(set_errno(EADDRNOTAVAIL));
            }

/* tbd: IP len and checksum                                                */
            /* fill in the UDP len and checksum and send out the fragments */
            ret_val = ipf_send_udp_pkt(pi, send_to_addr, (PUDPPORT)pport,
                                    msg, (word)len, (word)wait_count,
                                    dcu_flags);
            RTIP_API_EXIT(API_XN_PKT_SEND)
            return(ret_val);
#else
            RTIP_API_EXIT(API_XN_PKT_SEND)
            return(set_errno(EINVAL));
#endif
        }
    }
    else
#endif

    /* ******   */
#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        if (pport->options & SO_TCP_NO_COPY)
        {
            /* if DCU is invalid or sending pkt larger than MTU value, error   */
            if ( !msg || (len > (int)((PTCPPORT)pport)->out.mss) )
            {
                DEBUG_ERROR("xn_pkt_send - TCP - len, out.mss = ", EBS_INT2,
                    len, ((PTCPPORT)pport)->out.mss);
                RTIP_API_EXIT(API_XN_PKT_SEND)
                return(set_errno(EFAULT));
            }

            TOTAL_TCP_HLEN_NOOPT_SIZE(DCUTOPACKET(msg)->length, pport,
                                      pport->iface, (DCU)0)
            DCUTOPACKET(msg)->length += len;
            if (tc_tcp_write((PTCPPORT)pport, (PFBYTE)0, len, msg, 0,
                             (word)wait_count) == len)
            {
                RTIP_API_EXIT(API_XN_PKT_SEND)
                return(0);
            }
            else
            {
                RTIP_API_EXIT(API_XN_PKT_SEND)
                return(-1);
            }
        }
    }
#endif

    /* ******                                                           */
    /* If we get here it failed either do to incorrect port type or TCP */
    /* port not in no copy mode                                         */
    RTIP_API_EXIT(API_XN_PKT_SEND)
    return(set_errno(EOPNOTSUPPORT));
}

#endif      /* INCLUDE_PKT_API */

/* ********************************************************************   */
/* MEMORY ALLOCALTION                                                     */
/* ********************************************************************   */

/* ********************************************************************          */
/* xn_pkt_alloc() - Allocate a message buffer for the high speed interface       */
/*                                                                               */
/* Summary:                                                                      */
/*   #include "rtipapi.h"                                                        */
/*                                                                               */
/*   DCU xn_pkt_alloc(nbytes, RTIP_BOOLEAN is_802_2)                             */
/*      int nbytes - number of bytes in the data field of dcu to be              */
/*                   allocated                                                   */
/*      RTIP_BOOLEAN is_802_2 - set if packet going to be an 802.2 packet        */
/*                                                                               */
/* Description:                                                                  */
/*   The high speed interface allows data to be sent to a port with minimal      */
/*   data copying. The application copies data directly to the message structure */
/*   which is then placed directly on the output queue.                          */
/*                                                                               */
/*   For more details see the RTIP Manual.                                       */
/*                                                                               */
/* Returns:                                                                      */
/*   A valid message handle or 0 if no messages are available                    */
/*                                                                               */

DCU xn_pkt_alloc(int nbytes, RTIP_BOOLEAN is_802_2)                      /*__fn__*/
{
DCU msg;
PIPPKT pip;

    RTIP_API_ENTER(API_XN_PKT_ALLOC)

#if (!INCLUDE_802_2)
    ARGSUSED_INT(is_802_2)
#endif

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        RTIP_API_EXIT(API_XN_PKT_ALLOC)
        return((DCU)0);
    }
#endif

    msg = os_alloc_packet(nbytes, USER_ALLOC);
    if (!msg)
    {
        set_errno(ENOPKTS);
        RTIP_API_EXIT(API_XN_PKT_ALLOC)
        return((DCU)0);
    }

    /* set up pointer to protocol header and IP address    */
    /* (setup just in case using for xn_pkt_xxx interface) */
    /* NOTE: options not supported for xn_pkt_xxx          */
    DCUTOPACKET(msg)->ip_option_len = 0;

    /* setup IP and protocol header pointers   */
    DCUSETUPPTRS(msg, is_802_2)

    /* clear the packet; especially the fragoff field in the IP header   */
    /* (for send)                                                        */
    tc_memset(DCUTODATA(msg), 0, nbytes);
    pip = DCUTOIPPKT(msg);
    pip->ip_fragoff = 0;

    RTIP_API_EXIT(API_XN_PKT_ALLOC)
    return(msg);

}

/* ********************************************************************     */
/* xn_pkt_free() - frees a packet (DCU)                                     */
/*                                                                          */
/* Implemented as a macro in rtipapi.h                                      */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_pkt_free(msg)                                                   */
/*     DCU msg - packet to be freed                                         */
/*                                                                          */
/* Description:                                                             */
/*   Puts a packet on the free list so it can be allocated again.           */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   0 if successful; -1 if error                                           */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int xn_pkt_free(DCU msg)                                  /*__fn__*/
{

    RTIP_API_ENTER(API_XN_PKT_FREE)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        RTIP_API_EXIT(API_XN_PKT_FREE)
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    if (!msg)
    {
        RTIP_API_EXIT(API_XN_PKT_FREE)
        return(set_errno(EFAULT));
    }

    os_free_packet(msg);
    RTIP_API_EXIT(API_XN_PKT_FREE)
    return(0);
}

#if (INCLUDE_SELECT)
/* *****************************************************                   */
/* USEFUL SELECT UTILITIES                                                 */
/* *****************************************************                   */
/* THE FOLLOWING 3 ROUTINES ARE USEFUL AFTER CALLING SELECT:               */
/* NOTE: select checks if the socket will block, i.e. it is ready or       */
/*       it is in an illegal state where call to read/write/connect/accept */
/*       will not block                                                    */
/* these routines check for legal states; it is useful to call select      */
/* until the socket will not block, then you may call these routines       */
/* to check the reason select returned ready                               */

/* ********************************************************************   */
/* xn_tcp_is_write - checks state of socket                               */
/*                                                                        */
/* Summary:                                                               */
/*   #include "rtipapi.h"                                                 */
/*                                                                        */
/*   RTIP_BOOLEAN xn_tcp_is_write(int socket)                             */
/*      int socket   -  Socket returned by socket                         */
/*                                                                        */
/* Description:                                                           */
/*                                                                        */
/*   Checks if the socket is in a legal state to write data to it.        */
/*                                                                        */
/*   For more details see the RTIP Manual.                                */
/*                                                                        */
/* Returns:                                                               */
/*   returns TRUE if the socket is a TCP socket and it is legal           */
/*   write data to it; returns FALSE if socket is not a TCP socket        */
/*   or not legal to write to the socket                                  */
/*                                                                        */

RTIP_BOOLEAN xn_tcp_is_write(int socket)
{
PANYPORT pport;
RTIP_BOOLEAN ret_val;

    RTIP_API_ENTER(API_XN_TCP_IS_WRITE)

    pport = api_sock_to_port(socket);
    ret_val = FALSE;
    if (pport)
    {
        if (pport->port_type == TCPPORTTYPE)
        {
            if (tc_is_write_state((PTCPPORT)pport))
                   ret_val = TRUE;
        }
    }
    RTIP_API_EXIT(API_XN_TCP_IS_WRITE)
    return(ret_val);

}

/* ********************************************************************   */
/* xn_tcp_is_read - checks state of socket                                */
/*                                                                        */
/* Summary:                                                               */
/*   #include "rtipapi.h"                                                 */
/*                                                                        */
/*   RTIP_BOOLEAN xn_tcp_is_read(int socket)                              */
/*      int socket   -  Socket returned by socket                         */
/*                                                                        */
/* Description:                                                           */
/*                                                                        */
/*   Checks if the socket is in a legal state to read data to it.         */
/*                                                                        */
/*   For more details see the RTIP Manual.                                */
/*                                                                        */
/* Returns:                                                               */
/*   returns TRUE if the socket is a TCP socket and it is legal           */
/*   read data to it; returns FALSE if socket is not a TCP socket         */
/*   or not legal to read to the socket                                   */
/*                                                                        */

RTIP_BOOLEAN xn_tcp_is_read(int socket)
{
PANYPORT pport;
RTIP_BOOLEAN ret_val;

    RTIP_API_ENTER(API_XN_TCP_IS_READ)

    ret_val = FALSE;

    pport = api_sock_to_port(socket);
    if (pport)
    {
        if ( pport && (pport->port_type == TCPPORTTYPE) )
        {
            if (tc_is_read_state((PTCPPORT)pport))
               ret_val = TRUE;
        }
    }
    RTIP_API_EXIT(API_XN_TCP_IS_READ)
    return(ret_val);

}

/* ********************************************************************   */
/* xn_tcp_is_connect - checks state of socket                             */
/*                                                                        */
/* Summary:                                                               */
/*   #include "rtipapi.h"                                                 */
/*                                                                        */
/*   RTIP_BOOLEAN xn_tcp_is_connect(int socket)                           */
/*      int socket   -  Socket returned by socket                         */
/*                                                                        */
/* Description:                                                           */
/*                                                                        */
/*   Checks if the socket is connected.                                   */
/*                                                                        */
/*   For more details see the RTIP Manual.                                */
/*                                                                        */
/* Returns:                                                               */
/*   returns TRUE if the socket is a connected TCP socket.                */
/*   returns FALSE if socket is not a connected TCP socket.               */
/*                                                                        */

RTIP_BOOLEAN xn_tcp_is_connect(int socket)
{
PANYPORT pport;
PTCPPORT tport;
RTIP_BOOLEAN ret_val;

    RTIP_API_ENTER(API_XN_TCP_IS_CONNECT)

    ret_val = FALSE;
    pport = api_sock_to_port(socket);
    if (pport)
    {
        tport = (PTCPPORT)pport;
        if (pport->port_type == TCPPORTTYPE)
        {
            /* if the connection is established, it could be in one of the   */
            /* following states:                                             */
            /*    EST        - established                                   */
            /*    CWAIT      - established and remote host did a closesocket */
            /*    FW1 or FW2 - established and local host did a closesocket  */
            if ((tport->state == TCP_S_EST) || (tport->state == TCP_S_CWAIT) ||
                (tport->state == TCP_S_FW1) || (tport->state == TCP_S_FW2) )
                   ret_val = TRUE;
        }
    }
    RTIP_API_EXIT(API_XN_TCP_IS_CONNECT)
    return(ret_val);
}
#endif      /* INCLUDE_SELECT */

/* ********************************************************************   */
/* ERRNO ROUTINES                                                         */
/* ********************************************************************   */

/* ********************************************************************   */
/* xn_getlasterror() - get errno for the calling task                     */
/*                                                                        */
/* Summary:                                                               */
/*   #include "rtipapi.h"                                                 */
/*                                                                        */
/*   int xn_getlasterror(void)                                            */
/*                                                                        */
/* Description:                                                           */
/*    Returns last errno for the calling task.                            */
/*                                                                        */
/* Returns:                                                               */
/*    Returns errno or -1 upon error                                      */
/*                                                                        */

int xn_getlasterror(void)    /*__fn__*/
{
PSYSTEM_USER user;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        return(set_errno(ENOTINITIALIZED));
    }
#endif

    user = get_system_user();
    return(user->rtip_errno);
}

/* ********************************************************************   */
/* xn_geterror_string() - get string for specified errno                  */
/*                                                                        */
/* Summary:                                                               */
/*   #include "rtipapi.h"                                                 */
/*                                                                        */
/*   PFCCHAR xn_geterror_string(void)                                     */
/*                                                                        */
/* Description:                                                           */
/*    Returns string representation for errno_val.  The errno_val         */
/*    is returned by xn_getlasterror().                                   */
/*                                                                        */
/* Returns:                                                               */
/*    Returns the errno string or a warning message if errno not found    */
/*                                                                        */
/* WARNING: in case the 'unknown' error message is returned, the result   */
/*          is NOT 'multi-thread safe' as other tasks can overwrite       */
/*          the return string. You'll have a valid string at all times    */
/*          though the reported errno number might be incorrect.          */
/*                                                                        */

PFCCHAR xn_geterror_string(int errno_val)   /* __fn__ */
{
#if (INCLUDE_ERRNO_STR)
int i;
#endif

/*  RTIP_API_ENTER(API_XN_GETERROR_STRING)   */

    /* Search errno table for string which describes the errno   */
#if (INCLUDE_ERRNO_STR)
    for (i=0; ; i++)
    {
        if (error_strings_ptr[i-RTIP_ERRNO].errno_val == 0)
            break;
        if (errno_val == error_strings_ptr[i-RTIP_ERRNO].errno_val)
        {
            RTIP_API_EXIT(API_XN_GETERROR_STRING)
            return((PFCCHAR)error_strings_ptr[i-RTIP_ERRNO].errno_string);
        }
    }
#endif

#if (INCLUDE_ERROR_STRING_NUMBER)
    /* fill in errno value in output string... NOT thread-safe, though does NOT crash your system.    */
    /* only the reported number might be 'bad'.                                                       */
    {
        char scratch[13];
        PFCHAR p;
        PFCHAR q;

        p = (PFCHAR)tc_strstr(bad_errno_string, "errno");
        if (p) p += 6; else p = (PFCHAR)bad_errno_string + tc_strlen(bad_errno_string);

        tc_itoa(errno_val, scratch, 10);

        /* construct answer...   */
        tc_movebytes((PFBYTE)errno_buf, (PFBYTE)bad_errno_string,
            (int)(p - bad_errno_string));
        q = errno_buf + (int)(p - bad_errno_string);
        q[tc_strlen(scratch) + tc_strlen(p)] = 0;  /* make sure anyone hits a sentinel! Even while we build a new string! */
        tc_movebytes(q, scratch, tc_strlen(scratch));
        q += tc_strlen(scratch);
        tc_strcpy(q, p);

        RTIP_API_EXIT(API_XN_GETERROR_STRING)
        return errno_buf;
    }
#else
    RTIP_API_EXIT(API_XN_GETERROR_STRING)
    return(bad_errno_string);
#endif
}


#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
/* ********************************************************************                */
/* xn_ip_set_option() - Change an IP option associated with an UDP or TCP socket port. */
/*                                                                                     */
/* Summary:                                                                            */
/*   #include "socket.h"                                                               */
/*                                                                                     */
/*   int xn_ip_set_option(socket, option_name, option_value, optionlen)                */
/*     int socket                - socket returned by socket                           */
/*     int option_name           - options                                             */
/*     PFCCHAR option_value      - buffer which contains values for option             */
/*     int optionlen             - length of option_value                              */
/*                                                                                     */
/* Description:                                                                        */
/*   Allows application set the following IP options associated with a socket:         */
/*      SO_LOOSE_ROUTE_OPTION:                                                         */
/*         option value is of type struct route_info (PROUTE_INFO)                     */
/*      SO_TIMESTAMP_OPTION:                                                           */
/*      SO_RECORD_ROUTE_OPTION:                                                        */
/*         option value is of type int                                                 */
/*      SO_STRICT_ROUTE_OPTION:                                                        */
/*         option value is of type struct route_info (PROUTE_INFO)                     */
/*                                                                                     */
/*      NOTE: when xn_ip_set_option is called, the route_addresses are not             */
/*            copied off into RTIP data area but the address is saved,                 */
/*            therefore, the route_addresses should not be changed by                  */
/*            the application while the routing option is on                           */
/*      NOTE: only one route IP option can be in effect; for example                   */
/*            if the LOOSE route option is turned on and xn_ip_set_option              */
/*            is called to turn on RECORD route, then LOOSE route                      */
/*            will be turned off                                                       */
/*                                                                                     */
/*   For more details see the RTIP Manual.                                             */
/*                                                                                     */
/* Returns:                                                                            */
/*   0 if option set successfully.                                                     */
/*   -1 if an error occurred.                                                          */
/*                                                                                     */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set            */
/*   by this function.  For error values returned by this function see                 */
/*   RTIP Reference Manual.                                                            */
/*                                                                                     */

int xn_ip_set_option(int socket, int option_name,           /*__fn__*/
                     PFCCHAR option_value, int optionlen)   /*__fn__*/
{
PANYPORT pport;
PROUTE_INFO proute;
int ret_val;

    RTIP_API_ENTER(API_SETSOCKOPT)

    pport = api_sock_to_port(socket);
    if (!pport)
    {
        RTIP_API_EXIT(API_SETSOCKOPT)
        return(-1);
    }

    if (!option_value)
    {
        RTIP_API_EXIT(API_SETSOCKOPT)
        return(set_errno(EFAULT));
    }

    switch (option_name)
    {
    case SO_RECORD_ROUTE_OPTION:
        if (optionlen != sizeof(int))
        {
            RTIP_API_EXIT(API_SETSOCKOPT)
            return(set_errno(EFAULT));
        }

        if (*((PFINT)option_value) == 0)
        {
            /* turn option off   */
            pport->ip_options &= ~option_name;
            RTIP_API_EXIT(API_SETSOCKOPT)
            return(0);
        }
        else
        {
            /* turn option on but only allow one of the route options   */
            pport->ip_options = pport->ip_options &
                ~(SO_LOOSE_ROUTE_OPTION | SO_RECORD_ROUTE_OPTION |
                  SO_STRICT_ROUTE_OPTION);

            pport->ip_options |= option_name;
        }
        break;

    case SO_LOOSE_ROUTE_OPTION:
    case SO_STRICT_ROUTE_OPTION:
        if (optionlen != sizeof(ROUTE_INFO))
        {
            RTIP_API_EXIT(API_SETSOCKOPT)
            return(set_errno(EFAULT));
        }

        proute = (PROUTE_INFO)option_value;
        if (proute->route_len == 0)
        {
            /* turn option off   */
            pport->ip_options &= ~option_name;
            RTIP_API_EXIT(API_SETSOCKOPT)
            return(0);
        }
        else
        {
            /* turn option on but only allow one of the route options   */
            pport->ip_options = pport->ip_options &
                ~(SO_LOOSE_ROUTE_OPTION | SO_RECORD_ROUTE_OPTION |
                  SO_STRICT_ROUTE_OPTION);

            pport->ip_options |= option_name;
        }
        break;
    }

    ret_val = ip_set_option(pport, option_name, option_value);
    RTIP_API_EXIT(API_SETSOCKOPT)
    return(ret_val);
}

#endif      /* INCLUDE_IPV4 && INCLUDE_IP_OPTIONS */


#if (INCLUDE_ARP)
/* ********************************************************************      */
/* xn_arp_send() - Sends a gratuitous arp                                    */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "rtipapi.h"                                                    */
/*                                                                           */
/*   int xn_arp_send(interface_no)                                           */
/*      int interface_no - interface number returned by xn_interface_open or */
/*                         xn_interface_atach                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Broadcasts an ARP request to all hosts on the network using its own     */
/*   IP address as the sender and target protocol address in order to force  */
/*   all other hosts on the network to update the entry in their ARP cache   */
/*   for this IP address.                                                    */
/*                                                                           */
/*   For more details see the RTIP Manual.                                   */
/*                                                                           */
/* Returns:                                                                  */
/*   0 if successful or -1 if an error was detected                          */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

/* THIS FUNCTION IS DEFINED AS A MACRO (SEE RTIP.H)   */

/* ********************************************************************     */
/* xn_arp_add() - Add an entry to the arp cache                             */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_arp_add(ipn, ethn, time_to_live)                                */
/*      PFBYTE ipn          - IP address                                    */
/*      PFBYTE ethn         - Ethernet address                              */
/*      dword time_to_live  - number of seconds address should remain in    */
/*                            arp cache (ARPC_INF = INF)                    */
/*                            NOTE: time to live used only if INCLUDE_ARP   */
/*                                  is set                                  */
/*                                                                          */
/* Description:                                                             */
/*                                                                          */
/*   Adds an entry to ARP cache.  If an entry already exists for            */
/*   the specified IP address, the entry is updated with                    */
/*   the specified parameter information.                                   */
/*                                                                          */
/*   If time_to_live parameter is ARPC_INF, the entry never expires.        */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   0 if successful or -1 if an error was detected                         */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

/* THIS FUNCTION IS DEFINED AS A MACRO (SEE RTIP.H)   */

/* ********************************************************************     */
/* xn_arp_add() - Add an entry to the arp cache                             */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "rtipapi.h"                                                   */
/*                                                                          */
/*   int xn_arp_add(ipn)                                                    */
/*      PFBYTE ipn          - IP address                                    */
/*                                                                          */
/* Description:                                                             */
/*                                                                          */
/*   Delete an entry from the ARP cache.                                    */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   0 if successful or -1 if an error was detected                         */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

/* THIS FUNCTION IS DEFINED AS A MACRO (SEE RTIP.H)   */

#endif /* INCLUDE_ARP */

/* ********************************************************************                    */
/* xn_register_callbacks() - register all callbacks from RTIP to application               */
/*                                                                                         */
/* Summary:                                                                                */
/*   #include "rtipapi.h"                                                                  */
/*                                                                                         */
/*   void xn_register_callbacks(PRTIP_CALLBACKS rcb)                                       */
/*         PRTIP_CALLBACKS rcb - structure which contains functions so that                */
/*                               RTIP can make call backs to the application               */
/*                                                                                         */
/*                                                                                         */
/* Description:                                                                            */
/*                                                                                         */
/*   Registers functions in the application so RTIP can call them when                     */
/*   certain events occurs.  The parameter rcb is filled with the                          */
/*   functions which should be called.  If and entry in the parameter rcb                  */
/*   is 0, a callback to the application will not be made for that event.                  */
/*                                                                                         */
/* Returns:                                                                                */
/*   Nothing                                                                               */
/*                                                                                         */
void xn_register_callbacks(PRTIP_CALLBACKS rcb)
{
    rtip_callbacks = rcb;
}

