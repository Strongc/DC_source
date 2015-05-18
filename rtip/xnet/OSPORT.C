/*                                                                           */
/* OSPORT.C - OS PORTING functions                                           */
/*                                                                           */
/* EBS - RTIP                                                                */
/*                                                                           */
/* Copyright Peter Van Oudenaren , 1993                                      */
/* All rights reserved.                                                      */
/* This code may not be redistributed in source or linkable object form      */
/* without the consent of its author.                                        */
/*                                                                           */
/*  Module description:                                                      */
/*      This module consists of routines that need porting                   */
/*      to the different operating systems                                   */
/*  You must also port OSTASK.C. This contains os specific code for spawning */
/*  tasks and maintaining thread local user context blocks.                  */

/*
** ks_get_ticks            - Return unsigned long representation of the
                             period in dword format.
                             This routine has implementations for each
                             supported kernel seperated by #ifs.
** ks_ticks_p_sec          - Return the number of system ticks in a second
                             in word format.
                             This routine has implementations for each
                             supported kernel seperated by #ifs.
** ks_msec_p_tick          - Return the number of milliseconds per system
                             tick in word format.
                             This routine has implementations for each
                             supported kernel seperated by #ifs.
** ks_sleep                - Suspend the current task for a number of ticks.
                             This routine has implementations for each
                             supported kernel seperated by #ifs.
** ks_invoke_input         - Queue input packets to the IP layer. This
                             routine is called from the interrupt service
                             layer of device drivers. This routine takes
                             in a pointer to IFACE and a pointer to a
                             message. It must queue the msg onto the ifaces
                             IP exchange.
                             Most kernels allow signalling from the interrupt
                             service layers. In these cases
                             you may call OS_SNDX_IP_EXCHG() directly. For
                             kernels that don't support signalling from
                             interrupts we place the MSG on the iface's
                             input list and invoke a kernel specific
                             link service routine. When the link service
                             routine executes it can remove the MSG from
                             the input list and place it on the IP exchange
                             by executing the macro INPUT_TO_IP_EXCHANGE().
                             Note: - ks_invoke_input filters out packets that
                             can not be processed at a higher level.
** ks_invoke_output        - Signal completion of a network packet send.
                             Device driver send routines may queue a packet
                             for sending and then call os_test_output_signal
                             to wait for the interrupt service routine to
                             signal completion. To signal completion the
                             interrupt service routine calls ks_invoke_output.
                             which must cause a chain of events which
                             eventually cause OS_SET_OUTPUTSIGNAL() to be
                             called. If signals may be set directly from
                             interrupt service ks_invoke_output() may call
                             OS_SET_OUTPUTSIGNAL() directly, otherwise '
                             it should invoke a link service routine to
                             make the call.
                             This routine has implementations for each
                             supported kernel seperated by #ifs.
** ks_invoke_interrupt     - Signal interrupt needs processing.  Only minimal
                             amount of processing is done at the interrupt
                             level, the rest is done at the task level.
*/
/* ********************************************************************   */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"
#if (INCLUDE_MFS)
#include "memfile.h"
#endif
#if (INCLUDE_ERTFS_PRO)
extern dword ertfssemnexthandle;
extern dword ertfssignexthandle;
extern KS_RTIPSIG ertfssigs[ERTFS_PRO_NSIGS];
extern KS_RTIPSEM ertfssems[ERTFS_PRO_NSEMS];
#endif

/* ********************************************************************   */
RTIP_BOOLEAN resource_init(void);
RTIP_BOOLEAN is_resource_initted(void);

#if (INCLUDE_ERTFS_EXIT_TASK)
void pc_free_user(void);
#endif

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* NOTE: the following macros are used by this file only;                 */
/*       THESE DO NOT NEED PORTING                                        */
/* ********************************************************************   */

#if (NOSTRIP )
/* ********************************************************************   */
/* os_rcvx_input_list() - remove the first DCU from input list            */
/*                                                                        */
/*   This function removes the first DCU in the end of the doubly         */
/*   input list for the interface pi.                                     */
/*                                                                        */
/*   Returns the DCU which was removed.                                   */
/*                                                                        */

