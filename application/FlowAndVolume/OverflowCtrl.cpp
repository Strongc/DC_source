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
/* CLASS NAME       : OverflowCtrl                                          */
/*                                                                          */
/* FILE NAME        : OverflowCtrl.cpp                                      */
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
#include <OverflowCtrl.h>

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
OverflowCtrl::OverflowCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_OFC_ALARM_OBJ; fault_id < NO_OF_OFC_ALARM_OBJ; fault_id++)
  {
    mpOfcAlarmDelay[fault_id] = new AlarmDelay(this);
    mOfcAlarmDelayCheckFlag[fault_id] = false;
  }

  mOverflowIsActive = false;
  mConditionalOverflowIsActive = false;
  mOverflowStartingCountDown = 1;
  mOverflowEndingCountDown = 0;
  mOverflowRestartingCountDown = 0;
  mPreConditionalOverflowTimeInSeconds = 0;
  mDeltaVolumeInLitre = 0;
  mDeltaVolumeRemainsInLitre = 0.0f;
  mFreeRunningDeltaVolumeInLitre = 0;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
OverflowCtrl::~OverflowCtrl()
{
  for (int fault_id = FIRST_OFC_ALARM_OBJ; fault_id < NO_OF_OFC_ALARM_OBJ; fault_id++)
  {
    delete(mpOfcAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void OverflowCtrl::InitSubTask()
{
  for (int fault_id = FIRST_OFC_ALARM_OBJ; fault_id < NO_OF_OFC_ALARM_OBJ; fault_id++)
  {
    mpOfcAlarmDelay[fault_id]->InitAlarmDelay();
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 * NOTE:      Must run every second
 *
 *****************************************************************************/
void OverflowCtrl::RunSubTask()
{
  CheckOverflowPresent();
  UpdateOverflowVolume();
  UpdateOverflowCounters();
  HandleOverflowAlarm();
  // Set status bit 5 containing the over flow switch status for GENI
  SetFloatSwitchStatusGENI();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void OverflowCtrl::ConnectToSubjects()
{
  for (int fault_id = FIRST_OFC_ALARM_OBJ; fault_id < NO_OF_OFC_ALARM_OBJ; fault_id++)
  {
    mpOfcAlarmDelay[fault_id]->ConnectToSubjects();
  }
  mpOverflowVolumeM3ForDisplay.Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void OverflowCtrl::Update(Subject* pSubject)
{
  for (int fault_id = FIRST_OFC_ALARM_OBJ; fault_id < NO_OF_OFC_ALARM_OBJ; fault_id++)
  {
    if (pSubject == mpOfcAlarmDelay[fault_id])
    {
      mOfcAlarmDelayCheckFlag[fault_id] = true;
      break;
    }
  }
  if (!mUpdatingOverflowVolume)
  {
    mpOverflowVolumeM3ForDisplay.Update(pSubject);
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void OverflowCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_OFC_ALARM_OBJ; fault_id < NO_OF_OFC_ALARM_OBJ; fault_id++)
  {
    mpOfcAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void OverflowCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Variable inputs:
    case SP_OFC_HIGH_LEVEL_SWITCH_ACTIVATED:
      mpHighLevelSwitchActivated.Attach(pSubject);
      break;
    case SP_OFC_SURFACE_LEVEL:
      mpSurfaceLevel.Attach(pSubject);
      break;
    case SP_OFC_PIT_BASED_FLOW:
      mpPitBasedFlow.Attach(pSubject);
      break;
    case SP_OFC_REFERENCE_POINTS_BASED_FLOW:
      mpReferencePointsBasedFlow.Attach(pSubject);
      break;
    case SP_OFC_REACTIVATION_DELAY:
      mpReactivationDelay.Attach(pSubject);
      break;
    case SP_OFC_ACTIVATION_DELAY:
      mpActivationDelay.Attach(pSubject);
      break;
    case SP_OFC_DI_OVERFLOW_FUNC_STATE:
      mpDiOverflowFuncState.Attach(pSubject);
      break;
      

    // Outputs:
    case SP_OFC_OVERFLOW_TIME:
      mpOverflowTime.Attach(pSubject);
      break;
    case SP_OFC_OVERFLOW_VOLUME_LITRE_FOR_LOG:
      mpOverflowVolumeLitreForLog.Attach(pSubject);
      break;
    case SP_OFC_OVERFLOW_VOLUME_M3_FOR_DISPLAY:
      mpOverflowVolumeM3ForDisplay.Attach(pSubject);
      break;
    case SP_OFC_FREE_RUNNING_OVERFLOW_VOLUME:
      mpFreeRunningOverflowVolume.Attach(pSubject);
      break;
    case SP_OFC_NO_OF_OVERFLOWS:
      mpNoOfOverflow.Attach(pSubject);
      break;
    case SP_OFC_FLOAT_SWITCH_DI_STATUS:
      mpFloatSwitchDiStatus.Attach(pSubject);
      break;

    // Alarms
    case SP_OFC_OVERFLOW_ALARM_OBJ:
      mpOfcAlarmObj[OFC_ALARM_OBJ_OVERFLOW].Attach(pSubject);
      mpOfcAlarmDelay[OFC_ALARM_OBJ_OVERFLOW]->SetSubjectPointer(id, pSubject);
      break;
    case SP_OFC_HIGH_LEVEL_ALARM_OBJ:
      mpOfcHighLevelAlarmObj.Attach(pSubject);
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
 * Function - HandleOverflowAlarm
 * DESCRIPTION: Check against alarm/warnings limit and update alarm
 *
 * NOTE:        No warning or alarm if analog measured level is not present
 *
 *****************************************************************************/
void OverflowCtrl::HandleOverflowAlarm()
{
  float surface_level = -1.0f;

  for (int fault_id = FIRST_OFC_ALARM_OBJ; fault_id < NO_OF_OFC_ALARM_OBJ; fault_id++)
  {
    if (mOfcAlarmDelayCheckFlag[fault_id] == true)
    {
      mOfcAlarmDelayCheckFlag[fault_id] = false;
      mpOfcAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  if (mpSurfaceLevel->GetQuality() == DP_AVAILABLE)
  {
    surface_level = mpSurfaceLevel->GetValue();
  }

  // Evaluate alarm
  if (surface_level >= mpOfcAlarmObj[OFC_ALARM_OBJ_OVERFLOW]->GetAlarmConfig()->GetAlarmLimit()->GetAsFloat())
  {
    mpOfcAlarmDelay[OFC_ALARM_OBJ_OVERFLOW]->SetFault();
  }
  else
  {
    mpOfcAlarmDelay[OFC_ALARM_OBJ_OVERFLOW]->ResetFault();
  }

  // Evaluate warning
  if (surface_level >= mpOfcAlarmObj[OFC_ALARM_OBJ_OVERFLOW]->GetAlarmConfig()->GetWarningLimit()->GetAsFloat())
  {
    mpOfcAlarmDelay[OFC_ALARM_OBJ_OVERFLOW]->SetWarning();
  }
  else
  {
    mpOfcAlarmDelay[OFC_ALARM_OBJ_OVERFLOW]->ResetWarning();
  }

  for (int fault_id = FIRST_OFC_ALARM_OBJ; fault_id < NO_OF_OFC_ALARM_OBJ; fault_id++)
  {
    mpOfcAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - CheckOverflowPresent
 * DESCRIPTION: Check against high level flag from float switch control.
 *              Else check measured level against the overflow alarm limit.
 *              Read the related alarm delay for use in UpdateOverflowCounters.
 *
 *****************************************************************************/
void OverflowCtrl::CheckOverflowPresent()
{
  U32 overflow_alarm_delay;  
  
  DIGITAL_INPUT_FUNC_STATE_TYPE overflow_switch_state = mpDiOverflowFuncState->GetValue();
  
  // Use overflow switch as highest priority
  if (overflow_switch_state != DIGITAL_INPUT_FUNC_STATE_NOT_CONFIGURED)
  {
    overflow_alarm_delay = (U32)(mpActivationDelay->GetValue());
    if (overflow_switch_state == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
    {
      if (mOverflowIsActive == false || mOverflowStartingCountDown > overflow_alarm_delay)
      {
        mOverflowStartingCountDown = overflow_alarm_delay;
        mOverflowRestartingCountDown = 0;
      }
      mOverflowIsActive = true;
    }
    else
    {
      if (mOverflowIsActive == true)
      {
        mOverflowEndingCountDown = overflow_alarm_delay;
      }

      mOverflowIsActive = false;
      mOverflowStartingCountDown = 1;
    }
    }// Use high level switch (and high level alarm delay) as second highest priority
  else if (mpHighLevelSwitchActivated->GetValue() == true)
  {
    U32 high_level_alarm_delay = (U32)(1.0f + mpOfcHighLevelAlarmObj->GetAlarmConfig()->GetAlarmDelay());

    if (mOverflowIsActive == false || mOverflowStartingCountDown > high_level_alarm_delay)
    {
      mOverflowStartingCountDown = high_level_alarm_delay;
      mOverflowRestartingCountDown = 0;
    }
    mOverflowIsActive = true;
  }// Else use measured level and overflow alarm limit delay
  else if ( (mpSurfaceLevel->GetQuality() == DP_AVAILABLE) 
    && (mpSurfaceLevel->GetValue() >= mpOfcAlarmObj[OFC_ALARM_OBJ_OVERFLOW]->GetAlarmConfig()->GetAlarmLimit()->GetAsFloat()) )
  {
    overflow_alarm_delay = (U32)(1.0f + mpOfcAlarmObj[OFC_ALARM_OBJ_OVERFLOW]->GetAlarmConfig()->GetAlarmDelay());

    if (mOverflowIsActive == false)
    {
      mOverflowStartingCountDown = overflow_alarm_delay;
      mOverflowRestartingCountDown = 0;
    }
    mOverflowIsActive = true;
  }
  else
  {
    mOverflowIsActive = false;
    mOverflowStartingCountDown = 1;
  }
}

/*****************************************************************************
 * Function - UpdateOverflowVolume
 * DESCRIPTION: Update overflow volume accumulators
 *
 * NOTE:        Must run every second
 *
 *****************************************************************************/
void OverflowCtrl::UpdateOverflowVolume()
{
  mUpdatingOverflowVolume = true;
  DP_QUALITY_TYPE flow_quality = DP_NEVER_AVAILABLE;
  float flow_value = 0.0f; // [m3/s]


  if (mpOverflowVolumeM3ForDisplay.IsUpdated())
  {
    // user must have adjusted the volume (in Adjustment of counters menu)
    U32 total_litre = (U32) mpOverflowVolumeM3ForDisplay->GetValue() * 1000;
        
    mpOverflowVolumeLitreForLog->SetValue(total_litre); 
    mpOverflowVolumeM3ForDisplay->SetValue(((float)total_litre) / 1000);
  }

  // use reference point based flow if available.
  if (!mpReferencePointsBasedFlow->IsNeverAvailable())
  {
    flow_quality = mpReferencePointsBasedFlow->GetQuality();
    flow_value = mpReferencePointsBasedFlow->GetValue();
  }
  else
  {
    flow_quality = mpPitBasedFlow->GetQuality();
    flow_value = mpPitBasedFlow->GetValue();
  }


  // First update quality of volume accumulators
  if (flow_quality != DP_AVAILABLE)
  {
    mpFreeRunningOverflowVolume->SetQuality(flow_quality);
    if (flow_quality == DP_NOT_AVAILABLE)
    {
      // The overflow volume should still be available during sensor error
      mpOverflowVolumeLitreForLog->SetQuality(DP_AVAILABLE);
      mpOverflowVolumeM3ForDisplay->SetQuality(DP_AVAILABLE);
    }
    else
    {
      mpOverflowVolumeLitreForLog->SetQuality(DP_NEVER_AVAILABLE);
      mpOverflowVolumeM3ForDisplay->SetQuality(DP_NEVER_AVAILABLE);
    }
  }
  else
  {
    // Quality ok
    if (mpFreeRunningOverflowVolume->GetQuality() != DP_AVAILABLE)
    {
      mpFreeRunningOverflowVolume->SetQuality(DP_AVAILABLE);
      mpOverflowVolumeLitreForLog->SetQuality(DP_AVAILABLE);
      mpOverflowVolumeM3ForDisplay->SetQuality(DP_AVAILABLE);
    }

    // Update overflow volume in m3's and keep the rest of the litres
    if (mOverflowIsActive == true)
    {
      float delta_litre;

      // Update free running litres
      mDeltaVolumeRemainsInLitre += 1000.0f * flow_value;
      mDeltaVolumeRemainsInLitre = modff(mDeltaVolumeRemainsInLitre, &delta_litre);
      // Update accumulated m3's
      mDeltaVolumeInLitre += (U32)delta_litre;
      mFreeRunningDeltaVolumeInLitre += (U32)delta_litre;

      if (mConditionalOverflowIsActive)
      {
        mpFreeRunningOverflowVolume->SetValue(mFreeRunningDeltaVolumeInLitre + mpFreeRunningOverflowVolume->GetValue());
        mFreeRunningDeltaVolumeInLitre = 0;
        
        U32 total_litre = mpOverflowVolumeLitreForLog->GetValue() + mDeltaVolumeInLitre;
        mDeltaVolumeInLitre = 0;

        mpOverflowVolumeLitreForLog->SetValue(total_litre); 
        mpOverflowVolumeM3ForDisplay->SetValue(((float)total_litre) / 1000);
      }
    }
  }

/*  if (mpOverflowVolumeM3ForDisplay.IsUpdated())
  {
    // user must have adjusted the volume (in Adjustment of counters menu)
    U32 total_litre = (U32) mpOverflowVolumeM3ForDisplay->GetValue() * 1000;
        
    mpOverflowVolumeLitreForLog->SetValue(total_litre); 
    mpOverflowVolumeM3ForDisplay->SetValue(((float)total_litre) / 1000);
  }
*/

  mUpdatingOverflowVolume = false;
}

/*****************************************************************************
 * Function - UpdateOverflowCounters
 * DESCRIPTION: Update overflow related counters and accumulators
 *
 * NOTE:        Must run every second
 *
 *****************************************************************************/
void OverflowCtrl::UpdateOverflowCounters()
{
  if (mOverflowIsActive == true)
  {
    // Update overflow time in seconds for conditional overflows
    if (mConditionalOverflowIsActive)
    {
      mpOverflowTime->SetValue(mpOverflowTime->GetValue() + 1);
    }
    else
    {
      mPreConditionalOverflowTimeInSeconds++;
    }

    if (mOverflowStartingCountDown > 0)
    {
      mOverflowStartingCountDown--;

      if (mOverflowStartingCountDown == 0 
        && mOverflowRestartingCountDown == 0
        && mConditionalOverflowIsActive == false )
      {
        // conditional overflow has started
        mConditionalOverflowIsActive = true;

        // Increment overflow counter 
        // and start up delay to overflow time
        mpNoOfOverflow->SetValue(mpNoOfOverflow->GetValue() + 1);
        mpOverflowTime->SetValue(mpOverflowTime->GetValue() + mPreConditionalOverflowTimeInSeconds);
        mPreConditionalOverflowTimeInSeconds = 0;
      }
    }
  }
  else //overflow not active
  {
    // reset all intermediate counters 
    mPreConditionalOverflowTimeInSeconds = 0;
    mDeltaVolumeInLitre = 0;
    mDeltaVolumeRemainsInLitre = 0.0;
    mFreeRunningDeltaVolumeInLitre = 0;

    if (mOverflowEndingCountDown > 0)
    {
      mOverflowEndingCountDown--;
    }

    if (mOverflowEndingCountDown == 0)
    {
      // conditional overflow has ended
      if (mConditionalOverflowIsActive)
      {
        mConditionalOverflowIsActive = false;
        mOverflowRestartingCountDown = 1 + mpReactivationDelay->GetValue();
      }
    }

  }

  if (!mConditionalOverflowIsActive 
    && mOverflowRestartingCountDown > 0)
  {
    mOverflowRestartingCountDown--;
  }
}

/*****************************************************************************
 * Function - SetFloatSwitchStatusGENI
 * DESCRIPTION: Set bit 5 according to Over flow switch activated or not
 *****************************************************************************/
void OverflowCtrl::SetFloatSwitchStatusGENI()
{ 
  auto GF_UINT8  fsw = mpFloatSwitchDiStatus->GetValue();

  bool temp = (mpDiOverflowFuncState->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE);

  if(temp == 1)
	fsw |= ((mpDiOverflowFuncState->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 5);
  else
	fsw &= ~((1) << 5);

  mpFloatSwitchDiStatus->SetValue(fsw);
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
