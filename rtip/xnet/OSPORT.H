/*                                                                        */
/* osport.h                                                               */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993,1994,1995                       */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains macros which must be tailored to the         */
/*        target kernel and cpu.                                          */
/*                                                                        */
/*   Date                    Supported Kernel     #define to enable       */
/* March 1995                SMX Real mode large  SMXNET                  */
/* March 1995                Nucleus PLUS         PLUS                    */
/* March 1995                Polled version       POLLOS                  */
/*                                                                        */
/*                                                                        */

#ifndef __OS_PORT__
#define __OS_PORT__ 1



/* ********************************************************************   */
/* INFINITE DECLARATION FOR SIGNALS AND SEMAPHORES                        */
/* ********************************************************************   */
/*
** INF   - Value passed to SIGNAL_GET to indicate an infinite time
**
*/
#define RTIP_INF     0xffff          /* infinite wait count */

/* ********************************************************************   */
/* SIGNALS                                                                */
/* ********************************************************************   */
/*
** Definitions:
** KS_RTIPSIG - An object used to signal the occurance of events.
**              The signal must have an initial value of 0 (no event).
**              The set event operator should must increment the
**              event count when called. The request event operator
**              SIGNAL_GET() must decrement the signal count before it returns.
**              If a signal is already pending SIGNAL_GET() must decrement
**              the count immediately and return, otherwise it must block
**              until time expires or the event signal is signaled.
**              Most kernels will support KS_RTIPSIG with a counting semaphore
**              initialized to zero. Simpler kernels will use an event
**              queue.
**
** Implementation:
** KS_RTIPSIG              - Use define to declare the type for a signal
**                         - This type is used to declare signal
**                         - in os_port.c. The handles are initialized in
**                         - the os specific function os_resource_init()
**                         - in os_port.c. Note: these "handles" are only
**                         - accessed through macros in this file giving
**                         - maximum flexibility.
**                         - KS_RTIPSIG ifsig[][];
**                         - KS_RTIPSIG portsig[][];
**                         -   These handles are passed as arguments to the
**                         -   macros:
**                         -    KS_SIGNAL_BIND(X)
**                         -    KS_SIGNAL_SET(SIGNAL)
**                         -    KS_SIGNAL_GET(SIGNAL,T)
**
** KS_SIGNAL_GET(SIGNAL,T) - This macro blocks on a signal until it is set
**                         - by KS_SIGNAL_SET(), cleared by KS_SIGNAL_CLEAR()
**                         - or Time T in system ticks expires. If T is the
**                         - special value (RTIP_INF) the timeout value is
**                         - infinite. KS_SIGNAL_GET() must evaluate to true (1)
**                         - if the signal was set or false (0) if time
**                         - expired or KS_SIGNAL_CLEAR was called.
** KS_SIGNAL_CLEAR(SIGNAL) - Clear the signal. If anyone is already blocked
**                         - on the signal wake them up.
** KS_SIGNAL_BIND(X)       - This MACRO is guaranteed to be invoked before
**                         - KS_SIGNAL_GET() is called. For kernels where signals
**                         - must be sent to a task the porting layer
**                         - must bind the signal to the current task. Most
**                         - full service kernels don't require this
**                         - capability but many smaller home grown kernel
**                         - do.
** KS_SIGNAL_SET(SIGNAL)   - Send to a signal causing KS_SIGNAL_GET to wake up
*/

#if (EMBOS)
#define NEWSEMMETHOD 1

#endif

#if (defined(NEWSEMMETHOD))
#define KS_RTIPSIG              int
#define KS_SIGNAL_BUILD(X)      (X) = ks_kernel_sigalloc(); if ((X)==-1) return(FALSE);
#define KS_SIGNAL_CLEAR(SIGNAL) ks_kernel_sigclear(SIGNAL)
#define KS_SIGNAL_BIND(X)
#define KS_SIGNAL_SET(SIGNAL)   ks_kernel_sigset(SIGNAL,FALSE)
#define KS_SIGNAL_GET(SIGNAL,T) ks_kernel_sigtest(SIGNAL, (word) T)
#define KS_SIGNAL_DELETE(SIGNAL) ks_kernel_sigfree(SIGNAL)
#endif  /* NEWSEMMETHOD */

