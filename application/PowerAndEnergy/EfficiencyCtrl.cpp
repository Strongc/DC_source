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
/* CLASS NAME       : EfficiencyCtrl                                        */
/*                                                                          */
/* FILE NAME        : EfficiencyCtrl.cpp                                    */
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
#include <EfficiencyCtrl.h>
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
EfficiencyCtrl::EfficiencyCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
EfficiencyCtrl::~EfficiencyCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void EfficiencyCtrl::InitSubTask()
{
  mStartDelay = 0;
  mOldEnergy  = 0;
  mOldVolume  = 0;
  mOldHour    = -1;

  mpEfficiencyOneHourAvg->SetQuality(DP_NEVER_AVAILABLE);
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void EfficiencyCtrl::RunSubTask()
{
  bool update_efficiency = false;


  if (mpFreeRunningEnergyWh->GetQuality() != DP_AVAILABLE || mpFreeRunningVolumeLitre->GetQuality() != DP_AVAILABLE)
  {
    if (mpEfficiencyOneHourAvg->GetQuality() != DP_NEVER_AVAILABLE)
    {
      mpEfficiencyOneHourAvg->SetValue(0);
      mpEfficiencyOneHourAvg->SetQuality(DP_NEVER_AVAILABLE);
    }
  }
  else
  {
    if (mpEfficiencyOneHourAvg->GetQuality() == DP_NEVER_AVAILABLE)
    {
      // Initialize efficiency calculation
      mpEfficiencyOneHourAvg->SetQuality(DP_NOT_AVAILABLE);
      mOldEnergy = mpFreeRunningEnergyWh->GetValue();
      mOldVolume = mpFreeRunningVolumeLitre->GetValue();
      mStartDelay = 600; // First update after a 10 minutes
    }

    if (mStartDelay > 0) // Make the first calculation when start delay has elapsed
    {
      mStartDelay--;
      if (mStartDelay == 0)
      {
        mOldHour = -1;
        update_efficiency = true;
      }
    }
    else // Check hourly update (at hh:59:58 or hh:59:59)
    {
      mStartDelay = -1;
      if ( (ActTime::GetInstance()->GetTime(MINUTES) == 59)
        && (ActTime::GetInstance()->GetTime(SECONDS) >= 58)
        && (ActTime::GetInstance()->GetTime(HOURS) != mOldHour) )
      {
        mOldHour = ActTime::GetInstance()->GetTime(HOURS);
        update_efficiency = true;
      }
    }

    if (update_efficiency == true)
    {
      I32 delta_energy = mpFreeRunningEnergyWh->GetValue() - mOldEnergy;
      I32 delta_volume = mpFreeRunningVolumeLitre->GetValue() - mOldVolume;
      float efficiency = 0.0f;

      if (delta_energy > 0 && delta_volume > 0)
      {
        efficiency = (float)delta_energy/delta_volume;  // [Wh/l] = [kWh/m3]
        efficiency /= 2.7777778e-07f;                   // [kWh/m3] --> [J/m3]
      }
      mpEfficiencyOneHourAvg->SetValue(efficiency);

      if (mStartDelay < 0) // Normal hour update, prepare accumulation for next hour
      {
        mOldEnergy = mpFreeRunningEnergyWh->GetValue();
        mOldVolume = mpFreeRunningVolumeLitre->GetValue();
      }
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void EfficiencyCtrl::ConnectToSubjects()
{
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void EfficiencyCtrl::Update(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void EfficiencyCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void EfficiencyCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Variable inputs:
    case SP_EFF_FREE_RUNNING_ENERGY_WH:
      mpFreeRunningEnergyWh.Attach(pSubject);
      break;
    case SP_EFF_FREE_RUNNING_VOLUME_LITRE:
      mpFreeRunningVolumeLitre.Attach(pSubject);
      break;

    // Outputs:
    case SP_EFF_EFFICIENCY_ONE_HOUR_AVG:
      mpEfficiencyOneHourAvg.Attach(pSubject);
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
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
