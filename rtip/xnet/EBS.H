/*                                                                        */
/* EBS.H  - EBS header file                Version 4.1                    */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains prototypes constants and structure           */
/*        declarations required to build RTIP and RTFS libraries and      */
/*        applications.                                                   */
/*        There should be no need to change values in this file.          */
/*                                                                        */

#ifndef __EBS__
#define __EBS__ 1


/* ********************************************************************   */
#ifdef __cplusplus
#define RTIP_CDECL(X)  extern "C" {X}
#else
#define RTIP_CDECL(X)  X
#endif

/* ********************************************************************   */
#include "ostype.h"         
#include "xnconf.h"         /* uses types in ostype.h */
#include "vfconf.h"
#include "ethconf.h"        /* uses types in ostype.h */
#include "dymconf.h"
#include "osenv.h"          /* types etc */

#if (INCLUDE_RTIP)
#include "rtipapi.h"        
#endif
#include "osint.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (!INCLUDE_RTIP)
#include "debugapi.h"
#endif 

#if (defined(SMX) && !defined(STATIC))
/* *********************************************************************   */
/* Use STATIC in function definitions instead of static, so they will be
   public symbols when debugging.
*/
#ifdef DEBUGVER
 #define STATIC
#else
 #define STATIC static
#endif
#endif

/* *********************************************************************   */
#include "sock.h"
#include "servers.h"

/* *********************************************************************   */
/* entry in errno<->task table                                             */
typedef void (*PSPAWN_TASK)(void);
#if (INCLUDE_TERMMAN)
/* This is so it can be used below.   */
struct terminal_context;
typedef struct terminal_context KS_FAR * PTERMCONTEXT;
#endif

/* *********************************************************************   */
/* RTIP calls calls interrupts as normal 'C' functions. Code in 
   os_port.c converts the isr layer to 'C' and calls the device specific
   interrupt service routine */
typedef void (*RTIPINTFN_POINTER)(int);

/* RTIP's os_spawn_task() calls spanws tasks that ultimately call functions
  of type void func(void). This is a pointer to a function of that type
  and is used in the argument list to os_spawn_task() */
typedef void (*PEBS_TASKENTRY)(void);

/* *********************************************************************   */
typedef struct system_user
{
    int                 rtip_errno; /* current errno value for the task */
    PFVOID              udata0;     /* pointers for context blocks or */
    PFVOID              udata1;     /* other user data. We have up to */
    PFVOID              udata2;     /* four available */
    PFVOID              udata3;     /*  */
#if (INCLUDE_TERMMAN)
    PFVOID              terminal_context; /* Points to the current terminal */
                                          /* context for this task;    */
                                          /* the type is PTERMCONTEXT  */
                
#endif
#if (INCLUDE_ERTFS_USER_STRUCT)
    int           dfltdrv_is_setup;           /* Zero at first */
    int           dfltdrv;                    /* Default drive to use if no drive specified  */
    PFVOID /* struct drobj * */lcwd[NDRIVES]; /* current working dirs  */
#endif
#if (INCLUDE_MFS && INCLUDE_SUBDIRS)
    /* current working directory; MFS never changes the cwd but just   */
    /* keeps track of it; it does all commands on the full path        */
    PFCHAR              mfcwd;
#endif
#if INCLUDE_SYSLOG
    word                write2syslog_active;   /* used to prevent recursive diagnostic kernel-syslogging within RTIP */
#endif
} SYSTEM_USER;

typedef struct system_user KS_FAR *PSYSTEM_USER;

