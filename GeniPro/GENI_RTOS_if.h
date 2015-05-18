/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:   GENIpro                                        */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos Electronics A/S, 2000               */
/*                                                                          */
/*                            All rights reserved                           */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* MODULE NAME      :  Geni_RTOS_if                                         */
/*                                                                          */
/* FILE NAME        :  Geni_RTOS_if.h                                       */
/*                                                                          */
/* FILE DESCRIPTION :  Interface to RTOS                                    */
/*                                                                          */
/****************************************************************************/

#ifndef _GENI_RTOS_IF_H
  #define _GENI_RTOS_IF_H

#include <typedef.h>            /*definements for the Grundfos common types */
#include "geni_cnf.h"

/*****************************************************************************/
/* RTOS disabled specification                                               */
/*****************************************************************************/
#if (GENI_RTOS_TYPE == Disable)
  #include "microp.h"
  #define SEMA_TYPE UCHAR

/*****************************************************************************/
/* EmbOS specification                                                       */
/*****************************************************************************/
#elif (GENI_RTOS_TYPE == GENI_EMB_OS)
  #include "RTOS.H"
  #include "mips_irq.h"
  #define SEMA_TYPE OS_RSEMA

  #if (SEGMENT_CHANGE_ALLOWED == TRUE)
    #pragma memory=dataseg(GENI_TSK)
  #endif

  extern SEMA_TYPE geni_master;                       // semaphor for active GENI master task

  #if (SEGMENT_CHANGE_ALLOWED == TRUE)
    #pragma memory=default
  #endif

  //  interrupt definitions
  #if (defined MIPS)
    #define GENI_ENTER_NESTABLE_INT()
    #define GENI_ENTER_INT()
    #define GENI_LEAVE_NESTABLE_INT()
    #define GENI_LEAVE_INT()
  #else
    #define GENI_ENTER_NESTABLE_INT()   OS_EnterNestableInterrupt(); OS_EnterIntStack()
    #define GENI_ENTER_INT()            OS_EnterInterrupt(); OS_EnterIntStack()
    #define GENI_LEAVE_NESTABLE_INT()   OS_LeaveIntStack(); OS_LeaveNestableInterruptNoSwitch()
    #define GENI_LEAVE_INT()            OS_LeaveIntStack(); OS_LeaveInterruptNoSwitch()
  #endif

  #define GENI_ENABLE_GLOBAL_INT()    OS_DecRI()
  #define GENI_DISABLE_GLOBAL_INT()   OS_IncDI()
  //  RTOS definitions
  #define GENI_SYS_TIMER_INIT        {OS_CreateTimer(&GeniTimer, GeniSysIrq, 5); \
                                      OS_StartTimer(&GeniTimer);}
  // Ressource to enable the GeniTimer interrupt
  #define GENI_SYS_TIMER_ENABLE(a)    OS_DecRI()//OS_LeaveRegion()
  // Specify whether or not GENI_SYS_TIMER_DISABLE returns the previous state of the timer
  #define DISABLE_TIMER_RETURNS_STATE FALSE                         // TRUE: DISABLE TIMER returns state
                                                                     // FALSE: no return state
  // Ressource to disable the GeniTimer interrupt, must always be defined
  #define GENI_SYS_TIMER_DISABLE()    OS_IncDI()//OS_EnterRegion()
  // How to reload the timer
  #define GENI_SYS_TIMER_RELOAD       OS_RetriggerTimer(&GeniTimer)
  // Signal from Genipro to execute process ( Task )
  #define GENI_SYS_TASK_RUN           OS_SignalEvent(GENI_EVENT,&GeniSysTaskHandle)        // then run geni_task to handle it
  // Signal from Genipro that it has received control
  #define GENI_SYS_TASK_WAIT

  // Semaphors
  // Wait for access to class data
  #define GENI_USE_CLASS_DATA         OS_Use(&geni_class_data);
  // Stop using class data
  #define GENI_UNUSE_CLASS_DATA       OS_Unuse(&geni_class_data);
  // wait for access the master
  #define GENI_USE_MASTER             OS_Use(&geni_master);
  // Stop using the master
  #define GENI_UNUSE_MASTER           OS_Unuse(&geni_master);


