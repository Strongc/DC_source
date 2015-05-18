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
/* CLASS NAME       : AlarmConfig                                           */
/*                                                                          */
/* FILE NAME        : AlarmConfig.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 30-10-2007 (dd-mm-yyyy)                               */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <FloatDataPoint.h>
#include <BoolDataPoint.h>
#include <I32DataPoint.h>
#include <Factory.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <AlarmConfig.h>

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
*
*****************************************************************************/
AlarmConfig::AlarmConfig(SUBJECT_TYPE limitType, ALARM_CRITERIA_TYPE criteria)
{
  mCriteria = criteria;
  mSms1Enabled = false;
  mSms2Enabled = false;
  mSms3Enabled = false;
  mScadaEnabled = false;
  mCustomRelayForAlarmEnabled = false;
  mCustomRelayForWarningEnabled = false;
  mAlarmEnabled = false;
  mWarningEnabled = false;
  mAutoAck = false;
  mAlarmDelay = 0;
  mNotifyObserversEnabled = true;
  mLimitDatapointsChangedByAlarmConfig = false;

  switch ( limitType )
  {
    case SUBJECT_TYPE_BOOLDATAPOINT :
      mAlarmConfigType = AC_DIGITAL;
      break;
    case SUBJECT_TYPE_FLOATDATAPOINT :
      mAlarmConfigType = AC_FLOAT;
      break;
    case SUBJECT_TYPE_INTDATAPOINT :
      mAlarmConfigType = AC_INTEGER;
      break;
    default:
      FatalErrorOccured("AlarmConfig, unknown type");
      break;
  }

  if (limitType == SUBJECT_TYPE_FLOATDATAPOINT)
  {
    mpAlarmLimit = new FloatDataPoint();
    mpAlarmLimit->SetAsFloat(0.0);
    mpWarningLimit = new FloatDataPoint();
    mpWarningLimit->SetAsFloat(0.0);
  }
  else
  {
    mpAlarmLimit = new I32DataPoint();
    mpAlarmLimit->SetAsInt(0);
    mpWarningLimit = new I32DataPoint();
    mpWarningLimit->SetAsInt(0);
  }

  mpAlarmLimit->Subscribe(this);
  mpWarningLimit->Subscribe(this);
}

/*****************************************************************************
* Function - Destructor
* DESCRIPTION: -
*
****************************************************************************/
AlarmConfig::~AlarmConfig()
{
  delete mpAlarmLimit;
  delete mpWarningLimit;
}

/*****************************************************************************
* Function - Assignment operator
* DESCRIPTION: - param src The value to assign to this object.
*                return A reference to this object.
*
****************************************************************************/
AlarmConfig& AlarmConfig::operator=(const AlarmConfig& src)
{
  if (this != &src)
  {
    mNotifyObserversEnabled = false;
    mLimitDatapointsChangedByAlarmConfig = true;

    mSms1Enabled = src.mSms1Enabled;
    mSms2Enabled = src.mSms2Enabled;
    mSms3Enabled = src.mSms3Enabled;
    mScadaEnabled = src.mScadaEnabled;
    mCustomRelayForAlarmEnabled = src.mCustomRelayForAlarmEnabled;
    mCustomRelayForWarningEnabled = src.mCustomRelayForWarningEnabled;
    mAlarmEnabled = src.mAlarmEnabled;
    mWarningEnabled = src.mWarningEnabled;
    mAutoAck = src.mAutoAck;
    mAlarmDelay = src.mAlarmDelay;
    mCriteria = src.mCriteria;

    mpAlarmLimit->SetQuantity( src.mpAlarmLimit->GetQuantity() );
    mpWarningLimit->SetQuantity( src.mpWarningLimit->GetQuantity() );

    if (src.mpAlarmLimit->IsFloat())
    {
      SetMinLimit( src.mpAlarmLimit->GetMinAsFloat() );
      SetMaxLimit( src.mpAlarmLimit->GetMaxAsFloat() );
      mpAlarmLimit->SetAsFloat( src.mpAlarmLimit->GetAsFloat());
      mpWarningLimit->SetAsFloat( src.mpWarningLimit->GetAsFloat());
    }
    else
    {
      SetMinLimit( src.mpAlarmLimit->GetMinAsInt() );
      SetMaxLimit( src.mpAlarmLimit->GetMaxAsInt() );
      mpAlarmLimit->SetAsInt( src.mpAlarmLimit->GetAsInt());
      mpWarningLimit->SetAsInt( src.mpWarningLimit->GetAsInt());
    }

    mLimitDatapointsChangedByAlarmConfig = false;
    mNotifyObserversEnabled = true;

  }
  return *this;
}// =

