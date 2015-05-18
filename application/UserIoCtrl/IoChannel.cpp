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
/* CLASS NAME       : IoChannel                                             */
/*                                                                          */
/* FILE NAME        : IoChannel.h                                           */
/*                                                                          */
/* CREATED DATE     : 15-12-2008  (dd-mm-yyyy)                              */
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
#include "IoChannel.h"

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
/*****************************************************************************
  DEFINES
 *****************************************************************************/

const SUBJECT_ID_TYPE Io111Pump1AiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_1_TEMPERATURE,
  SUBJECT_ID_IO111_PUMP_1_TEMPERATURE_MAIN_BEARING,
  SUBJECT_ID_IO111_PUMP_1_TEMPERATURE_SUPPORT_BEARING,
  SUBJECT_ID_IO111_PUMP_1_TEMPERATURE_PT100,
  SUBJECT_ID_IO111_PUMP_1_TEMPERATURE_PT1000,
  SUBJECT_ID_IO111_PUMP_1_VIBRATION,
  SUBJECT_ID_IO111_PUMP_1_WATER_IN_OIL,
  SUBJECT_ID_IO111_PUMP_1_RESISTANCE
};

const SUBJECT_ID_TYPE Io111Pump2AiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_2_TEMPERATURE,
  SUBJECT_ID_IO111_PUMP_2_TEMPERATURE_MAIN_BEARING,
  SUBJECT_ID_IO111_PUMP_2_TEMPERATURE_SUPPORT_BEARING,
  SUBJECT_ID_IO111_PUMP_2_TEMPERATURE_PT100,
  SUBJECT_ID_IO111_PUMP_2_TEMPERATURE_PT1000,
  SUBJECT_ID_IO111_PUMP_2_VIBRATION,
  SUBJECT_ID_IO111_PUMP_2_WATER_IN_OIL,
  SUBJECT_ID_IO111_PUMP_2_RESISTANCE
};

const SUBJECT_ID_TYPE Io111Pump3AiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_3_TEMPERATURE,
  SUBJECT_ID_IO111_PUMP_3_TEMPERATURE_MAIN_BEARING,
  SUBJECT_ID_IO111_PUMP_3_TEMPERATURE_SUPPORT_BEARING,
  SUBJECT_ID_IO111_PUMP_3_TEMPERATURE_PT100,
  SUBJECT_ID_IO111_PUMP_3_TEMPERATURE_PT1000,
  SUBJECT_ID_IO111_PUMP_3_VIBRATION,
  SUBJECT_ID_IO111_PUMP_3_WATER_IN_OIL,
  SUBJECT_ID_IO111_PUMP_3_RESISTANCE
};

const SUBJECT_ID_TYPE Io111Pump4AiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_4_TEMPERATURE,
  SUBJECT_ID_IO111_PUMP_4_TEMPERATURE_MAIN_BEARING,
  SUBJECT_ID_IO111_PUMP_4_TEMPERATURE_SUPPORT_BEARING,
  SUBJECT_ID_IO111_PUMP_4_TEMPERATURE_PT100,
  SUBJECT_ID_IO111_PUMP_4_TEMPERATURE_PT1000,
  SUBJECT_ID_IO111_PUMP_4_VIBRATION,
  SUBJECT_ID_IO111_PUMP_4_WATER_IN_OIL,
  SUBJECT_ID_IO111_PUMP_4_RESISTANCE
};

const SUBJECT_ID_TYPE Io111Pump5AiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_5_TEMPERATURE,
  SUBJECT_ID_IO111_PUMP_5_TEMPERATURE_MAIN_BEARING,
  SUBJECT_ID_IO111_PUMP_5_TEMPERATURE_SUPPORT_BEARING,
  SUBJECT_ID_IO111_PUMP_5_TEMPERATURE_PT100,
  SUBJECT_ID_IO111_PUMP_5_TEMPERATURE_PT1000,
  SUBJECT_ID_IO111_PUMP_5_VIBRATION,
  SUBJECT_ID_IO111_PUMP_5_WATER_IN_OIL,
  SUBJECT_ID_IO111_PUMP_5_RESISTANCE
};

