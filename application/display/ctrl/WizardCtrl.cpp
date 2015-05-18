/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : WizardCtrl                                            */
/*                                                                          */
/* FILE NAME        : WizardCtrl.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 23-01-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Changes current display based on actions        */
/*                          and settings made in display wizard.            */
/*                                                                          */
/* I/O mapping is done according to "WW midrange ProductRange_V00.01.14.xls"*/
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Factory.h>
#include "DisplayController.h"
#include "DisplayTypes.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "WizardCtrl.h"


/*****************************************************************************
DEFINES
*****************************************************************************/
#define WIZARD_STEP_05  89 // installation no
#define WIZARD_STEP_06  90 // pit settings
#define WIZARD_STEP_07 151 // modules installed
#define WIZARD_STEP_08  92 // fsw setttings
#define WIZARD_STEP_09  93 // sensor settings
#define WIZARD_STEP_10  94 // AI overview
#define WIZARD_STEP_11  95 // levels
#define WIZARD_STEP_12 110 // options warning
#define WIZARD_STEP_13  96 // options
#define WIZARD_STEP_14  97 // DI overview
#define WIZARD_STEP_15  98 // volume meter
#define WIZARD_STEP_16  99 // energy meter
#define WIZARD_STEP_17 166 // User-Defined Counter 1
#define WIZARD_STEP_18 167 // User-Defined Counter 2
#define WIZARD_STEP_19 168 // User-Defined Counter 3
#define WIZARD_STEP_20 152 // VFD interface
#define WIZARD_STEP_21 153 // VFD AO electrical range
#define WIZARD_STEP_22 102 // communication settings
#define WIZARD_STEP_23 100 // wizard completed

#define FSW_CONFIG_ONE_FSW_AS_HIGH_LEVEL 2 //1:None, 2:High Level, 3:DryRun, 4:High Level + DryRun

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
namespace mpc
{
  namespace display
  {
    namespace ctrl
    {


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
  WizardCtrl::WizardCtrl()
  {
    mCurrentlyUpdating = true;
  }

