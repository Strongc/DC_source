/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/*                                                                          */
/* CLASS NAME       : PcSimulatorService                                    */
/*                                                                          */
/* FILE NAME        : PcSimulatorService.cpp                                */
/*                                                                          */
/* CREATED DATE     : 30-07-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <factory.h>
#include <AlarmConfig.h>
#include <MpcUnits.h>
#include <Languages.h>
#include <AlarmDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "PcSimulatorService.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/

PcSimulatorService* PcSimulatorService::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function: GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
PcSimulatorService* PcSimulatorService::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new PcSimulatorService();
  }
  return mInstance;
}


/*****************************************************************************
 * Function - GetSimulatorValues
 * DESCRIPTION:
 *
 *****************************************************************************/
BSTR PcSimulatorService::GetSimulatorValues(void)
{
  static char str[130];
  char* p_str = str;

  int surface_level_q = (int) mpDpSurface->GetQuality();
  int high_level_q    = (int) mpDpHigh->GetQuality();
  int alarm_level_q   = (int) mpDpAlarm->GetQuality();
  int dryrun_level_q  = (int) mpDpDryrun->GetQuality();

  float surface_level = (surface_level_q == (int)DP_AVAILABLE) ? mpDpSurface->GetAsFloat() : 0.0f;
  float high_level    = mpDpHigh->GetAsFloat();
  float alarm_level   = mpDpAlarm->GetAsFloat();
  float dryrun_level  = mpDpDryrun->GetAsFloat();

  float pit_depth = mpDpPitDepth->GetAsFloat();
  float upper = mpDpUpper ->GetAsFloat();
  float lower = mpDpLower->GetAsFloat();
  float delta_vol = mpDpDelta->GetAsFloat();
  
  float volume = 0.0;

  if ((upper - lower) > 0)
  {
    volume = pit_depth * delta_vol / (upper - lower);
  }

  // max size of version+quality+levels+depth+volume+type+vfdFrequencies: 12+8+24+6+6+2 = 58 chars
  p_str += sprintf(p_str, "Version 0.4;%i;%i;%i;%i;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%i;",
    surface_level_q, high_level_q, alarm_level_q, dryrun_level_q,
    surface_level, high_level, alarm_level, dryrun_level,
    pit_depth, volume, mpDpPitLevelCtrlType->GetAsInt());

  // max size of float switch states: 10 chars
  for (int i=0; i<5; i++)
  {
    p_str += sprintf(p_str, "%i;", mpDpFswTypes[i]->GetAsInt());
  }

  // max size of pump states: 12 chars
  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    p_str += sprintf(p_str, "%i;", mpDpPumpOm[i]->GetAsInt());
  }

  // max size of vfd frequency: 30 chars
  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    if (mpDpVfdInstalled[i]->GetValue())
    {
      p_str += sprintf(p_str, "%0.1f;", mpDpVfdFrequency[i]->GetValue());
    }
    else
    {
      p_str += sprintf(p_str, "%0.1f;", -1.0f);
    }
  }


  return SysAllocStringByteLen((const char*) str, sizeof(str));
}


/*****************************************************************************
 * Function - GetAIValues
 * DESCRIPTION:
 *
 *****************************************************************************/
BSTR PcSimulatorService::GetAIValues(void)
{
  static char str[20*NO_OF_AI];
  char* p_str = str;

  MpcUnits* p_u = MpcUnits::GetInstance();
  Languages* p_l = Languages::GetInstance();

  for (int i = 0; i < NO_OF_AI; i++)
  {
    p_str += sprintf(p_str, "%i;%i;%0.2f %s;",
      (int)mpDpAiValues[i]->GetQuantity(), 
      mpDpAiFuncs[i]->GetAsInt(), 
      p_u->GetFromStandardToActualUnit(mpDpAiValues[i]->GetValue(), mpDpAiValues[i]->GetQuantity()),
      p_l->GetString(p_u->GetActualUnitString(mpDpAiValues[i]->GetQuantity()))
      );
  }

  return SysAllocStringByteLen((const char*) str, sizeof(str));
}