DCU  os_rcvx_input_list(PIFACE pi)               /*__fn__*/
{
DCU msg;
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */

    msg = 0;
    sp = ks_splx(); /* Disable and  push interrupts */
    if (pi->ctrl.list_input)
    {
        msg = (DCU) pi->ctrl.list_input;
        pi->ctrl.list_input = os_list_remove_off(pi->ctrl.list_input,
                                                 pi->ctrl.list_input,
                                                 PACKET_OFFSET);
    }
    ks_spl(sp);
    return(msg);
}

/* Kernel independent macro that dequeues a message from the input list and
   places it on the ip exchange
   NOTE: !! - This macro is only used by link service routines of real time kernels
   that can signal directly from the interrupt service layer */

#define INPUT_TO_IP_EXCHANGE(PI, MSG) \
    if ( PI && ((MSG = os_rcvx_input_list(PI)) != (DCU)0) ) OS_SNDX_IP_EXCHG(PI, MSG);

#endif /* (SMXNET || PLUS || ECOS || NOSTRIP) */

/* Kernel independent macro that signals IP later (kernel independent)   */
#define SIGNAL_OUTPUT(PI) if (PI) OS_SET_IP_SIGNAL(PI);

/* Kernel independent macro that signals INTERRUPT task (kernel independent)   */
#define SIGNAL_INTERRUPT(PI) OS_SET_INTERRUPT_SIGNAL(PI);

#endif      /* INCLUDE_RTIP */

/* ********************************************************************   */
/* FUNCTION DECLARATIONS                                                  */

#if (INCLUDE_RTIP)
void tc_ip_(void);
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS)
void tc_interrupt_(void);
#endif      /* SLIP || CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS */

#endif      /* INCLUDE_RTIP */

void ks_yield(void);
void tc_timer_(void);
void tc_timer_main(void);

/* ********************************************************************   */
/* EXTERNAL DATA                                                          */

#if (INCLUDE_RTIP)

extern int KS_FAR static_iface_no;
extern int KS_FAR static_interrupt_no;
#endif /* INCLUDE_RTIP */

/* ********************************************************************   */
/*
**  Kernel dependent routines - These routines provide link services
**  and task entry points. A smapl number of lines of code (30 or so)
**  are required per kernel. These sections are seperated by #if
**  statements. To add another kernel a new kernel specific section
**  should be added.
**
**  The following services are required:
**  1. A timer task - This high priority task will be started by
**                    ks_resource_init(). It should immediately
**                    call the kernel independent function timer_main().
**  2. IP tasks     - One IP dispatch task is required per interface.
**                    There should be one IP task started per interface.
**                    The task should immediately call tc_ip_main()
**                    with interface number as an argument. These
**                    tasks will be started by ks_resource_int().
**  3. Link services- If the kernel requires special link services
**                    to implement ks_invoke_input() and
**                    ks_invoke_output() those routines should be
**                    placed here.
**/
/* ********************************************************************          */
/* RESOURCE INITIALIZATION                                                       */
/* ********************************************************************          */
/* ks_resource_init  -  This routine must activate the kernel services           */
/* required by RTIP. When this routine is completed the following                */
/* services must be available:                                                   */
/*      semaphores -                                                             */
/*                         -   RTIPSEM ifsem[]     - One per interface           */
/*                         -   RTIPSEM criticalsem - One                         */
/*                         -   RTIPSEM tablesem      - One                       */
/*                         -   RTIPSEM memfilesem  - One                         */
/*                         -   RTIPSEM tcpsem      - One                         */
/*                         -   RTIPSEM udpsem      - One                         */
/*                         -   RTIPSEM syslogsem   - One                         */
/*      If ERTFS is included the following semaphores are created                */
/*                         -   RTIPSEM fs_drivesem[]    - One per logical drive  */
/*                         -   RTIPSEM fs_iosem[]       - One per logical drive  */
/*                         -   RTIPSEM fs_critsem       - One                    */
/*      signals    -  Each interface requires a few signals as does each         */
/*                    UDP and TCP socket. There also is one signal for INTERRUPT */
/*                         -    RTIPSIG ifsig[][]                                */
/*                         -    RTIPSIG portsig[][]                              */
/*                                                                               */
/*      tasks       - There must be one timer task runnning and one              */
/*                    IP task per interface.                                     */
/*                                                                               */
/*      link service routines -  If your kernel requires link service routines   */
/*                    to send signals from interrupt service routines, they      */
/*                    should be initialized from os_kernel_init().               */
/*                                                                               */

