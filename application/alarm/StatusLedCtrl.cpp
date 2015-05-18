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
/* CLASS NAME       : StatusLedCtrl                                         */
/*                                                                          */
/* FILE NAME        : StatusLedCtrl.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 03-11-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
 /*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
 /*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "gpio.h"
#include "StatusLedCtrl.h"
#include <EventDataPoint.h>
#include <SwTimer.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define RED_LED_FLASH_DELAY 500
#define GREEN_LED_FLASH_TIME 500

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
enum  // TYPES OF SW TIMERS
{
  RED_LED_OFF_DELAY_WITH_ACK_TRY,
  GREEN_LED_FLASH
};

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function - Constructor
 *****************************************************************************/
StatusLedCtrl::StatusLedCtrl()
{
}
/*****************************************************************************
 * Function - Destructor
 ****************************************************************************/
StatusLedCtrl::~StatusLedCtrl()
{
}
/*****************************************************************************
 * Function - InitSubTask
 *****************************************************************************/
void StatusLedCtrl::InitSubTask()
{
  mGreenLedFlashTimeOut = false;
  mRedLedAckTimerStatus = RED_LED_ACK_TIMER_STATUS_IDLE;

  mpTimerObjList[RED_LED_OFF_DELAY_WITH_ACK_TRY] = new SwTimer(RED_LED_FLASH_DELAY, MS, FALSE, FALSE, this);
  mpTimerObjList[GREEN_LED_FLASH] = new SwTimer(GREEN_LED_FLASH_TIME, MS, TRUE, FALSE, this);

  ReqTaskTime();
}
/*****************************************************************************
 * Function - RunSubTask()
 * DESCRIPTION:
 *****************************************************************************/
void StatusLedCtrl::RunSubTask()
{
  GPio* pGPio = GPio::GetInstance();

  /*******************************************************************
   *
   *            RED LED CTRL
   *
   ******************************************************************/
  if (mHasUnackAlarms.IsUpdated() && (mRedLedAckTimerStatus != RED_LED_ACK_TIMER_STATUS_RUNNING))
  {
    if (mHasUnackAlarms->GetValue())
    {
      pGPio->Set(GP_IO_LED_RED, HIGH);
      mpRedStatusLedState->SetValue(STATUS_LED_STATE_ON);
    }
    else
    {
      pGPio->Set(GP_IO_LED_RED, LOW);
      mpRedStatusLedState->SetValue(STATUS_LED_STATE_OFF);
    }
  }

  // if an ack. is given, the red red shall be off for a defined time
  if (mpAlarmResetEvent.IsUpdated() && (mRedLedAckTimerStatus == RED_LED_ACK_TIMER_STATUS_IDLE))
  {
    mpTimerObjList[RED_LED_OFF_DELAY_WITH_ACK_TRY]->StopSwTimer();
    mpTimerObjList[RED_LED_OFF_DELAY_WITH_ACK_TRY]->RetriggerSwTimer();
    mpTimerObjList[RED_LED_OFF_DELAY_WITH_ACK_TRY]->StartSwTimer();
    pGPio->Set(GP_IO_LED_RED, LOW);
    mpRedStatusLedState->SetValue(STATUS_LED_STATE_OFF);
    mRedLedAckTimerStatus = RED_LED_ACK_TIMER_STATUS_RUNNING;
  }


  /*******************************************************************
   *
   *            GREEN LED CTRL
   *
   ******************************************************************/
/*  XHE - platform status LED code.
 *  This is not used in WW Midrange, where a very basic LED control is
 *  intended.
 */
//  if (mGreenLedFlashTimeOut == true)
//  {
//    mGreenLedFlashTimeOut = false;
//    GPio::GetInstance()->Toggle(GP_IO_LED_GREEN);
//  }

//  // basic green led implementation
//  if (mpSystemMode.IsUpdated() || mpOperationMode.IsUpdated())
//  {
//    if (mpOperationMode->GetValue() != OPERATION_MODE_NO_OPERATION)
//    {
//      mpTimerObjList[GREEN_LED_FLASH]->StopSwTimer();
//      pGPio->Set(GP_IO_LED_GREEN, HIGH);
//      mpGreenStatusLedState->SetValue(STATUS_LED_STATE_ON);
//    }
//    else
//    {
//      mpTimerObjList[GREEN_LED_FLASH]->StopSwTimer();
//      pGPio->Set(GP_IO_LED_GREEN, LOW);
//      mpGreenStatusLedState->SetValue(STATUS_LED_STATE_OFF);
//    }
//  }

  // WW Midrange GREEN LED CTRL - always on
  pGPio->Set(GP_IO_LED_GREEN, HIGH);
  mpGreenStatusLedState->SetValue(STATUS_LED_STATE_ON);
}
 /*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *****************************************************************************/
void StatusLedCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
  case SP_SLC_GREEN_STATUS_LED_STATE:
    mpGreenStatusLedState.Attach(pSubject);
    break;
  case SP_SLC_RED_STATUS_LED_STATE:
    mpRedStatusLedState.Attach(pSubject);
    break;

  case SP_SLC_SYSTEM_MODE:
    mpSystemMode.Attach(pSubject);
    break;
  case SP_SLC_OPERATION_MODE:
    mpOperationMode.Attach(pSubject);
    break;
  case SP_SLC_HAS_UNACK_ALARMS:
    mHasUnackAlarms.Attach(pSubject);
    break;
  case SP_SLC_ALARM_RESET_EVENT:
    mpAlarmResetEvent.Attach(pSubject);
    break;
  }
}
 /*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *****************************************************************************/
void StatusLedCtrl::ConnectToSubjects()
{
  mpSystemMode->Subscribe(this);
  mpOperationMode->Subscribe(this);
  mHasUnackAlarms->Subscribe(this);
  mpAlarmResetEvent->Subscribe(this);
}
 /*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *****************************************************************************/
void StatusLedCtrl::Update(Subject* pSubject)
{
  mpSystemMode.Update(pSubject);
  mpOperationMode.Update(pSubject);
  mHasUnackAlarms.Update(pSubject);

  if (mpAlarmResetEvent.Update(pSubject))
  {
    mRedLedAckTimerStatus = RED_LED_ACK_TIMER_STATUS_IDLE;
  }
  else if (pSubject == mpTimerObjList[RED_LED_OFF_DELAY_WITH_ACK_TRY])
  {
    mRedLedAckTimerStatus = RED_LED_ACK_TIMER_STATUS_TIMEOUT;
    mHasUnackAlarms.SetUpdated();
  }
  else if (pSubject == mpTimerObjList[GREEN_LED_FLASH])
  {
    mGreenLedFlashTimeOut = true;
  }

  ReqTaskTime();
}
/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
