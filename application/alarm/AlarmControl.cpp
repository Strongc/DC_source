/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : AlarmControl                                          */
/*                                                                          */
/* FILE NAME        : AlarmControl.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 16-09-2004 dd-mm-yyyy                                 */
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
#include <AlarmControl.h>
#include <AlarmLog.h>
#include <AlarmEvent.h>
#include <EventLogCtrl.h>
#include <PowerDown.h>


/*****************************************************************************
  DEFINES
 *****************************************************************************/

// The critical alarm table is an array containing the ID's for those alarms that
// should be regarded as critical.
// It is important to assure that the defined number of critical alarms matches
// the size of the array.
#define NO_OF_CRITICAL_ALARMS     5
const ALARM_ID_TYPE CRITICAL_ALARM_TABLE[NO_OF_CRITICAL_ALARMS] = {
  ALARM_ID_HIGH_LEVEL,
  ALARM_ID_ALARM_LEVEL,
  ALARM_ID_DRY_RUNNING,
  ALARM_ID_MAINS_FAULT,
  ALARM_ID_COMMON_PHASE
  };


// SW Timers
enum
{
  RESET_ALARM_DELAY_TIMER
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
 * DESCRIPTION: Initialises private data and reads configuration.
 *
 *****************************************************************************/
AlarmControl::AlarmControl()
{
  mReqTaskTimeFlag = false;


  mpTimerObjList[RESET_ALARM_DELAY_TIMER] = new SwTimer(500, MS, false, false, this);
  OS_CREATERSEMA(&mSemaErrorSourceQueue);

  mAlarmDpErrorVector.clear();
  mAlarmDpErrorUpdatedVector.clear();

  // Create objects for controlling the Alarm Relays
  for (int i = FIRST_ALARM_RELAY; i < NO_OF_ALARM_RELAYS; i++)
  {
    mpAlarmRelay[i] = new AlarmRelay(this, (ALARM_RELAY_TYPE)i);
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: Unsubscirbes the observed AlarmDataPoint's
 *
 ****************************************************************************/
AlarmControl::~AlarmControl()
{
  while (mErrorSourceVector.empty() == false)
  {
    mErrorSourceVector.back()->Unsubscribe(this);
    mErrorSourceVector.pop_back();
  }

  // Delete objects for controlling the Alarm Relays
  for (int i = FIRST_ALARM_RELAY; i < NO_OF_ALARM_RELAYS; i++)
  {
    delete mpAlarmRelay[i];
  }

}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION: Requests first run to initialize system mode and operation mode
 * alarm after power up.
 *
 *****************************************************************************/
void AlarmControl::InitSubTask()
{
  std::vector< SubjectPtr<AlarmDataPoint*> >::iterator iter;

  SetAlarmControlPtrForPowerDown(this);

  PowerDown(); //PowerDown is called to sequre that no alarms in the alarm log are active

  // Initalize objects for controlling the Alarm Relays
  for (int i = FIRST_ALARM_RELAY; i < NO_OF_ALARM_RELAYS; i++)
  {
    mpAlarmRelay[i]->InitAlarmList();
  }

  mpGroup2Enabled.SetUpdated();

  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    mpPumpOprMode[i].SetUpdated();
  }

  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION: Empties event queue, handles call of ResetAlarmLog and
 * ClearAlarmLog. Finally calls DetermineAlarmStates.
 *
 *****************************************************************************/
void AlarmControl::RunSubTask()
{
  int vector_size;
  bool update;

  std::vector< SubjectPtr<AlarmDataPoint*> >::iterator iter;

  mReqTaskTimeFlag = false;

  /* Handle pump alarm config change due to group change */
  update  = mpGroup2Enabled.IsUpdated();
  update |= mpFirstPumpInGroup2.IsUpdated();
  if (update)
  {
    vector_size = mErrorSourceVector.size();
    for (int i = 0; i < vector_size; i++)
    {
      if (mErrorSourceVector.at(i)->GetErroneousUnit() == ERRONEOUS_UNIT_PUMP || mErrorSourceVector.at(i)->GetErroneousUnit() == ERRONEOUS_UNIT_IO111 || mErrorSourceVector.at(i)->GetErroneousUnit() == ERRONEOUS_UNIT_MP204 || mErrorSourceVector.at(i)->GetErroneousUnit() == ERRONEOUS_UNIT_CUE)
      {
        if( mpGroup2Enabled->GetValue()==true && mErrorSourceVector.at(i)->GetErroneousUnitNumber()>=mpFirstPumpInGroup2->GetValue() )
        {
          mErrorSourceVector.at(i)->SetActualAlarmConfig(1);
        }
        else
        {
          mErrorSourceVector.at(i)->SetActualAlarmConfig(0);
        }
      }
    }
  }

  mpAlarmLog->UseAlarmLog();

  /* Handle change in PumpOprMode */
  bool state_changed = false;
  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    if (mpPumpOprMode[i].IsUpdated())
    {
      state_changed=true;
    }
  }
  if (state_changed)
  {
    vector_size = mErrorSourceVector.size();
    for (int i = 0; i < vector_size; i++)
    {
      if (mErrorSourceVector.at(i)->GetErroneousUnit() == ERRONEOUS_UNIT_PUMP || mErrorSourceVector.at(i)->GetErroneousUnit() == ERRONEOUS_UNIT_IO111 || mErrorSourceVector.at(i)->GetErroneousUnit() == ERRONEOUS_UNIT_CUE || mErrorSourceVector.at(i)->GetErroneousUnit() == ERRONEOUS_UNIT_MP204)
      {
        for (int j = 0; j < NO_OF_PUMPS; j++)
        {
          if (mErrorSourceVector.at(i)->GetErroneousUnitNumber()==(j+1))
          {
            if (mpPumpOprMode[j]->GetValue() == ACTUAL_OPERATION_MODE_NOT_INSTALLED || mpPumpOprMode[j]->GetValue() == ACTUAL_OPERATION_MODE_DISABLED)
            {
              mErrorSourceVector.at(i)->SetDataPointActive(false);
            }
            else
            {
              mErrorSourceVector.at(i)->SetDataPointActive(true);
            }
          }
        }
      }
    }
  }


  // The evaluations below were previously done in the Update function. As we should use as little time
  // as possible in the Update evaluations were moved into the Subtask. The previous use of a queue is
  // hereby avoided.
  // An iterator is not used because it is important to assure that the ErrorSourceVector and the
  // AlarmDpErrorVector are syncronized.
  vector_size = mErrorSourceVector.size();
  for (int i = 0; i < vector_size; i++)
  {
    if (mErrorSourceVector.at(i).IsUpdated())
    {
      AlarmDataPoint* pErrorSource = mErrorSourceVector.at(i).GetSubject();
      CheckAlarmToScadaCallback(pErrorSource);
      if (mErrorSourceVector.at(i)->GetErrorPresent() != mAlarmDpErrorVector.at(i))
      {
        EventLogCtrl::GetInstance()->AddEventLogElement(pErrorSource, mAlarmDpErrorVector.at(i));
        NewAlarmEvent(pErrorSource);
        UpdateAlarmRelayLists(pErrorSource);

        mAlarmDpErrorVector.at(i) = mErrorSourceVector.at(i)->GetErrorPresent();
      }
      else if (mAlarmDpErrorUpdatedVector.at(i) == true &&
               mErrorSourceVector.at(i)->GetErrorPresent() == ALARM_STATE_READY)
      {
        // An error has been present and has gone again within the same update.
        // This will not be registered in the log, but it is necessary to make sure, that
        // AlarmPresent is set back to ALARM_STATE_READY. AlarmPresent could still be WARNING or
        // ALARM if the ErrorSource is configured to manual acknowledgement.
        mErrorSourceVector.at(i)->SetAlarmPresent(ALARM_STATE_READY);
      }
      mAlarmDpErrorUpdatedVector.at(i) = false;
    }
  }

  if (mpAlarmResetEvent.IsUpdated())
  {
    mpAlarmLog->ResetAlarmLog(RESET_MANUAL);
    for (iter = mErrorSourceVector.begin(); iter != mErrorSourceVector.end(); iter++)
    {
      if ((*iter)->GetErrorPresent() == ALARM_STATE_READY)
      {
        // Error has gone - reset the alarm.
        (*iter)->SetAlarmPresent(ALARM_STATE_READY);
      }
    }
  }

  if (mpClearAlarmLogEvent.IsUpdated())
  {
    mpAlarmLog->ClearAlarmLog();
  }

  mpAlarmLog->ResetAlarmLog(RESET_AUTOMATIC);

  if (mpAlarmLog->ExecuteNotify())
  {
    int cnt = mpAlarmLogChangedCnt->GetValue();
    cnt++;
    cnt &= 0xFFFF;   // Secure that we never reach max value
    mpAlarmLogChangedCnt->SetValue(cnt);
  }

  mpAlarmLog->UnuseAlarmLog();

  // Check AlarmRelays
  for (int i = FIRST_ALARM_RELAY; i < NO_OF_ALARM_RELAYS; i++)
  {
    mpAlarmRelay[i]->CheckAlarmList();
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects. Only subscribes to error sources, because
 * updates from the other subjects are not wanted.
 *
 *****************************************************************************/
void AlarmControl::ConnectToSubjects()
{
  std::vector< SubjectPtr<AlarmDataPoint*> >::iterator iter;

  for (iter = mErrorSourceVector.begin(); iter != mErrorSourceVector.end(); iter++)
  {
    (*iter)->Subscribe(this);
  }
  mpAlarmResetEvent->Subscribe(this);
  mpClearAlarmLogEvent->Subscribe(this);
  mpGroup2Enabled->Subscribe(this);
  mpFirstPumpInGroup2->Subscribe(this);


  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    mpPumpOprMode[i]->Subscribe(this);
  }

  // Connect AlarmRelays to subjects
  for (int i = 0; i < NO_OF_ALARM_RELAYS; i++)
  {
    mpAlarmRelay[i]->ConnectToSubjects();
  }

}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void AlarmControl::Update(Subject* pSubject)
{
  int vector_size;

  vector_size = mErrorSourceVector.size();
  for (int i = 0; i < vector_size; i++)
  {
    if (mErrorSourceVector.at(i).Update(pSubject))
    {
      // If ErrorPresent has been set the set a flag. This is to assure that it is
      // always detected that ErrorPresent has been changed. Even if ErrorPresent is
      // set and then is cleared again before the AlarmControl has activated RunSubTask
      if (mErrorSourceVector.at(i)->GetErrorPresent() != mAlarmDpErrorVector.at(i))
      {
        mAlarmDpErrorUpdatedVector.at(i) = true;
      }
      // The updated subject is located - no need to search the rest of the vector
      i = vector_size;
    }
  }

  mpGroup2Enabled.Update(pSubject);
  mpFirstPumpInGroup2.Update(pSubject);

  if (pSubject == mpTimerObjList[RESET_ALARM_DELAY_TIMER])
  {
    mpAlarmResetEvent.SetUpdated();
  }
  else if (mpAlarmResetEvent.Update(pSubject))
  {
    mpTimerObjList[RESET_ALARM_DELAY_TIMER]->RetriggerSwTimer();
  }
  else if (mpClearAlarmLogEvent.Update(pSubject))
  {
    // nop
  }
  else
  {
    for (int i = 0; i < NO_OF_PUMPS; i++)
    {
      mpPumpOprMode[i].Update(pSubject);
    }
  }

  if (mReqTaskTimeFlag == false)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
* Function - SubscribtionCancelled
* DESCRIPTION: If pSubject is a pointer of AlarmContol then set to 0
*
*****************************************************************************/
void AlarmControl::SubscribtionCancelled(Subject* pSubject)
{
  int vector_size;

  vector_size = mErrorSourceVector.size();
  for (int i = 0; i < vector_size; i++)
  {
    if (mErrorSourceVector.at(i).Detach(pSubject))
    {
      return;
    }
  }
  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    mpPumpOprMode[i].Detach(pSubject);
  }
  mpGroup2Enabled.Detach(pSubject);
  mpFirstPumpInGroup2.Detach(pSubject);

  mpAlarmLog.Detach(pSubject);

}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed by AlarmControl.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void AlarmControl::SetSubjectPointer(int id, Subject* pSubject)
{
  SubjectPtr<AlarmDataPoint*>* p_element;
  switch(id)
  {
    // error sources
    case SP_AC_ALARM_DATA_POINT:
      p_element = new SubjectPtr<AlarmDataPoint*>;
      p_element->Attach(pSubject);
      mErrorSourceVector.push_back(*p_element);
      mAlarmDpErrorVector.push_back((*p_element)->GetErrorPresent());
      mAlarmDpErrorUpdatedVector.push_back(false);
      delete p_element;
      break;

    // misc.
    case SP_AC_GROUP_2_ENABLED:
      mpGroup2Enabled.Attach(pSubject);
    break;
    case SP_AC_FIRST_PUMP_IN_GROUP_2:
      mpFirstPumpInGroup2.Attach(pSubject);
    break;
    case SP_AC_ALARM_LOG:
      mpAlarmLog.Attach(pSubject);
    break;
    case SP_AC_ALARM_LOG_CHANGED_CNT:
      mpAlarmLogChangedCnt.Attach(pSubject);
    break;
    case SP_AC_ALARM_RESET_EVENT:
      mpAlarmResetEvent.Attach(pSubject);
      break;
    case SP_AC_CLEAR_ALARM_LOG_EVENT:
      mpClearAlarmLogEvent.Attach(pSubject);
      break;
    case SP_AC_ALARM_EVENT_NUMBER:
      mpAlarmEventNumber.Attach(pSubject);
      break;
    case SP_AC_ALARM_TO_SCADA_ARRIVED:
      mpAlarmToScadaArrived.Attach(pSubject);
      break;
    case SP_AC_PUMP_1_OPR_MODE:
      mpPumpOprMode[PUMP_1].Attach(pSubject);
      break;
    case SP_AC_PUMP_2_OPR_MODE:
      mpPumpOprMode[PUMP_2].Attach(pSubject);
      break;
    case SP_AC_PUMP_3_OPR_MODE:
      mpPumpOprMode[PUMP_3].Attach(pSubject);
      break;
    case SP_AC_PUMP_4_OPR_MODE:
      mpPumpOprMode[PUMP_4].Attach(pSubject);
      break;
    case SP_AC_PUMP_5_OPR_MODE:
      mpPumpOprMode[PUMP_5].Attach(pSubject);
      break;
    case SP_AC_PUMP_6_OPR_MODE:
      mpPumpOprMode[PUMP_6].Attach(pSubject);
      break;

    // Set subject pointers for AlarmRelay objects
    case SP_AC_ALARM_RELAY_RESET_EVENT:
      for (int i = 0; i < NO_OF_ALARM_RELAYS; i++)
      {
        mpAlarmRelay[i]->SetSubjectPointer(ALARM_RELAY_CONNECT_RESET_EVENT, pSubject);
      }
      break;
    case SP_AC_ALARM_RELAY_ACK_CUSTOM:
      mpAlarmRelay[ALARM_RELAY_CUSTOM]->SetSubjectPointer(ALARM_RELAY_CONNECT_ACK, pSubject);
      break;
    case SP_AC_ALARM_RELAY_ACK_HIGH_LEVEL:
      mpAlarmRelay[ALARM_RELAY_HIGH_LEVEL]->SetSubjectPointer(ALARM_RELAY_CONNECT_ACK, pSubject);
      break;
    case SP_AC_ALARM_RELAY_ACK_CRITICAL:
      mpAlarmRelay[ALARM_RELAY_CRITICAL]->SetSubjectPointer(ALARM_RELAY_CONNECT_ACK, pSubject);
      break;
    case SP_AC_ALARM_RELAY_ACK_ALL_ALARMS:
      mpAlarmRelay[ALARM_RELAY_ALL_ALARMS]->SetSubjectPointer(ALARM_RELAY_CONNECT_ACK, pSubject);
      break;
    case SP_AC_ALARM_RELAY_ACK_ALL_ALARMS_AND_WARNINGS:
      mpAlarmRelay[ALARM_RELAY_ALL_ALARMS_AND_WARNINGS]->SetSubjectPointer(ALARM_RELAY_CONNECT_ACK, pSubject);
      break;
    case SP_AC_ALARM_RELAY_OUTPUT_VALUE_CUSTOM:
      mpAlarmRelay[ALARM_RELAY_CUSTOM]->SetSubjectPointer(ALARM_RELAY_CONNECT_OUTPUT_VALUE, pSubject);
      break;
    case SP_AC_ALARM_RELAY_OUTPUT_VALUE_HIGH_LEVEL:
      mpAlarmRelay[ALARM_RELAY_HIGH_LEVEL]->SetSubjectPointer(ALARM_RELAY_CONNECT_OUTPUT_VALUE, pSubject);
      break;
    case SP_AC_ALARM_RELAY_OUTPUT_VALUE_CRITICAL:
      mpAlarmRelay[ALARM_RELAY_CRITICAL]->SetSubjectPointer(ALARM_RELAY_CONNECT_OUTPUT_VALUE, pSubject);
      break;
    case SP_AC_ALARM_RELAY_OUTPUT_VALUE_ALL_ALARMS:
      mpAlarmRelay[ALARM_RELAY_ALL_ALARMS]->SetSubjectPointer(ALARM_RELAY_CONNECT_OUTPUT_VALUE, pSubject);
      break;
    case SP_AC_ALARM_RELAY_OUTPUT_VALUE_ALL_ALARMS_AND_WARNINGS:
      mpAlarmRelay[ALARM_RELAY_ALL_ALARMS_AND_WARNINGS]->SetSubjectPointer(ALARM_RELAY_CONNECT_OUTPUT_VALUE, pSubject);
      break;
    case SP_AC_ALARM_RELAY_MANUAL_ACK_PRESENT_CUSTOM:
      mpAlarmRelay[ALARM_RELAY_CUSTOM]->SetSubjectPointer(ALARM_RELAY_MANUAL_ACK_PRESENT, pSubject);
      break;
    case SP_AC_ALARM_RELAY_MANUAL_ACK_PRESENT_HIGH_LEVEL:
      mpAlarmRelay[ALARM_RELAY_HIGH_LEVEL]->SetSubjectPointer(ALARM_RELAY_MANUAL_ACK_PRESENT, pSubject);
      break;
    case SP_AC_ALARM_RELAY_MANUAL_ACK_PRESENT_CRITICAL:
      mpAlarmRelay[ALARM_RELAY_CRITICAL]->SetSubjectPointer(ALARM_RELAY_MANUAL_ACK_PRESENT, pSubject);
      break;
    case SP_AC_ALARM_RELAY_MANUAL_ACK_PRESENT_ALL_ALARMS:
      mpAlarmRelay[ALARM_RELAY_ALL_ALARMS]->SetSubjectPointer(ALARM_RELAY_MANUAL_ACK_PRESENT, pSubject);
      break;
    case SP_AC_ALARM_RELAY_MANUAL_ACK_PRESENT_ALL_ALARMS_AND_WARNINGS:
      mpAlarmRelay[ALARM_RELAY_ALL_ALARMS_AND_WARNINGS]->SetSubjectPointer(ALARM_RELAY_MANUAL_ACK_PRESENT, pSubject);
      break;
    default:
      id++;
      break;
  }


}

/*****************************************************************************
 * Function - PowerDown
 * DESCRIPTION: Set departure time in active alarms, and then execute a
 * manual reset.
 *
 *****************************************************************************/
void AlarmControl::PowerDown()
{
  mpAlarmLog->UseAlarmLog();

  mpAlarmLog->SetDepartureTimeInAllActiveEvents();
  mpAlarmLog->ResetAlarmLog(RESET_MANUAL);
  mpAlarmLog->ExecuteNotify();

  mpAlarmLog->UnuseAlarmLog();
}


/*****************************************************************************
 * Function - ReqCheckAlarmList
 * DESCRIPTION: Called from an AlarmRelay when the alarm relay object wants to
 *              be serviced.
 *
 *****************************************************************************/
void AlarmControl::ReqCheckAlarmList()
{
  if (mReqTaskTimeFlag == false)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
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
 * Function - NewAlarmEvent
 * DESCRIPTION: This operation is called when the error source changes the
 * ID or AlarmState. Therefore is an old error departured, if the error source
 * changes the ID or AlarmState. If it is a new ID or a new AlarmState,
 * then a new alarm log element is build.
 *
 *****************************************************************************/
void AlarmControl::NewAlarmEvent(AlarmDataPoint* pErrorSource)
{
  U32 alarm_event_number;

  mpAlarmLog->ErrorDepartured(pErrorSource);  // Error is departured if error_present is false
  alarm_event_number = mpAlarmEventNumber->GetValue();
  if (mpAlarmLog->PutAlarmLogElement(pErrorSource, alarm_event_number) == true)
  {
    mpAlarmEventNumber->SetValue(alarm_event_number+1);
  }
}

/*****************************************************************************
 * Function - UpdateAlarmRelayLists
 * DESCRIPTION: This operation is called when the error source is changed.
 * The error source is removed from all lists and inserted
 * in the lists corresponding to the new ID, if the ID differs from NO_ALARM
 *
 *****************************************************************************/
void AlarmControl::UpdateAlarmRelayLists(AlarmDataPoint* pErrorSource)
{
  if ((pErrorSource->GetAlarmPresent() == pErrorSource->GetErrorPresent()) &&
      (pErrorSource->GetErrorPresent() != ALARM_STATE_READY))
  {
    // Find which relay(s) the error source activates
    // Check if configured for custom relay
    if (pErrorSource->GetAlarmConfig()->GetCustomRelayForAlarm() == true &&
        pErrorSource->GetErrorPresent() == ALARM_STATE_ALARM)
    {
      mpAlarmRelay[ALARM_RELAY_CUSTOM]->AddAlarm(pErrorSource);
    }
    if (pErrorSource->GetAlarmConfig()->GetCustomRelayForWarning() == true &&
        pErrorSource->GetErrorPresent() == ALARM_STATE_WARNING)
    {
      mpAlarmRelay[ALARM_RELAY_CUSTOM]->AddAlarm(pErrorSource);
    }
    // Check if it is high level alarm
    if (pErrorSource->GetValue() == ALARM_ID_HIGH_LEVEL)
    {
      mpAlarmRelay[ALARM_RELAY_HIGH_LEVEL]->AddAlarm(pErrorSource);
    }
    // Check if it is a critical alarm
    for (int i = 0; i < NO_OF_CRITICAL_ALARMS; i++)
    {
      if (pErrorSource->GetValue() == CRITICAL_ALARM_TABLE[i])
      {
        mpAlarmRelay[ALARM_RELAY_CRITICAL]->AddAlarm(pErrorSource);
        break;
      }
    }
//    Check if it is an alarm and not just a warning
    if (pErrorSource->GetErrorPresent() != ALARM_STATE_WARNING)
    {
      mpAlarmRelay[ALARM_RELAY_ALL_ALARMS]->AddAlarm(pErrorSource);
    }
//    Always register at the relay for all alarms and warnings
    mpAlarmRelay[ALARM_RELAY_ALL_ALARMS_AND_WARNINGS]->AddAlarm(pErrorSource);
  }

}

/*****************************************************************************
 * Function - CheckAlarmToScadaCallback
 * DESCRIPTION: This operation is called when the error source is changed.
 *              If a the alarm it is configured to trig a scada callback,
 *              then set a scada callback event.
 *
 *****************************************************************************/
void AlarmControl::CheckAlarmToScadaCallback(AlarmDataPoint* pErrorSource)
{
  //No need of scada callback when callback test disabled
  if ((pErrorSource->GetValue() != ALARM_ID_SCADA_CALLBACK_TEST) ||
      ((pErrorSource->GetValue() == ALARM_ID_SCADA_CALLBACK_TEST) && (pErrorSource->GetAlarmPresent() == ALARM_STATE_ALARM)))
  {
    if (pErrorSource->GetAlarmConfig()->GetScadaEnabled() == true)
    {
      mpAlarmToScadaArrived->SetEvent();
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