RTIP_BOOLEAN ks_resource_init(void)   /*__fn__*/
{
RTIP_BOOLEAN ret_val;

    /* if already initialized, do nothing; this is not considered an   */
    /* error                                                           */
    if (is_resource_initted()) {
        return(TRUE);
    }

    /* initialize global data   */
    ret_val = resource_init();
    resource_initialized = INIT_DONE;
    return(ret_val);
}

/* ********************************************************************   */
/* is_resource_initted() - perform synchronization of initializing RTIP   */
/* returns TRUE if initialization done by another task; returns           */
/* FALSE if caller needs to perform the initialization                    */
RTIP_BOOLEAN is_resource_initted(void)
{
RTIP_BOOLEAN ret_val;
int i;

    ret_val = TRUE;     /* initialization done by another task */
    ks_disable();       /* Disable interrupts */

    /* check if initialization has not been started, then it should   */
    /* be done by the caller                                          */
    if (resource_initialized == INIT_NOT_DONE)
    {
        resource_initialized = INIT_IN_PROG;
        ret_val = FALSE;    /* caller should initialize */
    }

    ks_enable();

    /* if another task is initializing, wait at most 5 seconds for it   */
    /* to complete                                                      */
    if (ret_val)
    {
        for (i=0; i < 5; i++)
        {
            if (resource_initialized == INIT_DONE)
                break;
            ks_sleep(ks_ticks_p_sec());
        }
        DEBUG_ASSERT(i<5, "is_resource_initted: rtip not init withing 5 seconds",
            NOVAR, 0, 0);
    }

    return(ret_val);
}

/* ********************************************************************   */

