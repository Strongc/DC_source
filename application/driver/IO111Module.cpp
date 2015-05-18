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
/* CLASS NAME       : I0111Module                                           */
/*                                                                          */
/* FILE NAME        : I0111Module.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 28-11-2007 dd-mm-yyyy                                 */
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
#include <IO111Module.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

typedef struct
{
  ALARM_ID_TYPE   slave_code;
  ALARM_ID_TYPE   cu361_code;
} ALARM_CONVERSION_TYPE;

const ALARM_CONVERSION_TYPE alarm_table[] =
{ // Slave ALARM code               CU361 ALARM code
  { (ALARM_ID_TYPE)10,              (ALARM_ID_TYPE)10   },
  { (ALARM_ID_TYPE)11,              (ALARM_ID_TYPE)11   },
  { (ALARM_ID_TYPE)20,              (ALARM_ID_TYPE)20   },
  { (ALARM_ID_TYPE)22,              (ALARM_ID_TYPE)22   },
  { (ALARM_ID_TYPE)24,              (ALARM_ID_TYPE)24   },
  { (ALARM_ID_TYPE)25,              (ALARM_ID_TYPE)25   },
  { (ALARM_ID_TYPE)64,              (ALARM_ID_TYPE)64   },
  { (ALARM_ID_TYPE)69,              (ALARM_ID_TYPE)69   },
  { (ALARM_ID_TYPE)145,             (ALARM_ID_TYPE)145  },
  { (ALARM_ID_TYPE)146,             (ALARM_ID_TYPE)146  },
  { (ALARM_ID_TYPE)170,             (ALARM_ID_TYPE)170 },
  { (ALARM_ID_TYPE)179,             (ALARM_ID_TYPE)179 },
  { (ALARM_ID_TYPE)180,             (ALARM_ID_TYPE)180 },
  { (ALARM_ID_TYPE)181,             (ALARM_ID_TYPE)181  },
  // End of table                   Must be last
  { ALARM_ID_NO_ALARM,              ALARM_ID_NO_ALARM }
};
#define ALARM_TABLE_SIZE  (sizeof(alarm_table)/sizeof(alarm_table[0]))

