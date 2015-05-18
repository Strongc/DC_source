/* TASKS.C - RTIP's TIMER and IP Tasks                                  */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"
/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_INTERRUPT_TASK 0

/* ********************************************************************   */
#if (defined(PEGRTIP))
#ifdef __cplusplus
extern "C" {
#endif
void PegPulseTimer(void);
#ifdef __cplusplus
}
#endif

#endif

#if (INCLUDE_RTIP)
/* ********************************************************************   */
#define DEBUG_EBS_TIMER 0       /* set this to 1 to get diag messages */
                                /* for timer   */
#define DEBUG_TIME_TASK 0       /* set this to 1 to implement the counter, */
                                /* time_task_loop, of the number of   */
                                /* times the timer task has run       */

#define DEBUG_TIMER   0         /* set to 1 to get diag messages for */
                                /* timer task   */

/* ********************************************************************   */
#if (DEBUG_TIME_TASK)
long time_task_loop = 0;
#endif

#endif      /* INCLUDE_RTIP */

/* ********************************************************************   */
/* FUNCTION DECLARATIONS                                                  */
#if (INCLUDE_ERTFS_TIMER)
void ertfs_timer_callback(void);
#endif

/* ********************************************************************   */
/* EXERNAL DECLARATIONS                                                   */

#if (INCLUDE_RTIP)

/* ********************************************************************     */
/* tc_ip_main() - This task waits for data to be delivered to it from the   */
/* interface input function. Once a packet is received it calls an OS       */
/* independent funtion ip_interpret to do something with it. The name       */
/* ip_interpret is a misnomer since that routine handle ARP RARP IP and TCP */
void tc_ip_main(int iface_no)                                     /*__fn__*/
{
PIFACE pi;

#if (INCLUDE_SYSLOG)
    xnsl_add_taskindex_to_blockinglist(ks_get_task_index());
#endif

    PI_FROM_OFF(pi, iface_no)

    OS_BIND_IP_SIGNAL(pi);

    for(;;)
    {
        OS_TEST_IP_SIGNAL(pi, RTIP_INF);


        /* **************************************************   */
        tc_ip_interpret(pi);
    }
}

#endif /* (INCLUDE_RTIP) */

/* ********************************************************************   */
void tc_timer_main(void)                      /*__fn__*/
{
int  timer_sec;
word sleep_time;
#if (INCLUDE_RTIP && INCLUDE_TCP) 
int  timer_to;
#endif
#if (INCLUDE_RTIP && INCLUDE_TCP) 
RTIP_BOOLEAN do_to_proc;
#endif


#if (INCLUDE_SYSLOG)
    xnsl_add_taskindex_to_blockinglist(ks_get_task_index());
#endif

    timer_sec = 0;
#    if (INCLUDE_RTIP && INCLUDE_TCP) 
    timer_to = 0;
#    endif

    /* If timer_freq is not set set it to something   */
    if (!timer_freq)
        timer_freq = 1000;

    /* round up   */
    sleep_time = (word)( ((word)timer_freq + ks_msec_p_tick() - (word)1) /
                         ks_msec_p_tick() );

#if (DEBUG_TIMER)
    DEBUG_ERROR("TIMEOUT START time, freq - ", DINT2, 
        ks_get_ticks(), timer_freq);
    DEBUG_ERROR("TIMEOUT START timer_incr - ", DINT1, 
        timer_freq, 0);
    DEBUG_ERROR("TIMEOUT START sleep_time - ", DINT1, 
        sleep_time, 0);
#endif /* DEBUG_TIMER */

    /* We don't unlock the timer task      */
    /* Call periodic maintenance functions */
    for (;;)
    {
        ks_sleep(sleep_time);

#if (DEBUG_TIME_TASK)
        time_task_loop++;
#endif

#if (USE_RTIP_TASKING_PACKAGE)
        if (ks_data_run_timer)
            ks_kernel_timer_callback();
#endif

        timer_sec  += timer_freq;
#        if (INCLUDE_RTIP && INCLUDE_TCP) 
        timer_to   += timer_freq;
#        endif

#if (DEBUG_TIMER)
        DEBUG_LOG("TIMEOUT after sleep, curr time, timer_freq- ", LEVEL_3, DINT2, 
            ks_get_ticks(), timer_freq);
#if (INCLUDE_RTIP && INCLUDE_TCP) 
        DEBUG_LOG("TIMEOUT after sleep, timer_to, timer_sec- ", LEVEL_3, DINT2, 
            timer_to, timer_sec);
#else
        DEBUG_LOG("TIMEOUT after sleep, timer_to, timer_sec- ", LEVEL_3, DINT1, 
            timer_sec, 0);
#endif
#endif

        /* execute registered timeout routines (see ebs_start_timer)   */
        ebs_timeout(ebs_timers);

#if (INCLUDE_RTIP) 
#if (defined(PEGRTIP))
        if (hand_timer_to_peg)
            PegPulseTimer();
#endif              

#        if (INCLUDE_TCP)        
            do_to_proc = FALSE;
            if (timer_to > CFG_TMO_PROC)
            {
                do_to_proc = TRUE;
                timer_to = 0;
            }

            /* perform timeout and delayed ack processing as many times   */
            /* as it asks                                                 */
            while (tc_tcp_timeout(do_to_proc, 
                                  (RTIP_BOOLEAN)(timer_sec >= 1000)))
            {
                ;
            }
#        endif      /* INCLUDE_TCP */
#endif /* (INCLUDE_RTIP) */

        /* do 1 second processing   */
        if (timer_sec >= 1000)
        {
#if (INCLUDE_RTIP)
#            if (INCLUDE_ARP)
                /* Call the arp timer callback   */
                tc_arp_timeout(1);        /* pass 1 sec to arp timeout */
#            endif      /* INCLUDE_ARP */

#            if (INCLUDE_ROUTING_TABLE && INCLUDE_RT_TTL)
                /* routing table maintanence   */
                rt_timer();      
#            endif

#            if (INCLUDE_DNS && INCLUDE_DNS_CACHE)
                /* DNS cache maintanence   */
                dns_timer();
#            endif

#            if (INCLUDE_FRAG)
                /* fragment table maintanence   */
                ipf_timer();
#            endif

            /* execute registered second timeout routines (see ebs_start_timer)   */
            ebs_timeout(ebs_one_sec_timers);

#            if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
                igmp_timer();
#            endif

#            if (INCLUDE_RIP && !INCLUDE_RIP_LISTEN_ONLY)
                _rip_timer();
#            endif

#if (!USE_DB_L0)  
            /* callback to application to print any error messages   */
            /* which occurred during an interrupt service routine    */
            CB_WR_INTERRUPT_STRING();
#endif

            /* check if any xmits have timed out   */
            xmit_done_timer();

#endif /* (INCLUDE_RTIP) */

#            if (INCLUDE_ERTFS_TIMER)
            /* Allow the floppy driver to shut off the motor if it needs to   */
                ertfs_timer_callback();
#            endif

            timer_sec -= 1000;
        }

    }
}


