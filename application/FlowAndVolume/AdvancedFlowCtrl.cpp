/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: Dedicated Controls                               */
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
/* CLASS NAME       : AdvancedFlowCtrl                                      */
/*                                                                          */
/* FILE NAME        : AdvancedFlowCtrl.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 29-10-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <math.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AdvancedFlowCtrl.h>
#include <GeneralCnf.h>

//HRG
extern "C" float estimated_in_flow = 0;


/*****************************************************************************
  DEFINES
 *****************************************************************************/

#define TIME_TO_STORE_PARAMETERS      (24*60*60)

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
AdvancedFlowCtrl::AdvancedFlowCtrl()
{
  CreateFlowCalculationObjects();
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AdvancedFlowCtrl::~AdvancedFlowCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::InitSubTask()
{
  InitializeFlowCalculationObjects();

  LoadFlowParametersFromFlash();

  mNewParametersLearnedEvent = false;
  mSecondsSinceParameterStorage = 0;

  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    mTrainingEnabledState[i] = AFC_TRAINING_OFF;
    mPumpLearningState[i] = FLOW_LEARNING_IDLE;
    mRequestTrainingEvent[i] = false;
    mPumpStartedEvent[i] = false;
    mConfigurationIsUpdated[i] = false;
    mAwaitPumpRampUpTime[i] = 0;
  }

  mpAdvFlowTempStopRequest->SetQuality(DP_NOT_AVAILABLE);
  mpIsLearningInProgress->SetValue(false);
}


/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 * called once every second
 *
 *****************************************************************************/
void AdvancedFlowCtrl::RunSubTask()
{
// TEST HRG: Special developer code to setup training:
if ((int)(mpVfdMaxFrequency[0]->GetValue()*10+0.5f) == 499 && (int)(mpVfdMinFrequency[0]->GetValue()*10+0.5f) == 333)
{
  mpFlowCalculationType->SetValue(FLOW_CALCULATION_ADVANCED);
}
// End developer code


  mSecondsSinceParameterStorage++;

  HandleAdvancedFlowCalculation();
}


/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void AdvancedFlowCtrl::ConnectToSubjects()
{
  mpStartLearningEvent.Subscribe(this);
  mpPitStartLevel.Subscribe(this);
  mpPitStopLevel.Subscribe(this);
  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    mpVfdMinFrequency[i].Subscribe(this);
    mpVfdMaxFrequency[i].Subscribe(this);
  }
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void AdvancedFlowCtrl::Update(Subject* pSubject)
{
  bool level_updated = false;
  mpStartLearningEvent.Update(pSubject);
  level_updated |= mpPitStartLevel.Update(pSubject);
  level_updated |= mpPitStopLevel.Update(pSubject);
  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    mConfigurationIsUpdated[i] |= level_updated;
    mConfigurationIsUpdated[i] |= mpVfdMinFrequency[i].Update(pSubject);
    mConfigurationIsUpdated[i] |= mpVfdMaxFrequency[i].Update(pSubject);
  }
}


/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::SubscribtionCancelled(Subject* pSubject)
{
  // not used
}