/* ********************************************************************   */
/* SEMAPHORES                                                             */
/* ********************************************************************   */
/*
** Definitions:
** KS_RTIPSEM- An object used to claim exclusive access to a resource.
**             The semaphore must have an initial value of "AVAILABLE"
**             so the first claim succeeds. Subsequent claims succeed
**             after it has been released. If a semaphore is not available
**             claims will suspend until the semaphore is released. The
**             order in which the semaphore is given to claimants is
**             first in first out.
**             Most kernels support KS_RTIPSEM with a binary semaphore
**             initialized to one, a resource semaphore or a mutex
**             semaphore.
** Implementation:
** KS_RTIPSEM              - Use define to declare the type for a semaphore
**                         - This type is used to declare semaphore handles
**                         - in os_port.c. The handles are initialized in
**                         - the os specific function os_resource_init()
**                         - in os_port.c. Note: these "handles" are only
**                         - accessed through macros in this file giving
**                         - maximum flexibility.
**                         - see the example implementations in os_port.h
**                         -   KS_RTIPSEM ifsem[];
**                         -   KS_RTIPSEM criticalsem;
**                         -   KS_RTIPSEM tablesem;
**                         -   KS_RTIPSEM memfilesem;
**                         -   KS_RTIPSEM tcpsem;
**                         -   KS_RTIPSEM udpsem;
**                         -   KS_RTIPSEM syslogsem;
**                         -   These handles are passed as arguments to the
**                         -   macros:
**                         -  KS_SEMAPHORE_GET(SEM)
**                         -  KS_SEMAPHORE_GIVE(SEM)
**
** Semaphore macros:
** KS_SEMAPHORE_GET(SEM)   - Claim a semaphore. This macro claims
**                         - exclusive access to a semaphore, blocking
**                         - until the semaphore is available. It is never
**                         - called from interrupts service, and semaphore
**                         - claims are never nested.. It is invoked
**                         - with one of the above semaphore handles
**                           described above (tablesem,tcpesm etc). The
**
** KS_SEMAPHORE_GIVE(SEM)  - Release a semaphore. This macro releases
**                         - a semaphore that was claimed by KS_SEMAPHORE_GET()
*/
#if (defined(NEWSEMMETHOD))

#define KS_RTIPSEM              int
#define KS_SEMAPHORE_BUILD(X)   (X) = ks_kernel_semalloc(); if ((X)==-1) return(FALSE);
#define KS_SEMAPHORE_FREE(X)    ks_kernel_semfree(X)
#define KS_SEMAPHORE_GET(SEM)   ks_kernel_semtest(SEM)
#define KS_SEMAPHORE_GIVE(SEM)  ks_kernel_semgive(SEM)

#endif




/* ********************************************************************   */
/* TASK CREATION                                                          */
/*                                                                        */
/* STACK SIZES and PRIORITIES                                             */
/*                                                                        */
/* Note: These constants are only needed by the porting layer             */
/* osport.c and in the data declaration section rtipdata.c                */
/* we should have an osport data file so we can keep osport.h             */
/* out of the rest of the code.                                           */
/* ********************************************************************   */
/*                                                                        */
/*                                                                        */
/* Define the following items for the RTOS                                */
/*                                                                        */

