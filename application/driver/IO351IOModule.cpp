/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : I0351IOModule                                         */
/*                                                                          */
/* FILE NAME        : I0351IOModule.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 04-03-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <microp.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <GeniSlaveIf.h>
#include <ErrorLog.h>
#include <FactoryTypes.h>
#include <WebIfHandler.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <IO351IOModule.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define IO_MODULE_TYPE_C      6
#define CONFIG_DELAY_TIMER    0
#define CONFIG_DELAY          5000
#define RESTART_DELAY         1000
#define CHECK_DIG_OUT_TIMER   1
#define CHECK_DIG_OUT_TIMEOUT 20000

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *****************************************************************************/
IO351IOModule::IO351IOModule(IO351_NO_TYPE moduleNo) : IO351(moduleNo)
{
  // init digital output state
  for (int i = 0; i < IO351_IOM_NO_OF_DIG_OUT; i++)
  {
    mDigOutState[i] = IO_MODULE_DIG_OUT_STATE_INITIAL;
  }

  mpTimerObjList[CONFIG_DELAY_TIMER] = new SwTimer(CONFIG_DELAY, MS, false, false, this);
  mpTimerObjList[CHECK_DIG_OUT_TIMER] = new SwTimer(CHECK_DIG_OUT_TIMEOUT, MS, false, false, this);
  mpGeniSlaveIf = GeniSlaveIf::GetInstance();

  /* Create objects for handling setting, clearing and delaying of alarm and warnings */
  for (int fault_id = FIRST_IO351_FAULT_OBJ; fault_id < NO_OF_IO351_FAULT_OBJ; fault_id++)
  {
    mpIO351AlarmDelay[fault_id] = new AlarmDelay(this);
    mIO351AlarmDelayCheckFlag[fault_id] = false;
  }

  #ifndef __PC__
  WebIfHandler::GetInstance()->SetIO351Pointer(moduleNo, this);
  #endif
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
IO351IOModule::~IO351IOModule()
{
  /* Delete objects for handling setting, clearing and delaying of alarm and warnings */
  for (int fault_id = FIRST_IO351_FAULT_OBJ; fault_id < NO_OF_IO351_FAULT_OBJ; fault_id++)
  {
    delete(mpIO351AlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *****************************************************************************/
void IO351IOModule::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
    // misc.
    case SP_IO351_IOM_NO_OF_IO_MODULES:
      mpNoOfIo351.Attach(pSubject);
      break;
    case SP_IO351_IOM_MODULE_READY:
      mpModuleReady.Attach(pSubject);
      break;
    case SP_IO351_IOM_RESTARTED_EVENT:
      mpRestartedEvent.Attach(pSubject);
      break;

    // analog inputs
    case SP_IO351_IOM_ANA_IN_1_CONF_SENSOR_ELECTRIC:
      mpAnaInConfSensorElectric[0].Attach(pSubject);
      break;
    case SP_IO351_IOM_ANA_IN_2_CONF_SENSOR_ELECTRIC:
      mpAnaInConfSensorElectric[1].Attach(pSubject);
      break;
    case SP_IO351_IOM_ANA_IN_1_CONF_MEASURED_VALUE:
      mpAnaInConfMeasuredValue[0].Attach(pSubject);
      break;
    case SP_IO351_IOM_ANA_IN_2_CONF_MEASURED_VALUE:
      mpAnaInConfMeasuredValue[1].Attach(pSubject);
      break;
    case SP_IO351_IOM_ANA_IN_1_VALUE:
      mpAnaInValue[0].Attach(pSubject);
      break;
    case SP_IO351_IOM_ANA_IN_2_VALUE:
      mpAnaInValue[1].Attach(pSubject);
      break;
    case SP_IO351_IOM_ANA_IN_1_ALARM_OBJ:
      mpAnaInAlarmObj[0].Attach(pSubject);
      break;
    case SP_IO351_IOM_ANA_IN_2_ALARM_OBJ:
      mpAnaInAlarmObj[1].Attach(pSubject);
      break;

    // counter inputs
    case SP_IO351_IOM_DIG_IN_1_COUNT:
      mpCounterValue[0].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_IN_2_COUNT:
      mpCounterValue[1].Attach(pSubject);
      break;

    // digital inputs
    case SP_IO351_IOM_DIG_IN_1_CONF_DIGITAL_INPUT_FUNC:
      mpDigInConfDigitalInputFunc[0].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_IN_2_CONF_DIGITAL_INPUT_FUNC:
      mpDigInConfDigitalInputFunc[1].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_IN_3_CONF_DIGITAL_INPUT_FUNC:
      mpDigInConfDigitalInputFunc[2].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_IN_4_CONF_DIGITAL_INPUT_FUNC:
      mpDigInConfDigitalInputFunc[3].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_IN_5_CONF_DIGITAL_INPUT_FUNC:
      mpDigInConfDigitalInputFunc[4].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_IN_6_CONF_DIGITAL_INPUT_FUNC:
      mpDigInConfDigitalInputFunc[5].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_IN_7_CONF_DIGITAL_INPUT_FUNC:
      mpDigInConfDigitalInputFunc[6].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_IN_8_CONF_DIGITAL_INPUT_FUNC:
      mpDigInConfDigitalInputFunc[7].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_IN_9_CONF_DIGITAL_INPUT_FUNC:
      mpDigInConfDigitalInputFunc[8].Attach(pSubject);
      break;

    // digital outputs
    case SP_IO351_IOM_DIG_OUT_1_CONF_RELAY_FUNC:
      mpDigOutConfRelayFunc[0].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_OUT_2_CONF_RELAY_FUNC:
      mpDigOutConfRelayFunc[1].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_OUT_3_CONF_RELAY_FUNC:
      mpDigOutConfRelayFunc[2].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_OUT_4_CONF_RELAY_FUNC:
      mpDigOutConfRelayFunc[3].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_OUT_5_CONF_RELAY_FUNC:
      mpDigOutConfRelayFunc[4].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_OUT_6_CONF_RELAY_FUNC:
      mpDigOutConfRelayFunc[5].Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_OUT_7_CONF_RELAY_FUNC:
      mpDigOutConfRelayFunc[6].Attach(pSubject);
      break;

    // digital input/output/ptc status bits
    case SP_IO351_IOM_DIG_IN_STATUS_BITS:
      mpDigInStatusBits.Attach(pSubject);
      break;
    case SP_IO351_IOM_DIG_OUT_STATUS_BITS:
      mpDigOutStatusBits.Attach(pSubject);
      break;
    case SP_IO351_IOM_PTC_STATUS:
      mpPtcBits.Attach(pSubject);
      break;

    /* Redirect SetSubjectPointer to AlarmDelays - and create an attach to the subject if
     * access is needed to alarm limits */
    case SP_IO351_IOM_GENI_COMM_FAULT_OBJ:
      mpIO351AlarmDelay[IO351_FAULT_OBJ_GENI_COMM]->SetSubjectPointer(id, pSubject);
      break;
    case SP_IO351_IOM_HW_ERROR_FAULT_OBJ:
      mpIO351AlarmDelay[IO351_FAULT_OBJ_HW_ERROR]->SetSubjectPointer(id, pSubject);
      break;

    // Demo suitcase
    case SP_IO351_IOM_DEMO_SUITCASE_MODE_ENABLED:
      mpDemoSuitcaseModeEnabled.Attach(pSubject);
      break;
    case SP_IO351_IOM_COMMUNICATION_CARD:
      mpCommunicationCard.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void IO351IOModule::Update(Subject* pSubject)
{
  if (mpDigOutStatusBits.Update(pSubject)) return;

  if (pSubject == mpTimerObjList[CONFIG_DELAY_TIMER])
  {
    mTimeOutFlag = true;
    return;
  } else if (pSubject == mpTimerObjList[CHECK_DIG_OUT_TIMER])
  {
    mCheckDigOutTimeout = true;
    return;
  }

  if (mpNoOfIo351.Update(pSubject)) return;
  for (int i = 0; i < IO351_IOM_NO_OF_ANA_IN; i++)
  {
    if (mpAnaInConfSensorElectric[i].Update(pSubject)) return;
    if (mpAnaInConfMeasuredValue[i].Update(pSubject)) return;
  }

  if (mpDemoSuitcaseModeEnabled.Update(pSubject)) return;
  if (mpCommunicationCard.Update(pSubject)) return;

  /* Updates from AlarmDelay's */
  for (int i = FIRST_IO351_FAULT_OBJ; i<= LAST_IO351_FAULT_OBJ; i++)
  {
    if (pSubject == mpIO351AlarmDelay[i])
    {
      mIO351AlarmDelayCheckFlag[i] = true;
      break;
    }
  }

}
/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void IO351IOModule::ConnectToSubjects()
{
  mpNoOfIo351->Subscribe(this);

  for (int i = 0; i < IO351_IOM_NO_OF_ANA_IN; i++)
  {
    mpAnaInConfSensorElectric[i]->Subscribe(this);
    mpAnaInConfMeasuredValue[i]->Subscribe(this);
  }

  mpDigOutStatusBits->Subscribe(this);

  /* Redirect ConnectToSubjects to AlarmDelay's */
  for (int fault_id = FIRST_IO351_FAULT_OBJ; fault_id < NO_OF_IO351_FAULT_OBJ; fault_id++)
  {
    mpIO351AlarmDelay[fault_id]->ConnectToSubjects();
  }

  mpDemoSuitcaseModeEnabled->Subscribe(this);
  mpCommunicationCard->Subscribe(this);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void IO351IOModule::InitSubTask()
{
  // distribute pointer
  GeniSlaveIf::GetInstance()->SetIO351Pointer(mModuleNo, this);

  mPowerOnCntValidFlag = false;
  mConfigReceivedFlag = false;
  mDoConfigFlag = false;
  mThisIoModuleExpectedFlag = false;
  mTimeOutFlag = false;
  mCheckDigOutTimeout = false;

  // do io module config determination
  mpNoOfIo351.SetUpdated();

  // initial update of digital outputs
  mpDigOutStatusBits.SetUpdated();

  /* Initialize AlarmDelay's */
  for (int fault_id = FIRST_IO351_FAULT_OBJ; fault_id < NO_OF_IO351_FAULT_OBJ; fault_id++)
  {
    mpIO351AlarmDelay[fault_id]->InitAlarmDelay();
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void IO351IOModule::RunSubTask()
{
  ALARM_ID_TYPE new_alarm_code = ALARM_ID_NO_ALARM;
  U16 new_power_on_cnt;

  /* Check if an AlarmDelay requests for attention - expired error timer. */
  for (int fault_id = FIRST_IO351_FAULT_OBJ; fault_id < NO_OF_IO351_FAULT_OBJ; fault_id++)
  {
    if (mIO351AlarmDelayCheckFlag[fault_id] == true)
    {
      mIO351AlarmDelayCheckFlag[fault_id] = false;
      mpIO351AlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  // handle change of AI settings
  bool config_changed = false;
  for (int i = 0; i < IO351_IOM_NO_OF_ANA_IN; i++)
  {
    if (mpAnaInConfSensorElectric[i].IsUpdated(false)) // Call with false, because IsUpdated is called again!
    {
      config_changed = true;
      break;
    }
    if (mpAnaInConfMeasuredValue[i].IsUpdated(false)) // Call with false, because IsUpdated is called again!
    {
      config_changed = true;
      break;
    }
  }

  if (config_changed)
  {
    HandleChangeOfAiSettings();
  }

  // handle change of no of modules
  if (mpNoOfIo351.IsUpdated())
  {
    DetermineIoModuleConfig();
    HandleIoConfigDataPoints();
    mIoModuleConfigState = IOM_CONFIG_STATE_CHECK_CONFIG;
  }

  #ifdef __PC__
    DP_QUALITY_TYPE quality = (mThisIoModuleExpectedFlag ? DP_AVAILABLE : DP_NEVER_AVAILABLE);
    mpDigInStatusBits->SetQuality(quality);
    mpDigOutStatusBits->SetQuality(quality);
    mpAnaInValue[0]->SetQuality(quality);
    mpAnaInValue[1]->SetQuality(quality);
    mpPtcBits->SetQuality(quality);
  #endif


  // module expected?
  if (mThisIoModuleExpectedFlag)
  {
    if (mIoModuleConfigState != IOM_CONFIG_STATE_CONFIG_DONE)
    {
#ifndef __PC__
      HandleIoModuleConfig();
#endif
    }

    else // "Normal" run
    {
      // handle alarm
      if (mpGeniSlaveIf->GetIO351AlarmCode(mModuleNo, &new_alarm_code))
      {
        // communication OK?
        if (new_alarm_code == ALARM_ID_NO_ALARM)
        {
          mpIO351AlarmDelay[IO351_FAULT_OBJ_GENI_COMM]->ResetFault();
          mpIO351AlarmDelay[IO351_FAULT_OBJ_HW_ERROR]->ResetFault();

          // handle restart
          if (mpGeniSlaveIf->GetIO351PowerOnCnt(mModuleNo, &new_power_on_cnt))
          {
            if (mPowerOnCntValidFlag)
            {
              if (new_power_on_cnt != mPowerOnCntActual)
              {
                // module has restarted
                mPowerOnCntActual = new_power_on_cnt;
                mpRestartedEvent->SetEvent();
                mIoModuleConfigState = IOM_CONFIG_STATE_CHECK_CONFIG;
              }
            }
            else
            {
              // now we know the power on count, set flag and store value
              mPowerOnCntValidFlag = true;
              mPowerOnCntActual = new_power_on_cnt;
            }
          }
        }
        else
        {
          if ( new_alarm_code == ALARM_ID_COMMUNICATION_FAULT)
          {
            mpIO351AlarmDelay[IO351_FAULT_OBJ_GENI_COMM]->SetFault();
          }
          else if ( new_alarm_code == ALARM_ID_HARDWARE_FAULT_TYPE_2)
          {
            mpIO351AlarmDelay[IO351_FAULT_OBJ_GENI_COMM]->ResetFault();

            mpIO351AlarmDelay[IO351_FAULT_OBJ_HW_ERROR]->SetFault();
          }
          else
          {
            mpIO351AlarmDelay[IO351_FAULT_OBJ_GENI_COMM]->ResetFault();

            mpIO351AlarmDelay[IO351_FAULT_OBJ_HW_ERROR]->ResetFault();
          }
        }
      }
      else
      {
        // we were unable to get the alarm code from GENI, set alarm and enter config again to recover
        mpIO351AlarmDelay[IO351_FAULT_OBJ_GENI_COMM]->SetFault();
        
        mIoModuleConfigState = IOM_CONFIG_STATE_CHECK_CONFIG;
      }

      // everything ok?
      if ((new_alarm_code == ALARM_ID_NO_ALARM) && (mIoModuleConfigState == IOM_CONFIG_STATE_CONFIG_DONE))
      {
        // handle digital input counters
        HandleDigitalInputCounters();

        // handle digital inputs
        HandleDigitalInputs();

        // handle PTC inputs
        HandlePTCInputs();

        // handle digital outputs
        HandleDigitalOutputs();

        // handle analog inputs
        HandleAnalogInputs();

        // set module ready flag
        mpModuleReady->SetValue(true);
      }
      else
      {
        // set module ready flag
        mpModuleReady->SetValue(false);

        // set quality to NOT_AVAILABLE
        mpDigInStatusBits->SetQuality(DP_NOT_AVAILABLE);

        mpPtcBits->SetQuality(DP_NOT_AVAILABLE);        

        for (int i = 0; i < IO351_IOM_NO_OF_ANA_IN; i++)
        {
          mpAnaInValue[i]->SetQuality(DP_NOT_AVAILABLE);
          //TEST JSM mpAnaInAlarmObj[i]->SetQuality(DP_NOT_AVAILABLE);
        }
      }
    }
  }
  else
  {
    // Module not expected - reset communication and hardware alarm
    mpIO351AlarmDelay[IO351_FAULT_OBJ_GENI_COMM]->ResetFault();
    mpIO351AlarmDelay[IO351_FAULT_OBJ_HW_ERROR]->ResetFault();
  }

  /* Update the AlarmDataPoints with the changes made in AlarmDelay */
  for (int fault_id = FIRST_IO351_FAULT_OBJ; fault_id < NO_OF_IO351_FAULT_OBJ; fault_id++)
  {
    mpIO351AlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - ConfigReceived
 * DESCRIPTION: GeniSlaveIf callback function on config received event.
 *              Update member flag mDoConfigFlag.
 ****************************************************************************/
void IO351IOModule::ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType)
{
  if (rxedOk)
  {
    if (noOfPumps == 0 &&
        noOfVlt == 0 &&
        pumpOffset == 0 &&
        moduleType == IO_MODULE_TYPE_C)
    {
      // configuration match -> configuration completed
      mDoConfigFlag = false;
    }
    else
    {
      // configuration doesn't match -> configuration is needed
      mDoConfigFlag = true;
    }
  }
  else
  {
    // configuration not received or failed, do not configure before communication is re-established
    mDoConfigFlag = false;
  }

  // set config received flag
  mConfigReceivedFlag = true;
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
/*****************************************************************************
 * Function - HandleIoModuleConfig
 * DESCRIPTION:
 ****************************************************************************/
void IO351IOModule::HandleIoModuleConfig()
{
  switch (mIoModuleConfigState)
  {
    case IOM_CONFIG_STATE_CHECK_CONFIG:
      if (GeniSlaveIf::GetInstance()->GetIO351ModuleConfig(mModuleNo))
      {
        mpTimerObjList[CONFIG_DELAY_TIMER]->SetSwTimerPeriod(CONFIG_DELAY, MS, false);
        mpTimerObjList[CONFIG_DELAY_TIMER]->RetriggerSwTimer();
        mTimeOutFlag = false;
        mIoModuleConfigState = IOM_CONFIG_STATE_READ_CONFIG;
      }
      else
      {
        // geni failed, try again later
        mIoModuleConfigState = IOM_CONFIG_STATE_CHECK_CONFIG;
      }
      break;

    case IOM_CONFIG_STATE_READ_CONFIG:
      if (mConfigReceivedFlag)
      {
        mConfigReceivedFlag = false;
        if (mDoConfigFlag)
        {
          if (GeniSlaveIf::GetInstance()->SetIO351ModuleConfig(mModuleNo, 0, 0, 0, IO_MODULE_TYPE_C))
          {
            // start timer
            mpTimerObjList[CONFIG_DELAY_TIMER]->SetSwTimerPeriod(CONFIG_DELAY, MS, false);
            mpTimerObjList[CONFIG_DELAY_TIMER]->RetriggerSwTimer();
            mTimeOutFlag = false;
            mIoModuleConfigState = IOM_CONFIG_STATE_SEND_CONFIG;
          }
          else
          {
            // geni failed, try again later
            mIoModuleConfigState = IOM_CONFIG_STATE_CHECK_CONFIG;
          }
        }
        else
        {
          mIoModuleConfigState = IOM_CONFIG_STATE_CONFIG_DONE;
          mpTimerObjList[CHECK_DIG_OUT_TIMER]->RetriggerSwTimer();
        }
      }
      else if (mTimeOutFlag)
      {
        // geni failed, try again later
        mIoModuleConfigState = IOM_CONFIG_STATE_CHECK_CONFIG;
      }
      break;

    case IOM_CONFIG_STATE_SEND_CONFIG:
      if (mTimeOutFlag)
      {
        mTimeOutFlag = false;
        if (GeniSlaveIf::GetInstance()->ResetIO351(mModuleNo))
        {
          // start timer
          mpTimerObjList[CONFIG_DELAY_TIMER]->SetSwTimerPeriod(RESTART_DELAY, MS, false);
          mpTimerObjList[CONFIG_DELAY_TIMER]->RetriggerSwTimer();
          mIoModuleConfigState = IOM_CONFIG_STATE_DELAY;
        }
        else
        {
          // geni failed, try again later
          mIoModuleConfigState = IOM_CONFIG_STATE_CHECK_CONFIG;
        }
      }
      break;

    case IOM_CONFIG_STATE_DELAY:
      if (mTimeOutFlag)
      {
        mTimeOutFlag = false;
        mIoModuleConfigState = IOM_CONFIG_STATE_CONFIG_DONE;
        mpTimerObjList[CHECK_DIG_OUT_TIMER]->RetriggerSwTimer();
      }
      break;
  }
}

/*****************************************************************************
 * Function - DetermineIoModuleConfig
 * DESCRIPTION:
 ****************************************************************************/
void IO351IOModule::DetermineIoModuleConfig()
{
  switch (mModuleNo)
  {
  case IO351_IOM_NO_1:
    mThisIoModuleExpectedFlag = (mpNoOfIo351->GetAsInt() > 0) ? true : false;
    break;

  case IO351_IOM_NO_2:
    mThisIoModuleExpectedFlag = (mpNoOfIo351->GetAsInt() > 1) ? true : false;
    break;

  case IO351_IOM_NO_3:
    mThisIoModuleExpectedFlag = (mpNoOfIo351->GetAsInt() > 2) ? true : false;
    break;

  default:
    FatalErrorOccured("IO351 index out of range!");
  }

#ifndef __PC__

  // connect or disconnect
  if (mThisIoModuleExpectedFlag)
  {
    GeniSlaveIf::GetInstance()->ConnectIO351(mModuleNo);
  }
  else
  {
    GeniSlaveIf::GetInstance()->DisconnectIO351(mModuleNo);
  }

#endif

}

/*****************************************************************************
 * Function - HandleChangeOfAiSettings
 * DESCRIPTION:
 ****************************************************************************/
void IO351IOModule::HandleChangeOfAiSettings()
{
  int i;

  if (!mThisIoModuleExpectedFlag)
  {
    for (i = 0; i < IO351_IOM_NO_OF_ANA_IN; i++)
    {
      mpAnaInValue[i]->SetQuality(DP_NEVER_AVAILABLE);
      mpAnaInConfSensorElectric[i]->SetQuality(DP_NEVER_AVAILABLE);
      mpAnaInConfMeasuredValue[i]->SetQuality(DP_NEVER_AVAILABLE);
    }
  }
}

/*****************************************************************************
 * Function - HandleIoConfigDataPoints
 * DESCRIPTION:
 ****************************************************************************/
void IO351IOModule::HandleIoConfigDataPoints()
{
  int i;

  if (mThisIoModuleExpectedFlag)
  {
    // mark as updated to ensure correct configuration
    mpAnaInConfSensorElectric[0].SetUpdated();
    mpAnaInConfSensorElectric[1].SetUpdated();

    // mark as updated to force update of digital outputs
    mpDigOutStatusBits.SetUpdated();

    // set config DP's quality to available
    for (i = 0; i < IO351_IOM_NO_OF_DIG_IN; i++)
    {
      mpDigInConfDigitalInputFunc[i]->SetQuality(DP_AVAILABLE);
    }

    for (i = 0; i < IO351_IOM_NO_OF_DIG_OUT; i++)
    {
      mpDigOutConfRelayFunc[i]->SetQuality(DP_AVAILABLE);
    }

    for (i = 0; i < IO351_IOM_NO_OF_ANA_IN; i++)
    {
      mpAnaInConfSensorElectric[i]->SetQuality(DP_AVAILABLE);
      mpAnaInConfMeasuredValue[i]->SetQuality(DP_AVAILABLE);
    }
  }
  else
  {
    mpModuleReady->SetValue(false);

    // set DP's quality to never available (values AND config)
    mpModuleReady->SetQuality(DP_NEVER_AVAILABLE);
    mpDigInStatusBits->SetQuality(DP_NEVER_AVAILABLE);
    
    mpPtcBits->SetQuality(DP_NEVER_AVAILABLE);   


    for (i = 0; i < IO351_IOM_NO_OF_DIG_IN; i++)
    {
      mpDigInConfDigitalInputFunc[i]->SetQuality(DP_NEVER_AVAILABLE);
    }

    for (i = 0; i < IO351_IOM_NO_OF_DIG_OUT; i++)
    {
      mpDigOutConfRelayFunc[i]->SetQuality(DP_NEVER_AVAILABLE);
    }

    for (i = 0; i < IO351_IOM_NO_OF_ANA_IN; i++)
    {
      mpAnaInValue[i]->SetQuality(DP_NEVER_AVAILABLE);
      //TEST JSM mpAnaInAlarmObj[i]->SetQuality(DP_NEVER_AVAILABLE);
      mpAnaInConfSensorElectric[i]->SetQuality(DP_NEVER_AVAILABLE);
      mpAnaInConfMeasuredValue[i]->SetQuality(DP_NEVER_AVAILABLE);
    }
  }
}

/*****************************************************************************
 * Function - HandleDigitalInputCounters
 * DESCRIPTION:
 ****************************************************************************/
void IO351IOModule::HandleDigitalInputCounters()
{
  U32 new_counter_value;

  if (mpGeniSlaveIf->GetIO351DigitalInputCounter(mModuleNo, IO351_DIG_IN_NO_1, &new_counter_value))
  {
    mpCounterValue[0]->SetValue(new_counter_value);
  }
  if (mpGeniSlaveIf->GetIO351DigitalInputCounter(mModuleNo, IO351_DIG_IN_NO_2, &new_counter_value))
  {
    mpCounterValue[1]->SetValue(new_counter_value);
  }
}

/*****************************************************************************
 * Function - HandleDigitalInputs
 * DESCRIPTION:
 ****************************************************************************/
void IO351IOModule::HandleDigitalInputs()
{
  U16 new_dig_in_status;

  // handle digital input status
  if (mpGeniSlaveIf->GetIO351DigitalInputStatus(mModuleNo, &new_dig_in_status))
  {
    mpDigInStatusBits->SetValue(new_dig_in_status);
  }
}

/*****************************************************************************
 * Function - HandlePTCInputs
 * DESCRIPTION:
 ****************************************************************************/

void IO351IOModule::HandlePTCInputs()
{
  U8 new_PTC_status;

  // handle PTC input status
  if (mpGeniSlaveIf->GetIO351PTCStatus(mModuleNo, &new_PTC_status))
  {
    new_PTC_status >>= 1;
    mpPtcBits->SetValue(new_PTC_status); //PTC terminals 30 to 40
  }
}

/*****************************************************************************
 * Function - HandleDigitalOutputs
 * DESCRIPTION:
 ****************************************************************************/
void IO351IOModule::HandleDigitalOutputs()
{
  int i;
  U8 digOutStatusBits;
  U8 readDigOutStatusBits;

  if (mModuleNo == IO351_IOM_NO_1 && mpDemoSuitcaseModeEnabled->GetValue()) // Demo Suitcase Mode, used to select communication card
  {
    digOutStatusBits = mpDigOutStatusBits->GetValue() & b10011111; // clear bit 6 and 5
    digOutStatusBits |= SelectComCardForDemoSuitcase();
  }
  else
  {
    digOutStatusBits = mpDigOutStatusBits->GetValue();
  }

  // handle digital output status
  bool update_flag = mpDigOutStatusBits.IsUpdated();
  update_flag |= mpCommunicationCard.IsUpdated();
  update_flag |= mpDemoSuitcaseModeEnabled.IsUpdated();
  if (update_flag == true)
  {
    for (i = 0; i < IO351_IOM_NO_OF_DIG_OUT; i++)
    {
      const bool newStatus = TEST_BIT_HIGH(digOutStatusBits, i) ? true : false;

      switch (mDigOutState[i])
      {
      case IO_MODULE_DIG_OUT_STATE_INITIAL:
        mDigOutState[i] = newStatus ? IO_MODULE_DIG_OUT_STATE_SET : IO_MODULE_DIG_OUT_STATE_RESET;
        break;

      case IO_MODULE_DIG_OUT_STATE_SET:
      case IO_MODULE_DIG_OUT_STATE_IS_SET:
        if (!newStatus)
        {
          mDigOutState[i] = IO_MODULE_DIG_OUT_STATE_RESET;
        }
        break;

      case IO_MODULE_DIG_OUT_STATE_RESET:
      case IO_MODULE_DIG_OUT_STATE_IS_RESET:
        if (newStatus)
        {
          mDigOutState[i] = IO_MODULE_DIG_OUT_STATE_SET;
        }
        break;

      default:  // we should never get here.... just in case!
        mDigOutState[i] = newStatus ? IO_MODULE_DIG_OUT_STATE_SET : IO_MODULE_DIG_OUT_STATE_RESET;
        break;
      }
    }
  }

  // check digital output state machine
  for (i = 0; i < IO351_IOM_NO_OF_DIG_OUT; i++)
  {
    switch (mDigOutState[i])
    {
    case IO_MODULE_DIG_OUT_STATE_IS_SET:
    case IO_MODULE_DIG_OUT_STATE_IS_RESET:
      break;
    case IO_MODULE_DIG_OUT_STATE_SET:
      mpTimerObjList[CHECK_DIG_OUT_TIMER]->RetriggerSwTimer();
      mCheckDigOutTimeout = false;
      if (mpGeniSlaveIf->SetIO351DigitalOutputStatus(mModuleNo, (IO351_DIG_OUT_NO_TYPE)i, true))
      {
        mDigOutState[i] = IO_MODULE_DIG_OUT_STATE_IS_SET;
      }
      break;
    case IO_MODULE_DIG_OUT_STATE_RESET:
      mpTimerObjList[CHECK_DIG_OUT_TIMER]->RetriggerSwTimer();
      mCheckDigOutTimeout = false;
      if (mpGeniSlaveIf->SetIO351DigitalOutputStatus(mModuleNo, (IO351_DIG_OUT_NO_TYPE)i, false))
      {
        mDigOutState[i] = IO_MODULE_DIG_OUT_STATE_IS_RESET;
      }
      break;
    }
  }

  // read and check digital output status on timeout
  if (mCheckDigOutTimeout)
  {
    mCheckDigOutTimeout = false;
    mpTimerObjList[CHECK_DIG_OUT_TIMER]->RetriggerSwTimer();

    if (mpGeniSlaveIf->GetIO351DigitalOutputStatus(mModuleNo, &readDigOutStatusBits))
    {
      if (digOutStatusBits != readDigOutStatusBits)
      {
        for (i = 0; i < IO351_IOM_NO_OF_DIG_OUT; i++)
        {
          if (TEST_BIT_HIGH(digOutStatusBits, i) != TEST_BIT_HIGH(readDigOutStatusBits, i))
          {
            if (mpGeniSlaveIf->SetIO351DigitalOutputStatus(mModuleNo, (IO351_DIG_OUT_NO_TYPE)i, TEST_BIT_HIGH(digOutStatusBits, i) ? true : false))
            {
              // TODO
            }
            else
            {
              // TODO
            }
          }
        }
      }
    }
  }
}

/*****************************************************************************
 * Function - HandleAnalogInputs
 * DESCRIPTION:
 ****************************************************************************/
void IO351IOModule::HandleAnalogInputs()
{
  float new_percentage;

  for (int i = 0; i < IO351_IOM_NO_OF_ANA_IN; i++)
  {
    // handle change of analog sensor electric config
    if (mpAnaInConfSensorElectric[i].IsUpdated(false))
    {
      if (mpGeniSlaveIf->SetIO351AnalogInputConfig(mModuleNo, (IO351_ANA_IN_NO_TYPE)i, mpAnaInConfSensorElectric[i]->GetValue()))
      {
        mpAnaInConfSensorElectric[i].ResetUpdated();
      }
    }

    // get new value from module if sensor enabled
    if ((mpAnaInConfSensorElectric[i]->GetValue() == SENSOR_ELECTRIC_DISABLED) ||
        (mpAnaInConfMeasuredValue[i]->GetValue() == MEASURED_VALUE_NOT_SELECTED))
    {
      // clear alarm and set quality to never available
      //TEST JSM mpAnaInAlarmObj[i]->SetValue(ALARM_ID_NO_ALARM);
      //TEST JSM mpAnaInAlarmObj[i]->SetQuality(DP_NEVER_AVAILABLE);
      mpAnaInValue[i]->SetQuality(DP_NEVER_AVAILABLE);
    }
    else
    {
      // get new value from geni
      if (mpGeniSlaveIf->GetIO351AnalogInputValue(mModuleNo, (IO351_ANA_IN_NO_TYPE)i, &new_percentage))
      {
        // set new value and clear alarm
        mpAnaInValue[i]->SetValue(new_percentage);
        //TEST JSM mpAnaInAlarmObj[i]->SetValue(ALARM_ID_NO_ALARM);
      }
      else
      {
        // set sensor faiult alarm and set measured value DP to not available
        //TEST JSM mpAnaInAlarmObj[i]->SetValue(ALARM_ID_SENSOR_FAULT);
        mpAnaInValue[i]->SetQuality(DP_NOT_AVAILABLE);
      }
    }
  }
}

/*****************************************************************************
 * Function - webifGetNoOfIOModules
 * DESCRIPTION: Called by the webifHandler
 *****************************************************************************/
void IO351IOModule::webifGetNoOfIOModules(std::ostream& res)
{
  res << mpNoOfIo351->GetAsInt();
}

/*****************************************************************************
 * Function - webifSetNoOfIOModules
 * DESCRIPTION: Called by the webifHandler
 *****************************************************************************/
void IO351IOModule::webifSetNoOfIOModules(int noOfIOModules)
{
  mpNoOfIo351->SetAsInt(noOfIOModules);
}

/*****************************************************************************
 * Function - fmtDPAsInt
 * DESCRIPTION: webif helper
 *****************************************************************************/
static void fmtDPAsInt(HttpResponse& res, AlarmDataPoint* pDP)
{
  switch (pDP->GetQuality())
  {
    case DP_AVAILABLE:
      res << pDP->GetValue();
    break;

    case DP_NOT_AVAILABLE:
      res << "<i>NOT AVAILABLE</i>";
    break;

    case DP_NEVER_AVAILABLE:
      res << "<i>NEVER AVAILABLE</i>";
    break;
  }
}

/*****************************************************************************
 * Function - fmtDPAsFloat
 * DESCRIPTION: webif helper
 *****************************************************************************/
static void fmtDPAsFloat(HttpResponse& res, INumberDataPoint* pDP)
{
  switch (pDP->GetQuality())
  {
    case DP_AVAILABLE:
      res << pDP->GetAsFloat();
    break;

    case DP_NOT_AVAILABLE:
      res << "<i>NOT AVAILABLE</i>";
    break;

    case DP_NEVER_AVAILABLE:
      res << "<i>NEVER AVAILABLE</i>";
    break;
  }
}

/*****************************************************************************
 * Function - fmtDPAsBits
 * DESCRIPTION: webif helper
 *****************************************************************************/
static void fmtDPAsBits(HttpResponse& res, IIntegerDataPoint* pDP, const int bitCount)
{
  switch (pDP->GetQuality())
  {
    case DP_AVAILABLE:
    {
      res << "<code>";
      for (int i = 0; i < bitCount; i++)
      {
        res << (i + 1) << " ";
      }

      res << "<br/>";

      for (int i = 0; i < bitCount; i++)
      {
        if (pDP->GetAsInt() & (1 << i))
          res << "X ";
        else
          res << "- ";
      }
      res << "</code>";
    }
    break;

    case DP_NOT_AVAILABLE:
      res << "<i>NOT AVAILABLE</i>";
    break;

    case DP_NEVER_AVAILABLE:
      res << "<i>NEVER AVAILABLE</i>";
    break;
  }
}

/*****************************************************************************
 * Function - webifMakeStatusHtml
 * DESCRIPTION: Called by the webifHandler
 *****************************************************************************/
void IO351IOModule::webifMakeStatusHtml(HttpResponse& res)
{
  int i;

  res << "<table cellspacing=\"0px\" cellpadding=\"0px\">";

  res << "<tr><td>Module ready:</td><td width=\"15px\">&nbsp;</td><td>" << (mpModuleReady->GetValue() ? "Yes" : "No") << "</td></tr>";

  res << "<tr><td>Power on count:</td><td></td><td>";
  if (mPowerOnCntValidFlag)
    res << mPowerOnCntActual;
  else
    res << "Unknown";
  res << "</td></tr>";

  res << "<tr><td>Digital input status:</td><td></td><td>";
  fmtDPAsBits(res, mpDigInStatusBits.GetSubject(), IO351_IOM_NO_OF_DIG_IN);
  res << "</td></tr>";

  res << "<tr><td>PTC status:</td><td></td><td>";
  fmtDPAsBits(res, mpPtcBits.GetSubject(), 6);
  res << "</td></tr>";

  res << "<tr><td>Digital output status:</td><td></td><td>";
  fmtDPAsBits(res, mpDigOutStatusBits.GetSubject(), IO351_IOM_NO_OF_DIG_OUT);
  res << "</td></tr>";

  for (i = 0; i < IO351_IOM_NO_OF_ANA_IN; i++)
  {
    res << "<tr><td>Analog input #" << (i + 1) << " alarm code:</td><td></td><td>";
    fmtDPAsInt(res, mpAnaInAlarmObj[i].GetSubject());
    res << "</td></tr>";

    res << "<tr><td>Analog input #" << (i + 1) << " value:</td><td></td><td>";
    fmtDPAsFloat(res, mpAnaInValue[i].GetSubject());
    res << "</td></tr>";
  }

  res << "</table>";
}

/*****************************************************************************
 * Function - SelectComCardForDemoSuitcase
 * DESCRIPTION: Called when Demo Suitcase Mode is enabled and the IO351 module
 *              is module 1. This function select communication card via DO6
 *              and DO7 depending on the card seleted via the display. This
 *              means that you can configure DO6 and DO7 via the display and
 *              the output would appear as working, but there are no connection
 *              to the metal.
 ****************************************************************************/
U8 IO351IOModule::SelectComCardForDemoSuitcase()
{
  // DO7 is placed at bit 6
  // DO6 is placed at bit 5
  enum
  {
    SELECT_NO_CARD      = b00000000, // DO7 = 0 - DO6 = 0
    SELECT_MODBUS_CARD  = b00100000, // DO7 = 0 - DO6 = 1
    SELECT_GSM_CARD     = b01000000, // DO7 = 1 - DO6 = 0
    SELECT_GRM_CARD     = b01100000  // DO7 = 1 - DO6 = 1
  };

  U8 dig_out;

  switch (mpCommunicationCard->GetValue())
  {
    case COM_CARD_NONE:
      dig_out = SELECT_NO_CARD;
      break;

    case COM_CARD_CIM_200_MODBUS:
      dig_out = SELECT_MODBUS_CARD;
      break;

    case COM_CARD_CIM_250_GSM:
      dig_out = SELECT_GSM_CARD;
      break;

    case COM_CARD_CIM_270_GRM:
      dig_out = SELECT_GRM_CARD;
      break;

    default:
      dig_out = SELECT_NO_CARD;
      break;
  }

  return dig_out;
}
