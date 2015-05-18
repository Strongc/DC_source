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
/* CLASS NAME       : MP204Module                                           */
/*                                                                          */
/* FILE NAME        : MP204Module.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 10-08-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <GeniSlaveIf.h>
#include <FactoryTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <MP204Module.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

typedef struct
{
  ALARM_ID_TYPE   mp204_alarm;
  ALARM_ID_TYPE   cu361_alarm;
} ALARM_CONVERSION_TYPE;
const ALARM_CONVERSION_TYPE alarm_table[] =
{ // MP204 ALARM code               CU361 alarm code
  { (ALARM_ID_TYPE)2,               (ALARM_ID_TYPE)2    },
  { (ALARM_ID_TYPE)3,               (ALARM_ID_TYPE)70   },  // PTC
  { (ALARM_ID_TYPE)4,               (ALARM_ID_TYPE)4    },
  { (ALARM_ID_TYPE)9,               (ALARM_ID_TYPE)9    },
  { (ALARM_ID_TYPE)15,              (ALARM_ID_TYPE)15   },
  { (ALARM_ID_TYPE)18,              (ALARM_ID_TYPE)18   },
  { (ALARM_ID_TYPE)20,              (ALARM_ID_TYPE)20   },
  { (ALARM_ID_TYPE)32,              (ALARM_ID_TYPE)32   },
  { (ALARM_ID_TYPE)40,              (ALARM_ID_TYPE)40   },
  { (ALARM_ID_TYPE)48,              (ALARM_ID_TYPE)48   },
  { (ALARM_ID_TYPE)56,              (ALARM_ID_TYPE)56   },
  { (ALARM_ID_TYPE)64,              (ALARM_ID_TYPE)64   },
  { (ALARM_ID_TYPE)71,              (ALARM_ID_TYPE)71   },
  { (ALARM_ID_TYPE)111,             (ALARM_ID_TYPE)111  },
  { (ALARM_ID_TYPE)112,             (ALARM_ID_TYPE)112  },
  { (ALARM_ID_TYPE)113,             (ALARM_ID_TYPE)113  },
  { (ALARM_ID_TYPE)120,             (ALARM_ID_TYPE)120  },
  { (ALARM_ID_TYPE)123,             (ALARM_ID_TYPE)123  },
  { (ALARM_ID_TYPE)124,             (ALARM_ID_TYPE)124  },
  // End of table                   Must be last
  { ALARM_ID_NO_ALARM,              ALARM_ID_NO_ALARM }
};
#define ALARM_TABLE_SIZE  (sizeof(alarm_table)/sizeof(alarm_table[0]))

typedef struct
{
  U32                     mp204_warning_bit;
  MP204_FAULT_OBJ_TYPE    fault_obj;
  U16                     subject_relation;
} WARNING_CONVERSION_TYPE;
#define WARNBIT(byte, bit)          ((U32)1<<((byte-1)*8+bit))
const WARNING_CONVERSION_TYPE warning_table[] =
{// Warning bit geni. Fault object index.                                 Subject relation.
  { WARNBIT(3,5),     MP204_FAULT_OBJ_WARNING_TIME_FOR_SERVICE          , SP_MP204_WARNING_TIME_FOR_SERVICE           },
  { WARNBIT(2,0),     MP204_FAULT_OBJ_WARNING_INSULATION_RESISTANCE_LOW , SP_MP204_WARNING_INSULATION_RESISTANCE_LOW  },
  { WARNBIT(2,6),     MP204_FAULT_OBJ_WARNING_TOO_MANY_STARTS_PER_HOUR  , SP_MP204_WARNING_TOO_MANY_STARTS_PER_HOUR   },
  { WARNBIT(3,6),     MP204_FAULT_OBJ_WARNING_LOAD_CONTINUES_DESPITE_OFF, SP_MP204_WARNING_LOAD_CONTINUES_DESPITE_OFF },
  { WARNBIT(1,0),     MP204_FAULT_OBJ_WARNING_OVERVOLTAGE               , SP_MP204_WARNING_OVERVOLTAGE                },
  { WARNBIT(1,1),     MP204_FAULT_OBJ_WARNING_UNDERVOLTAGE              , SP_MP204_WARNING_UNDERVOLTAGE               },
  { WARNBIT(1,2),     MP204_FAULT_OBJ_WARNING_OVERLOAD                  , SP_MP204_WARNING_OVERLOAD                   },
  { WARNBIT(1,3),     MP204_FAULT_OBJ_WARNING_UNDERLOAD                 , SP_MP204_WARNING_UNDERLOAD                  },
  { WARNBIT(2,1),     MP204_FAULT_OBJ_WARNING_OVERTEMPERATURE_TEMPCON   , SP_MP204_WARNING_OVERTEMPERATURE_TEMPCON    },
  { WARNBIT(2,2),     MP204_FAULT_OBJ_WARNING_OVERTEMPERATURE_PT100     , SP_MP204_WARNING_OVERTEMPERATURE_PT100      },
  { WARNBIT(3,3),     MP204_FAULT_OBJ_WARNING_TEMPCON_SENSOR_FAULT      , SP_MP204_WARNING_TEMPCON_SENSOR_FAULT       },
  { WARNBIT(1,4),     MP204_FAULT_OBJ_WARNING_CURRENT_ASYMMETRY         , SP_MP204_WARNING_CURRENT_ASYMMETRY          },
  { WARNBIT(2,4),     MP204_FAULT_OBJ_WARNING_COS_PHI_MAX               , SP_MP204_WARNING_COS_PHI_MAX                },
  { WARNBIT(2,5),     MP204_FAULT_OBJ_WARNING_COS_PHI_MIN               , SP_MP204_WARNING_COS_PHI_MIN                },
  { WARNBIT(3,0),     MP204_FAULT_OBJ_WARNING_START_CAPACITOR_TOO_LOW   , SP_MP204_WARNING_START_CAPACITOR_TOO_LOW    },
  { WARNBIT(3,1),     MP204_FAULT_OBJ_WARNING_RUN_CAPACITOR_TOO_LOW     , SP_MP204_WARNING_RUN_CAPACITOR_TOO_LOW      },
  { WARNBIT(3,4),     MP204_FAULT_OBJ_WARNING_PT100_SENSOR_FAULT        , SP_MP204_WARNING_PT100_SENSOR_FAULT         },
  //All bits at last. Other alarm                                         Must be last
  { 0xFFFFFFFF,       MP204_FAULT_OBJ_WARNING_OTHER,                      SP_MP204_WARNING_OTHER                      }
};
#define WARNING_TABLE_SIZE  (sizeof(warning_table)/sizeof(warning_table[0]))


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/


/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *****************************************************************************/
MP204Module::MP204Module(IO351_NO_TYPE moduleNo) : IO351(moduleNo)
{
  mpGeniSlaveIf = GeniSlaveIf::GetInstance();

  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_MP204_FAULT_OBJ; fault_id < NO_OF_MP204_FAULT_OBJ; fault_id++)
  {
    mpMP204AlarmDelay[fault_id] = new AlarmDelay(this);
    mMP204AlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
MP204Module::~MP204Module()
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *****************************************************************************/
void MP204Module::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
    // config
    case SP_MP204_INSTALLED:
      mpMP204Installed.Attach(pSubject);
      break;

    // input (commands)
    case SP_MP204_SYSTEM_ALARM_RESET_EVENT:
      mpSystemAlarmResetEvent.Attach(pSubject);
      break;
    case SP_MP204_MODULE_ALARM_RESET_EVENT:
      mpModuleAlarmResetEvent.Attach(pSubject);
      break;

    // output (measured values)
    case SP_MP204_VOLTAGE:
      mpVoltage.Attach(pSubject);
      break;
    case SP_MP204_CURRENT:
      mpCurrent.Attach(pSubject);
      break;
    case SP_MP204_CURRENT_ASYMMETRY:
      mpCurrentAsymmetry.Attach(pSubject);
      break;
    case SP_MP204_COS_PHI:
      mpCosPhi.Attach(pSubject);
      break;
    case SP_MP204_POWER:
      mpPower.Attach(pSubject);
      break;
    case SP_MP204_ENERGY:
      mpEnergy.Attach(pSubject);
      break;
    case SP_MP204_INSULATION_RESISTANCE:
      mpInsulationResistance.Attach(pSubject);
      break;
    case SP_MP204_TEMPERATURE_PTC:
      mpTemperaturePtc.Attach(pSubject);
      break;
    case SP_MP204_TEMPERATURE_PT:
      mpTemperaturePt.Attach(pSubject);
      break;
    case SP_MP204_TEMPERATURE_TEMPCON:
      mpTemperatureTempcon.Attach(pSubject);
      break;

    // communication
    case SP_MP204_COMMUNICATION_FLAG :
      mpMP204CommunicationFlag.Attach(pSubject);
      break;
    case SP_MP204_DEVICE_STATUS:
      mpMP204DeviceStatus.Attach(pSubject);
      break;
    case SP_MP204_GENI_COMM_FAULT_OBJ :
      mpMP204AlarmDelay[MP204_FAULT_OBJ_GENI_COMM]->SetSubjectPointer(id, pSubject);
      break;

    // alarms
    case SP_MP204_ALARM_FAULT_OBJ :
      mpMP204AlarmObj.Attach(pSubject);
      mpMP204AlarmDelay[MP204_FAULT_OBJ_ALARM]->SetSubjectPointer(id, pSubject);
      break;
    default:
      // Look up warnings in list
      for (int index = 0; index < WARNING_TABLE_SIZE; index++)
      {
        if (id == warning_table[index].subject_relation)
        {
          mpMP204AlarmDelay[warning_table[index].fault_obj]->SetSubjectPointer(id, pSubject);
          break;
        }
      }
      break;
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void MP204Module::Update(Subject* pSubject)
{
  mpMP204Installed.Update(pSubject);
  mpSystemAlarmResetEvent.Update(pSubject);
  mpModuleAlarmResetEvent.Update(pSubject);

  for (int fault_id = FIRST_MP204_FAULT_OBJ; fault_id < NO_OF_MP204_FAULT_OBJ; fault_id++)
  {
    if (pSubject == mpMP204AlarmDelay[fault_id])
    {
      mMP204AlarmDelayCheckFlag[fault_id] = true;
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void MP204Module::ConnectToSubjects()
{
  mpMP204Installed->Subscribe(this);
  mpSystemAlarmResetEvent->Subscribe(this);
  mpModuleAlarmResetEvent->Subscribe(this);

  for (int fault_id = FIRST_MP204_FAULT_OBJ; fault_id < NO_OF_MP204_FAULT_OBJ; fault_id++)
  {
    mpMP204AlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void MP204Module::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_MP204_FAULT_OBJ; fault_id < NO_OF_MP204_FAULT_OBJ; fault_id++)
  {
    mpMP204AlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void MP204Module::InitSubTask()
{
  mSkipRunCounter = LAST_MP204 - mModuleNo; // To distribute task load for the periodic task
  mExistingAlarmCode = ALARM_ID_NO_ALARM;
  mExistingWarnings = 0;
  SetMP204DataAvailability(DP_NEVER_AVAILABLE);

  mpMP204Installed.SetUpdated();  // Do  module config determination
  mpMP204CommunicationFlag->SetValue(false);  // No communication when starting up

  for (int fault_id = FIRST_MP204_FAULT_OBJ; fault_id < NO_OF_MP204_FAULT_OBJ; fault_id++)
  {
    mpMP204AlarmDelay[fault_id]->InitAlarmDelay();
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void MP204Module::RunSubTask()
{
  ALARM_ID_TYPE new_alarm_code;
  U32 new_warning_code;

  mSkipRunCounter++;
  if (mSkipRunCounter < 10)
  {
    // Just run this subtask every 10th time i.e. every 100 ms instead of every 10 ms.
    // This makes a significant reduction of the CPU load, due to the service of 6 instances
    return;
  }
  mSkipRunCounter = 0;

  // Check for setting of alarms
  for (int fault_id = FIRST_MP204_FAULT_OBJ; fault_id < NO_OF_MP204_FAULT_OBJ; fault_id++)
  {
    if (mMP204AlarmDelayCheckFlag[fault_id] == true)
    {
      mMP204AlarmDelayCheckFlag[fault_id] = false;
      mpMP204AlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  // handle change of installation
  if (mpMP204Installed.IsUpdated())
  {
    if (mpMP204Installed->GetValue() == true)
    {
      // Insert in auto poll list
      GeniSlaveIf::GetInstance()->ConnectMP204(mModuleNo);
    }
    else
    {
      // Remove from auto poll list
      GeniSlaveIf::GetInstance()->DisconnectMP204(mModuleNo);
      // The module is not present - make certain that all errors and data from the module is cleared
      HandleMP204Alarm(ALARM_ID_NO_ALARM);
      HandleMP204Warning(0);
      SetMP204DataAvailability(DP_NEVER_AVAILABLE);
      mpMP204AlarmDelay[MP204_FAULT_OBJ_GENI_COMM]->ResetFault();
      mpMP204DeviceStatus->SetValue(IO_DEVICE_STATUS_NOT_PRESENT);
      mpMP204CommunicationFlag->SetValue(false);
    }
  }

  // module expected?
  if (mpMP204Installed->GetValue() == true)
  {
    // check reset alarm
    if (mpSystemAlarmResetEvent.IsUpdated() || mpModuleAlarmResetEvent.IsUpdated())
    {
      if (mpMP204Installed->GetValue() == true)
      {
        mpGeniSlaveIf->MP204AlarmReset(mModuleNo);
      }
    }

    // handle alarm and status
    if (mpGeniSlaveIf->GetMP204AlarmCode(mModuleNo, &new_alarm_code) &&
        mpGeniSlaveIf->GetMP204WarningCode(mModuleNo, &new_warning_code))
    {
      if (mpMP204CommunicationFlag->GetValue() == false)
      {
        mpMP204CommunicationFlag->SetValue(true);
      }

      mpMP204AlarmDelay[MP204_FAULT_OBJ_GENI_COMM]->ResetFault();
      HandleMP204Alarm(new_alarm_code);
      HandleMP204Warning(new_warning_code);
      HandleMP204MeasuredValues();

      if (new_alarm_code == 0 && new_warning_code == 0)
      {
        mpMP204DeviceStatus->SetValue(IO_DEVICE_STATUS_PRESENT_AND_OK);
      }
      else
      {
        mpMP204DeviceStatus->SetValue(IO_DEVICE_STATUS_PRESENT_WITH_ERROR);
      }
    }
    else
    {
      // we were unable to get the alarm code from GENI, set alarm and set data not available
      if (mpMP204AlarmDelay[MP204_FAULT_OBJ_GENI_COMM]->GetFault() != true)
      {
        // clear any alarms and/or warnings caused by the MP204
        HandleMP204Alarm(ALARM_ID_NO_ALARM);
        HandleMP204Warning(0);
        mpMP204CommunicationFlag->SetValue(false);
        #ifndef __PC__
        mpMP204AlarmDelay[MP204_FAULT_OBJ_GENI_COMM]->SetFault();
        SetMP204DataAvailability(DP_NOT_AVAILABLE);
        mpMP204DeviceStatus->SetValue(IO_DEVICE_STATUS_NO_COMMUNICATION);
        #endif
      }
    }
  }

  // Update alarms
  for (int fault_id = FIRST_MP204_FAULT_OBJ; fault_id < NO_OF_MP204_FAULT_OBJ; fault_id++)
  {
    mpMP204AlarmDelay[fault_id]->UpdateAlarmDataPoint();
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
 * Function - HandleMP204MeasuredValues
 * DESCRIPTION:
 ****************************************************************************/
void MP204Module::HandleMP204MeasuredValues()
{
  float float_value;
  bool  bool_value;

  if (mpGeniSlaveIf->GetMP204Voltage(mModuleNo, &float_value))
  {
    mpVoltage->SetValue(float_value);
  }
  else
  {
    mpVoltage->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetMP204Current(mModuleNo, &float_value))
  {
    mpCurrent->SetValue(float_value);
  }
  else
  {
    mpCurrent->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetMP204CosPhi(mModuleNo, &float_value))
  {
    mpCosPhi->SetValue(float_value);
  }
  else
  {
    mpCosPhi->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetMP204Power(mModuleNo, &float_value))
  {
    mpPower->SetValue(float_value);
  }
  else
  {
    mpPower->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetMP204Energy(mModuleNo, &float_value))
  {
    mpEnergy->SetValue(float_value);
  }
  else
  {
    mpEnergy->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetMP204CurrentAsymmetry(mModuleNo, &float_value))
  {
    mpCurrentAsymmetry->SetValue(float_value);
  }
  else
  {
    mpCurrentAsymmetry->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetMP204InsulationResistance(mModuleNo, &float_value))
  {
    mpInsulationResistance->SetValue(float_value);
  }
  else
  {
    mpInsulationResistance->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetMP204TemperaturePtc(mModuleNo, &bool_value))
  {
    mpTemperaturePtc->SetValue(bool_value);
  }
  else
  {
    mpTemperaturePtc->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetMP204TemperaturePt(mModuleNo, &float_value))
  {
    mpTemperaturePt->SetValue(float_value);
  }
  else
  {
    mpTemperaturePt->SetQuality(DP_NOT_AVAILABLE);
  }
  if (mpGeniSlaveIf->GetMP204TemperatureTempcon(mModuleNo, &float_value))
  {
    mpTemperatureTempcon->SetValue(float_value);
  }
  else
  {
    mpTemperatureTempcon->SetQuality(DP_NOT_AVAILABLE);
  }
}

/*****************************************************************************
 * Function - HandleMP204Alarm
 * DESCRIPTION:
 ****************************************************************************/
void MP204Module::HandleMP204Alarm(ALARM_ID_TYPE alarm_code)
{
  if (mExistingAlarmCode != alarm_code)
  {
    if (alarm_code == ALARM_ID_NO_ALARM || mExistingAlarmCode != ALARM_ID_NO_ALARM)
    {
      // The existing alarm code is no longer reported from MP204 - it can be cleared
      mpMP204AlarmDelay[MP204_FAULT_OBJ_ALARM]->ResetFault();
      mExistingAlarmCode = ALARM_ID_NO_ALARM;
    }
    else if (alarm_code != 255) // Use an 'else' here to ensure a bit of time between alarm Reset/Set
    {
      // Convert from MP204 alarm code to CU361 alarm code
      int index = 0;
      ALARM_ID_TYPE cu361_alarm = ALARM_ID_OTHER;
      while (cu361_alarm == ALARM_ID_OTHER && index < ALARM_TABLE_SIZE)
      {
        if (alarm_code == alarm_table[index].mp204_alarm)
        {
          cu361_alarm = alarm_table[index].cu361_alarm;
        }
        index++;
      }
      mpMP204AlarmObj->SetValue(cu361_alarm);
      mpMP204AlarmDelay[MP204_FAULT_OBJ_ALARM]->SetFault();
      mExistingAlarmCode = alarm_code;
    }
  }
}

/*****************************************************************************
 * Function - HandleMP204Warning
 * DESCRIPTION:
 ****************************************************************************/
void MP204Module::HandleMP204Warning(U32 warnings)
{
  if (mExistingWarnings != warnings && warnings < 0xFFFFFF)
  {
    mExistingWarnings = warnings;
    // Check all warnings in list
    U32 check_pattern;
    for (int index = 0; index < WARNING_TABLE_SIZE; index++)
    {
      check_pattern = warning_table[index].mp204_warning_bit;
      if (warnings & check_pattern)
      {
        // Clear the bit to prepare for a check for unknown alarms at the end of the list.
        // This works since the check pattern at the end of the list is 0xFFFFFFFF.
        warnings &= ~check_pattern;
        mpMP204AlarmDelay[warning_table[index].fault_obj]->SetFault();
      }
      else
      {
        mpMP204AlarmDelay[warning_table[index].fault_obj]->ResetFault();
      }
    }
  }
}

/*****************************************************************************
 * Function - SetMP204DataAvailability
 * DESCRIPTION:
 ****************************************************************************/
void MP204Module::SetMP204DataAvailability(DP_QUALITY_TYPE quality)
{
  mpVoltage->SetQuality(quality);
  mpCurrent->SetQuality(quality);
  mpCurrentAsymmetry->SetQuality(quality);
  mpCosPhi->SetQuality(quality);
  mpPower->SetQuality(quality);
  mpEnergy->SetQuality(quality);
  mpInsulationResistance->SetQuality(quality);
  mpTemperaturePtc->SetQuality(quality);
  mpTemperaturePt->SetQuality(quality);
  mpTemperatureTempcon->SetQuality(quality);
}

/*****************************************************************************
 * Function - ConfigReceived
 * DESCRIPTION: Empty function to satisfy virtual function in base class.
 ****************************************************************************/
void MP204Module::ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType)
{
}