/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void AdvancedFlowCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_AFC_FLOW_CALCULATION_TYPE:
      mpFlowCalculationType.Attach(pSubject);
      break;
    case SP_AFC_NOMINAL_POWER:
      mpNominalPower.Attach(pSubject);
      break;
    case SP_AFC_NOMINAL_PRESSURE:
      mpNominalPressure.Attach(pSubject);
      break;
    case SP_AFC_NOMINAL_FLOW:
      mpNominalFlow.Attach(pSubject);
      break;
    case SP_AFC_PIT_AREA:
      mpPitArea.Attach(pSubject);
      break;
    case SP_AFC_PIT_START_LEVEL:
      mpPitStartLevel.Attach(pSubject);
      break;
    case SP_AFC_PIT_STOP_LEVEL:
      mpPitStopLevel.Attach(pSubject);
      break;
    case SP_AFC_TEMP_STOP_LEVEL:
      mpAdvFlowTempStopLevel.Attach(pSubject);
      break;
    case SP_AFC_TEMP_STOP_PERIOD:
      mpAdvFlowTempStopPeriod.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_AFC_START_LEARNING_EVENT:
      mpStartLearningEvent.Attach(pSubject);
      break;
    case SP_AFC_SURFACE_LEVEL:
      mpSurfaceLevel.Attach(pSubject);
      break;
    case SP_AFC_OUTLET_PRESSURE:
      mpAiOutletPressure.Attach(pSubject);
      break;
    case SP_AFC_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;

    case SP_AFC_VFD_STATE_PUMP_1:
      mpVfdState[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_VFD_STATE_PUMP_2:
      mpVfdState[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_VFD_STATE_PUMP_3:
      mpVfdState[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_VFD_STATE_PUMP_4:
      mpVfdState[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_VFD_STATE_PUMP_5:
      mpVfdState[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_VFD_STATE_PUMP_6:
      mpVfdState[PUMP_6].Attach(pSubject);
      break;

    case SP_AFC_VFD_INSTALLED_PUMP_1:
      mpVfdInstalled[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_VFD_INSTALLED_PUMP_2:
      mpVfdInstalled[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_VFD_INSTALLED_PUMP_3:
      mpVfdInstalled[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_VFD_INSTALLED_PUMP_4:
      mpVfdInstalled[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_VFD_INSTALLED_PUMP_5:
      mpVfdInstalled[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_VFD_INSTALLED_PUMP_6:
      mpVfdInstalled[PUMP_6].Attach(pSubject);
      break;

    case SP_AFC_MIN_FREQUENCY_PUMP_1:
      mpVfdMinFrequency[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_MIN_FREQUENCY_PUMP_2:
      mpVfdMinFrequency[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_MIN_FREQUENCY_PUMP_3:
      mpVfdMinFrequency[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_MIN_FREQUENCY_PUMP_4:
      mpVfdMinFrequency[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_MIN_FREQUENCY_PUMP_5:
      mpVfdMinFrequency[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_MIN_FREQUENCY_PUMP_6:
      mpVfdMinFrequency[PUMP_6].Attach(pSubject);
      break;

    case SP_AFC_MAX_FREQUENCY_PUMP_1:
      mpVfdMaxFrequency[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_MAX_FREQUENCY_PUMP_2:
      mpVfdMaxFrequency[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_MAX_FREQUENCY_PUMP_3:
      mpVfdMaxFrequency[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_MAX_FREQUENCY_PUMP_4:
      mpVfdMaxFrequency[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_MAX_FREQUENCY_PUMP_5:
      mpVfdMaxFrequency[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_MAX_FREQUENCY_PUMP_6:
      mpVfdMaxFrequency[PUMP_6].Attach(pSubject);
      break;


    case SP_AFC_POWER_PUMP_1:
      mpPower[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_POWER_PUMP_2:
      mpPower[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_POWER_PUMP_3:
      mpPower[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_POWER_PUMP_4:
      mpPower[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_POWER_PUMP_5:
      mpPower[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_POWER_PUMP_6:
      mpPower[PUMP_6].Attach(pSubject);
      break;

    case SP_AFC_VFD_FREQUENCY_PUMP_1:
      mpVfdFrequency[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_VFD_FREQUENCY_PUMP_2:
      mpVfdFrequency[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_VFD_FREQUENCY_PUMP_3:
      mpVfdFrequency[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_VFD_FREQUENCY_PUMP_4:
      mpVfdFrequency[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_VFD_FREQUENCY_PUMP_5:
      mpVfdFrequency[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_VFD_FREQUENCY_PUMP_6:
      mpVfdFrequency[PUMP_6].Attach(pSubject);
      break;

    case SP_AFC_CUE_FREQUENCY_PUMP_1:
      mpCUEFrequency[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_CUE_FREQUENCY_PUMP_2:
      mpCUEFrequency[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_CUE_FREQUENCY_PUMP_3:
      mpCUEFrequency[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_CUE_FREQUENCY_PUMP_4:
      mpCUEFrequency[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_CUE_FREQUENCY_PUMP_5:
      mpCUEFrequency[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_CUE_FREQUENCY_PUMP_6:
      mpCUEFrequency[PUMP_6].Attach(pSubject);
      break;

    case SP_AFC_OPERATION_MODE_PUMP_1:
      mpOperationMode[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_OPERATION_MODE_PUMP_2:
      mpOperationMode[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_OPERATION_MODE_PUMP_3:
      mpOperationMode[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_OPERATION_MODE_PUMP_4:
      mpOperationMode[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_OPERATION_MODE_PUMP_5:
      mpOperationMode[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_OPERATION_MODE_PUMP_6:
      mpOperationMode[PUMP_6].Attach(pSubject);
      break;

    case SP_AFC_LEARNING_PARAMETERS_PUMP_1:
      mpLearningParameters[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_LEARNING_PARAMETERS_PUMP_2:
      mpLearningParameters[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_LEARNING_PARAMETERS_PUMP_3:
      mpLearningParameters[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_LEARNING_PARAMETERS_PUMP_4:
      mpLearningParameters[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_LEARNING_PARAMETERS_PUMP_5:
      mpLearningParameters[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_LEARNING_PARAMETERS_PUMP_6:
      mpLearningParameters[PUMP_6].Attach(pSubject);
      break;


    // Outputs:
    case SP_AFC_LEARNING_IN_PROGRESS:
      mpIsLearningInProgress.Attach(pSubject);
      break;
    case SP_AFC_IS_READY_TO_LEARN:
      mpIsReadyToLearn.Attach(pSubject);
      break;
    case SP_AFC_FLOW_TOTAL:
      mpCalculatedFlow.Attach(pSubject);
      break;

    case SP_AFC_TRAINING_FREQUENCY_PUMP_1:
      mpVfdTrainingFrequency[PUMP_1].Attach(pSubject);
      break;
    case SP_AFC_TRAINING_FREQUENCY_PUMP_2:
      mpVfdTrainingFrequency[PUMP_2].Attach(pSubject);
      break;
    case SP_AFC_TRAINING_FREQUENCY_PUMP_3:
      mpVfdTrainingFrequency[PUMP_3].Attach(pSubject);
      break;
    case SP_AFC_TRAINING_FREQUENCY_PUMP_4:
      mpVfdTrainingFrequency[PUMP_4].Attach(pSubject);
      break;
    case SP_AFC_TRAINING_FREQUENCY_PUMP_5:
      mpVfdTrainingFrequency[PUMP_5].Attach(pSubject);
      break;
    case SP_AFC_TRAINING_FREQUENCY_PUMP_6:
      mpVfdTrainingFrequency[PUMP_6].Attach(pSubject);
      break;
    case SP_AFC_TEMP_STOP_REQUEST:
      mpAdvFlowTempStopRequest.Attach(pSubject);
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
 * Function - CreateFlowCalculationObjects
 * DESCRIPTION:
 * Called once by constructor
 *
 *****************************************************************************/
void AdvancedFlowCtrl::CreateFlowCalculationObjects()
{
  mpKalmanCalculator = new KalmanCalc();
  mpPitFlow = new PitFlow(mpKalmanCalculator);
  mpMotorModel = new MotorModel();
  mpPumpCurves = new PumpCurves();

  mpDynamicParameterIdentifier = new DynparIdent(mpKalmanCalculator);

  mpSystemFlow = new SystemFlow(mpPitFlow, mpDynamicParameterIdentifier);

  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    mpParameterIdentifiers[i] = new ParIdent(mpPitFlow, mpKalmanCalculator);
    mpPumpEvaluations[i] = new PumpEvaluation(mpParameterIdentifiers[i], mpPumpCurves);
    mpFlowLearnControl[i] = new FlowLearnControl(mpParameterIdentifiers[i], mpPitFlow);
  }
}


/*****************************************************************************
 * Function - InitializeFlowCalculationObjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::InitializeFlowCalculationObjects()
{
  float nominal_power = mpNominalPower->GetValue();
  float nominal_pressure = mpNominalPressure->GetValue();
  float nominal_flow = mpNominalFlow->GetValue();
  float nominal_speed = 50.0f;

  float nominal_torque = nominal_power / nominal_speed;

  float pressure_in_pit = GetPressureInPit();

  mpPitFlow->InitPitFlow(pressure_in_pit);

  mpSystemFlow->InitSystemFlow(nominal_flow);

  mpDynamicParameterIdentifier->InitDynparIdent(nominal_flow, nominal_pressure);

  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    mpPumpEvaluations[i]->InitPumpEvaluation();

    mpParameterIdentifiers[i]->InitParIdent(nominal_flow, nominal_pressure, nominal_torque, nominal_speed);
  }
}


/*****************************************************************************
 * Function - HandleAdvancedFlowCalculation
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::HandleAdvancedFlowCalculation()
{
  HandleTrainingRestart();

  UpdateIsAbleAndReadyFlags();

  if (mIsConfiguredForAdvancedFlow)
  {
    for (int i = FIRST_PUMP; i <= LAST_PUMP;  i++)
    {
      UpdatePumpState((PUMP_TYPE) i);

      UpdateTrainingState((PUMP_TYPE) i);

      UpdateTrainingEnabled((PUMP_TYPE) i);
    }

    RunPeriodFlowCalculationTasks();

    CalculateTotalFlow();

    HandleParameterStorage();
  }
  else
  {
    mpCalculatedFlow->SetQuality(DP_NEVER_AVAILABLE);
    mpAdvFlowTempStopRequest->SetQuality(DP_NOT_AVAILABLE);// Means no temp stop
    for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
    {
      mPumpLearningState[i] = FLOW_LEARNING_IDLE;
      mpVfdTrainingFrequency[i]->SetQuality(DP_NEVER_AVAILABLE);
    }
  }
}


/*****************************************************************************
 * Function - HandleTrainingRestart
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::HandleTrainingRestart()
{
  if (mpStartLearningEvent.IsUpdated())
  {
    if (mpIsReadyToLearn->GetValue())
    {
      for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
      {
        mpParameterIdentifiers[i]->ReInitTraining();
        mpFlowLearnControl[i]->ReInitLearning();

        mRequestTrainingEvent[i] = true;
        mPumpLearningState[i] = FLOW_LEARNING_IDLE;
      }

      InitializeFlowCalculationObjects();

      mpIsLearningInProgress->SetValue(true);
    }
    //ignore event if not ready
  }
}


/*****************************************************************************
 * Function - UpdateIsAbleAndReadyFlags
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::UpdateIsAbleAndReadyFlags()
{
  mIsConfiguredForAdvancedFlow = mpFlowCalculationType->GetValue() == FLOW_CALCULATION_ADVANCED
     && mpSurfaceLevel->GetQuality() != DP_NEVER_AVAILABLE
     && mpAiOutletPressure->GetQuality() != DP_NEVER_AVAILABLE;

  bool is_able_to_learn = mpFlowCalculationType->GetValue() == FLOW_CALCULATION_ADVANCED
    && mpSurfaceLevel->GetQuality() == DP_AVAILABLE
    && mpAiOutletPressure->GetQuality() == DP_AVAILABLE;

  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    mIsAbleToLearn[i] = is_able_to_learn
      && mPumpState[i] != AFC_PUMP_NOT_THERE
      && mpPower[i]->GetQuality() == DP_AVAILABLE;
  }

  if (!is_able_to_learn)
  {
    mpIsLearningInProgress->SetValue(false);
  }

  mpIsReadyToLearn->SetValue(is_able_to_learn && !mpIsLearningInProgress->GetValue());
}


/*****************************************************************************
 * Function - UpdatePumpState
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::UpdatePumpState(PUMP_TYPE pumpNo)
{
  switch (mpOperationMode[pumpNo]->GetValue())
  {
    case ACTUAL_OPERATION_MODE_STOPPED:
      mPumpState[pumpNo] = AFC_PUMP_STOPPED;
      break;

    case ACTUAL_OPERATION_MODE_STARTED:
      if (mpVfdState[pumpNo]->GetValue() == VFD_OPERATION_MODE_REV_FLUSH)
      {
        mPumpState[pumpNo] = AFC_PUMP_STOPPED;
      }
      else
      {
        if (mPumpState[pumpNo] != AFC_PUMP_RUNNING)
        {
          mPumpState[pumpNo] = AFC_PUMP_RUNNING;
          mPumpStartedEvent[pumpNo] = true;
        }
      }
      break;

    default:
      // ACTUAL_OPERATION_MODE_DISABLED:
      // ACTUAL_OPERATION_MODE_NOT_INSTALLED:
      mPumpState[pumpNo] = AFC_PUMP_NOT_THERE;
      break;
  }

  if (mPumpState[pumpNo] != AFC_PUMP_RUNNING)
  {
    if (mpCUEFrequency[pumpNo]->GetQuality() == DP_AVAILABLE)
    {
      // Initialize to a short delay since the wait time is counted after ramping up
      mAwaitPumpRampUpTime[pumpNo] = 2;
    }
    else
    {
      // Initialize to a longer delay. (3-8 seconds, but we really don't know)
      mAwaitPumpRampUpTime[pumpNo] = 5;
    }
  }
  else // Pump is running, determine when ramp up has passed
  {
    if (mAwaitPumpRampUpTime[pumpNo] > 0)
    {
      float ref_frequency = mpVfdFrequency[pumpNo]->GetValue();
      if ( (mpCUEFrequency[pumpNo]->GetQuality() != DP_AVAILABLE)
        || (ref_frequency > 0.0f && GetFrequency(pumpNo) >= 0.95f*ref_frequency) )
      {
        mAwaitPumpRampUpTime[pumpNo]--;
      }
    }
  }
}


/*****************************************************************************
 * Function - UpdateTrainingState
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::UpdateTrainingState(PUMP_TYPE pumpNo)
{
  if (mRequestTrainingEvent[pumpNo] && mPumpLearningState[pumpNo] == FLOW_LEARNING_IDLE)
  {
    mPumpLearningState[pumpNo] = FLOW_LEARNING_RUNNING;
  }
  mRequestTrainingEvent[pumpNo] = false;

  if (mPumpLearningState[pumpNo] != FLOW_LEARNING_IDLE)
  {
    if (!mIsAbleToLearn[pumpNo])
    {
      mPumpLearningState[pumpNo] = FLOW_LEARNING_IDLE;
    }
    else if (mPumpState[pumpNo] == AFC_PUMP_RUNNING && IsTrainingFinishedForPump(pumpNo))
    {
      mPumpLearningState[pumpNo] = FLOW_LEARNING_IDLE;
      mNewParametersLearnedEvent = true;
    }
  }
}


/*****************************************************************************
 * Function - UpdateTrainingEnabled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::UpdateTrainingEnabled(PUMP_TYPE pumpNo)
{
  int no_of_running_pumps = mpNoOfRunningPumps->GetValue();

  if (mPumpStartedEvent[pumpNo])
  {
    if (no_of_running_pumps > 0) // Needed to ensure that no_of_running_pumps is updated
    {
      mTrainingEnabledState[pumpNo] = AFC_TRAINING_ON;
      mPumpStartedEvent[pumpNo] = false;
    }
  }

  if (!mIsAbleToLearn[pumpNo]
    || (no_of_running_pumps > 1)
    || (no_of_running_pumps == 1 && mPumpState[pumpNo] != AFC_PUMP_RUNNING))
  {
    mTrainingEnabledState[pumpNo] = AFC_TRAINING_OFF;
  }
}


/*****************************************************************************
 * Function - UpdateTrainingFrequency
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::UpdateTrainingFrequency(PUMP_TYPE pumpNo)
{
  static bool level_ok;             // Use static since it should be used for all pumps
  static int no_of_running_pumps;   // Use static since it should be used for all pumps

  if (pumpNo == 0)
  {
    no_of_running_pumps = mpNoOfRunningPumps->GetValue();
    level_ok = (mpSurfaceLevel->GetValue() <= 0.05f + mpPitStartLevel->GetValue());
    if ( (mpIsLearningInProgress->GetValue() == false)
      || (level_ok == false)
      || (no_of_running_pumps > 0 && mpAdvFlowTempStopRequest->GetValue() < NO_OF_PUMPS) )
    {
      // Wrong conditions for temporary stop, abort it.
      // Do NOT use SetValue here because it makes a notify and the event task has higher priority
      mpAdvFlowTempStopRequest->SetQuality(DP_NOT_AVAILABLE);
    }
  }

  if ( (level_ok == true)
    && (mPumpLearningState[pumpNo] != FLOW_LEARNING_IDLE)
    && (mTrainingEnabledState[pumpNo] == AFC_TRAINING_ON) )
  {
    float training_frequency = mpFlowLearnControl[pumpNo]->GetFlowLearnFrequency();
    if (training_frequency > 0.0f)
    {
      if (mpAdvFlowTempStopRequest->GetQuality() == DP_AVAILABLE && mpAdvFlowTempStopRequest->GetValue() == pumpNo)
      {
        // Resume after temporary stop
        mpAdvFlowTempStopRequest->SetValue(NO_OF_PUMPS);
      }
      mpVfdTrainingFrequency[pumpNo]->SetValue(training_frequency);
    }
    else // Make a temporary stop if the pump is running alone
    {
      if ( (mPumpState[pumpNo] == AFC_PUMP_RUNNING)
        && (no_of_running_pumps == 1) )
      {
        mpAdvFlowTempStopRequest->SetValue(pumpNo);
      }
    }
  }
  else
  {
    // Make to use the normal vfd frequency if the level is above the start level or not training
    mpVfdTrainingFrequency[pumpNo]->SetQuality(DP_NOT_AVAILABLE);
  }
}


/*****************************************************************************
 * Function - UpdateFlowLearnControl
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::UpdateFlowLearnControl(PUMP_TYPE pumpNo)
{
  AFC_RUN_FLOW_LEARN_STRUCT run_learn;

  if (mConfigurationIsUpdated[pumpNo] == true)
  {
    float min_frequency, max_frequency;
    if (mpVfdInstalled[pumpNo]->GetValue() == true)
    {
      min_frequency = mpVfdMinFrequency[pumpNo]->GetValue();
      max_frequency = mpVfdMaxFrequency[pumpNo]->GetValue();
    }
    else
    {
      min_frequency = mpVfdFrequency[pumpNo]->GetMaxValue();
      max_frequency = min_frequency;
    }
    mpFlowLearnControl[pumpNo]->SetFlowLearnParameters( mpPitStopLevel->GetValue(), mpPitStartLevel->GetValue(),
                                                        min_frequency, max_frequency);
    mConfigurationIsUpdated[pumpNo] = false;
  }

  if (mPumpLearningState[pumpNo] == FLOW_LEARNING_RUNNING)
  {
    run_learn.level = mpSurfaceLevel->GetValue();
    run_learn.speed = GetFrequency(pumpNo);
    run_learn.pumpState = mPumpState[pumpNo];
    mpFlowLearnControl[pumpNo]->CtrlFlowLearnControl(&run_learn);
  }
}


/*****************************************************************************
 * Function - RunPeriodFlowCalculationTasks
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::RunPeriodFlowCalculationTasks()
{
  float pressure_in_pit = GetPressureInPit();
  float pressure_in_outlet = GetPressureInOutlet();

  // run pit flow calculation
  mpPitFlow->RunPitFlow(pressure_in_pit);

  AFC_RUN_PAR_UPDATE_STRUCT run_par_update_arguments;
  run_par_update_arguments.pressureIn = pressure_in_pit;
  run_par_update_arguments.pressureOut = pressure_in_outlet;

  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    run_par_update_arguments.speed = GetSpeed(i);
    run_par_update_arguments.torque = GetTorque(i);
    run_par_update_arguments.pumpState = mPumpState[i];
    run_par_update_arguments.pumpRampUpPassed = (mAwaitPumpRampUpTime[i] <= 0);
    run_par_update_arguments.trainingState = mTrainingEnabledState[i];

    // run learn and parameter update
    UpdateFlowLearnControl((PUMP_TYPE) i);
    UpdateTrainingFrequency((PUMP_TYPE) i);
    mpParameterIdentifiers[i]->RunParUpdate(&run_par_update_arguments);
  }

  AFC_RUN_SYSTEM_FLOW_STRUCT run_system_flow_arguments;
  run_system_flow_arguments.pitArea = GetPitArea();
  run_system_flow_arguments.pressureIn = pressure_in_pit;
  run_system_flow_arguments.pressureOut = pressure_in_outlet;

  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    run_system_flow_arguments.pump[i].speed = GetSpeed(i);
    run_system_flow_arguments.pump[i].torque = GetTorque(i);
    run_system_flow_arguments.pump[i].pumpState = mPumpState[i];
    run_system_flow_arguments.pump[i].pumpRampUpPassed = (mAwaitPumpRampUpTime[i] <= 0);

    t_calcvar* theta_Q = mpParameterIdentifiers[i]->GetPumpFlowParameters();

    for (int j = 0; j < FLOW_DIM; j++)
    {
      run_system_flow_arguments.pump[i].theta[j] = theta_Q[j];
    }
  }
  // run system flow calculation
  mpSystemFlow->RunSystemFlow(&run_system_flow_arguments);
}


/*****************************************************************************
 * Function - CalculateTotalFlow
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::CalculateTotalFlow()
{
  bool flow_is_available = false;
  bool at_least_one_pump_present = false;
  bool learning_completed = true;

  float calculated_flow = 0.0f;

  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    if (mPumpState[i] != AFC_PUMP_NOT_THERE)
    {
      at_least_one_pump_present = true;

      if (mIsAbleToLearn[i] && mPumpLearningState[i] == FLOW_LEARNING_IDLE)
      {
        calculated_flow += mpSystemFlow->GetPumpFlow(i);
        flow_is_available = true;
      }
      else
      {
        learning_completed = false;

        if (mPumpState[i] == AFC_PUMP_RUNNING)
        {
          flow_is_available = false;
          //break to set flow NOT_AVAILABLE if training is incomplete for a running pump
          break;
        }
        else
        {
          flow_is_available = true;
        }
      }
    }
  }

  if (learning_completed)
  {
    mpIsLearningInProgress->SetValue(false);
  }

  if (flow_is_available && at_least_one_pump_present)
  {
    mpCalculatedFlow->SetValue(calculated_flow);
  }
  else
  {
    mpCalculatedFlow->SetQuality(DP_NOT_AVAILABLE);
  }

  // HRG
  estimated_in_flow = mpSystemFlow->GetInflow();
}


/*****************************************************************************
 * Function - HandleParameterStorage
 * DESCRIPTION:
 * store parameters of flow algorithm
 *****************************************************************************/
void AdvancedFlowCtrl::HandleParameterStorage()
{
  if (mNewParametersLearnedEvent
    || mSecondsSinceParameterStorage >= TIME_TO_STORE_PARAMETERS)
  {
    StoreFlowParametersInFlash();

    mNewParametersLearnedEvent = false;
    mSecondsSinceParameterStorage = 0;
  }
}


/*****************************************************************************
 * Function - IsTrainingFinishedForPump
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AdvancedFlowCtrl::IsTrainingFinishedForPump(PUMP_TYPE pumpNo)
{
  return (mpParameterIdentifiers[pumpNo]->GetPumpModelConfidence() < 0);
}


/*****************************************************************************
 * Function - GetPitArea
 * DESCRIPTION:
 *****************************************************************************/
float AdvancedFlowCtrl::GetPitArea()
{
  return mpPitArea->GetValue();
}


/*****************************************************************************
 * Function - GetPressureDifference
 * DESCRIPTION:
 *****************************************************************************/
float AdvancedFlowCtrl::GetPressureDifference()
{
  if (mpAiOutletPressure->IsAvailable() && mpSurfaceLevel->IsAvailable())
  {
    return GetPressureInOutlet() - GetPressureInPit();
  }
  else
  {
    return 0.0f;
  }
}


/*****************************************************************************
 * Function - GetPressureInPit
 * DESCRIPTION:
 * convert surface level (meter water column) to pressure (Pascal)
 *****************************************************************************/
float AdvancedFlowCtrl::GetPressureInPit()
{
  if (mpSurfaceLevel->IsAvailable())
  {
    // conversion factor is found in application\display\MPCUnits\Units.xls (cell AX43)
    return mpSurfaceLevel->GetValue() * 9806.7098;
  }
  else
  {
    return 0.0f;
  }
}


/*****************************************************************************
 * Function - GetPressureInOutlet
 * DESCRIPTION:
 *****************************************************************************/
float AdvancedFlowCtrl::GetPressureInOutlet()
{
  if (mpAiOutletPressure->IsAvailable())
  {
    return mpAiOutletPressure->GetValue();
  }
  else
  {
    return 0.0f;
  }
}


/*****************************************************************************
 * Function - GetTorque
 * DESCRIPTION:
 *****************************************************************************/
float AdvancedFlowCtrl::GetTorque(int pumpIndex)
{
  float torque = 0.0f;
  if (pumpIndex < NO_OF_PUMPS && mpPower[pumpIndex]->IsAvailable())
  {
    float power = mpPower[pumpIndex]->GetValue();
    float speed = GetSpeed(pumpIndex);
    torque = mpMotorModel->GetShaftTorque(power, speed);
  }
  return torque;
}


/*****************************************************************************
 * Function - GetSpeed
 * DESCRIPTION:
 *****************************************************************************/
float AdvancedFlowCtrl::GetSpeed(int pumpIndex)
{
  float speed = 0.0f;
  if (pumpIndex < NO_OF_PUMPS && mpPower[pumpIndex]->IsAvailable())
  {
    float power = mpPower[pumpIndex]->GetValue();
    float frequency = GetFrequency(pumpIndex);
    speed = mpMotorModel->GetShaftSpeed(power, frequency);
  }
  return speed;
}


/*****************************************************************************
 * Function - GetFrequency
 * DESCRIPTION:
 *****************************************************************************/
float AdvancedFlowCtrl::GetFrequency(int pumpIndex)
{
  float frequency = 0.0f;
  if (pumpIndex < NO_OF_PUMPS && mPumpState[pumpIndex] == AFC_PUMP_RUNNING)
  {
    if (mpVfdInstalled[pumpIndex]->GetValue() == true)
    {
      if (mpCUEFrequency[pumpIndex]->GetQuality() == DP_AVAILABLE)
      {
        frequency = mpCUEFrequency[pumpIndex]->GetValue();
      }
      else
      {
        frequency = mpVfdFrequency[pumpIndex]->GetValue();
      }
    }
    else
    {
      frequency = mpVfdFrequency[pumpIndex]->GetMaxValue();
    }
  }
  return frequency;
}


/*****************************************************************************
 * Function - LoadFlowParametersInFlash
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::LoadFlowParametersFromFlash()
{
  if (mpLearningParameters[0]->GetSize() < PI_NUMBER_OF_STORED_PARAMETERS)
  {
    FatalErrorOccured("AFC: unexpected number of flow parameters");
  }

  double parameters[PI_NUMBER_OF_STORED_PARAMETERS];

  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    if (mpLearningParameters[i]->GetValue(0) != 0.0f)
    {
      for (int j = 0; j < PI_NUMBER_OF_STORED_PARAMETERS; j++)
      {
        parameters[j] = mpLearningParameters[i]->GetValue(j);
      }

      mpParameterIdentifiers[i]->SetFromPersistentStorage(parameters, PI_NUMBER_OF_STORED_PARAMETERS);
    }
  }
}


/*****************************************************************************
 * Function - StoreFlowParametersInFlash
 * DESCRIPTION:
 *
 *****************************************************************************/
void AdvancedFlowCtrl::StoreFlowParametersInFlash()
{
  if (mpLearningParameters[0]->GetSize() < PI_NUMBER_OF_STORED_PARAMETERS)
  {
    FatalErrorOccured("AFC: unexpected number of flow parameters");
  }

  double parameters[PI_NUMBER_OF_STORED_PARAMETERS];

  for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
  {
    int ret = mpParameterIdentifiers[i]->GetForPersistentStorage(parameters, PI_NUMBER_OF_STORED_PARAMETERS);

    if (ret == PI_PARAMETER_STORE_OK)
    {
      for (int j = 0; j < PI_NUMBER_OF_STORED_PARAMETERS; j++)
      {
        mpLearningParameters[i]->SetValue(j, parameters[j]);
      }
    }
    else
    {
      FatalErrorOccured("AFC: unable to get flow parameters");
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