/* KS_TASK_HANDLE   -   Kernel specific type of a task handle                */
/* KS_DECLARE_STACK -   If 1 then we must provide stack space if zero        */
/*                      the kernel allocates internally                      */
/* KS_PSTACK        -   If we provide stack then this is a pointer to the    */
/*                      the stack (eq: char far *.. or unsigned int *)       */
/* KS_STACK_TYPE    -   If we provide stack then this is the type which      */
/*                      we create an array of to make stacks                 */
/*                       (eq: char far .. or unsigned int)                   */
/* SIZESTACK_NORMAL -   The size stack required for most tasks. 2048 on most */
/*                      16 bit cpus. 4096 on most 32 bitters                 */
/* SIZESTACK_BIG    -   The size stack used for stack intensive application  */
/*                      layers like web server and nfs server. 4096 on most  */
/*                      16 bit cpus. 8192 on most 32 bitters                 */
/* SIZESTACK_HUGE   -   The size stack used for stack intensive application  */
/*                      layers like SSL server and SSL client.               */
/*      Note: We use a simple 3 priority scheme. Some kernels do not support */
/*      running multiple tasks at the same priority. For those kerenels      */
/*      a little bit of hand tuning will need to be done in                  */
/*      ks_kernel_spawn_task. (osport.c)                                     */
/* PRIOTASK_NORMAL  -   Priority of normal tasks                             */
/* PRIOTASK_HI      -   Priority value for the IP dispatch task              */
/* PRIOTASK_HIGHEST -   Priority value for the interrupt dispatch task       */

#define USE_RTIP_TASKING_PACKAGE 1 /* See ostask.c */

#if (!USE_RTIP_TASKING_PACKAGE)
#undef CFG_NUM_TASK_CONTROL_BLOCKS
#define CFG_NUM_TASK_CONTROL_BLOCKS 1
#endif

#if (EMBOS)
#define KS_TASK_HANDLE      OS_TASK *
#define KS_PSTACK           dword KS_FAR *   /*OS*/ /* was char KS_FAR *   */
#define KS_STACK_TYPE       dword KS_FAR     /*OS*/ /* was char KS_FAR     */
#define KS_DECLARE_STACK    1
#define _SIZESTACK_NORMAL   16384 /*OS*/ /* was 2048 */ /* 4096 bytes for smaller stacks */
#define _SIZESTACK_BIG      32768 /*OS*/ /* was 4096 */ /* 8192 bytes for larger stacks  */
#define _SIZESTACK_HUGE     32768U

#define _PRIOTASK_NORMAL        50   /* Priority value for normal tasks */
#define _PRIOTASK_HI            60   /* Priority value for higher pri tasks */
#define _PRIOTASK_HIGHEST   70   /* Priority value for highest pri tasks */

#else
#error - You must specify type and constants for your kernel in osport.h
#endif

/* ebs_task_context - We use a table of these task context              */
/* structures. These allow us to maintain task specific environment     */
/* variable and to pass in arguments to tasks through a user structure  */
/* one of the members of this structure is the user structure which     */
/* we can return to the caller through an API. The structure contains   */
/* current errno information, four task specific void pointers and      */
/* current working directory information if RTFS is included            */
/*                                                                      */
/* Note: This declaration is only needed by the porting layer           */
/* osport.c and in the data declaration section rtipdata.c              */
/* we should have an osport data file so we can keep osport.h           */
/* out of the rest of the code.                                         */
/* ******************************************************************** */


typedef struct ebs_task_context
{
    SYSTEM_USER     user_context;       /* ebs will call to get this data */

    /* The rest is private to the task porting layer   */
    int             taskindex;          /* Index number */
    KS_TASK_HANDLE  task_handle;        /* Task handle. We use this to sync
                                           the current task with the
                                           task control structure */

/* Flag values used by the kernel background timer task. These are used
   to provide an orderly approach to ending tasks and freeing task stacks */
#define EBS_TASK_IS_FREE             0
#define EBS_TASK_IS_RUNNING          1
#define EBS_TASK_MUST_BE_KILLED      2
#define EBS_TASK_STACK_MUST_BE_FREED 3
    int      flags;

#if (KS_DECLARE_STACK)
    KS_PSTACK pstack_base;              /* Stack base for the task if needed */
#endif

    PFCHAR   name;
    word     stack_size;                /* Stack size of the stack */
    int      priority;                  /* Priority of the task   */
    PEBS_TASKENTRY  ebs_entry_point;    /* Entry point of EBS function */

    /* If the operating system needs an external provided task control
       block we put it here */


#if (EMBOS)
#if (defined(SEGMC16) || defined(ARM_IAR) || defined(__ghs_board_is_nec_vr41xx)) /*OS*/ /* added || defined(ARM_IAR) */
    OS_TASK     kernel_tcb;
#elif (!defined(SEGBCP0))
/* For the borland build we will allocate TCB's from near memory   */
/* See osport.c                                                    */
#error: Implement
#endif
#endif


} EBS_TASK_CONTEXT;
typedef EBS_TASK_CONTEXT KS_FAR *PEBS_TASK_CONTEXT;


