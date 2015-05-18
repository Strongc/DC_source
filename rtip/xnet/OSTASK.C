/*                                                                           */
/* OSTASK.C - OS Task management PORTING functions                           */
/*                                                                           */
/* EBS - RTIP                                                                */
/*                                                                           */
/* Copyright Peter Van Oudenaren , 1993                                      */
/* All rights reserved.                                                      */
/* This code may not be redistributed in source or linkable object form      */
/* without the consent of its author.                                        */
/*                                                                           */
/*  Module description:                                                      */
/*      This module consists of OS specific routines that                    */
/*      spawn tasks and allocate and associate RTIP Task Context blocks      */
/*      with tasks (threads).                                                */
/*  You must also port OSPORT.C. This contains os specific code for spawning */
/*  tasks and maintaining thread local user context blocks.                  */


/*
** This file contains a task spawning function and a thread specific context
** management package for RTIP.
** This file needs only to export four files and no data structures.
**
** Several complexities arise for many real time OS's. First they require
** stacks to be provided by the user, second, it is hard to release a thread's
** stack from within it's own execution context and third, it is dificult to
** cleanly and transparently hook a private data structure to an RTOS TCB to
** provide thread specific management.
** To solve these problems we created a task management package and a
** stack management package. These two packages are enable via two
** #defines respectively:
**  USE_RTIP_TASKING_PACKAGE and KS_DECLARE_STACK
** in osport.h
**
** The following functions must be implemented.
** all other functions are static to this module
**
** os_spawn_task - This function takes a task class, task entry point
**                 and up to four arguments. The job of the function
**                 is to spawn a task that runs from the entry point
**                 and having the priority and stack size associated with
**                 the class. This routine should also associate an
**                 EBS task context block with the task. The task context
**                 block contains the four user arguments to this function as
**                 well as the current VFS's working directory for the task
**                 and the current errno value.
**                 The implementation should ensure that the MFS current
**                 working directory and the RTIP task context block are
**                 released when the task exits. RTIP will call os_get_user()
**                 to access the pointer.
**
**
** os_get_user -   When this function is called it must return the RTIP user
**                 context block associated with the current task. If none
**                 is assigned it should allocate one and schedue it for
**                 release when the task is freed. This function is used
**                 by set/get errno() and get/set_cwd()
**
** os_get_user_by_index - This routine is ONLY NEEDED if the ERTFS package
**                 is included. It iterates through all threads that
**                 contain RTIP context structure.
**
** os_exit_task -  This function must be called at exit by application
**                 threads that were not created by os_spawn_task() but
**                 called the RTIP API none the less. If this functions is
**                 not called when the tasks exit then a small memory leak
**                 in RTIP and potentially in ERTFS will consume resources.
**                 Note: RTKernel automatically does this so it is not
**                 necessary to call.
**
** If the tasking package is used the following routines must be implemented.
** see the sample source code and the comments in the code for what you need
** to do to make these routines work with your RTOS.
**
** ks_get_task_index
** ks_task_entry
** ks_kernel_spawn_task
** ks_kernel_timer_callback
** ks_kernel_exit_task
** ks_bind_task_index
*/

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

/* ********************************************************************   */
#include "rtip.h"
#include "sock.h"
#include "rtipext.h"
#if (INCLUDE_MFS)
#include "memfile.h"
#endif

/* ********************************************************************   */
#if (EMBOS)
    OS_TASK /* near */ /*OS*/ segger_kernel_tcb[CFG_NUM_TASK_CONTROL_BLOCKS];
#endif

#if (INCLUDE_ERTFS_EXIT_TASK)
void pc_free_user(void);
#endif

static void kernel_init(void);
static void _ks_exit_task(int taskindex);
static PEBS_TASK_CONTEXT ks_kernel_tcb_alloc(void);
static void ks_kernel_tcb_free(PEBS_TASK_CONTEXT ptask_context);
static int ks_kernel_spawn_task(PEBS_TASK_CONTEXT ptask_context);



#if (KS_DECLARE_STACK)
/* ********************************************************************   */
static void ks_kernel_stackalloc_init(void);
static KS_PSTACK ks_kernel_stack_alloc(int stack_size);
static void ks_kernel_stack_free(KS_PSTACK pstack);
#endif

#if (USE_RTIP_TASKING_PACKAGE)
/* ********************************************************************   */
int ks_get_task_index(void);
static void ks_kernel_exit_task(int taskindex);
static void ks_bind_task_index(int taskindex);
#endif


/* ********************************************************************   */
/* ===== BEGIN PUBLIC FUNCTIONS =========                                 */
/* ********************************************************************   */