RTIP_BOOLEAN resource_init(void)
{
int i;
#if (INCLUDE_RTIP)
int j;
#endif
PIFACE pi;
#if (INCLUDE_RUN_TIME_CONFIG)
int k;
#endif
#if (INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
PANYPORT port;
int num_proto;
int num_ports;
#endif


#if (defined(NEWSEMMETHOD) )
    /* Initialize the semaphore and signal tables   */
    if (!ks_kernel_semalloc_init()) 
    {
        return(FALSE);
    }
    if (!ks_kernel_sigalloc_init()) 
    {
        return(FALSE);
    }
#endif

#if (INCLUDE_ERTFS_PRO)
    /* Initialize the handle to 1 since 0 is an error condition   */
    ertfssemnexthandle = 1;
    for (i = 0; i < ERTFS_PRO_NSEMS; i++)
    {
        KS_SEMAPHORE_BUILD(ertfssems[i])
    }
#endif

#if (INCLUDE_ERTFS_SIGNALS)
    /* Build RTFS Semaphores if the file system is included;   */
    /* one per drive for accessing the fat and finodes         */
    /* one per drive to keep IO single threaded.               */
    /* NOTE: Since two logical drives can share the same       */
    /* interface at run time we calculate which semaphore to   */
    /* wait on when before we access the controller.           */

    for (i = 0; i < NDRIVES; i++)
    {
        KS_SEMAPHORE_BUILD(fs_drivesem[i])
        KS_SEMAPHORE_BUILD(fs_iosem[i])
    }
    /* Semaphores for exclusive access to the buffer pool and other   */
    /* critical regions..                                             */
    KS_SEMAPHORE_BUILD(fs_critsem)
#endif


#if (defined(PEGRTIP))
    for (i = 0; i < NUM_PEG_SEM; i++)
    {
        KS_SEMAPHORE_BUILD(ks_peg_sem[i])
    }
    for (i = 0; i < NUM_PEG_EVENTS; i++)
    {
        KS_SIGNAL_BUILD(ks_peg_sig[i])
    }

#endif


#if (INCLUDE_RTIP)

    /* Create a semaphore for critical sections   */
    KS_SEMAPHORE_BUILD(criticalsem)

    /* Create a semaphore the memory file system  */
    KS_SEMAPHORE_BUILD(memfilesem)

    /* Create UDP, TCP, and TABLE semaphores for critical sections   */
    KS_SEMAPHORE_BUILD(tablesem)
    KS_SEMAPHORE_BUILD(udpsem)
    KS_SEMAPHORE_BUILD(tcpsem)
    KS_SEMAPHORE_BUILD(syslogsem)

    for (i = 0; i < CFG_NIFACES; i++)
    {
        /* IFACE semaphore for critical sections   */
        PI_FROM_OFF(pi, i)
        KS_SEMAPHORE_BUILD(pi->ctrl.ifsem)

        /* Create IP RARP PING and OUTPUT signals   */
        for (j = 0; j < NUM_SIG_PER_IFACE; j++)
        {
            KS_SIGNAL_BUILD(IFSIG(pi, j))
        }

    }

#if (!INCLUDE_MALLOC_PORTS)
    /* Create write, read, send & select signals for each port   */
#if (INCLUDE_RUN_TIME_CONFIG)
    num_proto = 2;      /* TCP, UDP */
#if (!INCLUDE_TCP && !INCLUDE_UDP)
    num_proto = 0;
#elif (!INCLUDE_TCP || !INCLUDE_UDP)
    num_proto = 1;
#endif

    for (k=0; k<num_proto; k++)
    {
#if (INCLUDE_UDP)
        num_ports = CFG_NUDPPORTS;
#endif
#if (INCLUDE_TCP)
        if (k == 0)     /* 0=TCP, 1=UDP */
            num_ports = CFG_NTCPPORTS;
#endif
        for (i = 0; i < num_ports; i++)
        {
#if (INCLUDE_UDP)
            port = (PANYPORT)&tc_udpp_pool[i]; /* UDP ports */
#endif
#if (INCLUDE_TCP)
            if (k == 0)
                port = (PANYPORT)&tc_tcpp_pool[i]; /* TCP ports */
#endif
            for (j = 0; j < NUM_SIG_PER_PORT; j++)
            {
                /* Create write, read, send & select signals for each port   */
                KS_SIGNAL_BUILD(port->portsig[j])
            }
        }
    }
#else

    DEBUG_ERROR("TOTAL_PORTS, NUM_SIG_PER_PORT: ", EBS_INT2,
        TOTAL_PORTS, NUM_SIG_PER_PORT);
        
    for (i = 0; i < TOTAL_PORTS; i++)
    {
        for (j = 0; j < NUM_SIG_PER_PORT; j++)
        {
            KS_SIGNAL_BUILD(portsig[i][j])
        }
    }
#endif
#endif  /* !INCLUDE_MALLOC_PORTS */

#endif /* (INCLUDE_RTIP) */

#if (INCLUDE_ERTFS_PRO)
    /* Initialize the handle to 1 since 0 is an error condition   */
    ertfssignexthandle = 1;
    for (i = 0; i < ERTFS_PRO_NSIGS; i++)
    {
        KS_SIGNAL_BUILD(ertfssigs[i])
    }
#endif

#if (INCLUDE_ERTFS_SIGNALS)
    /* Build RTFS Interrupt signals if the file system is included   */
#if (USE_ATA)
    for (i = 0; i < N_ATA_CONTROLLERS; i++)
    {
         KS_SIGNAL_BUILD(idesig[i])
    }
#endif

#if (USE_FLOPPY)
    for (i = 0; i < N_FLOPPY_UNITS; i++)
    {
         KS_SIGNAL_BUILD(floppysig[i])
    }
#endif
#endif


#if (INCLUDE_RTIP)

    /* start the IP tasks   */
    for (i = 0; i < CFG_NIFACES; i++)
    {
        if (!os_spawn_task(TASKCLASS_IP,tc_ip_, 0,0,0,0))
        {

            DEBUG_ERROR("ks_resource_init - spawn IP task failed - IP task no = ",
                EBS_INT1, i, 0);
            return(FALSE);
        }

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS)
        /* spawn interrupt processing tasks   */
        if (!os_spawn_task(TASKCLASS_INTERRUPT,tc_interrupt_, 0,0,0,0))
        {
            DEBUG_ERROR("ks_resource_init - spawn INTERRUPT task failed - INTERRUPT task no = ",
                EBS_INT1, i, 0);
            return(FALSE);
        }
#endif      /* SLIP || CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS */
    }
