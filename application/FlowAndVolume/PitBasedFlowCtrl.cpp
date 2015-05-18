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
/* CLASS NAME       : PitBasedFlowCtrl                                      */
/*                                                                          */
/* FILE NAME        : PitBasedFlowCtrl.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 11-01-2008 dd-mm-yyyy                                 */
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
#include <PitBasedFlowCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define INFLOW_RESET_TIMEOUT    (2*3600)  // 2 Hours

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
PitBasedFlowCtrl::PitBasedFlowCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PitBasedFlowCtrl::~PitBasedFlowCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PitBasedFlowCtrl::InitSubTask()
{
  mPitState     = PIT_STATE_AWAIT_FILLING;
  mFillingTime  = 0;
  mEmptyingTime = 0;
  mOldNoOfPumps = 0;
  mOldPitLevel  = 100; // Must be abnormal high at start
  mInflowResetTime = 0;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PitBasedFlowCtrl::RunSubTask()
{
  // Update pit + pump flow
  if (mpFlowCalculationType->GetValue() != FLOW_CALCULATION_PIT_BASED)
  {
    mpPitBasedFlow->SetQuality(DP_NEVER_AVAILABLE);
  }
  else if (mpSurfaceLevel->GetQuality() != DP_AVAILABLE)
  {
    mpPitBasedFlow->SetQuality(mpSurfaceLevel->GetQuality());
  }
  else
  {
    if (mpPitBasedFlow->GetQuality() == DP_NEVER_AVAILABLE)
    {
      // A level sensor is now present, but the pit based flow is not calculated yet.
      mpPitBasedFlow->SetQuality(DP_NOT_AVAILABLE);
    }
    HandlePitBasedFlow();
  }
  mpPitBasedPumpFlow->SetQuality(mpPitBasedFlow->GetQuality());
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void PitBasedFlowCtrl::ConnectToSubjects()
{
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void PitBasedFlowCtrl::Update(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void PitBasedFlowCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void PitBasedFlowCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_PBF_FLOW_CALCULATION_TYPE:
      mpFlowCalculationType.Attach(pSubject);
      break;
    case SP_PBF_UPPER_MEASUREMENT_LEVEL:
      mpUpperMeasurementLevel.Attach(pSubject);
      break;
    case SP_PBF_LOWER_MEASUREMENT_LEVEL:
      mpLowerMeasurementLevel.Attach(pSubject);
      break;
    case SP_PBF_VOLUME_FOR_FLOW_MEASUREMENT:
      mpVolumeForFlowMeasurement.Attach(pSubject);
      break;
    case SP_PBF_MAX_FLOW_MEASUREMENT_TIME:
      mpMaxFlowMeasurementTime.Attach(pSubject);
      break;
    case SP_PBF_FLOW_MIN_MULTIPLY:
      mpFlowMinMultiply.Attach(pSubject);
      break;
    case SP_PBF_FLOW_MAX_MULTIPLY:
      mpFlowMaxMultiply.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_PBF_SURFACE_LEVEL:
      mpSurfaceLevel.Attach(pSubject);
      break;
    case SP_PBF_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;

    // Outputs:
    case SP_PBF_PIT_BASED_FLOW:
      mpPitBasedFlow.Attach(pSubject);
      break;
    case SP_PBF_PIT_BASED_PUMP_FLOW:
      mpPitBasedPumpFlow.Attach(pSubject);
      break;
    case SP_PBF_IN_FLOW_MEASUREMENT_TIME:
      mpInFlowMeasurementTime.Attach(pSubject);
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
 * Function - HandlePitBasedFlow
 * DESCRIPTION: Run the state machine for handling/calculation of pit based flow
 *
 * NOTE:        Must be called every second
 *
 *****************************************************************************/
void PitBasedFlowCtrl::HandlePitBasedFlow()
{
  // Run this function every second
  float lower_level = mpLowerMeasurementLevel->GetValue();
  float upper_level = mpUpperMeasurementLevel->GetValue();
  float pit_level   = mpSurfaceLevel->GetValue();
  U32   no_of_pumps = mpNoOfRunningPumps->GetValue();

  switch (mPitState)
  {
    default:
    case PIT_STATE_AWAIT_FILLING:
      if (no_of_pumps == 0 && pit_level > lower_level && mOldPitLevel <= lower_level)
      {
        // Lower level passed upwards. Start filling time measurement.
        mFillingTime = 0;
        mPitState = PIT_STATE_FILLING;
      }
      break;

    case PIT_STATE_FILLING:
      mFillingTime++;
      if (mOldNoOfPumps != 0)
      {
        // Wrong condition, abort this state
        mPitState = PIT_STATE_AWAIT_FILLING;
      }
      else if (pit_level > upper_level && mOldPitLevel <= upper_level)
      {
        // Upper level passed upwards. Calculate new flow
        mpPitBasedFlow->SetValue(mpVolumeForFlowMeasurement->GetValue()/mFillingTime);
        mInflowResetTime = INFLOW_RESET_TIMEOUT;
        mpInFlowMeasurementTime->SetValue(mFillingTime);
        mPitState = PIT_STATE_AWAIT_EMPTYING;
      }
      break;

    case PIT_STATE_AWAIT_EMPTYING:
      if (no_of_pumps > 1 || pit_level < lower_level)
      {
        // Wrong conditions, abort this state
        mPitState = PIT_STATE_AWAIT_FILLING;
      }
      else if (no_of_pumps == 1 && pit_level < upper_level && mOldPitLevel >= upper_level)
      {
        // Upper level passed downwards. Start emptying time measurement.
        mEmptyingTime = 0;
        mPitState = PIT_STATE_EMPTYING;
      }
      break;

    case PIT_STATE_EMPTYING:
      mEmptyingTime++;
      if (mOldNoOfPumps != 1)
      {
        // Wrong condition, abort this state
        mPitState = PIT_STATE_AWAIT_FILLING;
      }
      else if (pit_level < lower_level && mOldPitLevel >= lower_level)
      {
        // Lower level passed downwards. Validate and calculate pump flow.
        if ( mFillingTime <= mpMaxFlowMeasurementTime->GetValue()
          && mFillingTime >= mEmptyingTime*mpFlowMinMultiply->GetValue()
          && mFillingTime <= mEmptyingTime*mpFlowMaxMultiply->GetValue())
        {
          float pump_flow = mpVolumeForFlowMeasurement->GetValue()/mEmptyingTime + mpPitBasedFlow->GetValue();
          if (pump_flow == mpPitBasedPumpFlow->GetValue())
          {
            // Ensure a notifiaction of observers if the filling/emptying time should happen to be exactly the same as before
            pump_flow += 0.000001f;
          }
          mpPitBasedPumpFlow->SetValue(pump_flow);
        }
        mPitState = PIT_STATE_AWAIT_FILLING;
      }
      break;
  }

  // Reset inflow if long time since last inflow calculation
  if (mInflowResetTime > 0 && no_of_pumps == 0)
  {
    mInflowResetTime--;
    if (mInflowResetTime == 0)
    {
      mpPitBasedFlow->SetValue(0.0f);
    }
  }

  mOldPitLevel = pit_level;
  mOldNoOfPumps = no_of_pumps;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
