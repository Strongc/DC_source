/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos                                     */
/*               All rights reserved                                        */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/****************************************************************************/
/* CLASS NAME       : PowerDown                                             */
/*                                                                          */
/* FILE NAME        : PowerDown.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 16-03-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifndef __PC__
#include <vr4181a.h>
#endif
#include <rtos.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <display_task.h>
#include <ConfigControl.h>
#include <AlarmControl.h>
#include <GPio.h>
#include <pwm_drv.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <PowerDown.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define POWER_DOWN_STACK_SIZE   2000
/* Disable watchdog while setting timer cycle - Watchdog timer cycle = 1 sec - Enable watchdog */
#define WATCH_DOG_INIT_1_SEC()  (*(volatile GF_UINT16*)WDTCNTREG) = 0x0000; \
                                (*(volatile GF_UINT16*)WDTSETREG) = 0x0001; \
                                (*(volatile GF_UINT16*)WDTCNTREG) = 0x0001

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  VARIABLES
 *****************************************************************************/
static AlarmControl* mpAlarmControl = 0;
OS_STACKPTR int PowerDownTaskStack[POWER_DOWN_STACK_SIZE];
OS_TASK TCBPowerDownTask;


/*****************************************************************************
 FUNCTIONS
 *****************************************************************************/
void TurnOffBackLightAndLeds(void);

/*****************************************************************************
 * Function - PowerDownTask
 * DESCRIPTION: Task that handles power down sequence
 *
 *****************************************************************************/
EXTERN void PowerDownTask(void)
{
  OS_U8 event, wait_mask;

  event = OS_WaitEvent(POWER_DOWN_EVENT);

  // *** Back Light and LEDs ***
  TurnOffBackLightAndLeds();

  // *** Display ***
  StopDisplay();

    // *** Ethernet ***
  //EthernetPowerDown();

  // *** AlarmControl ***
  if (mpAlarmControl)
  {
    mpAlarmControl->PowerDown();
  }

  // *** ConfigControl ***
  ConfigControl::GetInstance()->PowerDown();


  wait_mask = CONFIG_CTRL_POWERED_DOWN_EVENT | DISPLAY_POWERED_DOWN_EVENT;
  while (wait_mask)
  {
    event = OS_WaitEvent(wait_mask);
    wait_mask &= ~event;
  }

  // Reset flash
  GPio::GetInstance()->SetAs(GPIO38, OUTPUT, 0);
  OS_Delay(1);//XFK use 1 ms delay as flash control
  GPio::GetInstance()->Set(GPIO38, 1);
  

  // Watch dog is reinitiated and started
  FEED_DOG();
  WATCH_DOG_INIT_1_SEC();

  // Kill program!!!!
  while (1)
  {
  }
}

/*****************************************************************************
 * Function - TurnOffBackLightAndLeds
 * DESCRIPTION:
 *
 *****************************************************************************/
void TurnOffBackLightAndLeds(void)
{
  SetDutyCycleHW_1(4095);                       // Backlight off
#ifndef TFT_16_BIT_LCD
  SetDutyCycleHW_2(4095);                       // Contrast off
#endif
  GPio::GetInstance()->SetAs(GPIO8, OUTPUT, 1); // Disable all LED's
}

/*****************************************************************************
 * Function - PowerDownIrq
 * DESCRIPTION: Checks if it is irq from early warning and handles irq if so
 *
 *****************************************************************************/
#ifdef __PC__
EXTERN BOOL PowerDownIrq(void)
{
  return true;
}
#else
EXTERN BOOL PowerDownIrq(void)
{
  auto BOOL ret_value = false;
  auto U16 local_sysint0reg;

  local_sysint0reg = *(U16*)SYSINT0REG;

  if (local_sysint0reg & 0x0001)
  {
    *(U16*)MSYSINT0REG &= ~1; /* disable interrupt from NMI pin */
    *(U16*)PMUINTREG |= (1 << 1); /* Clear interrupt request be writing 1 to NMIR */

    TurnOffBackLightAndLeds();
    SignalEventToPowerDown(POWER_DOWN_EVENT);
    ret_value = true;
  }

  return ret_value;
}
#endif
/*****************************************************************************
 * Function - EnablePowerDownIrq
 * DESCRIPTION: Assigns the NMI to Int(3:0), and enables irq
 *
 *****************************************************************************/
#ifdef __PC__
EXTERN void EnablePowerDownIrq(void)
{
  //noop
}
#else
EXTERN void EnablePowerDownIrq(void)
{
  OS_CREATETASK(&TCBPowerDownTask, "Power Down Task", PowerDownTask, 250, PowerDownTaskStack);

  *(U16*)PMUINTREG |= (1 << 1); /* Clear interrupt request be writing 1 to NMIR */
  *(U16*)MSYSINT0REG |= 1; /* Enable interrupt from NMI pin */
}
#endif
/*****************************************************************************
 * Function - ChkSupplyVoltage
 * DESCRIPTION:
 *
 *****************************************************************************/
#ifdef __PC__
EXTERN void ChkSupplyVoltage(void)
{
}
#else
EXTERN void ChkSupplyVoltage(void)
{
  while(!GPio::GetInstance()->Read(EARLY_WARNING))                   // Hold program until supply voltage is over threshold
  {
  }
  *(U16*)NMICTRLREG |= 1;  /* NMI works as normal interrupt */
  *(U16*)MSYSINT0REG &= ~1; /* disable interrupt from NMI pin */
  *(U16*)PMUINTREG |= (1 << 1); /* Clear interrupt request be writing 1 to NMIR */
}
#endif
/*****************************************************************************
 * Function - SetAlarmControlPtrForPowerDown
 * DESCRIPTION: Function to receive pointer to AlarmControl
 *
 *****************************************************************************/
EXTERN void SetAlarmControlPtrForPowerDown(void* pAlarmControl)
{
  mpAlarmControl = (AlarmControl*)pAlarmControl;
}

/*****************************************************************************
 * Function - SignalEventToPowerDown
 * DESCRIPTION: Function to receive events
 *
 *****************************************************************************/
EXTERN void SignalEventToPowerDown(OS_U8 event)
{
  OS_SignalEvent(event, &TCBPowerDownTask);
}


