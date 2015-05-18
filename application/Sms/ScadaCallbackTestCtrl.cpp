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
/* CLASS NAME       : ScadaCallbackTestCtrl                                 */
/*                                                                          */
/* FILE NAME        : ScadaCallbackTestCtrl.cpp                             */
/*                                                                          */
/* CREATED DATE     : 12-07-2012 dd-mm-yyyy                                 */
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
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <ScadaCallbackTestCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
// SW Timers
enum
{
  ALARM_STABILIZATION_TIMER,
  CALLBACK_ACK_WAITING_TIMER
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
ScadaCallbackTestCtrl::ScadaCallbackTestCtrl()
{
  mpCallbaclTestAlarmDelay = new AlarmDelay(this);
  mCallbackTestAlarmDelayCheckFlag = false;
  mpTimerObjList[ALARM_STABILIZATION_TIMER] = new SwTimer(2, S, false, false, this);
  mpTimerObjList[CALLBACK_ACK_WAITING_TIMER] = new SwTimer(6, MINUTE, false, false, this);  
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ScadaCallbackTestCtrl::~ScadaCallbackTestCtrl()
{
  delete (mpCallbaclTestAlarmDelay); 
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::InitSubTask()
{
  mpCallbaclTestAlarmDelay->InitAlarmDelay();   

  InitSettings();
  mRunRequestedFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if (mpCallbackTestEvent.IsUpdated())
  {
    InitSettings();
    mpCallbackTestEnabled->SetValue(true);
  }

  if (mpCallbackTestEnabled->GetValue())
  { 
    if ((mCallbackTestState == CALLBACK_TEST_STATE_NOT_STARTED) ||
        (mCallbackTestState == CALLBACK_TEST_STATE_WAITING_ACK)) 
    {
      mCallbackTestState = CALLBACK_TEST_STATE_WAITING_ACK;
      RaiseCallbackTestAlarm(true);

      if (!mStabilizationTimerStarted)                                      //wait until Callback test alarm get raised to monitor the status
      {
        mStabilizationTimerStarted = true;        
        mAlarmStabilized = false;
        mpTimerObjList[ALARM_STABILIZATION_TIMER]->RetriggerSwTimer();
        mpTimeStampLastScadaAck.ResetUpdated();
      }
    }  
  }
  else
  {
    InitSettings();           //Re-initialize all the settings
  } 
 
  //if Callback test alarm raised, wait for ACK
  CheckIsAlarmStabilized();

  CheckCallbackStatus();
 
  UpdateStatusDisplay();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::ConnectToSubjects()
{
  mpCallbackTestEnabled->Subscribe(this);
  mpTimeStampLastScadaAck->Subscribe(this);  
  mpScadaCallBackEnabled->Subscribe(this);
  mpCallbaclTestAlarmDelay->ConnectToSubjects();
  mpTimerObjList[ALARM_STABILIZATION_TIMER]->Subscribe(this);
  mpTimerObjList[CALLBACK_ACK_WAITING_TIMER]->Subscribe(this); 
  mpCallbackTestEvent->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::Update(Subject* pSubject)
{
  mpCallbackTestEnabled.Update(pSubject);
  mpTimeStampLastScadaAck.Update(pSubject); 
  mpScadaCallBackEnabled.Update(pSubject);
  mpCallbackTestEvent.Update(pSubject);
 
  if (pSubject == mpCallbaclTestAlarmDelay)
  {
    mCallbackTestAlarmDelayCheckFlag = true;
  }
  if (pSubject == mpTimerObjList[ALARM_STABILIZATION_TIMER])
  {
    mAlarmStabilizationTimerRestart = true;
  }
  if (pSubject == mpTimerObjList[CALLBACK_ACK_WAITING_TIMER])
  {
    mCallbackAckWaitingTimerRestart = true;
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
void ScadaCallbackTestCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpCallbaclTestAlarmDelay->SubscribtionCancelled(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    //inputs
     case SP_SCBTC_SCADA_CALLBACK_ENABLED:
      mpScadaCallBackEnabled.Attach(pSubject);
      break;
    case SP_SCBTC_SCADA_CALLBACK_TEST_ENABLED:
      mpCallbackTestEnabled.Attach(pSubject);
      break;
    case SP_SCBTC_CIU_CARD_FAULT:
      mpCiuCardFault.Attach(pSubject);
      break;
    case SP_SCBTC_TIMESTAMP_LAST_SCADA_ACK:
      mpTimeStampLastScadaAck.Attach(pSubject);
      break;
    case SP_SCBTC_SCADA_CALLBACK_TEST_EVENT:
      mpCallbackTestEvent.Attach(pSubject);
      break;

    //outputs
    case SP_SCBTC_SCADA_CALLBACK_TEST_REQ_STATE:
      mpCallbackTestReqString.Attach(pSubject);
      break;
    case SP_SCBTC_SCADA_CALLBACK_TEST_ACK_STATE:
      mpCallbackTestAckString.Attach(pSubject);
      break;
    case SP_SCBTC_SCADA_CALLBACK_TEST_ALARM_OBJ:
      mpCallbaclTestAlarmDelay->SetSubjectPointer(id, pSubject);
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
 * Function - InitSettings
 * DESCRIPTION: 
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::InitSettings()
{
  //initialize all the settings
  mCallbackTestState = CALLBACK_TEST_STATE_NOT_STARTED;  
  RaiseCallbackTestAlarm(false);
  
  mAlarmStabilized = false;
  mStabilizationTimerStarted = false;
  mAlarmStabilizationTimerRestart = false;
  mpTimerObjList[ALARM_STABILIZATION_TIMER]->StopSwTimer();

  mCallbackAckWaitingTimerRestart = false;
  mpTimerObjList[CALLBACK_ACK_WAITING_TIMER]->StopSwTimer();
}

/*****************************************************************************
 * Function - RaiseCallbackTestAlarm
 * DESCRIPTION:
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::RaiseCallbackTestAlarm(bool alarmRequest)
{
  if (mCallbackTestAlarmDelayCheckFlag == true)
  {
    mCallbackTestAlarmDelayCheckFlag = false;
    mpCallbaclTestAlarmDelay->CheckErrorTimers();
  }

  if (alarmRequest)
  {
    mpCallbaclTestAlarmDelay->SetFault();       //raise Callback test alarm
  }
  else
  {
    mpCallbaclTestAlarmDelay->ResetFault();    //clear Callback test alarm
  }

  mpCallbaclTestAlarmDelay->UpdateAlarmDataPoint();

}

/*****************************************************************************
 * Function - CheckIsAlarmStabilized
 * DESCRIPTION: 
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::CheckIsAlarmStabilized()
{  
  if (mAlarmStabilizationTimerRestart)
  {
    //Callback Test alarm raised properly
    mAlarmStabilizationTimerRestart = false;
    if (mpTimerObjList[ALARM_STABILIZATION_TIMER]->GetSwTimerStatus() == false)
    {      
      mpTimerObjList[ALARM_STABILIZATION_TIMER]->StopSwTimer();
      mAlarmStabilized = true;
      //Timeout for ACK
      mpTimerObjList[CALLBACK_ACK_WAITING_TIMER]->RetriggerSwTimer(); 
    }    
  }
}

/*****************************************************************************
 * Function - CheckCallbackStatus
 * DESCRIPTION: 
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::CheckCallbackStatus()
{
  //main Scada callback is disabled or CIM/CIU module problem
  if ((mCallbackTestState == CALLBACK_TEST_STATE_WAITING_ACK) && 
      ((mpScadaCallBackEnabled->GetValue() == false) || (mpCiuCardFault->GetValue() == true)))
  {
    mCallbackTestState = CALLBACK_TEST_STATE_SENT_FAILED;
    RaiseCallbackTestAlarm(false); 
  }

  if ((mCallbackTestState == CALLBACK_TEST_STATE_WAITING_ACK) && (mAlarmStabilized == true))
  {
    //ACK received
    if (mpTimeStampLastScadaAck.IsUpdated())
    {
      mpTimerObjList[CALLBACK_ACK_WAITING_TIMER]->StopSwTimer();
      mCallbackAckWaitingTimerRestart = false;
      mCallbackTestState = CALLBACK_TEST_STATE_SUCCESS;
      RaiseCallbackTestAlarm(false); 
    }
    //Timeout occured
    else if (mpTimerObjList[CALLBACK_ACK_WAITING_TIMER]->GetSwTimerStatus() == false)
    {
      mpTimerObjList[CALLBACK_ACK_WAITING_TIMER]->StopSwTimer();
      mCallbackAckWaitingTimerRestart = false;
      mCallbackTestState = CALLBACK_TEST_STATE_ACK_FAILED;
      RaiseCallbackTestAlarm(false); 
    }
  }
}

/*****************************************************************************
 * Function - UpdateStatusDisplay
 * DESCRIPTION: 
 *
 *****************************************************************************/
void ScadaCallbackTestCtrl::UpdateStatusDisplay()
{
  switch (mCallbackTestState)
  {
  case CALLBACK_TEST_STATE_WAITING_ACK:
 
    mpCallbackTestReqString->SetValue(Languages::GetInstance()->GetString(SID_SCADA_CALLBACK_TEST_SENT));
    mpCallbackTestAckString->SetValue(Languages::GetInstance()->GetString(SID_SCADA_CALLBACK_TEST_WAITING));
    break;

  case CALLBACK_TEST_STATE_SUCCESS:
    mpCallbackTestReqString->SetValue(Languages::GetInstance()->GetString(SID_SCADA_CALLBACK_TEST_SENT));
    mpCallbackTestAckString->SetValue(Languages::GetInstance()->GetString(SID_SCADA_CALLBACK_TEST_RECEIVED));
    break;

  case CALLBACK_TEST_STATE_ACK_FAILED:
    mpCallbackTestReqString->SetValue(Languages::GetInstance()->GetString(SID_SCADA_CALLBACK_TEST_SENT));
    mpCallbackTestAckString->SetValue(Languages::GetInstance()->GetString(SID_SCADA_CALLBACK_TEST_FAILED));
    break;

  case CALLBACK_TEST_STATE_SENT_FAILED:
    mpCallbackTestReqString->SetValue(Languages::GetInstance()->GetString(SID_SCADA_CALLBACK_TEST_FAILED));
    mpCallbackTestAckString->SetValue(Languages::GetInstance()->GetString(SID_SCADA_CALLBACK_TEST_FAILED));
    break;

  case CALLBACK_TEST_STATE_NOT_STARTED:
  default:
    mpCallbackTestReqString->SetValue(Languages::GetInstance()->GetString(SID_NONE));
    mpCallbackTestAckString->SetValue(Languages::GetInstance()->GetString(SID_NONE));
    break;
  }
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