#if (INCLUDE_RTIP)
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS)
/* Note: The rest of this file is only for RTIP. RTFS does not need it   */
/* ********************************************************************  */
/* tc_interrupt_main() - This task is signalled when an input interrupt  */
/* needs processing.                                                     */
/* ********************************************************************  */

void tc_interrupt_main(int iface_no)                      /*__fn__*/
{
PIFACE pi;

#if (DEBUG_INTERRUPT_TASK)
    DEBUG_ERROR("interrupt task started", NOVAR, 0, 0);
#endif

#if (INCLUDE_SYSLOG)
    xnsl_add_taskindex_to_blockinglist(ks_get_task_index());
#endif

    PI_FROM_OFF(pi, iface_no)
    OS_CLEAR_INTERRUPT_SIGNAL(pi);

    /* Wait for a first wakeup by ks_hook_interrupt   */
    if (OS_TEST_INTERRUPT_SIGNAL(pi, RTIP_INF)) 
    {
        ; /* keep compiler happy by doing a test */
    }


#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
    /* For efficiency the uart receive process never returns   */
    while (pi->pdev->iface_type == RS232_IFACE)
    {
        uart_receive(pi->minor_number);

        /* Wait for a first wakeup by ks_hook_interrupt   */
        if (OS_TEST_INTERRUPT_SIGNAL(pi, RTIP_INF)) 
        {
            ; /* keep compiler happy by doing a test */
        }
    }
#endif

#if (DEBUG_INTERRUPT_TASK)
    DEBUG_ERROR("interrupt task, start loop", NOVAR, 0, 0);
#endif
    for(;;)
    {
        /* Wait for an interrupt signal (signalled by calling    */
        /* ks_invoke_interrupt)                                  */
        if (OS_TEST_INTERRUPT_SIGNAL(pi, RTIP_INF)) 
        {
            ; /* keep compiler happy by doing a test */
        }
#if (DEBUG_INTERRUPT_TASK)
{
static long ctr = 0;
    ctr++;
    DEBUG_ERROR("interrupt task, got signal ", DINT1, ctr, 0);
}
#endif

        /* call that strategy routine passed to ks_hook_interrupt   */
        if (rtip_isr_strategy[pi->irq_no])
              rtip_isr_strategy[pi->irq_no](rtip_args[pi->irq_no]);
    }
}
#endif      /* SLIP || CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS */

void break_timer(void)
{
}