#endif /* (INCLUDE_RTIP) */

    /* start the timer task   */
    if (!os_spawn_task(TASKCLASS_TIMER,tc_timer_, 0,0,0,0))
    {
        DEBUG_ERROR("ks_resource_init - spawn timer task failed",
            NOVAR, 0, 0);
        return(FALSE);
    }



    return(TRUE);
}


/* ********************************************************************   */
/* TICK AND SLEEP ROUTINES                                                */
/* ********************************************************************   */

/* ********************************************************************   */
/* ks_get_ticks() - get period since system startup                       */
/*                                                                        */
/*   Returns number of ticks since system startup                         */
/*                                                                        */

dword ks_get_ticks(void)
{
#if (defined(EMBOS))
  return((dword) OS_GetTime32());

#endif
}

/* ********************************************************************   */
/* ks_ticks_p_sec(void) - ticks per second                                */
/*                                                                        */
/*   Returns number of ticks per second                                   */
/*                                                                        */
word ks_ticks_p_sec(void)
{
#if (defined(EMBOS))
    return(1000);
#endif
}

/* ********************************************************************   */
/* ks_msec_p_tick() - milliseconds per tick                               */
/*                                                                        */
/*   Returns number of milliseconds per tick                              */
/*                                                                        */
word ks_msec_p_tick(void)       /*__fn__*/
{
#if (defined(EMBOS))
    return(1);

#endif
}

/* ********************************************************************   */
/* ks_sleep() - sleep number of ticks                                     */
/*                                                                        */
/*   Seeps number of ticks specified.  If ticks is 0, task yields.        */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */
void ks_sleep(word no_ticks)
{
    if (no_ticks == 0)
    {
        ks_yield();
        return;
    }

#if (defined(EMBOS))
    OS_Delay((int) no_ticks);

#endif
}

/* ********************************************************************   */
/* ks_yield() - yields                                                    */
/*                                                                        */
/*   Current task yield to any scheduled tasks.                           */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */
void ks_yield(void)
{
#if (defined(EMBOS))
/*  ROLF - How do we do this   */
#endif
}


/* ********************************************************************   */
/* INVOKE                                                                 */
/* ********************************************************************   */
#if (INCLUDE_ERTFS_INTERRUPTS)
    /* Build RTFS Interrupt signals if the file system is included   */
#if (USE_ATA)
void ks_invoke_ide_interrupt(int controller_no)
{
    OS_IDE_SIGNAL_SET(controller_no); /* Set the signal directly from the ISR */
                                      /* because signals are legal from isr   */
}
#endif  /* USE_ATA */


#if (USE_FLOPPY)
void ks_invoke_floppy_interrupt(int controller_no)
{
    OS_FLOPPY_SIGNAL_SET(controller_no); /* Set the signal directly from the ISR */
                                    /* because signals are legal from isr   */
}
#endif /* USE_FLOPPY */
#endif /* INCLUDE_ERTFS_INTERRUPTS */

#if (INCLUDE_RTIP)

