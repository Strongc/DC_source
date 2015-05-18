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
/* CLASS NAME       : IoChannelConfig                                       */
/*                                                                          */
/* FILE NAME        : IoChannelConfig.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 12-12-2008 (dd-mm-yyyy)                               */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Factory.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "IoChannelConfig.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
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
* DESCRIPTION: Initialises private data.
*****************************************************************************/
IoChannelConfig::IoChannelConfig()
{
  mSourceDataType = CHANNEL_SOURCE_DATATYPE_CONSTANT;
  mSource = CHANNEL_SOURCE_CONSTANT_VALUE;
  mSourceIndex = 1;
  mAiLimit = new FloatDataPoint();
  mAiLimit->SetMinValue(DEFAULT_AI_LIMIT_MIN);
  mAiLimit->SetMaxValue(DEFAULT_AI_LIMIT_MAX);
  mAiLimit->SetValue(DEFAULT_AI_LIMIT_VALUE);
  mInvert = false;
  mResponseTime = 1;
  mTimerHigh = 0;
  mTimerLow = 0;
  mSourceChanged = true;
  mChannelPrefix[0] = '\0';
}

/*****************************************************************************
* Function - Destructor
* DESCRIPTION: -
****************************************************************************/
IoChannelConfig::~IoChannelConfig()
{
  delete mAiLimit;
}


/*****************************************************************************
* Function - GetSource
* DESCRIPTION: Returns data type of source input
*****************************************************************************/
CHANNEL_SOURCE_DATATYPE_TYPE IoChannelConfig::GetSourceType()
{
  return mSourceDataType;
}


/*****************************************************************************
* Function - GetSource
* DESCRIPTION: Returns type of source input
*****************************************************************************/
CHANNEL_SOURCE_TYPE IoChannelConfig::GetSource()
{
  return mSource;
}

/*****************************************************************************
* Function - GetSourceIndex
* DESCRIPTION: Returns index of source input
*****************************************************************************/
U8 IoChannelConfig::GetSourceIndex()
{
  return mSourceIndex;
}

/*****************************************************************************
* Function - GetAiLimit
* DESCRIPTION: Returns limit to compare with for analog input
*****************************************************************************/
FloatDataPoint* IoChannelConfig::GetAiLimit()
{
  return mAiLimit;
}

/*****************************************************************************
* Function - GetInverted
* DESCRIPTION: Returns if output should be inverted (before high-delay-time)
*****************************************************************************/
bool IoChannelConfig::GetInverted()
{
  return mInvert;
}

/*****************************************************************************
* Function - GetResponseTime
* DESCRIPTION: Returns high-delay-time in seconds
*****************************************************************************/
U32 IoChannelConfig::GetResponseTime()
{
  return mResponseTime;
}

/*****************************************************************************
* Function - GetTimerHighPeriod
* DESCRIPTION: Returns high-period in seconds
*****************************************************************************/
U32 IoChannelConfig::GetTimerHighPeriod()
{
  return mTimerHigh;
}

/*****************************************************************************
* Function - GetTimerLowPeriod
* DESCRIPTION: Returns high-period in seconds
*****************************************************************************/
U32 IoChannelConfig::GetTimerLowPeriod()
{
  return mTimerLow;
}

/*****************************************************************************
* Function - GetConstantInput
* DESCRIPTION: Returns true if constant high, false if low
*****************************************************************************/
bool IoChannelConfig::GetConstantValue()
{
  return mSourceIndex > 0;
}

/*****************************************************************************
* Function - GetChannelPrefix
* DESCRIPTION: Returns channel prefix: "<n>: " where <n> is channel number
*****************************************************************************/
const char* IoChannelConfig::GetChannelPrefix(void)
{
  return mChannelPrefix;
}