/*****************************************************************************
 * Function - GetDIValues
 * DESCRIPTION:
 *
 *****************************************************************************/
BSTR PcSimulatorService::GetDIValues(void)
{
  static char str[10*NO_OF_DI];
  char* p_str = str;

  for(int i=0; i<NO_OF_DI; i++)
  {
    p_str += sprintf( p_str, "%i;%i;%i;", 
      (mpDpDiLogics[i]->GetAsBool() ? 1 : 0),
      mpDpDiStates[i]->GetAsInt(), 
      mpDpDiFuncs[i]->GetAsInt());
  } 

  return SysAllocStringByteLen((const char*) str, sizeof(str));
}

/*****************************************************************************
 * Function - GetDOValues
 * DESCRIPTION:
 *
 *****************************************************************************/
BSTR PcSimulatorService::GetDOValues(void)
{
  static char str[10*NO_OF_DO];
  char* p_str = str;
  
  for (int i=0; i<NO_OF_DO; i++)
  {
    p_str += sprintf( p_str, "%i;%i;", 
      (U8) mpDpDoStates[i]->GetAsInt(), 
      (U8) mpDpDoFuncs[i]->GetAsInt());
  } 

  return SysAllocStringByteLen((const char*) str, sizeof(str));
}


/*****************************************************************************
* Function - GetIO111Values
* DESCRIPTION:
*
*****************************************************************************/
BSTR PcSimulatorService::GetIO111Values(int moduleNo)
{
  static char str[10*10];
  str[0] = '\0';
  
  int no = moduleNo;

  if (no >= 0 && no < 6)
  {
    sprintf( str, "%0.1f;%0.1f;%0.1f;%0.1f;%0.1f;%0.1f;%0.1f;%0.1f;%i;%i;",   
      mpDpIO111Temperature[no]->GetValue(),
      mpDpIO111TemperaturePt100[no]->GetValue(),
      mpDpIO111TemperaturePt1000[no]->GetValue(),
      mpDpIO111TemperatureMainBearing[no]->GetValue(),
      mpDpIO111TemperatureSupportBearing[no]->GetValue(),      
      mpDpIO111WaterInOil[no]->GetValue(),
      mpDpIO111Resistance[no]->GetValue(),
      mpDpIO111Vibration[no]->GetValue(),
      mpDpIO111Moisture[no]->GetAsInt(),
      mpDpIO111ThermalSwitch[no]->GetAsInt());
  }

  return SysAllocStringByteLen((const char*) str, sizeof(str));
  
}

/*****************************************************************************
* Function - SetDiValue
* DESCRIPTION:
*
*****************************************************************************/
int PcSimulatorService::SetDiValue(DiParameters* pParam)
{
  int index = pParam->index;
  bool isHigh = pParam->isHigh;
  
  if (index >= 0 && index < NO_OF_DI)
  {
    U32DataPoint* pDp_di_status;

    if (index < 3)
    {//IO-board
      pDp_di_status = mpDpCu361DiStatus;
    }
    else
    {//IO 351 module
      index -= 3;

      int module_no = (index / 9);

      index -= (module_no * 9);
      pDp_di_status = mpDpIo351DiStatus[module_no];
    }
     
    U32 val = pDp_di_status->GetValue();

    if (isHigh)
    {//SET_BIT_HIGH
      val |= (0x0001 << index);
    }
    else 
    {//SET_BIT_LOW
      val &= ~(0x0001 << index);
    }

    pDp_di_status->SetValue(val);
  }
   
  return 1;
}

