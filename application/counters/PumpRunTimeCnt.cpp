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
/* CLASS NAME       : PumpRunTimeCnt                                        */
/*                                                                          */
/* FILE NAME        : PumpRunTimeCnt.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
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
#include <PumpRunTimeCnt.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  PUMP_RUN_TIMER
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
PumpRunTimeCnt::PumpRunTimeCnt()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PumpRunTimeCnt::~PumpRunTimeCnt()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpRunTimeCnt::InitSubTask()
{
  mpTimerObjList[PUMP_RUN_TIMER] = new SwTimer(1000, MS, true, false, this);

  mPumpRunTimerFlag = false;
  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpRunTimeCnt::RunSubTask()
{
  mRunRequestedFlag = false;

  if (mpActualOperationMode.IsUpdated() == true)
  {
    if (mpActualOperationMode->GetValue() == ACTUAL_OPERATION_MODE_STARTED)
    {
      // Pump is running - start the timer and clear the latest runtime
      mpTimerObjList[PUMP_RUN_TIMER]->RetriggerSwTimer();
      mpPumpLatestRunTime->SetValue(0);
    }
    else
    {
      // Pump is not running - make sure timer is stopped.
      mpTimerObjList[PUMP_RUN_TIMER]->StopSwTimer();
    }
  }

  if (mPumpRunTimerFlag == true)
  {
    mPumpRunTimerFlag = false;

    // Increment if no danger for overrun
    if (mpPumpRunTime->GetValue() != mpPumpRunTime->GetMaxValue())
    {
      mpPumpRunTime->SetValue(mpPumpRunTime->GetValue() + 1);
    }
    if (mpPumpRunTimeSinceService->GetValue() != mpPumpRunTimeSinceService->GetMaxValue())
    {
      mpPumpRunTimeSinceService->SetValue(mpPumpRunTimeSinceService->GetValue() + 1);
    }
    if (mpPumpMaxRunTime->GetValue() != mpPumpMaxRunTime->GetMaxValue())
    {
      mpPumpMaxRunTime->SetValue(mpPumpMaxRunTime->GetValue() + 1);
    }
    if (mpPumpLatestRunTime->GetValue() != mpPumpLatestRunTime->GetMaxValue())
    {
      mpPumpLatestRunTime->SetValue(mpPumpLatestRunTime->GetValue() + 1);
    }
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void PumpRunTimeCnt::ConnectToSubjects()
{
  mpActualOperationMode->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void PumpRunTimeCnt::Update(Subject* pSubject)
{
  mpActualOperationMode.Update(pSubject);

  if (pSubject == mpTimerObjList[PUMP_RUN_TIMER])
  {
    mPumpRunTimerFlag = true;
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
void PumpRunTimeCnt::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void PumpRunTimeCnt::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_PRTC_ACTUAL_OPERATION_MODE:
      mpActualOperationMode.Attach(pSubject);
      break;
    case SP_PRTC_PUMP_RUN_TIME:
      mpPumpRunTime.Attach(pSubject);
      break;
    case SP_PRTC_PUMP_RUN_TIME_SINCE_SERVICE:
      mpPumpRunTimeSinceService.Attach(pSubject);
      break;
    case SP_PRTC_PUMP_MAX_RUN_TIME:
      mpPumpMaxRunTime.Attach(pSubject);
      break;
    case SP_PRTC_PUMP_LATEST_RUN_TIME:
      mpPumpLatestRunTime.Attach(pSubject);
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