/* ********************************************************************   */
void ks_invoke_input(PIFACE pi, DCU msg)
{
PETHER  pe;
PARPPKT pa;
#if (!INCLUDE_RAW)
PIPPKT  pip;
#endif
word    type;
RTIP_BOOLEAN toss_it;
#if (NOSTRIP)
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */
#endif
#if (INCLUDE_TOKEN || INCLUDE_802_2)
PSNAP   psnap;
#endif
    /* **************************************************   */
    /* issolate type of packet for filtering the packet     */
    {
        pe = DCUTOETHERPKT(msg);
        type = pe->eth_type;
#if (INCLUDE_802_2)
        if (msg_is_802_2(msg))
        {
            psnap = DCUTOSNAP8022(msg);
            type = psnap->snap_type;
        }
#endif
    }

    /* **************************************************          */
    /* First determine if can filter the packet at driver layer    */
    /* Default to toss_it equal true                               */
    toss_it = TRUE;

    if (type == EIP_68K)
    {
#if (INCLUDE_RAW)
        toss_it = FALSE;

#else   /* !INCLUDE_RAW */
        pip = (PIPPKT)(((PFBYTE)(msg->data))+ETH_HLEN_BYTES);
        switch (pip->ip_proto)
        {
#if (INCLUDE_UDP)
        case PROTUDP:
            /* Toss UDP broadcast that we aren't looking for them   */
            if (!allow_udp_broadcasts)
            {
                if (IS_ETH_ADDR_EQUAL(pe->eth_dest, broadaddr))
                {
                    /* toss packet, i.e. do not fall thru to set   */
                    /* toss to FALSE                               */
                    break;
                }
            }
            /* fall thru   */
#endif
#if (INCLUDE_TCP)
        case PROTTCP:
            /* fall thru   */
#endif
#if (INCLUDE_ICMP || INCLUDE_PING)
        case PROTICMP:
            /* fall thru   */
#endif
#if (INCLUDE_IGMP)
        case PROTIGMP:
#endif
            toss_it = FALSE;
        }       /* end of switch */
#endif          /* INCLUDE_RAW */
    }

    else if (type == EARP_68K)
    {
        /* Toss arp requests if they are not for us   */
        pa = ((PARPPKT)  (((PFBYTE)(msg->data))+ETH_HLEN_BYTES) );
        if ((pa->ar_opcode == ARPREQ_68K) &&
            !tc_cmp4(pi->addr.my_ip_addr, pa->ar_tpa, 4) )
        {
            toss_it = TRUE;
        }
        else
            toss_it = FALSE;
    }
    else if (type == ERARP_68K)
    {
        /* assume these rarely happen so let it pass   */
        toss_it = FALSE;
    }
#if (INCLUDE_PPPOE)
    else if ( (type == PPPOED_68K) || (type == PPPOES_68K) )
        toss_it = FALSE;
#endif

    /* **************************************************   */
    if (toss_it)
    {
        os_free_packet(msg);
        return;
    }

    /* **************************************************              */
/* / !!! NOTE: MUST CHANGE PACKET DRIVER TO SUPPORT THIS               */
/* For kernels that need an intermediate QUEUE put the packet there    */
/* Invoke a link service routine where appropriate   */
    /* Send the message directly to the IP exchange   */
    OS_SNDX_IP_EXCHG(pi, msg);
}


/* ********************************************************************   */
/* signal IP layer that send is done; can be called from ISR              */
void ks_invoke_output(PIFACE pi, int xmit_complete_cnt)
{
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */

#if (!INCLUDE_XMIT_QUE)
    ARGSUSED_INT(xmit_complete_cnt)
#endif

    sp = ks_splx(); /* Disable and  push interrupts */
    /* signal IP layer that send is done   */
#if (INCLUDE_XMIT_QUE)
    pi->xmit_done_counter += xmit_complete_cnt;
#else
    pi->xmit_done_counter++;
#endif
    ks_spl(sp);

    SIGNAL_OUTPUT(pi)
}

/* ********************************************************************   */
void ks_invoke_interrupt(PIFACE pi)     /*__fn__*/
{
    SIGNAL_INTERRUPT(pi)        /* Signal directly  because signals are */
                                /* legal from isr   */
}

#endif /* (INCLUDE_RTIP) */

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* MAIN PROGRAMS FOR SPAWNED TASKS                                        */
/* NOTE: the tasks spawned are in tasks.c                                 */
/* NOTE: these do not need porting                                        */
/* ********************************************************************   */

/* IP TASK - one is spawned per interface at initialization     */
/* not all OSs have the ability to pass a parameter, therefore, */
/* use a global variable                                        */
void tc_ip_(void)
{
    tc_ip_main(static_iface_no++);    /*__fn__*/
}

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS)
/* INTERRUPT TASK - one is spawned per interrupt interface at initialization   */
/* not all OSs have the ability to pass a parameter, therefore,                */
/* use a global variable                                                       */
void tc_interrupt_(void)
{
    tc_interrupt_main(static_interrupt_no++);    /*__fn__*/
}
#endif      /* SLIP || CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS */
#endif /* (INCLUDE_RTIP) */