const SUBJECT_ID_TYPE Io111Pump6AiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_6_TEMPERATURE,
  SUBJECT_ID_IO111_PUMP_6_TEMPERATURE_MAIN_BEARING,
  SUBJECT_ID_IO111_PUMP_6_TEMPERATURE_SUPPORT_BEARING,
  SUBJECT_ID_IO111_PUMP_6_TEMPERATURE_PT100,
  SUBJECT_ID_IO111_PUMP_6_TEMPERATURE_PT1000,
  SUBJECT_ID_IO111_PUMP_6_VIBRATION,
  SUBJECT_ID_IO111_PUMP_6_WATER_IN_OIL,
  SUBJECT_ID_IO111_PUMP_6_RESISTANCE
};

const SUBJECT_ID_TYPE Io111Pump1DiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_1_MOISTURE,
  SUBJECT_ID_IO111_PUMP_1_THERMAL_SWITCH
};

const SUBJECT_ID_TYPE Io111Pump2DiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_2_MOISTURE,
  SUBJECT_ID_IO111_PUMP_2_THERMAL_SWITCH
};

const SUBJECT_ID_TYPE Io111Pump3DiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_3_MOISTURE,
  SUBJECT_ID_IO111_PUMP_3_THERMAL_SWITCH
};

const SUBJECT_ID_TYPE Io111Pump4DiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_4_MOISTURE,
  SUBJECT_ID_IO111_PUMP_4_THERMAL_SWITCH
};

const SUBJECT_ID_TYPE Io111Pump5DiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_5_MOISTURE,
  SUBJECT_ID_IO111_PUMP_5_THERMAL_SWITCH
};

const SUBJECT_ID_TYPE Io111Pump6DiSubjects[] =
{
  SUBJECT_ID_IO111_PUMP_6_MOISTURE,
  SUBJECT_ID_IO111_PUMP_6_THERMAL_SWITCH
};

const SUBJECT_ID_TYPE OperationModeActualSubjects[] =
{
  SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_1,
  SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_2,
  SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_3,
  SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_4,
  SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_5,
  SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_6
};

const SUBJECT_ID_TYPE AnaInCu361Subjects[] =
{
  SUBJECT_ID_ANA_IN_1_MEASURED_VALUE,
  SUBJECT_ID_ANA_IN_2_MEASURED_VALUE,
  SUBJECT_ID_ANA_IN_3_MEASURED_VALUE
};

const SUBJECT_ID_TYPE AnaInIo3511Subjects[] =
{
  SUBJECT_ID_ANA_IN_4_MEASURED_VALUE,
  SUBJECT_ID_ANA_IN_5_MEASURED_VALUE
};

const SUBJECT_ID_TYPE AnaInIo3512Subjects[] =
{
  SUBJECT_ID_ANA_IN_6_MEASURED_VALUE,
  SUBJECT_ID_ANA_IN_7_MEASURED_VALUE
};

const SUBJECT_ID_TYPE AnaInIo3513Subjects[] =
{
  SUBJECT_ID_ANA_IN_8_MEASURED_VALUE,
  SUBJECT_ID_ANA_IN_9_MEASURED_VALUE
};

const SUBJECT_ID_TYPE CombiAlarmSubjects[] =
{
  SUBJECT_ID_COMBI_ALARM_1_ALARM_OBJ,
  SUBJECT_ID_COMBI_ALARM_2_ALARM_OBJ,
  SUBJECT_ID_COMBI_ALARM_3_ALARM_OBJ,
  SUBJECT_ID_COMBI_ALARM_4_ALARM_OBJ
};

const SUBJECT_ID_TYPE DigInCu361Subjects[] =
{
  SUBJECT_ID_DIG_IN_1_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_2_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_3_LOGIC_STATE
};

const SUBJECT_ID_TYPE DigInIo3511Subjects[] =
{
  SUBJECT_ID_DIG_IN_4_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_5_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_6_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_7_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_8_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_9_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_10_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_11_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_12_LOGIC_STATE
};

const SUBJECT_ID_TYPE DigInIo3512Subjects[] =
{
  SUBJECT_ID_DIG_IN_13_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_14_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_15_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_16_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_17_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_18_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_19_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_20_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_21_LOGIC_STATE
};

const SUBJECT_ID_TYPE DigInIo3513Subjects[] =
{
  SUBJECT_ID_DIG_IN_22_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_23_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_24_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_25_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_26_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_27_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_28_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_29_LOGIC_STATE,
  SUBJECT_ID_DIG_IN_30_LOGIC_STATE
};

