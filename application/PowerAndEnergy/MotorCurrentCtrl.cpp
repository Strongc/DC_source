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
/* CLASS NAME       : MotorCurrentCtrl                                      */
/*                                                                          */
/* FILE NAME        : MotorCurrentCtrl.cpp                                  */
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
#include <MotorCurrentCtrl.h>

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
 * DESCRIPTION:
 *
 *****************************************************************************/
MotorCurrentCtrl::MotorCurrentCtrl(int pumpNo)
{
  mSkipRunCounter = 6-pumpNo;

  // Create objects for handling setting, clearing and delaying of alarms and warnings
  for (int fault_id = FIRST_MOC_ALARM_OBJ; fault_id < NO_OF_MOC_ALARM_OBJ; fault_id++)
  {
    mpMocAlarmDelay[fault_id] = new AlarmDelay(this);
    mMocAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
MotorCurrentCtrl::~MotorCurrentCtrl()
{
  for (int fault_id = FIRST_MOC_ALARM_OBJ; fault_id < NO_OF_MOC_ALARM_OBJ; fault_id++)
  {
    delete(mpMocAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void MotorCurrentCtrl::InitSubTask()
{
  mFilteredCurrentHasBeenCalculated = false;
  mStartDelayElapsed = false;
  mAwaitStartDelayCount = mpDelayAfterStart->GetValue();

  for (int fault_id = FIRST_MOC_ALARM_OBJ; fault_id < NO_OF_MOC_ALARM_OBJ; fault_id++)
  {
    mpMocAlarmDelay[fault_id]->InitAlarmDelay();
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void MotorCurrentCtrl::RunSubTask()
{
  mSkipRunCounter++;
  if (mSkipRunCounter % 10 != 0)
  {
    // To reduce CPU load, just run this subtask every 10th time i.e. every 100 ms instead of every 10 ms.
    return;
  }

  // Update resulting motor current value every 100 ms (CUE / MP204 / Analog input)
  if (mpCUECurrent->GetQuality() != DP_NEVER_AVAILABLE)
  {
    mpResultingValueCurrent->CopyValues(mpCUECurrent.GetSubject());
  }
  else if (mpMP204Current->GetQuality() != DP_NEVER_AVAILABLE)
  {
    mpResultingValueCurrent->CopyValues(mpMP204Current.GetSubject());
  }
  else
  {
    mpResultingValueCurrent->CopyValues(mpMeasuredValueCurrent.GetSubject());
  }

  if (mSkipRunCounter < 100)
  {
    // To reduce CPU load, just run the following code every 100th time i.e. every second
    return;
  }
  mSkipRunCounter = 0;

  // Pre handling of alarms
  for (int fault_id = FIRST_MOC_ALARM_OBJ; fault_id < NO_OF_MOC_ALARM_OBJ; fault_id++)
  {
    if (mMocAlarmDelayCheckFlag[fault_id] == true)
    {
      mMocAlarmDelayCheckFlag[fault_id] = false;
      mpMocAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  // These function must be called every second
  UpdateCurrent();
  CheckAlarms();

  // Post handling of alarms
  for (int fault_id = FIRST_MOC_ALARM_OBJ; fault_id < NO_OF_MOC_ALARM_OBJ; fault_id++)
  {
    mpMocAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void MotorCurrentCtrl::ConnectToSubjects()
{
  mpActualOperationMode->Subscribe(this);
  for (int fault_id = FIRST_MOC_ALARM_OBJ; fault_id < NO_OF_MOC_ALARM_OBJ; fault_id++)
  {
    mpMocAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void MotorCurrentCtrl::Update(Subject* pSubject)
{
  mpActualOperationMode.Update(pSubject);
  for (int fault_id = FIRST_MOC_ALARM_OBJ; fault_id < NO_OF_MOC_ALARM_OBJ; fault_id++)
  {
    if (pSubject == mpMocAlarmDelay[fault_id])
    {
      mMocAlarmDelayCheckFlag[fault_id] = true;
      break;
    }
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void MotorCurrentCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_MOC_ALARM_OBJ; fault_id < NO_OF_MOC_ALARM_OBJ; fault_id++)
  {
    mpMocAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void MotorCurrentCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_MOC_DELAY_AFTER_START:
      mpDelayAfterStart.Attach(pSubject);
      break;
    case SP_MOC_FILTER_TIME:
      mpFilterTime.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_MOC_MEASURED_VALUE_MOTOR_CURRENT:
      mpMeasuredValueCurrent.Attach(pSubject);
      break;
    case SP_MOC_CUE_CURRENT:
      mpCUECurrent.Attach(pSubject);
      break;
    case SP_MOC_MP204_CURRENT:
      mpMP204Current.Attach(pSubject);
      break;

    case SP_MOC_OPERATION_MODE_ACTUAL:
      mpActualOperationMode.Attach(pSubject);
      break;
    case SP_MOC_UNDER_LOWEST_STOP_LEVEL:
      mpUnderLowestStopLevel.Attach(pSubject);
      break;
    case SP_MOC_FOAM_DRAINING_REQUESTED:
      mpFoamDrainingRequested.Attach(pSubject);
      break;

    // Outputs:
    case SP_MOC_LATEST_VALUE_MOTOR_CURRENT:
      mpLatestValueCurrent.Attach(pSubject);
      break;
    case SP_MOC_FILTERED_VALUE_MOTOR_CURRENT:
      mpFilteredValueCurrent.Attach(pSubject);
      break;
    case SP_MOC_RESULTING_VALUE_MOTOR_CURRENT:
      mpResultingValueCurrent.Attach(pSubject);
      break;

    // Alarms
    case SP_MOC_MOTOR_CURRENT_OVERLOAD_ALARM_OBJ:
      mpMocAlarmObj[MOC_OVERLOAD_ALARM_OBJ].Attach(pSubject);
      mpMocAlarmDelay[MOC_OVERLOAD_ALARM_OBJ]->SetSubjectPointer(id, pSubject);
      break;
    case SP_MOC_MOTOR_CURRENT_UNDERLOAD_ALARM_OBJ:
      mpMocAlarmObj[MOC_UNDERLOAD_ALARM_OBJ].Attach(pSubject);
      mpMocAlarmDelay[MOC_UNDERLOAD_ALARM_OBJ]->SetSubjectPointer(id, pSubject);
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
 * Function - UpdateCurrent
 * DESCRIPTION: Calculate a filtered value of the measured current
 *
 * NOTE: Must be called every second
 *
 *****************************************************************************/
void MotorCurrentCtrl::UpdateCurrent()
{
  // Set the quality and range of output values
  if (mpResultingValueCurrent->GetQuality() == DP_NEVER_AVAILABLE)
  {
    mpLatestValueCurrent->SetQuality(DP_NEVER_AVAILABLE);
    mpFilteredValueCurrent->SetQuality(DP_NEVER_AVAILABLE);
    mFilteredCurrentHasBeenCalculated = false;
  }
  else
  {
    mpLatestValueCurrent->SetQuality(DP_AVAILABLE);
    mpFilteredValueCurrent->SetQuality(DP_AVAILABLE);
    mpLatestValueCurrent->SetMaxValue(mpMeasuredValueCurrent->GetMaxValue());
    mpFilteredValueCurrent->SetMaxValue(mpResultingValueCurrent->GetMaxValue());
  }

  if (mpActualOperationMode.IsUpdated())
  {
    mAwaitStartDelayCount = mpDelayAfterStart->GetValue();
    // Special adjust of mAwaitStartDelayCount caused by the way the timeout of the delay is handled.
    // Not perfect at all, but not that important since DelayAfterStart is fixed to 5 seconds anyway.
    if (mAwaitStartDelayCount < 1)
    {
      mAwaitStartDelayCount = 1;
    }
    mStartDelayElapsed = false;
  }
  if (mpActualOperationMode->GetValue() != ACTUAL_OPERATION_MODE_STARTED)
  {
    ; // Pump not started, do nothing
  }
  else if (mpResultingValueCurrent->GetQuality() != DP_AVAILABLE)
  {
    ; // No usable motor current, do nothing
  }
  else if (mpFoamDrainingRequested->GetValue() == true && mpUnderLowestStopLevel->GetValue() == true)
  {
    // Foam draining below stop level. The measured current may be invalid. Don't update filtered value.
    mpLatestValueCurrent->SetValue(mpResultingValueCurrent->GetValue());
  }
  else if (mAwaitStartDelayCount > 0)
  {
    // Pump has just been started, update latest value
    float actual_current = mpResultingValueCurrent->GetValue();
    mpLatestValueCurrent->SetValue(actual_current);
    mAwaitStartDelayCount--;
    if (mAwaitStartDelayCount == 0)
    {
      // Start delay has just elapsed. Initialize filtered current to be able to clear alarms
      mpFilteredValueCurrent->SetValue(actual_current);
    }
  }
  else if (mAwaitStartDelayCount == 0)
  {
    // Start delay has elapsed. Update filtered current
    float filter  = mpFilterTime->GetValue();
    float actual_current = mpResultingValueCurrent->GetValue();
    mpLatestValueCurrent->SetValue(actual_current);
    if (filter <= 1.0f) // No filter
    {
      mpFilteredValueCurrent->SetValue(actual_current);
    }
    else // 1. order low pass filter
    {
      float filtered_current = mpFilteredValueCurrent->GetValue();
      filter = 1.0f/filter;
      filtered_current = filter*actual_current + (1.0f-filter)*filtered_current;
      mpFilteredValueCurrent->SetValue(filtered_current);
    }
    mFilteredCurrentHasBeenCalculated = true;
    mStartDelayElapsed = true;
  }
}

/*****************************************************************************
 * Function - CheckAlarms
 * DESCRIPTION: Check the warning/alarm limits (for analog input)
 *
 *****************************************************************************/
void MotorCurrentCtrl::CheckAlarms()
{
  float current = -1.0f;
  if ( (mpMeasuredValueCurrent->GetQuality() == DP_AVAILABLE) // Check that analog sensor is available
    && (mFilteredCurrentHasBeenCalculated == true) )
  {
    current = mpFilteredValueCurrent->GetValue();
  }

  // Overload warning
  if (current >= mpMocAlarmObj[MOC_OVERLOAD_ALARM_OBJ]->GetAlarmConfig()->GetWarningLimit()->GetAsFloat())
  {
    mpMocAlarmDelay[MOC_OVERLOAD_ALARM_OBJ]->SetWarning();
  }
  else
  {
    mpMocAlarmDelay[MOC_OVERLOAD_ALARM_OBJ]->ResetWarning();
  }
  // Overload alarm, only when pump has been running for a while
  if ( (mStartDelayElapsed == true)
    && (current >= mpMocAlarmObj[MOC_OVERLOAD_ALARM_OBJ]->GetAlarmConfig()->GetAlarmLimit()->GetAsFloat()) )
  {
    mpMocAlarmDelay[MOC_OVERLOAD_ALARM_OBJ]->SetFault();
  }
  else
  {
    mpMocAlarmDelay[MOC_OVERLOAD_ALARM_OBJ]->ResetFault();
  }

  // Underload warning
  if ( (current >= 0.0f)
    && (current <= mpMocAlarmObj[MOC_UNDERLOAD_ALARM_OBJ]->GetAlarmConfig()->GetWarningLimit()->GetAsFloat()) )
  {
    mpMocAlarmDelay[MOC_UNDERLOAD_ALARM_OBJ]->SetWarning();
  }
  else
  {
    mpMocAlarmDelay[MOC_UNDERLOAD_ALARM_OBJ]->ResetWarning();
  }
  // Underload alarm, only when pump has been running for a while
  if ( (current >= 0.0f && mStartDelayElapsed == true)
    && (current <= mpMocAlarmObj[MOC_UNDERLOAD_ALARM_OBJ]->GetAlarmConfig()->GetAlarmLimit()->GetAsFloat()) )
  {
    mpMocAlarmDelay[MOC_UNDERLOAD_ALARM_OBJ]->SetFault();
  }
  else
  {
    mpMocAlarmDelay[MOC_UNDERLOAD_ALARM_OBJ]->ResetFault();
  }
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
