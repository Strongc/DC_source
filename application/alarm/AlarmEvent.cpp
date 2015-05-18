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
/* CLASS NAME       : AlarmEvent                                            */
/*                                                                          */
/* FILE NAME        : AlarmEvent.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 17-09-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
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
#include <AlarmEvent.h>

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
 * DESCRIPTION: Initialises private data.
 *
 *****************************************************************************/
AlarmEvent::AlarmEvent(U32 alarmEventNumber)
{
  mAlarmEventNumber = alarmEventNumber;
  mAlarmId = ALARM_ID_NO_ALARM;
  mAlarmType = ALARM_STATE_READY;
  mAcknowledge = true;
  SetArrivalTimeNotAvailable();
  SetDepartureTimeNotAvailable();
  mErroneousUnit = ERRONEOUS_UNIT_CU361;
  mErroneousUnitNumber = 0;
  mSmsArrivalSent=false;
  mSmsDepartureSent=false;
  mWatchSmsArrivalSent=false;
  mWatchSmsDepartureSent=false;

  mSnapShotData.mSurfaceLevelReady = false;
  mSnapShotData.mSurfaceLevel      = 0.0;
  mSnapShotData.mAverageFlow       = 0.0;

  mSnapShotData.mSurfaceLevelReadyQuality = DP_NEVER_AVAILABLE;
  mSnapShotData.mSurfaceLevelQuality      = DP_NEVER_AVAILABLE;
  mSnapShotData.mAverageFlowQuality       = DP_NEVER_AVAILABLE;

  for (unsigned int i = 0; i < NO_OF_PUMPS; ++i)
  {
    mSnapShotData.mPumpOperatingMode[i] = FIRST_ACTUAL_OPERATION_MODE;
    mSnapShotData.mPumpVoltage[i]       = 0.0;
    mSnapShotData.mPumpCurrent[i]       = 0.0;
    mSnapShotData.mPumpCosPhi[i]        = 0.0;
    mSnapShotData.mPumpPower[i]         = 0.0;
    mSnapShotData.mPumpFlow[i]          = 0.0;
    mSnapShotData.mPumpTemperature[i]   = 0.0;

    mSnapShotData.mPumpOperatingModeQuality[i] = DP_NEVER_AVAILABLE;
    mSnapShotData.mPumpVoltageQuality[i]       = DP_NEVER_AVAILABLE;
    mSnapShotData.mPumpCurrentQuality[i]       = DP_NEVER_AVAILABLE;
    mSnapShotData.mPumpCosPhiQuality[i]        = DP_NEVER_AVAILABLE;
    mSnapShotData.mPumpPowerQuality[i]         = DP_NEVER_AVAILABLE;
    mSnapShotData.mPumpFlowQuality[i]          = DP_NEVER_AVAILABLE;
    mSnapShotData.mPumpTemperatureQuality[i]   = DP_NEVER_AVAILABLE;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: -
 *
 ****************************************************************************/
AlarmEvent::~AlarmEvent()
{

}

/*****************************************************************************
 * Function - Assignment operator
 * DESCRIPTION: - param src The value to assign to this object.
 *                return A reference to this object.
 *
 ****************************************************************************/
 AlarmEvent& AlarmEvent::operator=(const AlarmEvent& src)
{
  if (this != &src)
  {
    mAlarmEventNumber = src.mAlarmEventNumber;
    mAlarmId = src.mAlarmId;
    mAlarmType = src.mAlarmType;
    mAcknowledge = src.mAcknowledge;
    mArrivalTime = src.mArrivalTime;
    mDepartureTime = src.mDepartureTime;
    mErroneousUnit = src.mErroneousUnit;
    mErroneousUnitNumber = src.mErroneousUnitNumber;
  }
  return *this;
}// =

/*****************************************************************************
 * Function - GetAlarmEventNumber
 * DESCRIPTION: Returns mAlarmEventNumber.
 *
 *****************************************************************************/
U32 AlarmEvent::GetAlarmEventNumber()
{
  return mAlarmEventNumber;
}

/*****************************************************************************
 * Function - GetAlarmId
 * DESCRIPTION: Returns mAlarmId.
 *
 *****************************************************************************/
ALARM_ID_TYPE AlarmEvent::GetAlarmId()
{
  return mAlarmId;
}

/*****************************************************************************
 * Function - GetAlarmType
 * DESCRIPTION: Returns mAlarmType.
 *
 *****************************************************************************/
ALARM_STATE_TYPE AlarmEvent::GetAlarmType()
{
  return mAlarmType;
}

/*****************************************************************************
 * Function - GetAcknowledge
 * DESCRIPTION: Returns mAcknowledge.
 *
 *****************************************************************************/
bool AlarmEvent::GetAcknowledge()
{
  return mAcknowledge;
}

/*****************************************************************************
 * Function - GetArrivalTime
 * DESCRIPTION: Returns mArrivalTime.
 *
 *****************************************************************************/
MpcTime* AlarmEvent::GetArrivalTime()
{
  return &mArrivalTime;
}

/*****************************************************************************
 * Function - GetDepartureTime
 * DESCRIPTION: Returns mDepartureTime.
 *
 *****************************************************************************/
MpcTime* AlarmEvent::GetDepartureTime()
{
  return &mDepartureTime;
}

/*****************************************************************************
 * Function - GetErroneousUnit
 * DESCRIPTION: Returns mErroneousUnit.
 *
 *****************************************************************************/
ERRONEOUS_UNIT_TYPE AlarmEvent::GetErroneousUnit()
{
  return mErroneousUnit;
}

/*****************************************************************************
 * Function - GetErroneousUnitNumber
 * DESCRIPTION: Returns mErroneousUnitNumber.
 *
 *****************************************************************************/
int AlarmEvent::GetErroneousUnitNumber()
{
  return mErroneousUnitNumber;
}

/*****************************************************************************
 * Function - GetErrorSource
 * DESCRIPTION: Returns mpErrorSource.
 *
 *****************************************************************************/
AlarmDataPoint* AlarmEvent::GetErrorSource()
{
  Subject* sub = GetSubject( mAlarmDataPointSubjectId );

  AlarmDataPoint * pa;
  pa = dynamic_cast<AlarmDataPoint*>(sub);
  return pa;
}

/*****************************************************************************
 * Function - GetErrorSourceSubjectId
 * DESCRIPTION:
 *
 *****************************************************************************/
U32 AlarmEvent::GetErrorSourceSubjectId()
{
  return mAlarmDataPointSubjectId;
}


/*****************************************************************************
 * Function - SetAlarmId
 * DESCRIPTION: Sets mAlarmId to alarmId.
 *
 *****************************************************************************/
void AlarmEvent::SetAlarmId(ALARM_ID_TYPE alarmId)
{
  mAlarmId = alarmId;
}

/*****************************************************************************
 * Function - SetAlarmType
 * DESCRIPTION: Sets mAlarmType to alarmType.
 *
 *****************************************************************************/
void AlarmEvent::SetAlarmType(ALARM_STATE_TYPE alarmType)
{
  mAlarmType = alarmType;
}

/*****************************************************************************
 * Function - SetAcknowledge
 * DESCRIPTION: Sets mAcknowledge to acknowledge.
 *
 *****************************************************************************/
void AlarmEvent::SetAcknowledge(bool acknowledge)
{
  mAcknowledge = acknowledge;
}

/*****************************************************************************
 * Function - SetArrivalTime
 * DESCRIPTION: Sets mArrivalTime by calling UpdateTimeObj.
 *
 *****************************************************************************/
void AlarmEvent::SetArrivalTime()
{
  ActTime::GetInstance()->UpdateTimeObj(&mArrivalTime);
}

/*****************************************************************************
 * Function - SetArrivalTime
 * DESCRIPTION: Sets mArrivalTime with pArrivalTime.
 *
 *****************************************************************************/
void AlarmEvent::SetArrivalTime(MpcTime* pArrivalTime)
{
  mArrivalTime = *pArrivalTime;
}

/*****************************************************************************
 * Function - SetArrivalTimeToNotAvailable
 * DESCRIPTION: Sets mArrivalTime to not available.
 *
 *****************************************************************************/
void AlarmEvent::SetArrivalTimeNotAvailable()
{
  mArrivalTime.SetDate(YEAR, INVALID_DATE);
  mArrivalTime.SetDate(MONTH, INVALID_DATE);
  mArrivalTime.SetDate(DAY, INVALID_DATE);
  mArrivalTime.SetTime(HOURS, INVALID_TIME);
  mArrivalTime.SetTime(MINUTES, INVALID_TIME);
  mArrivalTime.SetTime(SECONDS, INVALID_TIME);
}

/*****************************************************************************
 * Function - SetDepartureTime
 * DESCRIPTION: Sets mDepartureTime by calling UpdateTimeObj.
 *
 *****************************************************************************/
void AlarmEvent::SetDepartureTime()
{
  ActTime::GetInstance()->UpdateTimeObj(&mDepartureTime);
}

/*****************************************************************************
 * Function - SetDepartureTime
 * DESCRIPTION: Sets mDepartureTime with pDepartureTime.
 *
 *****************************************************************************/
void AlarmEvent::SetDepartureTime(MpcTime* pDepartureTime)
{
  mDepartureTime = *pDepartureTime;
}

/*****************************************************************************
 * Function - SetDepartureTimeToNotAvailable
 * DESCRIPTION: Sets mDepartureTime to not available.
 *
 *****************************************************************************/
void AlarmEvent::SetDepartureTimeNotAvailable()
{
  mDepartureTime.SetDate(YEAR, INVALID_DATE);
  mDepartureTime.SetDate(MONTH, INVALID_DATE);
  mDepartureTime.SetDate(DAY, INVALID_DATE);
  mDepartureTime.SetTime(HOURS, INVALID_TIME);
  mDepartureTime.SetTime(MINUTES, INVALID_TIME);
  mDepartureTime.SetTime(SECONDS, INVALID_TIME);
}

/*****************************************************************************
 * Function - SetErroneousUnit
 * DESCRIPTION: Sets mErroneousUnit to erroneousUnit.
 *
 *****************************************************************************/
void AlarmEvent::SetErroneousUnit(ERRONEOUS_UNIT_TYPE erroneousUnit)
{
  mErroneousUnit = erroneousUnit;
}

/*****************************************************************************
 * Function - SetErroneousUnitNumber
 * DESCRIPTION: Sets mErroneousUnitNumber to erroneousUnitNumber.
 *
 *****************************************************************************/
void AlarmEvent::SetErroneousUnitNumber(int erroneousUnitNumber)
{
  mErroneousUnitNumber = erroneousUnitNumber;
}

/*****************************************************************************
 * Function - SetErrorSource
 * DESCRIPTION: Sets mpErrorSource to pErrorSource.
 *
 *****************************************************************************/
void AlarmEvent::SetErrorSource(int errorSourceSubject)
{
  mAlarmDataPointSubjectId = errorSourceSubject;
}

/*****************************************************************************
 * Function - SetSmsArrivalSent
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmEvent::SetSmsArrivalSent(bool sent)
{
  mSmsArrivalSent=sent;
}

/*****************************************************************************
 * Function - GetSmsArrivalSent
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AlarmEvent::GetSmsArrivalSent(void)
{
  return mSmsArrivalSent;
}

/*****************************************************************************
 * Function - SetSmsDepartureSent
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmEvent::SetSmsDepartureSent(bool sent)
{
  mSmsDepartureSent=sent;
}

/*****************************************************************************
 * Function - GetSmsDepartureSent
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AlarmEvent::GetSmsDepartureSent(void)
{
  return mSmsDepartureSent;
}

/*****************************************************************************
 * Function - SetWatchSmsArrivalSent
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmEvent::SetWatchSmsArrivalSent(bool sent)
{
  mWatchSmsArrivalSent=sent;
}

/*****************************************************************************
 * Function - GetWatchSmsArrivalSent
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AlarmEvent::GetWatchSmsArrivalSent(void)
{
  return mWatchSmsArrivalSent;
}

/*****************************************************************************
 * Function - SetWatchSmsDepartureSent
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmEvent::SetWatchSmsDepartureSent(bool sent)
{
  mWatchSmsDepartureSent=sent;
}

/*****************************************************************************
 * Function - GetWatchSmsDepartureSent
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AlarmEvent::GetWatchSmsDepartureSent(void)
{
  return mWatchSmsDepartureSent;
}

void AlarmEvent::SetSnapShotData(const AlarmLogDataHistory::DataHistoryEntry& data)
{
  mSnapShotData = data;
}

const AlarmLogDataHistory::DataHistoryEntry& AlarmEvent::GetSnapShotData() const
{
  return mSnapShotData;
}


// bool AlarmEvent::GetSurfaceLevelReady() const
// {
//   return mSurfaceLevelReady;
// }

// float AlarmEvent::GetSurfaceLevel() const
// {
//   return mSurfaceLevel;
// }

// float AlarmEvent::GetAverageFlow() const
// {
//   return mAverageFlow;
// }

// ACTUAL_OPERATION_MODE_TYPE AlarmEvent::GetPumpOperatingMode() const
// {
//   return mPumpOperatingMode;
// }

// float AlarmEvent::GetPumpVoltage() const
// {
//   return mPumpVoltage;
// }

// float AlarmEvent::GetPumpCurrent() const
// {
//   return mPumpCurrent;
// }

// float AlarmEvent::GetPumpCosPhi() const
// {
//   return mPumpCosPhi;
// }

// float AlarmEvent::GetPumpPower() const
// {
//   return mPumpPower;
// }

// float AlarmEvent::GetPumpFlow() const
// {
//   return mPumpFlow;
// }

// float AlarmEvent::GetPumpTemperature() const
// {
//   return mPumpTemperature;
// }


// DP_QUALITY_TYPE AlarmEvent::GetSurfaceLevelReadyQuality() const
// {
//   return mSurfaceLevelReadyQuality;
// }

// DP_QUALITY_TYPE AlarmEvent::GetSurfaceLevelQuality() const
// {
//   return mSurfaceLevelQuality;
// }

// DP_QUALITY_TYPE AlarmEvent::GetAverageFlowQuality() const
// {
//   return mAverageFlowQuality;
// }

// DP_QUALITY_TYPE AlarmEvent::GetPumpOperatingModeQuality() const
// {
//   return mPumpOperatingModeQuality;
// }

// DP_QUALITY_TYPE AlarmEvent::GetPumpVoltageQuality() const
// {
//   return mPumpVoltageQuality;
// }

// DP_QUALITY_TYPE AlarmEvent::GetPumpCurrentQuality() const
// {
//   return mPumpCurrentQuality;
// }

// DP_QUALITY_TYPE AlarmEvent::GetPumpCosPhiQuality() const
// {
//   return mPumpCosPhiQuality;
// }

// DP_QUALITY_TYPE AlarmEvent::GetPumpPowerQuality() const
// {
//   return mPumpPowerQuality;
// }

// DP_QUALITY_TYPE AlarmEvent::GetPumpFlowQuality() const
// {
//   return mPumpFlowQuality;
// }

// DP_QUALITY_TYPE AlarmEvent::GetPumpTemperatureQuality() const
// {
//   return mPumpTemperatureQuality;
// }


// void AlarmEvent::SetSurfaceLevelReady(bool val)
// {
//   mSurfaceLevelReady = val;
// }

// void AlarmEvent::SetSurfaceLevel(float val)
// {
//   mSurfaceLevel = val;
// }

// void AlarmEvent::SetAverageFlow(float val)
// {
//   mAverageFlow = val;
// }

// void AlarmEvent::SetPumpOperatingMode(ACTUAL_OPERATION_MODE_TYPE val)
// {
//   mPumpOperatingMode = val;
// }

// void AlarmEvent::SetPumpVoltage(float val)
// {
//   mPumpVoltage = val;
// }

// void AlarmEvent::SetPumpCurrent(float val)
// {
//   mPumpCurrent = val;
// }

// void AlarmEvent::SetPumpCosPhi(float val)
// {
//   mPumpCosPhi = val;
// }

// void AlarmEvent::SetPumpPower(float val)
// {
//   mPumpPower = val;
// }

// void AlarmEvent::SetPumpFlow(float val)
// {
//   mPumpFlow = val;
// }

// void AlarmEvent::SetPumpTemperature(float val)
// {
//   mPumpTemperature = val;
// }


// void AlarmEvent::SetSurfaceLevelReadyQuality(DP_QUALITY_TYPE q)
// {
//   mSurfaceLevelReadyQuality = q;
// }

// void AlarmEvent::SetSurfaceLevelQuality(DP_QUALITY_TYPE q)
// {
//   mSurfaceLevelQuality = q;
// }

// void AlarmEvent::SetAverageFlowQuality(DP_QUALITY_TYPE q)
// {
//   mAverageFlowQuality = q;
// }

// void AlarmEvent::SetPumpOperatingModeQuality(DP_QUALITY_TYPE q)
// {
//   mPumpOperatingModeQuality = q;
// }

// void AlarmEvent::SetPumpVoltageQuality(DP_QUALITY_TYPE q)
// {
//   mPumpVoltageQuality = q;
// }

// void AlarmEvent::SetPumpCurrentQuality(DP_QUALITY_TYPE q)
// {
//   mPumpCurrentQuality = q;
// }

// void AlarmEvent::SetPumpCosPhiQuality(DP_QUALITY_TYPE q)
// {
//   mPumpCosPhiQuality = q;
// }

// void AlarmEvent::SetPumpPowerQuality(DP_QUALITY_TYPE q)
// {
//   mPumpPowerQuality = q;
// }

// void AlarmEvent::SetPumpFlowQuality(DP_QUALITY_TYPE q)
// {
//   mPumpFlowQuality = q;
// }

// void AlarmEvent::SetPumpTemperatureQuality(DP_QUALITY_TYPE q)
// {
//   mPumpTemperatureQuality = q;
// }





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
