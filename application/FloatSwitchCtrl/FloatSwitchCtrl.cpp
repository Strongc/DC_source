/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MidRagne                                      */
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
/* CLASS NAME       : FloatSwitchCtrl                                       */
/*                                                                          */
/* FILE NAME        : FloatSwitchCtrl.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 09-07-2007  (dd-mm-yyyy)                              */
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
#include <FloatSwitchCtrl.h>
#include <AlarmDef.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define TA_OR_MASK (~0x80)

#define DRY_RUN_A_MASK      (0x01)
#define DRY_RUN_MASK        ((0x01 << 7))
#define HIGH_WATER_A_MASK   ((0x01 << 6))
#define HIGH_WATER_MASK_7   ((0x01 << 7))
#define HIGH_WATER_MASK_8   ((0x01 << 8))


// SW Timers
enum
{
  AFTER_RUN_DELAY_TIMER,
  AFTER_RUN_HIGH_LEVEL_DELAY_TIMER,
  FOAM_DRAIN_INTERVAL_TIMER,
  FOAM_DRAIN_TIMER,
  FOAM_DRAIN_CROSSED_DRY_RUN_ALARM_TIMER,
  FOAM_DRAIN_CROSSED_DRY_RUN_WARN_TIMER
};


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  LOCAL CONST VARIABLES
 *****************************************************************************/

/* Transision table for output fs_pump_ref NOT_ACTIVE to ACTIVE for floatswitch configurations */
static const FSW_CONFIG_TRANSISION_ACTION not_active_2_active[] =
{
  FSW_NOT_DEFINED,  TA_NONE,              TA_NONE,  TA_NONE,   TA_NONE,
  FSW_DRY_RUN,      TA_STOP_BOTH_PUMPS,   TA_NONE,  TA_FALSE,  TA_NONE,
  FSW_DRY_RUN_A,    TA_STOP_BOTH_PUMPS,   TA_NONE,  TA_FALSE,  TA_NONE,
  FSW_STOP,         TA_NONE,              TA_NONE,  TA_NONE,   TA_NONE,
  FSW_STOP1,        TA_NONE,              TA_NONE,  TA_NONE,   TA_NONE,
  FSW_STOP2,        TA_NONE,              TA_NONE,  TA_NONE,   TA_NONE,
  FSW_START1_STOP,  TA_START_PUMP_1,      TA_NONE,  TA_NONE,   TA_NONE,
  FSW_START1,       TA_START_PUMP_1,      TA_NONE,  TA_NONE,   TA_NONE,
  FSW_START2,       TA_START_BOTH_PUMPS,  TA_NONE,  TA_NONE,   TA_NONE,
  FSW_ALARM,        TA_NONE,              TA_NONE,  TA_NONE,   TA_TRUE,
  FSW_HIGH_WATER_A, TA_START_BOTH_PUMPS,  TA_TRUE,  TA_NONE,   TA_NONE,
  FSW_HIGH_WATER,   TA_START_BOTH_PUMPS,  TA_TRUE,  TA_NONE,   TA_NONE,
  NO_OF_FSW,        TA_NONE,              TA_NONE,  TA_NONE,   TA_NONE
};

