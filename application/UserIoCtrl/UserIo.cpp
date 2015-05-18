/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : UserIo                                                */
/*                                                                          */
/* FILE NAME        : UserIo.h                                              */
/*                                                                          */
/* CREATED DATE     : 15-12-2008  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
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
#include "UserIo.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
// SW Timers
enum
{
  MIN_HOLD_TIME_TIMER,
  MAX_HOLD_TIME_TIMER
};

/********************************************************************
LIFECYCLE - Default constructor.
********************************************************************/
UserIo::UserIo()
{
  mpTimerObjList[MIN_HOLD_TIME_TIMER] = new SwTimer(10, S, false, false, this);
  mpTimerObjList[MAX_HOLD_TIME_TIMER] = new SwTimer(10, S, false, false, this);
  mMinHoldTimeTimeOut = true;
  mMaxHoldTimeTimeOut = true;
}

/********************************************************************
LIFECYCLE - Destructor.
********************************************************************/
UserIo::~UserIo()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void UserIo::InitSubTask()
{
  mRunRequestedFlag = true;
  ReqTaskTime(); // Start up
}


/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void UserIo::RunSubTask()
{
  mRunRequestedFlag = false;
  bool config_enabled = mpUserIoConfig->GetEnabled();

  if (config_enabled)
  {
    SetQuality(DP_AVAILABLE);
  }
  else
  {
    if (GetValue() == true)
    {
      // release output relay when configuration is disabled
      SetValue(false);
    }

    SetQuality(DP_NOT_AVAILABLE);
    return;
  }
  
  bool newOutput = GetOutput();
  bool currentOutput = GetValue();

  U32 max_hold_time = mpUserIoConfig->GetMaxHoldTime();
  if (!mpUserIoConfig->GetMaxHoldTimeEnabled())
  {
    max_hold_time = 0;
  }

  if (max_hold_time > 0 && newOutput && currentOutput && mMaxHoldTimeTimeOut)
  {
    //force low if max hold time condition is meet
    SetValue(false);
  }
  else if (newOutput != currentOutput)
  {
    // output changed

    U32 min_hold_time = mpUserIoConfig->GetMinHoldTime();

    if (newOutput)
    {
      // rising edge

      SetValue(newOutput);
      if (min_hold_time > 0)
      {
        mMinHoldTimeTimeOut = false;
        mpTimerObjList[MIN_HOLD_TIME_TIMER]->SetSwTimerPeriod(min_hold_time, S, false);
        mpTimerObjList[MIN_HOLD_TIME_TIMER]->RetriggerSwTimer();
      }

      if (max_hold_time > 0)
      {
        mMaxHoldTimeTimeOut = false;
        mpTimerObjList[MAX_HOLD_TIME_TIMER]->SetSwTimerPeriod(max_hold_time, S, false);
        mpTimerObjList[MAX_HOLD_TIME_TIMER]->RetriggerSwTimer();
      }

    }
    else
    {
      // falling edge
      if (min_hold_time == 0 || mMinHoldTimeTimeOut)
      {
        SetValue(newOutput);
      }

    }
  }
}


/********************************************************************
Function - GetOutput
********************************************************************/
bool UserIo::GetOutput()
{
  bool ch1 = mpChannel1->GetValue();
  bool ch2 = mpChannel2->GetValue();
  bool output = GetValue();

  switch (mpUserIoConfig->GetLogic())
  {
    case USER_IO_LOGIC_AND:
      output = ch1 && ch2;
      break;
    case USER_IO_LOGIC_OR:
      output = ch1 || ch2;
      break;
    case USER_IO_LOGIC_XOR:
      output = ch1 ^ ch2;
      break;
    case USER_IO_LOGIC_RESET_SET_LATCH:
      output = (ch1 ? false : (ch2 ? true : output));
      break;
    case USER_IO_LOGIC_SET_RESET_LATCH:
      output = (ch1 ? true : (ch2 ? false : output));
      break;
    case USER_IO_LOGIC_TOGGLE_LATCH:
      // Toggle output when both both channels are high
      output = (ch1 && ch2 ? !output : output);
      break;
  }

  if (mpUserIoConfig->GetInverted())
  {
    output = !output;
  }

  return output;
}


/********************************************************************
Function - GetUserIoConfig
********************************************************************/
UserIoConfig* UserIo::GetUserIoConfig()
{
  return mpUserIoConfig.GetSubject();
}


/*****************************************************************************
* Observer::Update implementation
*****************************************************************************/
void UserIo::Update(Subject* pSubject)
{
  if (pSubject == mpTimerObjList[MIN_HOLD_TIME_TIMER])
  {
    mMinHoldTimeTimeOut = true;
  }
  else if (pSubject == mpTimerObjList[MAX_HOLD_TIME_TIMER])
  {
    mMaxHoldTimeTimeOut = true;
  }
  else if (mpUserIoConfig.Update(pSubject)){}
  
  if (mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
* Observer::SetSubjectPointer implementation
*****************************************************************************/
void UserIo::SetSubjectPointer(int Id,Subject* pSubject)
{
  switch (Id)
  {
  case SP_UDF_CONFIG:
    mpUserIoConfig.Attach(pSubject);
    mpUserIoConfig.SetUpdated();
    break;
  case SP_UDF_SOURCE_1:
    mpChannel1.Attach(pSubject);
    break;
  case SP_UDF_SOURCE_2:
    mpChannel2.Attach(pSubject);
    break;
  }
}

/*****************************************************************************
* Observer::ConnectToSubjects implementation
*****************************************************************************/
void UserIo::ConnectToSubjects(void)
{
  mpUserIoConfig->Subscribe(this);
  mpChannel1->Subscribe(this);
  mpChannel2->Subscribe(this);
}

/*****************************************************************************
* Observer::SubscribtionCancelled implementation
*****************************************************************************/
void UserIo::SubscribtionCancelled(Subject* pSubject)
{
  mpUserIoConfig.Detach(pSubject);
  mpChannel1.Detach(pSubject);
  mpChannel2.Detach(pSubject);
}