/* ********************************************************************   */
/* SPAWN TASK                                                             */
/* ********************************************************************   */
/* os_spawn_task() -
*
* This routine is used to spawn task for use by the ebs libraries and
* demonstration programs. It may also be invoked by the user to spawn
* application tasks.
* If the kernel being used requires the user to provide stack space
* then the stack space is allocated from this routine.
*
* The routine takes four arguments. The task_class, The 'C' entry point
* to execute from and four void pointers that may be used to preassign
* data to the task. (like passing arguments)
*
* the 'C' function is a function of the type void func(void). It will execute
* when the spawn is complete.
*
* The void pointers may be NULL or they may point to anything. They are not
* dereferenced. They are only assigned to the task's user structure for
* later retrieval.
*
* The task_class value may be any one of the following
* TASKCLASS_TIMER
* TASKCLASS_IP
* TASKCLASS_INTERRUPT
* TASKCLASS_FTP_DAEMON
* TASKCLASS_WEB_DAEMON
* TASKCLASS_TFTP_DAEMON
* TASKCLASS_TELNET_DAEMON
* TASKCLASS_FTP_SERVER
* TASKCLASS_WEB_SERVER
* TASKCLASS_HTTPS_SERVER
* TASKCLASS_TELNET_SERVER
* TASKCLASS_NFS_SERVER
* TASKCLASS_SNMP_AGENT
* TASKCLASS_MAIN_APPTASK
* TASKCLASS_DEMO_APPTASK
* TASKCLASS_USER_APPTASK
* TASKCLASS_RIP_DAEMON
*
* Note that TASKCLASS_USER_APPTASK is reserved for use by user applications
* code.
*
* Each class has a stack size, and a priority that are set up from defines
* in osport.h
*
*/

RTIP_BOOLEAN os_spawn_task(int task_class, PEBS_TASKENTRY entry_point,
                      PFVOID v0, PFVOID v1, PFVOID v2, PFVOID v3)
{
PEBS_TASK_CONTEXT ptask_context = 0;
#if (KS_DECLARE_STACK)
KS_PSTACK stack_base;
#endif

    /* initialize task templates and stack pool manager if it has not   */
    /* already been done                                                */
    kernel_init();

#if (USE_RTIP_TASKING_PACKAGE)
    /* free task context and stacks   */
    ks_kernel_timer_callback();
#endif

#if (KS_DECLARE_STACK)
    /* Allocate a stack for the task if necessary   */
    stack_base = ks_kernel_stack_alloc(class_templates[task_class].stack_size);
    if (!stack_base)
    {
        DEBUG_ERROR("os_spawn_task: Out of stacks of size ", EBS_INT1,
            class_templates[task_class].stack_size, 0);
        return(FALSE);
    }
#endif

    /* Allocate an RTIP task control block   */
    ptask_context = ks_kernel_tcb_alloc();
    if (!ptask_context)
    {
        DEBUG_ERROR("os_spawn_task: Out of task control blocks ", NOVAR, 0, 0);
#if (KS_DECLARE_STACK)
        ks_kernel_stack_free(stack_base);
#endif
        return(FALSE);
    }
#if (KS_DECLARE_STACK)
    ptask_context->pstack_base = stack_base;
#endif


   /* Copy the stack size and priority from the class template     */
    ptask_context->stack_size = class_templates[task_class].stack_size;
    ptask_context->priority   = class_templates[task_class].priority;
    ptask_context->name = (PFCHAR)class_names[task_class];
    ptask_context->user_context.udata0 = v0;
    ptask_context->user_context.udata1 = v1;
    ptask_context->user_context.udata2 = v2;
    ptask_context->user_context.udata3 = v3;
    ptask_context->ebs_entry_point = entry_point;

    if (ks_kernel_spawn_task(ptask_context) == 0)
    {
        return(TRUE);
    }
    else
    {
        DEBUG_ERROR("ks_kernel_spawn_task failed", NOVAR, 0, 0);
#if (KS_DECLARE_STACK)
        ks_kernel_stack_free(ptask_context->pstack_base);
#endif
        ks_kernel_tcb_free(ptask_context);
        return(FALSE);
    }
}

/* ********************************************************************   */
/* EXIT TASK                                                              */
/* ********************************************************************   */
/* os_exit_task -  This function must be called at exit by application
**                 threads that were not created by os_spawn_task() but
**                 called the RTIP API none the less. If this functions is
**                 not called when the tasks exit Then a small memory leak
**                 in RTIP and potential in ERTFS will consume resources.
**                 Note: RTKernel automatically does this so it is not
**                 necessary to call.
*/

void os_exit_task(void)   /*__fn__*/
{
    _ks_exit_task(0);
}


/* ********************************************************************   */
/* os_get_user -   When this function is called it must return the RTIP user
**                 context block associated with the current task. If none
**                 is assigned it should allocate one and schedue it for
**                 release when the task is freed. This function is used
**                 by set/get errno() and get/set_cwd()
*/


