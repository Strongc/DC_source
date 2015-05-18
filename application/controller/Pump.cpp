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
/* CLASS NAME       : Pump                                                  */
/*                                                                          */
/* FILE NAME        : Pump.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 05-07-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
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
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <Pump.h>             // class implemented

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  CONTACTOR_ALARM_RESET_TIMER,
  CONTACTOR_IGNORE_TIMER,
  START_START_TIMER,
  STOP_STOP_TIMER,
  START_STOP_START_TIMER
};


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  STATIC VARIABLES
 *****************************************************************************/
  int  Pump::mLastEvent;                    //TEST JSM

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
 *Sets pointers and ref. to 0
 *****************************************************************************/
Pump::Pump()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_PUMP_FAULT_OBJ; fault_id < NO_OF_PUMP_FAULT_OBJ; fault_id++)
  {
    mpPumpAlarmDelay[fault_id] = new AlarmDelay(this);
    mPumpAlarmDelayCheckFlag[fault_id] = false;
  }
  mStartStartTimeOut = false;
  mStartStopStartTimeOut = false;
  mStopStopTimeOut = false;
  mContactorAlarmFlag = false;

  mLastEvent=0;                       //0=stop, 1=start
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:  Default
 *
 ****************************************************************************/
Pump::~Pump()
{
  for (int fault_id = FIRST_PUMP_FAULT_OBJ; fault_id < NO_OF_PUMP_FAULT_OBJ; fault_id++)
  {
    delete(mpPumpAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void Pump::RunSubTask()
{
  mRunRequestedFlag = false;

  // Alarm pre handling
  for (int fault_id = FIRST_PUMP_FAULT_OBJ; fault_id < NO_OF_PUMP_FAULT_OBJ; fault_id++)
  {
    if (mPumpAlarmDelayCheckFlag[fault_id] == true)
    {
      mPumpAlarmDelayCheckFlag[fault_id] = false;
      mpPumpAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  NormalRun();

  // Alarm post handling
  for (int fault_id = FIRST_PUMP_FAULT_OBJ; fault_id < NO_OF_PUMP_FAULT_OBJ; fault_id++)
  {
    mpPumpAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }

}


/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *
 *****************************************************************************/
void Pump::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_PUMP_OPERATION_MODE_REQ:
      mpOprModeReq.Attach(pSubject);
      break;
    case SP_PUMP_OPERATION_MODE_LEVEL_CTRL:
      mpOprModeLevelCtrl.Attach(pSubject);
      break;
    case SP_PUMP_ANTI_BLOCKING_REQUEST:
      mpAntiBlockingRequest.Attach(pSubject);
      break;
    case SP_PUMP_VFD_STATE:
      mpVfdState.Attach(pSubject);
      break;
    case SP_PUMP_OPERATION_MODE_ACTUAL:
      mpActualOperationMode.Attach(pSubject);
      break;

    case SP_PUMP_ENABLE:
      mpPumpEnable.Attach(pSubject);
      break;
    case SP_PUMP_READY_FOR_AUTO_OPR:
      mpPumpReadyForAutoOpr.Attach(pSubject);
      break;
    case SP_PUMP_PUMP_NO :
      mpPumpNo.Attach(pSubject);
      break;
    case SP_PUMP_NO_OF_PUMPS :
      mpNoOfPumps.Attach(pSubject);
      break;
    case SP_PUMP_START_RELAY:
      mpPumpStartRelay.Attach(pSubject);
      break;
    case SP_PUMP_REVERSE_RELAY:
      mpPumpReverseRelay.Attach(pSubject);
      break;
    case SP_PUMP_AVAILABLE:
      mpPumpAvailable.Attach(pSubject);
      break;

    case SP_PUMP_CONTACTOR_FEEDBACK:
      mpPumpContactorFeedback.Attach(pSubject);
      break;
    case SP_PUMP_CONTACTOR_FEEDBACK_ALARM_OBJ:
      mpContactorAlarmObj.Attach(pSubject);
      mpPumpAlarmDelay[PUMP_FAULT_OBJ_CONTACTOR_FEEDBACK]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_PUMP_REFERENCE_FOR_CONTACTOR_FEEDBACK_AVAILABLE:
      mpReferenceForContactorFeedbackAvailable.Attach(pSubject);
      break;
    case SP_PUMP_CTRL_SOURCE:
      mpCtrlSource.Attach(pSubject);
      break;
    case SP_PUMP_ALARM_FLAG:
      mpPumpAlarmFlag.Attach(pSubject);
      break;
    case SP_PUMP_START_START_DELAY_ACTIVE:
      mpStartStartDelayActive.Attach(pSubject);
      break;
    case SP_PUMP_START_STOP_START_DELAY_ACTIVE:
      mpStartStopStartDelayActive.Attach(pSubject);
      break;
    case SP_PUMP_STOP_STOP_DELAY_ACTIVE:
      mpStopStopDelayActive.Attach(pSubject);
      break;
    case SP_PUMP_START_START_DELAY_CNF:
      mpStartStartDelayCnf.Attach(pSubject);
      break;
    case SP_PUMP_START_STOP_START_DELAY_CNF:
      mpStartStopStartDelayCnf.Attach(pSubject);
      break;
    case SP_PUMP_STOP_STOP_DELAY_CNF:
      mpStopStopDelayCnf.Attach(pSubject);
      break;
    case SP_PUMP_UNDER_LOWEST_STOP_LEVEL:
      mpUnderLowestStopLevel.Attach(pSubject);
      break;
    case SP_PUMP_VFD_PUMP_START_REQUEST:
      mpVfdPumpStartRequest.Attach(pSubject);
      break;
    default:
      break;

  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void Pump::Update(Subject* pSubject)
{
  mpPumpEnable.Update(pSubject);
  mpNoOfPumps.Update(pSubject);
  mpStartStartDelayCnf.Update(pSubject);
  mpStopStopDelayCnf.Update(pSubject);
  mpStartStopStartDelayCnf.Update(pSubject);
  mpVfdState.Update(pSubject);

  if (pSubject == mpTimerObjList[START_START_TIMER])
  {
    mStartStartTimeOut = true;
  }
  if (pSubject == mpTimerObjList[START_STOP_START_TIMER])
  {
    mStartStopStartTimeOut = true;
  }
  if (pSubject == mpTimerObjList[STOP_STOP_TIMER])
  {
    mStopStopTimeOut = true;
  }
  if (pSubject == mpPumpAlarmDelay[PUMP_FAULT_OBJ_CONTACTOR_FEEDBACK])
  {
    mPumpAlarmDelayCheckFlag[PUMP_FAULT_OBJ_CONTACTOR_FEEDBACK] = true;
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
void Pump::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_PUMP_FAULT_OBJ; fault_id < NO_OF_PUMP_FAULT_OBJ; fault_id++)
  {
    mpPumpAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void Pump::ConnectToSubjects()
{
  mpOprModeReq->Subscribe(this);
  mpOprModeLevelCtrl->Subscribe(this);
  mpAntiBlockingRequest->Subscribe(this);
  mpVfdState->Subscribe(this);
  mpPumpEnable->Subscribe(this);
  mpNoOfPumps->Subscribe(this);
  mpPumpContactorFeedback->Subscribe(this);
  mpReferenceForContactorFeedbackAvailable->Subscribe(this);
  mpStartStartDelayActive->Subscribe(this);
  mpStartStopStartDelayActive->Subscribe(this);
  mpStopStopDelayActive->Subscribe(this);
  mpStartStartDelayCnf->Subscribe(this);
  mpStopStopDelayCnf->Subscribe(this);
  mpStartStopStartDelayCnf->Subscribe(this);
  mpUnderLowestStopLevel->Subscribe(this);
  mpCtrlSource->Subscribe(this);
  mpPumpAlarmFlag->Subscribe(this);

  for (int fault_id = FIRST_PUMP_FAULT_OBJ; fault_id < NO_OF_PUMP_FAULT_OBJ; fault_id++)
  {
    mpPumpAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void Pump::InitSubTask()
{
  mPumpEnabled = false;
  mPumpInstalled = false;
  mPumpOperationModeAct = ACTUAL_OPERATION_MODE_NOT_INSTALLED;
  mPumpOperationModeReq = ACTUAL_OPERATION_MODE_NOT_INSTALLED;

  mpTimerObjList[CONTACTOR_ALARM_RESET_TIMER] = new SwTimer(100, MS, false, false, this);
  mpTimerObjList[CONTACTOR_IGNORE_TIMER] = new SwTimer(10, S, false, false, this);
  mpTimerObjList[START_START_TIMER] = new SwTimer(2, S, false, false, this);
  mpTimerObjList[STOP_STOP_TIMER] = new SwTimer(2, S, false, false, this);
  mpTimerObjList[START_STOP_START_TIMER] = new SwTimer(5, S, false, false, this);

  mpActualOperationMode->SetValue(mPumpOperationModeAct);
  mpVfdPumpStartRequest->SetValue(false);
  mpPumpStartRelay->SetValue(false);
  mpPumpReverseRelay->SetValue(false);
  mpPumpReadyForAutoOpr->SetValue(false);
  mpPumpAvailable->SetValue(false);

  // Assure that the pump is installed at start up (if it is present in the system)
  mpNoOfPumps.SetUpdated();
  mpPumpEnable.SetUpdated();

  /* Ensure using configuration */
  mpStartStartDelayCnf.SetUpdated();
  mpStopStopDelayCnf.SetUpdated();
  mpStartStopStartDelayCnf.SetUpdated();

  for (int fault_id = FIRST_PUMP_FAULT_OBJ; fault_id < NO_OF_PUMP_FAULT_OBJ; fault_id++)
  {
    mpPumpAlarmDelay[fault_id]->InitAlarmDelay();
  }

  mRunRequestedFlag = true;
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
 * Function - NormalRun
 * DESCRIPTION:
 *
 *****************************************************************************/
void Pump::NormalRun(void)
{
  //Check for new configuration
  if ( mpStartStartDelayCnf.IsUpdated() )
  {
    mpTimerObjList[START_START_TIMER]->SetSwTimerPeriod(mpStartStartDelayCnf->GetValue(), S, false);
  }
  if ( mpStopStopDelayCnf.IsUpdated() )
  {
    mpTimerObjList[STOP_STOP_TIMER]->SetSwTimerPeriod(mpStopStopDelayCnf->GetValue(), S, false);
  }
  if ( mpStartStopStartDelayCnf.IsUpdated() )
  {
    mpTimerObjList[START_STOP_START_TIMER]->SetSwTimerPeriod(mpStartStopStartDelayCnf->GetValue(), S, false);
  }

  // Check if pump is available for use - that is that it is installed and enabled
  if (mpNoOfPumps.IsUpdated())
  {
    if (mpPumpNo->GetValue() > mpNoOfPumps->GetValue())
    {
      mPumpOperationModeReq = ACTUAL_OPERATION_MODE_NOT_INSTALLED;
      mPumpOperationModeAct = ACTUAL_OPERATION_MODE_NOT_INSTALLED;
      mpActualOperationMode->SetValue(mPumpOperationModeAct);
      mpPumpReadyForAutoOpr->SetValue(false);
      SetContactorAlarm(false);
      mpPumpEnable->SetValue(false);
      mPumpInstalled = false;
      mpPumpStartRelay->SetValue(false);
      mpPumpReverseRelay->SetValue(false);
      mpVfdPumpStartRequest->SetValue(false);
    }
    else
    {
      mPumpInstalled = true;
      mPumpOperationModeAct = ACTUAL_OPERATION_MODE_STOPPED;
      mpPumpEnable.SetUpdated();
    }
  }
  if (mpPumpEnable.IsUpdated())
  {
    if( mPumpInstalled )
    {
      if (mpPumpEnable->GetValue() == true)
      {
        mPumpEnabled = true;
        mPumpOperationModeAct = ACTUAL_OPERATION_MODE_STOPPED;
      }
      else
      {
        if(mPumpOperationModeAct==ACTUAL_OPERATION_MODE_STARTED)
        {
          mpStartStopStartDelayActive->SetValue(true);
          mpStopStopDelayActive->SetValue(true);
          mpTimerObjList[STOP_STOP_TIMER]->RetriggerSwTimer();
          mpTimerObjList[START_STOP_START_TIMER]->RetriggerSwTimer();
          mLastEvent=0;  //Stop
        }
        mPumpOperationModeReq = ACTUAL_OPERATION_MODE_DISABLED;
        mPumpOperationModeAct = ACTUAL_OPERATION_MODE_DISABLED;
        mpActualOperationMode->SetValue(mPumpOperationModeAct);
        mpPumpReadyForAutoOpr->SetValue(false);
        SetContactorAlarm(false);
        mPumpEnabled = false;
        mpPumpStartRelay->SetValue(false);
        mpPumpReverseRelay->SetValue(false);
        mpVfdPumpStartRequest->SetValue(false);
      }
    }
  }

  //Check for delay timeout
  if(mStartStartTimeOut == true)
  {
    mStartStartTimeOut = false;
    mpStartStartDelayActive->SetValue(false);
  }
  if(mStartStopStartTimeOut == true)
  {
    mStartStopStartTimeOut = false;
    mpStartStopStartDelayActive->SetValue(false);
  }
  if(mStopStopTimeOut == true)
  {
    mStopStopTimeOut = false;
    mpStopStopDelayActive->SetValue(false);
  }

  if (mPumpInstalled == false || mPumpEnabled == false)
  {
    mpPumpAvailable->SetValue(false);
  }
  else // Normal start/stop operation
  {
    mpPumpAvailable->SetValue(true);
    mIgnoreDelays = false;
    switch (mpOprModeReq->GetValue())
    {
      case REQ_OPERATION_MODE_AUTO:
        // The manual on-off control is in AUTO - the level ctrl decides how pump is used
        if (mpOprModeLevelCtrl->GetValue() == PUMP_OPERATION_MODE_PUMP_ON)
        {
          mPumpOperationModeReq = ACTUAL_OPERATION_MODE_STARTED;
        }
        else
        {
          mPumpOperationModeReq = ACTUAL_OPERATION_MODE_STOPPED;
        }
        mpPumpReadyForAutoOpr->SetValue(mpPumpAlarmFlag->GetValue() == false); // Pump is ready for auto opr when no alarm
        break;
      case REQ_OPERATION_MODE_ON:
        // Pump is controlled manually
        mPumpOperationModeReq = ACTUAL_OPERATION_MODE_STARTED;
        mpPumpReadyForAutoOpr->SetValue(false);
        if(mpCtrlSource->GetValue()==CONTROL_SOURCE_DI)
        {
          mIgnoreDelays = true;
        }
        else if(mpCtrlSource->GetValue()==CONTROL_SOURCE_SCADA)
        {
          if(mpUnderLowestStopLevel->GetValue()==true) // Stop pump if SCADA controlled and level is under lowest stop level
          {
            mPumpOperationModeReq = ACTUAL_OPERATION_MODE_STOPPED;
          }
        }
        break;
      case REQ_OPERATION_MODE_OFF:
        // Pump is controlled manually
        mPumpOperationModeReq = ACTUAL_OPERATION_MODE_STOPPED;
        mpPumpReadyForAutoOpr->SetValue(false);
        if(mpCtrlSource->GetValue()==CONTROL_SOURCE_DI)
        {
          mIgnoreDelays = true;
        }
        break;
      default:
        break;
    }

    // Check start/stop delays and set actual start/stop request
    if(mPumpOperationModeReq==ACTUAL_OPERATION_MODE_STARTED && mPumpOperationModeAct==ACTUAL_OPERATION_MODE_STOPPED) //Start request
    {
      if( (mLastEvent==0 && mpStartStopStartDelayActive->GetValue()==false) || (mLastEvent==1 && mpStartStartDelayActive->GetValue()==false) || mIgnoreDelays)
      {
        mPumpOperationModeAct = ACTUAL_OPERATION_MODE_STARTED;
        mpStartStopStartDelayActive->SetValue(true);
        mpStartStartDelayActive->SetValue(true);
        mpTimerObjList[START_START_TIMER]->RetriggerSwTimer();
        mpTimerObjList[START_STOP_START_TIMER]->RetriggerSwTimer();
        // Set the contactor ignore equal to the contactor alarm delay, done here just to avoid doing it all the time
        mpTimerObjList[CONTACTOR_IGNORE_TIMER]->SetSwTimerPeriod(mpContactorAlarmObj->GetAlarmConfig()->GetAlarmDelay(), S, false);
        mLastEvent=1;  //Start
      }
    }
    else if(mPumpOperationModeReq==ACTUAL_OPERATION_MODE_STOPPED && mPumpOperationModeAct==ACTUAL_OPERATION_MODE_STARTED) //Stop request
    {
      if( (mLastEvent==1 && mpStartStopStartDelayActive->GetValue()==false) || (mLastEvent==0 && mpStopStopDelayActive->GetValue()==false)|| mIgnoreDelays )
      {
        mPumpOperationModeAct = ACTUAL_OPERATION_MODE_STOPPED;
        mpStartStopStartDelayActive->SetValue(true);
        mpStopStopDelayActive->SetValue(true);
        mpTimerObjList[STOP_STOP_TIMER]->RetriggerSwTimer();
        mpTimerObjList[START_STOP_START_TIMER]->RetriggerSwTimer();
        mLastEvent=0;  //Stop
      }
    }

    // Update relays
    bool contactor_feedback_possible = false;
    bool vfd_installed = (mpVfdState->GetQuality() == DP_AVAILABLE);
    if (vfd_installed == false)
    {
      if (mPumpOperationModeAct == ACTUAL_OPERATION_MODE_STARTED)
      {
        // Handle pump start or anti blocking request without Vfd
        ANTI_BLOCKING_REQUEST_TYPE anti_blocking_request = mpAntiBlockingRequest->GetValue();
        switch (anti_blocking_request)
        {
          default:
          case ANTI_BLOCKING_REQUEST_IDLE:
          case ANTI_BLOCKING_REQUEST_FORWARD_FLUSH:
            // Normal pump start situation
            contactor_feedback_possible = true;
            mpPumpReverseRelay->SetValue(false);
            if (mpPumpStartRelay->GetValue() == false)
            {
              mpPumpStartRelay->SetValue(true);
              mpTimerObjList[CONTACTOR_IGNORE_TIMER]->RetriggerSwTimer();
            }
            break;
          case ANTI_BLOCKING_REQUEST_REVERSE_FLUSH:
            mpPumpStartRelay->SetValue(false);
            mpPumpReverseRelay->SetValue(true);
            break;
          case ANTI_BLOCKING_REQUEST_AWAIT_REVERSE:
          case ANTI_BLOCKING_REQUEST_AWAIT_FORWARD:
            mpPumpStartRelay->SetValue(false);
            mpPumpReverseRelay->SetValue(false);
            break;
        }
      }
      else // Stop request without vfd
      {
        contactor_feedback_possible = true;
        mpPumpReverseRelay->SetValue(false);
        if (mpPumpStartRelay->GetValue() == true)
        {
          mpPumpStartRelay->SetValue(false);
          mpTimerObjList[CONTACTOR_IGNORE_TIMER]->RetriggerSwTimer();
        }
      }
      mpVfdPumpStartRequest->SetQuality(DP_NOT_AVAILABLE);
    }
    else // Vfd handling:
    {
      // Send start/stop request to Vfd control and handle relays
      VFD_OPERATION_MODE_TYPE vfd_state = mpVfdState->GetValue();
      if (mPumpOperationModeAct == ACTUAL_OPERATION_MODE_STARTED)
      {
        mpVfdPumpStartRequest->SetValue(true);
        // NOTE: Both start and reverse relay are set for vfd reversing
        mpPumpReverseRelay->SetValue(vfd_state == VFD_OPERATION_MODE_REV_FLUSH);
        // Await vfd start-feedback
        if (vfd_state != VFD_OPERATION_MODE_STOP)
        {
          if (mpPumpStartRelay->GetValue() == false)
          {
            // Start event
            mpPumpStartRelay->SetValue(true);
            mpTimerObjList[CONTACTOR_IGNORE_TIMER]->RetriggerSwTimer(); // To ensure correct handling in manual
          }
          switch (vfd_state)
          {
            case VFD_OPERATION_MODE_NOT_IN_CONTROL:   // Manual operation
            case VFD_OPERATION_MODE_START_FLUSH:      // Run
            case VFD_OPERATION_MODE_NORMAL:           // Run
            case VFD_OPERATION_MODE_RUN_FLUSH:        // Run
            // !!VFD_OPERATION_MODE_RUN_FLUSH:        // Do not check contactor during stop flush
              contactor_feedback_possible = true;
              if (mpVfdState.IsUpdated()) // Use this event to start the timer just once
              {
                mpTimerObjList[CONTACTOR_IGNORE_TIMER]->RetriggerSwTimer();
              }
            break;
          }
        }
      }
      else // Stop request with vfd
      {
        mpVfdPumpStartRequest->SetValue(false);
        mpPumpReverseRelay->SetValue(false);
        switch (vfd_state)
        {
          // Await vfd stop-feedback since the vfd pump may have to run a stop flush
          case VFD_OPERATION_MODE_NOT_IN_CONTROL:
          case VFD_OPERATION_MODE_INIT:
          case VFD_OPERATION_MODE_STOP:
            contactor_feedback_possible = true;
            if (mpPumpStartRelay->GetValue() == true)
            {
              mpPumpStartRelay->SetValue(false);
              mpTimerObjList[CONTACTOR_IGNORE_TIMER]->RetriggerSwTimer();
            }
            break;
          default:
            break;
        }
      }
    }

    // Read contactor feedback
    ACTUAL_OPERATION_MODE_TYPE feedback_operation_mode = ACTUAL_OPERATION_MODE_NOT_INSTALLED;
    switch (mpPumpContactorFeedback->GetValue())
    {
      case DIGITAL_INPUT_FUNC_STATE_ACTIVE :
        feedback_operation_mode = ACTUAL_OPERATION_MODE_STARTED;
        break;
      case DIGITAL_INPUT_FUNC_STATE_NOT_ACTIVE :
        feedback_operation_mode = ACTUAL_OPERATION_MODE_STOPPED;
        break;
      default:
        contactor_feedback_possible = false;
        break;
    }

    // Update actual operation mode
    if (contactor_feedback_possible == false || mpTimerObjList[CONTACTOR_IGNORE_TIMER]->GetSwTimerStatus() == true)
    {
      // No contactor or operation changed recently - do not use contactor feedback for setting the actual operation mode
      if (mpPumpStartRelay->GetValue() == true)
      {
        // Special handling since a vfd pump may run some seconds after a stop request (stop flush)
        mpActualOperationMode->SetValue(ACTUAL_OPERATION_MODE_STARTED);
      }
      else
      {
        mpActualOperationMode->SetValue(mPumpOperationModeAct);
      }
    }
    else // Use contactor feedback as operation mode
    {
      mpActualOperationMode->SetValue(feedback_operation_mode);
    }

    // Update contactor alarm
    if (contactor_feedback_possible == false)
    {
      SetContactorAlarm(false);
    }
    else if (mpReferenceForContactorFeedbackAvailable->GetValue() == true)
    {
      // Contactor feedback is reliable - it is allowed to report an alarm if feedback is not as expected
      SetContactorAlarm(feedback_operation_mode != mPumpOperationModeAct);
    }
  }
}


/*****************************************************************************
 * Function - SetContactorAlarm
 * DESCRIPTION: Set/reset contactor alarm with a small delay for reset.
 *              This is to ensure starting another pump in case of alarm,
 *              because a contactor alarm stop makes the pump ready again.
 *
 *****************************************************************************/
void Pump::SetContactorAlarm(bool alarm_present)
{
  if (alarm_present)
  {
    if (mContactorAlarmFlag == false)
    {
      mpPumpAlarmDelay[PUMP_FAULT_OBJ_CONTACTOR_FEEDBACK]->SetFault();
      mContactorAlarmFlag = true;
    }
  }
  else
  {
    if (mContactorAlarmFlag == true)
    {
      mContactorAlarmFlag = false;
      mpTimerObjList[CONTACTOR_ALARM_RESET_TIMER]->RetriggerSwTimer();
    }
    else if (mpTimerObjList[CONTACTOR_ALARM_RESET_TIMER]->GetSwTimerStatus() == false)
    {
      // Reset the alarm now that a small delay has elapsed
      mpPumpAlarmDelay[PUMP_FAULT_OBJ_CONTACTOR_FEEDBACK]->ResetFault();
    }
  }
}