/* TIMER TASK - one of these is spawned at initialization   */
void tc_timer_(void)                      /*__fn__*/
{
    tc_timer_main();
}


/* ********************************************************************   */
/* Kernel specific initialization function.
* This routine is called by the EBS demo applications when they are
* first entered, i.e. from the main program.
* If the kernel has an initializer funtion that must
* be called first then this routine should execute that routine.
* If the kernel runtime environment is already running when
* the EBS demo applications are entered then this routine should
* do nothing and simply return success
*
* Returns 0 on success -1 on failure
*/

#if (KS_DECLARE_STACK)
void    ks_kernel_stackalloc_init(void);
#endif

int ks_kernel_init(void)
{
    /* Operating system initialization code   */
#if (defined(EMBOS))
  OS_InitKern();        /* initialize OS */
  OS_InitHW();          /* initialize Hardware for OS */
#endif

    return (0);
}


/* Kernel specific enter run mode function.
* This routine is called by the EBS demo application from the main
* routine after it has spawned the main task and is ready to execute.
* This routine should never return.
* For some kernels the appropriate thing to do is
* call the kernel's scheduler loop. For others the appropriate
* behavior may be to block indefinately
*/

void ks_kernel_run(void)
{
    /* **************************************   */
#if (defined(EMBOS))
  OS_Start();           /* Start multitasking */

#else
#error Implement ks_kernel_run(void) in OSPORT.C, look at ks_kernel_init() too.
#endif
}

#if (defined(NEWSEMMETHOD)) /* Setting this to ISUCOS for now. Later for all */
/* ********************************************************************   */
/* NEWSEMMETHOD                                                           */
/* ********************************************************************   */
#define CFG_N_SIGNALS 200
#define CFG_N_SEMAPHORES 16

/* Used to managed allocating and freeing of mutex semaphores   */
struct  sem_manager
{
    int       is_free;
#if (EMBOS)
    OS_RSEMA    thesem;
#else
    KS_RTIPSEM thesem;
#endif
};
struct  sem_manager KS_FAR sem_array[CFG_N_SEMAPHORES];

/* This must be called from ks_resource_init.. first   */
RTIP_BOOLEAN ks_kernel_semalloc_init(void)
{
int i;

    for (i = 0; i < CFG_N_SEMAPHORES; i++)
    {
        sem_array[i].is_free = 1;
#if (EMBOS)
            OS_CREATERSEMA(&sem_array[i].thesem);
#else
#error Define your semaphore allocate function in OSPORT.C
#endif
    }
    return(TRUE);
}

int ks_kernel_semalloc(void)
{
int i;
    for (i = 0; i < CFG_N_SEMAPHORES; i++)
    {
        if(sem_array[i].is_free)
        {
            sem_array[i].is_free = 0;
            return(i);
        }
    }
    return(-1);
}

void ks_kernel_semfree(int semnum)
{
    sem_array[semnum].is_free = 1;
}

RTIP_BOOLEAN ks_kernel_semtest(int semnum)
{
RTIP_BOOLEAN claimed = FALSE;
#if (EMBOS)
            OS_Use(&sem_array[semnum].thesem);
            claimed = TRUE;
#else

#define your wait for a semaphore function here
#endif
        return(claimed);
}


void ks_kernel_semgive(int semnum)
{
#if (EMBOS)
        OS_Unuse(&sem_array[semnum].thesem);
#else
#error Implement void ks_kernel_semgive(int semnum) in OSPORT.C
#endif
}


/* Used to managed allocating and freeing of couning semaphores (signals)   */
struct  sig_manager {
    int       is_free;
#if (EMBOS)
    OS_CSEMA  thesig;
#else
    KS_RTIPSIG thesig;
#endif
};
struct  sig_manager KS_FAR sig_array[CFG_N_SIGNALS];



/*                                           */
/* This must be called from ks_resource init */
/*                                           */
/*                                           */
/*                                           */
RTIP_BOOLEAN ks_kernel_sigalloc_init(void)
{
int i;

    for (i = 0; i < CFG_N_SIGNALS; i++)
    {
        sig_array[i].is_free = 1;
#if (EMBOS)
        OS_CREATECSEMA(&sig_array[i].thesig);
#else
#error Implement ks_kernel_sigalloc_init in OSPORT.C
#endif
    }
    return(TRUE);
}