PSYSTEM_USER os_get_user(void)
#if (USE_RTIP_TASKING_PACKAGE)
{
int taskindex;

    taskindex = ks_get_task_index();

    /* If They ask for the current user structure and didn't get one
       then grab one */
    if (!taskindex)
    {
        for (taskindex = CFG_FIRST_EXTERNAL_TASK;
             taskindex < CFG_NUM_TASK_CONTROL_BLOCKS; taskindex++)
        {
            ks_disable();
            if (task_contexts[taskindex].flags == EBS_TASK_IS_FREE)
            {
                tc_memset((PFBYTE)&task_contexts[taskindex],0,
                           sizeof(task_contexts[taskindex]));
                task_contexts[taskindex].flags = EBS_TASK_IS_RUNNING;
                ks_enable();
                ks_bind_task_index(taskindex);
                break;
            }
            ks_enable();
        }
        /* Call get_task_index() again. If it returns zero we'll use that
           as a fallback */
        taskindex = ks_get_task_index();
    }
    return((PSYSTEM_USER) (&task_contexts[taskindex].user_context));
}
#else
#error: select a tasking model
#endif


/* ********************************************************************   */
/* ===== END PUBLIC FUNCTIONS =========                                   */
/* ********************************************************************   */


/* ********************************************************************     */
/* ===== BEGIN PRIVATE FUNCTIONS THAT ARE USED BY ALL IMPLEMENTATIONS ===== */
/* ********************************************************************     */

/* ********************************************************************   */
/* _ks_exit_task() - This routine is called by the various task exit      */
/* paths. It must make sure that RTIP resources are freed. These include  */
/* VFS and EBS current worknig directories. RTIP user context blocks      */
/* and stacks if RTIP is responsible                                      */
static void _ks_exit_task(int taskindex)   /*__fn__*/
{
PEBS_TASK_CONTEXT ptask_context;

#if (!USE_RTIP_TASKING_PACKAGE)
    ARGSUSED_INT(taskindex)
#endif

#if (USE_RTIP_TASKING_PACKAGE)
    /* If os_exit_task() was called from the API (taskindex == 0)
       then look up the task index. If it is in the external
       task group (ie was spawned external to ebs environment
       then set the task context to free so it is available
    */
    if (!taskindex) taskindex = ks_get_task_index();
    ptask_context = &task_contexts[taskindex];
#endif

    if (ptask_context)
    {
#if (INCLUDE_ERTFS_EXIT_TASK)
         pc_free_user();
#endif

#if (INCLUDE_MFS && INCLUDE_SUBDIRS)
/* Note: this calls os_free_packet() via dcu_alloc/dcu_free() -
   Check the safety of this under rtkernel since we are being called from task exit */
        if (ptask_context->user_context.mfcwd)
            mf_free_path(ptask_context->user_context.mfcwd);
        ptask_context->user_context.mfcwd = 0;
#endif

#if (USE_RTIP_TASKING_PACKAGE)
        if (taskindex >= CFG_FIRST_EXTERNAL_TASK)
        {
            ptask_context->flags = EBS_TASK_IS_FREE;
            ptask_context->task_handle =0; /*tvotvo */
        }
#endif
    }
}


/* ********************************************************************   */
static PEBS_TASK_CONTEXT ks_kernel_tcb_alloc(void)
{
PEBS_TASK_CONTEXT ptask_context;

#if (USE_RTIP_TASKING_PACKAGE)
{
int taskindex;
    ptask_context = 0;
    ks_disable();
    for (taskindex = 1; taskindex < CFG_FIRST_EXTERNAL_TASK; taskindex++)
    {
        if (task_contexts[taskindex].flags == EBS_TASK_IS_FREE)
        {
            tc_memset((PFBYTE)&task_contexts[taskindex], 0, sizeof(task_contexts[taskindex]));
            task_contexts[taskindex].flags = EBS_TASK_IS_RUNNING;
            task_contexts[taskindex].taskindex = taskindex;     /* __st__ */
            ptask_context = &task_contexts[taskindex];
            break;
        }
    }
    ks_enable();
}
#endif

    if (ptask_context)
        return(ptask_context);
    else
    {
        DEBUG_ERROR("RTIP: Out of task control blocks ", NOVAR, 0, 0);
        return(0);
    }
}

/* ********************************************************************   */
static void ks_kernel_tcb_free(PEBS_TASK_CONTEXT ptask_context)
{
#if (USE_RTIP_TASKING_PACKAGE)
    ptask_context->flags = EBS_TASK_IS_FREE;
    ptask_context->task_handle =0; /*tvotvo */

#endif
}




/* ********************************************************************   */
#define INIT_CLASS(classno, pri_ority, stacksize) \
        class_templates[classno].stack_size = stacksize; \
        class_templates[classno].priority = pri_ority