/*****************************************************************************/
/* Grundfos RTOS Timer 0 specification                                       */
/*****************************************************************************/
#elif (GENI_RTOS_TYPE == GENI_GF_OS_TIMER0)
  #include "rtos_if.h"

  #define SEMA_TYPE UCHAR
  //Ressource to init the GeniTimer interrupt - not used with rtos UserIrq
  #define GENI_SYS_TIMER_INIT         _SOFT_TIMER(0, MilliSec2T_Inc(5)); _SOFT_TIMER0_IRQ_ENABLE()
   //Ressource to enable the GeniTimer interrupt
  #define GENI_SYS_TIMER_ENABLE(a)    _SOFT_TIMER0_IRQ_ENABLE()
  // Specify whether or not GENI_SYS_TIMER_DISABLE returns the previous state of the timer
  #define DISABLE_TIMER_RETURNS_STATE FALSE                         // TRUE: DISABLE TIMER returns state
                                                                    // FALSE: no return state
  // Ressource to disable the GeniTimer interrupt, must always be defined
  #define GENI_SYS_TIMER_DISABLE()    _SOFT_TIMER0_IRQ_DISABLE()
  // How to reload the timer
  #define GENI_SYS_TIMER_RELOAD       _SOFT_TIMER(0, MilliSec2T_Inc(5)) // set Genipro system interrupt freq
  // Signal from Genipro to execute process ( Task )
  #define GENI_SYS_TASK_RUN           _RUN_TASK(TASK_ID_GENI_SYS)        // then run geni_task to handle it
  // Signal from Genipro that it has received control
  #define GENI_SYS_TASK_WAIT          _WAIT(255,0)                       // default

/*****************************************************************************/
/* Grundfos RTOS user irq specification                                      */
/*****************************************************************************/
#elif (GENI_RTOS_TYPE == GENI_GF_OS_USER_IRQ)
  #include "rtos_if.h"
  #define SEMA_TYPE UCHAR

  // Ressource to init the GeniTimer interrupt
  #define GENI_SYS_TIMER_INIT         // not used with rtos UserIrq
  // Ressource to enable the GeniTimer interrupt
  #define GENI_SYS_TIMER_ENABLE(a)    ENABLE_RTOS_TIMER_IRQ(a)     //_SOFT_TIMER0_IRQ_ENABLE()        // default
  // Specify whether or not GENI_SYS_TIMER_DISABLE returns the previous state of the timer
  #define DISABLE_TIMER_RETURNS_STATE TRUE                          // TRUE: DISABLE TIMER returns state
                                                                      // FALSE: no return state
  // Ressource to disable the GeniTimer interrupt, must always be defined
  #define GENI_SYS_TIMER_DISABLE()    DISABLE_RTOS_TIMER_IRQ()    //_SOFT_TIMER0_IRQ_DISABLE()       // default
  // How to reload the timer
  #define GENI_SYS_TIMER_RELOAD
  // Signal from Genipro to execute process ( Task )
  #define GENI_SYS_TASK_RUN           _RUN_TASK(TASK_ID_GENI_SYS)     // then run geni_task to handle it
  // Signal from Genipro that it has received control
  #define GENI_SYS_TASK_WAIT          _WAIT(255,0)                    // default

/*****************************************************************************/
/* User defined RTOS specification                                           */
/*****************************************************************************/
#elif (GENI_RTOS_TYPE == GENI_GF_OS_USER_DEF)
  #include "rtos_if.h"
  #define SEMA_TYPE UCHAR

#else
  #error "GENI_RTOS_TYPE not supported";
#endif

/*****************************************************************************/
/* Defines for not using embOs                                               */
/*****************************************************************************/
#if (GENI_RTOS_TYPE != GENI_EMB_OS)
  #define GENI_ENTER_NESTABLE_INT()  ENABLE_GLOBAL_INT()
  #define GENI_ENTER_INT()
  #define GENI_LEAVE_NESTABLE_INT()
  #define GENI_LEAVE_INT()

  #define GENI_ENABLE_GLOBAL_INT()   ENABLE_GLOBAL_INT()
  #define GENI_DISABLE_GLOBAL_INT()  DISABLE_GLOBAL_INT()

  // Semaphors
  // Wait for access to class data
  #define GENI_USE_CLASS_DATA
  // Stop using class data
  #define GENI_UNUSE_CLASS_DATA
  // wait for access the master
  #define GENI_USE_MASTER
  // Stop using the master
  #define GENI_UNUSE_MASTER

#endif

#endif // _GENI_RTOS_IF_H
/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/