  /*****************************************************************************
  * Function - Destructor
  * DESCRIPTION:
  *
  ****************************************************************************/
  WizardCtrl::~WizardCtrl()
  {
  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::InitSubTask(void)
  {
    mCurrentlyUpdating = false;                 // End of guarding the SubTask
    ReqTaskTime();
  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  * Executes an action when "go to next page"-button is pressed in wizard
  ****************************************************************************/
  void WizardCtrl::RunSubTask(void)
  {
    mCurrentlyUpdating = true;  // Guard the SubTask

    // set all pumps out of operation when wizard is started
    if (mWizardEnabled.IsUpdated() && mWizardEnabled->GetValue() == true)
    {
      for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
      {
        mPumpInOperation[pump_no]->SetAsBool(false);
      }
    }

    // mCurrentDisplayId is updated as the user leaves the display represented by its value.
    // Thus the current display id will not be current much longer, when processed by this WizardCtrl.
    int current_display_id = mCurrentDisplayId->GetAsInt();

    if (mCurrentDisplayId.IsUpdated())
    {
      int next_display_id = current_display_id;

      switch (current_display_id)
      {
      case WIZARD_STEP_05 :
        {
          InitializePitSettings();
          break;
        }

      case WIZARD_STEP_06 :
        {
          next_display_id = WIZARD_STEP_07;
          CommitPitSettings();
          break;
        }

      case WIZARD_STEP_07 :
        {
          for (int i = mNoOfPumps->GetValue(); i <= LAST_PUMP; i++)
          {
            mVfdInstalled[i]->SetValue(false);
          }

          next_display_id = WIZARD_STEP_08;
          break;
        }

      case WIZARD_STEP_08 :
        {
          if (mControlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)
          {
            // mixer must be disabled then level is float switch controlled
            CommitMixerSettings();

            InitializeOptionSettings();
            next_display_id = WIZARD_STEP_12;
          }
          else
          {
            next_display_id = WIZARD_STEP_09;

            if (mControlType->GetValue() == SENSOR_TYPE_PRESSURE)
            {
              mAI1MeasuredValue->SetValue(MEASURED_VALUE_LEVEL_PRESSURE);
            }
            else
            {
              mAI1MeasuredValue->SetValue(MEASURED_VALUE_LEVEL_ULTRA_SOUND);
              mUltrasonicSensorInversed->SetValue(false);
              mUltrasonicSensorOffset->SetValue(0);
              mAI1ElectricalType->SetValue(SENSOR_ELECTRIC_TYPE_4_20mA);
            }
          }
          break;
        }

      case WIZARD_STEP_11 :
        {
          CommitMixerSettings();
          InitializeOptionSettings();
          break;
        }
      case WIZARD_STEP_12 :
        {
          InitializeOptionSettings();
          break;
        }

      case WIZARD_STEP_13 :
        {
          CommitOptionSettings();
          break;
        }

      case WIZARD_STEP_14 :
        {
          if (mVolumeMeterEnabled->GetValue())
          {
            next_display_id = WIZARD_STEP_15;
          }
          else if (mEnergyMeterEnabled->GetValue())
          {
            next_display_id = WIZARD_STEP_16;
          }
          else if (mUserDefinedCounter1Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_17;
          }
          else if (mUserDefinedCounter2Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_18;
          }
          else if (mUserDefinedCounter3Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_19;
          }
          else if (OneOrMoreVfdInstalled())
          {
            next_display_id = WIZARD_STEP_20;
          }
          else
          {
            next_display_id = WIZARD_STEP_22;
          }
          break;
        }

      case WIZARD_STEP_15 :
        {
          if (mEnergyMeterEnabled->GetValue())
          {
            next_display_id = WIZARD_STEP_16;
          }
          else if (mUserDefinedCounter1Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_17;
          }
          else if (mUserDefinedCounter2Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_18;
          }
          else if (mUserDefinedCounter3Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_19;
          }
          else if (OneOrMoreVfdInstalled())
          {
            next_display_id = WIZARD_STEP_20;
          }
          else
          {
            next_display_id = WIZARD_STEP_22;
          }
          break;
        }

      case WIZARD_STEP_16 :
        {
          if (mUserDefinedCounter1Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_17;
          }
          else if (mUserDefinedCounter2Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_18;
          }
          else if (mUserDefinedCounter3Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_19;
          }
          else if (OneOrMoreVfdInstalled())
          {
            next_display_id = WIZARD_STEP_20;
          }
          else
          {
            next_display_id = WIZARD_STEP_22;
          }
          break;
        }

      case WIZARD_STEP_17 :
        {
          if (mUserDefinedCounter2Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_18;
          }
          else if (mUserDefinedCounter3Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_19;
          }
          else if (OneOrMoreVfdInstalled())
          {
            next_display_id = WIZARD_STEP_20;
          }
          else
          {
            next_display_id = WIZARD_STEP_22;
          }
          break;
        }

      case WIZARD_STEP_18 :
        {
          if (mUserDefinedCounter3Enabled->GetValue())
          {
            next_display_id = WIZARD_STEP_19;
          }
          else if (OneOrMoreVfdInstalled())
          {
            next_display_id = WIZARD_STEP_20;
          }
          else
          {
            next_display_id = WIZARD_STEP_22;
          }
          break;
        }

      case WIZARD_STEP_19 :
        {
          if (OneOrMoreVfdInstalled())
          {
            next_display_id = WIZARD_STEP_20;
          }
          else
          {
            next_display_id = WIZARD_STEP_22;
          }
          break;
        }

      case WIZARD_STEP_20 :
        {
          bool one_or_more_vfd_uses_ao = false;

          for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
          {
            bool set_ao_func_to_vfd = (mVfdInstalled[i]->GetValue() && !mCueInterfaceUsed[i]->GetValue() && i < mNoOfPumps->GetValue());

            ANA_OUT_FUNC_TYPE func = (ANA_OUT_FUNC_TYPE) (set_ao_func_to_vfd ? ANA_OUT_FUNC_VFD_1 + i : ANA_OUT_FUNC_NO_FUNCTION);
            mVfdAoFunc[i]->SetValue(func);

            one_or_more_vfd_uses_ao |= set_ao_func_to_vfd;
          }

          if (one_or_more_vfd_uses_ao)
          {
            next_display_id = WIZARD_STEP_21;
          }
          else
          {
            next_display_id = WIZARD_STEP_22;
          }
          break;
        }

      case WIZARD_STEP_21 :
        {
          next_display_id = WIZARD_STEP_22;
          break;
        }

      case WIZARD_STEP_22 :
        {
          CommitCommunicationSettings();
          break;
        }

      case WIZARD_STEP_23 :
        {
          mWizardEnabled->SetValue(false);
          break;
        }
      }

      if (next_display_id != current_display_id)
      {
        DisplayController::GetInstance()->RequestDisplayChange(next_display_id);
      }
    }

    int free_di = CalcFreeDigitalInputsCount();

    if (free_di < 0 || !IsOptionComboLegal())
    { // cancel last change(s)
      for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
      {
        if (mPumpCFEnabled[pump_no].IsUpdated())
        {
          mPumpCFEnabled[pump_no]->SetAsBool(false);
        }
        if (mPumpOOAEnabled[pump_no].IsUpdated())
        {
          mPumpOOAEnabled[pump_no]->SetAsBool(false);
        }
        if (mPumpMPEnabled[pump_no].IsUpdated())
        {
          mPumpMPEnabled[pump_no]->SetAsBool(false);
        }
        if (mPumpVfdReadyEnabled[pump_no].IsUpdated())
        {
          mPumpVfdReadyEnabled[pump_no]->SetAsBool(false);
        }
      }

      if (mOverflowSwitchInstalled.IsUpdated())
      {
        mOverflowSwitchInstalled->SetAsBool(false);
      }
      if (mCommonPhaseErrorEnabled.IsUpdated())
      {
        mCommonPhaseErrorEnabled->SetAsBool(false);
      }
      if (mVolumeMeterEnabled.IsUpdated())
      {
        mVolumeMeterEnabled->SetAsBool(false);
      }
      if (mEnergyMeterEnabled.IsUpdated())
      {
        mEnergyMeterEnabled->SetAsBool(false);
      }
      if (mMixerCFEnabled.IsUpdated())
      {
        mMixerCFEnabled->SetAsBool(false);
      }
      if (mAlarmResetEnabled.IsUpdated())
      {
        mAlarmResetEnabled->SetAsBool(false);
      }
      if (mRelayResetEnabled.IsUpdated())
      {
        mRelayResetEnabled->SetAsBool(false);
      }
      if (mExternalFaultEnabled.IsUpdated())
      {
        mExternalFaultEnabled->SetAsBool(false);
      }
      if (mUserDefinedCounter1Enabled.IsUpdated())
      {
        mUserDefinedCounter1Enabled->SetAsBool(false);
      }
      if (mUserDefinedCounter2Enabled.IsUpdated())
      {
        mUserDefinedCounter2Enabled->SetAsBool(false);
      }
      if (mUserDefinedCounter3Enabled.IsUpdated())
      {
        mUserDefinedCounter3Enabled->SetAsBool(false);
      }

      free_di = CalcFreeDigitalInputsCount();
    }
    else
    {
      for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
      {
        mPumpCFEnabled[pump_no].ResetUpdated();
        mPumpOOAEnabled[pump_no].ResetUpdated();
        mPumpMPEnabled[pump_no].ResetUpdated();
        mPumpVfdReadyEnabled[pump_no].ResetUpdated();
      }
      mOverflowSwitchInstalled.ResetUpdated();
      mCommonPhaseErrorEnabled.ResetUpdated();
      mVolumeMeterEnabled.ResetUpdated();
      mEnergyMeterEnabled.ResetUpdated();
      mMixerCFEnabled.ResetUpdated();
      mAlarmResetEnabled.ResetUpdated();
      mRelayResetEnabled.ResetUpdated();
      mExternalFaultEnabled.ResetUpdated();
      mUserDefinedCounter1Enabled.ResetUpdated();
      mUserDefinedCounter2Enabled.ResetUpdated();
      mUserDefinedCounter3Enabled.ResetUpdated();
    }

    mFreeDICount->SetAsInt(free_di);

    mCurrentlyUpdating = false; // End of: Guard the SubTask
  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::SubscribtionCancelled(Subject* pSubject)
  {
    //no need to implement - it is never called
  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::Update(Subject* pSubject)
  {
    mCurrentDisplayId.Update(pSubject);

    for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
    {
      if (mPumpCFEnabled[pump_no].Update(pSubject))
      {
        break;
      }
      else if (mPumpOOAEnabled[pump_no].Update(pSubject))
      {
        break;
      }
      else if (mPumpMPEnabled[pump_no].Update(pSubject))
      {
        break;
      }
      else if (mPumpVfdReadyEnabled[pump_no].Update(pSubject))
      {
        break;
      }
    }

    mOverflowSwitchInstalled.Update(pSubject);
    mCommonPhaseErrorEnabled.Update(pSubject);
    mVolumeMeterEnabled.Update(pSubject);
    mEnergyMeterEnabled.Update(pSubject);
    mMixerCFEnabled.Update(pSubject);
    mAlarmResetEnabled.Update(pSubject);
    mRelayResetEnabled.Update(pSubject);
    mExternalFaultEnabled.Update(pSubject);
    mWizardEnabled.Update(pSubject);
    mUserDefinedCounter1Enabled.Update(pSubject);
    mUserDefinedCounter2Enabled.Update(pSubject);
    mUserDefinedCounter3Enabled.Update(pSubject);

    if (!mCurrentlyUpdating && mWizardEnabled->GetAsBool())
    {
      ReqTaskTime();
    }
  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::SetSubjectPointer(int Id, Subject* pSubject)
  {
    switch (Id)
    {
    case SP_WC_CURRENT_DISPLAY :
      mCurrentDisplayId.Attach(pSubject);
      break;
    case SP_WC_PIT_LEVEL_CTRL_TYPE :
      mControlType.Attach(pSubject);
      break;
    case SP_WC_WIZARD_ENABLED :
      mWizardEnabled.Attach(pSubject);
      break;
    case SP_WC_FREE_DIGITAL_INPUTS :
      mFreeDICount.Attach(pSubject);
      break;
    case SP_WC_PUMP_1_CF_ENABLED :
      mPumpCFEnabled[PUMP_1].Attach(pSubject);
      break;
    case SP_WC_PUMP_1_OOA_ENABLED :
      mPumpOOAEnabled[PUMP_1].Attach(pSubject);
      break;
    case SP_WC_PUMP_1_MP_ENABLED :
      mPumpMPEnabled[PUMP_1].Attach(pSubject);
      break;
    case SP_WC_PUMP_1_VFD_READY_ENABLED :
      mPumpVfdReadyEnabled[PUMP_1].Attach(pSubject);
      break;
    case SP_WC_PUMP_2_CF_ENABLED :
      mPumpCFEnabled[PUMP_2].Attach(pSubject);
      break;
    case SP_WC_PUMP_2_OOA_ENABLED :
      mPumpOOAEnabled[PUMP_2].Attach(pSubject);
      break;
    case SP_WC_PUMP_2_MP_ENABLED :
      mPumpMPEnabled[PUMP_2].Attach(pSubject);
      break;
    case SP_WC_PUMP_2_VFD_READY_ENABLED :
      mPumpVfdReadyEnabled[PUMP_2].Attach(pSubject);
      break;
    case SP_WC_PUMP_3_CF_ENABLED :
      mPumpCFEnabled[PUMP_3].Attach(pSubject);
      break;
    case SP_WC_PUMP_3_OOA_ENABLED :
      mPumpOOAEnabled[PUMP_3].Attach(pSubject);
      break;
    case SP_WC_PUMP_3_MP_ENABLED :
      mPumpMPEnabled[PUMP_3].Attach(pSubject);
      break;
    case SP_WC_PUMP_3_VFD_READY_ENABLED :
      mPumpVfdReadyEnabled[PUMP_3].Attach(pSubject);
      break;
    case SP_WC_PUMP_4_CF_ENABLED :
      mPumpCFEnabled[PUMP_4].Attach(pSubject);
      break;
    case SP_WC_PUMP_4_OOA_ENABLED :
      mPumpOOAEnabled[PUMP_4].Attach(pSubject);
      break;
    case SP_WC_PUMP_4_MP_ENABLED :
      mPumpMPEnabled[PUMP_4].Attach(pSubject);
      break;
    case SP_WC_PUMP_4_VFD_READY_ENABLED :
      mPumpVfdReadyEnabled[PUMP_4].Attach(pSubject);
      break;
    case SP_WC_PUMP_5_CF_ENABLED :
      mPumpCFEnabled[PUMP_5].Attach(pSubject);
      break;
    case SP_WC_PUMP_5_OOA_ENABLED :
      mPumpOOAEnabled[PUMP_5].Attach(pSubject);
      break;
    case SP_WC_PUMP_5_MP_ENABLED :
      mPumpMPEnabled[PUMP_5].Attach(pSubject);
      break;
    case SP_WC_PUMP_5_VFD_READY_ENABLED :
      mPumpVfdReadyEnabled[PUMP_5].Attach(pSubject);
      break;
    case SP_WC_PUMP_6_CF_ENABLED :
      mPumpCFEnabled[PUMP_6].Attach(pSubject);
      break;
    case SP_WC_PUMP_6_OOA_ENABLED :
      mPumpOOAEnabled[PUMP_6].Attach(pSubject);
      break;
    case SP_WC_PUMP_6_MP_ENABLED :
      mPumpMPEnabled[PUMP_6].Attach(pSubject);
      break;
    case SP_WC_PUMP_6_VFD_READY_ENABLED :
      mPumpVfdReadyEnabled[PUMP_6].Attach(pSubject);
      break;
    case SP_WC_COMMON_PHASE_ERROR_ENABLED :
      mCommonPhaseErrorEnabled.Attach(pSubject);
      break;
    case SP_WC_VOLUME_METER_ENABLED :
      mVolumeMeterEnabled.Attach(pSubject);
      break;
    case SP_WC_ENERGY_METER_ENABLED :
      mEnergyMeterEnabled.Attach(pSubject);
      break;
    case SP_WC_MIXER_CF_ENABLED :
      mMixerCFEnabled.Attach(pSubject);
      break;
    case SP_WC_ALARM_RESET_ENABLED :
      mAlarmResetEnabled.Attach(pSubject);
      break;
    case SP_WC_RELAY_RESET_ENABLED :
      mRelayResetEnabled.Attach(pSubject);
      break;
    case SP_WC_EXTERNAL_FAULT_ENABLED :
      mExternalFaultEnabled.Attach(pSubject);
      break;
    case SP_WC_MIXER_INSTALLED :
      mMixerInstalled.Attach(pSubject);
      break;
    case SP_WC_MIXER_ENABLED :
      mMixerEnabled.Attach(pSubject);
      break;
    case SP_WC_ULTRA_SONIC_INVERSED :
      mUltrasonicSensorInversed.Attach(pSubject);
      break;
    case SP_WC_ULTRA_SONIC_OFFSET :
      mUltrasonicSensorOffset.Attach(pSubject);
      break;
    case SP_WC_AI_1_ELECTRICAL_TYPE :
      mAI1ElectricalType.Attach(pSubject);
      break;
    case SP_WC_NO_OF_FSW :
      mNoOfFsw.Attach(pSubject);
      break;
    case SP_WC_NO_OF_PUMPS :
      mNoOfPumps.Attach(pSubject);
      break;
    case SP_WC_AI_1_MEASURED_VALUE :
      mAI1MeasuredValue.Attach(pSubject);
      break;
    case SP_WC_MIXER_STOP_LEVEL :
      mMixerStopLevel.Attach(pSubject);
      break;
    case SP_WC_MIXER_START_LEVEL_OFFSET :
      mMixerStartLevelOffset.Attach(pSubject);
      break;
    case SP_WC_CURRENT_FLOAT_SWITCH_CONFIG_NUMBER :
      mFswConfigNumber.Attach(pSubject);
      break;
    case SP_WC_OVERFLOW_SWITCH_INSTALLED :
      mOverflowSwitchInstalled.Attach(pSubject);
      break;
    case SP_WC_USER_DEFINED_COUNTER_1_ENABLED :
      mUserDefinedCounter1Enabled.Attach(pSubject);
      break;
    case SP_WC_USER_DEFINED_COUNTER_2_ENABLED :
      mUserDefinedCounter2Enabled.Attach(pSubject);
      break;
    case SP_WC_USER_DEFINED_COUNTER_3_ENABLED :
      mUserDefinedCounter3Enabled.Attach(pSubject);
      break;
      
    case SP_WC_PUMP_GROUPS_ENABLED :
      mPumpGroupsEnabled.Attach(pSubject);
      break;
    case SP_WC_PUMP_GROUP_1_MAX_STARTED_PUMPS :
      mPumpGroup1MaxStartedPumps.Attach(pSubject);
      break;
    case SP_WC_PUMP_GROUP_1_MIN_STARTED_PUMPS :
      mPumpGroup1MinStartedPumps.Attach(pSubject);
      break;
    case SP_WC_PUMP_GROUP_1_ALTERNATION_ENABLED :
      mPumpGroup1AlternationEnabled.Attach(pSubject);
      break;


    case SP_WC_PUMP_1_VFD_INSTALLED :
      mVfdInstalled[PUMP_1].Attach(pSubject);
      break;
    case SP_WC_PUMP_2_VFD_INSTALLED :
      mVfdInstalled[PUMP_2].Attach(pSubject);
      break;
    case SP_WC_PUMP_3_VFD_INSTALLED :
      mVfdInstalled[PUMP_3].Attach(pSubject);
      break;
    case SP_WC_PUMP_4_VFD_INSTALLED :
      mVfdInstalled[PUMP_4].Attach(pSubject);
      break;
    case SP_WC_PUMP_5_VFD_INSTALLED :
      mVfdInstalled[PUMP_5].Attach(pSubject);
      break;
    case SP_WC_PUMP_6_VFD_INSTALLED :
      mVfdInstalled[PUMP_6].Attach(pSubject);
      break;
    case SP_WC_PUMP_1_CUE_IF_USED :
      mCueInterfaceUsed[PUMP_1].Attach(pSubject);
      break;
    case SP_WC_PUMP_2_CUE_IF_USED :
      mCueInterfaceUsed[PUMP_2].Attach(pSubject);
      break;
    case SP_WC_PUMP_3_CUE_IF_USED :
      mCueInterfaceUsed[PUMP_3].Attach(pSubject);
      break;
    case SP_WC_PUMP_4_CUE_IF_USED :
      mCueInterfaceUsed[PUMP_4].Attach(pSubject);
      break;
    case SP_WC_PUMP_5_CUE_IF_USED :
      mCueInterfaceUsed[PUMP_5].Attach(pSubject);
      break;
    case SP_WC_PUMP_6_CUE_IF_USED :
      mCueInterfaceUsed[PUMP_6].Attach(pSubject);
      break;
    case SP_WC_PUMP_1_AO_FUNC :
      mVfdAoFunc[PUMP_1].Attach(pSubject);
      break;
    case SP_WC_PUMP_2_AO_FUNC :
      mVfdAoFunc[PUMP_2].Attach(pSubject);
      break;
    case SP_WC_PUMP_3_AO_FUNC :
      mVfdAoFunc[PUMP_3].Attach(pSubject);
      break;
    case SP_WC_PUMP_4_AO_FUNC :
      mVfdAoFunc[PUMP_4].Attach(pSubject);
      break;
    case SP_WC_PUMP_5_AO_FUNC :
      mVfdAoFunc[PUMP_5].Attach(pSubject);
      break;
    case SP_WC_PUMP_6_AO_FUNC :
      mVfdAoFunc[PUMP_6].Attach(pSubject);
      break;

      // DP's for initialization of pump and system options
    case SP_WC_DI_NUM_PUMP_1_CF :
      mPumpCFDiNo[0].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_1_OO :
      mPumpOODiNo[0].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_1_AM :
      mPumpAMDiNo[0].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_1_MP :
      mPumpMPDiNo[0].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_1_VFD_READY :
      mPumpVfdReadyDiNo[PUMP_1].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_2_CF :
      mPumpCFDiNo[1].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_2_OO :
      mPumpOODiNo[1].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_2_AM :
      mPumpAMDiNo[1].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_2_MP :
      mPumpMPDiNo[1].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_2_VFD_READY :
      mPumpVfdReadyDiNo[PUMP_2].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_3_CF :
      mPumpCFDiNo[2].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_3_OO :
      mPumpOODiNo[2].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_3_AM :
      mPumpAMDiNo[2].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_3_MP :
      mPumpMPDiNo[2].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_3_VFD_READY :
      mPumpVfdReadyDiNo[PUMP_3].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_4_CF :
      mPumpCFDiNo[3].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_4_OO :
      mPumpOODiNo[3].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_4_AM :
      mPumpAMDiNo[3].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_4_MP :
      mPumpMPDiNo[3].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_4_VFD_READY :
      mPumpVfdReadyDiNo[PUMP_4].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_5_CF :
      mPumpCFDiNo[4].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_5_OO :
      mPumpOODiNo[4].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_5_AM :
      mPumpAMDiNo[4].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_5_MP :
      mPumpMPDiNo[4].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_5_VFD_READY :
      mPumpVfdReadyDiNo[PUMP_5].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_6_CF :
      mPumpCFDiNo[5].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_6_OO :
      mPumpOODiNo[5].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_6_AM :
      mPumpAMDiNo[5].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_6_MP :
      mPumpMPDiNo[5].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_PUMP_6_VFD_READY :
      mPumpVfdReadyDiNo[PUMP_6].Attach(pSubject);
      break;
    case SP_WC_DI_NUM_COMMON_PHASE :
      mCommonPhaseDiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_VOLUME_METER :
      mVolumeMeterDiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_ENERGY_METER :
      mEnergyMeterDiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_MIXER_CF :
      mMixerCFDiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_ALARM_RESET :
      mAlarmResetDiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_RELAY_RESET :
      mRelayResetDiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_EXTERNAL_FAULT :
      mExternalFaultDiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_OVERFLOW_SWITCH :
      mOverflowSwitchDiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_USER_DEFINED_COUNTER_1 :
      mUserDefinedCounter1DiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_USER_DEFINED_COUNTER_2 :
      mUserDefinedCounter2DiNo.Attach(pSubject);
      break;
    case SP_WC_DI_NUM_USER_DEFINED_COUNTER_3 :
      mUserDefinedCounter3DiNo.Attach(pSubject);
      break;

      
      // DP's for communication settings
    case SP_WC_SCADA_NO :
      mScadaNo.Attach(pSubject);
      break;
    case SP_WC_ENABLE_SCADA_IN_ALARM_CONFIGS :
      mEnableScadaInAlarmConfigs.Attach(pSubject);
      break;
    case SP_WC_PUMP_1_IN_OPERATION :
      mPumpInOperation[0].Attach(pSubject);
      break;
    case SP_WC_PUMP_2_IN_OPERATION :
      mPumpInOperation[1].Attach(pSubject);
      break;
    case SP_WC_PUMP_3_IN_OPERATION :
      mPumpInOperation[2].Attach(pSubject);
      break;
    case SP_WC_PUMP_4_IN_OPERATION :
      mPumpInOperation[3].Attach(pSubject);
      break;
    case SP_WC_PUMP_5_IN_OPERATION :
      mPumpInOperation[4].Attach(pSubject);
      break;
    case SP_WC_PUMP_6_IN_OPERATION :
      mPumpInOperation[5].Attach(pSubject);
      break;

      // DP's for pump and system options
    case SP_WC_FUNC_DI_1:
    case SP_WC_FUNC_DI_2:
    case SP_WC_FUNC_DI_3:
    case SP_WC_FUNC_DI_4:
    case SP_WC_FUNC_DI_5:
    case SP_WC_FUNC_DI_6:
    case SP_WC_FUNC_DI_7:
    case SP_WC_FUNC_DI_8:
    case SP_WC_FUNC_DI_9:
    case SP_WC_FUNC_DI_10:
    case SP_WC_FUNC_DI_11:
    case SP_WC_FUNC_DI_12:
    case SP_WC_FUNC_DI_13:
    case SP_WC_FUNC_DI_14:
    case SP_WC_FUNC_DI_15:
    case SP_WC_FUNC_DI_16:
    case SP_WC_FUNC_DI_17:
    case SP_WC_FUNC_DI_18:
    case SP_WC_FUNC_DI_19:
    case SP_WC_FUNC_DI_20:
    case SP_WC_FUNC_DI_21:
      {
        int index = (pSubject->GetSubjectId() - SUBJECT_ID_DIG_IN_1_CONF_DIGITAL_INPUT_FUNC + 1);
        if (index > 0 && index < NO_OF_DI_INDEX)
        {
          mDiFunc[index].Attach(pSubject);
        }
        else
        {
          FatalErrorOccured("WC: DI Function OOR");
        }
      }
      break;
    case SP_WC_FUNC_DI_22:
    case SP_WC_FUNC_DI_23:
    case SP_WC_FUNC_DI_24:
    case SP_WC_FUNC_DI_25:
    case SP_WC_FUNC_DI_26:
    case SP_WC_FUNC_DI_27:
    case SP_WC_FUNC_DI_28:
    case SP_WC_FUNC_DI_29:
    case SP_WC_FUNC_DI_30:
      {
        int index = (pSubject->GetSubjectId() - SUBJECT_ID_DIG_IN_22_CONF_DIGITAL_INPUT_FUNC + 22);
        if (index > 0 && index < NO_OF_DI_INDEX)
        {
          mDiFunc[index].Attach(pSubject);
        }
        else
        {
          FatalErrorOccured("WC: DI Function OOR");
        }
      }
      break;

    case SP_WC_LOGIC_DI_1:
    case SP_WC_LOGIC_DI_2:
    case SP_WC_LOGIC_DI_3:
    case SP_WC_LOGIC_DI_4:
    case SP_WC_LOGIC_DI_5:
    case SP_WC_LOGIC_DI_6:
    case SP_WC_LOGIC_DI_7:
    case SP_WC_LOGIC_DI_8:
    case SP_WC_LOGIC_DI_9:
    case SP_WC_LOGIC_DI_10:
    case SP_WC_LOGIC_DI_11:
    case SP_WC_LOGIC_DI_12:
    case SP_WC_LOGIC_DI_13:
    case SP_WC_LOGIC_DI_14:
    case SP_WC_LOGIC_DI_15:
    case SP_WC_LOGIC_DI_16:
    case SP_WC_LOGIC_DI_17:
    case SP_WC_LOGIC_DI_18:
    case SP_WC_LOGIC_DI_19:
    case SP_WC_LOGIC_DI_20:
    case SP_WC_LOGIC_DI_21:
      {
        int index = (pSubject->GetSubjectId() - SUBJECT_ID_DIG_IN_1_CONF_LOGIC + 1);
        if (index > 0 && index < NO_OF_DI_INDEX)
        {
          mDiLogic[index].Attach(pSubject);
        }
        else
        {
          FatalErrorOccured("WC: DI Logic OOR");
        }
      }
      break;
    case SP_WC_LOGIC_DI_22:
    case SP_WC_LOGIC_DI_23:
    case SP_WC_LOGIC_DI_24:
    case SP_WC_LOGIC_DI_25:
    case SP_WC_LOGIC_DI_26:
    case SP_WC_LOGIC_DI_27:
    case SP_WC_LOGIC_DI_28:
    case SP_WC_LOGIC_DI_29:
    case SP_WC_LOGIC_DI_30:
      {
        int index = (pSubject->GetSubjectId() - SUBJECT_ID_DIG_IN_22_CONF_LOGIC + 22);
        if (index > 0 && index < NO_OF_DI_INDEX)
        {
          mDiLogic[index].Attach(pSubject);
        }
        else
        {
          FatalErrorOccured("WC: DI Logic OOR");
        }
      }
      break;
    case SP_WC_NO_OF_IO351_MODULES:
      mNoOfIo351Modules.Attach(pSubject);
      break;
    
    case SP_WC_FUNC_DO_1:
    case SP_WC_FUNC_DO_2:
    case SP_WC_FUNC_DO_3:
    case SP_WC_FUNC_DO_4:
    case SP_WC_FUNC_DO_5:
    case SP_WC_FUNC_DO_6:
    case SP_WC_FUNC_DO_7:
    case SP_WC_FUNC_DO_8:
    case SP_WC_FUNC_DO_9:
    case SP_WC_FUNC_DO_10:
    case SP_WC_FUNC_DO_11:
    case SP_WC_FUNC_DO_12:
    case SP_WC_FUNC_DO_13:
    case SP_WC_FUNC_DO_14:
    case SP_WC_FUNC_DO_15:
    case SP_WC_FUNC_DO_16:
      {
        int index = (pSubject->GetSubjectId() - SUBJECT_ID_DIG_OUT_1_CONF_RELAY_FUNC + 1);
        if (index > 0 && index < NO_OF_DO_INDEX)
        {
          mDoFunc[index].Attach(pSubject);
        }
        else
        {
          FatalErrorOccured("WC: DO Function OOR");
        }
      }
      break;

    case SP_WC_FUNC_DO_17:
    case SP_WC_FUNC_DO_18:
    case SP_WC_FUNC_DO_19:
    case SP_WC_FUNC_DO_20:
    case SP_WC_FUNC_DO_21:
    case SP_WC_FUNC_DO_22:
    case SP_WC_FUNC_DO_23:
      {
        int index = (pSubject->GetSubjectId() - SUBJECT_ID_DIG_OUT_17_CONF_RELAY_FUNC + 17);
        if (index > 0 && index < NO_OF_DO_INDEX)
        {
          mDoFunc[index].Attach(pSubject);
        }
        else
        {
          FatalErrorOccured("WC: DO Function OOR");
        }
      }
      break;
    }

  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::ConnectToSubjects(void)
  {
    mCurrentDisplayId->SubscribeE(this); //need subscribeE to ensure display shift if <esc> button is used

    mControlType->Subscribe(this);

    for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
    {
      mPumpCFEnabled[pump_no]->Subscribe(this);
      mPumpOOAEnabled[pump_no]->Subscribe(this);
      mPumpMPEnabled[pump_no]->Subscribe(this);
      mPumpVfdReadyEnabled[pump_no]->Subscribe(this);
    }

    mOverflowSwitchInstalled->Subscribe(this);
    mCommonPhaseErrorEnabled->Subscribe(this);
    mVolumeMeterEnabled->Subscribe(this);
    mEnergyMeterEnabled->Subscribe(this);
    mMixerCFEnabled->Subscribe(this);
    mAlarmResetEnabled->Subscribe(this);
    mRelayResetEnabled->Subscribe(this);
    mExternalFaultEnabled->Subscribe(this);
    mNoOfFsw->Subscribe(this);
    mWizardEnabled->Subscribe(this);
    mUserDefinedCounter1Enabled->Subscribe(this);
    mUserDefinedCounter2Enabled->Subscribe(this);
    mUserDefinedCounter3Enabled->Subscribe(this);
  }

  /*****************************************************************************
  *
  *
  *              PRIVATE FUNCTIONS
  *
  *
  ****************************************************************************/

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::InitializePitSettings(void)
  {
    bool mixer_installed = (mDoFunc[DO_INDEX_DO6_IO351_41]->GetValue() == RELAY_FUNC_MIXER);

    mMixerInstalled->SetValue(mixer_installed);
  }

  /*****************************************************************************
  * Function - CommitPitSettings
  * DESCRIPTION: assign functions to digital outputs
  * 
  ****************************************************************************/
  void WizardCtrl::CommitPitSettings(void)
  {
    int no_of_pumps = mNoOfPumps->GetValue();

    if (no_of_pumps > 4)
    {
      mNoOfIo351Modules->SetValue(3);
    }
    else if (no_of_pumps > 2) 
    {
      mNoOfIo351Modules->SetValue(2);
    }
    else
    {
      mNoOfIo351Modules->SetValue(1);
    }

    mPumpGroupsEnabled->SetValue(false);
    mPumpGroup1MaxStartedPumps->SetValue(no_of_pumps);
    mPumpGroup1MinStartedPumps->SetValue(1);
    mPumpGroup1AlternationEnabled->SetValue(true);

    mDoFunc[DO_INDEX_DO1_CU361]->SetValue(RELAY_FUNC_ALARM_RELAY_HIGH_LEVEL);
    mDoFunc[DO_INDEX_DO2_CU361]->SetValue(RELAY_FUNC_ALARM_RELAY_ALL_ALARMS_AND_WARNINGS);

    mDoFunc[DO_INDEX_DO1_IO351_41]->SetValue(RELAY_FUNC_PUMP_1);
    mDoFunc[DO_INDEX_DO2_IO351_41]->SetValue(no_of_pumps > 1 ? RELAY_FUNC_PUMP_2 : RELAY_FUNC_NO_FUNCTION);
    mDoFunc[DO_INDEX_DO3_IO351_41]->SetValue(RELAY_FUNC_ALARM_RELAY_ALL_ALARMS);
    
    mDoFunc[DO_INDEX_DO7_IO351_41]->SetValue(RELAY_FUNC_ALARM_RELAY_CUSTOM);

    mDoFunc[DO_INDEX_DO1_IO351_42]->SetValue(no_of_pumps > 2 ? RELAY_FUNC_PUMP_3 : RELAY_FUNC_NO_FUNCTION);
    mDoFunc[DO_INDEX_DO2_IO351_42]->SetValue(no_of_pumps > 3 ? RELAY_FUNC_PUMP_4 : RELAY_FUNC_NO_FUNCTION);

    mDoFunc[DO_INDEX_DO1_IO351_43]->SetValue(no_of_pumps > 4 ? RELAY_FUNC_PUMP_5 : RELAY_FUNC_NO_FUNCTION);
    mDoFunc[DO_INDEX_DO2_IO351_43]->SetValue(no_of_pumps > 5 ? RELAY_FUNC_PUMP_6 : RELAY_FUNC_NO_FUNCTION);
  }

  /*****************************************************************************
  * Function - CommitMixerSettings
  * DESCRIPTION: sets DI3-CU361 (OR DI8-IO351.41) + DO6-IO351
  *
  ****************************************************************************/
  void WizardCtrl::CommitMixerSettings(void)
  {
    int no_of_fsw = mNoOfFsw->GetValue();

    if (mMixerInstalled->GetValue() == true)
    {
      mDoFunc[DO_INDEX_DO6_IO351_41]->SetValue(RELAY_FUNC_MIXER);
      if (!mOverflowSwitchInstalled->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI3_CU361, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_MIXER);
      }
      else
      {
        AssignDigitalInput(DI_INDEX_DI8_IO351_41, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_MIXER);
      }
      mMixerEnabled->SetValue(true);
    }
    else
    {
      mDoFunc[DO_INDEX_DO6_IO351_41]->SetValue(RELAY_FUNC_NO_FUNCTION);
      if (no_of_fsw <= 2 && !mOverflowSwitchInstalled->GetValue())
      {
        // If more than 2 float switches DI3 will be occupied by a float switch
        AssignDigitalInput(DI_INDEX_DI3_CU361, DIGITAL_INPUT_FUNC_NO_FUNCTION);
      }
      mMixerEnabled->SetValue(false);
    }

    // set stop level = 1.00 m
    mMixerStopLevel->SetValue(1.0f);

    // set start level offset = 0.05 m
    mMixerStartLevelOffset->SetValue(0.05f);
  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::InitializeOptionSettings(void)
  {
    for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
    {
      mPumpCFEnabled[pump_no]->SetValue(mPumpCFDiNo[pump_no]->GetAsInt() != 0);
      mPumpOOAEnabled[pump_no]->SetValue(mPumpOODiNo[pump_no]->GetAsInt() != 0 && mPumpAMDiNo[pump_no]->GetAsInt() != 0);
      mPumpMPEnabled[pump_no]->SetValue(mPumpMPDiNo[pump_no]->GetAsInt() != 0);
      mPumpVfdReadyEnabled[pump_no]->SetValue(mPumpVfdReadyDiNo[pump_no]->GetAsInt() != 0);
    }
    mOverflowSwitchInstalled->SetValue(mOverflowSwitchDiNo->GetAsInt() != 0);
    mCommonPhaseErrorEnabled->SetValue(mCommonPhaseDiNo->GetAsInt() != 0);
    mVolumeMeterEnabled->SetValue(mVolumeMeterDiNo->GetAsInt() != 0);
    mEnergyMeterEnabled->SetValue(mEnergyMeterDiNo->GetAsInt() != 0);
    mMixerCFEnabled->SetValue(mMixerCFDiNo->GetAsInt() != 0);
    mAlarmResetEnabled->SetValue(mAlarmResetDiNo->GetAsInt() != 0);
    mRelayResetEnabled->SetValue(mRelayResetDiNo->GetAsInt() != 0);
    mExternalFaultEnabled->SetValue(mExternalFaultDiNo->GetAsInt() != 0);
    mUserDefinedCounter1Enabled->SetValue(mUserDefinedCounter1DiNo->GetAsInt() != 0);
    mUserDefinedCounter2Enabled->SetValue(mUserDefinedCounter2DiNo->GetAsInt() != 0);
    mUserDefinedCounter3Enabled->SetValue(mUserDefinedCounter3DiNo->GetAsInt() != 0);
  }

  /*****************************************************************************
  * Function - CommitOptionSettings
  * DESCRIPTION: sets DI1-CU361 ... DI9-IO351
  *
  ****************************************************************************/
  void WizardCtrl::CommitOptionSettings(void)
  {
    std::vector<DIGITAL_INPUT_FUNC_TYPE> selected_options;
    std::vector<DI_INDEX_TYPE> available_inputs;

    selected_options.reserve(MAX_NO_OF_DIG_INPUTS);
    available_inputs.reserve(MAX_NO_OF_DIG_INPUTS);

    int no_of_fsw = mNoOfFsw->GetValue();
    int no_of_io351 = mNoOfIo351Modules->GetValue();
    bool mixer_need_allocation = mMixerInstalled->GetValue();
    bool overflow_switch_need_allocation = mOverflowSwitchInstalled->GetValue();

    SELECTED_COUNTER_INPUTS counter_inputs;
    counter_inputs.volume = mVolumeMeterEnabled->GetValue(); 
    counter_inputs.energy = mEnergyMeterEnabled->GetValue();
    counter_inputs.user1 = mUserDefinedCounter1Enabled->GetValue();
    counter_inputs.user2 = mUserDefinedCounter2Enabled->GetValue();
    counter_inputs.user3 = mUserDefinedCounter3Enabled->GetValue();

    bool only_high_level_switch_selected = ((no_of_fsw == 1) && (mFswConfigNumber->GetValue() == FSW_CONFIG_ONE_FSW_AS_HIGH_LEVEL));

    // all counter inputs have dedicated inputs, and is thus not included in selected_options collection

    if (mRelayResetEnabled->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_ALARM_RELAY_RESET);
    }
    if (mExternalFaultEnabled->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_EXTERNAL_FAULT);
    }
    if (mCommonPhaseErrorEnabled->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_FAIL_PHASE);
    }
    if (mAlarmResetEnabled->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_ALARM_RESET);
    }
    if (mPumpVfdReadyEnabled[PUMP_1]->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_VFD_READY_PUMP_1);
    }
    if (mPumpVfdReadyEnabled[PUMP_2]->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_VFD_READY_PUMP_2);
    }
    if (mPumpVfdReadyEnabled[PUMP_3]->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_VFD_READY_PUMP_3);
    }
    if (mPumpVfdReadyEnabled[PUMP_4]->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_VFD_READY_PUMP_4);
    }
    if (mPumpVfdReadyEnabled[PUMP_5]->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_VFD_READY_PUMP_5);
    }
    if (mPumpVfdReadyEnabled[PUMP_6]->GetValue())
    {
      selected_options.push_back(DIGITAL_INPUT_FUNC_VFD_READY_PUMP_6);
    }