typedef struct ebs_task_class
{
    word     stack_size;                /* Stack size of the stack */
    int      priority;                  /* Priority of the task   */
} EBS_TASK_CLASS;

#if (KS_DECLARE_STACK)
/* Used to managed allocating and freeing of stacks   */
struct  stack_manager
{
    int      stack_size;                /* Stack size of the stack */
    KS_PSTACK pstack_base;              /* Stack base for the task if needed */
};
#endif

/* Task priorities - These are the priorities assigned to the tasks   */
/* by class. These defines are local to the porting layer (osport.c)  */
/* (TASKCLASS_XXX are in ebs.h)                                       */
#define PRIOTASK_TIMER              PRIOTASK_HI
#define PRIOTASK_IP                 PRIOTASK_HIGHEST
#define PRIOTASK_INTERRUPT          PRIOTASK_HIGHEST
#define PRIOTASK_FTP_DAEMON         PRIOTASK_NORMAL
#define PRIOTASK_WEB_DAEMON         PRIOTASK_NORMAL
#define PRIOTASK_TFTP_DAEMON        PRIOTASK_NORMAL
#define PRIOTASK_TELNET_DAEMON      PRIOTASK_NORMAL
#define PRIOTASK_FTP_SERVER         PRIOTASK_NORMAL
#define PRIOTASK_WEB_SERVER         PRIOTASK_NORMAL
#define PRIOTASK_HTTPS_SERVER       PRIOTASK_NORMAL
#define PRIOTASK_TELNET_SERVER      PRIOTASK_NORMAL
#define PRIOTASK_NFS_SERVER         PRIOTASK_NORMAL
#define PRIOTASK_SNMP_AGENT         PRIOTASK_NORMAL
#define PRIOTASK_DHCP_SERVER        PRIOTASK_NORMAL
#define PRIOTASK_MAIN_APPTASK       PRIOTASK_NORMAL
#define PRIOTASK_DEMO_APPTASK       PRIOTASK_NORMAL
#define PRIOTASK_USER_APPTASK       PRIOTASK_NORMAL
#define PRIOTASK_RIP_DAEMON         PRIOTASK_NORMAL

/* Task stack sizes - These are the stack sizes assigned to the tasks   */
/* by class. These defines are local to the porting layer (osport.c)    */
/*                                                                      */


#define SIZESTACK_TIMER              SIZESTACK_NORMAL
#define SIZESTACK_IP                 SIZESTACK_NORMAL
#define SIZESTACK_INTERRUPT          SIZESTACK_NORMAL
#define SIZESTACK_FTP_DAEMON         SIZESTACK_NORMAL
#define SIZESTACK_WEB_DAEMON         SIZESTACK_NORMAL
#define SIZESTACK_TFTP_DAEMON        SIZESTACK_NORMAL
#define SIZESTACK_TELNET_DAEMON      SIZESTACK_NORMAL
#define SIZESTACK_FTP_SERVER         SIZESTACK_NORMAL
#define SIZESTACK_WEB_SERVER         SIZESTACK_BIG
#define SIZESTACK_HTTPS_SERVER       SIZESTACK_HUGE
#define SIZESTACK_TELNET_SERVER      SIZESTACK_NORMAL
#define SIZESTACK_NFS_SERVER         SIZESTACK_BIG
#define SIZESTACK_SNMP_AGENT         SIZESTACK_BIG
#define SIZESTACK_DHCP_SERVER        SIZESTACK_NORMAL
#if (INCLUDE_SSL)
#define SIZESTACK_MAIN_APPTASK       SIZESTACK_HUGE
#else
#define SIZESTACK_MAIN_APPTASK       SIZESTACK_BIG
#endif
#define SIZESTACK_DEMO_APPTASK       SIZESTACK_NORMAL
#define SIZESTACK_USER_APPTASK       SIZESTACK_NORMAL
#define SIZESTACK_RIP_DAEMON         SIZESTACK_NORMAL

