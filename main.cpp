/*****************************************************************************
* Copyright, Grundfos
* All rights reserved.
* No part of this program may be copied, used or delivered to anyone
* without the express written consent of Grundfos *
* MODULENAME.: main
*
* PROJECT....: Waste Water midrange project (Dedicated Controls)
*
* DESCRIPTION:
* This is the main-file build to build uppon
*
*****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <typedef.h>    /*definements for the Grundfos common types */
extern "C"
{
  #include <RTOS.H>
}
#include <microp.h>
#include <ledclass.h>
#include <lowlevel.h>
#include <factory\Factory.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <GUI_VNC.h>
#include <GUI.h>

#include <LCD_ConfDefaults.h>

#include <pwm_drv.h>
#include <gpio.h>

#include <TaskCtrlEvent.h>
#include <TaskCtrlPeriodic.h>
#include <geni_if.h>
#include <IobComDrv.h>

#include <Mailbox.h>
#include <AlarmControl.h>
#include <AlarmLog.h>
#include <AlarmEvent.h>
#include <AlarmDef.h>
#include <keyboard.h>
#include <display_task.h>
#include <ConfigControl.h>
#include <PowerDown.h>
#include <SwTimerTask.h>

#ifndef __PC__
#include <Ethernet_hwc.h>
#include <serv.h>
#include <ipconfig/MPCIPConfig.h>

#ifdef TCS_USB_SER_PORT
#include <USB_HW_Driver.h>
#endif
#endif

#ifdef TCS_USB_SER_PORT
#define USB_ENABLE	1
#define USB_TEST	1
#define USB_DEBUG	1
#else
#define USB_ENABLE	0
#define USB_TEST	0
#define USB_DEBUG	0
#endif

#include <Factory.h>

/*****************************************************************************
  DEFINES
 ***************************************************************************/
#ifdef  DISABLE_WATCHDOG
  #undef  ENABLE_WATCHDOG
#else
  #define ENABLE_WATCHDOG //replace with #define/#undef to enable/disable watchdog
#endif

#ifdef __PC__
  #define WEB_ENABLE 0  // disable web-server
  #define GUI_ENABLE 0  // disable gui support
  #define OS_DebugSendString(a)
#else
  #define WEB_ENABLE 1  // enable web-server
  #define GUI_ENABLE 1  // enable gui support
#endif

#define STACK_SIZE 2000

/*****************************************************************************
  LOCAL VARIABLES
 *****************************************************************************/
// task stacks
OS_STACKPTR int ControllerPeriodicStack[STACK_SIZE];
OS_STACKPTR int ControllerEventsTaskStack[STACK_SIZE];
OS_STACKPTR int GeniAppTaskStack[STACK_SIZE];
OS_STACKPTR int LowPrioPeriodicTaskStack[STACK_SIZE];
OS_STACKPTR int DisplayEventTaskStack[STACK_SIZE];

// Task-control-blocks
OS_TASK TCBControllerPeriodicTask;
OS_TASK TCBControllerEventsTask;
OS_TASK TCBGeniAppTask;
OS_TASK TCBLowPrioPeriodicTask;
OS_TASK TCBDisplayEventTask;

/*****************************************************************************
 *              PRIVATE FUNCTIONS
 *****************************************************************************/
extern "C" void ControllerPeriodicTask(void)
{
  int start_time, end_time, delay;

  TaskCtrl* pControllerPeriodicTask = dynamic_cast<TaskCtrl*>(GetControllerPeriodicTask());

  while (1)
  {
    start_time = OS_GetTime();

    FEED_DOG();

    pControllerPeriodicTask->RunSubTasks();

    end_time = OS_GetTime();
    delay = 10 - (end_time - start_time);
    if (delay < 1)
    {
      delay = 1;
    }
    else if (delay > 10)
    {
      delay = 10;
    }

    OS_Delay(delay);
  }
}

extern "C" void ControllerEventsTask(void)
{
  TaskCtrl* pControllerEventsTask = dynamic_cast<TaskCtrl*>(GetControllerEventsTask());

  while (1)
  {
    pControllerEventsTask->RunSubTasks();
    OS_Delay (10);
  }
}

extern "C" void GeniAppTask(void)
{
  TaskCtrl* pGeniAppTask = dynamic_cast<TaskCtrl*>(GetGeniAppTask());

  while (1)
  {
    pGeniAppTask->RunSubTasks();
    OS_Delay (10);
  }
}


