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
/* CLASS NAME       : AntiSeizingCtrl                                       */
/*                                                                          */
/* FILE NAME        : AntiSeizingCtrl.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 28-08-2007 dd-mm-yyyy                                 */
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
#include <AntiSeizingCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  ANTI_SEIZE_WAIT_TIMER,
  ANTI_SEIZE_RUN_TIMER
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
AntiSeizingCtrl::AntiSeizingCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AntiSeizingCtrl::~AntiSeizingCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AntiSeizingCtrl::InitSubTask()
{
  mpTimerObjList[ANTI_SEIZE_RUN_TIMER] = new SwTimer(mpAntiSeizingRunTime->GetValue(), S, false, false, this);
  mpTimerObjList[ANTI_SEIZE_WAIT_TIMER] = new SwTimer(mpAntiSeizingWaitTime->GetValue(), S, true, false, this);

  mAntiSeizingState = ANTI_SEIZING_PREVENTED;
  mAntiSeizingLegalState = ANTI_SEIZING_LEGAL_WAITING;
  mpPumpAntiSeizingReq->SetValue(false);  // Initiate to false as this is the value the request should have in
                                          // the state ANTI_SEIZING_PREVENTED

  mAntiSeizingRunTimerFlag = false;
  mAntiSeizingWaitTimerFlag = false;

  mRunRequestedFlag = true;
  ReqTaskTime();                         // Assures task is run at startup
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AntiSeizingCtrl::RunSubTask()
{
  ACTUAL_OPERATION_MODE_TYPE actual_operation_mode;
  bool anti_seizing_ed = false;
  bool event_was_anti_seizing_ed = false;
  bool event_was_operation_change = false;
  bool anti_seizing_request_by_dig_in = false;
  bool pump_ready = true;

  mRunRequestedFlag = false;

  // When leaving ANTI_SEIZING_PREVENTED it is necessary to know what caused the state change.
  // Thus the updates of the two possible causes are checked and the values are put into temporary
  // variables to be used for the rest of the function. This is to assure that all updates are cleared
  // and that a new update coming while executing the rest of the function will be carried on to
  // the next run of the function.
  if (mpAntiSeizingED.IsUpdated() == true)
  {
    event_was_anti_seizing_ed = true;
  }
  if (mpActualOperationModePump.IsUpdated() == true)
  {
    event_was_operation_change = true;
  }

  actual_operation_mode = mpActualOperationModePump->GetValue();
  if (actual_operation_mode == ACTUAL_OPERATION_MODE_NOT_INSTALLED || actual_operation_mode == ACTUAL_OPERATION_MODE_DISABLED)
  {
    pump_ready = false;
  }
  anti_seizing_ed = mpAntiSeizingED->GetValue();

  anti_seizing_request_by_dig_in = (mpAntiSeizingDigInRequest.IsUpdated() &&
                                   (mpAntiSeizingDigInRequest->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE));


  switch (mAntiSeizingState)
  {
    case ANTI_SEIZING_PREVENTED:
      // Change to legal is only allowed if Anti Seizing is enabled AND the pump is ready.
      if (pump_ready == true && anti_seizing_ed == true)
      {
        if (event_was_anti_seizing_ed == true)
        {
          // If anti seizing function is enabled an anti seizing should be carried out imideately
          mAntiSeizingState = ANTI_SEIZING_LEGAL;
          mpPumpAntiSeizingReq->SetValue(true);
          if (actual_operation_mode == ACTUAL_OPERATION_MODE_STARTED)
          {
            // Pump is already running - start run timer to check anti seizing will run for the requested time
            mAntiSeizingLegalState = ANTI_SEIZING_LEGAL_RUNNING;
            // Start run timer for Pump
            mpTimerObjList[ANTI_SEIZE_RUN_TIMER]->SetSwTimerPeriod(mpAntiSeizingRunTime->GetValue(), S, false);
            mpTimerObjList[ANTI_SEIZE_RUN_TIMER]->RetriggerSwTimer();
          }
          else
          {
            // Request an anti seizing
            mAntiSeizingLegalState = ANTI_SEIZING_LEGAL_REQUESTED;
          }
        }
        else
        {
          // Anti seizing is legal due to another event than enabling - just go into the wait state
          mAntiSeizingState = ANTI_SEIZING_LEGAL;
          mpPumpAntiSeizingReq->SetValue(false);
          mAntiSeizingLegalState = ANTI_SEIZING_LEGAL_WAITING;
          if (actual_operation_mode != ACTUAL_OPERATION_MODE_STARTED)
          {
            // Pump is not running - activate the wait timer
            mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->SetSwTimerPeriod(mpAntiSeizingWaitTime->GetValue(), S, false);
            mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->RetriggerSwTimer();
          }
        }
      }
      break;
    case ANTI_SEIZING_LEGAL:
      // First check, if anti seizing is not legal anymore and we have to go to the prevented state
      if (anti_seizing_ed == false ||
          pump_ready == false)
      {
        // Going to prevented. Stop all timers and remove a pending request flag
        // There is no need to initiate the wait-request-running state macine state. This state will be
        // initialized when returning to legal.
        // Leaving the wait-request-running state as it is could help during debugging as we can see what
        // the state machine was doing before going to prevented state.
        mAntiSeizingState = ANTI_SEIZING_PREVENTED;
        mpPumpAntiSeizingReq->SetValue(false);
        StopTimers();
      }
      else
      {
        // Anti seizing is legal - now check the wait-request-running state machine
        switch (mAntiSeizingLegalState)
        {
          case ANTI_SEIZING_LEGAL_WAITING:
            if (actual_operation_mode == ACTUAL_OPERATION_MODE_STARTED)
            {
              // Pump is running - stop the wait timer
              mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->StopSwTimer();
              mAntiSeizingWaitTimerFlag = false;
            }
            // Then check if events require a state change.
            else if (anti_seizing_request_by_dig_in == true || mAntiSeizingWaitTimerFlag == true)
            {
              // Wait timer timeout, or request from digital input
              // Stop wait timer - in case the change is caused by digital input
              mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->StopSwTimer();
              mAntiSeizingWaitTimerFlag = false;
              mpPumpAntiSeizingReq->SetValue(true);
              mAntiSeizingLegalState = ANTI_SEIZING_LEGAL_REQUESTED;
            }
            // If no state change needed then check if wait timer should be restarted.
            else if (event_was_operation_change == true && actual_operation_mode != ACTUAL_OPERATION_MODE_STARTED)  // In this state operation_mode can only be one of the two states STOPPED or STARTED
            {
              // Pump has just been removed from the running state - wait timer should be started.
              mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->SetSwTimerPeriod(mpAntiSeizingWaitTime->GetValue(), S, false);
              mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->RetriggerSwTimer();
            }
            break;
          case ANTI_SEIZING_LEGAL_REQUESTED:
            // Check for pump to start - this is the only way to leave the requested state
            if (actual_operation_mode == ACTUAL_OPERATION_MODE_STARTED)
            {
              mAntiSeizingLegalState = ANTI_SEIZING_LEGAL_RUNNING;
              // Start run timer for Pump
              mpTimerObjList[ANTI_SEIZE_RUN_TIMER]->SetSwTimerPeriod(mpAntiSeizingRunTime->GetValue(), S, false);
              mpTimerObjList[ANTI_SEIZE_RUN_TIMER]->RetriggerSwTimer();
            }
            break;
          case ANTI_SEIZING_LEGAL_RUNNING:
            if (mAntiSeizingRunTimerFlag == true)
            {
              // Run timer timeout - anti seizing is finished, return to waiting for next anti seizing
              mAntiSeizingRunTimerFlag = false;
              mAntiSeizingLegalState = ANTI_SEIZING_LEGAL_WAITING;
              mpPumpAntiSeizingReq->SetValue(false);
              // If the pump is not running then start wait timer for Pump
              if (actual_operation_mode != ACTUAL_OPERATION_MODE_STARTED)
              {
                mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->SetSwTimerPeriod(mpAntiSeizingWaitTime->GetValue(), S, false);
                mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->RetriggerSwTimer();
              }
            }
            break;
          default:
            // Illegal state - go to waiting to get state machine back on track
            mAntiSeizingLegalState = ANTI_SEIZING_LEGAL_WAITING;
            StopTimers();
            if (actual_operation_mode != ACTUAL_OPERATION_MODE_STARTED)
            {
              // Pump is not running - activate the wait timer
              mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->SetSwTimerPeriod(mpAntiSeizingWaitTime->GetValue(), S, false);
              mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->RetriggerSwTimer();
            }
            break;
        }
      }
      break;
    default:
      // Illegal state - go to prevented to get state machine back on track
      mAntiSeizingState = ANTI_SEIZING_PREVENTED;
      mpPumpAntiSeizingReq->SetValue(false);
      StopTimers();
      break;
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void AntiSeizingCtrl::ConnectToSubjects()
{
  mpAntiSeizingED->Subscribe(this);
  mpAntiSeizingDigInRequest->Subscribe(this);
  mpActualOperationModePump->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void AntiSeizingCtrl::Update(Subject* pSubject)
{
  mpActualOperationModePump.Update(pSubject);
  mpAntiSeizingDigInRequest.Update(pSubject);
  mpAntiSeizingED.Update(pSubject);

  if (pSubject == mpTimerObjList[ANTI_SEIZE_RUN_TIMER])
  {
    mAntiSeizingRunTimerFlag = true;
  }
  else if (pSubject == mpTimerObjList[ANTI_SEIZE_WAIT_TIMER])
  {
    mAntiSeizingWaitTimerFlag = true;
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
void AntiSeizingCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void AntiSeizingCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_ASC_OPERATION_MODE_ACTUAL:
      mpActualOperationModePump.Attach(pSubject);
      break;
    case SP_ASC_ANTI_SEIZING_DIG_IN_REQUEST:
      mpAntiSeizingDigInRequest.Attach(pSubject);
      break;
    case SP_ASC_ANTI_SEIZING_ED:
      mpAntiSeizingED.Attach(pSubject);
      break;
    case SP_ASC_ANTI_SEIZING_RUN_TIME:
      mpAntiSeizingRunTime.Attach(pSubject);
      break;
    case SP_ASC_ANTI_SEIZING_WAIT_TIME:
      mpAntiSeizingWaitTime.Attach(pSubject);
      break;
    case SP_ASC_PUMP_ANTI_SEIZING_REQ:
      mpPumpAntiSeizingReq.Attach(pSubject);
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
 * Function - StopTimers
 * DESCRIPTION: Stop the run- and waittimer and clear all timerflags. Intended
 *              for initializing the timers.
 *
 *****************************************************************************/
void AntiSeizingCtrl::StopTimers(void)
{
  mpTimerObjList[ANTI_SEIZE_RUN_TIMER]->StopSwTimer();
  mpTimerObjList[ANTI_SEIZE_WAIT_TIMER]->StopSwTimer();
  mAntiSeizingWaitTimerFlag = false;
  mAntiSeizingRunTimerFlag = false;
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