/* Transision table for output fs_pump_ref ACTIVE to NOT_ACTIVE for floatswitch configurations */
static const FSW_CONFIG_TRANSISION_ACTION active_2_not_active[] =
{
  FSW_NOT_DEFINED,  TA_NONE,             TA_NONE,   TA_NONE,  TA_NONE,
  FSW_DRY_RUN,      TA_NONE,             TA_NONE,   TA_TRUE,  TA_NONE,
  FSW_DRY_RUN_A,    TA_NONE,             TA_NONE,   TA_TRUE,  TA_NONE,
  FSW_STOP,         TA_STOP_BOTH_PUMPS,  TA_NONE,   TA_NONE,  TA_NONE,
  FSW_STOP1,        TA_STOP_PUMP_1,      TA_NONE,   TA_NONE,  TA_NONE,
  FSW_STOP2,        TA_STOP_PUMP_2,      TA_NONE,   TA_NONE,  TA_NONE,
  FSW_START1_STOP,  TA_STOP_BOTH_PUMPS,  TA_NONE,   TA_NONE,  TA_NONE,
  FSW_START1,       TA_NONE,             TA_NONE,   TA_NONE,  TA_NONE,
  FSW_START2,       TA_NONE,             TA_NONE,   TA_NONE,  TA_NONE,
  FSW_ALARM,        TA_NONE,             TA_NONE,   TA_NONE,  TA_FALSE,
  FSW_HIGH_WATER_A, TA_NONE,             TA_FALSE,  TA_NONE,  TA_NONE,
  FSW_HIGH_WATER,   TA_NONE,             TA_FALSE,  TA_NONE,  TA_NONE,
  NO_OF_FSW,        TA_NONE,             TA_NONE,   TA_NONE,  TA_NONE
};


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
FloatSwitchCtrl::FloatSwitchCtrl(void)
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_INCONSISTENCY] = new AlarmDelay(this);
  mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL] = new AlarmDelay(this);
  mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING] = new AlarmDelay(this);
  mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL] = new AlarmDelay(this);
  mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR] = new AlarmDelay(this);
  mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS] = new AlarmDelay(this);

  for (int fault_id = FIRST_FSW_FAULT_OBJ; fault_id < NO_OF_FSW_FAULT_OBJ; fault_id++)
  {
    mFloatSwitchAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
FloatSwitchCtrl::~FloatSwitchCtrl(void)
{
  int i;

  for ( i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++ )
  {
    mpFloatSwitch[i]->Unsubscribe(this);
    mpFloatSwitchCnf[i]->Unsubscribe(this);
  }

  for (int fault_id = FIRST_FSW_FAULT_OBJ; fault_id < NO_OF_FSW_FAULT_OBJ; fault_id++)
  {
    delete(mpFloatSwitchAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::InitSubTask(void)
{
  for (int fault_id = FIRST_FSW_FAULT_OBJ; fault_id < NO_OF_FSW_FAULT_OBJ; fault_id++)
  {
    mpFloatSwitchAlarmDelay[fault_id]->InitAlarmDelay();
  }


  mpTimerObjList[AFTER_RUN_DELAY_TIMER] = new SwTimer(mpAfterRunDelay->GetValue(), S, false, false, this);
  mpTimerObjList[AFTER_RUN_HIGH_LEVEL_DELAY_TIMER] = new SwTimer(mpAfterRunHighLevelDelay->GetValue(), S, false, false, this);
  mpTimerObjList[FOAM_DRAIN_INTERVAL_TIMER] = new SwTimer(mpFoamDrainIntervalTime->GetValue(), S, true, true, this);
  mpTimerObjList[FOAM_DRAIN_TIMER] = new SwTimer( mpFoamDrainTime->GetValue(), S, false, false, this);
  mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_ALARM_TIMER] = new SwTimer( 10, S, false, false, this);
  mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_WARN_TIMER] = new SwTimer( 10, S, false, false, this);

  mFoamDrainTimeOut = false;
  mFoamDrainCrossedDryRunAlarm = false;
  mFoamDrainCrossedDryRunWarn = false;
  mFoamDrainIntervalTimeOut = false;
  mFoamDrainBlockDryRunAlarm = false;
  mFoamDrainBlockDryRunWarn = false;
  mFoamDrainState = FD_IDLE;
  mpFoamDrainRequest->SetValue(false);
  mUnderDryRunAlarmLevel = false;

  mPreviousHighWater = false;

  for (int i = 0; i < 9; i++)
  {
    mFswCfg[i] = FSW_NOT_DEFINED;
  }
  // Initiate mFsw1 to a level in the middel of the pit - without alarms - to assure that an alarm situation at
  // startup will be detected as a change.
  mFsw1 = (MRG_FSW_VALUE)0x7;  // This is a level above analog dry run and the stop levels of both pumps - but below the start levels.


  // Initalize level flag to indicate a level not causing alarm
  mPreviousHighLevelAnalogFlag = FSW_BELOW_LEVEL;
  mPreviousHighLevelFloatSwitchFlag = FSW_BELOW_LEVEL;
  mPreviousDryRunAnalogFlag = FSW_ABOVE_LEVEL;
  mPreviousDryRunFloatSwitchFlag = FSW_ABOVE_LEVEL;

  mNoFaultDetectedCnt = 0xFF;           // Initialized to a high value indicating, that no fault has been detected for a long
                                        // time. Max value for the variable has been chosen as initial value, but it could be
                                        // any value higher than values used for comparing (that is higher than 1 at the time
                                        // of writing). There is no danger that the value rolls over because it will never be
                                        // incremented further than one beyond the highest value used for comparing (1 at the time
                                        // of writing).
  mLocalFswPumpRef = GetPumpRef();//mpPumpRef->GetValue();  // mLocalFswPumpRef must be initialized because it is used for comparing the actual
                                             // pump ref to a previous value (contained in mLocalFswPumpRef). Initially mpPumpRef
                                             // is set to NO_PUMPS in the database.
  mFloatSwitchCnfChanged = TRUE;        // TRUE assures that the CtrlType is checked.
  mFloatSwitchChanged = TRUE;           // TRUE assures that the first run is treated as a switch change.
  mPitLevelMeasuredSubscribed = false;  // When this value is set to false during a startup the following scenarios can happen:
                                        // 1) CtrlType is Float Switch - pitLevel is not subscribed which is what this value indicates.
                                        // 2) CtrlType is analog - when RunSubTask detects this it will see that this value is false
                                        //    and thus call for a subscribtion at pitLevel
                                        // In both cases the float switch controller is started up in the right mode and with the
                                        // correct subscribtions.
  mpAlarmLevelConf->SetAlarmConfigType(AC_DIGITAL);   // AlarmDataPoints are initialized to be digital - this matches the
  mpDryRunLevelConf->SetAlarmConfigType(AC_DIGITAL);  // FloatSwitch ctrl type assumed above.
  mpHighLevelConf->SetAlarmConfigType(AC_DIGITAL);

  mReqTaskTimeFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::RunSubTask(void)
{
  mReqTaskTimeFlag = false;

  if (mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
  {
    mpFloatSwitchWaterLevel->SetQuality(DP_NEVER_AVAILABLE);
    mpFloatSwitchWaterLevelPct->SetQuality(DP_NEVER_AVAILABLE);
    return;
  }

  auto GF_UINT32 fsw_idx;
  auto GF_UINT32 fsw_mask;
  auto MRG_FSW_VALUE fsw;
  auto FSW_TYPE act_fsw_cfg;
  auto const FSW_CONFIG_TRANSISION_ACTION *ta_ptr;


  // Do not attempt to control before the surface level has been initiated
  if (mpSurfaceLevelReadyFlag->GetValue() == true)
  {
    mRunSubTaskCausedByConfChange = false;
    mRunSubTaskCausedBySwitchChange = false;

    // Prepare update flags for a new update by transferring their value to a temporary flag to be used
    // further on in this function.
    if (mFloatSwitchCnfChanged == true)
    {
      mFloatSwitchCnfChanged = false;
      mRunSubTaskCausedByConfChange = true;
      mRunSubTaskCausedBySwitchChange = true;  // When the config is changed the switch change flag must also be set true.
                                               // This is because when changing a configuration a switch event can move to
                                               // another input (ie STOP_PUMP_1 can move from switch 2 to 3). This will
                                               // be interpreted by the floatswitch controller as a switch change if one of
                                               // the two switches is up and the other is down. To handle this the
    }
    if (mFloatSwitchChanged == true)
    {
      mFloatSwitchChanged = false;
      mRunSubTaskCausedBySwitchChange = true;
    }


    mHighLevelAnalogFlag = FSW_NO_CHANGE;
    mHighLevelFloatSwitchFlag = FSW_NO_CHANGE;
    mDryRunAnalogFlag = FSW_NO_CHANGE;
    mDryRunFloatSwitchFlag = FSW_NO_CHANGE;

    if ( mpFoamDrainIntervalTime.IsUpdated() )
    {
      mpTimerObjList[FOAM_DRAIN_INTERVAL_TIMER]->SetSwTimerPeriod(mpFoamDrainIntervalTime->GetValue(), S, true);
    }
    if ( mpFoamDrainTime.IsUpdated() )
    {
      mpTimerObjList[FOAM_DRAIN_TIMER]->SetSwTimerPeriod(mpFoamDrainTime->GetValue(), S, false);
    }


    for (int fault_id = FIRST_FSW_FAULT_OBJ; fault_id < NO_OF_FSW_FAULT_OBJ; fault_id++)
    {
      if (mFloatSwitchAlarmDelayCheckFlag[fault_id] == true)
      {
        mFloatSwitchAlarmDelayCheckFlag[fault_id] = false;
        mpFloatSwitchAlarmDelay[fault_id]->CheckErrorTimers();
      }
    }

    // Set a status byte containing the float switch status for GENI
    mpFloatSwitchDiStatus->SetValue(GetDiStatusOfFloatSwitches());

    fsw = MergeFloatSwitches();
    if (mRunSubTaskCausedByConfChange == true)
    {
      if (mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
      {
        if (mPitLevelMeasuredSubscribed == false)  // Prevent subscribing more than once
        {
          mpPitLevelMeasured->Subscribe(this);
          mpAlarmLevelConf->Subscribe(this);
          mpAlarmLevelConf->SetAlarmConfigType(AC_FLOAT);
          mpDryRunLevelConf->Subscribe(this);
          mpDryRunLevelConf->SetAlarmConfigType(AC_FLOAT);
          mpHighLevelConf->Subscribe(this);
          mpHighLevelConf->SetAlarmConfigType(AC_FLOAT);
          mpNoOfPumps->Subscribe(this);
          mPitLevelMeasuredSubscribed = true;
        }

        mNoOfFsw = 7; /* Min NOF sensors when ANALOG sensor is used */
        mFswCfg[0] = FSW_DRY_RUN_A;
        mFswCfg[1] = FSW_STOP1;
        mFswCfg[2] = FSW_STOP2;
        mFswCfg[3] = FSW_START1;
        mFswCfg[4] = FSW_START2;
        mFswCfg[5] = FSW_ALARM;
        mFswCfg[6] =FSW_HIGH_WATER_A;

        for (fsw_idx=0;((fsw_idx < MAX_NO_OF_FLOAT_SWITCHES) && (mpFloatSwitchCnf[fsw_idx]->GetQuality() == DP_AVAILABLE)); fsw_idx++)
        {
          // Empty body - the purpose of the for-loop is to increment fsw_idx until a not available float switch config is met
          // fsw_idx then shows how many float switches there is in the system.
          // The compare with MAX_NO_OF_FLOAT_SWITCHES is to make sure, that the loop does not run forever in case of an error
          // in the configuration - it should never be this condition that stops the loop as only 2 switches are possible in
          // an analog configuration at the time of writing.
        }

        // Add the number of real float switches to the virtual "switches" used for analog sensor - and put a content into
        // mFswCfg 7 and 8, where the two possible real float switches for an analog configuration is located.
        // This piece of code might benefit from refactoring - but it works with the possible configurations and the number
        // of switches allowed at the time of writing and thus don't mess with it unless it is needed.
        mNoOfFsw += fsw_idx;
        if (mpFloatSwitchCnf[0]->GetQuality() == DP_AVAILABLE)
        {
          mFswCfg[7] = mpFloatSwitchCnf[0]->GetValue();
        }
        else
        {
          mFswCfg[7] = FSW_NOT_DEFINED;
        }
        if (mpFloatSwitchCnf[1]->GetQuality() == DP_AVAILABLE)
        {
          mFswCfg[8] = mpFloatSwitchCnf[1]->GetValue();
        }
        else
        {
          mFswCfg[8] = FSW_NOT_DEFINED;
        }
      }
      else  // Float Switch level measuring
      {
        if (mPitLevelMeasuredSubscribed == true)  // Prevent unsubscribing if not subscribed. Searching for unnecessary
                                                  // unsubscribing slows performance.
        {
          mpPitLevelMeasured->Unsubscribe(this);
          mpAlarmLevelConf->Unsubscribe(this);
          mpAlarmLevelConf->SetAlarmConfigType(AC_DIGITAL);
          mpDryRunLevelConf->Unsubscribe(this);
          mpDryRunLevelConf->SetAlarmConfigType(AC_DIGITAL);
          mpHighLevelConf->Unsubscribe(this);
          mpHighLevelConf->SetAlarmConfigType(AC_DIGITAL);
          mpNoOfPumps->Unsubscribe(this);
          mPitLevelMeasuredSubscribed = false;
        }

        for (fsw_idx=0;((fsw_idx < MAX_NO_OF_FLOAT_SWITCHES) && (mpFloatSwitchCnf[fsw_idx]->GetQuality() == DP_AVAILABLE)); fsw_idx++)
        {
          mFswCfg[fsw_idx] = mpFloatSwitchCnf[fsw_idx]->GetValue();
        }
        mNoOfFsw = fsw_idx;
      }
      mpFloatSwitchWaterLevel->SetMaxValue(mNoOfFsw);
      mDryRunAnalogFlag = FSW_ABOVE_LEVEL;
      mDryRunFloatSwitchFlag = FSW_ABOVE_LEVEL;
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING]->ResetFault();
      mHighLevelAnalogFlag = FSW_BELOW_LEVEL;
      mHighLevelFloatSwitchFlag = FSW_BELOW_LEVEL;
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL]->ResetFault();
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL]->ResetFault();
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING]->ResetWarning();
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL]->ResetWarning();
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL]->ResetWarning();
    }

    // Update float switch output - but if we are controlled by switches then only update when swithces have
    // changed.
    if (mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES || mRunSubTaskCausedBySwitchChange == true)  // Analog level measuring
    {
      CheckForFSWInconsistency(fsw);
      fsw = Update2ValidFSW(fsw);
      if (mRunSubTaskCausedByConfChange == true)
      {
        // Force mFsw1 to a different value than the current fsw - this assures that
        // a configuration change will be detected as a change in the float switch situation
        // and thus the state of outputs and the alarm situation will be evaluated
        mFsw1 = (MRG_FSW_VALUE)~(GF_UINT32)fsw;
      }

      UpdateWaterLevelOutputs(fsw);
      UpdateUnderLowestStopLevel(fsw);
      UpdateUnderDryRunAlarmLevel(fsw);

      for (fsw_idx=0; (fsw_idx < mNoOfFsw); fsw_idx++)
      {
        fsw_mask = 1 << fsw_idx;
        if ((fsw & fsw_mask) != (mFsw1 & fsw_mask))
        {
          act_fsw_cfg = mFswCfg[fsw_idx];
          if (fsw & fsw_mask)
          {
            /* not_active_2_active action */
            ta_ptr = not_active_2_active;
          }
          else
          {
            /* active_2_not_active action */
            ta_ptr = active_2_not_active;
          }
          while ((act_fsw_cfg != ta_ptr->fsw_config) && (ta_ptr->fsw_config != NO_OF_FSW))
          {
            ta_ptr++;
          }
          UpdateFSWOutputs(ta_ptr);
        }
      }
      mFsw1 = fsw;
    }

    /* Handle FoamDrain */
    PUMP_REF_TYPE foam_drain_ref;
    bool c_break = false;

    if (mpFoamDrainEnabled->GetValue() &&
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR]->GetFault() == false &&
        mpFloatSwitchFaultObj[FSW_FAULT_OBJ_SENSOR]->GetAlarmPresent() == ALARM_STATE_READY &&  // A sensor fault is valid as long as an unacknowledged alarm is present
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS]->GetFault() == false)
    {
      if (mpFoamDrainDigInRequest.IsUpdated() == true && mpFoamDrainDigInRequest->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
      {
        // Digital input trigger should give same result as foam drain time out
        mFoamDrainIntervalTimeOut = true;
      }
      switch ( mFoamDrainState )
      {
        case FD_IDLE :
          if( mFoamDrainIntervalTimeOut )
          {
            mFoamDrainIntervalTimeOut = false;
            mFoamDrainState = FD_WAIT_FOR_PUMP_START;
          }
          else
            break;
        case FD_WAIT_FOR_PUMP_START :
          if( mLocalFswPumpRef != PUMP_REF_NO_PUMPS )
          {
            mFoamDrainState = FD_WAIT_FOR_FOAM_DRAIN_LEVEL;
            mFoamDrainBlockDryRunAlarm = true;
            mFoamDrainBlockDryRunWarn = true;
          }
          else
            break;
        case FD_WAIT_FOR_FOAM_DRAIN_LEVEL :
          foam_drain_ref = mLocalFswPumpRef;
          if( foam_drain_ref == PUMP_REF_NO_PUMPS )
            foam_drain_ref = PUMP_REF_PUMP_ONE;
          if(mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
          {
            if( mpPitLevelMeasured->GetValue() < mpFoamDrainLevel->GetValue() )
            {
              mFoamDrainState = FD_PRE_WAIT_FOR_FOAM_DRAIN_TIMEOUT;
            }
            else
            {
              c_break = true;
            }
          }
          else                          // Float Switch level measuring
          {
            if( mpUnderLowestStopLevel->GetValue() )
            {
              mFoamDrainState = FD_PRE_WAIT_FOR_FOAM_DRAIN_TIMEOUT;
            }
            else
            {
              c_break = true;
            }
          }
          if( c_break)
            break;
        case FD_PRE_WAIT_FOR_FOAM_DRAIN_TIMEOUT :
          mFoamDrainState = FD_WAIT_FOR_FOAM_DRAIN_TIMEOUT;
          if( mpFoamDrainTime->GetValue() > 0 )
          {
            mFoamDrainTimeOut = false;
            mpTimerObjList[FOAM_DRAIN_TIMER]->RetriggerSwTimer();
          }
          else
          {
            mFoamDrainTimeOut = true;
          }
          /*No break */
        case FD_WAIT_FOR_FOAM_DRAIN_TIMEOUT :
          foam_drain_ref = mLocalFswPumpRef;
          if( foam_drain_ref == PUMP_REF_NO_PUMPS )
            foam_drain_ref = PUMP_REF_PUMP_ONE;
          if( mFoamDrainTimeOut )
          {
            mFoamDrainState = FD_IDLE;
            if (mpFoamDrainDigInRequest->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
            {
              // Keep triggering foam draining when input is activated
              mFoamDrainIntervalTimeOut = true;
            }
          }
          else
          {
            break;
          }
        default:
          break;
      }

      // Check if any blocking of the dry run alarms and warnings should be suspended.
      if (mFoamDrainBlockDryRunAlarm == true)
      {
        if (mFoamDrainState == FD_IDLE || mFoamDrainState == FD_WAIT_FOR_PUMP_START)
        {
          if (mFoamDrainCrossedDryRunAlarm == true)
          {
            // We are above dry run alarm and has been there for some time - lift blocking
            mFoamDrainCrossedDryRunAlarm = false;
            mFoamDrainBlockDryRunAlarm = false;
          }
          else
          {
            // Foam Draining is not active - check if level is above dry run alarm so that blocking can be lifted
            if (mUnderDryRunAlarmLevel == false)
            {
              if (mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_ALARM_TIMER]->GetSwTimerStatus() == false)
              {
                mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_ALARM_TIMER]->RetriggerSwTimer();
              }
            }
            else
            {
              mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_ALARM_TIMER]->StopSwTimer();
            }
          }
        }
        else
        {
          mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_ALARM_TIMER]->StopSwTimer();
          mFoamDrainCrossedDryRunAlarm = false;
        }
      }

      if (mFoamDrainBlockDryRunWarn == true)
      {
        if (mFoamDrainState == FD_IDLE || mFoamDrainState == FD_WAIT_FOR_PUMP_START)
        {
          if (mFoamDrainCrossedDryRunWarn == true)
          {
            // We are above dry run warning and has been there for some time - lift blocking
            mFoamDrainCrossedDryRunWarn = false;
            mFoamDrainBlockDryRunWarn = false;
          }
          else
          {
            // Foam Draining is not active - check if level is above dry run alarm so that blocking can be lifted
            if(mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
            {
              if(mpPitLevelMeasured->GetValue() > mpDryRunLevelConf->GetWarningLimit()->GetAsFloat())
              {
                if (mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_WARN_TIMER]->GetSwTimerStatus() == false)
                {
                  mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_WARN_TIMER]->RetriggerSwTimer();
                }
              }
              else
              {
                mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_WARN_TIMER]->StopSwTimer();
              }
            }
            else  // Float Switch level measuring
            {
              // When float switching, there is no separte warning limit - both alarm and warning are
              // handled with the mFoamDrainBlockDryRunAlarm
              mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_WARN_TIMER]->StopSwTimer();
              mFoamDrainCrossedDryRunWarn = false;
              mFoamDrainBlockDryRunWarn = false;
            }
          }
        }
        else
        {
          mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_WARN_TIMER]->StopSwTimer();
          mFoamDrainCrossedDryRunWarn = false;
        }
      }

    }
    else
    {
      // Foam drain disabled or an unacknowledged sensor error - do not perform a foam drain and/or abort an ongoing foam drain
      mpTimerObjList[FOAM_DRAIN_TIMER]->StopSwTimer();
      mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_ALARM_TIMER]->StopSwTimer();
      mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_WARN_TIMER]->StopSwTimer();
      mFoamDrainTimeOut = false;
      mFoamDrainCrossedDryRunAlarm = false;
      mFoamDrainBlockDryRunAlarm = false;
      mFoamDrainCrossedDryRunWarn = false;
      mFoamDrainBlockDryRunWarn = false;
      mFoamDrainState = FD_IDLE;
      if (mpFoamDrainEnabled->GetValue() == false)
      {
        // Force a foam drain first time after it has been enabled
        mFoamDrainIntervalTimeOut = true;
      }
    }

    /* Handle FoamDrainRequest used for appmode */
    if( mFoamDrainState==FD_WAIT_FOR_FOAM_DRAIN_LEVEL || mFoamDrainState==FD_WAIT_FOR_FOAM_DRAIN_TIMEOUT )
    {
      mpFoamDrainRequest->SetValue(true);
    }
    else
    {
      mpFoamDrainRequest->SetValue(false);
    }

    if( mFoamDrainState==FD_WAIT_FOR_FOAM_DRAIN_LEVEL || mFoamDrainState==FD_WAIT_FOR_FOAM_DRAIN_TIMEOUT )
    {
      SetPumpRef(foam_drain_ref);//mpPumpRef->SetValue(foam_drain_ref);
    }
    else if (mpTimerObjList[AFTER_RUN_DELAY_TIMER]->GetSwTimerStatus() == false)
    {
      SetPumpRef(mLocalFswPumpRef);//mpPumpRef->SetValue(mLocalFswPumpRef);
    }

    // Before updating alarmdatapoint the two flags that decide the High Level alarm should be merged.
    if (mHighLevelAnalogFlag == FSW_NO_CHANGE)
    {
      mHighLevelAnalogFlag = mPreviousHighLevelAnalogFlag;
    }
    if (mHighLevelFloatSwitchFlag == FSW_NO_CHANGE)
    {
      mHighLevelFloatSwitchFlag = mPreviousHighLevelFloatSwitchFlag;
    }
    if (mHighLevelAnalogFlag == FSW_BELOW_LEVEL && mHighLevelFloatSwitchFlag == FSW_BELOW_LEVEL)
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL]->ResetFault();
      mpHighLevelStateDetected->SetValue(false);
    }
    else if (mHighLevelAnalogFlag == FSW_ABOVE_LEVEL || mHighLevelFloatSwitchFlag == FSW_ABOVE_LEVEL)
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL]->SetFault();
      mpHighLevelStateDetected->SetValue(true);
    }
    mPreviousHighLevelAnalogFlag = mHighLevelAnalogFlag;
    mPreviousHighLevelFloatSwitchFlag = mHighLevelFloatSwitchFlag;

    // Before updating alarmdatapoint the two flags that decide the Dry Run alarm should be merged.
    if (mDryRunAnalogFlag == FSW_NO_CHANGE)
    {
      mDryRunAnalogFlag = mPreviousDryRunAnalogFlag;
    }
    if (mDryRunFloatSwitchFlag == FSW_NO_CHANGE)
    {
      mDryRunFloatSwitchFlag = mPreviousDryRunFloatSwitchFlag;
    }
    if (mDryRunAnalogFlag == FSW_ABOVE_LEVEL && mDryRunFloatSwitchFlag == FSW_ABOVE_LEVEL)
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING]->ResetFault();
    }
    else if (mDryRunAnalogFlag == FSW_BELOW_LEVEL || mDryRunFloatSwitchFlag == FSW_BELOW_LEVEL)
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING]->SetFault();
    }
    mPreviousDryRunAnalogFlag = mDryRunAnalogFlag;
    mPreviousDryRunFloatSwitchFlag = mDryRunFloatSwitchFlag;


    for (int fault_id = FIRST_FSW_FAULT_OBJ; fault_id < NO_OF_FSW_FAULT_OBJ; fault_id++)
    {
      mpFloatSwitchAlarmDelay[fault_id]->UpdateAlarmDataPoint();
    }
  }
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void FloatSwitchCtrl::Update(Subject* pSubject)
{
  // Items that are updated often are placed first. Then they are found quickly
  // which reduces the amount of unnecessary compares.

  // Pit level - often updated, if system is analog controlled
  if(mpPitLevelMeasured.Update(pSubject))
  {
    // Nop
  }

  // Float switch action - pretty often, if system is float switch controlled, rarer when analog
  else if(mpFloatSwitch[0].Update(pSubject) || mpFloatSwitch[1].Update(pSubject) ||
          mpFloatSwitch[2].Update(pSubject) || mpFloatSwitch[3].Update(pSubject) ||
          mpFloatSwitch[4].Update(pSubject) || mpFloatSwitchInputMoved.Update(pSubject))
  {
    mFloatSwitchChanged = true;
  }


  // After run - happens regularly
  else if (pSubject == mpTimerObjList[AFTER_RUN_DELAY_TIMER])
  {
    // Nop
  }
  else if (pSubject == mpTimerObjList[AFTER_RUN_HIGH_LEVEL_DELAY_TIMER])
  {
    // Nop
  }


  // Foam draining - happens now and then
  else if(mpFoamDrainIntervalTime.Update(pSubject))
  {
    // Nop
  }
  else if(mpFoamDrainTime.Update(pSubject))
  {
    // Nop
  }
  else if(mpFoamDrainDigInRequest.Update(pSubject))
  {
    // Nop
  }
  else if (pSubject == mpTimerObjList[FOAM_DRAIN_INTERVAL_TIMER])
  {
    mFoamDrainIntervalTimeOut = true;
  }
  else if (pSubject == mpTimerObjList[FOAM_DRAIN_TIMER])
  {
    mFoamDrainTimeOut = true;
  }
  else if (pSubject == mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_ALARM_TIMER])
  {
    mFoamDrainCrossedDryRunAlarm = true;
  }
  else if (pSubject == mpTimerObjList[FOAM_DRAIN_CROSSED_DRY_RUN_WARN_TIMER])
  {
    mFoamDrainCrossedDryRunWarn = true;
  }


  // Configuration changes - rare
  else if(mpFloatSwitchCnf[0].Update(pSubject) || mpFloatSwitchCnf[1].Update(pSubject) ||
          mpFloatSwitchCnf[2].Update(pSubject) || mpFloatSwitchCnf[3].Update(pSubject) ||
          mpFloatSwitchCnf[4].Update(pSubject) || mpPitLevelCtrlType.Update(pSubject))
  {
    mFloatSwitchCnfChanged = true;
  }
  else if(mpAlarmLevelConf.Update(pSubject))
  {
    // Nop
  }
  else if(mpDryRunLevelConf.Update(pSubject))
  {
    // Nop
  }
  else if(mpHighLevelConf.Update(pSubject))
  {
    // Nop
  }
  else if (mpNoOfPumps.Update(pSubject)) {
    // Nop
  }


  // Alarms - extremely rare occurence, hopefully
  else if (pSubject == mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_INCONSISTENCY])
  {
    mFloatSwitchAlarmDelayCheckFlag[FSW_FAULT_OBJ_INCONSISTENCY] = true;
  }
  else if (pSubject == mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL])
  {
    mFloatSwitchAlarmDelayCheckFlag[FSW_FAULT_OBJ_HIGH_LEVEL] = true;
  }
  else if (pSubject == mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING])
  {
    mFloatSwitchAlarmDelayCheckFlag[FSW_FAULT_OBJ_DRY_RUNNING] = true;
  }
  else if (pSubject == mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL])
  {
    mFloatSwitchAlarmDelayCheckFlag[FSW_FAULT_OBJ_ALARM_LEVEL] = true;
  }
  else if (pSubject == mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR])
  {
    mFloatSwitchAlarmDelayCheckFlag[FSW_FAULT_OBJ_SENSOR] = true;
  }
  else if (pSubject == mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS])
  {
    mFloatSwitchAlarmDelayCheckFlag[FSW_FAULT_OBJ_CONFLICT_LEVELS] = true;
  }


  // Surface level ready flag - only once during startup
  else if(mpSurfaceLevelReadyFlag.Update(pSubject))
  {
    // Nop
  }

  else
  {
    // Update from unknown source - this must not happen
    FatalErrorOccured("FloatSwitchController updated by unknown subject!");
  }

  if (mReqTaskTimeFlag == false)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for ( int i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++ )
  {
    mpFloatSwitch[i].Detach(pSubject);
    mpFloatSwitchCnf[i].Detach(pSubject);
  }
  mpPitLevelCtrlType.Detach(pSubject);

  if (mPitLevelMeasuredSubscribed == true)  // Only detach those if they are currently subscribed
  {
    mpAlarmLevelConf.Detach(pSubject);
    mpDryRunLevelConf.Detach(pSubject);
    mpHighLevelConf.Detach(pSubject);
    mpPitLevelMeasured.Detach(pSubject);
    mpNoOfPumps.Detach(pSubject);
  }

  for (int fault_id = FIRST_FSW_FAULT_OBJ; fault_id < NO_OF_FSW_FAULT_OBJ; fault_id++)
  {
    mpFloatSwitchAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::ConnectToSubjects(void)
{
  for ( int i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++ )
  {
    mpFloatSwitch[i]->Subscribe(this);
    mpFloatSwitchCnf[i]->Subscribe(this);
  }
  mpPitLevelCtrlType->Subscribe(this);
  mpFoamDrainIntervalTime->Subscribe(this);
  mpFoamDrainTime->Subscribe(this);
  mpFoamDrainDigInRequest->Subscribe(this);
  mpFloatSwitchInputMoved->Subscribe(this);

  mpSurfaceLevelReadyFlag->Subscribe(this);

  for (int fault_id = FIRST_FSW_FAULT_OBJ; fault_id < NO_OF_FSW_FAULT_OBJ; fault_id++)
  {
    mpFloatSwitchAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_FSC_FLOAT_SWITCH_1 :
      mpFloatSwitch[0].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_2 :
      mpFloatSwitch[1].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_3 :
      mpFloatSwitch[2].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_4 :
      mpFloatSwitch[3].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_5 :
      mpFloatSwitch[4].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_CNF_1 :
      mpFloatSwitchCnf[0].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_CNF_2 :
      mpFloatSwitchCnf[1].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_CNF_3 :
      mpFloatSwitchCnf[2].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_CNF_4 :
      mpFloatSwitchCnf[3].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_CNF_5 :
      mpFloatSwitchCnf[4].Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_INCONSISTENCY_FAULT_OBJ :
      mpFloatSwitchFaultObj[FSW_FAULT_OBJ_INCONSISTENCY].Attach(pSubject);
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_INCONSISTENCY]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_SENSOR_FAULT_OBJ :
      mpFloatSwitchFaultObj[FSW_FAULT_OBJ_SENSOR].Attach(pSubject);
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_CONFLICT_LEVELS_FAULT_OBJ :
      mpFloatSwitchFaultObj[FSW_FAULT_OBJ_CONFLICT_LEVELS].Attach(pSubject);
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_HIGH_LEVEL_FAULT_OBJ :
      mpFloatSwitchFaultObj[FSW_FAULT_OBJ_HIGH_LEVEL].Attach(pSubject);
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_ALARM_LEVEL_OBJ :
      mpFloatSwitchFaultObj[FSW_FAULT_OBJ_ALARM_LEVEL].Attach(pSubject);
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_DRY_RUN_FAULT_OBJ :
      mpFloatSwitchFaultObj[FSW_FAULT_OBJ_DRY_RUNNING].Attach(pSubject);
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_WATER_LEVEL_PCT :
      mpFloatSwitchWaterLevelPct.Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_WATER_LEVEL :
      mpFloatSwitchWaterLevel.Attach(pSubject);
      break;
    case SP_FSC_UNDER_LOWEST_STOP_LEVEL :
      mpUnderLowestStopLevel.Attach(pSubject);
      break;
    case SP_FSC_HIGH_LEVEL_STATE_DETECTED :
      mpHighLevelStateDetected.Attach(pSubject);
      break;
    case SP_FSC_NO_OF_PUMPS :
      mpNoOfPumps.Attach(pSubject);
      break;
    case SP_FSC_PUMP_REF_LEVEL_1:
      mpPumpRefLevel[PUMP_1].Attach(pSubject);
      break;
    case SP_FSC_PUMP_REF_LEVEL_2:
      mpPumpRefLevel[PUMP_2].Attach(pSubject);
      break;
    case SP_FSC_PUMP_REF_LEVEL_3:
      mpPumpRefLevel[PUMP_3].Attach(pSubject);
      break;
    case SP_FSC_PUMP_REF_LEVEL_4:
      mpPumpRefLevel[PUMP_4].Attach(pSubject);
      break;
    case SP_FSC_PUMP_REF_LEVEL_5:
      mpPumpRefLevel[PUMP_5].Attach(pSubject);
      break;
    case SP_FSC_PUMP_REF_LEVEL_6:
      mpPumpRefLevel[PUMP_6].Attach(pSubject);
      break;
    case SP_FSC_START_LEVEL_PUMP_1 :
      mpStartLevelPump[PUMP_1].Attach(pSubject);
      break;
    case SP_FSC_START_LEVEL_PUMP_2 :
      mpStartLevelPump[PUMP_2].Attach(pSubject);
      break;
    case SP_FSC_STOP_LEVEL_PUMP_1 :
      mpStopLevelPump[PUMP_1].Attach(pSubject);
      break;
    case SP_FSC_STOP_LEVEL_PUMP_2 :
      mpStopLevelPump[PUMP_2].Attach(pSubject);
      break;
    case SP_FSC_ALARM_LEVEL :
      mpAlarmLevelConf.Attach(pSubject);
      break;
    case SP_FSC_DRY_RUN_LEVEL:
      mpDryRunLevelConf.Attach(pSubject);
      break;
    case SP_FSC_HIGH_LEVEL:
      mpHighLevelConf.Attach(pSubject);
      break;
    case SP_FSC_PIT_LEVEL_MEASURED:
      mpPitLevelMeasured.Attach(pSubject);
      break;
    case SP_FSC_PIT_LEVEL_CTRL_TYPE:
      mpPitLevelCtrlType.Attach(pSubject);
      break;
    case SP_FSC_AFTER_RUN_DELAY:
      mpAfterRunDelay.Attach(pSubject);
      break;
    case SP_FSC_AFTER_RUN_HIGH_LEVEL_DELAY:
      mpAfterRunHighLevelDelay.Attach(pSubject);
      break;
    case SP_FSC_FOAM_DRAIN_LEVEL:
      mpFoamDrainLevel.Attach(pSubject);
      break;
    case SP_FSC_FOAM_DRAIN_TIME:
      mpFoamDrainTime.Attach(pSubject);
      break;
    case SP_FSC_FOAM_DRAIN_INTERVAL_TIME:
      mpFoamDrainIntervalTime.Attach(pSubject);
      break;
    case SP_FSC_FOAM_DRAIN_ENABLED:
      mpFoamDrainEnabled.Attach(pSubject);
      break;
    case SP_FSC_FOAM_DRAIN_DIG_IN_REQUEST:
      mpFoamDrainDigInRequest.Attach(pSubject);
      break;
    case SP_FSC_FOAM_DRAIN_REQUEST:
      mpFoamDrainRequest.Attach(pSubject);
      break;
    case SP_FSC_HIGH_LEVEL_SWITCH_ACTIVATED:
      mpHighLevelSwitchActivated.Attach(pSubject);
      break;
    case SP_FSC_FLOAT_SWITCH_DI_STATUS:
      mpFloatSwitchDiStatus.Attach(pSubject);
      break;

    case SP_FSC_FSW_INPUT_MOVED:
      mpFloatSwitchInputMoved.Attach(pSubject);
      break;

    case SP_FSC_SURFACE_LEVEL_READY:
      mpSurfaceLevelReadyFlag.Attach(pSubject);
      break;
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
 * Function - MergeFloatSwitches
 * DESCRIPTION:
 *
 *****************************************************************************/
MRG_FSW_VALUE FloatSwitchCtrl::MergeFloatSwitches(void)
{
  auto GF_UINT32 fsw = 0;
  auto double pitlevel;

  if (mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
  {
    if (mpPitLevelMeasured->GetQuality() == DP_AVAILABLE &&
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR]->GetFault() == false &&
        mpFloatSwitchFaultObj[FSW_FAULT_OBJ_SENSOR]->GetAlarmPresent() == ALARM_STATE_READY && // A sensor fault is valid as long as an unacknowledged alarm is present
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS]->GetFault() == false)
    {

      //TODO2>6 handle pump 3 to 6

      // We have a valid pitlevel - and any previous sensor alarms has been acknowledged
      // It is now legal to convert the pitlevel to float switch levels and compared against limits
      pitlevel = mpPitLevelMeasured->GetValue();
      fsw = (pitlevel >= mpDryRunLevelConf->GetAlarmLimit()->GetAsFloat());
      fsw |= ((pitlevel >= mpStopLevelPump[PUMP_1]->GetValue()) << 1);
      if (mpNoOfPumps->GetValue() == 2)
      {
        fsw |= ((pitlevel >= mpStopLevelPump[PUMP_2]->GetValue()) << 2);
      }
      else
      {
        // If the pump is not a 2 pump system stop level for pump 1 is used again for float switch level stop 2
        // This avoids that the stop 2 level disturbs a 1 pump system.
        fsw |= ((pitlevel >= mpStopLevelPump[PUMP_1]->GetValue()) << 2);
      }
      fsw |= ((pitlevel >= mpStartLevelPump[PUMP_1]->GetValue()) << 3);
      if (mpNoOfPumps->GetValue() == 2)
      {
        fsw |= ((pitlevel >= mpStartLevelPump[PUMP_2]->GetValue()) << 4);
      }
      else
      {
        // If the pump is not a 2 pump system start level for pump 1 is used again for float switch level start 2
        // This avoids that the start 2 level disturbs a 1 pump system.
        fsw |= ((pitlevel >= mpStartLevelPump[PUMP_1]->GetValue()) << 4);
      }
      fsw |= ((pitlevel >= mpAlarmLevelConf->GetAlarmLimit()->GetAsFloat()) << 5);
      fsw |= ((pitlevel >= mpHighLevelConf->GetAlarmLimit()->GetAsFloat()) << 6);
      fsw |= ((mpFloatSwitch[0]->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 7);
      fsw |= ((mpFloatSwitch[1]->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 8);

      // Check for warnings
      if (pitlevel <= mpDryRunLevelConf->GetWarningLimit()->GetAsFloat())
      {
        // Do not report dry running while foam draining
        if(mFoamDrainBlockDryRunWarn == false)
        {
          mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING]->SetWarning();
        }
      }
      else
      {
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING]->ResetWarning();
      }
      if (pitlevel >= mpAlarmLevelConf->GetWarningLimit()->GetAsFloat())
      {
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL]->SetWarning();
      }
      else
      {
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL]->ResetWarning();
      }
      if (pitlevel >= mpHighLevelConf->GetWarningLimit()->GetAsFloat())
      {
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL]->SetWarning();
      }
      else
      {
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL]->ResetWarning();
      }
    }
    else
    {
      // We have an invalid pitlevel - warnings based upon this value must be reset.
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_DRY_RUNNING]->ResetWarning();
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL]->ResetWarning();
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_HIGH_LEVEL]->ResetWarning();
      // Pass an fsw that contains a pitlevel in the middle of the pit combined with
      // the state of the - possible - float switches.
      fsw = 0x7; // This is a level above analog dry run and the stop levels of both pumps - but below the start levels.
      fsw |= ((mpFloatSwitch[0]->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 7);
      fsw |= ((mpFloatSwitch[1]->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 8);
      // This default level will enable the following checks for inconsistency and for dryrun and highlevel alarms to
      // clearly distinguish the float switch values from the - invalid - pitlevel.
      // Setting of sensor error will be done during CheckForFSWInconsistency
    }
  }
  else  // Float Switch level measuring
  {
    fsw =  (mpFloatSwitch[0]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE);
    fsw |= ((mpFloatSwitch[1]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 1);
    fsw |= ((mpFloatSwitch[2]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 2);
    fsw |= ((mpFloatSwitch[3]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 3);
    fsw |= ((mpFloatSwitch[4]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 4);
  }
  return (MRG_FSW_VALUE)fsw;
}

/*****************************************************************************
 * Function - GetDiStatusOfFloatSwitches
 * DESCRIPTION: Set bit 0-4 according to float switch 1-5 activated or not
 *
 *****************************************************************************/
GF_UINT8 FloatSwitchCtrl::GetDiStatusOfFloatSwitches(void)
{
  auto GF_UINT8  fsw = mpFloatSwitchDiStatus->GetValue();

  bool temp = (mpFloatSwitch[0]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE);

  if(temp == 1)
	fsw |= ((mpFloatSwitch[0]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 0);
  else
	fsw &= ~((1) << 0);

  bool temp1 = (mpFloatSwitch[1]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE);

  if(temp1 == 1)
	fsw |= ((mpFloatSwitch[1]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 1);
  else
	fsw &= ~((1) << 1);

  bool temp2 = (mpFloatSwitch[2]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE);

  if(temp2 == 1)
	fsw |= ((mpFloatSwitch[2]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 2);
  else
	fsw &= ~((1) << 2);


  bool temp3 = (mpFloatSwitch[3]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE);

  if(temp3 == 1)
	fsw |= ((mpFloatSwitch[3]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 3);
  else
	fsw &= ~((1) << 3);

  bool temp4 = (mpFloatSwitch[4]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE);

  if(temp4 == 1)
	fsw |= ((mpFloatSwitch[4]->GetValue()== DIGITAL_INPUT_FUNC_STATE_ACTIVE) << 4);
  else
	fsw &= ~((1) << 4);

  return fsw;
}


/*****************************************************************************
 * Function - Update2ValidFSW
 * DESCRIPTION:
 *
 *****************************************************************************/
MRG_FSW_VALUE FloatSwitchCtrl::Update2ValidFSW(MRG_FSW_VALUE fsw)
{
  static MRG_FSW_VALUE fsw_old;
  auto GF_UINT32 fsw_idx, fsw_mask, fsw_updated_2_valid;
  auto GF_UINT32 top_most_switch_active_mask;
  auto GF_UINT32 chg_2_active = 0;
  auto GF_UINT32 chg_2_not_active = 0;
  auto GF_UINT32 min_bit_chg_2_not_active = 0xFF;
  auto GF_UINT32 max_bit_active = 0;
  auto GF_UINT32 min_bit_not_active = 0xFF;
  auto bool high_water = false;
  auto bool dry_run = false;

  /* Find top most switch in configuration */
  fsw_updated_2_valid = 0;
  for (fsw_idx=0; (fsw_idx < mNoOfFsw); fsw_idx++)
  {
    fsw_updated_2_valid <<= 1;
    fsw_updated_2_valid |= 1;
  }
  if (mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
  {
    /* Figure out if highwater and/or dryrun is present */
    if ((mFswCfg[7] == FSW_HIGH_WATER) && (fsw & HIGH_WATER_MASK_7) && ((fsw & HIGH_WATER_A_MASK) == FALSE))
    {
      high_water = true;
      mpTimerObjList[AFTER_RUN_HIGH_LEVEL_DELAY_TIMER]->StopSwTimer();  // Stop a security delay because switch controls
    }
    if ((mFswCfg[8] == FSW_HIGH_WATER) && (fsw & HIGH_WATER_MASK_8) && ((fsw & HIGH_WATER_A_MASK) == FALSE))
    {
      high_water = true;
      mpTimerObjList[AFTER_RUN_HIGH_LEVEL_DELAY_TIMER]->StopSwTimer();  // Stop a security delay because switch controls
    }
    if ((mFswCfg[7] == FSW_DRY_RUN) && ((fsw & DRY_RUN_MASK) == FALSE) && (fsw & DRY_RUN_A_MASK))
    {
      dry_run = true;
      mpTimerObjList[AFTER_RUN_HIGH_LEVEL_DELAY_TIMER]->StopSwTimer();  // Stop a security delay because switch controls
    }

    /* Find top most switch in configuration */
    if ((mFswCfg[7] == FSW_HIGH_WATER) && (fsw & HIGH_WATER_MASK_7) && ((fsw & HIGH_WATER_A_MASK) == FALSE))
    {
      fsw_old = fsw;
      fsw = (MRG_FSW_VALUE)fsw_updated_2_valid;
      mFsw1 = (MRG_FSW_VALUE)~fsw_updated_2_valid;
    }
    if ((mFswCfg[8] == FSW_HIGH_WATER) && (fsw & HIGH_WATER_MASK_8) && ((fsw & HIGH_WATER_A_MASK) == FALSE))
    {
      fsw_old = fsw;
      fsw = (MRG_FSW_VALUE)fsw_updated_2_valid;
      mFsw1 = (MRG_FSW_VALUE)~fsw_updated_2_valid;
    }
    if ((mFswCfg[7] == FSW_DRY_RUN) && ((fsw & DRY_RUN_MASK) == FALSE) && (fsw & DRY_RUN_A_MASK))
    {
      fsw_old = fsw;
      fsw = (MRG_FSW_VALUE)0;
      mFsw1 = (MRG_FSW_VALUE)~fsw;
    }

    if (mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR]->GetFault() == true || mpFloatSwitchFaultObj[FSW_FAULT_OBJ_SENSOR]->GetAlarmPresent() != ALARM_STATE_READY)
    {
      // There is a sensor error (or a previous has not been acknowledged). The sensor level must not be used - fsw
      // is set to the previous value if the water level is not defined by the float switches.
      if (high_water == false && dry_run == false)
      {
        if (mPreviousHighWater == true)
        {
          // High water switch has just gone down - start afterrun high level delay
          mpTimerObjList[AFTER_RUN_HIGH_LEVEL_DELAY_TIMER]->SetSwTimerPeriod(mpAfterRunHighLevelDelay->GetValue(), S, false);
          mpTimerObjList[AFTER_RUN_HIGH_LEVEL_DELAY_TIMER]->RetriggerSwTimer();
        }
        if (mpTimerObjList[AFTER_RUN_HIGH_LEVEL_DELAY_TIMER]->GetSwTimerStatus() == true)
        {
          // Security delay is on - pumps must not change state.
          fsw = mFsw1;
        }
        else
        {
          // Security delay is off - pumps should be stopped
          fsw = SW1_BIT_NO;                      // Level is set just over dry run.
          if (mFswCfg[7] == FSW_DRY_RUN)
          {
            fsw = (MRG_FSW_VALUE)(SW8_BIT_NO | fsw);          // Remember the dry run switch - if it is configured
          }
        }
      }
    }
    else
    {
      // Sensor is operating correct - security delay must be stopped.
      mpTimerObjList[AFTER_RUN_HIGH_LEVEL_DELAY_TIMER]->StopSwTimer();
    }

    mPreviousHighWater = high_water;
  }
  else  // Float Switch level measuring
  {
    top_most_switch_active_mask = 1 << (fsw_idx-1);
    if (fsw & top_most_switch_active_mask)
    {
      fsw_old = fsw;
      fsw = (MRG_FSW_VALUE)fsw_updated_2_valid;
      mFsw1 = (MRG_FSW_VALUE)~fsw_updated_2_valid;
    }
    else if ((mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_INCONSISTENCY]->GetFault() == true) && (mNoFaultDetectedCnt == 1))
    {
      for (fsw_idx=0; (fsw_idx < mNoOfFsw); fsw_idx++)
      {
        fsw_mask = 1 << fsw_idx;

        // Locate the highest active bit and the lowest not active bit
        if (fsw & fsw_mask) /* Switch active */
        {
          max_bit_active = fsw_idx + 1;
        }
        else
        {
          if (fsw_idx < min_bit_not_active)
          {
            min_bit_not_active = fsw_idx;
          }
        }

        // Locate the higest change to active bit and the lowest change to not active bit
        if ((fsw & fsw_mask) != (fsw_old & fsw_mask))
        {
          if (fsw & fsw_mask) /* Change to active */
          {
            chg_2_active++;
          }
          else
          {
            chg_2_not_active++;
            if (fsw_idx < min_bit_chg_2_not_active)
            {
              min_bit_chg_2_not_active = fsw_idx;
            }
          }
        }
      }
      /* Need for desision for selecting the right switch kombi */
      if (chg_2_active >= chg_2_not_active)
      {
        fsw_updated_2_valid = 0;
        while (max_bit_active)
        {
          max_bit_active--;
          fsw_updated_2_valid |= (1 << max_bit_active);
        }
      }
      else
      {
        fsw_updated_2_valid = ~0;
        while (min_bit_not_active < MAX_NO_OF_FLOAT_SWITCHES)
        {
          fsw_updated_2_valid &= ~(1 << min_bit_not_active);
          min_bit_not_active++;
        }
      }
      fsw_old = fsw;
      fsw = (MRG_FSW_VALUE)fsw_updated_2_valid;
      mFsw1 = (MRG_FSW_VALUE)~fsw_updated_2_valid;
    }
    else
    {
      fsw_old = fsw;
    }
  }
  return fsw;
}



/*****************************************************************************
 * Function - CommonTransisionAction
 * DESCRIPTION:
 *
 *****************************************************************************/
GF_UINT8 FloatSwitchCtrl::CommonTransisionAction(TRANSISION_ACTION ta, GF_UINT8 present_value)
{
  if (ta < TA_NONE)
  {
    present_value &= ta;
  }
  else if (ta > TA_NONE)
  {
    if (ta == TA_TRUE)
    {
      present_value = TRUE;
    }
    else
    {
      present_value |= (ta & TA_OR_MASK);
    }
  }
  return present_value;
}



/*****************************************************************************
 * Function - UpdateFSWOutputs
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::UpdateFSWOutputs(const FSW_CONFIG_TRANSISION_ACTION *ta_ptr)
{
  PUMP_REF_TYPE previous_pump_ref;

  if ((ta_ptr->fsw_config > FSW_DRY_RUN_A) && (ta_ptr->fsw_config < FSW_START_ALL))
  {
    previous_pump_ref = mLocalFswPumpRef;
    mLocalFswPumpRef = (PUMP_REF_TYPE)CommonTransisionAction(ta_ptr->ta_pump_ref, (GF_UINT8)mLocalFswPumpRef);
    if (mLocalFswPumpRef == PUMP_REF_NO_PUMPS)
    {
      if (mLocalFswPumpRef != previous_pump_ref)
      {
        // One or more pumps is stopped - induce an afterrun
        mpTimerObjList[AFTER_RUN_DELAY_TIMER]->SetSwTimerPeriod(mpAfterRunDelay->GetValue(), S, false);
        mpTimerObjList[AFTER_RUN_DELAY_TIMER]->RetriggerSwTimer();
      }
    }
    else
    {
      // Pump is running again - afterrun is suspended
      mpTimerObjList[AFTER_RUN_DELAY_TIMER]->StopSwTimer();
    }
  }

  if (ta_ptr->fsw_config == FSW_HIGH_WATER)
  {
    if (CommonTransisionAction(ta_ptr->ta_high_level, 0) == TRUE)
    {
      mHighLevelFloatSwitchFlag = FSW_ABOVE_LEVEL;
      mpHighLevelSwitchActivated->SetValue(true);
    }
    else
    {
      mHighLevelFloatSwitchFlag = FSW_BELOW_LEVEL;
      mpHighLevelSwitchActivated->SetValue(false);
    }
  }
  if (ta_ptr->fsw_config == FSW_HIGH_WATER_A)
  {
    if (CommonTransisionAction(ta_ptr->ta_high_level, 0) == TRUE)
    {
      mHighLevelAnalogFlag = FSW_ABOVE_LEVEL;
    }
    else
    {
      mHighLevelAnalogFlag = FSW_BELOW_LEVEL;
    }
  }

  if (ta_ptr->fsw_config == FSW_DRY_RUN)
  {
    // Dry run has been detected - afterrun is suspended
    mpTimerObjList[AFTER_RUN_DELAY_TIMER]->StopSwTimer();
    if (CommonTransisionAction(ta_ptr->ta_dry_run, 0) == TRUE)
    {
      // Do not report dry running while foam draining
      if(mFoamDrainBlockDryRunAlarm == false)
      {
        mDryRunFloatSwitchFlag = FSW_BELOW_LEVEL;
      }
    }
    else
    {
      mDryRunFloatSwitchFlag = FSW_ABOVE_LEVEL;
    }
  }
  if (ta_ptr->fsw_config == FSW_DRY_RUN_A)
  {
    // Dry run has been detected - afterrun is suspended
    mpTimerObjList[AFTER_RUN_DELAY_TIMER]->StopSwTimer();
    if (CommonTransisionAction(ta_ptr->ta_dry_run, 0) == TRUE)
    {
      // Do not report dry running while foam draining
      if(mFoamDrainBlockDryRunAlarm == false)
      {
        mDryRunAnalogFlag = FSW_BELOW_LEVEL;
      }
    }
    else
    {
      mDryRunAnalogFlag = FSW_ABOVE_LEVEL;
    }
  }


  if (ta_ptr->fsw_config == FSW_ALARM)
  {
    if (CommonTransisionAction(ta_ptr->ta_alarm_level,0) == TRUE)
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL]->SetFault();
    }
    else
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_ALARM_LEVEL]->ResetFault();
    }
  }
}



/*****************************************************************************
 * Function - UpdateUnderLowestStopLevel
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::UpdateUnderLowestStopLevel(MRG_FSW_VALUE fsw)
{
  auto GF_UINT32 fsw_idx = 0;
  auto GF_UINT32 fsw_mask;

  if (mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
  {
    //TODO2>6 handle pump 3 to 6

    /* Lowest in analog config is STOP1 or STOP2 depending on level sets for the two STOP's */
    if (mpNoOfPumps->GetValue() == 2 && (mpStopLevelPump[PUMP_2]->GetValue() < mpStopLevelPump[PUMP_1]->GetValue()))
    {
      /* It is a two pump system and STOP2 is lowest. STOP2 is shift 2 */
      fsw_idx = 2;
    }
    else
    {
      /* It is a one pump system or STOP1 is lowest. STOP1 is shift 1 */
      fsw_idx = 1;
    }
  }
  else  // Float Switch level measuring
  {
    /* Find lowest stop level in configuration */
    while ((mFswCfg[fsw_idx] < FSW_STOP_ALL) && (fsw_idx < mNoOfFsw))
    {
      fsw_idx++;
    }
  }
  fsw_mask = 1 << fsw_idx;
  if (fsw & fsw_mask)
  {
    mpUnderLowestStopLevel->SetValue(FALSE);
  }
  else
  {
    mpUnderLowestStopLevel->SetValue(TRUE);
  }

}



/*****************************************************************************
 * Function - UpdateUnderDryRunAlarmLevel
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::UpdateUnderDryRunAlarmLevel(MRG_FSW_VALUE fsw)
{
  auto GF_UINT32 fsw_idx = 0;
  auto GF_UINT32 fsw_mask;
  auto bool dry_run_switch_present = true;

  // When not using floatswitches DRY_RUN_A is always the first "switch".
  // Thus the initial value of fsw_idx points at the dry run.
  // If using floatswitches the dry run switch must be located.
  if (mpPitLevelCtrlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
  {
    /* Find dry run level in configuration - if any */
    if (mFswCfg[fsw_idx] > FSW_DRY_RUN)
    {
      // The first fsw is higher ranked than dry run - this means there is no dry run switch
      dry_run_switch_present = false;
    }
    else
    {
      while ((mFswCfg[fsw_idx] < FSW_DRY_RUN) && (fsw_idx < mNoOfFsw))
      {
        fsw_idx++;
      }
    }
  }
  fsw_mask = 1 << fsw_idx;
  if (dry_run_switch_present == false || fsw & fsw_mask)
  {
    // No dry run switch means that we can never be below dry run alarm limit
    // If fsw indicates that dry run switch pinpointed by fsw_mask is lifted we are not below the limit either
    mUnderDryRunAlarmLevel = false;
  }
  else
  {
    // Switch is present and not lifted - we are below limit.
    mUnderDryRunAlarmLevel = true;
  }

}



/*****************************************************************************
 * Function - CheckForFSWInconsistency
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::CheckForFSWInconsistency(MRG_FSW_VALUE fsw)
{
  auto GF_UINT32 fsw_idx = 1;
  auto GF_UINT32 fsw_diffs = 0;

  if (mRunSubTaskCausedByConfChange == true)
  {
    // Configuration has been changed - any old inconsistency must be cleared.
    // This is done by setting no_fault_detected high - if a fault is still present in
    // the new configuration the no_fault will be reset and the inconsistency will
    // remain.
    mNoFaultDetectedCnt = 2;
  }

  if (mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
  {
    // No Float Switch inconsistency can occur while running at sensor
    mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_INCONSISTENCY]->ResetFault();

    if (mpPitLevelMeasured->GetQuality() != DP_AVAILABLE)
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR]->SetFault();
      mNoFaultDetectedCnt = 0;
    }
    if ((mFswCfg[7] == FSW_DRY_RUN) && ((fsw & DRY_RUN_MASK)==FALSE) && (fsw & DRY_RUN_A_MASK))
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS]->SetFault();
      mNoFaultDetectedCnt = 0;
    }
    if ((mFswCfg[7] == FSW_HIGH_WATER) && (fsw & HIGH_WATER_MASK_7) && ((fsw & HIGH_WATER_A_MASK) == FALSE))
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS]->SetFault();
      mNoFaultDetectedCnt = 0;
    }
    if ((mFswCfg[8] == FSW_HIGH_WATER) && (fsw & HIGH_WATER_MASK_8) && ((fsw & HIGH_WATER_A_MASK) == FALSE))
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS]->SetFault();
      mNoFaultDetectedCnt = 0;
    }
  }
  else  // Float Switch level measuring
  {
    // No sensor error - or conflict with level switches - can occur while running at float switches
    mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS]->ResetFault();
    mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR]->ResetFault();

    // Check for float switch inconsistency is only done upon a float switch change - and when we are sure
    // that the float switch inputs are valid after a configuration change.
    if (mRunSubTaskCausedBySwitchChange == true && mpFloatSwitchInputMoved->GetValue() == false)
    {
      while (fsw_idx < mNoOfFsw)
      {
        if (((fsw & (1 << (fsw_idx - 1))) > 0) != ((fsw & (1 << fsw_idx)) > 0))
        {
          fsw_diffs++;
        }
        fsw_idx++;
      }
      if ((fsw & 0x01) && (fsw_diffs > 1))
      {
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_INCONSISTENCY]->SetFault();
        mNoFaultDetectedCnt = 0;
      }
      else if (((fsw & 0x01) == 0) && (fsw_diffs > 0))
      {
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_INCONSISTENCY]->SetFault();
        mNoFaultDetectedCnt = 0;
      }
    }
  }


  // If pit is controlled by float switches the mNoFaultDetectedCnt must only be incremented
  // when the task is run due to a float switch level change. This is to assure, that a detected
  // switch inconsistency is not cleared until two valid switch situations have been detected.
  if (mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
  {
    if (mNoFaultDetectedCnt > 1)
    {
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS]->ResetFault();
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_INCONSISTENCY]->ResetFault();
      mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR]->ResetFault();
    }
    else
    {
      mNoFaultDetectedCnt++;
    }
  }
  else  // Float Switch level measuring
  {
    if (mRunSubTaskCausedBySwitchChange == true)
    {
      if (mNoFaultDetectedCnt > 1)
      {
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_CONFLICT_LEVELS]->ResetFault();
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_INCONSISTENCY]->ResetFault();
        mpFloatSwitchAlarmDelay[FSW_FAULT_OBJ_SENSOR]->ResetFault();
      }
      else
      {
        mNoFaultDetectedCnt++;
      }
    }
  }

}



/*****************************************************************************
 * Function - UpdateWaterLevelOutputs
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrl::UpdateWaterLevelOutputs(MRG_FSW_VALUE fsw)
{
  auto GF_UINT32 fsw_idx;

  if (mpPitLevelCtrlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)  // Analog level measuring
  {
    for (fsw_idx=0; ((fsw_idx < mNoOfFsw) && (fsw & (1 << fsw_idx))); fsw_idx++)
    {
    }
    mpFloatSwitchWaterLevel->SetValue(fsw_idx);
    if (mNoOfFsw)
    {
      mpFloatSwitchWaterLevelPct->SetValue(fsw_idx * 100 / mNoOfFsw);
    }
  }
}




PUMP_REF_TYPE FloatSwitchCtrl::GetPumpRef()
{
  return (PUMP_REF_TYPE)(((mpPumpRefLevel[PUMP_1]->GetQuality() == DP_AVAILABLE && mpPumpRefLevel[PUMP_1]->GetValue()) ? 1 : 0) |
                         ((mpPumpRefLevel[PUMP_2]->GetQuality() == DP_AVAILABLE && mpPumpRefLevel[PUMP_2]->GetValue()) ? 2 : 0));
}


void FloatSwitchCtrl::SetPumpRef(PUMP_REF_TYPE ref)
{
  mpPumpRefLevel[PUMP_1]->SetValue(((GF_UINT8)ref) & 1);
  mpPumpRefLevel[PUMP_2]->SetValue(((GF_UINT8)ref) & 2);

  mpPumpRefLevel[PUMP_3]->SetQuality(DP_NEVER_AVAILABLE);
  mpPumpRefLevel[PUMP_4]->SetQuality(DP_NEVER_AVAILABLE);
  mpPumpRefLevel[PUMP_5]->SetQuality(DP_NEVER_AVAILABLE);
  mpPumpRefLevel[PUMP_6]->SetQuality(DP_NEVER_AVAILABLE);
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

