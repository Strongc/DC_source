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
/* CLASS NAME       : DailyEmptyingCtrl                                     */
/*                                                                          */
/* FILE NAME        : DailyEmptyingCtrl.cpp                                 */
/*                                                                          */
/* CREATED DATE     : 24-07-2007 dd-mm-yyyy                                 */
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
#include <DailyEmptyingCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  PIT_EMPTYING_MAX_TIMER
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
DailyEmptyingCtrl::DailyEmptyingCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
DailyEmptyingCtrl::~DailyEmptyingCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DailyEmptyingCtrl::InitSubTask()
{
  mpDailyEmptyingActivationTime = new MpcTime(true);

  mpDailyEmptyingActivationTime->SetTime(MINUTES, (mpPitEmptyingTimeOfTheDay->GetValue()/60)%60);
  mpDailyEmptyingActivationTime->SetTime(HOURS, mpPitEmptyingTimeOfTheDay->GetValue()/3600);

  mpDailyEmptyingActivationTimeObs = new MpcTimeCmpCtrl(mpDailyEmptyingActivationTime, this, CMP_TIME);

  mpTimerObjList[PIT_EMPTYING_MAX_TIMER] = new SwTimer(mpPitEmptyingMaxTime->GetValue(), S, false, false, this);

  mpUnderLowestStopLevel.SetUpdated();

  mPitEmptyingMaxTimeOutFlag = false;
  mTimeForDailyEmptying = false;
  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DailyEmptyingCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  // Handle change in daily emptying enable
  if (mpDailyEmptyingED.IsUpdated())
  {
    if (mpDailyEmptyingED->GetValue()==false)
    {
      //Request not needed anymore
      mpTimerObjList[PIT_EMPTYING_MAX_TIMER]->StopSwTimer();
      mpDailyEmptyingRequested->SetValue(false);
    }
  }

  // Handle change in daily emptying times
  if (mpPitEmptyingTimeOfTheDay.IsUpdated())
  {
    mpDailyEmptyingActivationTime->SetTime(MINUTES, (mpPitEmptyingTimeOfTheDay->GetValue()/60)%60);
    mpDailyEmptyingActivationTime->SetTime(HOURS, mpPitEmptyingTimeOfTheDay->GetValue()/3600);
  }

  // Handle daily emptying
  if ( (mpDailyEmptyingDigInRequest.IsUpdated() && mpDailyEmptyingDigInRequest->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
    || (mTimeForDailyEmptying == true) )
  {
    mTimeForDailyEmptying = false;

    if (mpDailyEmptyingED->GetValue() == true)
    {
      if (mpUnderLowestStopLevel->GetValue() == false)
      {
        mpDailyEmptyingRequested->SetValue(true);
      }

     // Start timer for clearing request
     mpTimerObjList[PIT_EMPTYING_MAX_TIMER]->SetSwTimerPeriod(mpPitEmptyingMaxTime->GetValue(), S, false);
     mpTimerObjList[PIT_EMPTYING_MAX_TIMER]->RetriggerSwTimer();
    }
  }

  /*****************************************************************************
   * Handle stop of daily emptying
   ****************************************************************************/
  if (mpUnderLowestStopLevel.IsUpdated() == true)
  {
    if ((mpUnderLowestStopLevel->GetValue() == true) &&
        (mpDailyEmptyingRequested->GetValue() == true))
    {
      // Level has sunk - request not needed anymore
      mpDailyEmptyingRequested->SetValue(false);
    }
  }

  /*****************************************************************************
   * Handle Max Allow timout
   ****************************************************************************/
  if (mPitEmptyingMaxTimeOutFlag == true)
  {
    mPitEmptyingMaxTimeOutFlag = false;
    mpDailyEmptyingRequested->SetValue(false);
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void DailyEmptyingCtrl::ConnectToSubjects()
{
  mpDailyEmptyingED->Subscribe(this);
  mpPitEmptyingTimeOfTheDay->Subscribe(this);
  mpUnderLowestStopLevel->Subscribe(this);
  mpDailyEmptyingDigInRequest->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void DailyEmptyingCtrl::Update(Subject* pSubject)
{
  mpDailyEmptyingED.Update(pSubject);
  mpPitEmptyingTimeOfTheDay.Update(pSubject);
  mpUnderLowestStopLevel.Update(pSubject);
  mpDailyEmptyingDigInRequest.Update(pSubject);

  if (pSubject == mpDailyEmptyingActivationTimeObs)
  {
    mTimeForDailyEmptying = true;
  }
  else if (pSubject == mpTimerObjList[PIT_EMPTYING_MAX_TIMER])
  {
    mPitEmptyingMaxTimeOutFlag = true;
  }


  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void DailyEmptyingCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void DailyEmptyingCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_DEC_PIT_EMPTYING_TIME_OF_THE_DAY:
      mpPitEmptyingTimeOfTheDay.Attach(pSubject);
      break;
    case SP_DEC_PIT_EMPTYING_MAX_TIME:
      mpPitEmptyingMaxTime.Attach(pSubject);
      break;
    case SP_DEC_UNDER_LOWEST_STOP_LEVEL:
      mpUnderLowestStopLevel.Attach(pSubject);
      break;
    case SP_DEC_DAILY_EMPTYING_REQUESTED:
      mpDailyEmptyingRequested.Attach(pSubject);
      break;
    case SP_DEC_DAILY_EMPTYING_DIG_IN_REQUEST:
      mpDailyEmptyingDigInRequest.Attach(pSubject);
      break;
    case SP_DEC_DAILY_EMPTYING_ED:
      mpDailyEmptyingED.Attach(pSubject);
      break;
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