/* ********************************************************************   */
/* initialize task templates and stack pool manager if it has not         */
/* already been done                                                      */
static void kernel_init(void)
{
    ks_disable();   /* Disable and  push interrupts */
    if (kernel_initialized)
    {
        ks_enable();
        return;
    }
    /* Initialize the task templates. These give rules for
       spawning tasks by the class of task */
    /* first initialize the default template    */
    INIT_CLASS(TASKCLASS_CURRENT,0,0);

    /* Now initialize the task contexts by class   */
    INIT_CLASS(TASKCLASS_TIMER,PRIOTASK_TIMER,SIZESTACK_TIMER);
    INIT_CLASS(TASKCLASS_IP,PRIOTASK_IP,SIZESTACK_IP);
    INIT_CLASS(TASKCLASS_INTERRUPT,PRIOTASK_INTERRUPT,SIZESTACK_INTERRUPT);
    INIT_CLASS(TASKCLASS_FTP_DAEMON,PRIOTASK_FTP_DAEMON,SIZESTACK_FTP_DAEMON);
    INIT_CLASS(TASKCLASS_WEB_DAEMON,PRIOTASK_WEB_DAEMON,SIZESTACK_WEB_DAEMON);
    INIT_CLASS(TASKCLASS_TFTP_DAEMON,PRIOTASK_TFTP_DAEMON,SIZESTACK_TFTP_DAEMON);
    INIT_CLASS(TASKCLASS_TELNET_DAEMON,PRIOTASK_TELNET_DAEMON,SIZESTACK_TELNET_DAEMON);
    INIT_CLASS(TASKCLASS_FTP_SERVER,PRIOTASK_FTP_SERVER,SIZESTACK_FTP_SERVER);
    INIT_CLASS(TASKCLASS_WEB_SERVER,PRIOTASK_WEB_SERVER,SIZESTACK_WEB_SERVER);
    INIT_CLASS(TASKCLASS_HTTPS_SERVER,PRIOTASK_HTTPS_SERVER,SIZESTACK_HTTPS_SERVER);
    INIT_CLASS(TASKCLASS_TELNET_SERVER,PRIOTASK_TELNET_SERVER,SIZESTACK_TELNET_SERVER);
    INIT_CLASS(TASKCLASS_NFS_SERVER,PRIOTASK_NFS_SERVER,SIZESTACK_NFS_SERVER);
    INIT_CLASS(TASKCLASS_SNMP_AGENT,PRIOTASK_SNMP_AGENT,SIZESTACK_SNMP_AGENT);
    INIT_CLASS(TASKCLASS_DHCP_SERVER,PRIOTASK_DHCP_SERVER,SIZESTACK_DHCP_SERVER);
    INIT_CLASS(TASKCLASS_MAIN_APPTASK,PRIOTASK_MAIN_APPTASK,SIZESTACK_MAIN_APPTASK);
    INIT_CLASS(TASKCLASS_DEMO_APPTASK,PRIOTASK_DEMO_APPTASK,SIZESTACK_DEMO_APPTASK);
    INIT_CLASS(TASKCLASS_USER_APPTASK,PRIOTASK_USER_APPTASK,SIZESTACK_USER_APPTASK);
    INIT_CLASS(TASKCLASS_EXTERN_APPTASK,0,0);
    INIT_CLASS(TASKCLASS_RIP_DAEMON,PRIOTASK_RIP_DAEMON,SIZESTACK_RIP_DAEMON);

    kernel_initialized = TRUE;

    /* Ebs runtime initialization code   */
    tc_memset((PFBYTE)task_contexts, 0, sizeof(task_contexts));

#if (!USE_RTIP_TASKING_PACKAGE)
#if (CFG_NUM_TASK_CONTROL_BLOCKS != 1)
/* tbd                                           */
/*#error - CFG_NUM_TASK_CONTROL_BLOCKS must be 1 */
#endif
#endif


#if (USE_RTIP_TASKING_PACKAGE)
    ks_data_run_timer = 0;
#endif

    ks_enable();

#if (KS_DECLARE_STACK)
    ks_kernel_stackalloc_init();
#endif
}



/* ********************************************************************   */
/* ===== END PRIVATE FUNCTIONS THAT ARE USED BY ALL IMPLEMENTATIONS ===== */
/* ********************************************************************   */

/* ********************************************************************   */
/* ===== END RTKERNEL TASK SPAWNING ROUTINES                              */
/* ********************************************************************   */


#if (USE_RTIP_TASKING_PACKAGE)

/* ********************************************************************   */
/* ********************************************************************   */
/* ********************************************************************   */
/* ********************************************************************   */
/* ===== BEGIN RTIP_TASKING_PACKAGE TASK SPAWNING ROUTINES                */
/* ********************************************************************   */
/* ********************************************************************   */
/* ********************************************************************   */
/* ********************************************************************   */