/*****************************************************************************
* Function - SetAiValueInPercent
* DESCRIPTION:
*
*****************************************************************************/
int PcSimulatorService::SetAiValueInPercent(AiParameters* pParam)
{
  int index = pParam->index;
  float valueInPercent = pParam->value;

  if (index >= 0 && index < 3)
  {
    U8 offset_if_current = 0;
    float voltage_division_factor = 1;

    if (mpDpAiElec[index]->GetValue() == SENSOR_ELECTRIC_TYPE_0_10V)
    {
      offset_if_current = 0;
      voltage_division_factor = 10.0f/11.5712f;
    }
    else
    {
      offset_if_current = 1;
      voltage_division_factor = 20.0f/23.5470f;
    }
    
    mpDpAiSim[ (index * 2) + offset_if_current ]->SetValueAsPercent(valueInPercent * voltage_division_factor); 

    return 0;
  }
  else if (index >= 3 && index < NO_OF_AI)
  {
    mpDpIO351AI[index-3]->SetValueAsPercent(valueInPercent);

    return 0;
  }

  return 1;
}

/*****************************************************************************
* Function - SetAiValueInInternalUnit
* DESCRIPTION:
*
*****************************************************************************/
int PcSimulatorService::SetAiValueInInternalUnit(AiParameters* pParam)
{
  int index = pParam->index;
  float valueInInternalUnit = pParam->value;

  if (index >= 0 && index < NO_OF_AI)
  {
    float valueInPercent = 0.0f;

    float min = mpDpAiMin[index]->GetValue();
    float max = mpDpAiMax[index]->GetValue();
    float rangeInInternalUnit = max - min;
    
    if (rangeInInternalUnit > 0)
    {
      if (mpDpAiElec[index]->GetValue() == SENSOR_ELECTRIC_TYPE_4_20mA && index < 3)
      {
        valueInPercent = (80.0 * (valueInInternalUnit - min) / rangeInInternalUnit) + 20;
      }
      else 
      {
        valueInPercent = (100.0 * (valueInInternalUnit - min) / rangeInInternalUnit);
      }
    }

    pParam->value = valueInPercent;

    return SetAiValueInPercent(pParam);
  }

  return 1;
}

/*****************************************************************************
* Function - SetIO111Values
* DESCRIPTION:
* negative parameter values are ignored
*****************************************************************************/
int PcSimulatorService::SetIO111Values(Io111Parameters* pParam)
{
  int no = pParam->moduleNo;

  if (no >= 0 && no < 2)
  {
    if (pParam->temperature > 0)
    {
      mpDpIO111Temperature[no]->SetValue(pParam->temperature);
    }
    if (pParam->waterInOil > 0)
    {
      mpDpIO111WaterInOil[no]->SetValue(pParam->waterInOil);
    }
    if (pParam->resistance > 0)
    {
      mpDpIO111Resistance[no]->SetValue(pParam->resistance);
    }
    if (pParam->temperatureMainBearing > 0)
    {
      mpDpIO111TemperatureMainBearing[no]->SetValue(pParam->temperatureMainBearing);
    }
    if (pParam->temperaturePt100 > 0)
    {
      mpDpIO111TemperaturePt100[no]->SetValue(pParam->temperaturePt100);
    }
    if (pParam->temperaturePt1000 > 0)
    {
      mpDpIO111TemperaturePt1000[no]->SetValue(pParam->temperaturePt1000);
    }
    if (pParam->vibration > 0)
    {
      mpDpIO111Vibration[no]->SetValue(pParam->vibration);
    }
    if (pParam->temperatureSupportBearing > 0)
    {
      mpDpIO111TemperatureSupportBearing[no]->SetValue(pParam->temperatureSupportBearing);
    }
    if (pParam->moisture >= 0)
    {
      mpDpIO111Moisture[no]->SetValue( pParam->moisture > 0 );
    }
    if (pParam->thermalSwitch >= 0)
    {
      mpDpIO111ThermalSwitch[no]->SetValue( pParam->thermalSwitch > 0 );
    }

    return 0;
  }

  return 1;
}