/* ENTER/EXIT API and TASKS                                             */
/* ******************************************************************** */
#define KS_API_ENTER()

/* ********************************************************************   */
/*  KS_INTERRUPT_CONTEXT - The type returned from ks_splx() and           */
/*  passed into ks_spl(). This is either a word or a dword depending      */
/*  on the length of the processors flags word                            */
/* ********************************************************************   */

#if (defined(EMBOS))
#define KS_INTERRUPT_CONTEXT int /* Not used for EMBOS see ks_splx */

#elif (defined(__BORLANDC__) )
#define KS_INTERRUPT_CONTEXT word

#elif (defined(EBSMC603INT))
#define KS_INTERRUPT_CONTEXT int

/*#elif (RTKERNELRISC && EBSARM7TDMIINT)   */
/*#define KS_INTERRUPT_CONTEXT int         */

#elif (ARM7TDMI)
#define KS_INTERRUPT_CONTEXT int

#else
    #error You must define KS_INTERRUPT_CONTEXT in osport.h
#endif

#ifdef _MWERKS_
#ifdef __cplusplus
extern "C" {
#endif
#endif
KS_INTERRUPT_CONTEXT ks_splx(void);     /* Disable interrupts & return old flags */
void ks_spl(KS_INTERRUPT_CONTEXT oldlevel); /* Restore interrupts to previous state */
#ifdef _MWERKS_
#ifdef __cplusplus
}
#endif
#endif

#if (INCLUDE_ERTFS_INTERRUPTS)
/* ********************************************************************   */
/* ERTFS: INTERRUPT SIGNALS                                               */
/* ********************************************************************   */
/* Build RTFS Interrupt signals if the file system is included            */
#if (USE_ATA)
void ks_invoke_ide_interrupt(int controller_no);
#endif
#if (USE_FLOPPY)
void ks_invoke_floppy_interrupt(int controller_no);
#endif
#endif      /* INCLUDE_ERTFS */

#if (defined(NEWSEMMETHOD) )
RTIP_BOOLEAN ks_kernel_semalloc_init(void);
int ks_kernel_semalloc(void);
void ks_kernel_semfree(int semnum);
RTIP_BOOLEAN ks_kernel_semtest(int semnum);
void ks_kernel_semgive(int semnum);
RTIP_BOOLEAN ks_kernel_sigalloc_init(void);
int ks_kernel_sigalloc(void);
void ks_kernel_sigfree(int signum);
RTIP_BOOLEAN ks_kernel_sigtest(int signum, word timeout);
RTIP_BOOLEAN ks_kernel_sigclear(int signum);
void ks_kernel_sigset(int signum, RTIP_BOOLEAN from_isr);
#endif


/* ********************************************************************   */
/* MISC FUNCTION DECLARATIONS                                             */
/* ********************************************************************   */
#if (PPC603) /* functions included from init603.c */
void ee_enable();
void ee_disable();
dword longswap(dword l);          /* change 4 bytes in long word 0,1,2,3 -> 3,2,1,0 */
word wordswap(word w);          /* change 2 bytes in word 0,1 -> 1,0 */
void cog603_mask_isr_on(int irq);
void cog603_mask_isr_off(int irq);
#endif

#endif      /* end of __OS_PORT__ */