    // evaluating availability of each digital input in sequence of priority: DI1-CU361...DI9-IO351-43

    /// Locating available digital inputs of CU 361
    if (no_of_fsw == 0)
    {
      available_inputs.push_back(DI_INDEX_DI1_CU361);
    }
    // else DI1-CU361 is set by fsw ctrl

    if (no_of_fsw <= 1)
    {
      if (only_high_level_switch_selected)
      {
        available_inputs.push_back(DI_INDEX_DI1_CU361);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI2_CU361);
      }
    }
    // else DI1-CU361 or DI2-CU361 is set by fsw ctrl

    //use DI3-CU361 for (highest priority first): float switch, overflow switch, mixer feedback, , option
    if (no_of_fsw <= 2)
    {
      if (overflow_switch_need_allocation)
      {
        overflow_switch_need_allocation = false;
        AssignDigitalInput(DI_INDEX_DI3_CU361, DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH);
      }
      else if (mixer_need_allocation && mMixerCFEnabled->GetValue())
      {// mixer will only be used in a configuration with up to 2 float switches 
        mixer_need_allocation = false;
        AssignDigitalInput(DI_INDEX_DI3_CU361, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_MIXER);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI3_CU361);
      }
    }
    // else DI3-CU361 is set by fsw ctrl

    /// Locating available digital inputs of IO351 module 41
    if (no_of_io351 > 0)
    {
      AssignCounterInput(DI_INDEX_DI1_IO351_41, &counter_inputs, &available_inputs);

      if (mPumpCFEnabled[PUMP_1]->GetValue() == true)
      {
        AssignDigitalInput(DI_INDEX_DI2_IO351_41, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_1);
      }
      else
      {
        AssignCounterInput(DI_INDEX_DI2_IO351_41, &counter_inputs, &available_inputs);
      }

      if (mPumpCFEnabled[PUMP_2]->GetValue() == true)
      {
        AssignDigitalInput(DI_INDEX_DI3_IO351_41, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_2);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI3_IO351_41);
      }

      if (mPumpOOAEnabled[PUMP_1]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI4_IO351_41, DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_1);
        AssignDigitalInput(DI_INDEX_DI5_IO351_41, DIGITAL_INPUT_FUNC_ON_OFF_PUMP_1);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI4_IO351_41);
        available_inputs.push_back(DI_INDEX_DI5_IO351_41);
      }

      if (mPumpOOAEnabled[PUMP_2]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI6_IO351_41, DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_2);
        AssignDigitalInput(DI_INDEX_DI7_IO351_41, DIGITAL_INPUT_FUNC_ON_OFF_PUMP_2);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI6_IO351_41);
        available_inputs.push_back(DI_INDEX_DI7_IO351_41);
      }

      if (no_of_fsw <= 3)
      {
        if (mixer_need_allocation)
        {
          mixer_need_allocation = false;
          AssignDigitalInput(DI_INDEX_DI8_IO351_41, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_MIXER);

          if (mPumpMPEnabled[PUMP_1]->GetValue())
          {
            selected_options.push_back(DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_1);
          }
        }
        else if (mPumpMPEnabled[PUMP_1]->GetValue())
        {
          AssignDigitalInput(DI_INDEX_DI8_IO351_41, DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_1);
        }
        else
        {
          available_inputs.push_back(DI_INDEX_DI8_IO351_41);
        }

        if (mPumpMPEnabled[PUMP_2]->GetValue())
        {
          AssignDigitalInput(DI_INDEX_DI9_IO351_41, DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_2);
        }
        else
        {
          available_inputs.push_back(DI_INDEX_DI9_IO351_41);
        }
      }
      else if (no_of_fsw == 4)
      {
        // FSW_4 is placed at DI 9 - FSW_5 is at DI 8 and thus is freed.
        available_inputs.push_back(DI_INDEX_DI8_IO351_41);
      }
      // else DI8_IO351 + DI9_IO351 is set by fsw ctrl
    }


    
    /// Locating available digital inputs of IO351 module 42
    if (no_of_io351 > 1)
    {      
      AssignCounterInput(DI_INDEX_DI1_IO351_42, &counter_inputs, &available_inputs);

      if (mPumpCFEnabled[PUMP_3]->GetValue() == true)
      {
        AssignDigitalInput(DI_INDEX_DI2_IO351_42, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_3);
      }
      else
      {
        AssignCounterInput(DI_INDEX_DI2_IO351_42, &counter_inputs, &available_inputs);
      }

      if (mPumpCFEnabled[PUMP_4]->GetValue() == true)
      {
        AssignDigitalInput(DI_INDEX_DI3_IO351_42, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_4);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI3_IO351_42);
      }

      if (mPumpOOAEnabled[PUMP_3]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI4_IO351_42, DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_3);
        AssignDigitalInput(DI_INDEX_DI5_IO351_42, DIGITAL_INPUT_FUNC_ON_OFF_PUMP_3);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI4_IO351_42);
        available_inputs.push_back(DI_INDEX_DI5_IO351_42);
      }

      if (mPumpOOAEnabled[PUMP_4]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI6_IO351_42, DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_4);
        AssignDigitalInput(DI_INDEX_DI7_IO351_42, DIGITAL_INPUT_FUNC_ON_OFF_PUMP_4);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI6_IO351_42);
        available_inputs.push_back(DI_INDEX_DI7_IO351_42);
      }

      if (mPumpMPEnabled[PUMP_3]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI8_IO351_42, DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_3);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI8_IO351_42);
      }

      if (mPumpMPEnabled[PUMP_4]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI9_IO351_42, DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_4);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI9_IO351_42);
      }
    }


    /// Locating available digital inputs of IO351 module 43
    if (no_of_io351 > 2)
    {
      if (mPumpCFEnabled[PUMP_5]->GetValue() == true)
      {
        AssignDigitalInput(DI_INDEX_DI1_IO351_43, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_5);
      }
      else
      {
        AssignCounterInput(DI_INDEX_DI1_IO351_43, &counter_inputs, &available_inputs);
      }

      if (mPumpCFEnabled[PUMP_6]->GetValue() == true)
      {
        AssignDigitalInput(DI_INDEX_DI2_IO351_43, DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_6);
      }
      else
      {
        AssignCounterInput(DI_INDEX_DI2_IO351_43, &counter_inputs, &available_inputs);
      }


      if (mPumpOOAEnabled[PUMP_5]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI3_IO351_43, DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_5);
        AssignDigitalInput(DI_INDEX_DI4_IO351_43, DIGITAL_INPUT_FUNC_ON_OFF_PUMP_5);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI3_IO351_43);
        available_inputs.push_back(DI_INDEX_DI4_IO351_43);
      }

      if (mPumpOOAEnabled[PUMP_6]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI5_IO351_43, DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_6);
        AssignDigitalInput(DI_INDEX_DI6_IO351_43, DIGITAL_INPUT_FUNC_ON_OFF_PUMP_6);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI5_IO351_43);
        available_inputs.push_back(DI_INDEX_DI6_IO351_43);
      }

      if (mPumpMPEnabled[PUMP_5]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI7_IO351_43, DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_5);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI7_IO351_43);
      }

      if (mPumpMPEnabled[PUMP_6]->GetValue())
      {
        AssignDigitalInput(DI_INDEX_DI8_IO351_43, DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_6);
      }
      else
      {
        available_inputs.push_back(DI_INDEX_DI8_IO351_43);
      }
      
      available_inputs.push_back(DI_INDEX_DI9_IO351_43);
    }

    // All available DI slots found, time to match selected options with available inputs

    std::vector<DIGITAL_INPUT_FUNC_TYPE>::iterator option_iter = selected_options.begin();
    std::vector<DIGITAL_INPUT_FUNC_TYPE>::iterator option_iterEnd = selected_options.end();
    std::vector<DI_INDEX_TYPE>::iterator free_slot_iter = available_inputs.begin();
    std::vector<DI_INDEX_TYPE>::iterator free_slot_iterEnd = available_inputs.end();

    for(; free_slot_iter != free_slot_iterEnd; free_slot_iter++)
    {
      if (option_iter != option_iterEnd)
      {
        AssignDigitalInput(*free_slot_iter, *option_iter);

        option_iter++;
      }
      else
      {
        AssignDigitalInput(*free_slot_iter, DIGITAL_INPUT_FUNC_NO_FUNCTION);
      }
    }

    // set logic of all inputs to NORMALLY_OPEN
    for(int i = DI_INDEX_DI1_CU361; i <= (DI_INDEX_DI3_CU361 + no_of_io351 * NO_OF_DIG_INPUTS_IO351); i++)
    {
      mDiLogic[i]->SetValue(false);
    }

  }

  /*****************************************************************************
  * Function - AssignDigitalInput
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::AssignDigitalInput(DI_INDEX_TYPE di, DIGITAL_INPUT_FUNC_TYPE input)
  {
    mDiFunc[di]->SetValue(input);
  }

  /*****************************************************************************
  * Function - AssignCounterInput
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::AssignCounterInput(DI_INDEX_TYPE di, SELECTED_COUNTER_INPUTS* inputs, std::vector<DI_INDEX_TYPE>* pAvailableInputs)
  {
    if (inputs->energy)
    {
      inputs->energy = false;
      mDiFunc[di]->SetValue(DIGITAL_INPUT_FUNC_ENERGY_CNT);
    }
    else if (inputs->volume)
    {
      inputs->volume = false;
      mDiFunc[di]->SetValue(DIGITAL_INPUT_FUNC_VOLUME_CNT);
    }
    else if (inputs->user1)
    {
      inputs->user1 = false;
      mDiFunc[di]->SetValue(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1);
    }
    else if (inputs->user2)
    {
      inputs->user2 = false;
      mDiFunc[di]->SetValue(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2);
    }
    else if (inputs->user3)
    {
      inputs->user3 = false;
      mDiFunc[di]->SetValue(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3);
    }
    else
    {
      pAvailableInputs->push_back(di);
    }
  }



  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  void WizardCtrl::CommitCommunicationSettings(void)
  {
    const int minPhoneNoLength = 1;

    if (strlen(mScadaNo->GetValue()) > minPhoneNoLength)
    {
      //A scada number has been entered
      mEnableScadaInAlarmConfigs->SetEvent();
    }

  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  int WizardCtrl::CalcFreeDigitalInputsCount(void)
  {
    int free_di = NO_OF_DIG_INPUTS_IOB + (NO_OF_DIG_INPUTS_IO351 * mNoOfIo351Modules->GetValue());
    int no_of_fsw = mNoOfFsw->GetValue();

    free_di -= no_of_fsw;

    if (mOverflowSwitchInstalled->GetValue() && no_of_fsw != MAX_NO_OF_FLOAT_SWITCHES)
    {
      // overflow switch is not allowed if 5 other switches are installed.
      free_di--;
    }

    for (int pump_no = FIRST_PUMP_NO; pump_no < mNoOfPumps->GetValue(); pump_no++)
    {
      if (mPumpCFEnabled[pump_no]->GetValue())
        free_di--;

      if (mPumpOOAEnabled[pump_no]->GetValue())
        free_di -= 2;

      if (mPumpMPEnabled[pump_no]->GetValue())
        free_di--;

      if (mPumpVfdReadyEnabled[pump_no]->GetValue())
        free_di--;
    }

    if (mCommonPhaseErrorEnabled->GetValue())
      free_di--;

    if (mVolumeMeterEnabled->GetValue())
      free_di--;

    if (mEnergyMeterEnabled->GetValue())
      free_di--;

    if (mMixerInstalled->GetValue() && mMixerCFEnabled->GetValue())
      free_di--;

    if (mAlarmResetEnabled->GetValue())
      free_di--;

    if (mRelayResetEnabled->GetValue())
      free_di--;

    if (mExternalFaultEnabled->GetValue())
      free_di--;

    if (mUserDefinedCounter1Enabled->GetValue())
      free_di--;   

    if (mUserDefinedCounter2Enabled->GetValue())
      free_di--;  

    if (mUserDefinedCounter3Enabled->GetValue())
      free_di--;  

    return free_di;
  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  bool WizardCtrl::IsOptionComboLegal(void)
  {
    if ( mNoOfIo351Modules->GetValue() == 1
      && mPumpCFEnabled[PUMP_1]->GetValue()
      && mEnergyMeterEnabled->GetValue()
      && mVolumeMeterEnabled->GetValue()
      && mUserDefinedCounter1Enabled->GetValue()
      && mUserDefinedCounter2Enabled->GetValue()
      && mUserDefinedCounter3Enabled->GetValue())
    {
      return false;
    }

    return true;
  }

  /*****************************************************************************
  * Function -
  * DESCRIPTION:
  *
  ****************************************************************************/
  bool WizardCtrl::OneOrMoreVfdInstalled()
  {

    for (int i = FIRST_PUMP; i < mNoOfPumps->GetValue(); i++)
    {
      if (mVfdInstalled[i]->GetValue())
      {
        return true;
      }
    }
    
    return false;
  }



  

  /*****************************************************************************
  *
  *
  *              PROTECTED FUNCTIONS
  *                 - RARE USED -
  *
  ****************************************************************************/
    } // namespace ctrl
  } // namespace display
} // namespace mpc
