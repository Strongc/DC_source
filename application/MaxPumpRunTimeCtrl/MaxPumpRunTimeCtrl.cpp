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
/* CLASS NAME       : MaxPumpRunTimeCtrl                                    */
/*                                                                          */
/* FILE NAME        : MaxPumpRunTimeCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 20-08-2007 dd-mm-yyyy                                 */
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
#include <MaxPumpRunTimeCtrl.h>

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
 *
 *****************************************************************************/
MaxPumpRunTimeCtrl::MaxPumpRunTimeCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  mpMaxPumpRunTimeAlarmDelay = new AlarmDelay(this);
  mMaxPumpRunTimeAlarmDelayCheckFlag = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
MaxPumpRunTimeCtrl::~MaxPumpRunTimeCtrl()
{
  delete(mpMaxPumpRunTimeAlarmDelay);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void MaxPumpRunTimeCtrl::InitSubTask()
{
  // Assure initial check of all parameters when running the task for the first fime
  mpReqOperationMode.SetUpdated();
  mpActualOperationMode.SetUpdated();
  mpMaxPumpRunTimeFaultObj.SetUpdated();

  mMaxPumpRunTimeActiveFlag = false;

  mpMaxPumpRunTimeAlarmDelay->InitAlarmDelay();


  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void MaxPumpRunTimeCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if (mMaxPumpRunTimeAlarmDelayCheckFlag == true)
  {
    mMaxPumpRunTimeAlarmDelayCheckFlag = false;
    mpMaxPumpRunTimeAlarmDelay->CheckErrorTimers();
  }

  /*****************************************************************************
   * Handle change in pump requested operation mode
   * - if anything else but REQ_OPERATION_MODE_AUTO the maxpumpruntimectrl is
   *   not active because manual operation overrules max run time check
   * - if requested operation mode changes to REQ_OPERATION_MODE_AUTO the time
   *   counter must be reset
   ****************************************************************************/
  if (mpReqOperationMode.IsUpdated() == true)
  {
    if (mpReqOperationMode->GetValue() == REQ_OPERATION_MODE_AUTO)
    {
      mMaxPumpRunTimeActiveFlag = true;
      mpPumpRunTimeSinceStart->SetValue(0);
    }
    else
    {
      mMaxPumpRunTimeActiveFlag = false;
    }
  }

  /*****************************************************************************
   * Handle change in pump actual operation mode
   * - if pump changes to ACTUAL_OPERATION_MODE_STARTED the time counter must be reset
   ****************************************************************************/
  if (mpActualOperationMode.IsUpdated() == true)
  {
    mpPumpRunTimeSinceStart->SetValue(0);
  }

  /*****************************************************************************
   * Check for if MaxPumpRunTime has been exceeded if function is active
   ****************************************************************************/
  if (mMaxPumpRunTimeActiveFlag == true)
  {

    if (mpPumpRunTimeSinceStart->GetValue() >= mpMaxPumpRunTimeFaultObj->GetAlarmConfig()->GetWarningLimit()->GetAsInt())
    {
      // Warning limit is reached - set warning
      mpMaxPumpRunTimeAlarmDelay->SetWarning();
    }
    else
    {
      mpMaxPumpRunTimeAlarmDelay->ResetWarning();
    }


    if (mpPumpRunTimeSinceStart->GetValue() >= mpMaxPumpRunTimeFaultObj->GetAlarmConfig()->GetAlarmLimit()->GetAsInt())
    {
      // Alarm limit is reached - set alarm
      mpMaxPumpRunTimeAlarmDelay->SetFault();
    }
    else
    {
      mpMaxPumpRunTimeAlarmDelay->ResetFault();
    }
  }
  else
  {
    // MaxPumpRunTime not active - clear alarm
    mpMaxPumpRunTimeAlarmDelay->ResetFault();
  }

  mpMaxPumpRunTimeAlarmDelay->UpdateAlarmDataPoint();
}


/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void MaxPumpRunTimeCtrl::ConnectToSubjects()
{
  mpPumpRunTimeSinceStart->Subscribe(this);
  mpMaxPumpRunTimeFaultObj->Subscribe(this);  // To catch if alarm/warning limits are changed

  mpActualOperationMode->Subscribe(this);
  mpReqOperationMode->Subscribe(this);

  mpMaxPumpRunTimeAlarmDelay->ConnectToSubjects();
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void MaxPumpRunTimeCtrl::Update(Subject* pSubject)
{
  mpActualOperationMode.Update(pSubject);
  mpReqOperationMode.Update(pSubject);
  mpMaxPumpRunTimeFaultObj.Update(pSubject);

  if (pSubject == mpMaxPumpRunTimeAlarmDelay)
  {
    mMaxPumpRunTimeAlarmDelayCheckFlag = true;
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
void MaxPumpRunTimeCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpMaxPumpRunTimeAlarmDelay->SubscribtionCancelled(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void MaxPumpRunTimeCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_MPRTC_OPERATION_MODE_ACTUAL:
      mpActualOperationMode.Attach(pSubject);
      break;
    case SP_MPRTC_OPERATION_MODE_REQ:
      mpReqOperationMode.Attach(pSubject);
      break;
    case SP_MPRTC_PUMP_MAX_RUN_TIME:
      mpPumpRunTimeSinceStart.Attach(pSubject);
      break;
    case SP_MPRTC_MAX_PUMP_RUN_TIME_FAULT_OBJ:
      mpMaxPumpRunTimeFaultObj.Attach(pSubject);
      mpMaxPumpRunTimeAlarmDelay->SetSubjectPointer(id, pSubject);
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
