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
/* CLASS NAME       : VfdEnergyTestCtrl                                     */
/*                                                                          */
/* FILE NAME        : VfdEnergyTestCtrl.cpp                                 */
/*                                                                          */
/* CREATED DATE     : 14-05-2009 dd-mm-yyyy                                 */
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
#include <VfdEnergyTestCtrl.h>
#include <ActTime.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define FREQUENCY_STEP 1.0f
#define PI       3.14159265358979323846

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
VfdEnergyTestCtrl::VfdEnergyTestCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
VfdEnergyTestCtrl::~VfdEnergyTestCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdEnergyTestCtrl::InitSubTask()
{
  mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_IDLE;
  mEnergyTestRunState = ENERGY_TEST_RUN_STATE_IDLE;
  mTestInProgress = TEST_IN_PROGRESS_IDLE;
  mTimeSinceSelfLearning = 0;
  mPumpRunningInTestTime = 0;
  mAveragingTime = 0;
  mAccumulatedSpecificEnergy = 0.0f;
  mSpecificEnergyAtMax = 0.0f;
  mLastSpecificEnergy = 0.0f;
  mTestFrequency = 0.0f;
  mLastTestFrequency = 0.0f;
  mFilteredFlow = 0.0f;

  mpVfdEnergyTestFrequency->SetQuality(DP_NOT_AVAILABLE); // To disable request
  mpVfdEconomySelfLearningEnabled.SetUpdated();           // To trig a test after power up
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdEnergyTestCtrl::RunSubTask()
{
  bool  self_learning_request = false;
  bool  energy_test_request = false;
  bool  configuration_ok = false;
  bool  conditions_ok = false;
  bool  level_ok = false;
  float specific_energy = -1.0f;

  if (mpVfdEnergyTestStartTest.IsUpdated())
  {
    energy_test_request = true;
  }
  if (energy_test_request == true || mpVfdEconomySelfLearningEnabled->GetValue() == false)
  {
    // Special handling just to solve the situation when self learning is disabled while it is running,
    // or if an energy test has been requested while self learning is running.
    if (mTestInProgress == TEST_IN_PROGRESS_SELF_LEARNING)
    {
      mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_IDLE;
    }
  }
  else
  {
    if (mpVfdEconomySelfLearningEnabled.IsUpdated())
    {
      mTimeSinceSelfLearning = mpVfdEconomySelfLearningInterval->GetValue();
    }
    mTimeSinceSelfLearning++;
    if ( (mTimeSinceSelfLearning >= mpVfdEconomySelfLearningInterval->GetValue() )
      && (mEnergyTestMasterState == ENERGY_TEST_MASTER_STATE_IDLE)
      && (mpVfdRunMode->GetValue() == VFD_RUN_MODE_LINEAR_REGULATION || mpVfdRunMode->GetValue() == VFD_RUN_MODE_MINIMUM_REGULATION) )
    {
      self_learning_request = true;
    }
  }

  // Determine if test configuation ok
  if ( (mpVfdInstalled->GetValue() == true)
    && (mpPumpPower->GetQuality() != DP_NEVER_AVAILABLE || mpMeasuredValuePower->GetQuality() != DP_NEVER_AVAILABLE)
    && (mpMeasuredValueFlow->GetQuality() != DP_NEVER_AVAILABLE || mpEstimatedFlow->GetQuality() != DP_NEVER_AVAILABLE ) )
  {
    configuration_ok = true;
  }
  else
  {
    mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_IDLE;
  }

  // Determine if test conditions are ok
  if ( (configuration_ok == true)
    && (mpVfdState->GetValue() == VFD_OPERATION_MODE_NORMAL)
    && (mpNoOfRunningPumps->GetValue() == 1) )
  {
    float flow = -1.0f, power = -1.0f;
    if (mpMeasuredValueFlow->GetQuality() == DP_AVAILABLE)
    {
      flow = mpMeasuredValueFlow->GetValue();
    }
    else if (mpEstimatedFlow->GetQuality() == DP_AVAILABLE)
    {
      flow = mpEstimatedFlow->GetValue();
    }
    mFilteredFlow = flow;

    if (mpPumpPower->GetQuality() == DP_AVAILABLE)
    {
      power = mpPumpPower->GetValue();
    }
    else if (mpMeasuredValuePower->GetQuality() == DP_AVAILABLE)
    {
      power = mpMeasuredValuePower->GetValue();
    }
    if (flow > 0.0f && power > 0.0f)
    {
      specific_energy = power / flow; // [J/s]/[m3/s] = [J/m3]
      conditions_ok = true;
    }
  }

  // Determine if surface level is ok
  if (mTestInProgress == TEST_IN_PROGRESS_ENERGY_TEST)
  {
    // Level must be in a range of xxx cm below the start level when running the energy test
    float level = mpSurfaceLevel->GetValue();
    if ( (level < mpLowestStartLevel->GetValue())
      && (level > mpLowestStartLevel->GetValue()-mpVfdEnergyTestLevelRange->GetValue()) )
    {
      level_ok = true;
    }
  }
  else if (mTestInProgress == TEST_IN_PROGRESS_SELF_LEARNING)
  {
    // Level must be below eco.level when running the self learning test
    if (mpSurfaceLevel->GetValue() < mpVfdEconomyLevel->GetValue())
    {
      level_ok = true;
    }
  }

  // Check that we are not hanging at a low frequency during test
  if (mTestInProgress != TEST_IN_PROGRESS_IDLE && mpVfdState->GetValue() != VFD_OPERATION_MODE_STOP)
  {
    mPumpRunningInTestTime++;
    if (mPumpRunningInTestTime > 15*60)
    {
      // Too long time elapsed so just abort the test
      mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_IDLE;
      if (mTestInProgress == TEST_IN_PROGRESS_SELF_LEARNING)
      {
        // Force the economy frequency a bit above this too low frequency
        mpVfdEconomyFrequency->SetValue(mTestFrequency+2.0*FREQUENCY_STEP);
      }
    }
  }
  else
  {
    mPumpRunningInTestTime = 0;
  }

  switch (mEnergyTestMasterState)
  {
    case ENERGY_TEST_MASTER_STATE_IDLE:
      mpVfdEnergyTestFrequency->SetQuality(DP_NOT_AVAILABLE);
      mTestInProgress = TEST_IN_PROGRESS_IDLE;
      if (configuration_ok == true)
      {
        if (energy_test_request == true)
        {
          mTestInProgress = TEST_IN_PROGRESS_ENERGY_TEST;
          PrepareEnergyTest();
          mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_OK;
        }
        else if (self_learning_request == true)
        {
          mTimeSinceSelfLearning = 0;
          mTestInProgress = TEST_IN_PROGRESS_SELF_LEARNING;
          PrepareEnergyTest();
          if (mpSurfaceLevel->GetValue() < mpVfdEconomyLevel->GetValue())
          {
            mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_RAISED;
          }
          else
          {
            mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_OK;
          }
        }

        // Just update the average specific energy when test is idle and one pump is running at normal conditions
        if (conditions_ok == true)
        {
          mAveragingTime++;
          if (mAveragingTime > 0)
          {
            mAccumulatedSpecificEnergy += specific_energy;
            mpVfdEnergyTestSpecificEnergy->SetValue(mAccumulatedSpecificEnergy/(float)mAveragingTime);
          }
        }
        else
        {
          mAveragingTime = -mpVfdEnergyTestSettlingTime->GetValue();
          mAccumulatedSpecificEnergy = 0.0f;
        }
      }
      break;

    case ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_RAISED:
      mpVfdEnergyTestFrequency->SetQuality(DP_NOT_AVAILABLE);
      if (conditions_ok == true)
      {
        if (mTestInProgress == TEST_IN_PROGRESS_ENERGY_TEST)
        {
          if (level_ok)
          {
            mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_OK;
          }
        }
        else if (mTestInProgress == TEST_IN_PROGRESS_SELF_LEARNING)
        {
          float level = mpSurfaceLevel->GetValue();
          if ( (level > mpVfdEconomyLevel->GetValue())
            || (level > mpLowestStartLevel->GetValue()-0.1f) )
          {
            mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_OK;
          }
        }
      }
      break;

    case ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_OK:
      mpVfdEnergyTestFrequency->SetQuality(DP_NOT_AVAILABLE);
      if (conditions_ok == false)
      {
        mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_RAISED;
      }
      else if (level_ok == true)
      {
        StartEnergyTestStep(mTestFrequency);
        mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_RUNNING;
      }
      break;

    case ENERGY_TEST_MASTER_STATE_RUNNING:
      if (level_ok == false)
      {
        mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_OK;
      }
      // Then call RunEnergyTest to perform the actual test and to check valid stop conditions
      else if (RunEnergyTest(specific_energy) == true)
      {
        // Test done
        mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_IDLE;
      }
      else if (conditions_ok == false)
      // Conditions temporary not ok. Continue the test when ready again.
      {
        mEnergyTestMasterState = ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_RAISED;
      }
      break;
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void VfdEnergyTestCtrl::ConnectToSubjects()
{
  mpVfdEconomySelfLearningEnabled->Subscribe(this);
  mpVfdEnergyTestStartTest->Subscribe(this);
  mpMinVelocityEnabled->Subscribe(this);
  mpMinVelocity->Subscribe(this);
  mpPipeDiameter->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class.
 *
 *****************************************************************************/
void VfdEnergyTestCtrl::Update(Subject* pSubject)
{
  mpVfdEconomySelfLearningEnabled.Update(pSubject);
  mpVfdEnergyTestStartTest.Update(pSubject);

  if (mpMinVelocityEnabled.GetSubject() == pSubject
    || mpMinVelocity.GetSubject() == pSubject
    || mpPipeDiameter.GetSubject() == pSubject)
  {
    mpVfdEnergyTestStartTest.SetUpdated();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdEnergyTestCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void VfdEnergyTestCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_VFE_VFD_INSTALLED:
      mpVfdInstalled.Attach(pSubject);
      break;
    case SP_VFE_VFD_ECONOMY_SELF_LEARNING_ENABLED:
      mpVfdEconomySelfLearningEnabled.Attach(pSubject);
      break;
    case SP_VFE_VFD_ECONOMY_SELF_LEARNING_INTERVAL:
      mpVfdEconomySelfLearningInterval.Attach(pSubject);
      break;
    case SP_VFE_VFD_RUN_MODE:
      mpVfdRunMode.Attach(pSubject);
      break;
    case SP_VFE_VFD_MIN_FREQUENCY:
      mpVfdMinFrequency.Attach(pSubject);
      break;
    case SP_VFE_VFD_MAX_FREQUENCY:
      mpVfdMaxFrequency.Attach(pSubject);
      break;
    case SP_VFE_VFD_ECONOMY_LEVEL:
      mpVfdEconomyLevel.Attach(pSubject);
      break;
    case SP_VFE_VFD_ENERGY_TEST_SETTLING_TIME:
      mpVfdEnergyTestSettlingTime.Attach(pSubject);
      break;
    case SP_VFE_ENERGY_TEST_LEVEL_RANGE:
      mpVfdEnergyTestLevelRange.Attach(pSubject);
      break;
    case SP_VFE_LOWEST_START_LEVEL:
      mpLowestStartLevel.Attach(pSubject);
      break;
    case SP_VFE_MIN_VELOCITY_ENABLED:
      mpMinVelocityEnabled.Attach(pSubject);
      break;
    case SP_VFE_MIN_VELOCITY:
      mpMinVelocity.Attach(pSubject);
      break;
    case SP_VFE_PIPE_DIAMETER:
      mpPipeDiameter.Attach(pSubject);
      break;      

    // Variable inputs:
    case SP_VFE_VFD_STATE:
      mpVfdState.Attach(pSubject);
      break;
    case SP_VFE_MEASURED_VALUE_FLOW:
      mpMeasuredValueFlow.Attach(pSubject);
      break;
    case SP_VFE_ESTIMATED_FLOW:
      mpEstimatedFlow.Attach(pSubject);
      break;
    case SP_VFE_MEASURED_VALUE_POWER:
      mpMeasuredValuePower.Attach(pSubject);
      break;
    case SP_VFE_PUMP_POWER:
      mpPumpPower.Attach(pSubject);
      break;
    case SP_VFE_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;
    case SP_VFE_SURFACE_LEVEL:
      mpSurfaceLevel.Attach(pSubject);
      break;
    case SP_VFE_ENERGY_TEST_START_TEST:
      mpVfdEnergyTestStartTest.Attach(pSubject);
      break;

    // Outputs:
    case SP_VFE_VFD_ENERGY_TEST_FREQUENCY:
      mpVfdEnergyTestFrequency.Attach(pSubject);
      break;
    case SP_VFE_VFD_ENERGY_TEST_SPECIFIC_ENERGY:
      mpVfdEnergyTestSpecificEnergy.Attach(pSubject);
      break;
    case SP_VFE_ENERGY_TEST_TIMESTAMP:
      mpVfdEnergyTestTimeStamp.Attach(pSubject);
      break;
    case SP_VFE_VFD_ECONOMY_FREQUENCY:
      mpVfdEconomyFrequency.Attach(pSubject);
      break;
    case SP_VFE_VFD_ECONOMY_MIN_FREQUENCY:
      mpVfdEconomyMinFrequency.Attach(pSubject);
      break;
    case SP_VFE_SPECIFIC_ENERGY_TEST_RESULT:
      mpSpecificEnergyTestResult.Attach(pSubject);
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
 * Function - PrepareEnergyTest
 * DESCRIPTION: Prepare parameters for a new energy test
 *
 *****************************************************************************/
void VfdEnergyTestCtrl::PrepareEnergyTest(void)
{
  if (mTestInProgress == TEST_IN_PROGRESS_ENERGY_TEST)
  {
    // Start a sweep from max frequency (but round down to an even frequency)
    mTestFrequency = 2.0f*floor(0.5f*mpVfdMaxFrequency->GetValue()+0.1f);

    for (int i = 0; i < mpSpecificEnergyTestResult->GetMaxSize(); i++)
    {
      float val = ((mTestFrequency <= 50.0f && i > 10) ? -1 : 0);
      mpSpecificEnergyTestResult->SetValue(i, val);
    }

    mEnergyTestRunState = ENERGY_TEST_RUN_STATE_SWEEP_START;
  }
  else // Self learning
  {
    // Check the specific energy around the actual optimal frequency
    mTestFrequency = mpVfdEconomyFrequency->GetValue();
    mEnergyTestRunState = ENERGY_TEST_RUN_STATE_ECO_START;
  }
}

/*****************************************************************************
 * Function - StartEnergyTestStep
 * DESCRIPTION: Set parameters for starting a test step
 *
 *****************************************************************************/
void VfdEnergyTestCtrl::StartEnergyTestStep(float testFrequency)
{
  // Negative value for mAveragingTime means wait until settling time has elapsed
  if (mTestInProgress == TEST_IN_PROGRESS_ENERGY_TEST)
  {
    mAveragingTime = -mpVfdEnergyTestSettlingTime->GetValue();
  }
  else
  {
    mAveragingTime = -10;
  }
  mAccumulatedSpecificEnergy = 0.0f;
  mpVfdEnergyTestSpecificEnergy->SetQuality(DP_NOT_AVAILABLE);
  mpVfdEnergyTestFrequency->SetValue(testFrequency);
}

/*****************************************************************************
 * Function - RunEnergyStep
 * DESCRIPTION: Execute an the energy measurement for the test step and
 *              determine when the test step is finished.
 *              To be called every second during energy test.
 *
 *****************************************************************************/
bool VfdEnergyTestCtrl::RunEnergyStep(float specificEnergy)
{
  bool test_step_finished = false;

  if (mAveragingTime < 0)
  {
    // Just await settling
    mAveragingTime++;
  }
  else
  {
    // Check for test step done
    if (mTestInProgress == TEST_IN_PROGRESS_ENERGY_TEST)
    {
      if (mAveragingTime >= mpVfdEnergyTestSettlingTime->GetValue())
      {
        test_step_finished = true;
      }
    }
    else if (mAveragingTime >= 5) // Self learning (use at least a few seconds averaging)
    {
      if (mpVfdState->GetValue() == VFD_OPERATION_MODE_STOP || mpVfdState->GetValue() == VFD_OPERATION_MODE_STOP_FLUSH)
      {
        test_step_finished = true;
      }
    }

    // Update the time in test step and the average specific energy
    if (test_step_finished == false && specificEnergy > 0.0f)
    {
      mAveragingTime++;
      mAccumulatedSpecificEnergy += specificEnergy;
      mpVfdEnergyTestSpecificEnergy->SetValue(mAccumulatedSpecificEnergy/(float)mAveragingTime);
    }
  }

  return test_step_finished;
}

/*****************************************************************************
 * Function - RunEnergyTest
 * DESCRIPTION: Execute the actual energy test.
 *              To be called every second during energy test.
 *
 *****************************************************************************/
bool VfdEnergyTestCtrl::RunEnergyTest(float specificEnergy)
{
  if (RunEnergyStep(specificEnergy) == true)
  {
    // Step done
    float specific_energy_in_step = mpVfdEnergyTestSpecificEnergy->GetValue();
    switch (mEnergyTestRunState)
    {
      case ENERGY_TEST_RUN_STATE_ECO_START:
        mpVfdEnergyTestSpecificEnergy->SetValue(specific_energy_in_step);
        mLastSpecificEnergy = specific_energy_in_step;
        mLastTestFrequency = mpVfdEconomyFrequency->GetValue();
        if (mLastTestFrequency < mpVfdMaxFrequency->GetValue())
        {
          mTestFrequency = mpVfdEconomyFrequency->GetValue() + FREQUENCY_STEP;
          mEnergyTestRunState = ENERGY_TEST_RUN_STATE_ECO_INCREASING;
        }
        else
        {
          mTestFrequency = mpVfdMaxFrequency->GetValue() - FREQUENCY_STEP;
          mEnergyTestRunState = ENERGY_TEST_RUN_STATE_ECO_DECREASING;
        }
        break;
      case ENERGY_TEST_RUN_STATE_ECO_INCREASING:
        if ((specific_energy_in_step < mLastSpecificEnergy 
            || IsVelocityConstraintFulfilled() == false) 
          && mTestFrequency < mpVfdMaxFrequency->GetValue())
        {
          // Improvement, increase speed again
          mLastSpecificEnergy = specific_energy_in_step;
          mLastTestFrequency = mTestFrequency;
          mTestFrequency += FREQUENCY_STEP;
        }
        else
        {
          if (mTestFrequency >= mpVfdMaxFrequency->GetValue())
          {
            // Must have reached the maximum frequency
            mLastSpecificEnergy = specific_energy_in_step;
            mLastTestFrequency = mpVfdMaxFrequency->GetValue();
            mTestFrequency = mpVfdMaxFrequency->GetValue() - FREQUENCY_STEP;
          }
          else
          {
            // Must have passed the optimal frequency
            mTestFrequency -= 2*FREQUENCY_STEP;
          }
          mEnergyTestRunState = ENERGY_TEST_RUN_STATE_ECO_DECREASING;
        }
        break;
      case ENERGY_TEST_RUN_STATE_ECO_DECREASING:
        if ((specific_energy_in_step > mLastSpecificEnergy) 
          || IsVelocityConstraintFulfilled() == false)
        {
          // The optimal frequency has been passed, use the previous test frequency
          mpVfdEconomyFrequency->SetValue(mLastTestFrequency);
          mEnergyTestRunState = ENERGY_TEST_RUN_STATE_IDLE;
        }
        else if (mTestFrequency <= mpVfdMinFrequency->GetValue())
        {
          // Must have reached the minimum frequency 
          mpVfdEconomyFrequency->SetValue(mpVfdMinFrequency->GetValue());
          mEnergyTestRunState = ENERGY_TEST_RUN_STATE_IDLE;
        }
        else
        {
          // Improvement, decrease speed again
          mLastSpecificEnergy = specific_energy_in_step;
          mLastTestFrequency = mTestFrequency;
          mTestFrequency -= FREQUENCY_STEP;
        }        
        break;
      case ENERGY_TEST_RUN_STATE_SWEEP_START:
        {
          mSpecificEnergyAtMax = specific_energy_in_step;
          int index = (mTestFrequency - 30.0f + 0.1f)/2.0f;
          mpSpecificEnergyTestResult->SetValue(index, specific_energy_in_step);
          // Start down search max frequency and down
          mTestFrequency -= 2*FREQUENCY_STEP;
          mEnergyTestRunState = ENERGY_TEST_RUN_STATE_SWEEP_DECREASING;
        }
        break;
      case ENERGY_TEST_RUN_STATE_SWEEP_DECREASING:
        // Store log value for energy test, but only if it's within a reasonable range to make the graph look nice
        if (specific_energy_in_step < 1.5f*mSpecificEnergyAtMax)
        {
          int index = (mTestFrequency - 30.0f + 0.1f)/2.0f;
          mpSpecificEnergyTestResult->SetValue(index, specific_energy_in_step);
        }
        if (mTestFrequency <= mpVfdMinFrequency->GetValue() || specific_energy_in_step >= 1.2f*mSpecificEnergyAtMax)
        {
          // We have reached the lowest frequency or the the specific energy has went away, so the test is done
          mEnergyTestRunState = ENERGY_TEST_RUN_STATE_IDLE;
          mpVfdEnergyTestTimeStamp->SetValue(ActTime::GetInstance()->GetSecondsSince1Jan1970());
        }
        else
        {
          // Reduce speed and make one more test
          mTestFrequency -= 2*FREQUENCY_STEP;
        }
        break;

      default:
        break;
    }
    if (mEnergyTestRunState != ENERGY_TEST_RUN_STATE_IDLE)
    {
      mTestFrequency = floor(mTestFrequency+0.5f);
      if (mTestFrequency < mpVfdMinFrequency->GetValue())
      {
        mTestFrequency = mpVfdMinFrequency->GetValue();
      }
      if (mTestFrequency > mpVfdMaxFrequency->GetValue())
      {
        mTestFrequency = mpVfdMaxFrequency->GetValue();
      }
      StartEnergyTestStep(mTestFrequency);
    }
  }

  return (mEnergyTestRunState == ENERGY_TEST_RUN_STATE_IDLE);
}


/*****************************************************************************
 * Function - IsVelocityConstraintFulfilled
 * DESCRIPTION: When minimum discharge velocity constraint is disabled return true,
 *  else return true when flow is above threshold configured, false otherwise. 
 *
 *****************************************************************************/
bool VfdEnergyTestCtrl::IsVelocityConstraintFulfilled(void)
{
  if (mpMinVelocityEnabled->GetValue() == true)
  {
    float r_pipe = mpPipeDiameter->GetValue() / 2;

    // flow [m3/s] = velocity [m/s] * radius [m]^2 * PI 
    float q_min = mpMinVelocity->GetValue() * r_pipe * r_pipe * PI;

    if (q_min > 0.0f)
    {
      return mFilteredFlow >= q_min;
    }
  }

  return true;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
