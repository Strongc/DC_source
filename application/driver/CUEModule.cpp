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
/* CLASS NAME       : CUEModule                                             */
/*                                                                          */
/* FILE NAME        : CUEModule.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 6-4-2009 dd-mm-yyyy                                   */
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
#include <FactoryTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <CUEModule.h>
#include <SwTimer.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define COMMAND_RESPONSE_CHECK_TIME 5 // Do not make it too fast for Geni bus

// SW Timers
enum
{
  COMMAND_RESPONSE_CHECK_TIMER
};


typedef struct
{
  ALARM_ID_TYPE   slave_code;
  ALARM_ID_TYPE   cu361_code;
} ALARM_CONVERSION_TYPE;

const ALARM_CONVERSION_TYPE alarm_table[] =
{ // Slave ALARM code               CU361 ALARM code
  { (ALARM_ID_TYPE)1,               (ALARM_ID_TYPE)1   },
  { (ALARM_ID_TYPE)2,               (ALARM_ID_TYPE)2   },
  { (ALARM_ID_TYPE)3,               (ALARM_ID_TYPE)3   },
  { (ALARM_ID_TYPE)32,              (ALARM_ID_TYPE)32  },
  { (ALARM_ID_TYPE)40,              (ALARM_ID_TYPE)40  },
  { (ALARM_ID_TYPE)48,              (ALARM_ID_TYPE)48  },
  { (ALARM_ID_TYPE)49,              (ALARM_ID_TYPE)49  },
  { (ALARM_ID_TYPE)55,              (ALARM_ID_TYPE)55  },
  { (ALARM_ID_TYPE)57,              (ALARM_ID_TYPE)57  },
  { (ALARM_ID_TYPE)64,              (ALARM_ID_TYPE)64  },
  { (ALARM_ID_TYPE)70,              (ALARM_ID_TYPE)70  },
  { (ALARM_ID_TYPE)85,              (ALARM_ID_TYPE)85  },
  { (ALARM_ID_TYPE)89,              (ALARM_ID_TYPE)89  },
  { (ALARM_ID_TYPE)91,              (ALARM_ID_TYPE)91  },
  { (ALARM_ID_TYPE)93,              (ALARM_ID_TYPE)93  },
  { (ALARM_ID_TYPE)148,             (ALARM_ID_TYPE)148 },
  { (ALARM_ID_TYPE)149,             (ALARM_ID_TYPE)149 },
  { (ALARM_ID_TYPE)155,             (ALARM_ID_TYPE)155 },
  { (ALARM_ID_TYPE)175,             (ALARM_ID_TYPE)175 },
  { (ALARM_ID_TYPE)176,             (ALARM_ID_TYPE)176 },
  { (ALARM_ID_TYPE)241,             (ALARM_ID_TYPE)241 },
  { (ALARM_ID_TYPE)242,             (ALARM_ID_TYPE)242 },
  // End of table                   Must be last
  { ALARM_ID_NO_ALARM,              ALARM_ID_NO_ALARM }
};
#define ALARM_TABLE_SIZE  (sizeof(alarm_table)/sizeof(alarm_table[0]))

const ALARM_CONVERSION_TYPE warning_table[] =
{ // Slave WARNING code             CU361 WARNING code
  { (ALARM_ID_TYPE)30,              (ALARM_ID_TYPE)30  },
  { (ALARM_ID_TYPE)48,              (ALARM_ID_TYPE)48  },
  { (ALARM_ID_TYPE)64,              (ALARM_ID_TYPE)64  },
  { (ALARM_ID_TYPE)70,              (ALARM_ID_TYPE)70  },
  { (ALARM_ID_TYPE)77,              (ALARM_ID_TYPE)77  },
  { (ALARM_ID_TYPE)89,              (ALARM_ID_TYPE)89  },
  { (ALARM_ID_TYPE)91,              (ALARM_ID_TYPE)91  },
  { (ALARM_ID_TYPE)93,              (ALARM_ID_TYPE)93  },
  { (ALARM_ID_TYPE)148,             (ALARM_ID_TYPE)148 },
  { (ALARM_ID_TYPE)149,             (ALARM_ID_TYPE)149 },
  { (ALARM_ID_TYPE)175,             (ALARM_ID_TYPE)175 },
  { (ALARM_ID_TYPE)176,             (ALARM_ID_TYPE)176 },
  { (ALARM_ID_TYPE)240,             (ALARM_ID_TYPE)240 },
  // End of table                   Must be last
  { ALARM_ID_NO_ALARM,              ALARM_ID_NO_ALARM }
};
#define WARNING_TABLE_SIZE  (sizeof(warning_table)/sizeof(warning_table[0]))


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/