/*****************************************************************************
* Function - SimulateAlarm
* DESCRIPTION:
*
*****************************************************************************/
int PcSimulatorService::SimulateAlarm(AlarmParameters* pParam)
{
  int subjectId = pParam->alarmDataPointId;
  bool errorPresent = pParam->alarmActive;
  bool warningPresent = pParam->warningActive;

  Subject* sub = GetSubject(subjectId);
  AlarmDataPoint* adp = dynamic_cast<AlarmDataPoint*>(sub); 

  if (adp != NULL)
  {
    adp->SimulateErrorPresent(errorPresent, warningPresent);
    return 0;
  }
  return 1;
}



/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
PcSimulatorService::PcSimulatorService()
{
  //initializing members for GetSimulatorValues
  mpDpPumpOm[0] = (EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*) GetSubject(SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_1);
  mpDpPumpOm[1] = (EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*) GetSubject(SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_2);
  mpDpPumpOm[2] = (EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*) GetSubject(SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_3);
  mpDpPumpOm[3] = (EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*) GetSubject(SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_4);
  mpDpPumpOm[4] = (EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*) GetSubject(SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_5);
  mpDpPumpOm[5] = (EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*) GetSubject(SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_6);

  mpDpMixerOm = (EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*) GetSubject(SUBJECT_ID_MIXER_OPERATING_MODE);
  mpDpSurface = ((FloatDataPoint*) GetSubject(SUBJECT_ID_SURFACE_LEVEL));
  mpDpHigh = ((AlarmConfig*) GetSubject(SUBJECT_ID_SYS_ALARM_HIGH_LEVEL_ALARM_CONF))->GetAlarmLimit();
  mpDpAlarm = ((AlarmConfig*) GetSubject(SUBJECT_ID_SYS_ALARM_LEVEL_ALARM_CONF))->GetAlarmLimit();
  mpDpDryrun = ((AlarmConfig*) GetSubject(SUBJECT_ID_SYS_ALARM_DRY_RUNNING_ALARM_CONF))->GetAlarmLimit();
  mpDpPitDepth = ((FloatDataPoint*) GetSubject(SUBJECT_ID_PIT_DEPTH));
  mpDpUpper = ((FloatDataPoint*) GetSubject(SUBJECT_ID_UPPER_MEASUREMENT_PIT_LEVEL));
  mpDpLower = ((FloatDataPoint*) GetSubject(SUBJECT_ID_LOWER_MEASUREMENT_PIT_LEVEL));
  mpDpDelta = ((FloatDataPoint*) GetSubject(SUBJECT_ID_VOLUME_FOR_FLOW_MEASUREMENT));
  mpDpPitLevelCtrlType = (EnumDataPoint<SENSOR_TYPE_TYPE>*) GetSubject(SUBJECT_ID_PIT_LEVEL_CTRL_TYPE);

  mpDpVfdInstalled[0] = ((BoolDataPoint*) GetSubject(SUBJECT_ID_VFD_1_INSTALLED));
  mpDpVfdInstalled[1] = ((BoolDataPoint*) GetSubject(SUBJECT_ID_VFD_2_INSTALLED));
  mpDpVfdInstalled[2] = ((BoolDataPoint*) GetSubject(SUBJECT_ID_VFD_3_INSTALLED));
  mpDpVfdInstalled[3] = ((BoolDataPoint*) GetSubject(SUBJECT_ID_VFD_4_INSTALLED));
  mpDpVfdInstalled[4] = ((BoolDataPoint*) GetSubject(SUBJECT_ID_VFD_5_INSTALLED));
  mpDpVfdInstalled[5] = ((BoolDataPoint*) GetSubject(SUBJECT_ID_VFD_6_INSTALLED));

  mpDpVfdFrequency[0] = ((FloatDataPoint*) GetSubject(SUBJECT_ID_VFD_1_OUTPUT_FREQUENCY));
  mpDpVfdFrequency[1] = ((FloatDataPoint*) GetSubject(SUBJECT_ID_VFD_2_OUTPUT_FREQUENCY));
  mpDpVfdFrequency[2] = ((FloatDataPoint*) GetSubject(SUBJECT_ID_VFD_3_OUTPUT_FREQUENCY));
  mpDpVfdFrequency[3] = ((FloatDataPoint*) GetSubject(SUBJECT_ID_VFD_4_OUTPUT_FREQUENCY));
  mpDpVfdFrequency[4] = ((FloatDataPoint*) GetSubject(SUBJECT_ID_VFD_5_OUTPUT_FREQUENCY));
  mpDpVfdFrequency[5] = ((FloatDataPoint*) GetSubject(SUBJECT_ID_VFD_6_OUTPUT_FREQUENCY));

  for (int i=0; i < 5; i++)
  {
    mpDpFswStates[i] = (EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*) GetSubject(SUBJECT_ID_DIG_IN_FUNC_STATE_FLOAT_SWITCH_1 + i);
    mpDpFswTypes[i] = (EnumDataPoint<FSW_TYPE>*) GetSubject(SUBJECT_ID_FLOAT_SWITCH_1_CNF + i);
  }

  //initializing members for GetAIValues
  for (int i = 0; i < NO_OF_AI - 2; i++)
  {
    mpDpAiValues[i] = ((FloatDataPoint*) GetSubject(SUBJECT_ID_ANA_IN_1_MEASURED_VALUE + i));
    mpDpAiFuncs[i] = (EnumDataPoint<MEASURED_VALUE_TYPE>*) GetSubject(SUBJECT_ID_ANA_IN_1_CONF_MEASURED_VALUE + i);
  }
  for (int i = 0; i < 2; i++)
  {
    mpDpAiValues[NO_OF_AI - 2 + i] = ((FloatDataPoint*) GetSubject(SUBJECT_ID_ANA_IN_8_MEASURED_VALUE + i));
    mpDpAiFuncs[NO_OF_AI - 2 + i] = (EnumDataPoint<MEASURED_VALUE_TYPE>*) GetSubject(SUBJECT_ID_ANA_IN_8_CONF_MEASURED_VALUE + i);
  }

  //initializing members for GetDIValues
  for (int i = 0; i < NO_OF_DI - 9; i++)
  {
    mpDpDiLogics[i] = (BoolDataPoint*) GetSubject(SUBJECT_ID_DIG_IN_1_CONF_LOGIC + i);
    mpDpDiStates[i] = (EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*) GetSubject(SUBJECT_ID_DIG_IN_1_STATE + i);
    mpDpDiFuncs[i]  = (EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*) GetSubject(SUBJECT_ID_DIG_IN_1_CONF_DIGITAL_INPUT_FUNC + i);
  }
  for (int i = 0; i < 9; i++)
  {
    mpDpDiLogics[NO_OF_DI - 9 + i] = (BoolDataPoint*) GetSubject(SUBJECT_ID_DIG_IN_22_CONF_LOGIC + i);
    mpDpDiStates[NO_OF_DI - 9 + i] = (EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*) GetSubject(SUBJECT_ID_DIG_IN_22_STATE + i);
    mpDpDiFuncs[NO_OF_DI - 9 + i]  = (EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*) GetSubject(SUBJECT_ID_DIG_IN_22_CONF_DIGITAL_INPUT_FUNC + i);
  }

  //initializing members for GetDOValues
  for (int i = 0; i < NO_OF_DO - 7; i++)
  {
    mpDpDoStates[i] = (BoolDataPoint*) GetSubject(SUBJECT_ID_DIG_OUT_1_STATE + i);
    mpDpDoFuncs[i]  = (EnumDataPoint<RELAY_FUNC_TYPE>*) GetSubject(SUBJECT_ID_DIG_OUT_1_CONF_RELAY_FUNC + i);
  }
  for (int i = 0; i < 7; i++)
  {
    mpDpDoStates[NO_OF_DO - 7 + i] = (BoolDataPoint*) GetSubject(SUBJECT_ID_DIG_OUT_17_STATE + i);
    mpDpDoFuncs[NO_OF_DO - 7 + i]  = (EnumDataPoint<RELAY_FUNC_TYPE>*) GetSubject(SUBJECT_ID_DIG_OUT_17_CONF_RELAY_FUNC + i);
  }

  //initializing members for SetAiValueInInternalUnit
  for (int i = 0; i < NO_OF_AI - 2; i++)
  { 
    mpDpAiElec[i] = (EnumDataPoint<SENSOR_ELECTRIC_TYPE>*) GetSubject(SUBJECT_ID_ANA_IN_1_CONF_SENSOR_ELECTRIC + i);
    mpDpAiMin[i] = (FloatDataPoint*) GetSubject(SUBJECT_ID_ANA_IN_1_CONF_SENSOR_MIN_VALUE + (i*2));
    mpDpAiMax[i] = (FloatDataPoint*) GetSubject(SUBJECT_ID_ANA_IN_1_CONF_SENSOR_MAX_VALUE + (i*2));
  }
  for (int i = 0; i < 2; i++)
  { 
    mpDpAiElec[NO_OF_AI - 2 + i] = (EnumDataPoint<SENSOR_ELECTRIC_TYPE>*) GetSubject(SUBJECT_ID_ANA_IN_8_CONF_SENSOR_ELECTRIC + i);
    mpDpAiMin[NO_OF_AI - 2 + i] = (FloatDataPoint*) GetSubject(SUBJECT_ID_ANA_IN_8_CONF_SENSOR_MIN_VALUE + i);
    mpDpAiMax[NO_OF_AI - 2 + i] = (FloatDataPoint*) GetSubject(SUBJECT_ID_ANA_IN_8_CONF_SENSOR_MAX_VALUE + i);
  }

  for (int i = 0; i < 4; i++)
  {
    mpDpIO351AI[i] = (FloatDataPoint*) GetSubject(SUBJECT_ID_ANA_IN_4_VALUE + i);
  }
  for (int i = 0; i < 2; i++)
  {
    mpDpIO351AI[4 + i] = (FloatDataPoint*) GetSubject(SUBJECT_ID_ANA_IN_8_VALUE + i);
  }

  for (int i = 0; i < 6; i++)
  {
    mpDpAiSim[i] = (U32DataPoint*) GetSubject(SUBJECT_ID_IOB_SIM_ANA_IN_0 + i);
  }

  //initializing members for SetDiValue
  mpDpCu361DiStatus = (U32DataPoint*) GetSubject(SUBJECT_ID_IOB_SIM_DIG_IN);

  mpDpIo351DiStatus[0] = (U32DataPoint*) GetSubject(SUBJECT_ID_IO351_IO_MODULE_1_DIG_IN_STATUS_BITS);
  mpDpIo351DiStatus[1] = (U32DataPoint*) GetSubject(SUBJECT_ID_IO351_IO_MODULE_2_DIG_IN_STATUS_BITS);
  mpDpIo351DiStatus[2] = (U32DataPoint*) GetSubject(SUBJECT_ID_IO351_IO_MODULE_3_DIG_IN_STATUS_BITS);


  //initializing members for IO111
  mpDpIO111Temperature[0] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_TEMPERATURE);
  mpDpIO111WaterInOil[0] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_WATER_IN_OIL);
  mpDpIO111Resistance[0] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_RESISTANCE);
  mpDpIO111Moisture[0] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_MOISTURE);
  mpDpIO111TemperatureMainBearing[0] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_TEMPERATURE_MAIN_BEARING);
  mpDpIO111TemperaturePt100[0] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_TEMPERATURE_PT100);
  mpDpIO111TemperaturePt1000[0] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_TEMPERATURE_PT1000);
  mpDpIO111Vibration[0] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_VIBRATION);
  mpDpIO111TemperatureSupportBearing[0]  = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_TEMPERATURE_SUPPORT_BEARING);
  mpDpIO111ThermalSwitch[0] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_1_THERMAL_SWITCH);

  mpDpIO111Temperature[1] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_TEMPERATURE);
  mpDpIO111WaterInOil[1] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_WATER_IN_OIL);
  mpDpIO111Resistance[1] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_RESISTANCE);
  mpDpIO111Moisture[1] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_MOISTURE);
  mpDpIO111TemperatureMainBearing[1] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_TEMPERATURE_MAIN_BEARING);
  mpDpIO111TemperaturePt100[1] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_TEMPERATURE_PT100);
  mpDpIO111TemperaturePt1000[1] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_TEMPERATURE_PT1000);
  mpDpIO111Vibration[1] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_VIBRATION);
  mpDpIO111TemperatureSupportBearing[1]  = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_TEMPERATURE_SUPPORT_BEARING);
  mpDpIO111ThermalSwitch[1] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_2_THERMAL_SWITCH);
   
  mpDpIO111Temperature[2] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_TEMPERATURE);
  mpDpIO111WaterInOil[2] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_WATER_IN_OIL);
  mpDpIO111Resistance[2] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_RESISTANCE);
  mpDpIO111Moisture[2] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_MOISTURE);
  mpDpIO111TemperatureMainBearing[2] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_TEMPERATURE_MAIN_BEARING);
  mpDpIO111TemperaturePt100[2] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_TEMPERATURE_PT100);
  mpDpIO111TemperaturePt1000[2] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_TEMPERATURE_PT1000);
  mpDpIO111Vibration[2] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_VIBRATION);
  mpDpIO111TemperatureSupportBearing[2]  = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_TEMPERATURE_SUPPORT_BEARING);
  mpDpIO111ThermalSwitch[2] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_3_THERMAL_SWITCH);

  mpDpIO111Temperature[3] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_TEMPERATURE);
  mpDpIO111WaterInOil[3] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_WATER_IN_OIL);
  mpDpIO111Resistance[3] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_RESISTANCE);
  mpDpIO111Moisture[3] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_MOISTURE);
  mpDpIO111TemperatureMainBearing[3] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_TEMPERATURE_MAIN_BEARING);
  mpDpIO111TemperaturePt100[3] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_TEMPERATURE_PT100);
  mpDpIO111TemperaturePt1000[3] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_TEMPERATURE_PT1000);
  mpDpIO111Vibration[3] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_VIBRATION);
  mpDpIO111TemperatureSupportBearing[3]  = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_TEMPERATURE_SUPPORT_BEARING);
  mpDpIO111ThermalSwitch[3] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_4_THERMAL_SWITCH);

  mpDpIO111Temperature[4] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_TEMPERATURE);
  mpDpIO111WaterInOil[4] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_WATER_IN_OIL);
  mpDpIO111Resistance[4] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_RESISTANCE);
  mpDpIO111Moisture[4] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_MOISTURE);
  mpDpIO111TemperatureMainBearing[4] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_TEMPERATURE_MAIN_BEARING);
  mpDpIO111TemperaturePt100[4] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_TEMPERATURE_PT100);
  mpDpIO111TemperaturePt1000[4] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_TEMPERATURE_PT1000);
  mpDpIO111Vibration[4] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_VIBRATION);
  mpDpIO111TemperatureSupportBearing[4]  = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_TEMPERATURE_SUPPORT_BEARING);
  mpDpIO111ThermalSwitch[4] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_5_THERMAL_SWITCH);

  mpDpIO111Temperature[5] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_TEMPERATURE);
  mpDpIO111WaterInOil[5] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_WATER_IN_OIL);
  mpDpIO111Resistance[5] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_RESISTANCE);
  mpDpIO111Moisture[5] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_MOISTURE);
  mpDpIO111TemperatureMainBearing[5] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_TEMPERATURE_MAIN_BEARING);
  mpDpIO111TemperaturePt100[5] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_TEMPERATURE_PT100);
  mpDpIO111TemperaturePt1000[5] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_TEMPERATURE_PT1000);
  mpDpIO111Vibration[5] = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_VIBRATION);
  mpDpIO111TemperatureSupportBearing[5]  = (FloatDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_TEMPERATURE_SUPPORT_BEARING);
  mpDpIO111ThermalSwitch[5] = (BoolDataPoint*) GetSubject(SUBJECT_ID_IO111_PUMP_6_THERMAL_SWITCH);

}