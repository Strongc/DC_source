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
/* CLASS NAME       : EventLogCtrl                                          */
/*                                                                          */
/* FILE NAME        : EventLogCtrl.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 29-04-2008 dd-mm-yyyy                                 */
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
#include <EventLogCtrl.h>
#include <ActTime.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  CREATES AN OBJECT.
 ******************************************************************************/
EventLogCtrl* EventLogCtrl::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
EventLogCtrl* EventLogCtrl::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new EventLogCtrl();
  }
  return mInstance;
}

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
EventLogCtrl::EventLogCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
EventLogCtrl::~EventLogCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void EventLogCtrl::InitSubTask()
{
  mRunRequestedFlag = false;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void EventLogCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if (mpResetEventLog.IsUpdated())
  {
    // Reset the complete event log, including the ID counter
    EventLogRecord init_value;
    init_value.EventCode = 0;
    for (int i = 0; i < mpEventLogVector->GetMaxSize(); i++)
    {
      mpEventLogVector->SetValue(i, init_value);
    }
    mpEventLogLatestId->SetValue(0);
  }

  if (mpEventLogClearId.IsUpdated())
  {
    // Reset the event log older than or equal the given clear ID
    EventLogRecord init_value;
    init_value.EventCode = 0;
    for (int i = 0; i < mpEventLogVector->GetMaxSize(); i++)
    {
      if (mpEventLogVector->GetValue(i).EventId <= mpEventLogClearId->GetValue())
      {
        mpEventLogVector->SetValue(i, init_value);
      }
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void EventLogCtrl::ConnectToSubjects()
{
  mpResetEventLog->Subscribe(this);
  mpEventLogClearId->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void EventLogCtrl::Update(Subject* pSubject)
{
  mpResetEventLog.Update(pSubject);
  mpEventLogClearId.Update(pSubject);

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
void EventLogCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void EventLogCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
   // Inputs
    case SP_EVL_RESET_EVENT_LOG:
      mpResetEventLog.Attach(pSubject);
      break;
    case SP_EVL_EVENT_LOG_CLEAR_ID:
      mpEventLogClearId.Attach(pSubject);
      break;

    // Outputs
    case SP_EVL_EVENT_LOG_LATEST_ID:
      mpEventLogLatestId.Attach(pSubject);
      break;
    case SP_EVL_EVENT_LOG_VECTOR:
      mpEventLogVector.Attach(pSubject);
      break;


    default:
      break;
  }
}

/*****************************************************************************
 * Function - AddEventLogElement
 * DESCRIPTION:
 *
 *****************************************************************************/
void EventLogCtrl::AddEventLogElement(AlarmDataPoint* pChangedAlarm, ALARM_STATE_TYPE oldAlarmState)
{
  MpcTime           act_time(true); // Create an object with the actual time
  ALARM_STATE_TYPE  new_alarm_state = pChangedAlarm->GetErrorPresent();
  EVENT_LOG_TYPE    disappear_event_log_type = NO_EVENT_LOG;
  EVENT_LOG_TYPE    appear_event_log_type = NO_EVENT_LOG;
  EventLogRecord    new_record;
  U16 event_id;

  if (new_alarm_state != ALARM_STATE_ALARM && oldAlarmState == ALARM_STATE_ALARM)
  {
    disappear_event_log_type = EVENT_LOG_ALARM_DISAPPEAR;
  }
  else if (new_alarm_state != ALARM_STATE_WARNING && oldAlarmState == ALARM_STATE_WARNING)
  {
    disappear_event_log_type = EVENT_LOG_WARNING_DISAPPEAR;
  }

  if (new_alarm_state == ALARM_STATE_ALARM && oldAlarmState != ALARM_STATE_ALARM)
  {
    appear_event_log_type = EVENT_LOG_ALARM_APPEAR;
  }
  else if (new_alarm_state == ALARM_STATE_WARNING && oldAlarmState != ALARM_STATE_WARNING)
  {
    appear_event_log_type = EVENT_LOG_WARNING_APPEAR;
  }

  new_record.EventCode      = (U8)pChangedAlarm->GetValue();
  new_record.EventSource    = (U8)pChangedAlarm->GetErroneousUnit() + (pChangedAlarm->GetErroneousUnitNumber() << 5);
  new_record.EventTimeValue = act_time.GetSecondsSince1Jan1970();

  if (disappear_event_log_type != NO_EVENT_LOG)
  {
    event_id = 1 + (mpEventLogLatestId->GetValue() % (0xFFFF-1));
    new_record.EventId    = event_id;
    new_record.EventType  = disappear_event_log_type;
    mpEventLogVector->PushValue(new_record);
    mpEventLogLatestId->SetValue(event_id);
  }

  if (appear_event_log_type != NO_EVENT_LOG)
  {
    event_id = 1 + (mpEventLogLatestId->GetValue() % (0xFFFF-1));
    new_record.EventId    = event_id;
    new_record.EventType  = appear_event_log_type;
    mpEventLogVector->PushValue(new_record);
    mpEventLogLatestId->SetValue(event_id);
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
