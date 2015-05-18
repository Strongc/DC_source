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
/* CLASS NAME       : PidCtrl                                               */
/*                                                                          */
/* FILE NAME        : PidCtrl.cpp                                           */
/*                                                                          */
/* CREATED DATE     : 14-05-2009 dd-mm-yyyy                                 */
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
#include <PidCtrl.h>
#include <SwTimer.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

#define PID_TS  0.1f // Pid update/sample rate [s]

// SW Timers
enum
{
  PID_RUN_TIMER
};

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
PidCtrl::PidCtrl()
{
  for (int index = 0; index < NO_OF_MEASURED_VALUE; index++)
  {
    mSubjectAttached[index] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PidCtrl::~PidCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PidCtrl::InitSubTask()
{
  mpTimerObjList[PID_RUN_TIMER] = new SwTimer(1000*PID_TS, MS, true, false, this);  // Periodic timer, no started yet

  mRunRequestedFlag = false;
  mPidRunTimeOut = false;
  mValidSignals = false;
  mConfigChanged = true; // Ensure correct settings at first run

  mPidBlock = new PidBlock();
  mPidFrequency = 0.0f;
  mpVfdPidFrequency->SetQuality(DP_NOT_AVAILABLE);
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PidCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  U32   index = 0;
  bool  preset_pid = false;
  float sp_value = -1.0f;
  float pv_value = -1.0f;

  if (mpPidSetpointType->GetValue() == PID_SETPOINT_TYPE_FIXED)
  {
    if (mpPidFixedSetpoint->GetQuality() == DP_AVAILABLE)
    {
      sp_value = mpPidFixedSetpoint->GetValueAsPercent();
    }
  }
  else
  {
    index = mpPidAiSetpoint->GetValue();
    if (index < NO_OF_MEASURED_VALUE && mSubjectAttached[index] == true && mpMeasuredValue[index]->GetQuality() == DP_AVAILABLE)
    {
      sp_value = mpMeasuredValue[index]->GetValueAsPercent();
    }
  }

  index = mpPidAiFeedback->GetValue();
  if (index < NO_OF_MEASURED_VALUE && mSubjectAttached[index] == true && mpMeasuredValue[index]->GetQuality() == DP_AVAILABLE)
  {
    pv_value = mpMeasuredValue[index]->GetValueAsPercent();
  }

  // Check SP and PV available
  if (sp_value >= 0.0f && pv_value >= 0.0f)
  {
    if (mValidSignals == false)
    {
      // Force a preset if signals have just become available
      preset_pid = true;
    }
    mValidSignals = true;
  }
  else // Invalid signals
  {
    mValidSignals = false;
  }

  if (mConfigChanged == true)
  {
    mConfigChanged = false;
    UpdatePidConfig();
    preset_pid = true;
  }

  if (mpVfdState.IsUpdated())
  {
    if (mpVfdState->GetValue() == VFD_OPERATION_MODE_NORMAL)
    {
      mpTimerObjList[PID_RUN_TIMER]->StartSwTimer();
      preset_pid = true;
    }
    else
    {
      mpTimerObjList[PID_RUN_TIMER]->StopSwTimer();
      mPidRunTimeOut = false;
      mPidFrequency = 0.0f;
      mpVfdPidFrequency->SetQuality(DP_NOT_AVAILABLE);
    }
  }

  // Check if something else than PidCtrl has changed the vfd output. E.g. 'Max if other pump is running'.
  // If so, then preset the pid block to reset integrator windup.
  // NOTE: This solution is simple and works well, BUT requires that the VfdMaster is executed at least once per pid timer run!
  // No problem when VfdMaster runs periodic every 10 ms and the pid timer is 100 ms.
  if (mpVfdActFrequency->GetValue() != mPidFrequency)
  {
    preset_pid = true;
  }

  if (preset_pid == true && mValidSignals == true)
  {
    float percent_2_hertz = mpVfdPidFrequency->GetMaxValue()/100.0f;
    mPidFrequency = mpVfdActFrequency->GetValue(); // Use the actual frequency to ensure bumpless transfer after e.g. flush
    if (mPidFrequency < mpVfdMinFrequency->GetValue())
    {
      mPidFrequency = mpVfdMinFrequency->GetValue();
    }
    mPidBlock->PresetPid(sp_value, pv_value, mPidFrequency/percent_2_hertz);
  }

  if (mPidRunTimeOut == true)
  {
    mPidRunTimeOut = false;
    if (mValidSignals == true)
    {
      float percent_2_hertz = mpVfdPidFrequency->GetMaxValue()/100.0f;
      mPidFrequency = percent_2_hertz*mPidBlock->UpdatePid(sp_value, pv_value);
      mpVfdPidFrequency->SetValue(mPidFrequency);
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void PidCtrl::ConnectToSubjects()
{
  mpVfdState->Subscribe(this);
  mpPidType->Subscribe(this);
  mpPidInverseControl->Subscribe(this);
  mpPidKp->Subscribe(this);
  mpPidTi->Subscribe(this);
  mpPidTd->Subscribe(this);
  mpPidAiFeedback->Subscribe(this);
  mpPidSetpointType->Subscribe(this);
  mpPidAiSetpoint->Subscribe(this);
  mpPidFixedSetpoint->Subscribe(this);
  mpVfdMinFrequency->Subscribe(this);
  mpVfdMaxFrequency->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class.
 *
 *****************************************************************************/
void PidCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpTimerObjList[PID_RUN_TIMER])
  {
    mPidRunTimeOut = true;
  }
  else if (mpVfdState.Update(pSubject) == true)
  {
    ; // No need for further check
  }
  else
  {
    // At last: Check configuration parameters updates
    mConfigChanged |= mpPidType.Update(pSubject);
    mConfigChanged |= mpPidInverseControl.Update(pSubject);
    mConfigChanged |= mpPidKp.Update(pSubject);
    mConfigChanged |= mpPidTi.Update(pSubject);
    mConfigChanged |= mpPidTd.Update(pSubject);
    mConfigChanged |= mpPidAiFeedback.Update(pSubject);
    mConfigChanged |= mpPidSetpointType.Update(pSubject);
    mConfigChanged |= mpPidAiSetpoint.Update(pSubject);
    mConfigChanged |= mpVfdMinFrequency.Update(pSubject);
    mConfigChanged |= mpVfdMaxFrequency.Update(pSubject);
  }

  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void PidCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void PidCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_PID_TYPE:
      mpPidType.Attach(pSubject);
      break;
    case SP_PID_INVERSE_CONTROL:
      mpPidInverseControl.Attach(pSubject);
      break;
    case SP_PID_KP:
      mpPidKp.Attach(pSubject);
      break;
    case SP_PID_TI:
      mpPidTi.Attach(pSubject);
      break;
    case SP_PID_TD:
      mpPidTd.Attach(pSubject);
      break;
    case SP_PID_AI_FEEDBACK:
      mpPidAiFeedback.Attach(pSubject);
      break;
    case SP_PID_SETPOINT_TYPE:
      mpPidSetpointType.Attach(pSubject);
      break;
    case SP_PID_AI_SETPOINT:
      mpPidAiSetpoint.Attach(pSubject);
      break;
    case SP_PID_FIXED_SETPOINT:
      mpPidFixedSetpoint.Attach(pSubject);
      break;
    case SP_PID_VFD_MIN_FREQUENCY:
      mpVfdMinFrequency.Attach(pSubject);
      break;
    case SP_PID_VFD_MAX_FREQUENCY:
      mpVfdMaxFrequency.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_PID_VFD_STATE:
      mpVfdState.Attach(pSubject);
      break;
    case SP_PID_VFD_ACT_FREQUENCY:
      mpVfdActFrequency.Attach(pSubject);
      break;
    // Measured values
    case SP_PID_MEASURED_VALUE_FLOW:
      mpMeasuredValue[MEASURED_VALUE_FLOW].Attach(pSubject);
      mSubjectAttached[MEASURED_VALUE_FLOW] = true;
      break;
    case SP_PID_MEASURED_VALUE_LEVEL_ULTRA_SOUND:
      mpMeasuredValue[MEASURED_VALUE_LEVEL_ULTRA_SOUND].Attach(pSubject);
      mSubjectAttached[MEASURED_VALUE_LEVEL_ULTRA_SOUND] = true;
      break;
    case SP_PID_MEASURED_VALUE_LEVEL_PRESSURE:
      mpMeasuredValue[MEASURED_VALUE_LEVEL_PRESSURE].Attach(pSubject);
      mSubjectAttached[MEASURED_VALUE_LEVEL_PRESSURE] = true;
      break;
    case SP_PID_MEASURED_VALUE_USER_DEFINED_SOURCE_1:
      mpMeasuredValue[MEASURED_VALUE_USER_DEFINED_SOURCE_1].Attach(pSubject);
      mSubjectAttached[MEASURED_VALUE_USER_DEFINED_SOURCE_1] = true;
      break;
    case SP_PID_MEASURED_VALUE_USER_DEFINED_SOURCE_2:
      mpMeasuredValue[MEASURED_VALUE_USER_DEFINED_SOURCE_2].Attach(pSubject);
      mSubjectAttached[MEASURED_VALUE_USER_DEFINED_SOURCE_2] = true;
      break;
    case SP_PID_MEASURED_VALUE_USER_DEFINED_SOURCE_3:
      mpMeasuredValue[MEASURED_VALUE_USER_DEFINED_SOURCE_3].Attach(pSubject);
      mSubjectAttached[MEASURED_VALUE_USER_DEFINED_SOURCE_3] = true;
      break;

    // Outputs:
    case SP_PID_VFD_PID_FREQUENCY:
      mpVfdPidFrequency.Attach(pSubject);
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
 * Function - UpdatePidConfig
 * DESCRIPTION: Update the pid block with actual configuration parameters
 *
 *****************************************************************************/
void PidCtrl::UpdatePidConfig(void)
{
  float percent_2_hertz = mpVfdPidFrequency->GetMaxValue()/100.0f;
  float Ti = 0.0f, Td = 0.0f;
  float Kp = mpPidKp->GetValue();
  if (mpPidInverseControl->GetValue() == true)
  {
    Kp *= -1.0f;
  }
  // Set Ti and Td if used
  switch (mpPidType->GetValue())
  {
    case PID_TYPE_PI:
      Ti = mpPidTi->GetValue();
      break;

    case PID_TYPE_PD:
      Td = mpPidTd->GetValue();
      break;

    case PID_TYPE_PID:
      Ti = mpPidTi->GetValue();
      Td = mpPidTd->GetValue();
      break;

    case PID_TYPE_P:
    default:
      // Nothing
      break;
  }

  mPidBlock->ConfigPid(Kp, Ti, Td, PID_TS, mpVfdMinFrequency->GetValue()/percent_2_hertz, mpVfdMaxFrequency->GetValue()/percent_2_hertz);
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
