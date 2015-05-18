/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MidRange                                      */
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
/* CLASS NAME       : LevelCtrl                                             */
/*                                                                          */
/* FILE NAME        : LevelCtrl.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 20-05-2009  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <LevelCtrl.h>
#include <float.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  LOCAL CONST VARIABLES
 *****************************************************************************/



/*****************************************************************************
  C functions declarations
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
 * DESCRIPTION: This is the constructor for the class, to construct
 * an object of the class type
 *****************************************************************************/
LevelCtrl::LevelCtrl(void)
{
  mFirstRunInAnalogueMode = true;
  mOldNoOfPumps = 0xFF;
  mOldHighLevel = false;

  for (unsigned int i = FIRST_LEVEL_FAULT; i < NO_OF_LEVEL_FAULT; i++)
  {
    mAlarmDelays[i] = new AlarmDelay(this);
    mAlarmDelayCheckFlags[i] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
LevelCtrl::~LevelCtrl(void)
{
  for (unsigned int i = FIRST_LEVEL_FAULT; i < NO_OF_LEVEL_FAULT; i++)
  {
    delete mAlarmDelays[i];
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void LevelCtrl::InitSubTask(void)
{
  mpDryRunAlarmBlockedByFoamDrainTimer = new SwTimer(10, S, false, false, this);
  mpDryRunWarningBlockedByFoamDrainTimer = new SwTimer(10, S, false, false, this);
  mpHighLevelAfterRunTimer = new SwTimer(10, S, false, false, this);
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void LevelCtrl::RunSubTask(void)
{
  if (mpPitLevelCtrlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)
  {
    if (!mFirstRunInAnalogueMode)
    {
      mAlarms[LEVEL_FAULT_ALARM_LEVEL]->SetErrorPresent(ALARM_STATE_READY);
      mAlarms[LEVEL_FAULT_SENSOR]->SetErrorPresent(ALARM_STATE_READY);
      mAlarms[LEVEL_FAULT_CONFLICT_LEVELS]->SetErrorPresent(ALARM_STATE_READY);
    }

    mFirstRunInAnalogueMode = true;
    return;
  }

  if (mFirstRunInAnalogueMode)
  {
    mpAlarmLevelAlarmConf->SetAlarmConfigType(AC_FLOAT);
    mpDryRunAlarmConf->SetAlarmConfigType(AC_FLOAT);
    mpHighLevelAlarmConf->SetAlarmConfigType(AC_FLOAT);

    for (unsigned int i = FIRST_LEVEL_FAULT; i < NO_OF_LEVEL_FAULT; i++)
    {
      mAlarmDelays[i]->InitAlarmDelay();
      mAlarmDelays[i]->ResetFault();
      mAlarmDelays[i]->ResetWarning();
      mAlarmDelayCheckFlags[i] = false;
    }

    mpDryRunAlarmBlockedByFoamDrainTimer->StopSwTimer();
    mpDryRunWarningBlockedByFoamDrainTimer->StopSwTimer();
    mpHighLevelAfterRunTimer->StopSwTimer();

    mOldHighLevel = false;
  }


  // Service AlarmDelays

  for (unsigned int i = FIRST_LEVEL_FAULT; i < NO_OF_LEVEL_FAULT; i++)
  {
    if (mAlarmDelayCheckFlags[i] == true)
    {
      mAlarmDelayCheckFlags[i] = false;
      mAlarmDelays[i]->CheckErrorTimers();
    }
  }


  // Trigger foam drain block timers

  if (mpFoamDrainActive->GetValue())
  {
    mpDryRunAlarmBlockedByFoamDrainTimer->RetriggerSwTimer();
    mpDryRunWarningBlockedByFoamDrainTimer->RetriggerSwTimer();
  }


  // Read float switches and set switch-status DataPoints

  bool dry_run_sw = true;
  bool high_level_sw = false;
  GF_UINT8 fsw_status = 0;

  if (mpFloatSwitchCnf[0]->GetQuality() == DP_AVAILABLE)
  {
    if (mpFloatSwitchCnf[0]->GetValue() == FSW_HIGH_WATER)
    {
      high_level_sw = mpFloatSwitch[0]->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE;

      if (high_level_sw)
      {
        fsw_status = 0x01;
      }
    } // if (mpFloatSwitchCnf[0]->GetValue() == FSW_HIGH_WATER)
    else if (mpFloatSwitchCnf[0]->GetValue() == FSW_DRY_RUN)
    {
      dry_run_sw = mpFloatSwitch[0]->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE;

      if (dry_run_sw)
      {
        fsw_status = 0x01;
      }

      if (mpFloatSwitchCnf[1]->GetQuality() == DP_AVAILABLE)
      {
        if (mpFloatSwitchCnf[1]->GetValue() == FSW_HIGH_WATER)
        {
          high_level_sw = mpFloatSwitch[1]->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE;

          if (high_level_sw)
          {
            fsw_status |= 0x02;
          }
        } // if (mpFloatSwitchCnf[1]->GetValue() == FSW_HIGH_WATER)
      } // if (mpFloatSwitchCnf[1]->GetQuality() == DP_AVAILABLE)
    } // else if (mpFloatSwitchCnf[0]->GetValue() == FSW_DRY_RUN)
  } // if (mpFloatSwitchCnf[0]->GetQuality() == DP_AVAILABLE)

  mpFloatSwitchDiStatus->SetValue(fsw_status);
  mpHighLevelSwitchActivated->SetValue(high_level_sw);


  // Calc PumpRunForLowestStopLevel DataPoints

  GF_UINT8 no_of_pumps = mpNoOfPumps->GetValue();
  float pump_stop_levels[NO_OF_PUMPS];
  float lowest_stop_level = FLT_MAX;

  for (unsigned int i = 0; i < no_of_pumps; i++)
  {
    pump_stop_levels[i] = mpStopLevelPump[i]->GetValue();

    if (lowest_stop_level > pump_stop_levels[i])
    {
      lowest_stop_level = pump_stop_levels[i];
    }
  }

  for (unsigned int i = 0; i < no_of_pumps; i++)
  {
    mpPumpRunForLowestStopLevel[i]->SetValue(lowest_stop_level == pump_stop_levels[i]);
  }


  // Check analogue sensor and compare with float switches

  bool surface_level_ready = mpSurfaceLevelReady->GetValue();
  bool sensor_fault = surface_level_ready && mpSurfaceLevel->GetQuality() != DP_AVAILABLE;
  bool use_analogue_sensor = surface_level_ready && !sensor_fault && mAlarms[LEVEL_FAULT_SENSOR]->GetAlarmPresent() == ALARM_STATE_READY;
  float surface_level = use_analogue_sensor ? mpSurfaceLevel->GetValue() : 0;

  bool high_level_alarm = false;
  bool high_level_warning = false;

  bool dry_run_alarm = false;
  bool dry_run_warning = false;

  bool alarm_level_alarm = false;
  bool alarm_level_warning = false;

  bool conflicting_levels = false;

  // Switches take priority over analogue sensor
  high_level_alarm = high_level_sw;
  if (!high_level_alarm) // High level takes priority over dry run
  {
    dry_run_alarm = !dry_run_sw;
  }

  if (use_analogue_sensor)
  {
    bool high_level_analogue_alarm = surface_level >= mpHighLevelAlarmConf->GetAlarmLimit()->GetAsFloat();
    bool dry_run_analogue_alarm = surface_level <= mpDryRunAlarmConf->GetAlarmLimit()->GetAsFloat();

    conflicting_levels = high_level_alarm && !high_level_analogue_alarm || dry_run_alarm && !dry_run_analogue_alarm;

    if (!conflicting_levels)
    {
      bool high_level_analogue_warning = surface_level >= mpHighLevelAlarmConf->GetWarningLimit()->GetAsFloat();
      bool dry_run_analogue_warning = surface_level <= mpDryRunAlarmConf->GetWarningLimit()->GetAsFloat();

      bool alarm_level_analogue_alarm = surface_level >= mpAlarmLevelAlarmConf->GetAlarmLimit()->GetAsFloat();
      bool alarm_level_analogue_warning = surface_level >= mpAlarmLevelAlarmConf->GetWarningLimit()->GetAsFloat();

      high_level_alarm |= high_level_analogue_alarm;
      high_level_warning = high_level_analogue_warning;

      alarm_level_alarm = alarm_level_analogue_alarm;
      alarm_level_warning = alarm_level_analogue_warning;

      dry_run_alarm |= dry_run_analogue_alarm;
      dry_run_warning = dry_run_analogue_warning;
    }
  }

  if (conflicting_levels || mAlarms[LEVEL_FAULT_CONFLICT_LEVELS]->GetAlarmPresent() != ALARM_STATE_READY)
  {
    use_analogue_sensor = false;
  }

  // Calc UnderDryRunLevel and UnderLowestStopLevel

  if (use_analogue_sensor)
  {
    mpUnderDryRunLevel->SetValue(dry_run_alarm);

    bool under_lowest_stop_level = true;

    for (unsigned int i = 0; i < no_of_pumps; i++)
    {
      if (surface_level > pump_stop_levels[i])
      {
        under_lowest_stop_level = false;
        break;
      }
    }
    mpUnderLowestStopLevel->SetValue(under_lowest_stop_level);
  }
  else
  {
    mpUnderDryRunLevel->SetQuality(DP_NOT_AVAILABLE);
    mpUnderLowestStopLevel->SetQuality(DP_NOT_AVAILABLE);
  }


  // Set/reset alarms

  if (!dry_run_sw && high_level_sw) mAlarmDelays[LEVEL_FAULT_FSW_INCONSISTENCY]->SetFault();
  else                              mAlarmDelays[LEVEL_FAULT_FSW_INCONSISTENCY]->ResetFault();

  if (sensor_fault) mAlarmDelays[LEVEL_FAULT_SENSOR]->SetFault();
  else              mAlarmDelays[LEVEL_FAULT_SENSOR]->ResetFault();

  if (conflicting_levels) mAlarmDelays[LEVEL_FAULT_CONFLICT_LEVELS]->SetFault();
  else                    mAlarmDelays[LEVEL_FAULT_CONFLICT_LEVELS]->ResetFault();


  if (high_level_alarm) mAlarmDelays[LEVEL_FAULT_HIGH_LEVEL ]->SetFault();
  else                  mAlarmDelays[LEVEL_FAULT_HIGH_LEVEL ]->ResetFault();

  if (high_level_warning) mAlarmDelays[LEVEL_FAULT_HIGH_LEVEL ]->SetWarning();
  else                    mAlarmDelays[LEVEL_FAULT_HIGH_LEVEL ]->ResetWarning();

  if (alarm_level_alarm) mAlarmDelays[LEVEL_FAULT_ALARM_LEVEL]->SetFault();
  else                   mAlarmDelays[LEVEL_FAULT_ALARM_LEVEL]->ResetFault();

  if (alarm_level_warning) mAlarmDelays[LEVEL_FAULT_ALARM_LEVEL]->SetWarning();
  else                     mAlarmDelays[LEVEL_FAULT_ALARM_LEVEL]->ResetWarning();

  if (dry_run_alarm)
  {
    if (!mpDryRunAlarmBlockedByFoamDrainTimer->GetSwTimerStatus())
    {
      mAlarmDelays[LEVEL_FAULT_DRY_RUNNING]->SetFault();
    }
    else
    {
      mpDryRunAlarmBlockedByFoamDrainTimer->RetriggerSwTimer();
      mAlarmDelays[LEVEL_FAULT_DRY_RUNNING]->ResetFault();
    }
  }
  else
  {
    mAlarmDelays[LEVEL_FAULT_DRY_RUNNING]->ResetFault();
  }

  if (dry_run_warning)
  {
    if (!mpDryRunWarningBlockedByFoamDrainTimer->GetSwTimerStatus())
    {
      mAlarmDelays[LEVEL_FAULT_DRY_RUNNING]->SetWarning();
    }
    else
    {
      mpDryRunWarningBlockedByFoamDrainTimer->RetriggerSwTimer();
      mAlarmDelays[LEVEL_FAULT_DRY_RUNNING]->ResetWarning();
    }
  }
  else
  {
    mAlarmDelays[LEVEL_FAULT_DRY_RUNNING]->ResetWarning();
  }


  // Start/stop pumps

  if (mOldHighLevel && !high_level_alarm)
  {
    if (!use_analogue_sensor)
    {
      mpHighLevelAfterRunTimer->SetSwTimerPeriod(mpAfterRunHighLevelDelay->GetValue(), S, false);
      mpHighLevelAfterRunTimer->RetriggerSwTimer();
    }
  }
  mOldHighLevel = high_level_alarm;

  mpHighLevelStateDetected->SetValue(high_level_alarm);
  if (high_level_alarm)
  {
    for (unsigned int i = 0; i < no_of_pumps; i++)
    {
      mpPumpRefLevel[i]->SetValue(true);
    }	

  }
  else if (dry_run_alarm)
  {
    for (unsigned int i = 0; i < no_of_pumps; i++)
    {
      mpPumpRefLevel[i]->SetValue(false);
    }
    mpHighLevelAfterRunTimer->StopSwTimer();
  }
  else if (use_analogue_sensor)
  {
    for (unsigned int i = 0; i < no_of_pumps; i++)
    {
      if (surface_level >= mpStartLevelPump[i]->GetValue())
      {
        mpPumpRefLevel[i]->SetValue(true);
      }
      else if ((surface_level <= pump_stop_levels[i] || mFirstRunInAnalogueMode)) // Pumps neither specifically started nor stopped, are stopped the first time around.
      {
		if(!mpHighLevelAfterRunTimer->GetSwTimerStatus())
        mpPumpRefLevel[i]->SetValue(false);
      }
    }
  //  mpHighLevelAfterRunTimer->StopSwTimer();
  }
  else
  {
    if (mpHighLevelAfterRunTimer->GetSwTimerStatus())
    {
      for (unsigned int i = 0; i < no_of_pumps; i++)
      {
        mpPumpRefLevel[i]->SetValue(true);
      }
    }
    else
    {
      for (unsigned int i = 0; i < no_of_pumps; i++)
      {
        mpPumpRefLevel[i]->SetValue(false);
      }
    }
  }


  // Set non-existing pump data to never available

  if (mOldNoOfPumps != no_of_pumps || mFirstRunInAnalogueMode)
  {
    mOldNoOfPumps = no_of_pumps;

    for (unsigned int i = no_of_pumps; i < MAX_NO_OF_PUMPS; i++)
    {
      mpPumpRefLevel[i]->SetQuality(DP_NEVER_AVAILABLE);
      mpPumpRunForLowestStopLevel[i]->SetQuality(DP_NEVER_AVAILABLE);
    }
  }


  // Service AlarmDelays

  for (unsigned int i = FIRST_LEVEL_FAULT; i < NO_OF_LEVEL_FAULT; i++)
  {
    mAlarmDelays[i]->UpdateAlarmDataPoint();
  }


  // Done

  mFirstRunInAnalogueMode = false;
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void LevelCtrl::Update(Subject* pSubject)
{
  for (unsigned int i = FIRST_LEVEL_FAULT; i < NO_OF_LEVEL_FAULT; i++)
  {
    if (pSubject == mAlarmDelays[i])
    {
      mAlarmDelayCheckFlags[i] = true;
      break;
    }
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void LevelCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (unsigned int i = FIRST_LEVEL_FAULT; i < NO_OF_LEVEL_FAULT; i++)
  {
    mAlarmDelays[i]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void LevelCtrl::ConnectToSubjects(void)
{
  for (unsigned int i = FIRST_LEVEL_FAULT; i < NO_OF_LEVEL_FAULT; i++)
  {
    mAlarmDelays[i]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void LevelCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
    case SP_LC_PIT_LEVEL_CTRL_TYPE:                    mpPitLevelCtrlType                 .Attach(pSubject); break;
    case SP_LC_SURFACE_LEVEL_READY:                    mpSurfaceLevelReady                .Attach(pSubject); break;
    case SP_LC_PIT_LEVEL_MEASURED:                     mpSurfaceLevel                     .Attach(pSubject); break;
    case SP_LC_FLOAT_SWITCH_1:                         mpFloatSwitch[0]                   .Attach(pSubject); break;
    case SP_LC_FLOAT_SWITCH_2:                         mpFloatSwitch[1]                   .Attach(pSubject); break;
    case SP_LC_FLOAT_SWITCH_CNF_1:                     mpFloatSwitchCnf[0]                .Attach(pSubject); break;
    case SP_LC_FLOAT_SWITCH_CNF_2:                     mpFloatSwitchCnf[1]                .Attach(pSubject); break;
    case SP_LC_FOAM_DRAIN_ACTIVE:                      mpFoamDrainActive                  .Attach(pSubject); break;
    case SP_LC_DRY_RUN_LEVEL:                          mpDryRunAlarmConf                  .Attach(pSubject); break;
    case SP_LC_HIGH_LEVEL:                             mpHighLevelAlarmConf               .Attach(pSubject); break;
    case SP_LC_ALARM_LEVEL:                            mpAlarmLevelAlarmConf              .Attach(pSubject); break;
    case SP_LC_START_LEVEL_PUMP_1:                     mpStartLevelPump[PUMP_1]           .Attach(pSubject); break;
    case SP_LC_START_LEVEL_PUMP_2:                     mpStartLevelPump[PUMP_2]           .Attach(pSubject); break;
    case SP_LC_START_LEVEL_PUMP_3:                     mpStartLevelPump[PUMP_3]           .Attach(pSubject); break;
    case SP_LC_START_LEVEL_PUMP_4:                     mpStartLevelPump[PUMP_4]           .Attach(pSubject); break;
    case SP_LC_START_LEVEL_PUMP_5:                     mpStartLevelPump[PUMP_5]           .Attach(pSubject); break;
    case SP_LC_START_LEVEL_PUMP_6:                     mpStartLevelPump[PUMP_6]           .Attach(pSubject); break;
    case SP_LC_STOP_LEVEL_PUMP_1:                      mpStopLevelPump[PUMP_1]            .Attach(pSubject); break;
    case SP_LC_STOP_LEVEL_PUMP_2:                      mpStopLevelPump[PUMP_2]            .Attach(pSubject); break;
    case SP_LC_STOP_LEVEL_PUMP_3:                      mpStopLevelPump[PUMP_3]            .Attach(pSubject); break;
    case SP_LC_STOP_LEVEL_PUMP_4:                      mpStopLevelPump[PUMP_4]            .Attach(pSubject); break;
    case SP_LC_STOP_LEVEL_PUMP_5:                      mpStopLevelPump[PUMP_5]            .Attach(pSubject); break;
    case SP_LC_STOP_LEVEL_PUMP_6:                      mpStopLevelPump[PUMP_6]            .Attach(pSubject); break;
    case SP_LC_NO_OF_PUMPS:                            mpNoOfPumps                        .Attach(pSubject); break;
    case SP_LC_AFTER_RUN_HIGH_LEVEL_DELAY:             mpAfterRunHighLevelDelay           .Attach(pSubject); break;
    case SP_LC_HIGH_LEVEL_SWITCH_ACTIVATED:            mpHighLevelSwitchActivated         .Attach(pSubject); break;
    case SP_LC_FLOAT_SWITCH_DI_STATUS:                 mpFloatSwitchDiStatus              .Attach(pSubject); break;
    case SP_LC_UNDER_DRY_RUN_LEVEL:                    mpUnderDryRunLevel                 .Attach(pSubject); break;
    case SP_LC_UNDER_LOWEST_STOP_LEVEL:                mpUnderLowestStopLevel             .Attach(pSubject); break;
    case SP_LC_HIGH_LEVEL_STATE_DETECTED:              mpHighLevelStateDetected           .Attach(pSubject); break;
    case SP_LC_PUMP_1_RUN_FOR_LOWEST_STOP_LEVEL:       mpPumpRunForLowestStopLevel[PUMP_1].Attach(pSubject); break;
    case SP_LC_PUMP_2_RUN_FOR_LOWEST_STOP_LEVEL:       mpPumpRunForLowestStopLevel[PUMP_2].Attach(pSubject); break;
    case SP_LC_PUMP_3_RUN_FOR_LOWEST_STOP_LEVEL:       mpPumpRunForLowestStopLevel[PUMP_3].Attach(pSubject); break;
    case SP_LC_PUMP_4_RUN_FOR_LOWEST_STOP_LEVEL:       mpPumpRunForLowestStopLevel[PUMP_4].Attach(pSubject); break;
    case SP_LC_PUMP_5_RUN_FOR_LOWEST_STOP_LEVEL:       mpPumpRunForLowestStopLevel[PUMP_5].Attach(pSubject); break;
    case SP_LC_PUMP_6_RUN_FOR_LOWEST_STOP_LEVEL:       mpPumpRunForLowestStopLevel[PUMP_6].Attach(pSubject); break;
    case SP_LC_PUMP_REF_LEVEL_1:                       mpPumpRefLevel[PUMP_1]             .Attach(pSubject); break;
    case SP_LC_PUMP_REF_LEVEL_2:                       mpPumpRefLevel[PUMP_2]             .Attach(pSubject); break;
    case SP_LC_PUMP_REF_LEVEL_3:                       mpPumpRefLevel[PUMP_3]             .Attach(pSubject); break;
    case SP_LC_PUMP_REF_LEVEL_4:                       mpPumpRefLevel[PUMP_4]             .Attach(pSubject); break;
    case SP_LC_PUMP_REF_LEVEL_5:                       mpPumpRefLevel[PUMP_5]             .Attach(pSubject); break;
    case SP_LC_PUMP_REF_LEVEL_6:                       mpPumpRefLevel[PUMP_6]             .Attach(pSubject); break;

    case SP_LC_FLOAT_SWITCH_INCONSISTENCY_FAULT_OBJ:   mAlarms[LEVEL_FAULT_FSW_INCONSISTENCY].Attach(pSubject); mAlarmDelays[LEVEL_FAULT_FSW_INCONSISTENCY]->SetSubjectPointer(id, pSubject); break;
    case SP_LC_FLOAT_SWITCH_SENSOR_FAULT_OBJ:          mAlarms[LEVEL_FAULT_SENSOR           ].Attach(pSubject); mAlarmDelays[LEVEL_FAULT_SENSOR           ]->SetSubjectPointer(id, pSubject); break;
    case SP_LC_FLOAT_SWITCH_CONFLICT_LEVELS_FAULT_OBJ: mAlarms[LEVEL_FAULT_CONFLICT_LEVELS  ].Attach(pSubject); mAlarmDelays[LEVEL_FAULT_CONFLICT_LEVELS  ]->SetSubjectPointer(id, pSubject); break;
    case SP_LC_FLOAT_SWITCH_HIGH_LEVEL_FAULT_OBJ:      mAlarms[LEVEL_FAULT_HIGH_LEVEL       ].Attach(pSubject); mAlarmDelays[LEVEL_FAULT_HIGH_LEVEL       ]->SetSubjectPointer(id, pSubject); break;
    case SP_LC_FLOAT_SWITCH_ALARM_LEVEL_OBJ:           mAlarms[LEVEL_FAULT_ALARM_LEVEL      ].Attach(pSubject); mAlarmDelays[LEVEL_FAULT_ALARM_LEVEL      ]->SetSubjectPointer(id, pSubject); break;
    case SP_LC_FLOAT_SWITCH_DRY_RUN_FAULT_OBJ:         mAlarms[LEVEL_FAULT_DRY_RUNNING      ].Attach(pSubject); mAlarmDelays[LEVEL_FAULT_DRY_RUNNING      ]->SetSubjectPointer(id, pSubject); break;
  }
}



/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/




/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

