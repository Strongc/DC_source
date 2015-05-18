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
/* CLASS NAME       : VfdMasterCtrl                                         */
/*                                                                          */
/* FILE NAME        : VfdMasterCtrl.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 04-05-2009 dd-mm-yyyy                                 */
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
#include <VfdMasterCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

// VFD master state
typedef enum
{
  FIRST_VFD_MASTER_STATE = 0,
  VFD_MASTER_STATE_NOT_USED = FIRST_VFD_MASTER_STATE,
  VFD_MASTER_STATE_AUTO,
  VFD_MASTER_STATE_FORCED_ON,
  VFD_MASTER_STATE_FORCED_OFF,
  VFD_MASTER_STATE_ANTI_SEIZING,

  // insert new items above
  NO_OF_VFD_MASTER_STATE,
  LAST_VFD_MASTER_STATE = NO_OF_VFD_MASTER_STATE - 1
} VFD_MASTER_STATE_TYPE;


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
VfdMasterCtrl::VfdMasterCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
VfdMasterCtrl::~VfdMasterCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdMasterCtrl::InitSubTask()
{
  mLinearScale = 0.0f;
  mLinearOffset = mpVfdEconomyFrequency->GetValue();
  mpVfdEconomyLevel.SetUpdated();
  mpVfdEconomyMaxLevel.SetUpdated();
  mpVfdEconomyFrequency.SetUpdated();
  mpVfdMaxFrequency.SetUpdated();

  mMinimumRegulationFrequency = mpVfdEconomyFrequency->GetValue();;
  MinimumRegulationSwitchTime = 0;
  mSkipRunCounter = 0;

  mpVfdFrequency->SetQuality(DP_NEVER_AVAILABLE);
  mpVfdState->SetQuality(DP_NEVER_AVAILABLE);
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdMasterCtrl::RunSubTask()
{
  if (mpVfdInstalled->GetValue() == false)
  {
    // Vfd not installed
    if (mpVfdFrequency->GetQuality() != DP_NEVER_AVAILABLE)
    {
      mpVfdFrequency->SetValue(0.0f);
      mpVfdFrequency->SetQuality(DP_NEVER_AVAILABLE);
      mpVfdState->SetValue(VFD_OPERATION_MODE_NOT_IN_CONTROL);
      mpVfdState->SetQuality(DP_NEVER_AVAILABLE);
      mpVfdAutoOprRequest->SetQuality(DP_NOT_AVAILABLE);
    }
  }
  else
  {
    // Vfd installed
    VFD_MASTER_STATE_TYPE mVfdMasterState = VFD_MASTER_STATE_NOT_USED;
    bool  pump_start_request = mpVfdPumpStartRequest->GetValue();
    float vfd_frequency = 0.0f;
    bool  vfd_start     = false;
    bool  vfd_reverse   = false;

    // Update master state:
    if ( (mpPumpReadyForAutoOpr->GetValue() == true)
      && (mpHighLevelSwitchActivated->GetValue() == false)
      && (mpSurfaceLevel->GetQuality() == DP_AVAILABLE || mpVfdRunMode->GetValue() == VFD_RUN_MODE_PID_REGULATION) )
    {
      // Auto operation, but special check for anti seizing
      if (pump_start_request == true && mpAntiSeizingRequestPump->GetValue() == true)
      {
        mVfdMasterState = VFD_MASTER_STATE_ANTI_SEIZING;
      }
      else
      {
        mVfdMasterState = VFD_MASTER_STATE_AUTO;
      }
    }
    else
    {
      // NOT auto operation
      if (pump_start_request == true)
      {
        mVfdMasterState = VFD_MASTER_STATE_FORCED_ON;
      }
      else
      {
        mVfdMasterState = VFD_MASTER_STATE_FORCED_OFF;
      }
    }

    // Update vfd output for actual master state:
    switch (mVfdMasterState)
    {
      case VFD_MASTER_STATE_AUTO:
        mpVfdState->SetValue(HandleAutoOperation(vfd_frequency, vfd_start, vfd_reverse));
        break;

      case VFD_MASTER_STATE_NOT_USED:
      case VFD_MASTER_STATE_FORCED_OFF:
        vfd_frequency = 0.0f;
        vfd_start = false;
        vfd_reverse = false;
        mpVfdState->SetValue(VFD_OPERATION_MODE_NOT_IN_CONTROL);
        break;

      case VFD_MASTER_STATE_FORCED_ON:
      case VFD_MASTER_STATE_ANTI_SEIZING:
        if (mpVfdRunMode->GetValue() == VFD_RUN_MODE_FIXED_FREQUENCY)
        {
          vfd_frequency = mpVfdFixedFrequency->GetValue();
        }
        else
        {
          vfd_frequency = mpVfdMaxFrequency->GetValue();
        }
        vfd_start = true;
        vfd_reverse = false;
        mpVfdState->SetValue(VFD_OPERATION_MODE_NOT_IN_CONTROL);
        break;

      default:
        // Nothing
        break;
    }

    // Validate and update frequency
    if (vfd_start == true)
    {
      if (vfd_frequency < mpVfdMinFrequency->GetValue())
      {
        vfd_frequency = mpVfdMinFrequency->GetValue();
      }
      if (vfd_frequency > mpVfdMaxFrequency->GetValue())
      {
        vfd_frequency = mpVfdMaxFrequency->GetValue();
      }
      if (mpVfdReactionMode->GetValue() == VFD_REACTION_MODE_MAX_SPEED && mpNoOfRunningPumps->GetValue() > 1)
      {
        vfd_frequency = mpVfdMaxFrequency->GetValue();
      }
    }
    else
    {
      vfd_frequency = 0.0f;
    }
    mpVfdFrequency->SetValue(vfd_frequency);
    mpVfdStart->SetValue(vfd_start);
    mpVfdReverse->SetValue(vfd_reverse);

    // Set trigger for use by VfdFlushStateCtrl
    if (mVfdMasterState == VFD_MASTER_STATE_AUTO)
    {
      mpVfdAutoOprRequest->SetValue(pump_start_request);
    }
    else
    {
      mpVfdAutoOprRequest->SetQuality(DP_NOT_AVAILABLE);
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void VfdMasterCtrl::ConnectToSubjects()
{
  mpVfdFlushRequest->Subscribe(this);
  mpVfdEconomyLevel.Subscribe(this);
  mpVfdEconomyMaxLevel.Subscribe(this);
  mpVfdEconomyFrequency.Subscribe(this);
  mpVfdMaxFrequency.Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class.
 *
 *****************************************************************************/
void VfdMasterCtrl::Update(Subject* pSubject)
{
  mpVfdFlushRequest.Update(pSubject);
  mpVfdEconomyLevel.Update(pSubject);
  mpVfdEconomyMaxLevel.Update(pSubject);
  mpVfdEconomyFrequency.Update(pSubject);
  mpVfdMaxFrequency.Update(pSubject);
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdMasterCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void VfdMasterCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_VFM_VFD_INSTALLED:
      mpVfdInstalled.Attach(pSubject);
      break;
    case SP_VFM_VFD_RUN_MODE:
      mpVfdRunMode.Attach(pSubject);
      break;
    case SP_VFM_VFD_REACTION_MODE:
      mpVfdReactionMode.Attach(pSubject);
      break;
    case SP_VFM_VFD_FIXED_FREQUENCY:
      mpVfdFixedFrequency.Attach(pSubject);
      break;
    case SP_VFM_VFD_ECONOMY_FREQUENCY:
      mpVfdEconomyFrequency.Attach(pSubject);
      break;
    case SP_VFM_VFD_ECONOMY_LEVEL:
      mpVfdEconomyLevel.Attach(pSubject);
      break;
    case SP_VFM_VFD_ECONOMY_MAX_LEVEL:
      mpVfdEconomyMaxLevel.Attach(pSubject);
      break;
    case SP_VFM_VFD_ECONOMY_MIN_FREQUENCY:
      mpVfdEconomyMinFrequency.Attach(pSubject);
      break;
    case SP_VFM_VFD_MIN_FREQUENCY:
      mpVfdMinFrequency.Attach(pSubject);
      break;
    case SP_VFM_VFD_MAX_FREQUENCY:
      mpVfdMaxFrequency.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_VFM_VFD_PUMP_START_REQUEST:
      mpVfdPumpStartRequest.Attach(pSubject);
      break;
    case SP_VFM_READY_FOR_AUTO_OPR_PUMP:
      mpPumpReadyForAutoOpr.Attach(pSubject);
      break;
    case SP_VFM_ANTI_SEIZING_REQUEST_PUMP:
      mpAntiSeizingRequestPump.Attach(pSubject);
      break;
    case SP_VFM_HIGH_LEVEL_SWITCH_ACTIVATED:
      mpHighLevelSwitchActivated.Attach(pSubject);
      break;
    case SP_VFM_SURFACE_LEVEL:
      mpSurfaceLevel.Attach(pSubject);
      break;
    case SP_VFM_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;
    case SP_VFM_VFD_FLUSH_REQUEST:
      mpVfdFlushRequest.Attach(pSubject);
      break;
    case SP_VFM_VFD_PID_FREQUENCY:
      mpVfdPidFrequency.Attach(pSubject);
      break;
    case SP_VFM_VFD_ENERGY_TEST_FREQUENCY:
      mpVfdEnergyTestFrequency.Attach(pSubject);
      break;
    case SP_VFM_VFD_FLOW_TRAINING_FREQUENCY:
      mpVfdFlowTrainingFrequency.Attach(pSubject);
      break;

    // Outputs:
    case SP_VFM_VFD_FREQUENCY:
      mpVfdFrequency.Attach(pSubject);
      break;
    case SP_VFM_VFD_START:
      mpVfdStart.Attach(pSubject);
      break;
    case SP_VFM_VFD_REVERSE:
      mpVfdReverse.Attach(pSubject);
      break;
    case SP_VFM_VFD_STATE:
      mpVfdState.Attach(pSubject);
      break;
    case SP_VFM_VFD_AUTO_OPR_REQUEST:
      mpVfdAutoOprRequest.Attach(pSubject);
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
 * Function - HandleAutoOperation
 * DESCRIPTION: Find the the VFD control values for auto operation mode.
 *  1) Check if a flush is about to run and update the vfd state
 *  2) Calculate the vfd frequency for normal run base on actual run mode
 *
 *****************************************************************************/
VFD_OPERATION_MODE_TYPE VfdMasterCtrl::HandleAutoOperation(float &vfdFrequency, bool &vfdStart, bool &vfdReverse)
{
  VFD_OPERATION_MODE_TYPE vfd_state = mpVfdState->GetValue();

  // Update Vfd state based on flush request
  if (mpVfdFlushRequest.IsUpdated())
  {
    // Special check to reject run flush when level is below economy level. (requirement)
    if ( (mpVfdFlushRequest->GetValue() == VFD_OPERATION_MODE_RUN_FLUSH)
      && (mpVfdRunMode->GetValue() == VFD_RUN_MODE_LINEAR_REGULATION || mpVfdRunMode->GetValue() == VFD_RUN_MODE_MINIMUM_REGULATION)
      && (mpSurfaceLevel->GetValue() < mpVfdEconomyLevel->GetValue()) )
    {
      vfd_state = VFD_OPERATION_MODE_NORMAL;
    }
    else
    {
      vfd_state = mpVfdFlushRequest->GetValue();
    }
  }

  // Determine vfd output based on the actual vfd state:
  switch (vfd_state)
  {
    case VFD_OPERATION_MODE_NORMAL:
      // First make a special check for energy test or flow training
      if (mpVfdEnergyTestFrequency->GetQuality() == DP_AVAILABLE)
      {
        vfdFrequency = mpVfdEnergyTestFrequency->GetValue();
      }
      else if (mpVfdFlowTrainingFrequency->GetQuality() == DP_AVAILABLE)
      {
        vfdFrequency = mpVfdFlowTrainingFrequency->GetValue();
      }
      else switch (mpVfdRunMode->GetValue()) // vfdFrequency as determined by run mode
      {
        case VFD_RUN_MODE_FIXED_FREQUENCY:
          vfdFrequency = mpVfdFixedFrequency->GetValue();
          break;

        case VFD_RUN_MODE_LINEAR_REGULATION:
          mSkipRunCounter++;
          // Just run this function every 10th time i.e. every 100 ms. This is to reduce task load in VfdSignalCtrl.
          if (mSkipRunCounter >= 10)
          {
            vfdFrequency = LinearRegulation();
            mSkipRunCounter = 0;
          }
          else
          {
            vfdFrequency =  mpVfdFrequency->GetValue();
          }
          break;

        case VFD_RUN_MODE_MINIMUM_REGULATION:
          vfdFrequency = MinimumRegulation();
          break;

        case VFD_RUN_MODE_PID_REGULATION:
          if (mpVfdPidFrequency->GetQuality() == DP_AVAILABLE)
          {
            vfdFrequency = mpVfdPidFrequency->GetValue();
          }
          else
          {
            // Use the existing frequency if the pid output is not yet ready
            vfdFrequency =  mpVfdFrequency->GetValue();
          }
          break;

        default:
          vfdFrequency = mpVfdMaxFrequency->GetValue();
          break;
      }
      vfdStart = true;
      vfdReverse = false;
      break;

    case VFD_OPERATION_MODE_REV_FLUSH:
      vfdFrequency = mpVfdMaxFrequency->GetValue();
      vfdStart = true;
      vfdReverse = true;
      break;

    case VFD_OPERATION_MODE_START_FLUSH:
    case VFD_OPERATION_MODE_RUN_FLUSH:
    case VFD_OPERATION_MODE_STOP_FLUSH:
      vfdFrequency = mpVfdMaxFrequency->GetValue();
      vfdStart = true;
      vfdReverse = false;
      break;

    case VFD_OPERATION_MODE_STOP:
      vfdFrequency = 0.0f;
      vfdStart = false;
      vfdReverse = false;
      break;

    case VFD_OPERATION_MODE_NOT_IN_CONTROL:
    default:
      // Nothing
      break;
  }

  return vfd_state;
}

/*****************************************************************************
 * Function - LinearRegulation
 * DESCRIPTION: Calculate the VFD frequency for run mode linear regulation
 *
 * Formula:
 *  f = a x + b
 *  a = (Max frequency - Economy frequency) / (Economy max level - Economy level)
 *  b = Economy frequency - (Economy level * a)
 * Example:
 *  mf = 50 Hz
 *  ef = 40 Hz
 *  ml = 2 m
 *  el = 1 m
 *  a = (50-40) / (2-1) = 10 Hz/m
 *  b = 40 - (1*10) = 30 Hz
 * Some check values:
 *  f(1.0m) = 10*1.0 + 30 = 40 Hz
 *  f(1.5m) = 10*1.5 + 30 = 45 Hz
 *  f(2.0m) = 10*2.0 + 30 = 50 Hz
 *
 *****************************************************************************/
float VfdMasterCtrl::LinearRegulation(void)
{
  float surface_level = mpSurfaceLevel->GetValue();
  float vfd_frequency = mpVfdEconomyFrequency->GetValue();

  if (surface_level > mpVfdEconomyMaxLevel->GetValue())
  {
    vfd_frequency = mpVfdMaxFrequency->GetValue();
  }
  else if (surface_level > mpVfdEconomyLevel->GetValue())
  {
    // First check if configuration for lninear regulation has changed.
    bool  update_constants = false;
    update_constants |= mpVfdEconomyLevel.IsUpdated();
    update_constants |= mpVfdEconomyMaxLevel.IsUpdated();
    update_constants |= mpVfdEconomyFrequency.IsUpdated();
    update_constants |= mpVfdMaxFrequency.IsUpdated();
    if (update_constants == true)
    {
      if ( (mpVfdEconomyMaxLevel->GetValue() > mpVfdEconomyLevel->GetValue())
        && (mpVfdMaxFrequency->GetValue() > mpVfdEconomyFrequency->GetValue()) )
      {
        mLinearScale  = (mpVfdMaxFrequency->GetValue() - mpVfdEconomyFrequency->GetValue()) /
                        (mpVfdEconomyMaxLevel->GetValue() - mpVfdEconomyLevel->GetValue());
        mLinearOffset = mpVfdEconomyFrequency->GetValue() - mpVfdEconomyLevel->GetValue() * mLinearScale;
      }
      else
      {
        // Invalid configuration. Just use the economy frequency.
        mLinearScale  = 0.0f;
        mLinearOffset = mpVfdEconomyFrequency->GetValue();
      }
    }

    // Then calculate the frequency. It's just that simple.
    vfd_frequency = mLinearScale*surface_level + mLinearOffset;
  }

  return vfd_frequency;
}

/*****************************************************************************
 * Function - MinimumRegulation
 * DESCRIPTION: Calculate the VFD frequency for run mode minimum regulation
 *
 *****************************************************************************/
float VfdMasterCtrl::MinimumRegulation(void)
{
  float surface_level = mpSurfaceLevel->GetValue();

  // TBD: How to use hysteresis !!!
  MinimumRegulationSwitchTime += 10;

  if (surface_level > mpVfdEconomyMaxLevel->GetValue())
  {
    mMinimumRegulationFrequency = mpVfdMaxFrequency->GetValue();
    MinimumRegulationSwitchTime = 0;
  }
  else if (surface_level > mpVfdEconomyLevel->GetValue())
  {
    if (MinimumRegulationSwitchTime > 10000)
    {
      mMinimumRegulationFrequency = mpVfdEconomyFrequency->GetValue();
    }
  }
  else
  {
    mMinimumRegulationFrequency = mpVfdEconomyMinFrequency->GetValue();
    MinimumRegulationSwitchTime = 0;
  }

  return mMinimumRegulationFrequency;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