/* ********************************************************************         */
/* TASK CREATE/EXIT/BIND                                                        */
/* ********************************************************************         */
/* Task creation management package:                                            */
/*                                                                              */
/* This package is required to provide dynamic task creation, task destruction  */
/* and current task identification.                                             */
/*                                                                              */
/* The following routines must be implemented:                                  */
/*                                                                              */
/* int ks_get_task_index(void) -                                                */
/*                                                                              */
/* This routine is required to ascertain the current tasks taskindex. The       */
/* taskindex is a small integer that was passed to the routine                  */
/* ks_kernel_spawn_task if the task was spawned by the ebs run time or to       */
/* the routine ks_bind_task_index() if the thread was spawned external to       */
/* the ebs run time environment. (these routines are described later)           */
/*                                                                              */
/* int ks_bind_task_index(int taskindex) -                                      */
/*                                                                              */
/* This routine is required to associate the taskindex. With the current thread */
/* so that that same index value will be returned later by calls to             */
/* ks_get_task_index.                                                           */
/*                                                                              */
/*                                                                              */
/* void ks_kernel_timer_callback() -                                            */
/*  This routine is called by the timer task.                                   */
/*  It may be used to free the stacks and delete the tasks that are             */
/*  associated with those tasks. The function ks_kernel_exit_task               */
/*  tack and free its stack space.                                              */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/* void ks_kernel_exit_task(int taskindex)                                      */
/*                                                                              */
/* This routine must exit the current task and release its stack memory         */
/* if the operating system requires externel stack allocation. For kernels      */
/* which maintain stack values externally or those that keep the task control   */
/* block in memory seperate from the stack this function can usually be         */
/* done by making a simple call. For kernels that use the stack to hold         */
/* the task control block this function can be a little tricky. In these        */
/* systems a way to accomplish the goal is to shedule the task for              */
/* and then either block or lower the tasks priority and enter a for loop       */
/* that continually yields (IE: Put the task in a state so that it will be      */
/* save to destroy it from the timer task.)                                     */
/* Finally this routine or the callback routine must put the task_contexts      */
/* block associated with task in the FREE state.                                */
/*                                                                              */
/*                                                                              */
/* TYPE ks_task_entry(ARGS) -                                                   */
/* This is a kernel specific task entry                                         */
/* point which must be supplied. The function type and number of                */
/* arguments vary from kernel to kernel. This routine must ascertain            */
/* its taskindex (which is a small integer) and call a 'C' function             */
/* task_contexts[taskindex].ebs_entry_point(). This function should             */
/* also work in conjuction with ks_kernel_spawn_task() to ensure that           */
/* ks_get_task_index() when called by functions in this thread returns          */
/* the appropriate task index value (see the next section for more on this).    */
/*                                                                              */
/*                                                                              */
/* int ks_kernel_spawn_task(int taskindex)                                      */
/*                                                                              */
/*  This routine must spawn a task that then calls a 'C' function that          */
/*  is the main entry point to the thread of code we wish to execute.           */
/*  A pointer to the 'C' function, the stack size, stack base (if needed)       */
/*  and priority are provided in a task table at offset taskindex.              */
/*                                                                              */
/*  Note: This routine must have the following side effects:                    */
/*   . It must set up a scenario which makes it possible for the                */
/*     application thread to call the routine ks_get_task_index()               */
/*     which will return the integer taskindex.                                 */
/*   . It must schedule the thread to run. The thread may execute either        */
/*     immediately or at a later time as priorities dictate                     */
/*                                                                              */
/*  There are several possible strategies to synchronize                        */
/*  ks_kernel_spawn_task() and ks_get_task_index().                             */
/*                                                                              */
/*    . If the ks_kernel_spawn_task() can store the task handle in the          */
/*      task_context BEFORE ks_task_entry() executes (IE you can seperate       */
/*      the task creation from the task start). Then if the spawned task        */
/*      can ascertain its own task handle it can then look its taskindex        */
/*      by iterating through the task_contexts array until the current          */
/*      task id matches the one stored in the array.                            */
/*    . If the ks_kernel_spawn_task() can not store the task handle in the      */
/*      task_context BEFORE ks_task_entry() executes but it can pass            */
/*      an integer argument to the spawned task. Then the spawned               */
/*      task should get it's task ID from the operating system and put          */
/*      it in the task_context array. Future calls to ks_get_task_index()       */
/*      will look up the task id using the task id from the OS.                 */
/*                                                                              */
/*    Note: These methods rely on ks_get_task_index() iterating through         */
/*    the task_context array. It is possible to optomize ks_get_task_index()    */
/*    if your kernel supports assigning user data with a thread. If this        */
/*    is possible then ks_task_entry() should assign taskindex to the           */
/*    thread and ks_get_task_index() should return this value.                  */
/*                                                                              */
/*                                                                              */
/*  Taskindex is an index into an array (task_contexts[]) of type               */
/*  EBS_TASK_CONTEXT. The following fields are relevant to the                  */
/*  ks_kernel_spawn_task() function.                                            */
/*                                                                              */
/* These fields are set up prior to calling this function                       */
/*  task_contexts[taskindex].stack_size         - The stack size in byte        */
/*  task_contexts[taskindex].pstack_base        - The stack core (if needed)    */
/*  task_contexts[taskindex].name               - The name of the task          */
/*  task_contexts[taskindex].priority           - The task priority             */
/*  task_contexts[taskindex].ebs_entry_point    - 'C' function to call          */
/*                                                                              */
/* These fields are set up by the task                                          */
/*  task_contexts[taskindex].task_handle- This handle will be used by the       */
/*                                        spawned task to identify itself       */
/*                                        The handle may either be set up       */
/*                                        by ks_kernel_spawn_task() or          */
/*                                        by the kernel specific task entry     */
/*                                        point function before it calls        */
/*                                        the ebs function.                     */



#if (EMBOS)
void ks_task_entry(void);
#else
#error Implement ks_task_entry for your kernel in OSTASK.C
#endif