/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *****************************************************************************/
CUEModule::CUEModule(IO351_NO_TYPE moduleNo) : IO351(moduleNo)
{
  mpGeniSlaveIf = GeniSlaveIf::GetInstance();

  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_CUE_FAULT_OBJ; fault_id < NO_OF_CUE_FAULT_OBJ; fault_id++)
  {
    mpCUEAlarmDelay[fault_id] = new AlarmDelay(this);
    mCUEAlarmDelayCheckFlag[fault_id] = false;
  }

  mCUEStartRequest = false;
  mCUEReverseRequest = false;

  mpTimerObjList[COMMAND_RESPONSE_CHECK_TIMER] = new SwTimer(COMMAND_RESPONSE_CHECK_TIME, S, true, false, this);  // Periodic
  mCheckCommandResponseTimeout = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
CUEModule::~CUEModule()
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *****************************************************************************/
void CUEModule::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
    // misc.
    case SP_CUE_INSTALLED:
      mpCUEInstalled.Attach(pSubject);
      break;

    // commands
    case SP_CUE_RESET_EVENT:
      mpResetEvent.Attach(pSubject);
      break;
    case SP_CUE_SYSTEM_ALARM_RESET_EVENT:
      mpSystemAlarmResetEvent.Attach(pSubject);
      break;
    case SP_CUE_MODULE_ALARM_RESET_EVENT:
      mpModuleAlarmResetEvent.Attach(pSubject);
      break;
    case SP_CUE_STOP_EVENT:
      mpStopEvent.Attach(pSubject);
      break;
    case SP_CUE_START_EVENT:
      mpStartEvent.Attach(pSubject);
      break;
    case SP_CUE_REMOTE_EVENT:
      mpRemoteEvent.Attach(pSubject);
      break;
    case SP_CUE_CONST_FREQ_EVENT:
      mpConstFreqEvent.Attach(pSubject);
      break;
    case SP_CUE_FORWARD_EVENT:
      mpForwardEvent.Attach(pSubject);
      break;
    case SP_CUE_REVERSE_EVENT:
      mpReverseEvent.Attach(pSubject);
      break;
    case SP_CUE_REFERENCE:
      mpCUEReference.Attach(pSubject);
      break;

    // measured values
    case SP_CUE_VOLTAGE:
      mpVoltage.Attach(pSubject);
      break;
    case SP_CUE_POWER:
      mpPower.Attach(pSubject);
      break;
    case SP_CUE_ENERGY:
      mpEnergy.Attach(pSubject);
      break;
    case SP_CUE_CURRENT:
      mpCurrent.Attach(pSubject);
      break;
    case SP_CUE_FREQUENCY:
      mpFrequency.Attach(pSubject);
      break;
    case SP_CUE_SPEED:
      mpSpeed.Attach(pSubject);
      break;
    case SP_CUE_TORQUE:
      mpTorque.Attach(pSubject);
      break;
    case SP_CUE_REMOTE_TEMPERATURE:
      mpRemoteTemperature.Attach(pSubject);
      break;

    // status
    case SP_CUE_OPERATION_MODE:
      mpCUEOperationMode.Attach(pSubject);
      break;
    case SP_CUE_SYSTEM_MODE:
      mpCUESystemMode.Attach(pSubject);
      break;
    case SP_CUE_LOOP_MODE:
      mpCUEControlMode.Attach(pSubject);
      break;
    case SP_CUE_SOURCE_MODE:
      mpCUESourceMode.Attach(pSubject);
      break;
    case SP_CUE_REVERSE_STATUS:
      mpCUEReverseStatus.Attach(pSubject);
      break;

    // alarms
    case SP_CUE_GENI_COMM_FAULT_OBJ :
      mpCUEAlarmDelay[CUE_FAULT_OBJ_GENI_COMM]->SetSubjectPointer(id, pSubject);
      break;
    case SP_CUE_ALARM_FAULT_OBJ :
      mpCUEAlarmObj.Attach(pSubject);
      mpCUEAlarmDelay[CUE_FAULT_OBJ_ALARM]->SetSubjectPointer(id, pSubject);
      break;
    case SP_CUE_WARNING_FAULT_OBJ :
      mpCUEWarningObj.Attach(pSubject);
      mpCUEAlarmDelay[CUE_FAULT_OBJ_WARNING]->SetSubjectPointer(id, pSubject);
      break;

    // misc output
    case SP_CUE_CUE_DEVICE_STATUS:
      mpCUEDeviceStatus.Attach(pSubject);
      break;
    case SP_CUE_COMMUNICATION_FLAG :
      mpCUECommunicationFlag.Attach(pSubject);
      break;

  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void CUEModule::Update(Subject* pSubject)
{
  if (mpCUEReference.Update(pSubject))
  {
    // Nothing, but skip further checks since this is the most frequent update
  }
  else if (pSubject == mpTimerObjList[COMMAND_RESPONSE_CHECK_TIMER])
  {
    mCheckCommandResponseTimeout = true;
  }
  else if (mpStartEvent.Update(pSubject))
  {
    mCUEStartRequest = true;
  }
  else if (mpStopEvent.Update(pSubject))
  {
    mCUEStartRequest = false;
  }
  else if (mpForwardEvent.Update(pSubject))
  {
    mCUEReverseRequest = false;
  }
  else if (mpReverseEvent.Update(pSubject))
  {
    mCUEReverseRequest = true;
  }
  else
  {
    mpCUEInstalled.Update(pSubject);
    mpResetEvent.Update(pSubject);
    mpSystemAlarmResetEvent.Update(pSubject);
    mpModuleAlarmResetEvent.Update(pSubject);
    mpRemoteEvent.Update(pSubject);
    mpConstFreqEvent.Update(pSubject);

    for (int fault_id = FIRST_CUE_FAULT_OBJ; fault_id < NO_OF_CUE_FAULT_OBJ; fault_id++)
    {
      if (pSubject == mpCUEAlarmDelay[fault_id])
      {
        mCUEAlarmDelayCheckFlag[fault_id] = true;
      }
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void CUEModule::ConnectToSubjects()
{
  mpCUEInstalled->Subscribe(this);

  mpResetEvent->Subscribe(this);
  mpSystemAlarmResetEvent->Subscribe(this);
  mpModuleAlarmResetEvent->Subscribe(this);
  mpStopEvent->Subscribe(this);
  mpStartEvent->Subscribe(this);
  mpRemoteEvent->Subscribe(this);
  mpConstFreqEvent->Subscribe(this);
  mpForwardEvent->Subscribe(this);
  mpReverseEvent->Subscribe(this);
  mpCUEReference->Subscribe(this);

  for (int fault_id = FIRST_CUE_FAULT_OBJ; fault_id < NO_OF_CUE_FAULT_OBJ; fault_id++)
  {
    mpCUEAlarmDelay[fault_id]->ConnectToSubjects();
  }

}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void CUEModule::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_CUE_FAULT_OBJ; fault_id < NO_OF_CUE_FAULT_OBJ; fault_id++)
  {
    mpCUEAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}


/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CUEModule::InitSubTask()
{
  mSkipRunCounter = LAST_CUE - mModuleNo; // To distribute task load for the periodic task
  mExistingAlarmCode = ALARM_ID_NO_ALARM;
  mExistingWarningCode = ALARM_ID_NO_ALARM;
  SetCUEDataAvailability(DP_NEVER_AVAILABLE);

  mpCUEInstalled.SetUpdated();  // Do io module config determination
  mpCUECommunicationFlag->SetValue(false);  // No communication when starting up

  for (int fault_id = FIRST_CUE_FAULT_OBJ; fault_id < NO_OF_CUE_FAULT_OBJ; fault_id++)
  {
    mpCUEAlarmDelay[fault_id]->InitAlarmDelay();
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void CUEModule::RunSubTask()
{
  ALARM_ID_TYPE new_alarm_code;
  ALARM_ID_TYPE new_warning_code;

  // NOTICE: Send the reference as fast as possible.
  // There is no need to check if the CUE is installed as the reference is not sent directly
  // to the CUE. It is put into the class 5 table and here we must always have the newest reference - no
  // matter if the pump is present or not.
  if (mpCUEReference.IsUpdated())
  {
    mpGeniSlaveIf->SetCUEReference(mModuleNo, mpCUEReference->GetValue());
  }

  mSkipRunCounter++;
  if (mSkipRunCounter < 10)
  {
    // Just run this subtask every 10th time i.e. every 100 ms instead of every 10 ms.
    // This makes a significant reduction of the CPU load, due to the service of 50 alarms. (25 alarms and 2 instanses)
    return;
  }
  mSkipRunCounter = 0;

  // Check for setting of alarms
  for (int fault_id = FIRST_CUE_FAULT_OBJ; fault_id < NO_OF_CUE_FAULT_OBJ; fault_id++)
  {
    if (mCUEAlarmDelayCheckFlag[fault_id] == true)
    {
      mCUEAlarmDelayCheckFlag[fault_id] = false;
      mpCUEAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  // handle change of installation
  if (mpCUEInstalled.IsUpdated())
  {
    if (mpCUEInstalled->GetValue() == true)
    {
      // Insert in auto poll list
      GeniSlaveIf::GetInstance()->ConnectCUE(mModuleNo);
      HandleCUECommands(false); // To clear pending updates
    }
    else
    {
      // Put the CUE in local (But avoid sending a message to not installed CUE's after power up)
      if (mpCUECommunicationFlag->GetValue() == true)
      {
        mpGeniSlaveIf->CUELocal(mModuleNo);
      }
      // Remove from auto poll list
      GeniSlaveIf::GetInstance()->DisconnectCUE(mModuleNo);
      // The pump is not present - make certain that all errors and data from the pump is cleared
      HandleCUEAlarm(ALARM_ID_NO_ALARM);
      HandleCUEWarning(ALARM_ID_NO_ALARM);
      SetCUEDataAvailability(DP_NEVER_AVAILABLE);
      mpCUEAlarmDelay[CUE_FAULT_OBJ_GENI_COMM]->ResetFault();
      mpCUEDeviceStatus->SetValue(IO_DEVICE_STATUS_NOT_PRESENT);
      mpCUECommunicationFlag->SetValue(false);
    }
  }

  // module expected?
  if (mpCUEInstalled->GetValue() == true)
  {
    // handle alarms and status
    if (mpGeniSlaveIf->GetCUEAlarmCode(mModuleNo, &new_alarm_code) &&
        mpGeniSlaveIf->GetCUEWarningCode(mModuleNo, &new_warning_code))
    {
      if (mpCUECommunicationFlag->GetValue() == false)
      {
        // Communication status has changed to ok. Set CUE remote and constant frequency
        mpRemoteEvent.SetUpdated();
        mpConstFreqEvent.SetUpdated();
        // This flag will ensure a syncronizing of start/stop/forward/reverse via VfdSignalCtrl
        mpCUECommunicationFlag->SetValue(true);
      }

      mpCUEAlarmDelay[CUE_FAULT_OBJ_GENI_COMM]->ResetFault();
      HandleCUEAlarm(new_alarm_code);
      HandleCUEWarning(new_warning_code);
      HandleCUECommands(true);
      HandleCUEMeasuredValues();
      HandleCUEStatus();
      mpCUEReference.SetUpdated(); // Refresh geni reference since f_upper scaling in GeniSlaveIf may has changed

      if (new_alarm_code == 0 && new_warning_code == 0)
      {
        mpCUEDeviceStatus->SetValue(IO_DEVICE_STATUS_PRESENT_AND_OK);
      }
      else
      {
        mpCUEDeviceStatus->SetValue(IO_DEVICE_STATUS_PRESENT_WITH_ERROR);
      }
    }
    else
    {
      // we were unable to get the alarm code from GENI, set alarm and set data not available
      if (mpCUEAlarmDelay[CUE_FAULT_OBJ_GENI_COMM]->GetFault() != true)
      {
        mpCUECommunicationFlag->SetValue(false);
        HandleCUEAlarm(ALARM_ID_NO_ALARM);
        HandleCUEWarning(ALARM_ID_NO_ALARM);
        #ifndef __PC__
        mpCUEAlarmDelay[CUE_FAULT_OBJ_GENI_COMM]->SetFault();
        SetCUEDataAvailability(DP_NOT_AVAILABLE);
        mpCUEDeviceStatus->SetValue(IO_DEVICE_STATUS_NO_COMMUNICATION);
        #endif
      }
    }
  }

  // Update alarms
  for (int fault_id = FIRST_CUE_FAULT_OBJ; fault_id < NO_OF_CUE_FAULT_OBJ; fault_id++)
  {
    mpCUEAlarmDelay[fault_id]->UpdateAlarmDataPoint();
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
 * Function - HandleCUECommands
 * DESCRIPTION: Send commands via Geni
 *
 ****************************************************************************/
void CUEModule::HandleCUECommands(bool cueReady)
{
  if (cueReady == false)
  {
    // Clear pending updates
    mpResetEvent.ResetUpdated();
    mpSystemAlarmResetEvent.ResetUpdated();
    mpModuleAlarmResetEvent.ResetUpdated();
    mpRemoteEvent.ResetUpdated();
    mpConstFreqEvent.ResetUpdated();
    mpForwardEvent.ResetUpdated();
    mpReverseEvent.ResetUpdated();
    mpStartEvent.ResetUpdated();
    mpStopEvent.ResetUpdated();

    mpTimerObjList[COMMAND_RESPONSE_CHECK_TIMER]->StopSwTimer();;
    mCheckCommandResponseTimeout = false;
  }
  else
  {
    bool command_sent = true;
    if (mpResetEvent.IsUpdated())
    {
      mpGeniSlaveIf->CUEReset(mModuleNo);
    }
    else if (mpSystemAlarmResetEvent.IsUpdated() || mpModuleAlarmResetEvent.IsUpdated())
    {
      mpGeniSlaveIf->CUEAlarmReset(mModuleNo);
    }
    else if (mpRemoteEvent.IsUpdated())
    {
      mpGeniSlaveIf->CUERemote(mModuleNo);
    }
    else if (mpConstFreqEvent.IsUpdated())
    {
      mpGeniSlaveIf->CUEConstFreq(mModuleNo);
    }
    else if (mpForwardEvent.IsUpdated() || mpReverseEvent.IsUpdated())
    {
      // Ensure correct forward/reverse state if both has changed fast
      mpForwardEvent.ResetUpdated();
      mpReverseEvent.ResetUpdated();
      if (mCUEReverseRequest == true)
      {
        mpGeniSlaveIf->CUEReverse(mModuleNo);
      }
      else
      {
        mpGeniSlaveIf->CUEForward(mModuleNo);
      }
    }
    else if (mpStartEvent.IsUpdated() || mpStopEvent.IsUpdated())
    {
      // Ensure correct start/stop state if both has changed fast
      mpStartEvent.ResetUpdated();
      mpStopEvent.ResetUpdated();
      if (mCUEStartRequest == true)
      {
        mpGeniSlaveIf->CUEStart(mModuleNo);
      }
      else
      {
        mpGeniSlaveIf->CUEStop(mModuleNo);
      }
    }
    else
    {
      command_sent = false;
    }

    // Now some handling to check if the commands have been accepted by the CUE
    if (command_sent)
    {
      mpTimerObjList[COMMAND_RESPONSE_CHECK_TIMER]->RetriggerSwTimer();;
      mCheckCommandResponseTimeout = false;
    }
    else if (mCheckCommandResponseTimeout == true)
    {
      CheckCommandResponse();
      mCheckCommandResponseTimeout = false;
    }
  }
}

/*****************************************************************************
 * Function - CheckCommandResponse
 * DESCRIPTION: Check if commands have been accepted by the CUE,
 *              Otherwise send them again
 *
 ****************************************************************************/
void CUEModule::CheckCommandResponse()
{
  // Check remote
  if (mpCUESourceMode->GetQuality() == DP_AVAILABLE)
  {
    if (mpCUESourceMode->GetValue() != CUE_SOURCE_MODE_REMOTE)
    {
      mpRemoteEvent.SetUpdated();
    }
  }
  // Check constant frequency
  if (mpCUEControlMode->GetQuality() == DP_AVAILABLE)
  {
    if (mpCUEControlMode->GetValue() != CUE_LOOP_MODE_PUMP_CONST_FREQ)
    {
      mpConstFreqEvent.SetUpdated();
    }
  }
  // Check forward/reverse
  if (mpCUEReverseStatus->GetQuality() == DP_AVAILABLE)
  {
    if (mCUEReverseRequest == false && mpCUEReverseStatus->GetValue() != false)
    {
      mpForwardEvent.SetUpdated();
    }
    else if (mCUEReverseRequest == true && mpCUEReverseStatus->GetValue() != true)
    {
      mpReverseEvent.SetUpdated();
    }
  }
  // Check start/stop
  if (mpCUEOperationMode->GetQuality() == DP_AVAILABLE)
  {
    CUE_OPERATION_MODE_TYPE opr_mode = mpCUEOperationMode->GetValue();
    if (mCUEStartRequest == true && opr_mode != CUE_OPERATION_MODE_PUMP_ON)
    {
      mpStartEvent.SetUpdated();
    }
    else if (mCUEStartRequest == false && opr_mode != CUE_OPERATION_MODE_PUMP_OFF)
    {
      mpStopEvent.SetUpdated();
    }
  }
}

/*****************************************************************************
 * Function - HandleCUEMeasuredValues
 * DESCRIPTION:
 ****************************************************************************/
void CUEModule::HandleCUEMeasuredValues()
{
  float value;

  if (mpGeniSlaveIf->GetCUEVoltage(mModuleNo, &value))
  {
    mpVoltage->SetValue(value);
  }
  else
  {
    mpVoltage->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetCUEPower(mModuleNo, &value))
  {
    mpPower->SetValue(value);
  }
  else
  {
    mpPower->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetCUEEnergy(mModuleNo, &value))
  {
    mpEnergy->SetValue(value);
  }
  else
  {
    mpEnergy->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetCUECurrent(mModuleNo, &value))
  {
    mpCurrent->SetValue(value);
  }
  else
  {
    mpCurrent->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetCUEFrequency(mModuleNo, &value))
  {
    mpFrequency->SetValue(value);
  }
  else
  {
    mpFrequency->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetCUESpeed(mModuleNo, &value))
  {
    mpSpeed->SetValue(value);
  }
  else
  {
    mpSpeed->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetCUETorque(mModuleNo, &value))
  {
    mpTorque->SetValue(value);
  }
  else
  {
    mpTorque->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetCUERemoteTemperature(mModuleNo, &value))
  {
    mpRemoteTemperature->SetValue(value);
  }
  else
  {
    mpRemoteTemperature->SetQuality(DP_NOT_AVAILABLE);
  }
}


/*****************************************************************************
 * Function - HandleCUEStatus
 * DESCRIPTION:
 ****************************************************************************/
void CUEModule::HandleCUEStatus()
{
  CUE_OPERATION_MODE_TYPE opr_mode;
  if (mpGeniSlaveIf->GetCUEOperationMode(mModuleNo, &opr_mode))
  {
    mpCUEOperationMode->SetValue(opr_mode);
  }
  else
  {
    mpCUEOperationMode->SetQuality(DP_NOT_AVAILABLE);
  }

  CUE_SYSTEM_MODE_TYPE system_mode;
  if (mpGeniSlaveIf->GetCUESystemMode(mModuleNo, &system_mode))
  {
    mpCUESystemMode->SetValue(system_mode);
  }
  else
  {
    mpCUESystemMode->SetQuality(DP_NOT_AVAILABLE);
  }

  CUE_LOOP_MODE_TYPE control_mode;
  if (mpGeniSlaveIf->GetCUEControlMode(mModuleNo, &control_mode))
  {
    mpCUEControlMode->SetValue(control_mode);
  }
  else
  {
    mpCUEControlMode->SetQuality(DP_NOT_AVAILABLE);
  }

  CUE_SOURCE_MODE_TYPE source_mode;
  if (mpGeniSlaveIf->GetCUESourceMode(mModuleNo, &source_mode))
  {
    mpCUESourceMode->SetValue(source_mode);
  }
  else
  {
    mpCUESourceMode->SetQuality(DP_NOT_AVAILABLE);
  }

  bool reverse_status;
  if (mpGeniSlaveIf->GetCUEReverseStatus(mModuleNo, &reverse_status))
  {
    mpCUEReverseStatus->SetValue(reverse_status);
  }
  else
  {
    mpCUEReverseStatus->SetQuality(DP_NOT_AVAILABLE);
  }
}


/*****************************************************************************
 * Function - HandleCUEAlarm
 * DESCRIPTION:
 ****************************************************************************/
void CUEModule::HandleCUEAlarm(ALARM_ID_TYPE new_code)
{
  if (mExistingAlarmCode != new_code)
  {
    if (new_code == ALARM_ID_NO_ALARM || mExistingAlarmCode != ALARM_ID_NO_ALARM)
    {
      // The existing code is no longer reported from slave - it can be cleared
      mpCUEAlarmDelay[CUE_FAULT_OBJ_ALARM]->ResetFault();
      mExistingAlarmCode = ALARM_ID_NO_ALARM;
    }
    else if (new_code != 255) // Use an 'else' here to ensure a bit of time between alarm Reset/Set
    {
      // Convert from slave code to CU361 code
      int index = 0;
      ALARM_ID_TYPE cu361_code = ALARM_ID_OTHER;
      while (cu361_code == ALARM_ID_OTHER && index < ALARM_TABLE_SIZE)
      {
        if (new_code == alarm_table[index].slave_code)
        {
          cu361_code = alarm_table[index].cu361_code;
        }
        index++;
      }
      mpCUEAlarmObj->SetValue(cu361_code);
      mpCUEAlarmDelay[CUE_FAULT_OBJ_ALARM]->SetFault();
      mExistingAlarmCode = new_code;
    }
  }
}

/*****************************************************************************
 * Function - HandleCUEWarning
 * DESCRIPTION:
 ****************************************************************************/
void CUEModule::HandleCUEWarning(ALARM_ID_TYPE new_code)
{
  if (mExistingWarningCode != new_code)
  {
    if (new_code == ALARM_ID_NO_ALARM || mExistingWarningCode != ALARM_ID_NO_ALARM)
    {
      // The existing code is no longer reported from slave - it can be cleared
      mpCUEAlarmDelay[CUE_FAULT_OBJ_WARNING]->ResetFault();
      mExistingWarningCode = ALARM_ID_NO_ALARM;
    }
    else if (new_code != 255) // Use an 'else' here to ensure a bit of time between alarm Reset/Set
    {
      // Convert from slave code to CU361 code
      int index = 0;
      ALARM_ID_TYPE cu361_code = ALARM_ID_OTHER;
      while (cu361_code == ALARM_ID_OTHER && index < WARNING_TABLE_SIZE)
      {
        if (new_code == warning_table[index].slave_code)
        {
          cu361_code = warning_table[index].cu361_code;
        }
        index++;
      }
      mpCUEWarningObj->SetValue(cu361_code);
      mpCUEAlarmDelay[CUE_FAULT_OBJ_WARNING]->SetFault();
      mExistingWarningCode = new_code;
    }
  }
}


/*****************************************************************************
 * Function - SetCUEDataAvailability
 * DESCRIPTION:
 ****************************************************************************/
void CUEModule::SetCUEDataAvailability(DP_QUALITY_TYPE quality)
{
  mpVoltage->SetQuality(quality);
  mpPower->SetQuality(quality);
  mpEnergy->SetQuality(quality);
  mpCurrent->SetQuality(quality);
  mpFrequency->SetQuality(quality);
  mpSpeed->SetQuality(quality);
  mpTorque->SetQuality(quality);
  mpRemoteTemperature->SetQuality(quality);

  mpCUEOperationMode->SetQuality(quality);
  mpCUESystemMode->SetQuality(quality);
  mpCUEControlMode->SetQuality(quality);
  mpCUESourceMode->SetQuality(quality);
  mpCUEReverseStatus->SetQuality(quality);
}


/*****************************************************************************
 * Function - ConfigReceived
 * DESCRIPTION: Empty function to satisfy virtual function in base class.
 ****************************************************************************/
void CUEModule::ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType)
{
}

