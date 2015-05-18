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
/* CLASS NAME       : ScadaStateCtrl                                        */
/*                                                                          */
/* FILE NAME        : ScadaStateCtrl.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 26-07-2012 dd-mm-yyyy                                 */
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
#include <ScadaStateCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  SCADA_STATE_TIMER
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
ScadaStateCtrl::ScadaStateCtrl()
{
  mpTimerObjList[SCADA_STATE_TIMER] = new SwTimer(3600, S, false, false, this);

  mRunRequestedFlag         = false;
  mScadaStateTimerRestart   = false;
  mScadaTimerState          = SCADA_TIMER_NOT_STARTED;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ScadaStateCtrl::~ScadaStateCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ScadaStateCtrl::InitSubTask()
{
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ScadaStateCtrl::RunSubTask()
{  
  bool scada_connected = false;  
  
  // get GSM state
  PHONE_CALL_STATE_TYPE phone_call_state = (PHONE_CALL_STATE_TYPE)(mpGsmState->GetValue() & 0x07); // Bit 0-2

  scada_connected = (((mpGprsCommunicationState->GetQuality() == DP_AVAILABLE) && (mpGprsCommunicationState->GetValue() == COM_STATE_CONNECTED)) ||    //Check GPRS
                     ((phone_call_state == PHONE_CALL_STATE_CU_CONNECTION) || (phone_call_state == PHONE_CALL_STATE_CB_CONNECTION)));                  //Check GSM

  
  mRunRequestedFlag = false; 

  //Assumed that only alarm data can be send to SCADA  
  if (mpAlarmToScadaArrived.IsUpdated())                  //Data to SCADA received
  {
    if ((!scada_connected) && (mScadaTimerState == SCADA_TIMER_NOT_STARTED))        //SCADA disconnected, so start monitoring SCADA connection    
    {      
      mpScadaState->SetValue(SCADA_STATE_IDLE);
      mScadaTimerState = SCADA_TIMER_STARTED;
      mpTimerObjList[SCADA_STATE_TIMER]->SetSwTimerPeriod(mpScadaCallbackAlarmObj->GetAlarmConfig()->GetAlarmDelay(), S, false);
      mpTimerObjList[SCADA_STATE_TIMER]->RetriggerSwTimer();       
    }
  }

  if ((mpScadaCallbackAlarmObj.IsUpdated()) && (mScadaTimerState == SCADA_TIMER_STARTED))
  {
    mpTimerObjList[SCADA_STATE_TIMER]->SetSwTimerPeriod(mpScadaCallbackAlarmObj->GetAlarmConfig()->GetAlarmDelay(), S, false);
    mpTimerObjList[SCADA_STATE_TIMER]->RetriggerSwTimer(); 
  }

  if (scada_connected)
  {
    mpScadaState->SetValue(SCADA_STATE_CONNECTED);
    mpTimerObjList[SCADA_STATE_TIMER]->StopSwTimer();
    mScadaStateTimerRestart = false;
    mScadaTimerState = SCADA_TIMER_NOT_STARTED;
  }
  else if (mScadaStateTimerRestart == true)
  {
    mpScadaState->SetValue(SCADA_STATE_NOT_CONNECTED);
    mpTimerObjList[SCADA_STATE_TIMER]->StopSwTimer();
    mScadaStateTimerRestart = false;
    mScadaTimerState = SCADA_TIMER_TIME_OUT;
  }
  mpScadaStateUpdated->SetEvent();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void ScadaStateCtrl::ConnectToSubjects()
{
  mpGsmState->Subscribe(this);
  mpAlarmToScadaArrived->Subscribe(this);  
  mpGprsCommunicationState->Subscribe(this);
  mpScadaCallbackAlarmObj->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void ScadaStateCtrl::Update(Subject* pSubject)
{
  mpGsmState.Update(pSubject);
  mpAlarmToScadaArrived.Update(pSubject);  
  mpGprsCommunicationState.Update(pSubject);
  mpScadaCallbackAlarmObj.Update(pSubject);

  if (pSubject == mpTimerObjList[SCADA_STATE_TIMER])
  {
    mScadaStateTimerRestart = true;
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
void ScadaStateCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpGsmState.Detach(pSubject);
  mpAlarmToScadaArrived.Detach(pSubject);  
  mpGprsCommunicationState.Detach(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void ScadaStateCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Input:
    case SP_SSC_ALARM_TO_SCADA_ARRIVED:
      mpAlarmToScadaArrived.Attach(pSubject);
      break;
    case SP_SSC_GSM_STATE:
      mpGsmState.Attach(pSubject);
      break;
    case SP_SSC_GPRS_COMMUNICATION_STATE:
      mpGprsCommunicationState.Attach(pSubject);
      break;
    case SP_SSC_SCADA_CALLBACK_ALARM_OBJ:
      mpScadaCallbackAlarmObj.Attach(pSubject);
      break;

    // Output:
    case SP_SSC_SCADA_STATE:
      mpScadaState.Attach(pSubject);
      break; 
    case SP_SSC_SCADA_STATE_UPDATED:
      mpScadaStateUpdated.Attach(pSubject);
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
