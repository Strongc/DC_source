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
/* CLASS NAME       : BlockageDetectionCtrl                                 */
/*                                                                          */
/* FILE NAME        : BlockageDetectionCtrl.cpp                             */
/*                                                                          */
/* CREATED DATE     : 16-12-2009 dd-mm-yyyy                                 */
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
#include <BlockageDetectionCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
// Note: Remember to change size of mpNormalValues if this constant is changed
#define NO_OF_MEASUREMENTS  10
#define WAIT_AFTER_START    5  // Seconds before learn/detect

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
BlockageDetectionCtrl::BlockageDetectionCtrl()
{
  mStoringState = BDC_IDLE;

  mIntervalSize = 0.0f;
  mIntervalIndex = 0;
  mNoOfSamplesInInterval = 0;
  mBlockedTimeInSeconds = 0;
  mPumpRunningSeconds = 0;
  mPumpRunningNormal = false;
  mPumpStartedAtStartLevel = false;

  for (int i = FIRST_PARAMETER; i <= LAST_PARAMETER; i++)
  {
    mTotalValueInInterval[i] = 0.0f;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
BlockageDetectionCtrl::~BlockageDetectionCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void BlockageDetectionCtrl::InitSubTask()
{
  bool no_normal_parameters_stored = true;

  for (int i = FIRST_PARAMETER; i <= LAST_PARAMETER; i++)
  {
    // check if normal values are already stored (start level at index 0 should be higher than zero)
    if (mpNormalValues[i]->GetValue(0) <= 0.0f)
    {
      mpMinOfNormal[i]->SetQuality(DP_NOT_AVAILABLE);
      mpMaxOfNormal[i]->SetQuality(DP_NOT_AVAILABLE);
    }
    else
    {
      no_normal_parameters_stored = false;
    }
  }

  if (no_normal_parameters_stored)
  {
    mpDateOfNormalParameters->SetQuality(DP_NOT_AVAILABLE);
  }

  UpdateMinMaxValues();

  mpBlockageDetected->SetValue(false);
}


/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 * NOTE: This task must run once every second.
 *****************************************************************************/
void BlockageDetectionCtrl::RunSubTask()
{
  if ( (mpMeasuredLevel->GetQuality() == DP_AVAILABLE)
    && (mpNoOfRunningPumps->GetValue() == 1)
    && (mpPumpStarted->GetValue() == true) )
  {
    mPumpRunningSeconds++;
    if (mpVfdState->IsAvailable())
    {
      // Do not try to detect a blockage if using VFD and its state is not normal.
      switch (mpVfdState->GetValue())
      {
        case VFD_OPERATION_MODE_NORMAL:
        case VFD_OPERATION_MODE_START_FLUSH:
          // OK
          break;

        default:
          // Not OK
          mPumpRunningSeconds = 0;
          break;
      }
    }
  }
  else
  {
    mPumpRunningSeconds = 0;
  }
  mPumpRunningNormal = (mPumpRunningSeconds > WAIT_AFTER_START);

  RunParameterStorage();

  RunBlockageDetection();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void BlockageDetectionCtrl::ConnectToSubjects()
{
  mpStoreNormalParametersEvent->Subscribe(this);
  mpPumpStarted->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class.
 *
 *****************************************************************************/
void BlockageDetectionCtrl::Update(Subject* pSubject)
{
  mpStoreNormalParametersEvent.Update(pSubject);
  mpPumpStarted.Update(pSubject);
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void BlockageDetectionCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void BlockageDetectionCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_BDC_MAX_COSPHI_VAR:
      mpMaxVariation[PARAMETER_COSHPI].Attach(pSubject);
      break;
    case SP_BDC_MAX_CURRENT_VAR:
      mpMaxVariation[PARAMETER_CURRENT].Attach(pSubject);
      break;
    case SP_BDC_MAX_POWER_VAR:
      mpMaxVariation[PARAMETER_POWER].Attach(pSubject);
      break;
    case SP_BDC_DETECTION_DELAY:
      mpDetectionDelay.Attach(pSubject);
      break;

      // Variable inputs:
    case SP_BDC_STORE_PARAMETERS_EVENT:
      mpStoreNormalParametersEvent.Attach(pSubject);
      break;
    case SP_BDC_COSPHI:
      mpMeasuredValue[PARAMETER_COSHPI].Attach(pSubject);
      break;
    case SP_BDC_CURRENT:
      mpMeasuredValue[PARAMETER_CURRENT].Attach(pSubject);
      break;
    case SP_BDC_POWER:
      mpMeasuredValue[PARAMETER_POWER].Attach(pSubject);
      break;
    case SP_BDC_START_LEVEL:
      mpStartLevel.Attach(pSubject);
      break;
    case SP_BDC_STOP_LEVEL:
      mpStopLevel.Attach(pSubject);
      break;
    case SP_BDC_LEVEL:
      mpMeasuredLevel.Attach(pSubject);
      break;
    case SP_BDC_VFD_FREQUENCY:
      mpVfdFrequency.Attach(pSubject);
      break;
    case SP_BDC_VFD_STATE:
      mpVfdState.Attach(pSubject);
      break;
    case SP_BDC_PUMP_STARTED:
      mpPumpStarted.Attach(pSubject);
      break;
    case SP_BDC_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;

      // Outputs:
    case SP_BDC_MIN_COSPHI:
      mpMinOfNormal[PARAMETER_COSHPI].Attach(pSubject);
      break;
    case SP_BDC_MIN_CURRENT:
      mpMinOfNormal[PARAMETER_CURRENT].Attach(pSubject);
      break;
    case SP_BDC_MIN_POWER:
      mpMinOfNormal[PARAMETER_POWER].Attach(pSubject);
      break;
    case SP_BDC_MAX_COSPHI:
      mpMaxOfNormal[PARAMETER_COSHPI].Attach(pSubject);
      break;
    case SP_BDC_MAX_CURRENT:
      mpMaxOfNormal[PARAMETER_CURRENT].Attach(pSubject);
      break;
    case SP_BDC_MAX_POWER:
      mpMaxOfNormal[PARAMETER_POWER].Attach(pSubject);
      break;
    case SP_BDC_DATE_OF_PARAMETERS:
      mpDateOfNormalParameters.Attach(pSubject);
      break;
    case SP_BDC_BLOCKAGE_DETECTED:
      mpBlockageDetected.Attach(pSubject);
      break;

      // Local variables:
    case SP_BDC_NORMAL_COSPHI:
      mpNormalValues[PARAMETER_COSHPI].Attach(pSubject);
      break;
    case SP_BDC_NORMAL_CURRENT:
      mpNormalValues[PARAMETER_CURRENT].Attach(pSubject);
      break;
    case SP_BDC_NORMAL_POWER:
      mpNormalValues[PARAMETER_POWER].Attach(pSubject);
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
 * Function - RunParameterStorage
 * DESCRIPTION: sample and store normal pump parameters
 *
 *****************************************************************************/
 void BlockageDetectionCtrl::RunParameterStorage(void)
{
  switch (mStoringState)
  {
    case BDC_IDLE:
      if (mpStoreNormalParametersEvent.IsUpdated())
      {
        mpDateOfNormalParameters->SetQuality(DP_NOT_AVAILABLE);
        for (int i = FIRST_PARAMETER; i <= LAST_PARAMETER; i++)
        {
          for (int j = 0; j < mpNormalValues[i]->GetSize(); j++)
          {
            mpNormalValues[i]->SetValue(j, -1.0f);
          }
        }
        UpdateMinMaxValues();

        if (mpNoOfRunningPumps->GetValue() > 0)
        {
          mStoringState = BDC_WAITING_FOR_ALL_PUMPS_TO_STOP;
        }
        else
        {
          mStoringState = BDC_WAITING_FOR_PUMP_TO_START;
        }
      }
      break;

    case BDC_WAITING_FOR_ALL_PUMPS_TO_STOP:
      if (mpNoOfRunningPumps->GetValue() == 0)
      {
        mStoringState = BDC_WAITING_FOR_PUMP_TO_START;
      }
      break;

    case BDC_WAITING_FOR_PUMP_TO_START:
      if (mpPumpStarted->GetValue() == false)
      {
        mPumpStartedAtStartLevel = false;
      }
      else if (mPumpStartedAtStartLevel == false)
      {
        float level_range = mpStartLevel->GetValue() - mpStopLevel->GetValue();
        float relative_level = (mpMeasuredLevel->GetValue() - mpStopLevel->GetValue()) / level_range;
        if (relative_level > 0.90f)
        {
          mPumpStartedAtStartLevel = true;
        }
      }
      if (mPumpRunningNormal == true)
      {
        if (mPumpStartedAtStartLevel == false)
        {
          // pump is not started near the start level, wait until next pump start
          mStoringState = BDC_WAITING_FOR_ALL_PUMPS_TO_STOP;
        }
        else
        {
          mIntervalSize = (mpStartLevel->GetValue() - mpStopLevel->GetValue()) / NO_OF_MEASUREMENTS;
          mIntervalIndex = 0;
          mNoOfSamplesInInterval = 0;
          for (int i = FIRST_PARAMETER; i <= LAST_PARAMETER; i++)
          {
            mTotalValueInInterval[i] = 0.0f;
          }
          mStoringState = BDC_SAMPLE_VALUES;
        }
      }
      break;

    case BDC_SAMPLE_VALUES:
      if (mPumpRunningNormal == true)
      {
        SampleParameters(false);
      }
      else
      {
        float level_range = mpStartLevel->GetValue() - mpStopLevel->GetValue();
        float relative_level = (mpMeasuredLevel->GetValue() - mpStopLevel->GetValue()) / level_range;
        if (relative_level < 0.25f)
        {
          // pump was stopped near or below the normal stop level -> ok
          mStoringState = BDC_SAMPLING_COMPLETED;
          SampleParameters(true);
        }
        else
        {
          // abort sampling
          mStoringState = BDC_WAITING_FOR_ALL_PUMPS_TO_STOP;
        }
      }
      break;

    case BDC_SAMPLING_COMPLETED:
      mStoringState = BDC_IDLE;
      mpStoreNormalParametersEvent.ResetUpdated();
      UpdateMinMaxValues();
      for (int i = FIRST_PARAMETER; i <= LAST_PARAMETER; i++)
      {
        if (mpMinOfNormal[i]->GetQuality() == DP_AVAILABLE && mpMaxOfNormal[i]->GetQuality() == DP_AVAILABLE)
        {
          mpNormalValues[i]->SetValue(0, mpStartLevel->GetValue());
          mpNormalValues[i]->SetValue(1, mpStopLevel->GetValue());
        }
      }
      mpDateOfNormalParameters->SetValue(ActTime::GetInstance()->GetSecondsSince1Jan1970());
      break;
  }
}


 /*****************************************************************************
 * Function - SampleParameters
 * DESCRIPTION: sample values and calculate mean value to store
 *
 *****************************************************************************/
void BlockageDetectionCtrl::SampleParameters(bool samplingIsCompleted)
{
  float level = mpMeasuredLevel->GetValue();
  float start_level = mpStartLevel->GetValue();
  float start_of_next_interval = start_level - mIntervalSize*(1+mIntervalIndex);

  if ( (level < start_of_next_interval || samplingIsCompleted)
    && (mNoOfSamplesInInterval >= 2)
    && (mIntervalIndex < NO_OF_MEASUREMENTS) )
  {
    // Next interval in entered. Calculate and store mean value as new normal parameter.
    for (int i = FIRST_PARAMETER; i <= LAST_PARAMETER; i++)
    {
      float mean_value = -1.0f;
      if (mpMeasuredValue[i]->IsAvailable())
      {
        mean_value = mTotalValueInInterval[i] / (float)mNoOfSamplesInInterval;
      }

      // Note: index 0 and 1 are reserved to level range.
      mpNormalValues[i]->SetValue(2+mIntervalIndex, mean_value);
      mTotalValueInInterval[i] = 0.0;
    }

    mIntervalIndex++;
    mNoOfSamplesInInterval = 0;
  }

  if (level <= start_level)
  {
    //sample all available parameters
    for (int i = FIRST_PARAMETER; i <= LAST_PARAMETER; i++)
    {
      if (mpMeasuredValue[i]->IsAvailable())
      {
        mTotalValueInInterval[i] += mpMeasuredValue[i]->GetValue();
      }
    }
    mNoOfSamplesInInterval++;
  }
}

 /*****************************************************************************
 * Function - UpdateMinMaxValues
 * DESCRIPTION: get min and max value of stored parameters
 *
 *****************************************************************************/
void BlockageDetectionCtrl::UpdateMinMaxValues()
{
  for (int i = FIRST_PARAMETER; i <= LAST_PARAMETER; i++)
  {
    float min = mpMinOfNormal[i]->GetMaxValue();
    float max = mpMaxOfNormal[i]->GetMinValue();

    // Note: index 0 and 1 are reserved to level range.
    for (int j = 2; j < mpNormalValues[i]->GetSize(); j++)
    {
      float value = mpNormalValues[i]->GetValue(j);
      if (value >= 0.0f)
      {
        if (value > max)
        {
          max = value;
        }
        if (value < min)
        {
          min = value;
        }
      }
    }

    if (min <= max)
    {
      mpMinOfNormal[i]->SetValue(min);
      mpMaxOfNormal[i]->SetValue(max);
    }
    else
    {
      mpMinOfNormal[i]->SetQuality(DP_NOT_AVAILABLE);
      mpMaxOfNormal[i]->SetQuality(DP_NOT_AVAILABLE);
    }
  }
}


/*****************************************************************************
 * Function - RunBlockageDetection
 * DESCRIPTION: Detect if the pump is blocked:
 *              Compares actual and stored parameters when the pump is running.
 * NOTE:        Must be called once every second.
 *****************************************************************************/
void BlockageDetectionCtrl::RunBlockageDetection(void)
{
  bool blockage_detected = false;

  if (mpPumpStarted.IsUpdated() && mpPumpStarted->GetValue() == true)
  {
    // Pump just started. Clear the blockage flag.
    mpBlockageDetected->SetValue(false);
  }

  if (mStoringState == BDC_IDLE && mPumpRunningNormal == true)
  {
    float level = mpMeasuredLevel->GetValue();

    //TODO/TBD special handling of VFD controlled pumps (use frequency, not level)

    for (int i = FIRST_PARAMETER; i <= LAST_PARAMETER; i++)
    {
      if (mpNormalValues[i]->GetValue(0) > 0.0f && mpMeasuredValue[i]->GetQuality() == DP_AVAILABLE)
      {
        float max_level = mpNormalValues[i]->GetValue(0);
        float min_level = mpNormalValues[i]->GetValue(1);

        if (level > max_level)
        {
          // Use the start level when the level is above the start level
          level = max_level;
        }

        if (max_level > min_level && level > min_level)
        {
          float interval = (max_level - min_level) / NO_OF_MEASUREMENTS;
          float f_index = (max_level - level) / interval - 0.5f; // move index a half for correct interpolation
          I32   i_index = (I32)(f_index);
          if (i_index < 0 )
          {
            i_index = 0; // 100-85.1% of level range
          }
          if (i_index > NO_OF_MEASUREMENTS-2)
          {
            i_index = NO_OF_MEASUREMENTS-2; // 5-0% of level range
          }
          float interpolation = f_index - (float)i_index;

          // Note: index 0 and 1 are reserved to level range.
          float normal_ref_upper = mpNormalValues[i]->GetValue(i_index + 2);
          float normal_ref_lower = mpNormalValues[i]->GetValue(i_index + 3);
          if (normal_ref_upper >= 0.0f && normal_ref_lower >= 0.0f)
          {
            float normal_value = normal_ref_upper * (1.0f - interpolation) + normal_ref_lower * interpolation;
            float tolerance = mpMaxVariation[i]->GetValue();
            if (tolerance < 99.5f) // 100% means disabled
            {
              float min_normal_value = normal_value * (100.0f - tolerance) / 100.0f;
              float max_normal_value = normal_value * (100.0f + tolerance) / 100.0f;

              float value = mpMeasuredValue[i]->GetValue();
              if (value < min_normal_value || value > max_normal_value)
              {
                blockage_detected = true;
              }
            }
          }
        }
      }
    }
  }

  if (blockage_detected == true)
  {
    mBlockedTimeInSeconds++;
    if (mBlockedTimeInSeconds > mpDetectionDelay->GetValue())
    {
      mpBlockageDetected->SetValue(true);
    }
  }
  else
  {
    mBlockedTimeInSeconds = 0;
  }
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