/***************************************************************************   */
/* TIMER FUNCTIONS                                                             */
/***************************************************************************   */
/* Process that handles clock ticks                                            */
/* called once per second                                                      */
void ebs_timeout(PTIMER timers)
{
PTIMER t, t1;

    if (!timers)
        return;         /* No active timers, all done  */

    /* loop thru active list of timers                 */
    /* NOTE: all in list should have state = TIMER_RUN */
    for (t = timers; t != (PTIMER)0; t = t1)
    {
        /* get next timer incase function puts same timer on list     */
        /* NOTE: ebs_stop_timer takes it off list and ebs_start_timer */
        /*       always puts it at the beginning of the list so       */
        /*       we won't process the same entry twice                */
        t1 = t->next;
        if (t1 == t)
            break_timer();
#if (DEBUG_EBS_TIMER)
        DEBUG_ERROR("ebs_timeout: t, t1 = ", DINT2, t, t1);
#endif
        if (t->state == TIMER_RUN)
        {
            t->expiration--;
            if (t->expiration < 0)
            {
#if (DEBUG_EBS_TIMER)
                DEBUG_ERROR("ebs_timeout: timer expired: stop timer", 
                    DINT1, t, 0);
#endif
                ebs_stop_timer(t);          /* remove it from list */
                t->state = TIMER_EXPIRE;
                if (t->func)
                {
                    (*t->func)(t->arg);
                }
            }
        }
    }
}

RTIP_BOOLEAN check_timer_on_list(PTIMER t1)
{
PTIMER t;

    for (t = ebs_one_sec_timers; t != (PTIMER)0; t = t->next)
    {
        if (t == t1)
        {
            break_timer();
            return(TRUE);
        }
    }
    for (t = ebs_timers; t != (PTIMER)0; t = t->next)
    {
        if (t == t1)
        {
            break_timer();
            return(TRUE);
        }
    }
    return(FALSE);
}
/***********************************************************************  */
/* Start a timer                                                          */
void ebs_start_timer(PTIMER t)
{
    DEBUG_LOG("ebs_start_timer entered: state, RUN = ", LEVEL_3, 
        EBS_INT2, t->state, TIMER_RUN);
    DEBUG_LOG("ebs_start_timer entered: duration, func = ", 
        LEVEL_3, DINT2, t->duration, t->func);
    if (!t)
        return;

    /* if already running, take off list   */
    if (t->state == TIMER_RUN)
        ebs_stop_timer(t);

    if (t->duration == 0)
        return;     /* A duration value of 0 disables the timer */

    if (check_timer_on_list(t))
    {
        DEBUG_ERROR("ebs_start_timer: timer already on list: ", DINT1, t, 0);
        return;
    }

    t->expiration = t->duration;
    t->state = TIMER_RUN;

    /* Put at beginning of timer list   */
    if (t->is_second)
    {
        t->next = ebs_one_sec_timers;
        ebs_one_sec_timers = t;     
    }
    else
    {
        t->next = ebs_timers;
        ebs_timers = t;     
    }
#if (DEBUG_EBS_TIMER)
    DEBUG_ERROR("ebs_start_timer: timer started (added to list, TIMER_RUN)", 
        DINT1, t, 0);
#endif
    DEBUG_LOG("ebs_start_timer: timer started", LEVEL_3, NOVAR, 0, 0);
}

/***********************************************************************  */
/* Stop a timer - if it is on timer list, take it off                     */
void ebs_stop_timer(PTIMER timer)
{
PTIMER t;
PTIMER tprev = (PTIMER)0;
PTIMER *list_timers;

/*  if (!timer || timer->state != TIMER_RUN)   */
    if (!timer)
    {
#if (DEBUG_EBS_TIMER)
        DEBUG_ERROR("ebs_stop_timer: cannot stop timer, state, t = ", 
            DINT2, timer->state, timer);
#endif
        return;
    }

    /* Verify that timer is really on list   */
    if (timer->is_second)
    {
        list_timers = &ebs_one_sec_timers;
    }
    else
    {
        list_timers = &ebs_timers;
    }

    for (t = *list_timers; t != (PTIMER)0; tprev = t,t = t->next)
    {
        if (t == timer)
            break;
    }

    if (!t)
    {
        /* DEBUG_ERROR("ebs_stop_timer: timer not on list = ", DINT1, timer, 0);   */
        return;     /* This could be ok for CHAP */
    }

#if (DEBUG_EBS_TIMER)
    DEBUG_ERROR("ebs_stop_timer: timer deleted from list", DINT1, t, 0);
#endif

    /* Delete from active timer list   */
    if (tprev)   /* if not first in list */
        tprev->next = t->next;
    else         /* if first on list */
        *list_timers = t->next; 
    
    t->next = 0;
    t->state = TIMER_STOP;
}

/***********************************************************************  */
/* set timer value; interval is number of seconds to run function         */
/* specified by t->func                                                   */
void ebs_set_timer(PTIMER t, int interval, RTIP_BOOLEAN is_sec)
{
    if (!t)
        return;

#if (DEBUG_EBS_TIMER)
    DEBUG_ERROR("ebs_set_timer: timer t to interval: ", DINT2,
        t, interval);
#endif

    if(interval != 0)
        t->duration = interval;
    else
        t->duration = 0;
    t->is_second = is_sec;
    t->state = TIMER_STOP; 
}

#endif  /* (INCLUDE_RTIP) */


