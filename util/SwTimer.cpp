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
/* CLASS NAME       : SwTimer                                               */
/*                                                                          */
/* FILE NAME        : SwTimer.cpp                                           */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifdef TO_RUN_ON_PC
#else
#endif
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "SwTimerBassClass.h"
#include "SwTimer.h"
#include "SwTimerTask.h"
/*****************************************************************************
  DEFINES
 *****************************************************************************/
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 * timervalue = between 0 og 32000
 * Unit - MS, S or MINUTE
 * AutoReTrigger, If the SwTimer shall AutoReTrigger after timeout == TRUE
 * startSwTimer, If the SwTimer shall Start at the same time == TRUE
 * timeOutType, If the same obj, starts more than one SwTimer, the type can be specifyed, eg. as an emum.
 * pCallBackObj,  A pointer to the Obj that the TimeOutFunc shall be called in.
 *
 * CAUCTION... CHANGE > MS timer can be over 32000, it is change to be 32Bit value of MS
 *
 *****************************************************************************/
SwTimer::SwTimer(unsigned int timerValue, TIMER_UNIT_TYPE unit, bool autoReTrigger, bool startSwTimer, Observer* pObserver)
{
	if ( timerValue == 0)
	{
		timerValue = 1;
		unit = MS;
	  autoReTrigger = false;
	}

	Subscribe(pObserver);  // AUTO SUBSCRIBE THE SW TIMER.
  mUnit = unit;
  mAutoReTrigger = autoReTrigger;
  mTimer.mpcSpecialData = this;
  mTimerValue = timerValue;

  if (mUnit == MS)
  {
    if ( mTimerValue > 32000)
    {
      mMsPrescaler = ((mTimerValue/32000)-1);
      mMs = 32000;
      mMsRunningLastTime = false;
    }
    else
    {
      mMs = timerValue;
      mMsRunningLastTime = true;
    }
    OS_CreateTimer(&mTimer, &call_back, mMs);
  }
  else if ( mUnit == S)
  {
    mMs = 1000;
    mSecCounter = mSec = (timerValue - 1);
    OS_CreateTimer(&mTimer, &call_back, mMs);
  }
  else if ( mUnit == MINUTE)
  {
    mMs = 30000;
    mHalfMinutesCounter = (timerValue*2 - 1);
    mMin = timerValue;
    OS_CreateTimer(&mTimer, &call_back, mMs);
  }
  if ( startSwTimer == true)
  {
    StartSwTimer();
  }
  else
  {
    mTimerStarted = false;
  }
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
SwTimer::~SwTimer()
{
}
/*****************************************************************************
 * Function - StartSwTimer
 * DESCRIPTION:
 *
 ****************************************************************************/
void SwTimer::StartSwTimer()
{
  OS_StartTimer(&mTimer);
  mTimerStarted = (OS_GetTimerStatus(&mTimer) != 0);
}
/*****************************************************************************
 * Function - StopTimer
 * DESCRIPTION:
 *
 ****************************************************************************/
void SwTimer::StopSwTimer()
{
  OS_StopTimer(&mTimer);
  mTimerStarted = (OS_GetTimerStatus(&mTimer) != 0);
}
/*****************************************************************************
 * Function - RetriggerTimer
 * DESCRIPTION: Retrigger the Timer
 *
 ****************************************************************************/
void SwTimer::RetriggerSwTimer()
{
  if ( mUnit == MS)
  {
    if ( mTimerValue > 32000)
    {
      mMsPrescaler = ((mTimerValue/32000)-1);
      mMs = 32000;
      mMsRunningLastTime = false;
    }
    else
    {
      mMs = mTimerValue;
      mMsPrescaler = 0;
      mMsRunningLastTime = true;
    }
    OS_SetTimerPeriod(&mTimer, mMs);
  }
  else if ( mUnit == S)
  {
    mMs = 1000;
    mSecCounter = mSec;
  }
  else if ( mUnit == MINUTE)
  {
    mMs = 30000;
    mHalfMinutesCounter = (mMin*2 - 1);
  }
  OS_RetriggerTimer(&mTimer);
  mTimerStarted = (OS_GetTimerStatus(&mTimer) != 0);
}
/*****************************************************************************
 * Function - SetSwTimerPeriod
 * DESCRIPTION: Sets the SwTimerPeriod according to the arguments.
 *
 ****************************************************************************/
void SwTimer::SetSwTimerPeriod(unsigned int timerValue, TIMER_UNIT_TYPE unit, bool autoReTrigger)
{
	if ( timerValue == 0)
	{
		timerValue = 1;
		unit = MS;
	  autoReTrigger = false;
	}
	mAutoReTrigger = autoReTrigger;

	mUnit = unit;
  mTimerValue = timerValue;
  if ( mUnit == MS)
  {
    if ( mTimerValue > 32000)
    {
      mMsPrescaler = ((mTimerValue/32000)-1);
      mMs = 32000;
      mMsRunningLastTime = false;
    }
    else
    {
      mMs = mTimerValue;
      mMsPrescaler = 0;
      mMsRunningLastTime = true;
    }
    OS_SetTimerPeriod(&mTimer, mMs);
  }
  else if ( mUnit == S)
  {
    mMs = 1000;
    mSecCounter = mSec = (timerValue - 1);
    OS_SetTimerPeriod(&mTimer, mMs);
  }
  else if ( mUnit == MINUTE)
  {
    mMs = 30000;
    mHalfMinutesCounter = (timerValue*2 - 1);
    mMin = timerValue;
    OS_SetTimerPeriod(&mTimer, mMs);
  }
  if (mTimerStarted == true || mAutoReTrigger == true)
  {
    OS_RetriggerTimer(&mTimer);
  }
}
/*****************************************************************************
 * Function - GetSwTimerPeriod
 * DESCRIPTION: Returns the time according to the timerUnit Type as Argument.
 *
 ****************************************************************************/
unsigned int SwTimer::GetSwTimerPeriode()
{
  return mTimerValue;
}
/*****************************************************************************
 * Function - GetSwTimerValue
 * DESCRIPTION:
 *
 ****************************************************************************/
unsigned int SwTimer::GetSwTimerValue()
{
  unsigned int returnvalue = 0;
  if ( mUnit == MS)
  {
    returnvalue = (mMsPrescaler*32000);
    if ( mMsRunningLastTime != true)
    {
      returnvalue = returnvalue + (mTimerValue%32000);
    }
    returnvalue = returnvalue + OS_GetTimerValue(&mTimer);
  }
  else if ( mUnit == S)
  {
    returnvalue = (mSecCounter);
  }
  else if ( mUnit == MINUTE)
  {
    returnvalue = (mHalfMinutesCounter/2);
  }
  return returnvalue;
}
/*****************************************************************************
 * Function - GetSwTimerStatus
 * DESCRIPTION:
 * Return TRUE is STARTED, FALSE IF STOPPE.
 *
 ****************************************************************************/
bool SwTimer::GetSwTimerStatus()
{
  return mTimerStarted;
}

/*****************************************************************************
 * Function - TimeOut
 * DESCRIPTION: This operations is called from SwTimerTask
 *
 ****************************************************************************/
void SwTimer::TimeOut()
{
  NotifyObservers();
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - RealTimeOut
 * DESCRIPTION: This is the function that is called via the RealCallback function.
 * This is called when a SwTimer EXPIRE.
 *
 ****************************************************************************/
void SwTimer::RealTimeOut()
{
  if ( mAutoReTrigger == true)
  {
    if ( mUnit == MS)
    {
      if ( mTimerValue > 32000)
      {
        mMsPrescaler = ((mTimerValue/32000)-1);
        mMs = 32000;
        mMsRunningLastTime = false;
      }
      else
      {
        mMs = mTimerValue;
        mMsPrescaler = 0;
        mMsRunningLastTime = true;
      }
      OS_SetTimerPeriod(&mTimer, mMs);
      OS_RetriggerTimer(&mTimer);
    }
    else if ( mUnit == S)
    {
      mMs = 1000;
      mSecCounter = mSec;
    }
    else if ( mUnit == MINUTE)
    {
      mMs = 30000;
      mHalfMinutesCounter = (mMin*2 - 1);
    }
    OS_RetriggerTimer(&mTimer);
  }
  else
  {
    mTimerStarted = false;
  }
  SwTimerTask::GetInstance()->PutTimeOutEventInQueue(this);
}
/*****************************************************************************
 * Function - RealCallback
 * DESCRIPTION: This is the C++ function, that the C functions calls.
 *
 ****************************************************************************/
void SwTimer::RealCallback()
{
  if ( mUnit == MS)
  {
    if ( mTimerValue > 32000)
    {
      if (mMsPrescaler > 0)
      {
        mMsPrescaler --;
        OS_RetriggerTimer(&mTimer);
      }
      else if (mMsPrescaler == 0 && mMsRunningLastTime == false)
      {
        mMsRunningLastTime = true;
        auto int tempvalue;
        tempvalue = (mTimerValue%32000);
        //tempvalue = (mTimerValue%tempvalue);
        OS_SetTimerPeriod(&mTimer, (tempvalue));
        OS_RetriggerTimer(&mTimer);
      }
      else if (mMsPrescaler == 0 && mMsRunningLastTime == true)
      {
        RealTimeOut();
      }
    }
    else
    {
      RealTimeOut();
    }
  }
  else if ( mUnit == S)
  {
    if ( mSecCounter > 0)
    {
      mSecCounter--;
      OS_RetriggerTimer(&mTimer);
    }
    else if (mSecCounter == 0)
    {
      RealTimeOut();
    }
  }
  else if ( mUnit == MINUTE)
  {
    if ( mHalfMinutesCounter > 0)
    {
      mHalfMinutesCounter--;
      OS_RetriggerTimer(&mTimer);
    }
    else if (mHalfMinutesCounter == 0)
    {
      RealTimeOut();
    }
  }
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
/*****************************************************************************
 *
 *
 *              ANSI C FUNCTIONS
 *
 ****************************************************************************/
/*****************************************************************************
 * Function:    call_back
 * DESCRIPTION:
 *
 *****************************************************************************/
extern "C" void call_back(void)
{
  SwTimer* real_this = (SwTimer*)(OS_GetpCurrentTimer()->mpcSpecialData);
  real_this->RealCallback();
}