/*                                                          */
/* Call this when we allocate signals from ks_resource_init */
/*                                                          */
/*                                                          */
int ks_kernel_sigalloc(void)
{
int i;
    for (i = 0; i < CFG_N_SIGNALS; i++)
    {
        if(sig_array[i].is_free)
        {
            sig_array[i].is_free = 0;
            return(i);
        }
    }
    return(-1);
}

void ks_kernel_sigfree(int signum)
{
    sig_array[signum].is_free = 1;
}

RTIP_BOOLEAN ks_kernel_sigtest(int signum, word timeout)
{
int signalled;

#if (EMBOS)
        if (timeout == RTIP_INF)
        {
            OS_WaitCSema(&sig_array[signum].thesig);
            signalled = TRUE;
        }
        else
        {
            word timeout_this;
            if (timeout <= 32765)
                signalled = OS_WaitCSema_Timed(&sig_array[signum].thesig, timeout);
            else
            {
                /* embOS doesn't work with large timeout values so devise
                   a workaround */
                do
                {
                    if (timeout <= 32765)
                        timeout_this = timeout;
                    else
                        timeout_this = 32765;
                    signalled = OS_WaitCSema_Timed(&sig_array[signum].thesig, timeout_this);
                    timeout -= timeout_this;
                } while (!signalled && timeout);
            }
        }

#else
#define your wait for a signal test function here
#endif
    return((RTIP_BOOLEAN)signalled);
}

RTIP_BOOLEAN ks_kernel_sigclear(int signum)
{
    while (ks_kernel_sigtest(signum,0))
        ;
    return(TRUE);
}




void ks_kernel_sigset(int signum, RTIP_BOOLEAN from_isr)
{
#if (EMBOS)
    OS_SignalCSema(&sig_array[signum].thesig);
#else
#error Implement your signal set in OSPORT.C
#endif
}

#endif /* NEWSEMMETHOD */

#if (INCLUDE_ERTFS_PRO)
/* This code should perhaps be placed in another file   */
dword ks_alloc_mutex_handle(void)
{
    return(ertfssemnexthandle++);
}
void ks_claim_mutex_handle(dword handle)
{
    KS_SEMAPHORE_GET(ertfssems[handle-1]);
}
void ks_release_mutex_handle(dword handle)
{
    KS_SEMAPHORE_GIVE(ertfssems[handle-1]);
}
dword ks_alloc_signal_handle(void)
{
    return(ertfssignexthandle++);
}
void ks_clear_signal_handle(dword handle)
{
    KS_SIGNAL_CLEAR(ertfssigs[handle-1]);
}
int ks_test_signal_handle(dword handle, int timeout)
{
    if (KS_SIGNAL_GET(ertfssigs[handle-1], (word)timeout))
        return(0);
    else
        return(-1);
}
void ks_set_signal_handle(dword handle)
{
    KS_SIGNAL_SET(ertfssigs[handle-1]);
}
/*
*  Hook an ERTFS interrupt. 
*  c_int_handler is the C routine to call.
*  intargs is the argument to pass
*  irq is the interrupt number (6 on floppy, 15 or 14 on ATA, 
*  10 perhaps on PCMCIA 
*  whichdevice - What are we hooking ?
*  1 == pcmcia controller
*  2 == ide controller
*  3 == floppy controllerm
*   
*/
void ks_hook_rtfs_interrupt_handle(void (*c_int_handler)(int), int intarg, int irq, int which_device)
{
    ks_hook_interrupt(irq, (PFVOID) 0, 
                     (RTIPINTFN_POINTER) c_int_handler, 
                     (RTIPINTFN_POINTER) 0, intarg);
}

dword ks_get_taskid_handle(void)
{
    return( (dword) os_get_user());
}


void    ks_rtfs_promptstring(byte * buffer)
{
    tm_gets(buffer);
}


void ks_rtfs_cputs(byte *buffer)
{
    tm_cputs((PFCCHAR) buffer);
}
#endif