/* ********************************************************************   */
static int ks_kernel_spawn_task(PEBS_TASK_CONTEXT ptask_context)
{
#if (EMBOS)
#if (defined(SEGBCP0))
    ptask_context->task_handle = &segger_kernel_tcb[ptask_context->taskindex];

#elif (defined(SEGMC16) || defined(ARM_IAR) || defined(__ghs_board_is_nec_vr41xx)) /*OS*/ /* added ARM_IAR, added MIPS_GHS */
    ptask_context->task_handle = &ptask_context->kernel_tcb;

#else
#error: Please fix this
#endif

    OS_CreateTask(ptask_context->task_handle,
         ptask_context->name,
         ptask_context->priority,
         ks_task_entry,
         ptask_context->pstack_base,        /* stack */
         ptask_context->stack_size,     /* stack size */
         5);

#else
#error implement ks_kernel_spawn_task for Your kernel in OSPORT.C
#endif

    return(0);
}

/* ********************************************************************   */
/* TASK ENTRY                                                             */
/* ********************************************************************   */
#if (EMBOS)
void ks_task_entry(void)
{
int taskindex;
    for (taskindex = 1; taskindex < CFG_FIRST_EXTERNAL_TASK; taskindex++)
    {
        if (task_contexts[taskindex].flags == EBS_TASK_IS_RUNNING)
        {
            if (task_contexts[taskindex].task_handle == OS_pCurrentTask)
            {
                task_contexts[taskindex].ebs_entry_point();
                /* After the ebs task returns make sure we delete it   */
                ks_kernel_exit_task(taskindex);
                break;
            }
        }
    }
}

#else
#error Implement ks_task_entry for your kernel in OSPORT.C
    task_contexts[taskindex].task_handle =
#endif



/* ********************************************************************   */
/* int ks_get_task_index(void) - This routine will be called be ebs threads. It
*  must return the task index value that was passed in to
*  ks_kernel_spawn_task(int taskindex) if ebs spawned the task or it must
*  return the value that was passed to void ks_bind_task_index(int taskindex)
*  if the task was spawned outside of the ebs environment and subseqantly
*  bound in as a user task.
*
*  If the task is not registered (does not have a task index) this routine
*  should return zero.
*
*
*/

int ks_get_task_index(void) /* __st__ made routine globally accessible */
{
#if (EMBOS)
int i;

    for (i = 1; i < CFG_NUM_TASK_CONTROL_BLOCKS; i++)
        if ((task_contexts[i].flags == EBS_TASK_IS_RUNNING) &&
            (task_contexts[i].task_handle == OS_pCurrentTask))
            return(i);

#else
#error implement ks_get_task_index for Your kernel
#endif

    return(0);
}

/* ********************************************************************   */
/* void ks_kernel_timer_callback() - This routine is called by the timer
*  task. If the global value ks_data_run_timer is non-zero.
*  It is used to free the stacks and delete the tasks that are associated
*  with those tasks. The function ks_kernel_exit_task(int taskindex)
*  schedules the timer callback to be exectuted.


*
*  For some kernel environments there may be other uses for this
*  call back.
*
*/


void ks_kernel_timer_callback(void)
{
int taskindex;

    /* if not initialized, semaphore not available   */
    if (!rtip_initialized)
        return;

    OS_CLAIM_TABLE(KERNEL_TABLE_CLAIM)
    if (!ks_data_run_timer)
    {
        OS_RELEASE_TABLE()
        return;
    }
    ks_data_run_timer = 0;

    for (taskindex = 1; taskindex < CFG_FIRST_EXTERNAL_TASK; taskindex++)
    {
        if (task_contexts[taskindex].flags == EBS_TASK_STACK_MUST_BE_FREED)
        {
        /* If the stack is sheduled to be freed free it. This is common
           to all kernels that require user supplied stacks */
#if (KS_DECLARE_STACK)
            ks_kernel_stack_free(task_contexts[taskindex].pstack_base);
            task_contexts[taskindex].flags = EBS_TASK_IS_FREE;
            task_contexts[taskindex].task_handle =0; /*tvotvo */

#endif
        }
        else if (task_contexts[taskindex].flags == EBS_TASK_MUST_BE_KILLED)
        {
#if (EMBOS)
            /* Nothing to do for EMBOS. Tasks delete themselves   */

#else
#error implement ks_kernel_timer_callback for Your kernel
#endif
            task_contexts[taskindex].task_handle =0; /*tvotvo */
        } /* else if (task_contexts[taskindex].flags == EBS_TASK_MUST_BE_KILLED) */
    } /* for (taskindex = 1; taskindex < CFG_FIRST_EXTERNAL_TASK; taskindex++) */
    OS_RELEASE_TABLE()
}

/* ********************************************************************   */
/* void ks_kernel_exit_task(int taskindex) - This routine removes a task
*  and deallocates the task's stack if necessary. If it is difficult
*  for a task to delete itself and de-allocate it's stack then the
*  routine should schedule the deletion to take place in the kernel
*  timer callback routine.
*
*/