const SUBJECT_ID_TYPE UserIoSubjects[] =
{
  SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_1,
  SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_2,
  SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_3,
  SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_4,
  SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_5,
  SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_6,
  SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_7,
  SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_8
};



/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
// SW Timers
enum
{
  RESPONSE_TIME_TIMER,
  TIMER_FUNCTION_TIMER,
  REDUCE_UPDATE_RATE_TIMER
};

/********************************************************************
LIFECYCLE - Default constructor.
********************************************************************/
IoChannel::IoChannel() : BoolDataPoint()
{
  mValue = false;

  // IO channels are always available
  mQuality = DP_AVAILABLE;

  mResponseTimeTimeOut = false;
  mTimerFunctionTimeOut = false;
  mReduceUpdateRateTimeOut = false;
  mReduceUpdateRate = false;

  mpTimerObjList[RESPONSE_TIME_TIMER] = new SwTimer(10, S, true, false, this);
  mpTimerObjList[TIMER_FUNCTION_TIMER] = new SwTimer(10, S, true, false, this);
  mpTimerObjList[REDUCE_UPDATE_RATE_TIMER] = new SwTimer(250, MS, false, false, this);
}

/********************************************************************
LIFECYCLE - Destructor.
********************************************************************/
IoChannel::~IoChannel()
{
}


/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void IoChannel::InitSubTask()
{
  mRunRequestedFlag = true;
  ReqTaskTime(); // Start up
}


/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void IoChannel::RunSubTask()
{
  mRunRequestedFlag = false;

  if (mReduceUpdateRateTimeOut)
  {
    mReduceUpdateRateTimeOut = false;
    if (mReduceUpdateRate)
    {
      mpTimerObjList[REDUCE_UPDATE_RATE_TIMER]->RetriggerSwTimer();
    }
  }

  if (mTimerFunctionTimeOut)
  { // mTimerFunctionTimeOut is cleared in GetValueFromSource

    if (mpIoChannelConfig->GetSource() == CHANNEL_SOURCE_TIMER_FUNC)
    {
      U16 timerValue = (U16) mpTimerObjList[TIMER_FUNCTION_TIMER]->GetSwTimerPeriode();

      // set timer to toggle between high and low period
      if (timerValue == mpIoChannelConfig->GetTimerHighPeriod())
      {
        timerValue = mpIoChannelConfig->GetTimerLowPeriod();
      }
      else
      {
        timerValue = mpIoChannelConfig->GetTimerHighPeriod();
      }

      mpTimerObjList[TIMER_FUNCTION_TIMER]->SetSwTimerPeriod(timerValue, S, false);
      mpTimerObjList[TIMER_FUNCTION_TIMER]->RetriggerSwTimer();
    }
    else
    {
      mTimerFunctionTimeOut = false;
      mpTimerObjList[TIMER_FUNCTION_TIMER]->StopSwTimer();
    }
  }


  if (mpIoChannelConfig.IsUpdated())
  {
    if (mpIoChannelConfig->IsSourceChanged())
    {
      // observe the new source
      SwitchSource();
    }
  }

  bool output = GetValueFromSource();

  // output value has changed
  if (GetValue() != output)
  {
    CHANNEL_SOURCE_DATATYPE_TYPE type = mpIoChannelConfig->GetSourceType();

    // check for response time (high delay)
    if (type == CHANNEL_SOURCE_DATATYPE_CONSTANT
      || type == CHANNEL_SOURCE_DATATYPE_TIMER)
    {
      // set output at once for constant value and timer function
      SetValue(output);
    }
    else
    {

      if (IsReadyToChangeOutput())
      {
        SetValue(output);
      }

    }
  }
  else
  {
    mpTimerObjList[RESPONSE_TIME_TIMER]->StopSwTimer();
    mResponseTimeTimeOut = false;
  }

}


/********************************************************************
Function - GetIoChannelConfig
********************************************************************/
IoChannelConfig* IoChannel::GetIoChannelConfig()
{
  return mpIoChannelConfig.GetSubject();
}