extern "C" void LowPrioPeriodicTask(void)
{
  TaskCtrl* pLowPrioPeriodicTask = dynamic_cast<TaskCtrl*>(GetLowPrioPeriodicTask());

  int old_ms  = OS_GetTime() - 1000;
  int new_ms  = 0;
  int wait_ms = 0;
  int diff_ms = 0;
  int lost_ms = 0;

  while (1)
  {
    new_ms = OS_GetTime();
    pLowPrioPeriodicTask->RunSubTasks();
    // Now calculate how long to wait before next task-run.
    // This is made to make the task run exactly one time per second.
    diff_ms = OS_GetTime() - new_ms;
    wait_ms = 1000 - diff_ms;
    diff_ms = new_ms - old_ms;
    old_ms = new_ms;
    lost_ms += diff_ms;
    lost_ms -= 1000;
    if (lost_ms > 10000)
    {
      // The accumulated time loss is very high. Do not try to catch up any further.
      // This may happen, but only at extreemly high cpu load.
      lost_ms = 10000;
    }
    else if (lost_ms < 0)
    {
      // Should not happen, but just in case.
      lost_ms = 0;
    }
    // Wait a bit shorter to catch up in case of accumulated time loss.
    wait_ms -= lost_ms/20;
    if (wait_ms > 1000)
    {
      // Should not happen, but just in case.
      wait_ms = 1000;
    }
    else if (wait_ms < 500)
    {
      // Avoid running the task too often.
      wait_ms = 500;
    }
    OS_Delay(wait_ms);
  }
}

extern "C" void DisplayEventTask(void)
{
  TaskCtrl* pDisplayEventsTask = dynamic_cast<TaskCtrl*>(GetDisplayEventsTask());

  while (1)
  {
    pDisplayEventsTask->RunSubTasks();
    OS_Delay (20);
  }
}

void InitRandomGenerator()
{
  U32DataPoint* pPowerOnCnt = dynamic_cast<U32DataPoint*>(GetSubject(SUBJECT_ID_POWER_ON_CNT));
  U32DataPoint* pMacLo = dynamic_cast<U32DataPoint*>(GetSubject(SUBJECT_ID_ETHERNET_MAC_ADDRESS_LO));

  int random = 0;
  int count = 0;
  int seed;
  int start_cnt;
  int mac_lo;

#ifndef __PC__
  count =  OS_MIPS_GetCount();
  random = OS_MIPS_GetRandom()<<11;
#endif
  start_cnt =  pPowerOnCnt->GetValue();
  mac_lo    =  pMacLo->GetValue();
  seed = 0x1234;
  seed = random*count*mac_lo*start_cnt;
  srand( seed%0x7FFF );
  for(int i=0;i<rand()%10;i++)
  {
    rand();
  }
  srand(rand());
}


