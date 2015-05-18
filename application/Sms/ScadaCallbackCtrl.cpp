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
/* CLASS NAME       : ScadaCallbackCtrl                                     */
/*                                                                          */
/* FILE NAME        : ScadaCallbackCtrl.cpp                                 */
/*                                                                          */
/* CREATED DATE     : 30-05-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <SwTimer.h>
#include <ActTime.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <ScadaCallbackCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  ALARM_RESTART_TIMER,
  CALLBACK_TIMEOUT_TIMER
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
ScadaCallbackCtrl::ScadaCallbackCtrl()
{
  mpTimerObjList[ALARM_RESTART_TIMER] = new SwTimer(100, MS, false, false, this);
  mpTimerObjList[CALLBACK_TIMEOUT_TIMER] = new SwTimer(1000, S, false, false, this);

  mRunRequestedFlag         = false;
  mScadaCallbackInProgress  = false;
  mCallbackSessionWaiting   = false;
  mCheckAlarmRestart        = false;
  mCallbackTimeout          = false;
  mConnectionLostCount      = 0;
  mLastPhoneCallState       = PHONE_CALL_STATE_IDLE;
  mLastCallBackAck          = true;
  mCallbackEnabledSuppressFirstUpdated = true;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ScadaCallbackCtrl::~ScadaCallbackCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ScadaCallbackCtrl::InitSubTask()
{  
  mpScadaCallbackEnabled.SetUpdated();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ScadaCallbackCtrl::RunSubTask()
{
  bool callback_wanted = false;
  bool callback_finish = false;
  bool callback_enabled;

  mRunRequestedFlag = false;
  callback_enabled = mpScadaCallbackEnabled->GetValue();

  // Master callback setting has changed. Force a start or stop of callback
  if (mpScadaCallbackEnabled.IsUpdated())
  {
    if (callback_enabled == true)
    {
      callback_wanted = true;
    }
    else
    {
      HandleCallbackAlarmRequest(false);
      mCallbackSessionWaiting = false;
      callback_finish = true;
    }
  }

  // Normal start of callback situation
  if (mpAlarmToScadaArrived.IsUpdated())
  {
    if (callback_enabled == true)
    {
      callback_wanted = true;
    }
  }

  // Check callback timeout
  if (mCallbackTimeout == true)
  {
    // If the scada callback max time has elapsed, abort the callback
    mCallbackTimeout = false;
    //callback_finish = true; //to keep callback attempt alive forever (CQ15863), V1-B08233
  }

  // Check feedback from GSM module
  PHONE_CALL_STATE_TYPE phone_call_state = (PHONE_CALL_STATE_TYPE)(mpGsmState->GetValue() & 0x07); // Bit 0-2
  if (mpGsmState.IsUpdated())
  {
    switch (phone_call_state)
    {
      case PHONE_CALL_STATE_IDLE:
        if (mLastPhoneCallState != PHONE_CALL_STATE_IDLE)
        {
          if (mCallbackSessionWaiting == true)
          {
            callback_wanted = true;
          }
        }
        break;

      case PHONE_CALL_STATE_CB_CALLUP:
      case PHONE_CALL_STATE_CB_AWAIT_RETRY:
        if (mLastPhoneCallState == PHONE_CALL_STATE_CB_CONNECTION)
        {
          mConnectionLostCount++;
        }
        break;

      case PHONE_CALL_STATE_CB_CONNECTION:
        if (mScadaCallbackInProgress == true && mLastPhoneCallState != PHONE_CALL_STATE_CB_CONNECTION)
        {
          mCallbackSessionWaiting = false;  // No need to trig another session since data are not collected yet
          if (mConnectionLostCount == 0)
          {
            // The call back connection is now established the first time in session, restart the callback timeout
            mpTimerObjList[CALLBACK_TIMEOUT_TIMER]->RetriggerSwTimer();
            // Restart the alarm delay (but only if the alarm is not already present)
            if (mpScadaCallbackAlarmObj->GetErrorPresent() == ALARM_STATE_READY)
            {
              HandleCallbackAlarmRequest(false); // Will restart the alarm delay again
            }
          }
        }
        break;

      case PHONE_CALL_STATE_CU_CONNECTION:
        // A call up (another session) has been established. Wait until the connection is idle again.
        // In case a call back was already started, it must be set waiting.
        if (mScadaCallbackInProgress == true)
        {
          callback_finish = true;
          mCallbackSessionWaiting = true;
        }
        break;

      default:
        // Nothing
        break;
    }
    mLastPhoneCallState = phone_call_state;

    // Check call back acknowledge (gsm_state bit 6 changing to 1)
    bool call_back_ack = ((mpGsmState->GetValue()&0x40) == 0x40);
    if (mScadaCallbackInProgress == true && call_back_ack == true && mLastCallBackAck == false)
    {
      HandleCallbackAlarmRequest(false);
      mpTimeStampLastScadaAck->SetValue(ActTime::GetInstance()->GetSecondsSince1Jan1970());
      callback_finish = true;
    }
    mLastCallBackAck = call_back_ack;
  }


  // Special event from geni (used for GPRS callbacks) to force an update of the acknowledge time stamp
  if (mpScadaAckEvent.IsUpdated())
  {
    HandleCallbackAlarmRequest(false);
    mpTimeStampLastScadaAck->SetValue(ActTime::GetInstance()->GetSecondsSince1Jan1970());
    callback_finish = true;
  }

  // Perform stop of the callback
  if (callback_finish == true)
  {
    mpTimerObjList[CALLBACK_TIMEOUT_TIMER]->StopSwTimer();
    mScadaCallbackInProgress = false;    
    if ( (mpScadaCallbackFault->GetValue() == true)
      && (mpScadaCallbackAlarmObj->GetErrorPresent() != ALARM_STATE_READY)
      && (mpScadaCallbackAlarmObj->GetAlarmConfig()->GetScadaEnabled() == true) )
    {
      // If the scada callback alarm itself is configured to send a scada alarm, then restart the callback.
      // This leads to continous scada callback as long as a callback is not acknowledged.
      callback_wanted = true;
    }
    if (mCallbackSessionWaiting == true)
    {
      callback_wanted = true;
      mCallbackSessionWaiting = false;
    }
  }

  // Perform start of the callback
  if (callback_wanted == true)
  {
    if ( (phone_call_state != PHONE_CALL_STATE_CU_CONNECTION)
      && (mScadaCallbackInProgress == false) )
    {
      mCallbackSessionWaiting = false;
      mpTimerObjList[CALLBACK_TIMEOUT_TIMER]->SetSwTimerPeriod(mpScadaCallbackAlarmObj->GetAlarmConfig()->GetAlarmDelay(), S, false);
      mpTimerObjList[CALLBACK_TIMEOUT_TIMER]->RetriggerSwTimer();
      mScadaCallbackInProgress = true;
      mConnectionLostCount = 0;
    }
    else
    {
      // Another session is running. Set the waiting flag to start a session later.
      mCallbackSessionWaiting = true;
    }
    HandleCallbackAlarmRequest(true); // Request the alarm here, asuming that a reasonable alarm delay is set
  }

  // Check delayed set of the alarm (used to ensure alarm set just after an alarm reset)
  if (mCheckAlarmRestart == true)
  {
    mCheckAlarmRestart = false;
    if (mScadaCallbackInProgress == true)
    {
      HandleCallbackAlarmRequest(true);
    }
  }

  if (mpServiceModeEnabled->GetValue())
  {
    HandleCallbackAlarmRequest(false);
  }

  // Inform the ModemCtrl class
  mpScadaCallbackRequest->SetValue(mScadaCallbackInProgress);
  mpScadaCallbackPending->SetValue(mCallbackSessionWaiting);
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void ScadaCallbackCtrl::ConnectToSubjects()
{
  mpScadaCallbackEnabled->Subscribe(this);
  mpGsmState->Subscribe(this);
  mpAlarmToScadaArrived->Subscribe(this);
  mpScadaAckEvent->Subscribe(this);
  mpServiceModeEnabled->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void ScadaCallbackCtrl::Update(Subject* pSubject)
{
  mpGsmState.Update(pSubject);
  mpAlarmToScadaArrived.Update(pSubject);
  mpScadaAckEvent.Update(pSubject);
  mpServiceModeEnabled.Update(pSubject);
  mpScadaCallbackEnabled.Update(pSubject);    
  if (mCallbackEnabledSuppressFirstUpdated)       //at power on updated from config and makes callback, hence suppress it
  {
    if (mpScadaCallbackEnabled.IsUpdated())
    {
      mCallbackEnabledSuppressFirstUpdated = false;
    }
  }

  if (pSubject == mpTimerObjList[ALARM_RESTART_TIMER])
  {
    mCheckAlarmRestart = true;
  }
  if (pSubject == mpTimerObjList[CALLBACK_TIMEOUT_TIMER])
  {
    mCallbackTimeout = true;
  }

  if (mRunRequestedFlag == false)
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
void ScadaCallbackCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpScadaCallbackEnabled.Detach(pSubject);
  mpGsmState.Detach(pSubject);
  mpAlarmToScadaArrived.Detach(pSubject);
  mpScadaAckEvent.Detach(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void ScadaCallbackCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Input:
    case SP_SCB_SCADA_CALLBACK_ENABLED:
      mpScadaCallbackEnabled.Attach(pSubject);
      break;
    case SP_SCB_ALARM_TO_SCADA_ARRIVED:
      mpAlarmToScadaArrived.Attach(pSubject);
      break;
    case SP_SCB_GSM_STATE:
      mpGsmState.Attach(pSubject);
      break;
    case SP_SCB_SCADA_ACK_EVENT:
      mpScadaAckEvent.Attach(pSubject);
      break;
    case SP_SCB_SERVICE_MODE_ENABLED:
      mpServiceModeEnabled.Attach(pSubject);
      break;

    // Output:
    case SP_SCB_SCADA_CALLBACK_REQUEST:
      mpScadaCallbackRequest.Attach(pSubject);
      break;
    case SP_SCB_SCADA_CALLBACK_PENDING:
      mpScadaCallbackPending.Attach(pSubject);
      break;
    case SP_SCB_TIMESTAMP_LAST_SCADA_ACK :
      mpTimeStampLastScadaAck.Attach(pSubject);
      break;


    // Alarm
    case SP_SCB_SCADA_CALLBACK_FAULT:
      mpScadaCallbackFault.Attach(pSubject);
      break;
    case SP_SCB_SCADA_CALLBACK_ALARM_OBJ:
      mpScadaCallbackAlarmObj.Attach(pSubject);
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
 * Function - HandleCallbackAlarmRequest
 * DESCRIPTION: Set or reset the callback alarm using a delay from reset to
 *              set alarm to ensure that the alarm handling has processed
 *              the last reset
 *
 *****************************************************************************/
void ScadaCallbackCtrl::HandleCallbackAlarmRequest(bool alarm_request)
{
  //no need to raise callback alarm in service mode
  if ((alarm_request == true) && (!mpServiceModeEnabled->GetValue()))
  {
    if (mpTimerObjList[ALARM_RESTART_TIMER]->GetSwTimerStatus() == false)
    {
      // The delay since alarm reset has elapsed. OK to set the fault now.
      mpScadaCallbackFault->SetValue(true);
    }
  }
  else
  {
    mpScadaCallbackFault->SetValue(false);
    mCheckAlarmRestart = false;
    mpTimerObjList[ALARM_RESTART_TIMER]->RetriggerSwTimer();
  }
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