/*****************************************************************************
* Observer::Update implementation
*****************************************************************************/
void IoChannel::Update(Subject* pSubject)
{
  if (pSubject == mpTimerObjList[RESPONSE_TIME_TIMER])
  {
    mResponseTimeTimeOut = true;
  }
  else if (pSubject == mpTimerObjList[TIMER_FUNCTION_TIMER])
  {
    mTimerFunctionTimeOut = true;
  }
  else if (pSubject == mpTimerObjList[REDUCE_UPDATE_RATE_TIMER])
  {
    mReduceUpdateRateTimeOut = true;
  }
  else if (mpIoChannelConfig.Update(pSubject)){}


  if (mRunRequestedFlag == false)
  {
    if (!mReduceUpdateRate
      || (mReduceUpdateRate && mReduceUpdateRateTimeOut))
    {
      mRunRequestedFlag = true;
      ReqTaskTime();
    }
  }

}

/*****************************************************************************
* Observer::SetSubjectPointer implementation
*****************************************************************************/
void IoChannel::SetSubjectPointer(int Id,Subject* pSubject)
{
  mpIoChannelConfig.Attach(pSubject);
  mpIoChannelConfig.SetUpdated();
}

/*****************************************************************************
* Observer::ConnectToSubjects implementation
*****************************************************************************/
void IoChannel::ConnectToSubjects(void)
{
  mpIoChannelConfig->Subscribe(this);
}

/*****************************************************************************
* Observer::SubscribtionCancelled implementation
*****************************************************************************/
void IoChannel::SubscribtionCancelled(Subject* pSubject)
{
  mpIoChannelConfig.Detach(pSubject);
}


/********************************************************************
Function - GetValueFromSource
********************************************************************/
bool IoChannel::GetValueFromSource()
{
  bool invert = mpIoChannelConfig->GetInverted();
  CHANNEL_SOURCE_DATATYPE_TYPE type = mpIoChannelConfig->GetSourceType();

  bool value = false;
  switch (type)
  {
  case CHANNEL_SOURCE_DATATYPE_AI:
    if (mpFloatSource != NULL)
    {
      FloatDataPoint* limit = mpIoChannelConfig->GetAiLimit();
      value = (mpFloatSource->GetValue() >= limit->GetValue());
      if (invert)
      {
        value = !value;
      }
    }
    break;
  case CHANNEL_SOURCE_DATATYPE_ALARM:
    if (mpAlarmSource != NULL)
    {
      value = (mpAlarmSource->GetAlarmPresent() != ALARM_STATE_READY);
      if (invert)
      {
        value = !value;
      }
    }
    break;
  case CHANNEL_SOURCE_DATATYPE_DI:
  case CHANNEL_SOURCE_DATATYPE_USERIO:
    if (mpBoolSource != NULL)
    {
      value = mpBoolSource->GetValue();
      if (invert)
      {
        value = !value;
      }
    }
    else if (mpDiEnumSource != NULL)
    {
      value = (mpDiEnumSource->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE);
      if (invert)
      {
        value = !value;
      }
    }
    break;
  case CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES:
    if (mpPumpOperationMode != NULL)
    {
      value = (mpPumpOperationMode->GetValue() == ACTUAL_OPERATION_MODE_STARTED);
      if (invert)
      {
        value = !value;
      }
    }
    else if (mpBoolSource != NULL)
    {
      value = mpBoolSource->GetValue();
      if (invert)
      {
        value = !value;
      }
    }
    break;
  case CHANNEL_SOURCE_DATATYPE_TIMER:
    if (mTimerFunctionTimeOut)
    {
      mTimerFunctionTimeOut = false;
      value = !GetValue();
    }
    else
    {
      value = GetValue();
    }
    break;
  case CHANNEL_SOURCE_DATATYPE_CONSTANT:
    value = GetValue();
    break;
  }

  return value;
}