const ALARM_CONVERSION_TYPE warning_table[] =
{ // Slave WARNING code             CU361 WARNING code
  { (ALARM_ID_TYPE)10,              (ALARM_ID_TYPE)10  },
  { (ALARM_ID_TYPE)11,              (ALARM_ID_TYPE)11  },
  { (ALARM_ID_TYPE)20,              (ALARM_ID_TYPE)20  },
  { (ALARM_ID_TYPE)24,              (ALARM_ID_TYPE)24  },
  { (ALARM_ID_TYPE)25,              (ALARM_ID_TYPE)25  },
  { (ALARM_ID_TYPE)64,              (ALARM_ID_TYPE)64  },
  { (ALARM_ID_TYPE)72,              (ALARM_ID_TYPE)72  },
  { (ALARM_ID_TYPE)83,              (ALARM_ID_TYPE)83  },
  { (ALARM_ID_TYPE)85,              (ALARM_ID_TYPE)85  },
  { (ALARM_ID_TYPE)145,             (ALARM_ID_TYPE)145 },
  { (ALARM_ID_TYPE)146,             (ALARM_ID_TYPE)146 },
  { (ALARM_ID_TYPE)170,             (ALARM_ID_TYPE)170 },
  { (ALARM_ID_TYPE)179,             (ALARM_ID_TYPE)179 },
  { (ALARM_ID_TYPE)180,             (ALARM_ID_TYPE)180 },
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
IO111Module::IO111Module(IO351_NO_TYPE moduleNo) : IO351(moduleNo)
{
  mpGeniSlaveIf = GeniSlaveIf::GetInstance();

  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_IO111_FAULT_OBJ; fault_id < NO_OF_IO111_FAULT_OBJ; fault_id++)
  {
    mpIO111AlarmDelay[fault_id] = new AlarmDelay(this);
    mIO111AlarmDelayCheckFlag[fault_id] = false;
  }

}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
IO111Module::~IO111Module()
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *****************************************************************************/
void IO111Module::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
    // misc.
    case SP_IO111_INSTALLED:
      mpIo111Installed.Attach(pSubject);
      break;
    case SP_IO111_SYSTEM_ALARM_RESET_EVENT:
      mpSystemAlarmResetEvent.Attach(pSubject);
      break;
    case SP_IO111_MODULE_ALARM_RESET_EVENT:
      mpModuleAlarmResetEvent.Attach(pSubject);
      break;

    // measured values
    case SP_IO111_TEMPERATURE_SUPPORT_BEARING:
      mpTemperatureSupportBearing.Attach(pSubject);
      break;
    case SP_IO111_TEMPERATURE_MAIN_BEARING:
      mpTemperatureMainBearing.Attach(pSubject);
      break;
    case SP_IO111_TEMPERATURE_PT100:
      mpTemperaturePT100.Attach(pSubject);
      break;
    case SP_IO111_TEMPERATURE_PT1000:
      mpTemperaturePT1000.Attach(pSubject);
      break;
    case SP_IO111_TEMPERATURE:
      mpTemperature.Attach(pSubject);
      break;
    case SP_IO111_RESISTANCE:
      mpInsulationResistance.Attach(pSubject);
      break;
    case SP_IO111_VIBRATION:
      mpVibration.Attach(pSubject);
      break;
    case SP_IO111_WATER_IN_OIL:
      mpWaterInOil.Attach(pSubject);
      break;
    case SP_IO111_MOISTURE_SWITCH:
      mpMoistureSwitch.Attach(pSubject);
      break;
    case SP_IO111_THERMAL_SWITCH:
      mpThermalSwitch.Attach(pSubject);
      break;

    case SP_IO111_IO113_MOTOR_SPEED:
      mpSpeed.Attach(pSubject);
      break;
    case SP_IO111_IO11X_UNIT_TYPE:
      mpUnitType.Attach(pSubject);
      break;

    // Alarms
    case SP_IO111_GENI_COMM_FAULT_OBJ :
      mpIO111AlarmDelay[IO111_FAULT_OBJ_GENI_COMM]->SetSubjectPointer(id, pSubject);
      break;
    case SP_IO111_ALARM_FAULT_OBJ :
      mpIO111AlarmObj.Attach(pSubject);
      mpIO111AlarmDelay[IO111_FAULT_OBJ_ALARM]->SetSubjectPointer(id, pSubject);
      break;
    case SP_IO111_WARNING_FAULT_OBJ :
      mpIO111WarningObj.Attach(pSubject);
      mpIO111AlarmDelay[IO111_FAULT_OBJ_WARNING]->SetSubjectPointer(id, pSubject);
      break;

    case SP_IO111_IO111_DEVICE_STATUS:
      mpIO111DeviceStatus.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void IO111Module::Update(Subject* pSubject)
{
  mpIo111Installed.Update(pSubject);
  mpSystemAlarmResetEvent.Update(pSubject);
  mpModuleAlarmResetEvent.Update(pSubject);

  for (int fault_id = FIRST_IO111_FAULT_OBJ; fault_id < NO_OF_IO111_FAULT_OBJ; fault_id++)
  {
    if (pSubject == mpIO111AlarmDelay[fault_id])
    {
      mIO111AlarmDelayCheckFlag[fault_id] = true;
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void IO111Module::ConnectToSubjects()
{
  mpIo111Installed->Subscribe(this);
  mpSystemAlarmResetEvent->Subscribe(this);
  mpModuleAlarmResetEvent->Subscribe(this);

  for (int fault_id = FIRST_IO111_FAULT_OBJ; fault_id < NO_OF_IO111_FAULT_OBJ; fault_id++)
  {
    mpIO111AlarmDelay[fault_id]->ConnectToSubjects();
  }

}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void IO111Module::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_IO111_FAULT_OBJ; fault_id < NO_OF_IO111_FAULT_OBJ; fault_id++)
  {
    mpIO111AlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}


/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void IO111Module::InitSubTask()
{
  mSkipRunCounter = LAST_IO111 - mModuleNo; // To distribute task load for the periodic task
  mExistingAlarmCode = ALARM_ID_NO_ALARM;
  mExistingWarningCode = ALARM_ID_NO_ALARM;
  SetIO111DataAvailability(DP_NEVER_AVAILABLE);

  // do io module config determination
  mpIo111Installed.SetUpdated();

  for (int fault_id = FIRST_IO111_FAULT_OBJ; fault_id < NO_OF_IO111_FAULT_OBJ; fault_id++)
  {
    mpIO111AlarmDelay[fault_id]->InitAlarmDelay();
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void IO111Module::RunSubTask()
{
  ALARM_ID_TYPE new_alarm_code;
  ALARM_ID_TYPE new_warning_code;

  mSkipRunCounter++;
  if (mSkipRunCounter < 10)
  {
    // Just run this subtask every 10th time i.e. every 100 ms instead of every 10 ms.
    // This makes a significant reduction of the CPU load, due to the service of 50 alarms. (25 alarms and 2 instanses)
    return;
  }
  mSkipRunCounter = 0;

  // Check for setting of alarms
  for (int fault_id = FIRST_IO111_FAULT_OBJ; fault_id < NO_OF_IO111_FAULT_OBJ; fault_id++)
  {
    if (mIO111AlarmDelayCheckFlag[fault_id] == true)
    {
      mIO111AlarmDelayCheckFlag[fault_id] = false;
      mpIO111AlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  // Reset alarms
  if (mpSystemAlarmResetEvent.IsUpdated() || mpModuleAlarmResetEvent.IsUpdated())
  {
    if (mpIo111Installed->GetValue() == true)
    {
      mpGeniSlaveIf->IO111AlarmReset(mModuleNo);
    }
  }

  // handle change of installation
  if (mpIo111Installed.IsUpdated())
  {
    if (mpIo111Installed->GetValue() == true)
    {
      // Insert in auto poll list
      GeniSlaveIf::GetInstance()->ConnectIO111(mModuleNo);
    }
    else
    {
      // Remove from auto poll list
      GeniSlaveIf::GetInstance()->DisconnectIO111(mModuleNo);
      // The module is not present - make certain that all errors and data from the module is cleared
      HandleIO111Alarm(ALARM_ID_NO_ALARM);
      HandleIO111Warning(ALARM_ID_NO_ALARM);
      SetIO111DataAvailability(DP_NEVER_AVAILABLE);
      mpIO111AlarmDelay[IO111_FAULT_OBJ_GENI_COMM]->ResetFault();
      mpIO111DeviceStatus->SetValue(IO_DEVICE_STATUS_NOT_PRESENT);
    }
  }

  // module expected?
  if (mpIo111Installed->GetValue() == true)
  {
    // handle alarms and status
    if (mpGeniSlaveIf->GetIO111AlarmCode(mModuleNo, &new_alarm_code) &&
        mpGeniSlaveIf->GetIO111WarningCode(mModuleNo, &new_warning_code))
    {
      mpIO111AlarmDelay[IO111_FAULT_OBJ_GENI_COMM]->ResetFault();
      HandleIO111Alarm(new_alarm_code);
      HandleIO111Warning(new_warning_code);
      HandleIO111MeasuredValues();

      if (new_alarm_code == 0 && new_warning_code == 0)
      {
        mpIO111DeviceStatus->SetValue(IO_DEVICE_STATUS_PRESENT_AND_OK);
      }
      else
      {
        mpIO111DeviceStatus->SetValue(IO_DEVICE_STATUS_PRESENT_WITH_ERROR);
      }
    }
    else
    {
      // we were unable to get the alarm code from GENI, set alarm and set data not available
      if (mpIO111AlarmDelay[IO111_FAULT_OBJ_GENI_COMM]->GetFault() != true)
      {
        HandleIO111Alarm(ALARM_ID_NO_ALARM);
        HandleIO111Warning(ALARM_ID_NO_ALARM);
        #ifndef __PC__
        mpIO111AlarmDelay[IO111_FAULT_OBJ_GENI_COMM]->SetFault();
        SetIO111DataAvailability(DP_NOT_AVAILABLE);
        mpIO111DeviceStatus->SetValue(IO_DEVICE_STATUS_NO_COMMUNICATION);
        #endif
      }
    }
  }

  // Update alarms
  for (int fault_id = FIRST_IO111_FAULT_OBJ; fault_id < NO_OF_IO111_FAULT_OBJ; fault_id++)
  {
    mpIO111AlarmDelay[fault_id]->UpdateAlarmDataPoint();
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
 * Function - HandleIO111MeasuredValues
 * DESCRIPTION:
 ****************************************************************************/
void IO111Module::HandleIO111MeasuredValues()
{
  bool bitvalue;
  float value;
  U8 u8value;
  U16 u16value;

  if (mpGeniSlaveIf->GetIO111TemperatureSupportBearing(mModuleNo, &value))
  {
    mpTemperatureSupportBearing->SetValue(value);
  }
  if (mpGeniSlaveIf->GetIO111TemperatureMainBearing(mModuleNo, &value))
  {
    mpTemperatureMainBearing->SetValue(value);
  }
  if (mpGeniSlaveIf->GetIO111TemperaturePT1000(mModuleNo, &value))
  {
    mpTemperaturePT1000->SetValue(value);
    mpTemperature->SetValue(value);
  }
// If no PT1000 a PT100 could be looked for by including code below
// However a PT100 value must come from a SM111 and first version of Midrange
// does not support SM111. Thus the code is commented out until it should be used.
//  else if (mpGeniSlaveIf->GetIO111TemperaturePT100(mModuleNo, &value))
//  {
////    mpTemperaturePT1000->SetValue(value);
//    mpTemperature->SetValue(value);
//  }
// end
  if (mpGeniSlaveIf->GetIO111InsulationResistance(mModuleNo, &value))
  {
    mpInsulationResistance->SetValue(value);
  }
  if (mpGeniSlaveIf->GetIO111Vibration(mModuleNo, &value))
  {
    mpVibration->SetValue(value);
  }
  if (mpGeniSlaveIf->GetIO111WaterInOil(mModuleNo, &value))
  {
    mpWaterInOil->SetValue(value);
  }
  if (mpGeniSlaveIf->GetIO111MoistureSwitch(mModuleNo, &bitvalue))
  {
    mpMoistureSwitch->SetValue(bitvalue);
  }
  if (mpGeniSlaveIf->GetIO111ThermalSwitch(mModuleNo, &bitvalue))
  {
    mpThermalSwitch->SetValue(bitvalue);
  }

  if (mpGeniSlaveIf->GetIO11xUnitType(mModuleNo, &u8value))
  {
    IO11X_UNIT_TYPE_TYPE unit_type = (IO11X_UNIT_TYPE_TYPE)(u8value);

    mpUnitType->SetValue(unit_type);

    if (unit_type == IO11X_UNIT_TYPE_IO111_WITH_3_WIRE_SENSOR 
      || unit_type == IO11X_UNIT_TYPE_IO111_WITH_POWERLINE_SENSOR
      || unit_type == IO11X_UNIT_TYPE_IO111_WITH_SM111)
    {
      // IO111 does not have speed.!
      mpSpeed->SetQuality(DP_NEVER_AVAILABLE);
    }
    else if (unit_type == IO11X_UNIT_TYPE_IO113_WITH_PUMP_C_AND_SM111_V2
      || unit_type == IO11X_UNIT_TYPE_IO113_WITH_PUMP_A_AND_SENSORS
      || unit_type == IO11X_UNIT_TYPE_IO113_WITH_PUMP_B_AND_SM111_V2
      || unit_type == IO11X_UNIT_TYPE_IO113_WITH_PUMP_D_AND_SENSORS
      || unit_type == IO11X_UNIT_TYPE_IO113_WITH_MIXER_C_AND_SM111_V2
      || unit_type == IO11X_UNIT_TYPE_IO113_WITH_MIXER_A_AND_SENSORS
      || unit_type == IO11X_UNIT_TYPE_IO113_WITH_MIXER_D_AND_SENSORS)
    {
      // IO113 may have pump or mixer speed.!
      if(mpGeniSlaveIf->GetIO113Speed(mModuleNo, &u16value))
      {
        mpSpeed->SetValue(u16value);
      }
      else
      {
        // Data not available.
        mpSpeed->SetQuality(DP_NOT_AVAILABLE);
      }
    }
    else
    {
      // The unit type is unknown, what to do about it?
      mpUnitType->SetQuality(DP_NEVER_AVAILABLE);
      mpSpeed->SetQuality(DP_NEVER_AVAILABLE);
    }
  }


}

/*****************************************************************************
 * Function - HandleIO111Alarm
 * DESCRIPTION:
 ****************************************************************************/
void IO111Module::HandleIO111Alarm(ALARM_ID_TYPE new_code)
{
  if (mExistingAlarmCode != new_code)
  {
    if (new_code == ALARM_ID_NO_ALARM || mExistingAlarmCode != ALARM_ID_NO_ALARM)
    {
      // The existing code is no longer reported from slave - it can be cleared
      mpIO111AlarmDelay[IO111_FAULT_OBJ_ALARM]->ResetFault();
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
      mpIO111AlarmObj->SetValue(cu361_code);
      mpIO111AlarmDelay[IO111_FAULT_OBJ_ALARM]->SetFault();
      mExistingAlarmCode = new_code;
    }
  }
}

/*****************************************************************************
 * Function - HandleIO111Warning
 * DESCRIPTION:
 ****************************************************************************/
void IO111Module::HandleIO111Warning(ALARM_ID_TYPE new_code)
{
  if (mExistingWarningCode != new_code)
  {
    if (new_code == ALARM_ID_NO_ALARM || mExistingWarningCode != ALARM_ID_NO_ALARM)
    {
      // The existing code is no longer reported from slave - it can be cleared
      mpIO111AlarmDelay[IO111_FAULT_OBJ_WARNING]->ResetFault();
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
      mpIO111WarningObj->SetValue(cu361_code);
      mpIO111AlarmDelay[IO111_FAULT_OBJ_WARNING]->SetFault();
      mExistingWarningCode = new_code;
    }
  }
}

/*****************************************************************************
 * Function - SetIO111DataAvailability
 * DESCRIPTION:
 ****************************************************************************/
void IO111Module::SetIO111DataAvailability(DP_QUALITY_TYPE quality)
{
  mpTemperatureSupportBearing->SetQuality(quality);
  mpTemperatureMainBearing->SetQuality(quality);
  mpTemperaturePT100->SetQuality(quality);
  mpTemperaturePT1000->SetQuality(quality);
  mpTemperature->SetQuality(quality);
  mpInsulationResistance->SetQuality(quality);
  mpVibration->SetQuality(quality);
  mpWaterInOil->SetQuality(quality);
  mpMoistureSwitch->SetQuality(quality);
  mpThermalSwitch->SetQuality(quality);

  mpUnitType->SetQuality(quality);
  mpSpeed->SetQuality(quality);
}


/*****************************************************************************
 * Function - ConfigReceived
 * DESCRIPTION: Empty function to satisfy virtual function in base class.
 ****************************************************************************/
void IO111Module::ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType)
{
}

