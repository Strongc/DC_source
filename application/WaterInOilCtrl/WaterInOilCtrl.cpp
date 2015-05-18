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
/* CLASS NAME       : WaterInOilCtrl                                        */
/*                                                                          */
/* FILE NAME        : WaterInOilCtrl.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
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
#include <WaterInOilCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

 #define ONE_SECOND_COUNT      1000/10

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
WaterInOilCtrl::WaterInOilCtrl()
{
  mOneSecondCounter = 0;
  mOneSecondAccumulator = 0;
  mStartDelayCounter = 0;
  mAveragingCounter = 0;
  mAveragingAccumulator = 0;
  mSensorConnected = false;
  mWioLimitAlarmAllowed = false;

  // Create objects for handling setting, clearing and delaying of alarms and warnings
  for (int fault_id = FIRST_WIO_ALARM_OBJ; fault_id < NO_OF_WIO_ALARM_OBJ; fault_id++)
  {
    mpWioAlarmDelay[fault_id] = new AlarmDelay(this);
    mWioAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
WaterInOilCtrl::~WaterInOilCtrl()
{
  for (int fault_id = FIRST_WIO_ALARM_OBJ; fault_id < NO_OF_WIO_ALARM_OBJ; fault_id++)
  {
    delete(mpWioAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void WaterInOilCtrl::InitSubTask()
{
  for (int fault_id = FIRST_WIO_ALARM_OBJ; fault_id < NO_OF_WIO_ALARM_OBJ; fault_id++)
  {
    mpWioAlarmDelay[fault_id]->InitAlarmDelay();
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void WaterInOilCtrl::RunSubTask()
{
  mOneSecondCounter++;
  mOneSecondAccumulator += mpMeasuredValueWaterInOil->GetValue();

  if (mOneSecondCounter >= ONE_SECOND_COUNT)
  {
    // Pre handling of alarms
    for (int fault_id = FIRST_WIO_ALARM_OBJ; fault_id < NO_OF_WIO_ALARM_OBJ; fault_id++)
    {
      if (mWioAlarmDelayCheckFlag[fault_id] == true)
      {
        mWioAlarmDelayCheckFlag[fault_id] = false;
        mpWioAlarmDelay[fault_id]->CheckErrorTimers();
      }
    }

    // Do the real stuff. (must be called every second)
    CheckWioSensorPresent();
    HandleWio(mOneSecondAccumulator/mOneSecondCounter);
    CheckWioLimits();
    mOneSecondCounter = 0;
    mOneSecondAccumulator = 0;

    // Post handling of alarms
    for (int fault_id = FIRST_WIO_ALARM_OBJ; fault_id < NO_OF_WIO_ALARM_OBJ; fault_id++)
    {
      mpWioAlarmDelay[fault_id]->UpdateAlarmDataPoint();
    }

    // Update resulting value
    if (mpFilteredValueWaterInOil->GetQuality() != DP_NEVER_AVAILABLE)
    {
      mpResultingValueWaterInOil->CopyValues(mpFilteredValueWaterInOil.GetSubject());
    }
    else
    {
      // If present, use the IO111 value instead (no need to check if present since CopyValues gives the quality as well)
      mpResultingValueWaterInOil->CopyValues(mpIO111WaterInOil.GetSubject());
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void WaterInOilCtrl::ConnectToSubjects()
{
  for (int fault_id = FIRST_WIO_ALARM_OBJ; fault_id < NO_OF_WIO_ALARM_OBJ; fault_id++)
  {
    mpWioAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void WaterInOilCtrl::Update(Subject* pSubject)
{
  for (int fault_id = FIRST_WIO_ALARM_OBJ; fault_id < NO_OF_WIO_ALARM_OBJ; fault_id++)
  {
    if (pSubject == mpWioAlarmDelay[fault_id])
    {
      mWioAlarmDelayCheckFlag[fault_id] = true;
      break;
    }
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void WaterInOilCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_WIO_ALARM_OBJ; fault_id < NO_OF_WIO_ALARM_OBJ; fault_id++)
  {
    mpWioAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void WaterInOilCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_WIO_DELAY_AFTER_START:
      mpDelayAfterStart.Attach(pSubject);
      break;
    case SP_WIO_AVERAGING_TIME:
      mpAveragingTime.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_WIO_MEASURED_VALUE_WATER_IN_OIL:
      mpMeasuredValueWaterInOil.Attach(pSubject);
      break;
    case SP_WIO_IO111_WATER_IN_OIL_VALUE:
      mpIO111WaterInOil.Attach(pSubject);
      break;
    case SP_WIO_OPERATION_MODE_ACTUAL:
      mpActualOperationMode.Attach(pSubject);
      break;

    // Outputs:
    case SP_WIO_FILTERED_VALUE_WATER_IN_OIL:
      mpFilteredValueWaterInOil.Attach(pSubject);
      break;
    case SP_WIO_RESULTING_VALUE_WATER_IN_OIL:
      mpResultingValueWaterInOil.Attach(pSubject);
      break;

    // Alarms
    case SP_WIO_WATER_IN_OIL_LIMIT_ALARM_OBJ:
      mpWioAlarmObj[WIO_ALARM_OBJ_LIMIT].Attach(pSubject);
      mpWioAlarmDelay[WIO_ALARM_OBJ_LIMIT]->SetSubjectPointer(id, pSubject);
      break;
    case SP_WIO_WATER_IN_OIL_SENSOR_ALARM_OBJ:
      mpWioAlarmObj[WIO_ALARM_OBJ_SENSOR].Attach(pSubject);
      mpWioAlarmDelay[WIO_ALARM_OBJ_SENSOR]->SetSubjectPointer(id, pSubject);
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
 * Function - CheckWioSensorPresent
 * DESCRIPTION: Check if the WIO sensor is connected and configured
 *
 * NOTE:  Only the low limit electrical error is used
 *
 *****************************************************************************/
