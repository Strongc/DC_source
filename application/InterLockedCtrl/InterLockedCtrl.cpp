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
/* CLASS NAME       : InterLockedCtrl                                       */
/*                                                                          */
/* FILE NAME        : InterLockedCtrl.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 16-03-2008 dd-mm-yyyy                                 */
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
#include <InterLockedCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  INTERLOCKED_TIMEOUT_TIMER,
  INTERLOCKED_REMAIN_TIME_TIMER
  };


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
 *
 *****************************************************************************/
InterLockedCtrl::InterLockedCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
InterLockedCtrl::~InterLockedCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void InterLockedCtrl::InitSubTask()
{
  mInterLockTimeoutTimerFlag = false;
  mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER] = new SwTimer(mpInterLockTimeout->GetValue(), S, false, false, this);
  ///\Todo 20150424 JMH-> Add Interlock Update Timer Here
  mpTimerObjList[INTERLOCKED_REMAIN_TIME_TIMER] = new SwTimer( 1, S, true, true, this);

  if (mpInterLocked->GetValue() == true)
  {
    // The system is starting up in the interlocked state - the timeout timer must be started.
    mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->RetriggerSwTimer();
  }

  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void InterLockedCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if (mpInterLockTimeout.IsUpdated())
  {
    // If an interlock is already active an update of the time should cause a restart of the interlock period
    if (mpInterLocked->GetValue() == true)
    {
      mpInterLockRequestEvent.SetUpdated();
    }
  }

  if (mpInterLockIgnore->GetValue() == false)
  {
    // Interlock is allowed
    bool interlock_on_request = false;
    bool interlock_off_request = false;

    if (mpInterlockDigInRequest.IsUpdated())
    {
      if (mpInterlockDigInRequest->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
      {
        interlock_on_request = true;
        mpInterLockTimeout->SetValueAsPercent(100); //As long as possible
      }
      else
      {
        interlock_off_request = true;
      }
    }
    if (mpInterLockRequestEvent.IsUpdated())
    {
      interlock_on_request = true;
    }
    if (interlock_on_request == true)
    {
      // A request for interlocking has been received - set interlocked and (re)start timer for interlock duration
      mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->SetSwTimerPeriod(mpInterLockTimeout->GetValue(), S, false);
      mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->RetriggerSwTimer();
      mInterLockTimeoutTimerFlag = false;  // This flag is cleared because the timer has just been restarted, so any pending timeout is not relevant any more.
      mpInterLocked->SetValue(true);
    }

    if (mpInterLockOffEvent.IsUpdated())
    {
      interlock_off_request = true;
    }

    if (interlock_off_request == true)
    {
      // A request for interlocking off has been received - remove interlocking.
      // As this request is handled after an ON-request this means that if both on and off requests are received simultaneously the OFF-request
      // will have the highest priority.
      mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->StopSwTimer();
      mInterLockTimeoutTimerFlag = false;  // This flag is cleared because it is not needed now that the timer has been stopped.
      mpInterLocked->SetValue(false);
    }


  }
  else
  {
    // Interlock is not allowed - if an interlock is present then remove it
    mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->StopSwTimer();
    mInterLockTimeoutTimerFlag = false;  // This flag is cleared because it is not needed now that the timer has been stopped.
    mpInterLocked->SetValue(false);

    // Clear any requests for interlocking on or off as they are not allowed
    mpInterLockRequestEvent.ResetUpdated();
    mpInterLockOffEvent.ResetUpdated();
  }


  // An interlock period has timed out - remove the interlock
  if (mInterLockTimeoutTimerFlag == true)
  {
    mInterLockTimeoutTimerFlag = false;

    mpInterLocked->SetValue(false);
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void InterLockedCtrl::ConnectToSubjects()
{
  mpInterLockTimeout->Subscribe(this);
  mpInterLockIgnore->Subscribe(this);
  mpInterLockRequestEvent->Subscribe(this);
  mpInterLockOffEvent->Subscribe(this);
  mpInterlockDigInRequest->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void InterLockedCtrl::Update(Subject* pSubject)
{
  mpInterLockRequestEvent.Update(pSubject);
  mpInterLockOffEvent.Update(pSubject);
  mpInterLockTimeout.Update(pSubject);
  mpInterlockDigInRequest.Update(pSubject);

  if (pSubject == mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER])
  {
    mInterLockTimeoutTimerFlag = true;
  }

  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }

  ///\Todo 20150424 JMH-> Add Interlock Remain Time Here
  mpInterLockTimeRemain->SetValue(mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->GetSwTimerValue());
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void InterLockedCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void InterLockedCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_ILC_INTERLOCK_REQUEST:
      mpInterLockRequestEvent.Attach(pSubject);
      break;
    case SP_ILC_INTERLOCK_OFF_REQUEST:
      mpInterLockOffEvent.Attach(pSubject);
      break;
    case SP_ILC_INTERLOCK_TIMEOUT:
      mpInterLockTimeout.Attach(pSubject);
      break;
    case SP_ILC_INTERLOCK_IGNORE:
      mpInterLockIgnore.Attach(pSubject);
      break;
    case SP_ILC_INTERLOCKED:
      mpInterLocked.Attach(pSubject);
      break;
    case SP_ILC_INTERLOCK_DIG_IN_REQUEST:
      mpInterlockDigInRequest.Attach(pSubject);
      break;
    case SP_ILC_INTERLOCK_REMAIN_TIME:
      mpInterLockTimeRemain.Attach(pSubject);

    default:
      break;

  }
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
