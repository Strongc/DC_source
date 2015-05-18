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
/* CLASS NAME       : AlarmLog                                              */
/*                                                                          */
/* FILE NAME        : AlarmLog.cpp                                          */
/*                                                                          */
/* CREATED DATE     : 17-09-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <ActTime.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AlarmLog.h>
#include <AlarmEvent.h>
#include <AlarmLogDataHistory.h>

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
 * DESCRIPTION: Creates the AlarmEvent's and put them in the mAlarmLog.
 *
 *****************************************************************************/
AlarmLog::AlarmLog()
{
  AlarmEvent* p_alarm_event;

  for (int i = 0; i < ALARM_LOG_SIZE; i++)
  {
    p_alarm_event = new AlarmEvent(0);
    mAlarmLog.push_front(p_alarm_event);
  }

  mAlarmLogChangedFlag = false;

  OS_CREATERSEMA(&mSemaAlarmLog);
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: Delete all AlarmEvents in the alarm log. Erase the mAlarmLog.
 *
 ****************************************************************************/
AlarmLog::~AlarmLog()
{
  std::deque<AlarmEvent*>::iterator iter;

  for (iter = mAlarmLog.begin(); iter != mAlarmLog.end(); iter++)
  {
    delete (*iter);
  }

  mAlarmLog.erase(mAlarmLog.begin(), mAlarmLog.end());
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmLog::ConnectToSubjects()
{
  //ignore
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmLog::Update(Subject* pSubject)
{
  //ignore
}

/*****************************************************************************
* Function - SubscribtionCancelled
* DESCRIPTION:
*
*****************************************************************************/
void AlarmLog::SubscribtionCancelled(Subject* pSubject)
{
  mDpHasUnAckAlarms.Detach();
  mDateTimeOfLatestClear.Detach();
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmLog::SetSubjectPointer(int id, Subject* pSubject)
{
  if (id == SP_AL_HAS_UNACK_ALARMS)
    mDpHasUnAckAlarms.Attach(pSubject);
  else if (id == SP_AL_DATETIME_OF_LATEST_ALARM_LOG_RESET)
    mDateTimeOfLatestClear.Attach(pSubject);

}

/*****************************************************************************
 * Function - GetAlarmLogElement
 * DESCRIPTION: Returns a pointer to AlarmEvent in the mAlarmLog at posistion
 * index.
 *
 *****************************************************************************/
AlarmEvent* AlarmLog::GetAlarmLogElement(int index)
{
  std::deque<AlarmEvent*>::iterator iter;

  if (index >= ALARM_LOG_SIZE)
  {
    index = ALARM_LOG_SIZE - 1;
  }

  iter = mAlarmLog.begin();
  for (int i = 0; i < index; i++)
  {
    iter++;
  }

  return *iter;
}

/*****************************************************************************
 * Function - GetFirstAlarmLogElement
 * DESCRIPTION: Returns an iterator pointing at the first element in the
 * mAlarmLog.
 *
 *****************************************************************************/
std::deque<AlarmEvent*>::iterator AlarmLog::GetFirstAlarmLogElement()
{
  return mAlarmLog.begin();
}

/*****************************************************************************
 * Function - PutAlarmLogElement
 * DESCRIPTION: Delete oldest alarm event which is acknowledged. Inserts a
 * copy of pElement at the beginning and calls NotifyObservers.
 *
 *****************************************************************************/
bool AlarmLog::PutAlarmLogElement(AlarmDataPoint* pErrorSource, U32 alarmEventNumber)
{
  AlarmEvent* p_element;
  std::deque<AlarmEvent*>::iterator iter;
  bool element_deleted = false;
  bool put_element = true;

  // Check if new element should be put into alarmlog
  if (pErrorSource->GetErrorPresent() != ALARM_STATE_READY)
  {
    for (iter = mAlarmLog.begin(); iter != mAlarmLog.end(); iter++)
    {
      if (((*iter)->GetErrorSource() == pErrorSource) &&
          ((*iter)->GetAlarmType() == pErrorSource->GetErrorPresent()))
      {
        // Identical event exists - is it unacknowledged?
        if ( (*iter)->GetDepartureTime()->GetDate(YEAR) == INVALID_DATE )
        {
          // An identical element that is not acknowledged is already present in the log.
          // Do not put a new element into the log.
          put_element = false;
        }
      }
    }
  }
  else
  {
    // The element has no error present - it should not be put into the alarm log.
    put_element = false;
  }

  if (put_element == true)
  {
    // The element was not found - put it into the log
    iter = mAlarmLog.end();
    while (!element_deleted && iter != mAlarmLog.begin())
    {
      iter--;
      if ((*iter)->GetAcknowledge())
      {
        delete (*iter);                   // Delete oldest alarm event which is acknowledged
        mAlarmLog.erase(iter);            // Removes the element
        element_deleted = true;
      }
    }

    if (!element_deleted)
    {
      iter = mAlarmLog.end();
      while (!element_deleted && iter != mAlarmLog.begin())
      {
        iter--;
        if ((*iter)->GetAlarmType() == SYSTEM_MODE_READY)
        {
          delete (*iter);                   // Delete oldest alarm event which is a warning
          mAlarmLog.erase(iter);            // Removes the element
          element_deleted = true;
        }
      }
    }

    if (!element_deleted)
    {
      iter = mAlarmLog.end();
      while (!element_deleted && iter != mAlarmLog.begin())
      {
        iter--;
        if ((*iter)->GetDepartureTime()->GetDate(YEAR) != INVALID_DATE)
        {
          delete (*iter);                   // Delete oldest alarm event where departure time is set
          mAlarmLog.erase(iter);            // Removes the element
          element_deleted = true;
        }
      }
    }

//  if (element_deleted)
//  {
//    mAlarmLog.push_front(pElement);       // Inserts a copy of pElement at the beginning
//    mAlarmLogChangedFlag = true;
//  }
//  else
//  {
//    delete pElement;                    // Error!! Alarm Log overflow!!
//  }

    if (element_deleted)
    {
      ERRONEOUS_UNIT_TYPE erroneous_unit_type = pErrorSource->GetErroneousUnit();

      p_element = new AlarmEvent(alarmEventNumber);
      p_element->SetAlarmId(pErrorSource->GetValue());
      p_element->SetAlarmType(pErrorSource->GetErrorPresent());
      p_element->SetAcknowledge(false);
      p_element->SetArrivalTime();
      p_element->SetDepartureTimeNotAvailable();
      p_element->SetErroneousUnit(erroneous_unit_type);
      p_element->SetErroneousUnitNumber(pErrorSource->GetErroneousUnitNumber());
      p_element->SetErrorSource(pErrorSource->GetSubjectId() );

      int pump_error_index = -1;

      if (erroneous_unit_type == ERRONEOUS_UNIT_MP204 ||
          erroneous_unit_type == ERRONEOUS_UNIT_IO111 ||
          erroneous_unit_type == ERRONEOUS_UNIT_PUMP  ||
          erroneous_unit_type == ERRONEOUS_UNIT_CUE   )
      {
        pump_error_index = pErrorSource->GetErroneousUnitNumber() - 1;
      }

      AlarmLogDataHistory::DataHistoryEntry hist = AlarmLogDataHistory::GetInstance()->GetBestDataHistoryEntry(pump_error_index);
      p_element->SetSnapShotData(hist);

//       p_element->SetSurfaceLevelReady(hist.mSurfaceLevelReady);
//       p_element->SetSurfaceLevel(hist.mSurfaceLevel);
//       p_element->SetAverageFlow(hist.mAverageFlow);
//       p_element->SetPumpOperatingMode(hist.mPumpOperatingMode);
//       p_element->SetPumpVoltage(hist.mPumpVoltage);
//       p_element->SetPumpCurrent(hist.mPumpCurrent);
//       p_element->SetPumpCosPhi(hist.mPumpCosPhi);
//       p_element->SetPumpPower(hist.mPumpPower);
//       p_element->SetPumpFlow(hist.mPumpFlow);
//       p_element->SetPumpTemperature(hist.mPumpTemperature);

//       p_element->SetSurfaceLevelReadyQuality(hist.mSurfaceLevelReadyQuality);
//       p_element->SetSurfaceLevelQuality(hist.mSurfaceLevelQuality);
//       p_element->SetAverageFlowQuality(hist.mAverageFlowQuality);
//       p_element->SetPumpOperatingModeQuality(hist.mPumpOperatingModeQuality);
//       p_element->SetPumpVoltageQuality(hist.mPumpVoltageQuality);
//       p_element->SetPumpCurrentQuality(hist.mPumpCurrentQuality);
//       p_element->SetPumpCosPhiQuality(hist.mPumpCosPhiQuality);
//       p_element->SetPumpPowerQuality(hist.mPumpPowerQuality);
//       p_element->SetPumpFlowQuality(hist.mPumpFlowQuality);
//       p_element->SetPumpTemperatureQuality(hist.mPumpTemperatureQuality);

      mAlarmLog.push_front(p_element);       // Inserts a copy of pElement at the beginning
      mAlarmLogChangedFlag = true;
    }
    else
    {
      return false;;                    // Error!! Alarm Log overflow!!
    }
  }
  else
  {
    return false;                       // Element was already in the log
  }

  return true;
}

/*****************************************************************************
 * Function - AlarmDepartured
 * DESCRIPTION: Look for pErrorSource in AlarmLog. If departure time is NA,
 * then set departure time and calls NotifyObservers.
 *
 *****************************************************************************/
void AlarmLog::ErrorDepartured(AlarmDataPoint* pErrorSource)
{
  std::deque<AlarmEvent*>::iterator iter;

  for (iter = mAlarmLog.begin(); iter != mAlarmLog.end(); iter++)  // Maybe more than one element with this error source
  {                                                                // Therefore scan all elements in the Alarm log
    if ((*iter)->GetErrorSource() == pErrorSource)
    {
      switch (pErrorSource->GetErrorPresent())
      {
        case ALARM_STATE_WARNING:
          // If ErrorPresent is WARNING existing alarms should be departured
          if ( (*iter)->GetAlarmType() == ALARM_STATE_ALARM)
          {
            if ( (*iter)->GetDepartureTime()->GetDate(YEAR) == INVALID_DATE )
            {
              (*iter)->SetDepartureTime();
              mAlarmLogChangedFlag = true;
            }
          }
          break;
        case ALARM_STATE_READY:
          // If no error is present all alarms and warnings should be departured
          if ( (*iter)->GetDepartureTime()->GetDate(YEAR) == INVALID_DATE )
          {
            (*iter)->SetDepartureTime();
            mAlarmLogChangedFlag = true;
          }
          break;
        default:
          // If ErrorPresent is ALARM then nothing is departured.
          break;
      }
    }
  }
}

/*****************************************************************************
 * Function - ResetAlarmLog
 * DESCRIPTION: Scan the mAlarmLog for departured alarms. If reset type is MANUAL
 * then set acknowledge for all departured alarms. If reset type is AUTOMATIC
 * then only set acknowledge, if alarm is departured and error source has AUTOMATIC
 * reset type.
 *
 *****************************************************************************/
void AlarmLog::ResetAlarmLog(RESET_TYPE resetType)
{
  UseAlarmLog();

  std::deque<AlarmEvent*>::iterator iter;
  AlarmDataPoint* p_error_source;

  for (iter = mAlarmLog.begin(); iter != mAlarmLog.end(); iter++)
  {
    if ((*iter)->GetDepartureTime()->GetDate(YEAR) != INVALID_DATE)
    {
      if (resetType == RESET_AUTOMATIC)
      {
        p_error_source = (*iter)->GetErrorSource();
        if (p_error_source)
        {
          if ((*iter)->GetAlarmType() == ALARM_STATE_ALARM)
          {
            // An alarm acknowledge is dependent upon the alarm configuration
            if (p_error_source->GetAlarmConfig()->GetAutoAcknowledge() == true)
            {
              (*iter)->SetAcknowledge(true);
              mAlarmLogChangedFlag = true;
            }
          }
          else
          {
            // A warning acknowledge is always regarded as automatic
            (*iter)->SetAcknowledge(true);
            mAlarmLogChangedFlag = true;
          }
        }
      }
      else if (resetType == RESET_MANUAL)
      {
        (*iter)->SetAcknowledge(true);
        mAlarmLogChangedFlag = true;
      }
    }
  }
  UnuseAlarmLog();
}

/*****************************************************************************
 * Function - ClearAlarmLog
 * DESCRIPTION: Set all alarm log elements to default values.
 *
 *****************************************************************************/
void AlarmLog::ClearAlarmLog()
{
  UseAlarmLog();
  std::deque<AlarmEvent*>::iterator iter;
  AlarmEvent* p_alarm_event;
  int delete_cnt = 0;

  iter = mAlarmLog.begin();
  while (iter != mAlarmLog.end())
  {
    MpcTime* p_time = (*iter)->GetDepartureTime();
    if ( p_time->GetDate(YEAR) != INVALID_DATE )
    {
      delete (*iter);
      mAlarmLog.erase(iter);            // Removes the element
      delete_cnt++;
      iter = mAlarmLog.begin();
    }
    else
    {
      iter++;
    }
  }

  for (int i = 0; i < delete_cnt; i++)
  {
    p_alarm_event = new AlarmEvent(0);
    mAlarmLog.push_back(p_alarm_event);
  }
  mAlarmLogChangedFlag = true;
  UnuseAlarmLog();

  ActTime* dt = ActTime::GetInstance();
  mDateTimeOfLatestClear->SetValue( dt->GetSecondsSince1Jan1970() );
}

/*****************************************************************************
 * Function - HasUnAckAlarms
 * DESCRIPTION: Returns true if one or more alarms hasn't been acked
 *
 *****************************************************************************/
bool AlarmLog::HasUnAckAlarms()
{
  bool rc = false;

  UseAlarmLog();

  for (std::deque<AlarmEvent*>::iterator itr = mAlarmLog.begin(); itr != mAlarmLog.end(); itr++)
  {
   if ((*itr)->GetAlarmType() == ALARM_STATE_ALARM)
	 {
      if (!(*itr)->GetAcknowledge())
      {
       rc = true;
       break;
      }
	 }
  }

  UnuseAlarmLog();

  return rc;
}

/*****************************************************************************
 * Function - ExecuteNotify
 * DESCRIPTION: If alarm log is changed the call NotifyObservers()
 *
 *****************************************************************************/
bool AlarmLog::ExecuteNotify()
{
  bool return_val = false;

  if (mAlarmLogChangedFlag)
  {
    if (mDpHasUnAckAlarms.IsValid())
      mDpHasUnAckAlarms->SetAsBool( this->HasUnAckAlarms() );

    mAlarmLogChangedFlag = false;
    return_val = true;
    NotifyObservers();
  }

  return return_val;
}

/*****************************************************************************
 * Function - SetDepartureTimeInAllActiveEvents
 * DESCRIPTION: Set departure time in events where arrival time is valid and
 * departure time isn't valid
 *
 *****************************************************************************/
void AlarmLog::SetDepartureTimeInAllActiveEvents()
{
  std::deque<AlarmEvent*>::iterator iter;

  for (iter = mAlarmLog.begin(); iter != mAlarmLog.end(); iter++)
  {
    if ((*iter)->GetArrivalTime()->GetDate(YEAR) != INVALID_DATE)
    {
      if ((*iter)->GetDepartureTime()->GetDate(YEAR) == INVALID_DATE)
      {
        (*iter)->SetDepartureTime();
        mAlarmLogChangedFlag = true;
      }
    }
  }
}

/*****************************************************************************
 * Function - UseAlarmLog
 * DESCRIPTION: Routine to be called before using alarm log
 *
 *****************************************************************************/
void AlarmLog::UseAlarmLog()
{
  OS_Use(&mSemaAlarmLog);
}

/*****************************************************************************
 * Function - UnuseAlarmLog
 * DESCRIPTION: Routine to be called after using alarm log
 *
 *****************************************************************************/
void AlarmLog::UnuseAlarmLog()
{
  OS_Unuse(&mSemaAlarmLog);
}

/*****************************************************************************
* FUNCTION - GetFlashId
* DESCRIPTION:
*****************************************************************************/
FLASH_ID_TYPE AlarmLog::GetFlashId(void)
{
	return FLASH_ID_ALARM_LOG;
}

/*****************************************************************************
* FUNCTION - SaveToFlash
* DESCRIPTION:
*****************************************************************************/
void AlarmLog::SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
{
  AlarmEvent* p_alarm_event;

  // write version
  pWriter->WriteU8(3);

  // write alarm log size
  pWriter->WriteI32(ALARM_LOG_SIZE);

  // lock access
  UseAlarmLog();

  // save elements
  for (int i = 0; i < ALARM_LOG_SIZE; i++)
  {
    p_alarm_event = GetAlarmLogElement(i);

    pWriter->WriteU8((U8)p_alarm_event->GetAlarmId());
    pWriter->WriteU8((U8)p_alarm_event->GetAlarmType());
    pWriter->WriteBool(p_alarm_event->GetAcknowledge());

    pWriter->WriteMpcTime(p_alarm_event->GetArrivalTime());
    pWriter->WriteMpcTime(p_alarm_event->GetDepartureTime());

    pWriter->WriteU8((U8)p_alarm_event->GetErroneousUnit());
    pWriter->WriteI16((I16)(I32)p_alarm_event->GetErroneousUnitNumber());
    pWriter->WriteU32(p_alarm_event->GetErrorSourceSubjectId());

    pWriter->WriteBool(p_alarm_event->GetSmsArrivalSent());
    pWriter->WriteBool(p_alarm_event->GetSmsDepartureSent());

    const AlarmLogDataHistory::DataHistoryEntry& snap_shot_data = p_alarm_event->GetSnapShotData();

    pWriter->WriteBool (    snap_shot_data.mSurfaceLevelReady       );
    pWriter->WriteU8   ((U8)snap_shot_data.mSurfaceLevelReadyQuality);

    pWriter->WriteFloat(    snap_shot_data.mSurfaceLevel            );
    pWriter->WriteU8   ((U8)snap_shot_data.mSurfaceLevelQuality     );

    pWriter->WriteFloat(    snap_shot_data.mAverageFlow             );
    pWriter->WriteU8   ((U8)snap_shot_data.mAverageFlowQuality      );

    ERRONEOUS_UNIT_TYPE erroneous_unit_type = p_alarm_event->GetErroneousUnit();
    int pump_error_index = -1;

    if (erroneous_unit_type == ERRONEOUS_UNIT_MP204 ||
        erroneous_unit_type == ERRONEOUS_UNIT_IO111 ||
        erroneous_unit_type == ERRONEOUS_UNIT_PUMP  ||
        erroneous_unit_type == ERRONEOUS_UNIT_CUE   )
    {
      pump_error_index = p_alarm_event->GetErroneousUnitNumber() - 1;
    }
    bool is_a_pump_alarm = (pump_error_index >= 0 && pump_error_index < NO_OF_PUMPS);

    if (is_a_pump_alarm)
    {
      pWriter->WriteFloat(    snap_shot_data.mPumpVoltage             [pump_error_index]);
      pWriter->WriteU8   ((U8)snap_shot_data.mPumpVoltageQuality      [pump_error_index]);

      pWriter->WriteFloat(    snap_shot_data.mPumpCurrent             [pump_error_index]);
      pWriter->WriteU8   ((U8)snap_shot_data.mPumpCurrentQuality      [pump_error_index]);

      pWriter->WriteFloat(    snap_shot_data.mPumpCosPhi              [pump_error_index]);
      pWriter->WriteU8   ((U8)snap_shot_data.mPumpCosPhiQuality       [pump_error_index]);

      pWriter->WriteFloat(    snap_shot_data.mPumpPower               [pump_error_index]);
      pWriter->WriteU8   ((U8)snap_shot_data.mPumpPowerQuality        [pump_error_index]);

      pWriter->WriteFloat(    snap_shot_data.mPumpFlow                [pump_error_index]);
      pWriter->WriteU8   ((U8)snap_shot_data.mPumpFlowQuality         [pump_error_index]);

      pWriter->WriteFloat(    snap_shot_data.mPumpTemperature         [pump_error_index]);
      pWriter->WriteU8   ((U8)snap_shot_data.mPumpTemperatureQuality  [pump_error_index]);
    }
    else
    {
      pWriter->WriteFloat(0.0);
      pWriter->WriteU8   ((U8)DP_NEVER_AVAILABLE);

      pWriter->WriteFloat(0.0);
      pWriter->WriteU8   ((U8)DP_NEVER_AVAILABLE);

      pWriter->WriteFloat(0.0);
      pWriter->WriteU8   ((U8)DP_NEVER_AVAILABLE);

      pWriter->WriteFloat(0.0);
      pWriter->WriteU8   ((U8)DP_NEVER_AVAILABLE);

      pWriter->WriteFloat(0.0);
      pWriter->WriteU8   ((U8)DP_NEVER_AVAILABLE);

      pWriter->WriteFloat(0.0);
      pWriter->WriteU8   ((U8)DP_NEVER_AVAILABLE);
    }

    for (unsigned int i = 0; i < NO_OF_PUMPS; i++)
    {
      pWriter->WriteU8((U8)snap_shot_data.mPumpOperatingMode        [i]);
      pWriter->WriteU8((U8)snap_shot_data.mPumpOperatingModeQuality [i]);
    }
    
    pWriter->WriteBool(p_alarm_event->GetWatchSmsArrivalSent());
    pWriter->WriteBool(p_alarm_event->GetWatchSmsDepartureSent());
  }

  // un-lock access
  UnuseAlarmLog();
}

/*****************************************************************************
* FUNCTION - LoadFromFlash
* DESCRIPTION:
*****************************************************************************/
void AlarmLog::LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
{
  // read version and size
  const U8 version = pReader->ReadU8(0);
	const I32 alarmLogSize = pReader->ReadI32(0);

  // read alarm log if known version and size
  if ((version == 1 || version == 2 || version == 3) && (alarmLogSize == ALARM_LOG_SIZE))
  {
    AlarmEvent* p_alarm_event;

    // lock access
    UseAlarmLog();

    // load elements
    for (int i = 0; i < ALARM_LOG_SIZE; i++)
    {
      p_alarm_event = GetAlarmLogElement(i);

      if (version == 1)
      {
        LoadFromFlashLogVersion1(pReader, p_alarm_event);
      }
      else if (version == 2)
      {
        LoadFromFlashLogVersion2(pReader, p_alarm_event);
      }
      else
      {
        LoadFromFlashLogVersion3(pReader, p_alarm_event);
      }
    }

    // un-lock access
    UnuseAlarmLog();
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
* FUNCTION - LoadFromFlashLogVersion2
* DESCRIPTION:
*****************************************************************************/
void AlarmLog::LoadFromFlashLogVersion1(IFlashReader* pReader, AlarmEvent* pAlarmEvent)
{
  pAlarmEvent->SetAlarmId((ALARM_ID_TYPE)pReader->ReadI32((I32)pAlarmEvent->GetAlarmId()));
  pAlarmEvent->SetAlarmType((ALARM_STATE_TYPE)pReader->ReadI32((I32)pAlarmEvent->GetAlarmType()));
  pAlarmEvent->SetAcknowledge(pReader->ReadBool(pAlarmEvent->GetAcknowledge()));

  pReader->ReadMpcTime(pAlarmEvent->GetArrivalTime());
  pReader->ReadMpcTime(pAlarmEvent->GetDepartureTime());

  pAlarmEvent->SetErroneousUnit((ERRONEOUS_UNIT_TYPE)pReader->ReadI32(pAlarmEvent->GetErroneousUnit()));
  pAlarmEvent->SetErroneousUnitNumber(pReader->ReadI32(pAlarmEvent->GetErroneousUnitNumber()));
  pAlarmEvent->SetErrorSource(pReader->ReadU32(pAlarmEvent->GetErrorSourceSubjectId()));

  pAlarmEvent->SetSmsArrivalSent(pReader->ReadBool(pAlarmEvent->GetSmsArrivalSent()));
  pAlarmEvent->SetSmsDepartureSent(pReader->ReadBool(pAlarmEvent->GetSmsDepartureSent()));
}

/*****************************************************************************
* FUNCTION - LoadFromFlashLogVersion2
* DESCRIPTION:
*****************************************************************************/
void AlarmLog::LoadFromFlashLogVersion2(IFlashReader* pReader, AlarmEvent* pAlarmEvent)
{
  pAlarmEvent->SetAlarmId((ALARM_ID_TYPE)pReader->ReadU8((U8)pAlarmEvent->GetAlarmId()));
  pAlarmEvent->SetAlarmType((ALARM_STATE_TYPE)pReader->ReadU8((U8)pAlarmEvent->GetAlarmType()));
  pAlarmEvent->SetAcknowledge(pReader->ReadBool(pAlarmEvent->GetAcknowledge()));

  pReader->ReadMpcTime(pAlarmEvent->GetArrivalTime());
  pReader->ReadMpcTime(pAlarmEvent->GetDepartureTime());

  pAlarmEvent->SetErroneousUnit((ERRONEOUS_UNIT_TYPE)pReader->ReadU8((U8)pAlarmEvent->GetErroneousUnit()));
  pAlarmEvent->SetErroneousUnitNumber((I32)pReader->ReadI16((I16)(I32)pAlarmEvent->GetErroneousUnitNumber()));
  pAlarmEvent->SetErrorSource(pReader->ReadU32(pAlarmEvent->GetErrorSourceSubjectId()));

  pAlarmEvent->SetSmsArrivalSent(pReader->ReadBool(pAlarmEvent->GetSmsArrivalSent()));
  pAlarmEvent->SetSmsDepartureSent(pReader->ReadBool(pAlarmEvent->GetSmsDepartureSent()));

  AlarmLogDataHistory::DataHistoryEntry snap_shot_data;

  snap_shot_data.mSurfaceLevelReady        =                  pReader->ReadBool (    snap_shot_data.mSurfaceLevelReady       );
  snap_shot_data.mSurfaceLevelReadyQuality = (DP_QUALITY_TYPE)pReader->ReadU8   ((U8)snap_shot_data.mSurfaceLevelReadyQuality);

  snap_shot_data.mSurfaceLevel             =                  pReader->ReadFloat(    snap_shot_data.mSurfaceLevel            );
  snap_shot_data.mSurfaceLevelQuality      = (DP_QUALITY_TYPE)pReader->ReadU8   ((U8)snap_shot_data.mSurfaceLevelQuality     );

  snap_shot_data.mAverageFlow              =                  pReader->ReadFloat(    snap_shot_data.mAverageFlow             );
  snap_shot_data.mAverageFlowQuality       = (DP_QUALITY_TYPE)pReader->ReadU8   ((U8)snap_shot_data.mAverageFlowQuality      );

  for (unsigned int i = 0; i < NO_OF_PUMPS; i++)
  {
    snap_shot_data.mPumpVoltage              [i] = 0.0;
    snap_shot_data.mPumpCurrent              [i] = 0.0;
    snap_shot_data.mPumpCosPhi               [i] = 0.0;
    snap_shot_data.mPumpPower                [i] = 0.0;
    snap_shot_data.mPumpFlow                 [i] = 0.0;
    snap_shot_data.mPumpTemperature          [i] = 0.0;

    snap_shot_data.mPumpVoltageQuality       [i] = DP_NEVER_AVAILABLE;
    snap_shot_data.mPumpCurrentQuality       [i] = DP_NEVER_AVAILABLE;
    snap_shot_data.mPumpCosPhiQuality        [i] = DP_NEVER_AVAILABLE;
    snap_shot_data.mPumpPowerQuality         [i] = DP_NEVER_AVAILABLE;
    snap_shot_data.mPumpFlowQuality          [i] = DP_NEVER_AVAILABLE;
    snap_shot_data.mPumpTemperatureQuality   [i] = DP_NEVER_AVAILABLE;
  }

  ERRONEOUS_UNIT_TYPE erroneous_unit_type = pAlarmEvent->GetErroneousUnit();
  int pump_error_index = -1;

  if (erroneous_unit_type == ERRONEOUS_UNIT_MP204 ||
      erroneous_unit_type == ERRONEOUS_UNIT_IO111 ||
      erroneous_unit_type == ERRONEOUS_UNIT_PUMP  ||
      erroneous_unit_type == ERRONEOUS_UNIT_CUE   )
  {
    pump_error_index = pAlarmEvent->GetErroneousUnitNumber() - 1;
  }
  bool is_a_pump_alarm = (pump_error_index >= 0 && pump_error_index < NO_OF_PUMPS);

  if (is_a_pump_alarm)
  {
    snap_shot_data.mPumpVoltage              [pump_error_index] =                  pReader->ReadFloat(    snap_shot_data.mPumpVoltage              [pump_error_index]);
    snap_shot_data.mPumpVoltageQuality       [pump_error_index] = (DP_QUALITY_TYPE)pReader->ReadU8   ((U8)snap_shot_data.mPumpVoltageQuality       [pump_error_index]);

    snap_shot_data.mPumpCurrent              [pump_error_index] =                  pReader->ReadFloat(    snap_shot_data.mPumpCurrent              [pump_error_index]);
    snap_shot_data.mPumpCurrentQuality       [pump_error_index] = (DP_QUALITY_TYPE)pReader->ReadU8   ((U8)snap_shot_data.mPumpCurrentQuality       [pump_error_index]);

    snap_shot_data.mPumpCosPhi               [pump_error_index] =                  pReader->ReadFloat(    snap_shot_data.mPumpCosPhi               [pump_error_index]);
    snap_shot_data.mPumpCosPhiQuality        [pump_error_index] = (DP_QUALITY_TYPE)pReader->ReadU8   ((U8)snap_shot_data.mPumpCosPhiQuality        [pump_error_index]);

    snap_shot_data.mPumpPower                [pump_error_index] =                  pReader->ReadFloat(    snap_shot_data.mPumpPower                [pump_error_index]);
    snap_shot_data.mPumpPowerQuality         [pump_error_index] = (DP_QUALITY_TYPE)pReader->ReadU8   ((U8)snap_shot_data.mPumpPowerQuality         [pump_error_index]);

    snap_shot_data.mPumpFlow                 [pump_error_index] =                  pReader->ReadFloat(    snap_shot_data.mPumpFlow                 [pump_error_index]);
    snap_shot_data.mPumpFlowQuality          [pump_error_index] = (DP_QUALITY_TYPE)pReader->ReadU8   ((U8)snap_shot_data.mPumpFlowQuality          [pump_error_index]);

    snap_shot_data.mPumpTemperature          [pump_error_index] =                  pReader->ReadFloat(    snap_shot_data.mPumpTemperature          [pump_error_index]);
    snap_shot_data.mPumpTemperatureQuality   [pump_error_index] = (DP_QUALITY_TYPE)pReader->ReadU8   ((U8)snap_shot_data.mPumpTemperatureQuality   [pump_error_index]);
  }
  else
  {
    //Throw away

    pReader->ReadFloat(0.0);
    pReader->ReadU8   (0);

    pReader->ReadFloat(0.0);
    pReader->ReadU8   (0);

    pReader->ReadFloat(0.0);
    pReader->ReadU8   (0);

    pReader->ReadFloat(0.0);
    pReader->ReadU8   (0);

    pReader->ReadFloat(0.0);
    pReader->ReadU8   (0);

    pReader->ReadFloat(0.0);
    pReader->ReadU8   (0);
  }

  for (unsigned int i = 0; i < NO_OF_PUMPS; i++)
  {
    snap_shot_data.mPumpOperatingMode       [i] = (ACTUAL_OPERATION_MODE_TYPE) pReader->ReadU8((U8)snap_shot_data.mPumpOperatingMode       [i]);
    snap_shot_data.mPumpOperatingModeQuality[i] = (DP_QUALITY_TYPE)            pReader->ReadU8((U8)snap_shot_data.mPumpOperatingModeQuality[i]);
  }

  pAlarmEvent->SetSnapShotData(snap_shot_data);
}

/*****************************************************************************
* FUNCTION - LoadFromFlashLogVersion3
* DESCRIPTION:
*****************************************************************************/
void AlarmLog::LoadFromFlashLogVersion3(IFlashReader* pReader, AlarmEvent* pAlarmEvent)
{
  LoadFromFlashLogVersion2(pReader, pAlarmEvent);
  pAlarmEvent->SetWatchSmsArrivalSent(pReader->ReadBool(pAlarmEvent->GetWatchSmsArrivalSent()));
  pAlarmEvent->SetWatchSmsDepartureSent(pReader->ReadBool(pAlarmEvent->GetWatchSmsDepartureSent()));
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