/* --------------------------------------------------
* Entry point
* --------------------------------------------------*/
int main (void)
{
  /* --------------------------------------------------
  * Initialize watch dog
  * --------------------------------------------------*/
#ifndef __PC__
  #ifdef ENABLE_WATCHDOG
    // Watch dog is initiated and started - it must now be fed periodically
    WATCH_DOG_INIT();
    FEED_DOG();
  #else
    WATCH_DOG_DISABLE();
  #endif
  ClearBevBit();
#endif
  
#if USB_ENABLE == 1
  InitUsbDriverStack();
#endif

  /* --------------------------------------------------
  * Check supply voltage
  * --------------------------------------------------*/
  ChkSupplyVoltage();

  /* --------------------------------------------------
  * Initialize OS kernel
  * --------------------------------------------------*/
  OS_InitKern();        

  /* -----------------02-05-2005 14:30----------- FKA -
  * Has to be called before interrupt is enabled otherwize
  * new is called within the interrupt !
  * --------------------------------------------------*/
  IobComDrv::GetInstance()->InitInterComTask();

  /* --------------------------------------------------
  * Initialize software timer task
  * --------------------------------------------------*/
  SwTimerTask::GetInstance();

  /* --------------------------------------------------
  * Create IP objects
  * --------------------------------------------------*/
#if WEB_ENABLE == 1
  MPCIPConfig::createObjects();
#endif

  /* --------------------------------------------------
  * Initialize Hardware for OS
  * --------------------------------------------------*/
  OS_InitHW();

  /* --------------------------------------------------
  * Enable power down
  * --------------------------------------------------*/
  EnablePowerDownIrq();

  /* --------------------------------------------------
  * Init display and create display task
  * --------------------------------------------------*/
  DisplayInit();

  /* --------------------------------------------------
  * Init ConfigControl and create ConfigControl task
  * --------------------------------------------------*/
  ConfigControl::GetInstance()->Init();

  /* --------------------------------------------------
  * Create tasks
  * --------------------------------------------------*/
  OS_CREATETASK(&TCBControllerPeriodicTask, "Controller Periodic Task", ControllerPeriodicTask, 150, ControllerPeriodicStack);
  OS_CREATETASK(&TCBControllerEventsTask, "Controller Events Task", ControllerEventsTask, 180, ControllerEventsTaskStack);
  OS_CREATETASK(&TCBGeniAppTask, "Geni App Task", GeniAppTask, 120, GeniAppTaskStack);
#ifdef __PC__
  //Priority 100 is used on pc, because inter com is prio 100 and uses all the task time given
  OS_CREATETASK(&TCBLowPrioPeriodicTask, "Low Prio Periodic Task", LowPrioPeriodicTask, 100, LowPrioPeriodicTaskStack);
#else
  OS_CREATETASK(&TCBLowPrioPeriodicTask, "Low Prio Periodic Task", LowPrioPeriodicTask, 10, LowPrioPeriodicTaskStack);
#endif
  OS_CREATETASK(&TCBDisplayEventTask, "Display Event Task", DisplayEventTask, 110, DisplayEventTaskStack);

#ifdef __PERFORMANCE_TEST___
  extern void InitPerformanceTest(void);
  InitPerformanceTest();  // Create a task for performance test
#endif

  /* --------------------------------------------------
  * Run the factory
  * --------------------------------------------------*/
  RunFactory();

  /* --------------------------------------------------
  * Initialize IP config
  * --------------------------------------------------*/
#if WEB_ENABLE == 1
  MPCIPConfig::init();
#endif

  /* The line below disables interrupt from GPIO 47
   * If geni needs it, it will enable it in GeniSysInit().
   * If running with embOSView from jtag emulator internal
   * registers are not reset, so the CPU will probably run with
   * the configuration from the flash image.
   * We need to make sure it is disabled, so we do it here */
#ifndef __PC__
  (*(UINT *)GPINTMSK2) &= 0x7FFF;
#endif

  /*  Geni */
  geni_setup = 2;            // Reply on connection request. Must be set prior to GeniSysInit.
  GeniSysInit();
  slave_unit_addr  = 231;    // set unit address for bus channel
  master_unit_addr = 1;      // set unit address for bus channel
  slave_group_addr = 232;    // set group address for bus channel
  EnableMasterMode();        // the default mode is slave mode - so change it

  /* --------------------------------------------------
  * Initialize the clock
  * --------------------------------------------------*/
  Clock::GetInstance()->InitSubTask();

  /* --------------------------------------------------
  * initialize random generator
  * --------------------------------------------------*/
  InitRandomGenerator();

  /* --------------------------------------------------
  * Initialize sub tasks
  * --------------------------------------------------*/
  GetGeniAppTask()->InitSubTasks();
  GetControllerEventsTask()->InitSubTasks();
  GetControllerPeriodicTask()->InitSubTasks();
  GetLowPrioPeriodicTask()->InitSubTasks();
  GetDisplayEventsTask()->InitSubTasks();

  /* --------------------------------------------------
  * Internal communication
  * --------------------------------------------------*/
#ifndef __PC__
  OS_CREATEMB(&MBIobTransmit, 1, sizeof(MBIobTransmitBuf), &MBIobTransmitBuf);
  OS_CREATEMB(&MBIobReceive, 1, sizeof(MBIobReceiveBuf), &MBIobReceiveBuf);
#endif

  FEED_DOG();

  /* --------------------------------------------------
  * Start IP
  * --------------------------------------------------*/
#if WEB_ENABLE == 1
  MPCIPConfig::startThreads();
#endif

  /* --------------------------------------------------
  * Start multitasking
  * --------------------------------------------------*/
  OS_Start();
}