void WaterInOilCtrl::CheckWioSensorPresent()
{
  if (mpMeasuredValueWaterInOil->GetQuality() == DP_NEVER_AVAILABLE)
  {
    // Wio input is not configured. Make sure that alarms and warnings are removed.
    mpWioAlarmDelay[WIO_ALARM_OBJ_SENSOR]->ResetFault();
    mpWioAlarmDelay[WIO_ALARM_OBJ_LIMIT]->ResetWarning();
    mpWioAlarmDelay[WIO_ALARM_OBJ_LIMIT]->ResetFault();
    mpFilteredValueWaterInOil->SetQuality(DP_NEVER_AVAILABLE);
    mSensorConnected = false;
  }
  else if (mpMeasuredValueWaterInOil->GetQuality() == DP_NOT_AVAILABLE && mpMeasuredValueWaterInOil->GetValueAsPercent() < 10.0f)
  {
    // Electrical error and low ad value. The sensor must be disconnected.
    mpFilteredValueWaterInOil->SetQuality(DP_NOT_AVAILABLE);
    mpWioAlarmDelay[WIO_ALARM_OBJ_SENSOR]->SetFault();
    mSensorConnected = false;
  }
  else if (mSensorConnected == false)
  {
    // The sensor has been connected
    mpFilteredValueWaterInOil->SetQuality(DP_NOT_AVAILABLE);
    mpWioAlarmDelay[WIO_ALARM_OBJ_SENSOR]->ResetFault();
    mSensorConnected = true;
  }
}

/*****************************************************************************
 * Function - HandleWioSensor
 * DESCRIPTION: Calculate a filtered WIO value
 *
 * NOTE: Must be called every second
 *
 *****************************************************************************/
void WaterInOilCtrl::HandleWio(float wio_value)
{
  // Do nothing unless the pump has been started for a while
  if (mpActualOperationMode->GetValue() != ACTUAL_OPERATION_MODE_STARTED)
  {
    mWioLimitAlarmAllowed = false;
    mStartDelayCounter = 0;
  }
  else if (mStartDelayCounter < mpDelayAfterStart->GetValue())
  {
    mStartDelayCounter++;
    mAveragingCounter = 0;
    mAveragingAccumulator = 0;
  }
  else if (mSensorConnected == false)
  {
    // Prepare for new averaging when the sensor is ok again
    mAveragingCounter = 0;
    mAveragingAccumulator = 0;
  }
  else // Start delay has elapsed and sensor is connected, now do some more averaging
  {
    if (mpMeasuredValueWaterInOil->GetQuality() == DP_NOT_AVAILABLE)
    {
      // Sensor must be above 21 mA. This is a special situation, see the WIO sensor specification
      // Since this is could be caused by a very high water rate, just use the max sensor value as wio value
      wio_value = mpMeasuredValueWaterInOil->GetMaxValue();
    }
    mAveragingCounter++;
    mAveragingAccumulator += wio_value;
    if (mAveragingCounter >= mpAveragingTime->GetValue())
    {
      mWioLimitAlarmAllowed = true;
      mpFilteredValueWaterInOil->SetValue(mAveragingAccumulator/mAveragingCounter);
      mAveragingCounter = 0;
      mAveragingAccumulator = 0;
    }
  }
}

/*****************************************************************************
 * Function - CheckWioLimits
 * DESCRIPTION: Update the WIO limit warning/alarm
 *
 *****************************************************************************/
void WaterInOilCtrl::CheckWioLimits()
{
  if (mSensorConnected == true)
  {
    float wio_value = mpFilteredValueWaterInOil->GetValue();

    // WIO warning, no matter if the pump is stopped or running
    if (wio_value >= mpWioAlarmObj[WIO_ALARM_OBJ_LIMIT]->GetAlarmConfig()->GetWarningLimit()->GetAsFloat())
    {
      mpWioAlarmDelay[WIO_ALARM_OBJ_LIMIT]->SetWarning();
    }
    else
    {
      mpWioAlarmDelay[WIO_ALARM_OBJ_LIMIT]->ResetWarning();
    }

    // WIO alarm, only when pump has been running for a while
    if ( (mWioLimitAlarmAllowed == true)
      && (wio_value >= mpWioAlarmObj[WIO_ALARM_OBJ_LIMIT]->GetAlarmConfig()->GetAlarmLimit()->GetAsFloat()) )
    {
      mpWioAlarmDelay[WIO_ALARM_OBJ_LIMIT]->SetFault();
    }
    else
    {
      mpWioAlarmDelay[WIO_ALARM_OBJ_LIMIT]->ResetFault();
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