/*****************************************************************************
* Function - GetSms1Enabled
* DESCRIPTION: Returns sms1 enabled.
*
*****************************************************************************/
bool AlarmConfig::GetSms1Enabled()
{
  return mSms1Enabled;
}

/*****************************************************************************
* Function - GetSms2Enabled
* DESCRIPTION: Returns sms2 enabled.
*
*****************************************************************************/
bool AlarmConfig::GetSms2Enabled()
{
  return mSms2Enabled;
}

/*****************************************************************************
* Function - GetSms3Enabled
* DESCRIPTION: Returns sms3 enabled.
*
*****************************************************************************/
bool AlarmConfig::GetSms3Enabled()
{
  return mSms3Enabled;
}
/*****************************************************************************
* Function - GetScadaEnabled
* DESCRIPTION: Returns scada enabled.
*
*****************************************************************************/
bool AlarmConfig::GetScadaEnabled()
{
  return mScadaEnabled;
}

/*****************************************************************************
* Function - GetCustomRelayForAlarm
* DESCRIPTION: Returns custom alarm relay for alarm is enabled.
*
*****************************************************************************/
bool AlarmConfig::GetCustomRelayForAlarm()
{
  return mCustomRelayForAlarmEnabled;
}

/*****************************************************************************
* Function - GetCustomRelayForWarning
* DESCRIPTION: Returns custom alarm relay for warning is enabled.
*
*****************************************************************************/
bool AlarmConfig::GetCustomRelayForWarning()
{
  return mCustomRelayForWarningEnabled;
}

/*****************************************************************************
* Function - GetAlarmEnabled
* DESCRIPTION: Returns alarm (limit) enabled.
*
*****************************************************************************/
bool AlarmConfig::GetAlarmEnabled()
{
  return mAlarmEnabled;
}

/*****************************************************************************
* Function - GetWarningEnabled
* DESCRIPTION: Returns warning (limit) enabled.
*
*****************************************************************************/
bool AlarmConfig::GetWarningEnabled()
{
  return mWarningEnabled;
}

/*****************************************************************************
* Function - GetAutoAcknowledge
* DESCRIPTION: Returns auto acknowledge enabled.
*
*****************************************************************************/
bool AlarmConfig::GetAutoAcknowledge()
{
  return mAutoAck;
}

/*****************************************************************************
* Function - GetAlarmDelay
* DESCRIPTION: Returns alarm delay.
*
*****************************************************************************/
float AlarmConfig::GetAlarmDelay()
{
  return mAlarmDelay;
}

/*****************************************************************************
* Function - GetLimitTypeIsFloat
* DESCRIPTION: Returns true if (alarm) limit is a float.
*
*****************************************************************************/
bool AlarmConfig::GetLimitTypeIsFloat()
{
  return mpAlarmLimit->IsFloat();
}



/*****************************************************************************
* Function - GetAlarmLimit
* DESCRIPTION: Returns alarm limit.
*
*****************************************************************************/
INumberDataPoint* AlarmConfig::GetAlarmLimit()
{
  return mpAlarmLimit;
}

/*****************************************************************************
* Function - GetWarningLimit
* DESCRIPTION: Returns warning limit.
*
*****************************************************************************/
INumberDataPoint* AlarmConfig::GetWarningLimit()
{
  return mpWarningLimit;
}


/*****************************************************************************
* Function - GetAlarmCriteria
* DESCRIPTION: Returns alarm criteria for alarm/warning detection.
*
*****************************************************************************/
ALARM_CRITERIA_TYPE AlarmConfig::GetAlarmCriteria()
{
  return mCriteria;
}


/*****************************************************************************
* Function - GetAlarmConfigType
* DESCRIPTION: Returns alarm config type
*
*****************************************************************************/
AC_TYPE AlarmConfig::GetAlarmConfigType()
{
  return mAlarmConfigType;
}