static void ks_kernel_exit_task(int taskindex)
{
    /* Release ebs resources like file system blocks terminal handles etc   */

    _ks_exit_task(taskindex);

#if (EMBOS)
    task_contexts[taskindex].flags = EBS_TASK_STACK_MUST_BE_FREED; // was EBS_TASK_IS_FREE (FKA)
    ks_data_run_timer = 1;              // added by FKA 2004-04-28 to get ks_kernel_stack_free running
    OS_Terminate(task_contexts[taskindex].task_handle);

#else
#error implement ks_kernel_exit_task for Your kernel
#endif

    if (task_contexts[taskindex].flags != EBS_TASK_IS_FREE)
        ks_data_run_timer = 1;
    else
        task_contexts[taskindex].task_handle =0; /*tvotvo */
}

/* ********************************************************************   */
/* int ks_bind_task_index(int taskindex) - This routine will be called to bind a task
*  that was spawned outside of the ebs environment to an internal ebs
*  task management structure. It should attach a value to
*  task_contex[taskindex] so that subsequant calls to ks_get_task_index()
*  will return taskindex. If the kernel supports storing thread specific
*  data and ks_get_task_index() is using this to look up the index then this
*  routine should set the thread specific data to taskindex. Otherwise
*  this routine can get the tasks task handle from the OS and put it in
*  the task_handle field of the task_context structure.
*/

void ks_bind_task_index(int taskindex)
{
    /* initialize task templates and stack pool manager if it has not   */
    /* already been done                                                */
    kernel_init();

#if (EMBOS)
    task_contexts[taskindex].task_handle = OS_pCurrentTask;

#else

#error Implement ks_bind_task_index() for your kernel in OSPORT.C
    task_contexts[taskindex].task_handle =
#endif
}
/* ===== END RTIP_TASKING_PACKAGE TASK SPAWNING ROUTINES   */

#endif /* (USE_RTIP_TASKING_PACKAGE) */


#if (KS_DECLARE_STACK)

/* ********************************************************************   */
/* ===== BEGIN RTIP TASKING STACK MANAGEMENT ROUTINES                     */
/* ********************************************************************   */

/* ********************************************************************           */
/* Stack management package:                                                      */
/* ********************************************************************           */
/*                                                                                */
/*                                                                                */
/*                                                                                */
/* These three functions implement a stack management package for Operating       */
/* systems that require the user to provide a stack to the OS's task create       */
/* function.                                                                      */
/*                                                                                */
/* NOTE: !!! If the operating system allocates stacks internally                  */
/* the this should be indicated by setting KS_DECLARE_STACK to 0 in               */
/* osport.h. In this case these functions will never be called and                */
/* will not be compiled                                                           */
/*                                                                                */
/* The three functions are:                                                       */
/*                                                                                */
/* ks_kernel_stackalloc_init(void)                                                */
/*  This functions is called by the kernel init function to prepare               */
/*  the stack allocation system to work.                                          */
/* ks_kernel_stack_alloc(int stack_size)                                          */
/*  This function is called to allocate a stack of stack_size bytes.              */
/*  stack_size will be one of two sizes only SIZESTACK_NORMAL or                  */
/*  SIZESTACK_BIG. (these are configuration options in OSPORT.H the               */
/*  appropriate value depend on the kernel and CPU being used.                    */
/*                                                                                */
/* void ks_kernel_stack_free(KS_STACK *pstack)                                    */
/*  This fuction is called after a task exits. It frees the stack for             */
/*  later reuse. When this function is invoked will depend on the kernel          */
/*  and may be problematic since we must make sure that the kernel no             */
/*  longer needs the stack before we free it.                                     */
/*                                                                                */
/* Note: The requirements of this package are that                                */
/* ks_kernel_stack_alloc(void) return a stack base and that                       */
/* ks_kernel_stack_free(void) return is to a heap. We implement an                */
/* algorithm here that preallocates all the stack space needed. It                */
/* is possible to use a malloc/free pair to do achieve this as well.              */
/*                                                                                */
/* These routine use the following algorithm:                                     */
/*                                                                                */
/* ks_kernel_stackalloc_init initializes and array of structures that             */
/* contain a stack size element and a stack pointer element. The stack            */
/* pointer element points to entries in the arrays:                               */
/*                                                                                */
/*  normal_1_stacks_array - First array of stacks of size SIZESTACK_NORMAL        */
/*  normal_2_stacks_array - Second array of stacks of size SIZESTACK_NORMAL       */
/*  big_stacks_array      - Array of stacks of size SIZESTACK_BIG                 */
/*  huge_stacks_array     - Array of stacks of size SIZESTACK_HUGE                */
/*                                                                                */
/*  Note: The NORMAL stacks are broken into two arrays because on some            */
/*  architectures an array can not exceed 64K. It is unlikely that                */
/*  the stack requirments of the whole system will exceed 64K but                 */
/*  just in case it is provided for.                                              */
/*                                                                                */
/*  These kernel specific configuration paramaters from osport.h effect the       */
/*  stack alloaction routines:                                                    */
/*                                                                                */
/*  KS_DECLARE_STACK    - If 1 then these routines are needed                     */
/*  KS_STACK_TYPE   - Used to declare arrays that will become stacks              */
/*                        for example unsigned char                               */
/*  KS_PSTACK           - Type of pointer to the stacks for example               */
/*                        'unsigned char *'                                       */
/*  SIZESTACK_NORMAL    - Size of a normal stack. This can be determined          */
/*                        experimentally. We typically use 2K for 16 bit          */
/*                        applications and 4K for 32 bit aplications.             */
/*  SIZESTACK_BIG       - Size of a big stack. This can be determined             */
/*                        experimentally. We typically use 4K for 16 bit          */
/*                        applications and 8K for 32 bit aplications.             */
/*                        ftp server and web server tasks usually require         */
/*                        large stacks as does the nfs server task. If you        */
/*                        are not using any of these servers set CFG_N_STACKS_BIG */
/*                        to zero.                                                */
/*  CFG_N_STACKS_NORMAL_1   - Number of entries to create in the first allocator  */
/*                        array of stacks of size  SIZESTACK_NORMAL.              */
/*  CFG_N_STACKS_NORMAL_2   - Number of entries to create in the second allocator */
/*                        array of stacks of size  SIZESTACK_NORMAL. If the       */
/*                        first allocator array is big enough to hold all of      */
/*                        the normal stacks then make this zero.                  */
/*  CFG_N_STACKS_BIG        - Number of entries to create in the allocator        */
/*                        array of stacks of size SIZESTACK_BIG. If no            */
/*                        big stacks are needed then make this zero.              */
/*                                                                                */

