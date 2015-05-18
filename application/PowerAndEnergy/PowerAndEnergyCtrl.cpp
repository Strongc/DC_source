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
/* CLASS NAME       : PowerAndEnergyCtrl                                    */
/*                                                                          */
/* FILE NAME        : PowerAndEnergyCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 ****************************************************************************/
 #include <Math.h>

/*****************************************************************************
  PROJECT INCLUDES
 ****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <PowerAndEnergyCtrl.h>

/*****************************************************************************
  DEFINES
 ****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 ****************************************************************************/



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
PowerAndEnergyCtrl::PowerAndEnergyCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PowerAndEnergyCtrl::~PowerAndEnergyCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::InitSubTask()
{
  mDeltaEnergy = 0.0f;
  mEnergyBasedPower = 0.0f;
  mDeltaPulses = 0;
  mOldPulses = 0;
  mPulsesReady = false;
  mIncrementWh = 0.0f;
  mIncrementKWh = 0;
  mPulsesForUpdate = 0;
  mTimeSinceUpdate = 0;
  mPumpHasBeenStopped = true;

  mpPulseEnergyUnit.SetUpdated(); // To force the first init of display scaling

  // Update total_energy variable.
  float energy = mpTotalEnergyKwh->GetValue();
  mpTotalEnergy->SetValue(energy * 3600000);

}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::RunSubTask()
{
  CheckEnergyConfig();

  CalculateDeltaEnergy();

  UpdateAccumulatedEnergy();

  UpdateEnergyBasedPower();

  UpdateSystemPower();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::ConnectToSubjects()
{
  mpPulseEnergyRatioDisplay->Subscribe(this);
  mpPulseEnergyUnit->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::Update(Subject* pSubject)
{
  mpPulseEnergyRatioDisplay.Update(pSubject);
  mpPulseEnergyUnit.Update(pSubject);
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs
    case SP_PAE_PULSE_ENERGY_RATIO_CONFIG:
      mpPulseEnergyRatioConfig.Attach(pSubject);
      break;
    case SP_PAE_PULSE_ENERGY_RATIO_DISPLAY:
      mpPulseEnergyRatioDisplay.Attach(pSubject);
      break;
    case SP_PAE_PULSE_ENERGY_UNIT:
      mpPulseEnergyUnit.Attach(pSubject);
      break;
    case SP_PAE_POWER_UPDATE_TIME:
      mpPowerUpdateTime.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_PAE_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;
    case SP_PAE_MEASURED_VALUE_POWER:
      mpMeasuredValuePower.Attach(pSubject);
      break;
    case SP_PAE_RAW_ENERGY_PULSES:
      mpRawEnergyPulses.Attach(pSubject);
      break;
    case SP_PAE_POWER_PUMP_1:
      mpPowerPump[0].Attach(pSubject);
      break;
    case SP_PAE_POWER_PUMP_2:
      mpPowerPump[1].Attach(pSubject);
      break;
    case SP_PAE_POWER_PUMP_3:
      mpPowerPump[2].Attach(pSubject);
      break;
    case SP_PAE_POWER_PUMP_4:
      mpPowerPump[3].Attach(pSubject);
      break;
    case SP_PAE_POWER_PUMP_5:
      mpPowerPump[4].Attach(pSubject);
      break;
    case SP_PAE_POWER_PUMP_6:
      mpPowerPump[5].Attach(pSubject);
      break;
    case SP_PAE_NO_OF_PUMPS:
      mpNoOfPumps.Attach(pSubject);
      break;
    case SP_PAE_OPERATION_MODE_ACTUAL_PUMP_1:
      mpOperationModeActualPump[0].Attach(pSubject);
      break;
    case SP_PAE_OPERATION_MODE_ACTUAL_PUMP_2:
      mpOperationModeActualPump[1].Attach(pSubject);
      break;
    case SP_PAE_OPERATION_MODE_ACTUAL_PUMP_3:
      mpOperationModeActualPump[2].Attach(pSubject);
      break;
    case SP_PAE_OPERATION_MODE_ACTUAL_PUMP_4:
      mpOperationModeActualPump[3].Attach(pSubject);
      break;
    case SP_PAE_OPERATION_MODE_ACTUAL_PUMP_5:
      mpOperationModeActualPump[4].Attach(pSubject);
      break;
    case SP_PAE_OPERATION_MODE_ACTUAL_PUMP_6:
      mpOperationModeActualPump[5].Attach(pSubject);
      break;

    // Outputs:
    case SP_PAE_SYSTEM_POWER:
      mpSystemPower.Attach(pSubject);
      break;
    case SP_PAE_ENERGY_PULSE_COUNTER:
      mpEnergyPulseCounter.Attach(pSubject);
      break;
    case SP_PAE_TOTAL_ENERGY_KWH:
      mpTotalEnergyKwh.Attach(pSubject);
      break;
    case SP_PAE_FREE_RUNNING_ENERGY_WH:
      mpFreeRunningEnergyWh.Attach(pSubject);
      break;

    case SP_PAE_TOTAL_ENERGY:
      mpTotalEnergy.Attach(pSubject);
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
 * Function - CheckEnergyConfig
 * DESCRIPTION: Handle the energy configuration parameters for display.
 *
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::CheckEnergyConfig()
{
  auto U32 convert_factor = 1000;

  switch (mpPulseEnergyUnit->GetValue())
  {
    case PULSE_ENERGY_UNIT_KWH:
      convert_factor = 1000;
      break;

    case PULSE_ENERGY_UNIT_MWH:
    default:
      convert_factor = 1;
      break;
  }

  if(mpPulseEnergyUnit.IsUpdated())
  {
    mpPulseEnergyRatioDisplay->SetMaxValue(mpPulseEnergyRatioConfig->GetMaxValue()/convert_factor);  // Truncate
  }
  else if (mpPulseEnergyRatioDisplay.IsUpdated()) // Important 'else if' to avoid modifying the config value when changing unit
  {
    mpPulseEnergyRatioConfig->SetValue(mpPulseEnergyRatioDisplay->GetValue()*convert_factor);
  }

  mpPulseEnergyRatioDisplay->SetValue((mpPulseEnergyRatioConfig->GetValue()+convert_factor/2)/convert_factor); // Round
  mpPulseEnergyRatioDisplay.ResetUpdated(); // Clear flag to avoid rounding config value when display value is changed from here
}

/*****************************************************************************
 * Function - CalculateDeltaEnergy
 * DESCRIPTION: If an energy counter input is present then calculate the
 *              delta energy based on the counter input.
 *              Otherwise, if a power meter is present then update the
 *              delta energy based on the power.
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::CalculateDeltaEnergy()
{
  if (mpRawEnergyPulses->GetQuality() == DP_AVAILABLE)
  {
    U8 new_pulses = (U8)mpRawEnergyPulses->GetValue();
    if (mPulsesReady == false)
    {
      // Wait until pulse input is alive
      if (new_pulses != 0)
      {
        mPulsesReady = true;
        mOldPulses = new_pulses;
      }
      mDeltaEnergy = 0.0f;
    }
    else
    {
      // Update pulses and calculate energy increment
      mDeltaPulses = new_pulses - mOldPulses;
      if (mDeltaPulses > 40)
      {
        // Some kind of start up or illegal situation
        mDeltaPulses = 0;
      }
      mOldPulses = new_pulses;
      mpEnergyPulseCounter->SetValue(mDeltaPulses+mpEnergyPulseCounter->GetValue());
      mDeltaEnergy = (1000.0f*mDeltaPulses)/mpPulseEnergyRatioConfig->GetValue();
    }
  }
  else
  {
    // No energy pulses available. Estimate the energy by integrating the actual power
    mPulsesReady = false;
    mDeltaPulses = 0;
    // If power available, then use it to estimate the energy
    if (mpSystemPower->GetQuality() == DP_AVAILABLE)
    {
      mDeltaEnergy = mpSystemPower->GetValue()/3600.0f/1000;
    }
    else
    {
      mDeltaEnergy = -1.0f; // Not available
    }
  }
}

/*****************************************************************************
 * Function - UpdateAccumulatedEnergy
 * DESCRIPTION: Update energy accumulators
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::UpdateAccumulatedEnergy()
{
  if (mDeltaEnergy < 0.0f)
  {
    // No sensor present or sensor error, set quality
    mpFreeRunningEnergyWh->SetQuality(mpSystemPower->GetQuality());
    if (mpSystemPower->GetQuality() == DP_NOT_AVAILABLE)
    {
      // The total energy shouldn't be NOT_AVAILABLE, since it still makes sense to read in case of sensor error
      mpTotalEnergyKwh->SetQuality(DP_AVAILABLE);
    }
    else
    {
      mpTotalEnergyKwh->SetQuality(DP_NEVER_AVAILABLE);
    }
  }
  else
  {
    float unit_increment;
    // Updata free running Wh's
    mIncrementWh += 1000.0f*mDeltaEnergy;
    mIncrementWh = modff(mIncrementWh, &unit_increment);
    mpFreeRunningEnergyWh->SetValue((U32)unit_increment + mpFreeRunningEnergyWh->GetValue());


    float total_joule = mpTotalEnergy->GetValue();

    total_joule += (unit_increment * 3600); // convert watthour to joule.

    mpTotalEnergy->SetValue(total_joule);


    // Update accumulated kWh's (as integer value)
    mIncrementKWh += (U32)(unit_increment+0.5f);
    if (mIncrementKWh >= 1000)
    {
      unit_increment = mIncrementKWh / 1000;
      mIncrementKWh  = mIncrementKWh % 1000;
      mpTotalEnergyKwh->SetValue(unit_increment + mpTotalEnergyKwh->GetValue());
    }
    else if (mpTotalEnergyKwh->GetQuality() != DP_AVAILABLE)
    {
      mpTotalEnergyKwh->SetQuality(DP_AVAILABLE);
    }

  }
}

/*****************************************************************************
 * Function - UpdateEnergyBasedPower
 * DESCRIPTION: Calculate a power based accumulated energy over a time period
 *
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::UpdateEnergyBasedPower()
{
  if (mpNoOfRunningPumps->GetValue() == 0)
  {
    mPulsesForUpdate = 0;
    mTimeSinceUpdate = 0;
    mEnergyBasedPower = 0;
    mPumpHasBeenStopped = true;
  }
  else
  {
    mTimeSinceUpdate++;
    mPulsesForUpdate += mDeltaPulses;
    if (mTimeSinceUpdate >= mpPowerUpdateTime->GetValue() || mPumpHasBeenStopped == true)
    {
      mEnergyBasedPower = ((3600000000.0f/mTimeSinceUpdate)*mPulsesForUpdate)/(mpPulseEnergyRatioConfig->GetValue());
      // Prepare next period
      if (mTimeSinceUpdate >= mpPowerUpdateTime->GetValue())
      {
        mPumpHasBeenStopped = false;
        mPulsesForUpdate = 0;
        mTimeSinceUpdate = 0;
      }
    }
  }
}

/*****************************************************************************
 * Function - UpdateSystemPower
 * DESCRIPTION: Update the system Power with the best Power available
 *
 *
 *****************************************************************************/
void PowerAndEnergyCtrl::UpdateSystemPower()
{
  if (mpMeasuredValuePower->GetQuality() == DP_AVAILABLE)
  {
    mpSystemPower->SetValue(mpMeasuredValuePower->GetValue());
  }
  else if (mpRawEnergyPulses->GetQuality() == DP_AVAILABLE)
  {
    mpSystemPower->SetValue(mEnergyBasedPower);
  }
  else
  {
    DP_QUALITY_TYPE pump_based_power_quality = DP_AVAILABLE;
    float pump_based_power = 0;
    int i, pumps_available = 0;

    for (i=0; i<mpNoOfPumps->GetValue(); i++)
    {
      // Check power for pumps available for operation
      if ( (mpOperationModeActualPump[i]->GetValue()==ACTUAL_OPERATION_MODE_STARTED)
        || (mpOperationModeActualPump[i]->GetValue()==ACTUAL_OPERATION_MODE_STOPPED) )
      {
        pumps_available++;
        if (mpPowerPump[i]->GetQuality()==DP_NOT_AVAILABLE)
        {
          if(pump_based_power_quality == DP_AVAILABLE)
          {
            pump_based_power_quality = DP_NOT_AVAILABLE;
          }
        }
        else if (mpPowerPump[i]->GetQuality()==DP_NEVER_AVAILABLE)
        {
          pump_based_power_quality = DP_NEVER_AVAILABLE;
        }
        else
        {
          pump_based_power+=mpPowerPump[i]->GetValue();
        }
      }
    }
    if (pumps_available == 0)
    {
      pump_based_power_quality = DP_NEVER_AVAILABLE;
    }
    if (pump_based_power_quality==DP_AVAILABLE)
    {
      mpSystemPower->SetValue(pump_based_power);
    }
    else
    {
      if (mpMeasuredValuePower->GetQuality() == DP_NOT_AVAILABLE || pump_based_power_quality==DP_NOT_AVAILABLE)
      {
        mpSystemPower->SetQuality(DP_NOT_AVAILABLE);
      }
      else
      {
        mpSystemPower->SetQuality(DP_NEVER_AVAILABLE);
      }
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
