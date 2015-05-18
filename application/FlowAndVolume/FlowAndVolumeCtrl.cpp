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
/* CLASS NAME       : FlowAndVolumeCtrl                                     */
/*                                                                          */
/* FILE NAME        : FlowAndVolumeCtrl.cpp                                 */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
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
#include <FlowAndVolumeCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

U16							  mcounter;

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
FlowAndVolumeCtrl::FlowAndVolumeCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
FlowAndVolumeCtrl::~FlowAndVolumeCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::InitSubTask()
{
  mDeltaVolumeInM3 = 0.0f;
  mVolumeBasedFlow = 0.0f;
  mDeltaPulses = 0;
  mOldPulses = 0;
  mPulsesReady = false;
  mIncrementLitre = 0.0f;
  mIncrementM3 = 0;
  mPulsesForUpdate = 0;
  mTimeSinceUpdate = 0;
  mcounter = 0;
  mPreviousLitre =0;


  mMaxFlow = mpSystemFlow->GetMaxValue();
  mpSystemFlow->SetMaxValue(0.01f); // To get decimals on display without analog sensor

  mpPulseVolumeUnit.SetUpdated(); // To force the first init of display scaling
  U32 total_litre=0;
  mpPumpedVolumeM3ForDisplay->SetValue(total_litre);
  mValidateOverflowVolume = false;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::RunSubTask()
{
  CheckVolumeConfig();

  CalculateDeltaVolume();

  UpdateAccumulatedVolume();

  UpdateVolumeBasedFlow();

  UpdateSystemFlow();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::ConnectToSubjects()
{
  mpPulseVolumeRatioDisplay->Subscribe(this);
  mpPulseVolumeUnit->Subscribe(this);
  mpPumpedVolumeM3ForDisplay->Subscribe(this);
  mpTotalVolumeOverRun->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::Update(Subject* pSubject)
{
  mpPulseVolumeRatioDisplay.Update(pSubject);
  mpPulseVolumeUnit.Update(pSubject);
  
  if (!mUpdatingAccumulatedVolume)
 {
    // ignore updates made from within this ctrl
    mpPumpedVolumeM3ForDisplay.Update(pSubject);
  }
  mpTotalVolumeOverRun.Update(pSubject);
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs
    case SP_FAV_PULSE_VOLUME_RATIO_CONFIG:
      mpPulseVolumeRatioConfig.Attach(pSubject);
      break;
    case SP_FAV_PULSE_VOLUME_RATIO_DISPLAY:
      mpPulseVolumeRatioDisplay.Attach(pSubject);
      break;
    case SP_FAV_PULSE_VOLUME_UNIT:
      mpPulseVolumeUnit.Attach(pSubject);
      break;
    case SP_FAV_FLOW_UPDATE_TIME:
      mpFlowUpdateTime.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_FAV_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;
    case SP_FAV_RAW_VOLUME_PULSES:
      mpRawVolumePulses.Attach(pSubject);
      break;
    case SP_FAV_MEASURED_VALUE_FLOW:
      mpMeasuredValueFlow.Attach(pSubject);
      break;
    case SP_FAV_PIT_BASED_FLOW:
      mpPitBasedFlow.Attach(pSubject);
      break;
    case SP_FAV_ADV_FLOW_BASED_FLOW:
      mpAdvFlowBasedFlow.Attach(pSubject);
      break;

    // Outputs:
    case SP_FAV_SYSTEM_FLOW:
      mpSystemFlow.Attach(pSubject);
      break;
    case SP_FAV_FLOW_QUALITY:
      mpFlowQuality.Attach(pSubject);
      break;
    case SP_FAV_VOLUME_PULSE_COUNTER:
      mpVolumePulseCounter.Attach(pSubject);
      break;
    case SP_FAV_PUMPED_VOLUME_LITRE_FOR_LOG:
      mpPumpedVolumeLitreForLog.Attach(pSubject);
      break;
    case SP_FAV_PUMPED_VOLUME_M3_FOR_DISPLAY:
      mpPumpedVolumeM3ForDisplay.Attach(pSubject);
      break;
    case SP_FAV_FREE_RUNNING_VOLUME_LITRE:
      mpFreeRunningVolumeLitre.Attach(pSubject);
      break;

	case SP_FAV_TOTAL_VOLUME_OVERRUN_CNT:
		mpTotalVolumeOverRun.Attach(pSubject);
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
 * Function - CheckVolumeConfig
 * DESCRIPTION: Handle the volume configuration parameters for display.
 *
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::CheckVolumeConfig()
{
  auto float convert_factor = 1000;

  switch (mpPulseVolumeUnit->GetValue())
  {
    case PULSE_VOLUME_UNIT_GAL:
      convert_factor = 1000000.0/3.7854118; // 1 gallon = 3.7854118 l
      break;

    case PULSE_VOLUME_UNIT_L:
      convert_factor = 1000000;
      break;

    case PULSE_VOLUME_UNIT_M3:
    default:
      convert_factor = 1000;
      break;
  }

  if(mpPulseVolumeUnit.IsUpdated())
  {
    mpPulseVolumeRatioDisplay->SetMaxValue((float)mpPulseVolumeRatioConfig->GetMaxValue()/convert_factor);
    mpPulseVolumeRatioDisplay->SetValue((float)mpPulseVolumeRatioConfig->GetValue()/convert_factor);
  }

  if (mpPulseVolumeRatioDisplay.IsUpdated())
  {
    mpPulseVolumeRatioConfig->SetValue(mpPulseVolumeRatioDisplay->GetValue()*convert_factor + 0.5f);
  }

  mpPulseVolumeRatioDisplay->SetValue((float)mpPulseVolumeRatioConfig->GetValue()/convert_factor);
}

/*****************************************************************************
 * Function - CalculateDeltaVolume
 * DESCRIPTION: If a volume counter input is present then calculate the
 *              delta volume based on the counter input.
 *              Otherwise, if a flow is present then then calculate the
 *              delta volume based on the flow.
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::CalculateDeltaVolume()
{
  if (mpRawVolumePulses->GetQuality() == DP_AVAILABLE)
  {
    U8 new_pulses = (U8)mpRawVolumePulses->GetValue();
    if (mPulsesReady == false)
    {
      // Wait until pulse input is alive
      if (new_pulses != 0)
      {
        mPulsesReady = true;
        mOldPulses = new_pulses;
      }
      mDeltaVolumeInM3 = 0.0f;
    }
    else
    {
      // Update pulses and calculate volume increment
      mDeltaPulses = new_pulses - mOldPulses;
      if (mDeltaPulses > 40)
      {
        // Some kind of start up or illegal situation
        mDeltaPulses = 0;
      }
      mOldPulses = new_pulses;
      mpVolumePulseCounter->SetValue(mDeltaPulses+mpVolumePulseCounter->GetValue());
      mDeltaVolumeInM3 = (1000.0f*mDeltaPulses)/mpPulseVolumeRatioConfig->GetValue();
    }
  }
  else
  {
    // No volume pulses.
    mPulsesReady = false;
    mDeltaPulses = 0;
    // If system flow available, then use it to estimate the volume
    if (mpSystemFlow->GetQuality() == DP_AVAILABLE)
    {
      mDeltaVolumeInM3 = mpSystemFlow->GetValue();
    }
    else
    {
      mDeltaVolumeInM3 = -1.0f; // Not available
    }
  }
}

/*****************************************************************************
 * Function - UpdateAccumulatedVolume
 * DESCRIPTION: Update volume accumulators
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::UpdateAccumulatedVolume()
{
  mUpdatingAccumulatedVolume = true; 

  if(mpTotalVolumeOverRun.IsUpdated())
  {
	  U16 count = mpTotalVolumeOverRun->GetValue();

	  if(count>99)
	  {
			mcounter = 0;
			mpTotalVolumeOverRun->SetValue(mcounter);	
	  }
	  else
	  {
			mcounter = count;
	  }

  }

  if (mpPumpedVolumeM3ForDisplay.IsUpdated())
  {
    // user must have adjusted the volume (in Adjustment of counters menu)
    U32 total_litre = (U32) mpPumpedVolumeM3ForDisplay->GetValue() * 1000;
    
    mpPumpedVolumeLitreForLog->SetValue(total_litre);	

    mpPumpedVolumeM3ForDisplay->SetValue(((float)total_litre) / 1000);
  }
  if (mDeltaVolumeInM3 < 0.0f)
  {
    DP_QUALITY_TYPE flow_quality = mpSystemFlow->GetQuality();
    // No sensor present or sensor error, set quality
    mpFreeRunningVolumeLitre->SetQuality(flow_quality);
    if (flow_quality == DP_NOT_AVAILABLE)
    {
      // The total volume shouldn't be NOT_AVAILABLE, since it still makes sense to read it in case of sensor error
      mpPumpedVolumeLitreForLog->SetQuality(DP_AVAILABLE);
      mpPumpedVolumeM3ForDisplay->SetQuality(DP_AVAILABLE);
    }
    else
    {
      mpPumpedVolumeLitreForLog->SetQuality(flow_quality);
      mpPumpedVolumeM3ForDisplay->SetQuality(flow_quality);
    }
  }
  else
  {
    float delta_volume_in_litre;
    // Update free running litres
    mIncrementLitre += 1000.0f * mDeltaVolumeInM3;
    mIncrementLitre = modff(mIncrementLitre, &delta_volume_in_litre);

    mpFreeRunningVolumeLitre->SetValue((U32)delta_volume_in_litre + mpFreeRunningVolumeLitre->GetValue());

	

    U32 total_litre = mpPumpedVolumeLitreForLog->GetValue() + (U32)delta_volume_in_litre;


	if(total_litre < mPreviousLitre)
	{		
		mcounter++;
		mpTotalVolumeOverRun->SetValue(mcounter);		

	}
	if(mcounter == 0)
	{
		  mpTotalVolumeOverRun->SetValue(0);
	}
	if(mcounter > 99)
	{	
		  mcounter	= 0;
		  mpTotalVolumeOverRun->SetValue(0);		
	}
	
	
	mpPumpedVolumeLitreForLog->SetValue(total_litre);
    mpPumpedVolumeM3ForDisplay->SetValue(((float)total_litre) / 1000);

	mPreviousLitre = total_litre;

  }
/*  if (mpPumpedVolumeM3ForDisplay.IsUpdated())
  {
    // user must have adjusted the volume (in Adjustment of counters menu)
    U32 total_litre = (U32) mpPumpedVolumeM3ForDisplay->GetValue() * 1000;
    
    mpPumpedVolumeLitreForLog->SetValue(total_litre);
    mpPumpedVolumeM3ForDisplay->SetValue(((float)total_litre) / 1000);
  }*/

  mUpdatingAccumulatedVolume = false;
}

/*****************************************************************************
 * Function - UpdateVolumeBasedFlow
 * DESCRIPTION: Calculate a flow based accumulated volume over a time period
 *
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::UpdateVolumeBasedFlow()
{
  mTimeSinceUpdate++;
  mPulsesForUpdate += mDeltaPulses;
  if (mTimeSinceUpdate >= mpFlowUpdateTime->GetValue())
  {
    mVolumeBasedFlow = ((1000.0f/mTimeSinceUpdate)*mPulsesForUpdate)/mpPulseVolumeRatioConfig->GetValue();
    // Prepare next period
    mPulsesForUpdate = 0;
    mTimeSinceUpdate = 0;
  }
}

/*****************************************************************************
 * Function - UpdateSystemFlow
 * DESCRIPTION: Update the system flow with the best flow available
 *
 *
 *****************************************************************************/
void FlowAndVolumeCtrl::UpdateSystemFlow()
{
  FLOW_QUALITY_TYPE flow_quality = FLOW_QUALITY_NO_FLOW_MEASUREMENT;

  if (mpMeasuredValueFlow->GetQuality() != DP_NEVER_AVAILABLE)
  {
    flow_quality = FLOW_QUALITY_FLOW_METER;
    mpSystemFlow->CopyValues(mpMeasuredValueFlow.GetSubject());
  }
  else
  {
    float flow = -1.0f;
    if (mpRawVolumePulses->GetQuality() == DP_AVAILABLE)
    {
      flow_quality = FLOW_QUALITY_VOLUME_METER;
      flow = mVolumeBasedFlow;
    }
    else if (mpAdvFlowBasedFlow->GetQuality() != DP_NEVER_AVAILABLE)
    {
      flow_quality = FLOW_QUALITY_ADVANCED_FLOW_ESTIMATION;
      if (mpAdvFlowBasedFlow->GetQuality() == DP_AVAILABLE)
      {
        flow = mpAdvFlowBasedFlow->GetValue();
      }
    }
    else if (mpPitBasedFlow->GetQuality() != DP_NEVER_AVAILABLE)
    {
      flow_quality = FLOW_QUALITY_LEVEL_SENSOR;
      if (mpPitBasedFlow->GetQuality() == DP_AVAILABLE)
      {
        flow = mpPitBasedFlow->GetValue();
      }
    }

    if (flow_quality != FLOW_QUALITY_NO_FLOW_MEASUREMENT)
    {
      if (flow >= 0.0f)
      {
        if (flow > mpSystemFlow->GetMaxValue() && flow < mMaxFlow)
        {
          // Adjust max value if flow has increased. (Used for decimals on display)
          mpSystemFlow->SetMaxValue(flow);
        }
        mpSystemFlow->SetValue(flow);
      }
      else
      {
        mpSystemFlow->SetQuality(DP_NOT_AVAILABLE);
      }
    }
    else
    {
      mpSystemFlow->SetQuality(DP_NEVER_AVAILABLE);
    }
  }

  mpFlowQuality->SetValue(flow_quality);
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