extern KS_STACK_TYPE normal_1_stacks_array[CFG_N_STACKS_NORMAL_1][SIZESTACK_NORMAL];
#if (CFG_N_STACKS_NORMAL_2)
extern KS_STACK_TYPE normal_2_stacks_array[CFG_N_STACKS_NORMAL_2][SIZESTACK_NORMAL];
#endif
#if (CFG_N_STACKS_BIG)
extern KS_STACK_TYPE big_stacks_array[CFG_N_STACKS_BIG][SIZESTACK_BIG];
#endif
#if (CFG_N_STACKS_HUGE)
extern KS_STACK_TYPE huge_stacks_array[CFG_N_STACKS_HUGE][SIZESTACK_HUGE];
#endif
extern struct  stack_manager KS_FAR sm_array[CFG_N_STACKS_TOTAL];


/* ********************************************************************   */
static void ks_kernel_stackalloc_init(void)
{
int i;
struct  stack_manager KS_FAR *psm;
    psm = &sm_array[0];
    for (i = 0; i < CFG_N_STACKS_NORMAL_1; i++)
    {
        psm->stack_size   = SIZESTACK_NORMAL;
        psm->pstack_base  = normal_1_stacks_array[i];
        psm++;
    }
#if (CFG_N_STACKS_NORMAL_2)
    for (i = 0; i < CFG_N_STACKS_NORMAL_2; i++)
    {
        psm->stack_size   = SIZESTACK_NORMAL;
        psm->pstack_base  = normal_2_stacks_array[i];
        psm++;
    }
#endif
#if (CFG_N_STACKS_BIG)
    for (i = 0; i < CFG_N_STACKS_BIG; i++)
    {
        psm->stack_size   = SIZESTACK_BIG;
        psm->pstack_base  = big_stacks_array[i];
        psm++;
    }
#endif
#if (CFG_N_STACKS_HUGE)
    for (i = 0; i < CFG_N_STACKS_HUGE; i++)
    {
        psm->stack_size   = SIZESTACK_HUGE;
        psm->pstack_base  = huge_stacks_array[i];
        psm++;
    }
#endif
}

/* ********************************************************************   */
static KS_PSTACK ks_kernel_stack_alloc(int stack_size)
{
int i;
struct  stack_manager KS_FAR *psm;

    psm = &sm_array[0];

    for (i = 0; i < CFG_N_STACKS_TOTAL; i++, psm++)
    {
        ks_disable();
        if (psm->stack_size  == stack_size)
        {
            psm->stack_size  = -stack_size;
            ks_enable();
            return(psm->pstack_base);
        }
        ks_enable();
    }
    return(0);
}

/* ********************************************************************   */
static void ks_kernel_stack_free(KS_PSTACK pstack)
{
int i;
struct  stack_manager KS_FAR *psm;
    psm = &sm_array[0];

    for (i = 0; i < CFG_N_STACKS_TOTAL; i++, psm++)
    {
        ks_disable();
        if (psm->pstack_base  == pstack)
        {
            psm->stack_size  = -psm->stack_size;
            ks_enable();
            break;
        }
        ks_enable();
    }
}
/* ********************************************************************   */
/* ===== END RTIP TASKING STACK MANAGEMENT ROUTINES                       */
/* ********************************************************************   */
#endif  /* KS_DECLARE_STACK */

