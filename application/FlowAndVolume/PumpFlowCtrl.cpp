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
/* CLASS NAME       : PumpFlowCtrl                                          */
/*                                                                          */
/* FILE NAME        : PumpFlowCtrl.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 11-04-2008 dd-mm-yyyy                                 */
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
#include <PumpFlowCtrl.h>
#include <ActTime.h>

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
PumpFlowCtrl::PumpFlowCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_PFC_ALARM_OBJ; fault_id < NO_OF_PFC_ALARM_OBJ; fault_id++)
  {
    mpPfcAlarmDelay[fault_id] = new AlarmDelay(this);
    mPfcAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PumpFlowCtrl::~PumpFlowCtrl()
{
  for (int fault_id = FIRST_PFC_ALARM_OBJ; fault_id < NO_OF_PFC_ALARM_OBJ; fault_id++)
  {
    delete(mpPfcAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpFlowCtrl::InitSubTask()
{
  for (int fault_id = FIRST_PFC_ALARM_OBJ; fault_id < NO_OF_PFC_ALARM_OBJ; fault_id++)
  {
    mpPfcAlarmDelay[fault_id]->InitAlarmDelay();
  }

  mStartDelayCounter = 0;
  mHighestFlowDuringPumping = 0.0f;
  mPumpWasRunningAlone = false;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpFlowCtrl::RunSubTask()
{
  // Pre alarm update
  for (int fault_id = FIRST_PFC_ALARM_OBJ; fault_id < NO_OF_PFC_ALARM_OBJ; fault_id++)
  {
    if (mPfcAlarmDelayCheckFlag[fault_id] == true)
    {
      mPfcAlarmDelayCheckFlag[fault_id] = false;
      mpPfcAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  // Do the good stuff
  UpdatePumpFlow();
  CheckLowFlowAlarm();

  // Post alarm update
  for (int fault_id = FIRST_PFC_ALARM_OBJ; fault_id < NO_OF_PFC_ALARM_OBJ; fault_id++)
  {
    mpPfcAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void PumpFlowCtrl::ConnectToSubjects()
{
  mpPitBasedPumpFlow->Subscribe(this);

  for (int fault_id = FIRST_PFC_ALARM_OBJ; fault_id < NO_OF_PFC_ALARM_OBJ; fault_id++)
  {
    mpPfcAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void PumpFlowCtrl::Update(Subject* pSubject)
{
  mpPitBasedPumpFlow.Update(pSubject);

  for (int fault_id = FIRST_PFC_ALARM_OBJ; fault_id < NO_OF_PFC_ALARM_OBJ; fault_id++)
  {
    if (pSubject == mpPfcAlarmDelay[fault_id])
    {
      mPfcAlarmDelayCheckFlag[fault_id] = true;
      break;
    }
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpFlowCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_PFC_ALARM_OBJ; fault_id < NO_OF_PFC_ALARM_OBJ; fault_id++)
  {
    mpPfcAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void PumpFlowCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_PFC_FLOW_CALCULATION_DELAY_AFTER_START:
      mpDelayAfterStart.Attach(pSubject);
      break;
    case SP_PFC_FLOW_CALCULATION_FILTER_FACTOR:
      mpFilterFactor.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_PFC_SYSTEM_FLOW:
      mpSystemFlow.Attach(pSubject);
      break;
    case SP_PFC_PIT_BASED_PUMP_FLOW:
      mpPitBasedPumpFlow.Attach(pSubject);
      break;
    case SP_PFC_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;
    case SP_PFC_OPERATION_MODE_ACTUAL:
      mpActualOperationMode.Attach(pSubject);
      break;
    case SP_PFC_FLOW_QUALITY:
      mpFlowQuality.Attach(pSubject);
      break;
    case SP_PFC_UNDER_LOWEST_STOP_LEVEL:
      mpUnderLowestStopLevel.Attach(pSubject);
      break;

    // Outputs:
    case SP_PFC_CALCULATED_FLOW_RAW:
      mpCalculatedPumpFlowRaw.Attach(pSubject);
      break;
    case SP_PFC_CALCULATED_FLOW_FILTERED:
      mpCalculatedPumpFlowFiltered.Attach(pSubject);
      break;
    case SP_PFC_FLOW_CALCULATION_TIMESTAMP:
      mpFlowCalculationTimestamp.Attach(pSubject);
      break;
    case SP_PFC_NO_OF_FLOW_CALCULATIONS:
      mpNoOfPumpFlowCalculations.Attach(pSubject);
      break;

    // Alarms
    case SP_PFC_LOW_FLOW_ALARM_OBJ:
      mpPfcAlarmObj[PFC_ALARM_OBJ_LOW_FLOW].Attach(pSubject);
      mpPfcAlarmDelay[PFC_ALARM_OBJ_LOW_FLOW]->SetSubjectPointer(id, pSubject);
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
 * Function - UpdatePumpFlow
 * DESCRIPTION: Determine if the pump flow is or should be updated.
 *              Filter the pump flow and update some flow related data points.
 *
 *****************************************************************************/
void PumpFlowCtrl::UpdatePumpFlow()
{
  float pump_flow = 0.0f;

  switch (mpFlowQuality->GetValue())
  {
    case FLOW_QUALITY_FLOW_METER:
    case FLOW_QUALITY_VOLUME_METER:
    case FLOW_QUALITY_ADVANCED_FLOW_ESTIMATION:
      if (mpActualOperationMode->GetValue() != ACTUAL_OPERATION_MODE_STARTED || mpNoOfRunningPumps->GetValue() != 1)
      {
        mStartDelayCounter = 0;
        if (mHighestFlowDuringPumping > 0.0f)
        {
          // A valid flow was found when one pump was running. Use it for updating the average pump flow
          pump_flow = mHighestFlowDuringPumping;
          mHighestFlowDuringPumping = 0.0f;
        }
      }
      else if (mStartDelayCounter < mpDelayAfterStart->GetValue())
      {
        // Do nothing unless the pump has been started for a while
        mStartDelayCounter++;
      }
      else
      {
        // Start delay has elapsed, find the highest valid flow during pumping
        if ( (mpSystemFlow->GetQuality() == DP_AVAILABLE)
          && (mHighestFlowDuringPumping < mpSystemFlow->GetValue())
          && (mpUnderLowestStopLevel->GetValue() == false) )
        {
          mHighestFlowDuringPumping = mpSystemFlow->GetValue();
        }
      }
      break;

    case FLOW_QUALITY_LEVEL_SENSOR:
      if  (mpPitBasedPumpFlow.IsUpdated() && mpPitBasedPumpFlow->GetQuality() == DP_AVAILABLE && mPumpWasRunningAlone == true)
      {
        pump_flow = mpPitBasedPumpFlow->GetValue();
      }
      break;

    case FLOW_QUALITY_NO_FLOW_MEASUREMENT:
    default:
      // No valid pump flow
      break;
  }

  if (pump_flow > 0.0f)
  {
    U32 no_of_calculations = mpNoOfPumpFlowCalculations->GetValue();
    float filtered_flow = mpCalculatedPumpFlowFiltered->GetValue();
    if (no_of_calculations >= 2 && mpFilterFactor->GetValue() > 1.0f)
    {
      float filter = 1.0f/mpFilterFactor->GetValue();
      filtered_flow = filter*pump_flow + (1.0f-filter)*filtered_flow;
    }
    else
    {
      // Just use the raw value for the first calculations
      filtered_flow = pump_flow;
    }
    mpCalculatedPumpFlowRaw->SetValue(pump_flow);
    mpCalculatedPumpFlowFiltered->SetValue(filtered_flow);
    mpFlowCalculationTimestamp->SetValue(ActTime::GetInstance()->GetSecondsSince1Jan1970());
    mpNoOfPumpFlowCalculations->SetValue(1+no_of_calculations);
  }

  // Prepare the flag mPumpWasRunningAlone for next loop
  if (mpActualOperationMode->GetValue() == ACTUAL_OPERATION_MODE_STARTED && mpNoOfRunningPumps->GetValue() == 1)
  {
    mPumpWasRunningAlone = true;
  }
  else
  {
    mPumpWasRunningAlone = false;
  }

  // Set quality of output data
  if (mpFlowQuality->GetValue() == FLOW_QUALITY_NO_FLOW_MEASUREMENT)
  {
    mpCalculatedPumpFlowRaw->SetQuality(DP_NEVER_AVAILABLE);
    mpCalculatedPumpFlowFiltered->SetQuality(DP_NEVER_AVAILABLE);
    mpFlowCalculationTimestamp->SetQuality(DP_NEVER_AVAILABLE);
    mpNoOfPumpFlowCalculations->SetQuality(DP_NEVER_AVAILABLE);
  }
  else if (mpCalculatedPumpFlowRaw->GetQuality() == DP_NEVER_AVAILABLE)
  {
    mpCalculatedPumpFlowRaw->SetQuality(DP_AVAILABLE);
    mpCalculatedPumpFlowFiltered->SetQuality(DP_AVAILABLE);
    mpFlowCalculationTimestamp->SetQuality(DP_AVAILABLE);
    mpNoOfPumpFlowCalculations->SetQuality(DP_AVAILABLE);
  }
}

/*****************************************************************************
 * Function - CheckLowFlowAlarm
 * DESCRIPTION: Check low flow alarm/warning
 *
 *****************************************************************************/
void PumpFlowCtrl::CheckLowFlowAlarm()
{
  float filtered_flow = mpCalculatedPumpFlowFiltered->GetValue();
  if (mpCalculatedPumpFlowFiltered->GetQuality() == DP_NEVER_AVAILABLE || mpNoOfPumpFlowCalculations->GetValue() == 0)
  {
    filtered_flow = mpCalculatedPumpFlowFiltered->GetMaxValue(); // To reset alarm/warning on invalid conditions
  }

  // Evaluate warning
  if (filtered_flow < mpPfcAlarmObj[PFC_ALARM_OBJ_LOW_FLOW]->GetAlarmConfig()->GetWarningLimit()->GetAsFloat())
  {
    mpPfcAlarmDelay[PFC_ALARM_OBJ_LOW_FLOW]->SetWarning();
  }
  else
  {
    mpPfcAlarmDelay[PFC_ALARM_OBJ_LOW_FLOW]->ResetWarning();
  }
  // Evaluate alarm
  if (filtered_flow < mpPfcAlarmObj[PFC_ALARM_OBJ_LOW_FLOW]->GetAlarmConfig()->GetAlarmLimit()->GetAsFloat())
  {
    mpPfcAlarmDelay[PFC_ALARM_OBJ_LOW_FLOW]->SetFault();
  }
  else
  {
    mpPfcAlarmDelay[PFC_ALARM_OBJ_LOW_FLOW]->ResetFault();
  }
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