/* *********************************************************************    */
/* Task Classes - Each type of task has an associated class. Each           */
/* class has a priority and a stack size. The task class is passed          */
/* to os_spawn_task() when the task is spawned. The task priority           */
/* and task stack are assigned to the task internally                       */
/* by os_spawn_task(). Hence the TASKCLASS_XX defines are publicly          */
/* and the PRIOTASK_XXX and SIZESTACK_XXX defines are used by osport.c only */
/* (PRIOTASK_XXX and SIZESTACK_XXX are in osport.h)                         */
/* Each SIZESTACK_XXX is assigned a size (SIZESTACK_NORMAL or SIZESTACK_BIG */
/* are defined in osport.h and are kernel dependent).                       */
/*                                                                          */
#define TASKCLASS_CURRENT            0   /* Virtual task class */
#define TASKCLASS_TIMER              1   /* The background timer */
#define TASKCLASS_IP                 2   /* IP dispatcher one per interface */
#define TASKCLASS_INTERRUPT          3   /* Interrupt task one per interface     */
#define TASKCLASS_FTP_DAEMON         4   /* The ftp daemon */
#define TASKCLASS_WEB_DAEMON         5   /* The web daemon */
#define TASKCLASS_HTTPS_DAEMON       6   /* The https server */
#define TASKCLASS_TELNET_DAEMON      7   /* The telnet daemon */
#define TASKCLASS_FTP_SERVER         8   /* The ftp server task(s) */
#define TASKCLASS_TFTP_DAEMON        9   /* The ftp server task(s) */
#define TASKCLASS_WEB_SERVER         10  /* The web server task(s) */
#define TASKCLASS_HTTPS_SERVER       11  /* The https server task(s) */
#define TASKCLASS_TELNET_SERVER      12  /* The telnet server task(s) */
#define TASKCLASS_NFS_SERVER         13  /* The nfs server task(s) */
#define TASKCLASS_SNMP_AGENT         14  /* The snmp agent task(s) */
#define TASKCLASS_DHCP_SERVER        15  /* The snmp agent task(s) */
#define TASKCLASS_MAIN_APPTASK       16  /* The main task for ebs demos */
#define TASKCLASS_DEMO_APPTASK       17  /* Tasks spawned by the demos */
#define TASKCLASS_USER_APPTASK       18  /* User applications code */
#define TASKCLASS_RIP_DAEMON         19  /* RIP daemon task */
/* NOTE: TASKCLASS_EXTERN_APPTASK needs to be last   */
#define TASKCLASS_EXTERN_APPTASK     20  /* Here and above are never spawned */
                                         /* this is the task_class passed   */
                                         /* to os_spawn_task for user       */
                                         /* tasks                           */


/* *********************************************************************   */
#include "ebslib.h"

/* *********************************************************************   */
/* ******           Function prototypes common to rtip/rtfs ************   */
/* *********************************************************************   */

/* File OSPORT.C   */
void         ks_sleep(word);
dword        ks_get_ticks(void);
word         ks_ticks_p_sec(void);
word         ks_msec_p_tick(void);
int          ks_kernel_init(void);
void         ks_kernel_run(void);
PSYSTEM_USER os_get_user(void);
PSYSTEM_USER os_get_user_by_index(int taskindex);
RTIP_BOOLEAN      os_spawn_task(int task_class, PEBS_TASKENTRY entry_point, 
                           PFVOID v0, PFVOID v1, PFVOID v2, PFVOID v3);
int          ks_get_task_index(void);

RTIP_BOOLEAN         ks_hook_interrupt(int irq, PFVOID piface, RTIPINTFN_POINTER c_strategy, RTIPINTFN_POINTER c_interrupt, int c_arg);
void         ks_mask_isr_on(int irq);
void         ks_mask_isr_off(int irq);
RTIP_BOOLEAN     ks_resource_init(void);
void        os_exit_task(void);

/* File OSENV.C   */
void    ks_enable(void);
void    ks_disable(void);
int     phys_to_virtual(PFBYTE * virt, unsigned long phys);
dword   kvtop(PFBYTE p);
void    ks_restore_interrupts(void);

/* File OSINT.C   */
void ks_clear_isr(int irq);

/* File TASKS.c   */
void tc_timer_main(void);

/* File TOOLS.c   */
#if (KS_ARGSUSED == ARGSUSED_FUNC)
void      argsused_pvoid(PFVOID p);  /* Use these to keep the compiler happy  */
void      argsused_int(int i);       /* in a few places */
#endif

int       set_errno(int error);

PSYSTEM_USER get_system_user(void);

#ifdef __cplusplus
}
#endif

#include "osport.h"
#include "terminal.h"
#include "ostime.h"

#endif /* __EBS__ */