/********************************************************************
Function - SwitchSource
********************************************************************/
void IoChannel::SwitchSource()
{
  if (mpDpSource.IsValid())
  {
    mpDpSource.UnsubscribeAndDetach(this);
  }

  //reset all timeout flags
  mResponseTimeTimeOut = false;
  mTimerFunctionTimeOut = false;
  mReduceUpdateRateTimeOut = false;

  mReduceUpdateRate = (mpIoChannelConfig->GetSourceType() == CHANNEL_SOURCE_DATATYPE_AI);

  if (mReduceUpdateRate)
  {
    mpTimerObjList[REDUCE_UPDATE_RATE_TIMER]->RetriggerSwTimer();
  }

  mpFloatSource = NULL;
  mpAlarmSource = NULL;
  mpBoolSource = NULL;
  mpDiEnumSource = NULL;
  mpPumpOperationMode = NULL;

  Subject* newAiSource = NULL;
  Subject* newAlarmSource = NULL;
  Subject* newDiSource = NULL;

  U8 index = mpIoChannelConfig->GetSourceIndex();

  switch (mpIoChannelConfig->GetSource())
  {
  case CHANNEL_SOURCE_AI_INDEX_CU_361:
    if (index >= 1 && index <= 3)
    {
      newAiSource = GetSubject(AnaInCu361Subjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_AI_INDEX_IO_351_1:
    if (index >= 1 && index <= 2)
    {
      newAiSource = GetSubject(AnaInIo3511Subjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_AI_INDEX_IO_351_2:
    if (index >= 1 && index <= 2)
    {
      newAiSource = GetSubject(AnaInIo3512Subjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_AI_INDEX_IO_351_3:
    if (index >= 1 && index <= 2)
    {
      newAiSource = GetSubject(AnaInIo3513Subjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_AI_INDEX_IO_111_1:
    if (index < 8)
    {
      newAiSource = GetSubject(Io111Pump1AiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_AI_INDEX_IO_111_2:
    if (index < 8)
    {
      newAiSource = GetSubject(Io111Pump2AiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_AI_INDEX_IO_111_3:
    if (index < 8)
    {
      newAiSource = GetSubject(Io111Pump3AiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_AI_INDEX_IO_111_4:
    if (index < 8)
    {
      newAiSource = GetSubject(Io111Pump4AiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_AI_INDEX_IO_111_5:
    if (index < 8)
    {
      newAiSource = GetSubject(Io111Pump5AiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_AI_INDEX_IO_111_6:
    if (index < 8)
    {
      newAiSource = GetSubject(Io111Pump6AiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_COMBI_ALARM:
    if (index >= 1 && index <= 4)
    {
      newAlarmSource = GetSubject(CombiAlarmSubjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_CONSTANT_VALUE:
    //use the channel itself as source
    SetValue(mpIoChannelConfig->GetConstantValue());
    mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_CONSTANT);
    break;
  case CHANNEL_SOURCE_DI_INDEX_CU_361:
    if (index >= 1 && index <= 3)
    {
      newDiSource = GetSubject(DigInCu361Subjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_DI_INDEX_IO_351_1:
    if (index >= 1 && index <= 9)
    {
      newDiSource = GetSubject(DigInIo3511Subjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_DI_INDEX_IO_351_2:
    if (index >= 1 && index <= 9)
    {
      newDiSource = GetSubject(DigInIo3512Subjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_DI_INDEX_IO_351_3:
    if (index >= 1 && index <= 9)
    {
      newDiSource = GetSubject(DigInIo3513Subjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_DI_INDEX_IO_111_1:
    if (index < 2)
    {
      newDiSource = GetSubject(Io111Pump1DiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_DI_INDEX_IO_111_2:
    if (index < 2)
    {
      newDiSource = GetSubject(Io111Pump2DiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_DI_INDEX_IO_111_3:
    if (index < 2)
    {
      newDiSource = GetSubject(Io111Pump3DiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_DI_INDEX_IO_111_4:
    if (index < 2)
    {
      newDiSource = GetSubject(Io111Pump4DiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_DI_INDEX_IO_111_5:
    if (index < 2)
    {
      newDiSource = GetSubject(Io111Pump5DiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_DI_INDEX_IO_111_6:
    if (index < 2)
    {
      newDiSource = GetSubject(Io111Pump6DiSubjects[index]);
    }
    break;
  case CHANNEL_SOURCE_USER_IO:
    if (index >= 1 && index <= 8)
    {
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_USERIO);
      newDiSource = GetSubject(UserIoSubjects[index - 1]);
    }
    break;
  case CHANNEL_SOURCE_SYSTEM_STATES:
    if (index <= 5)
    {// system states: pump X running
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);

      mpPumpOperationMode = dynamic_cast<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*>(GetSubject(OperationModeActualSubjects[index]));
      mpDpSource.Attach(mpPumpOperationMode);
    }
    else if(index == 6)
    {
      // system state: all pumps running
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_ALL_PUMPS_RUNNING));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 7)
    {
      // system state: any pump running
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_ANY_PUMP_RUNNING));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 8)
    {
      // system state: all pumps in alarm
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_ALL_PUMP_ALARM_FLAG));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 9)
    {
      // system state: any pump in alarm
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_ANY_PUMP_ALARM_FLAG));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 10)
    {
      // system state: pump 1 in alarm
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_PUMP_1_ALARM_FLAG));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 11)
    {
      // system state: pump 2 in alarm
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_PUMP_2_ALARM_FLAG));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 12)
    {
      // system state: pump 3 in alarm
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_PUMP_3_ALARM_FLAG));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 13)
    {
      // system state: pump 4 in alarm
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_PUMP_4_ALARM_FLAG));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 14)
    {
      // system state: pump 5 in alarm
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_PUMP_5_ALARM_FLAG));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 15)
    {
      // system state: pump 6 in alarm
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_PUMP_6_ALARM_FLAG));
      mpDpSource.Attach(mpBoolSource);
    }
    else if(index == 16)
    {
      // system state: interlocked
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES);
      mpBoolSource = dynamic_cast<BoolDataPoint*>(GetSubject(SUBJECT_ID_INTERLOCKED));
      mpDpSource.Attach(mpBoolSource);
    }

    break;
  case CHANNEL_SOURCE_TIMER_FUNC:
    mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_TIMER);
    mpTimerObjList[TIMER_FUNCTION_TIMER]->SetSwTimerPeriod(mpIoChannelConfig->GetTimerHighPeriod(), S, false);
    mpTimerObjList[TIMER_FUNCTION_TIMER]->RetriggerSwTimer();
    //always start with high period when switching to timer function
    SetValue(true);

    break;
  }

  if (newAiSource != NULL)
  {
    mpDpSource.Attach(newAiSource);
    mpFloatSource = dynamic_cast<FloatDataPoint*>(newAiSource);

    // ignore AI settings at start-up where AI have not been initialized (Quantity == Q_NO_UNIT)
    if (mpFloatSource->GetQuantity() != Q_NO_UNIT)
    {
      // setup limit range but NOT its value, to avoid overwrite of a value set from geni.
      // The value will be set from: GeniObjectInterface, IoChannelSlipPoint (edit on display)
      // or IoChannelConfig (when loaded from flash)
      mpIoChannelConfig->SetAiLimitRange(mpFloatSource);
    }

    mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_AI);
  }
  else if (newAlarmSource != NULL)
  {
    mpDpSource.Attach(newAlarmSource);
    mpAlarmSource = dynamic_cast<AlarmDataPoint*>(newAlarmSource);
    mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_ALARM);
  }
  else if (newDiSource != NULL)
  {
    mpDpSource.Attach(newDiSource);
    mpBoolSource = dynamic_cast<BoolDataPoint*>(newDiSource);
    if (mpBoolSource == NULL)
    {
      mpDiEnumSource = dynamic_cast<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*>(newDiSource);
    }

    if (mpIoChannelConfig->GetSource() != CHANNEL_SOURCE_USER_IO)
    {
      mpIoChannelConfig->SetSourceType(CHANNEL_SOURCE_DATATYPE_DI);
    }
  }
  else
  {
    //is either timer function, constant value or system state (already handled)
  }

  if (mpDpSource.IsValid())
  {
    mpDpSource.Subscribe(this);
  }

}

/********************************************************************
Function - IsReadyToChangeOutput
********************************************************************/
bool IoChannel::IsReadyToChangeOutput()
{
  U16 response_time = mpIoChannelConfig->GetResponseTime();

  if (response_time > 0 && !mResponseTimeTimeOut)
  {
    // postpone output change

    // start timer if not already started
    if (!mpTimerObjList[RESPONSE_TIME_TIMER]->GetSwTimerStatus())
    {
      mpTimerObjList[RESPONSE_TIME_TIMER]->SetSwTimerPeriod(response_time, S, false);
      mpTimerObjList[RESPONSE_TIME_TIMER]->RetriggerSwTimer();
    }

    return false;
  }

  mResponseTimeTimeOut = false;
  return true;
}