/*****************************************************************************
* Function - SetSourceType
* DESCRIPTION: Sets data type of source
*****************************************************************************/
bool IoChannelConfig::SetSourceType(CHANNEL_SOURCE_DATATYPE_TYPE sourceDataType)
{
  bool notify = false;

  OS_EnterRegion();
  if (mSourceDataType != sourceDataType)
  {
    mSourceDataType = sourceDataType;
    notify = true;
    mSourceChanged = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetSource
* DESCRIPTION: Sets source
*****************************************************************************/
bool IoChannelConfig::SetSource(CHANNEL_SOURCE_TYPE source)
{
  bool notify = false;

  OS_EnterRegion();
  if (mSource != source)
  {
    mSource = source;
    notify = true;
    mSourceChanged = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetSourceIndex
* DESCRIPTION: Sets index in source (0-indexed for some sources, 1 for others!)
*****************************************************************************/
bool IoChannelConfig::SetSourceIndex(U8 index)
{
  bool notify = false;

  OS_EnterRegion();
  if (mSourceIndex != index && IndexIsValid(index))
  {
    mSourceIndex = index;
    notify = true;
    mSourceChanged = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetAiLimitRange
* DESCRIPTION: Sets limit min, max and quantity (used if source equals CHANNEL_SOURCE_AI_...)
*****************************************************************************/
bool IoChannelConfig::SetAiLimitRange(float min, float max, QUANTITY_TYPE quantity)
{
  bool notify = false;

  OS_EnterRegion();
  if ( mAiLimit->GetMinValue() != min 
    || mAiLimit->GetMaxValue() != max
    || mAiLimit->GetQuantity() != quantity)
  {
    mAiLimit->SetMinValue(min);
    mAiLimit->SetMaxValue(max);
    mAiLimit->SetQuantity(quantity);
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}


/*****************************************************************************
* Function - SetAiLimitRange
* DESCRIPTION: Sets limit min, max and quantity (used if source equals CHANNEL_SOURCE_AI_...)
*****************************************************************************/
bool IoChannelConfig::SetAiLimitRange(FloatDataPoint* pLimit)
{
  bool notify = false;

  OS_EnterRegion();
  if ( mAiLimit->GetMinValue() != pLimit->GetMinValue() 
    || mAiLimit->GetMaxValue() != pLimit->GetMaxValue()
    || mAiLimit->GetQuantity() != pLimit->GetQuantity())
  {
    mAiLimit->SetMinValue(pLimit->GetMinValue());
    mAiLimit->SetMaxValue(pLimit->GetMaxValue());
    mAiLimit->SetQuantity(pLimit->GetQuantity());

    mAiLimit->SetQuality(DP_AVAILABLE);
    
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}


/*****************************************************************************
* Function - SetAiLimitValue
* DESCRIPTION: Sets limit (used if source equals CHANNEL_SOURCE_AI_...)
*****************************************************************************/
bool IoChannelConfig::SetAiLimitValue(float limit)
{
  bool notify = false;

  OS_EnterRegion();
  if ( mAiLimit->GetValue() != limit)
  {
    mAiLimit->SetValue(limit);
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetInverted
* DESCRIPTION: Sets inverted property
*****************************************************************************/
bool IoChannelConfig::SetInverted(bool invert)
{
  bool notify = false;

  OS_EnterRegion();
  if (mInvert != invert)
  {
    mInvert = invert;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetResponseTime
* DESCRIPTION: Sets high-delay-time. 
* The time from input value is detected as high (rising edge) to channel output is set high
*****************************************************************************/
bool IoChannelConfig::SetResponseTime(U32 timeInSeconds)
{
  bool notify = false;

  OS_EnterRegion();
  if (mResponseTime != timeInSeconds)
  {
    mResponseTime = timeInSeconds;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetTimerHighPeriod
* DESCRIPTION: Sets time-high/on period (used if source equals CHANNEL_SOURCE_TIMER_FUNC)
*****************************************************************************/
bool IoChannelConfig::SetTimerHighPeriod(U32 timeInSeconds)
{
  bool notify = false;

  OS_EnterRegion();
  if (mTimerHigh != timeInSeconds)
  {
    mTimerHigh = timeInSeconds;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetTimerLowPeriod
* DESCRIPTION: Sets time-low/off period (used if source equals CHANNEL_SOURCE_TIMER_FUNC)
*****************************************************************************/
bool IoChannelConfig::SetTimerLowPeriod(U32 timeInSeconds)
{
  bool notify = false;

  OS_EnterRegion();
  if (mTimerLow != timeInSeconds)
  {
    mTimerLow = timeInSeconds;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetConstantValue
* DESCRIPTION: Sets constant value (used if source equals CHANNEL_SOURCE_CONSTANT_VALUE)
*****************************************************************************/
bool IoChannelConfig::SetConstantValue(bool value)
{
  bool notify = false;

  int constant_value_as_index = (value ? 1 : 0);

  OS_EnterRegion();
  if (mSource == CHANNEL_SOURCE_CONSTANT_VALUE 
    && mSourceIndex != constant_value_as_index)
  {
    mSourceIndex = constant_value_as_index;
    notify = true;
    mSourceChanged = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetChannelPrefix
* DESCRIPTION: Sets prefix string: "<n>: " where <n> is channel number
* (Called by UserIoSlipPoint)
*****************************************************************************/
void IoChannelConfig::SetChannelPrefix(char* pPrefix)
{
  strncpy(mChannelPrefix, pPrefix, MAX_CHANNEL_PREFIX_LEN);
  mChannelPrefix[MAX_CHANNEL_PREFIX_LEN - 1] = '\0';

}

/*****************************************************************************
* Function - CopyValues
* DESCRIPTION: 
*****************************************************************************/
bool IoChannelConfig::CopyValues(IoChannelConfig* pSource)
{
  bool notify = false;
  CHANNEL_SOURCE_DATATYPE_TYPE sourceDataType;
  CHANNEL_SOURCE_TYPE source;
  U8 sourceIndex;
  bool invert;
  U32 responseTime;
  U32 timerHigh;
  U32 timerLow;

  OS_EnterRegion();
  sourceDataType = pSource->GetSourceType();
  if (sourceDataType != mSourceDataType)
  {
    notify |= true;
    mSourceDataType = sourceDataType;
  }
  
  source = pSource->GetSource();
  if (source != mSource)
  {
    notify |= true;
    mSource = source;
  }

  sourceIndex = pSource->GetSourceIndex();
  if (sourceIndex != mSourceIndex)
  {
    notify |= true;
    mSourceIndex = sourceIndex;
  }

  notify |= mAiLimit->CopyValues(pSource->GetAiLimit());

  invert = pSource->GetInverted();
  if (invert != mInvert)
  {
    notify |= true;
    mInvert = invert;
  }

  responseTime = pSource->GetResponseTime();
  if (responseTime != mResponseTime)
  {
    notify |= true;
    mResponseTime = responseTime;
  }

  timerHigh = pSource->GetTimerHighPeriod();
  if (timerHigh != mTimerHigh)
  {
    notify |= true;
    mTimerHigh = timerHigh;
  }

  timerLow = pSource->GetTimerLowPeriod();
  if (timerLow != mTimerLow)
  {
    notify |= true;
    mTimerLow = timerLow;
  }

  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

	return notify;
}


/*****************************************************************************
* Function - IsSourceChanged
* DESCRIPTION: Return true if source or index is changed since last call to IsSourceChanged
*****************************************************************************/
bool IoChannelConfig::IsSourceChanged(void)
{
  bool ret = mSourceChanged;
  mSourceChanged = false;

  return ret;
}


/*****************************************************************************
* subject::GetFlashId implementation
*****************************************************************************/
FLASH_ID_TYPE IoChannelConfig::GetFlashId(void)
{
  return FLASH_ID_IO_CHANNEL_CONFIG;
}

/*****************************************************************************
* subject::SaveToFlash implementation
*****************************************************************************/
void IoChannelConfig::SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
{
  // Write a version code of the data type
  pWriter->WriteU8(2);

  switch (save)
  {
  case FLASH_SAVE_VALUE:
  case FLASH_SAVE_ALL:
    pWriter->WriteI16((I16) mSource);
    pWriter->WriteU8(mSourceIndex);
    pWriter->WriteFloat(mAiLimit->GetMinValue());
    pWriter->WriteFloat(mAiLimit->GetMaxValue());
    pWriter->WriteFloat(mAiLimit->GetValue());
    pWriter->WriteI32((I32) mAiLimit->GetQuantity());
    pWriter->WriteBool(mInvert);
    pWriter->WriteU32(mResponseTime);
    pWriter->WriteU32(mTimerHigh);
    pWriter->WriteU32(mTimerLow);
    break;
  }
}

/*****************************************************************************
* subject::LoadFromFlash implementation
*****************************************************************************/
void IoChannelConfig::LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
{
  const U8 version = pReader->ReadU8(0);

  if (version != 2)
  {
    // Nothing, unknown version code for the data type
  }
  else switch (savedAs)
  {
  case FLASH_SAVE_VALUE:
  case FLASH_SAVE_ALL:
    SetSource((CHANNEL_SOURCE_TYPE) pReader->ReadI16(mSource));
    SetSourceIndex(pReader->ReadU8(mSourceIndex));
    float f_min = pReader->ReadFloat(mAiLimit->GetMinValue());
    float f_max = pReader->ReadFloat(mAiLimit->GetMaxValue());
    float f_value = pReader->ReadFloat(mAiLimit->GetValue());
    QUANTITY_TYPE f_quantity = (QUANTITY_TYPE)pReader->ReadI32(mAiLimit->GetQuantity());
    SetAiLimitRange(f_min, f_max, f_quantity);
    SetAiLimitValue(f_value);
    SetInverted(pReader->ReadBool(mInvert));
    SetResponseTime(pReader->ReadU32(mResponseTime));
    SetTimerHighPeriod(pReader->ReadU32(mTimerHigh));
    SetTimerLowPeriod(pReader->ReadU32(mTimerLow));
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
*
*
*              PROTECTED FUNCTIONS
*                 - RARE USED -
*
****************************************************************************/

/*****************************************************************************
* Function - IndexIsValid
* DESCRIPTION: validates index according to source
*****************************************************************************/
bool IoChannelConfig::IndexIsValid(U8 index)
{
  switch (mSource)
  {
    case CHANNEL_SOURCE_AI_INDEX_CU_361:
      return (index >= 1 && index <= 3);
    case CHANNEL_SOURCE_AI_INDEX_IO_351_1:
    case CHANNEL_SOURCE_AI_INDEX_IO_351_2:
    case CHANNEL_SOURCE_AI_INDEX_IO_351_3:
      return (index >= 1 && index <= 2);
    case CHANNEL_SOURCE_AI_INDEX_IO_111_1:
    case CHANNEL_SOURCE_AI_INDEX_IO_111_2:
    case CHANNEL_SOURCE_AI_INDEX_IO_111_3:
    case CHANNEL_SOURCE_AI_INDEX_IO_111_4:
    case CHANNEL_SOURCE_AI_INDEX_IO_111_5:
    case CHANNEL_SOURCE_AI_INDEX_IO_111_6:
      return (index <= 7);
    case CHANNEL_SOURCE_DI_INDEX_CU_361:
      return (index >= 1 && index <= 3);
    case CHANNEL_SOURCE_DI_INDEX_IO_351_1:
    case CHANNEL_SOURCE_DI_INDEX_IO_351_2:
    case CHANNEL_SOURCE_DI_INDEX_IO_351_3:
      return (index >= 1 && index <= 9);
    case CHANNEL_SOURCE_DI_INDEX_IO_111_1:
    case CHANNEL_SOURCE_DI_INDEX_IO_111_2:
    case CHANNEL_SOURCE_DI_INDEX_IO_111_3:
    case CHANNEL_SOURCE_DI_INDEX_IO_111_4:
    case CHANNEL_SOURCE_DI_INDEX_IO_111_5:
    case CHANNEL_SOURCE_DI_INDEX_IO_111_6:
      return (index <= 1);
    case CHANNEL_SOURCE_COMBI_ALARM:
      return (index >= 1 && index <= 4);
    case CHANNEL_SOURCE_USER_IO:
      return (index >= FIRST_USER_IO && index <= LAST_USER_IO);
    case CHANNEL_SOURCE_CONSTANT_VALUE:
      return (index <= 1);
    case CHANNEL_SOURCE_SYSTEM_STATES:
      return (index <= 16);
    case CHANNEL_SOURCE_TIMER_FUNC:
      return (index == 0);
    default:
      {
        FatalErrorOccured("Unknown I/O-Ch source");
        return false;
      }
  }
}

  