/*****************************************************************************
* Function - SetSms1Enabled
* DESCRIPTION: Sets sms1Enabled.
*
*****************************************************************************/
bool AlarmConfig::SetSms1Enabled(bool sms1Enabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mSms1Enabled != sms1Enabled)
  {
    mSms1Enabled = sms1Enabled;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetSms2Enabled
* DESCRIPTION: Sets sms2Enabled.
*
*****************************************************************************/
bool AlarmConfig::SetSms2Enabled(bool sms2Enabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mSms2Enabled != sms2Enabled)
  {
    mSms2Enabled = sms2Enabled;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetSms3Enabled
* DESCRIPTION: Sets sms3Enabled.
*
*****************************************************************************/
bool AlarmConfig::SetSms3Enabled(bool sms3Enabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mSms3Enabled != sms3Enabled)
  {
    mSms3Enabled = sms3Enabled;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetScadaEnabled
* DESCRIPTION: Sets ScadaEnabled.
*
*****************************************************************************/
bool AlarmConfig::SetScadaEnabled(bool scadaEnabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mScadaEnabled != scadaEnabled)
  {
    mScadaEnabled = scadaEnabled;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetCustomRelayForAlarm
* DESCRIPTION: Sets customRelayForAlarmEnabled.
*
*****************************************************************************/
bool AlarmConfig::SetCustomRelayForAlarm(bool customRelayForAlarmEnabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mCustomRelayForAlarmEnabled != customRelayForAlarmEnabled)
  {
    mCustomRelayForAlarmEnabled = customRelayForAlarmEnabled;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetCustomRelayForWarning
* DESCRIPTION: Sets customRelayForWarningEnabled.
*
*****************************************************************************/
bool AlarmConfig::SetCustomRelayForWarning(bool customRelayForWarningEnabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mCustomRelayForWarningEnabled != customRelayForWarningEnabled)
  {
    mCustomRelayForWarningEnabled = customRelayForWarningEnabled;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}


/*****************************************************************************
* Function - SetAlarmEnabled
* DESCRIPTION: Sets alarmEnabled.
*
*****************************************************************************/
bool AlarmConfig::SetAlarmEnabled(bool alarmEnabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mAlarmEnabled != alarmEnabled)
  {
    mAlarmEnabled = alarmEnabled;

    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetWarningEnabled
* DESCRIPTION: Sets warningEnabled.
*
*****************************************************************************/
bool AlarmConfig::SetWarningEnabled(bool warningEnabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mWarningEnabled != warningEnabled)
  {
    mWarningEnabled = warningEnabled;

    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetAutoAcknowledge
* DESCRIPTION: Sets autoAcknowledge enabled.
*
*****************************************************************************/
bool AlarmConfig::SetAutoAcknowledge(bool autoAcknowledge)
{
  bool notify = false;

  OS_EnterRegion();
  if (mAutoAck != autoAcknowledge)
  {
    mAutoAck = autoAcknowledge;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetAlarmDelay
* DESCRIPTION: Sets alarmDelay in s
*
*****************************************************************************/
bool AlarmConfig::SetAlarmDelay(float alarmDelay)
{
  bool notify = false;

  OS_EnterRegion();
  if (mAlarmDelay != alarmDelay)
  {
    mAlarmDelay = alarmDelay;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetAlarmConfigType
* DESCRIPTION: Sets the alarm config type
*
*****************************************************************************/
bool AlarmConfig::SetAlarmConfigType(AC_TYPE alarmType)
{
  bool notify = false;

  OS_EnterRegion();
  if (mAlarmConfigType != alarmType)
  {
    mAlarmConfigType = alarmType;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}


/*****************************************************************************
* Function - SetAlarmLimit
* DESCRIPTION: Sets alarmLimit.
*
*****************************************************************************/
bool AlarmConfig::SetAlarmLimit(float alarmLimit)
{
  bool notify = false;

  OS_EnterRegion();
  mLimitDatapointsChangedByAlarmConfig = true;
  if (mpAlarmLimit->GetAsFloat() != alarmLimit)
  {
    mpAlarmLimit->SetAsFloat(alarmLimit);
    notify = true;
  }
  mLimitDatapointsChangedByAlarmConfig = false;
  OS_LeaveRegion();

  if (notify && mNotifyObserversEnabled)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetAlarmLimit
* DESCRIPTION: Sets alarmLimit.
*
*****************************************************************************/
bool AlarmConfig::SetAlarmLimit(int alarmLimit)
{
  bool notify = false;

  OS_EnterRegion();
  mLimitDatapointsChangedByAlarmConfig = true;
  if (mpAlarmLimit->GetAsInt() != alarmLimit)
  {
    mpAlarmLimit->SetAsInt(alarmLimit);
    notify = true;
  }
  mLimitDatapointsChangedByAlarmConfig = false;
  OS_LeaveRegion();

  if (notify && mNotifyObserversEnabled)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}


/*****************************************************************************
* Function - SetWarningLimit
* DESCRIPTION: Sets warningLimit.
*
*****************************************************************************/
bool AlarmConfig::SetWarningLimit(float warningLimit)
{
  bool notify = false;

  OS_EnterRegion();
  mLimitDatapointsChangedByAlarmConfig = true;
  if (mpWarningLimit->GetAsFloat() != warningLimit)
  {
    mpWarningLimit->SetAsFloat(warningLimit);
    notify = true;
  }
  mLimitDatapointsChangedByAlarmConfig = false;
  OS_LeaveRegion();

  if (notify && mNotifyObserversEnabled)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetWarningLimit
* DESCRIPTION: Sets warningLimit.
*
*****************************************************************************/
bool AlarmConfig::SetWarningLimit(int warningLimit)
{
  bool notify = false;

  OS_EnterRegion();
  mLimitDatapointsChangedByAlarmConfig = true;
  if (mpWarningLimit->GetAsInt() != warningLimit)
  {
    mpWarningLimit->SetAsInt(warningLimit);
    notify = true;
  }
  mLimitDatapointsChangedByAlarmConfig = false;
  OS_LeaveRegion();

  if (notify && mNotifyObserversEnabled)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}


/*****************************************************************************
* Function - SetMinLimit
* DESCRIPTION: Sets min for Limits.
*
*****************************************************************************/
void AlarmConfig::SetMinLimit(float minLimit)
{
  MinMaxDataPoint<float>* p_mm_a = dynamic_cast<MinMaxDataPoint<float>*>(mpAlarmLimit);
  MinMaxDataPoint<float>* p_mm_w = dynamic_cast<MinMaxDataPoint<float>*>(mpWarningLimit);

  if (p_mm_a && p_mm_w)
  {
    bool notify = false;

    OS_EnterRegion();
    mLimitDatapointsChangedByAlarmConfig = true;
    if (p_mm_a->GetMinValue() != minLimit || p_mm_w->GetMinValue() != minLimit)
    {
      p_mm_a->SetMinValue( minLimit );
      p_mm_w->SetMinValue( minLimit );

      notify = true;
    }
    mLimitDatapointsChangedByAlarmConfig = false;
    OS_LeaveRegion();

    if (notify && mNotifyObserversEnabled)
      NotifyObservers();

    NotifyObserversE();
  }
  else
  {
    FatalErrorOccured("AlarmConfig: type!=float");
  }
}

/*****************************************************************************
* Function - SetMinLimit
* DESCRIPTION: Sets min for Limits.
*
*****************************************************************************/
void AlarmConfig::SetMinLimit(int minLimit)
{
  bool notify = false;

  OS_EnterRegion();
  mLimitDatapointsChangedByAlarmConfig = true;
  if (mpAlarmLimit->GetMinAsInt() != minLimit || mpWarningLimit->GetMinAsInt() != minLimit)
  {
    mpAlarmLimit->SetMinAsInt(minLimit);
    mpWarningLimit->SetMinAsInt(minLimit);

    notify = true;
  }
  mLimitDatapointsChangedByAlarmConfig = false;
  OS_LeaveRegion();

  if (notify && mNotifyObserversEnabled)
    NotifyObservers();

  NotifyObserversE();
}

/*****************************************************************************
* Function - SetMaxLimit
* DESCRIPTION: Sets max for Limits.
*
*****************************************************************************/
void AlarmConfig::SetMaxLimit(float maxLimit)
{
  MinMaxDataPoint<float>* p_mm_a = dynamic_cast<MinMaxDataPoint<float>*>(mpAlarmLimit);
  MinMaxDataPoint<float>* p_mm_w = dynamic_cast<MinMaxDataPoint<float>*>(mpWarningLimit);

  if (p_mm_a && p_mm_w)
  {
    bool notify = false;

    OS_EnterRegion();
    mLimitDatapointsChangedByAlarmConfig = true;
    if (p_mm_a->GetMaxValue() != maxLimit || p_mm_w->GetMaxValue() != maxLimit)
    {
      p_mm_a->SetMaxValue( maxLimit );
      p_mm_w->SetMaxValue( maxLimit );

      notify = true;
    }
    mLimitDatapointsChangedByAlarmConfig = false;
    OS_LeaveRegion();

    if (notify && mNotifyObserversEnabled)
      NotifyObservers();

    NotifyObserversE();
  }
  else
  {
    FatalErrorOccured("AlarmConfig: type!=float");
  }
}

/*****************************************************************************
* Function - SetMaxLimit
* DESCRIPTION: Sets max for Limits.
*
*****************************************************************************/
void AlarmConfig::SetMaxLimit(int maxLimit)
{
  bool notify = false;

  OS_EnterRegion();
  mLimitDatapointsChangedByAlarmConfig = true;
  if (mpAlarmLimit->GetMaxAsInt() != maxLimit || mpWarningLimit->GetMaxAsInt() != maxLimit)
  {
    mpAlarmLimit->SetMaxAsInt(maxLimit);
    mpWarningLimit->SetMaxAsInt(maxLimit);

    notify = true;
  }
  mLimitDatapointsChangedByAlarmConfig = false;
  OS_LeaveRegion();

  if (notify && mNotifyObserversEnabled)
    NotifyObservers();

  NotifyObserversE();
}

/*****************************************************************************
* Function - SetQuantity
* DESCRIPTION: Sets quantity for Limits.
*
*****************************************************************************/
void AlarmConfig::SetQuantity(QUANTITY_TYPE quantity)
{
  bool notify = false;

  OS_EnterRegion();
  mLimitDatapointsChangedByAlarmConfig = true;
  if (mpAlarmLimit->GetQuantity() != quantity || mpWarningLimit->GetQuantity() != quantity)
  {
    mpAlarmLimit->SetQuantity(quantity);
    mpWarningLimit->SetQuantity(quantity);

    notify = true;
  }
  mLimitDatapointsChangedByAlarmConfig = false;
  OS_LeaveRegion();

  if (notify && mNotifyObserversEnabled)
    NotifyObservers();

  NotifyObserversE();
}

/*****************************************************************************
* subject::GetFlashId implementation
*****************************************************************************/
FLASH_ID_TYPE AlarmConfig::GetFlashId(void)
{
  return FLASH_ID_ALARM_CONFIG;
}

/*****************************************************************************
* subject::SaveToFlash implementation
*****************************************************************************/
void AlarmConfig::SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
{
  // Write a version code of the data type
  pWriter->WriteU8(1);

  switch (save)
  {
  case FLASH_SAVE_VALUE:
  case FLASH_SAVE_ALL:
    pWriter->WriteBool(mSms1Enabled);
    pWriter->WriteBool(mSms2Enabled);
    pWriter->WriteBool(mSms3Enabled);
    pWriter->WriteBool(mScadaEnabled);
    pWriter->WriteBool(mCustomRelayForWarningEnabled);
    pWriter->WriteBool(mCustomRelayForAlarmEnabled);
    pWriter->WriteBool(mWarningEnabled);
    pWriter->WriteBool(mAlarmEnabled);
    pWriter->WriteBool(mAutoAck);
    pWriter->WriteFloat(mAlarmDelay);

    if ( mpWarningLimit->IsFloat())
    {
      pWriter->WriteFloat( mpWarningLimit->GetAsFloat() );
    }
    else
    {
      pWriter->WriteI32((I32) mpWarningLimit->GetAsInt() );
    }
    if ( mpAlarmLimit->IsFloat())
    {
      pWriter->WriteFloat( mpAlarmLimit->GetAsFloat() );
    }
    else
    {
      pWriter->WriteI32((I32) mpAlarmLimit->GetAsInt() );
    }
    break;
  }
}

/*****************************************************************************
* subject::LoadFromFlash implementation
*****************************************************************************/
void AlarmConfig::LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
{
  const U8 version = pReader->ReadU8(0);

  if (version != 1)
  {
    // Nothing, unknown version code for the data type
  }
  else switch (savedAs)
  {
  case FLASH_SAVE_VALUE:
  case FLASH_SAVE_ALL:

    SetSms1Enabled( pReader->ReadBool(mSms1Enabled) );
    SetSms2Enabled( pReader->ReadBool(mSms2Enabled) );
    SetSms3Enabled( pReader->ReadBool(mSms3Enabled) );
    SetScadaEnabled( pReader->ReadBool(mScadaEnabled) );
    SetCustomRelayForWarning( pReader->ReadBool(mCustomRelayForWarningEnabled) );
    SetCustomRelayForAlarm( pReader->ReadBool(mCustomRelayForAlarmEnabled) );
    SetWarningEnabled( pReader->ReadBool(mWarningEnabled) );
    SetAlarmEnabled( pReader->ReadBool(mAlarmEnabled) );
    SetAutoAcknowledge( pReader->ReadBool(mAutoAck) );
    SetAlarmDelay( pReader->ReadFloat(mAlarmDelay) );

    if (mpWarningLimit->IsFloat())
      SetWarningLimit( pReader->ReadFloat(mpWarningLimit->GetAsFloat()) );
    else
      SetWarningLimit( pReader->ReadI32(mpWarningLimit->GetAsInt()) );

    if (mpAlarmLimit->IsFloat())
      SetAlarmLimit( pReader->ReadFloat(mpAlarmLimit->GetAsFloat()) );
    else
      SetAlarmLimit( pReader->ReadI32(mpAlarmLimit->GetAsInt()) );

    break;
  }
}
/*****************************************************************************
* subject::
*****************************************************************************/
void AlarmConfig::AlarmConfigFromGeni(ALARM_CONFIG_GENI_TYPE *pAc)
{
  bool notify = false;
  float f1, f2;
  U32 u32_1, u32_2;

  OS_EnterRegion();
  mLimitDatapointsChangedByAlarmConfig = true;


  /* Alarm enabled */
  if (mAlarmEnabled != pAc->AlarmEnabled)
  {
    mAlarmEnabled = pAc->AlarmEnabled;
    notify = true;
  }

  /* Warning enabled */
  if (mWarningEnabled != pAc->WarningEnabled)
  {
    mWarningEnabled = pAc->WarningEnabled;
    notify = true;
  }

  /* SMS */
  if (mSms1Enabled != pAc->Sms1Enabled)
  {
    mSms1Enabled = pAc->Sms1Enabled;
    notify = true;
  }
  if (mSms2Enabled != pAc->Sms2Enabled)
  {
    mSms2Enabled = pAc->Sms2Enabled;
    notify = true;
  }
  if (mSms3Enabled != pAc->Sms3Enabled)
  {
    mSms3Enabled = pAc->Sms3Enabled;
    notify = true;
  }

  /* SCADA */
  if (mScadaEnabled != pAc->Scada)
  {
    mScadaEnabled = pAc->Scada;
    notify = true;
  }

  /* Warning and alarm limits */
  if (mpWarningLimit->IsFloat())  //float limits
  {
    FloatDataPoint* pFloat = dynamic_cast<FloatDataPoint*>( mpAlarmLimit );

    u32_1 =pAc->AlarmLimit;
    f1=*(float*)&u32_1;
    f2=mpAlarmLimit->GetAsFloat();
    if(f1!=f2)
    {
      if(pFloat)
      {
        pFloat->SetValueUnLtd(f1);
      }
      else
      {
        FatalErrorOccured("AlarmConfig, Lim!=float");
      }
      notify = true;
    }
    u32_1 = pAc->WarningLimit;
    f1=*(float*)&u32_1;
    f2=mpWarningLimit->GetAsFloat();
    if(f1!=f2)
    {
      pFloat = dynamic_cast<FloatDataPoint*>( mpWarningLimit );
      if(pFloat)
      {
        pFloat->SetValueUnLtd(f1);
      }
      else
      {
        FatalErrorOccured("AlarmConfig, Lim!=float");
      }
      notify = true;
    }
  }
  else  //integer limits
  {
    I32DataPoint* pI32 = dynamic_cast<I32DataPoint*>(mpAlarmLimit);

    u32_1 = pAc->AlarmLimit;
    u32_2 = mpAlarmLimit->GetAsInt();
    if( u32_1 != u32_2 )
    {
      if(pI32)
      {
        pI32->SetValueUnLtd(u32_1);
      }
      else
      {
        FatalErrorOccured("AlarmConfig, Lim!=int");
      }
      notify = true;
    }
    u32_1 = pAc->WarningLimit;
    u32_2 = mpWarningLimit->GetAsInt();
    if( u32_1 != u32_2 )
    {
      pI32 = dynamic_cast<I32DataPoint*>(mpWarningLimit);
      if(pI32)
      {
        pI32->SetValueUnLtd(u32_1);
      }
      else
      {
        FatalErrorOccured("AlarmConfig, Lim!=int");
      }
      notify = true;
    }
  }

  /* Custom alarm relay */
  if ( mCustomRelayForWarningEnabled != pAc->CustomWarningRelay )
  {
    mCustomRelayForWarningEnabled = pAc->CustomWarningRelay;
    notify = true;
  }
  if ( mCustomRelayForAlarmEnabled != pAc->CustomAlarmRelay )
  {
    mCustomRelayForAlarmEnabled = pAc->CustomAlarmRelay;
    notify = true;
  }

  /* Alarm Acknolege */
  if ( mAutoAck != pAc->AlarmAutoAck )
  {
    mAutoAck = pAc->AlarmAutoAck;
    notify = true;
  }

  /* Alarm delay */
  f1 =pAc->AlarmDelay;
  f2 = mAlarmDelay;
  if( f1!=f2 )
  {
    mAlarmDelay = f1;
    notify = true;
  }

  mLimitDatapointsChangedByAlarmConfig = false;
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();
  NotifyObserversE();
}

/*****************************************************************************
* subject::
*****************************************************************************/
void AlarmConfig::AlarmConfigToGeni(ALARM_CONFIG_GENI_TYPE *pAc)
{
  float f1;
  U32 u32;

  OS_EnterRegion();

  pAc->AlarmEnabled = mAlarmEnabled;
  pAc->WarningEnabled = mWarningEnabled;
  pAc->Sms1Enabled = mSms1Enabled;
  pAc->Sms2Enabled = mSms2Enabled;
  pAc->Sms3Enabled = mSms3Enabled;
  pAc->Scada = mScadaEnabled;

  /* Warning and alarm limits */
  if (mpWarningLimit->IsFloat())  //float limits
  {
    f1 = mpAlarmLimit->GetAsFloat();
    u32=*(U32*)&f1;
    pAc->AlarmLimit=u32;
    f1 = mpWarningLimit->GetAsFloat();
    u32=*(U32*)&f1;
    pAc->WarningLimit=u32;

  }
  else  //integer limits
  {
    u32 = mpAlarmLimit->GetAsInt();
    pAc->AlarmLimit = u32;
    u32 = mpWarningLimit->GetAsInt();
    pAc->WarningLimit = u32;
  }

  pAc->CustomAlarmRelay = mCustomRelayForAlarmEnabled;
  pAc->CustomWarningRelay = mCustomRelayForWarningEnabled;
  pAc->AlarmAutoAck = mAutoAck;

  u32 = mAlarmDelay+0.5;
  pAc->AlarmDelay = u32;

  OS_LeaveRegion();

  pAc->AlarmConfigType=mAlarmConfigType;
}


/*****************************************************************************
 * Function -
 * DESCRIPTION:
 * Update subscribers when the alarm limit or warning limit datapoint
 * have been changed through the interface obtained by calls to
 * GetAlarmLimit() and GetWarningLimit()
 ****************************************************************************/
void AlarmConfig::Update(Subject* pSubject)
{
  if (!mLimitDatapointsChangedByAlarmConfig)
  {
    NotifyObservers();
    NotifyObserversE();
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
