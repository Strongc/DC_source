/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Plaform                                   */
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
/* CLASS NAME       : CurrentPointCurveData                                 */
/*                                                                          */
/* FILE NAME        : CurrentPointCurveData.cpp                             */
/*                                                                          */
/* CREATED DATE     : 30-01-2008   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* See header file                                                          */
/****************************************************************************/

#include <CurrentPointCurveData.h>

/*****************************************************************************
* LIFECYCLE - Constructor
*****************************************************************************/
CurrentPointCurveData::CurrentPointCurveData(const int size) : mSize(size), Subject()
{
  mCurveValidFlags = new bool[mSize];
  mCurveValues = new float[mSize];
  mMaxY = 0.0f;
  mMinY = 0.0f;
  mMaxX = 0.0f;
  mMinX = 0.0f;
  mYQuantity = Q_NO_UNIT;
  mXQuantity = Q_NO_UNIT;
  mCurrentXQuality = DP_NEVER_AVAILABLE;
  mCurrentYQuality = DP_NEVER_AVAILABLE;;
  mCurrentXValue = 0.0;
  mCurrentYValue = 0.0;
  ClearCurveValues();
}

/*****************************************************************************
* LIFECYCLE - Destructor
*****************************************************************************/
CurrentPointCurveData::~CurrentPointCurveData()
{
  delete [] mCurveValues;
  delete [] mCurveValidFlags;
}

/*****************************************************************************
* FUNCTION - GetSize
* DESCRIPTION: See header file
*****************************************************************************/
int CurrentPointCurveData::GetSize(void)
{
  return mSize;
}

/*****************************************************************************
* FUNCTION - Lock
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::Lock(void)
{
}

/*****************************************************************************
* FUNCTION - UnLock
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::UnLock(void)
{
}

/*****************************************************************************
* FUNCTION - SetMinXValue
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetMinXValue(const float value)
{
  if (mMinX != value)
  {
    mMinX = value;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - SetMaxXValue
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetMaxXValue(const float value)
{
  if (mMaxX != value)
  {
    mMaxX = value;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - GetMinXValue
* DESCRIPTION: See header file
*****************************************************************************/
float CurrentPointCurveData::GetMinXValue(void)
{
  return mMinX;
}

/*****************************************************************************
* FUNCTION - GetMaxXValue
* DESCRIPTION: See header file
*****************************************************************************/
float CurrentPointCurveData::GetMaxXValue(void)
{
  return mMaxX;
}

/*****************************************************************************
* FUNCTION - SetMinYValue
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetMinYValue(const float value)
{
  if (mMinY != value)
  {
    mMinY = value;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - SetMaxYValue
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetMaxYValue(const float value)
{
  if (mMaxY != value)
  {
    mMaxY = value;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - GetMinYValue
* DESCRIPTION: See header file
*****************************************************************************/
float CurrentPointCurveData::GetMinYValue(void)
{
  return mMinY;
}

/*****************************************************************************
* FUNCTION - GetMaxYValue
* DESCRIPTION: See header file
*****************************************************************************/
float CurrentPointCurveData::GetMaxYValue(void)
{
  return mMaxY;
}

/*****************************************************************************
* FUNCTION - ClearCurveValues
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::ClearCurveValues(void)
{
  for (int i = 0; i < mSize; i++)
  {
    mModified |= mCurveValidFlags[i] != false;
    mCurveValidFlags[i] = false;
  }
}

/*****************************************************************************
* FUNCTION - SetCurveValue
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetCurveValue(int index, float value)
{
  // check index
  if ((index >= mSize) || (index < 0))
  {
    return;
  }

  // within range?
  if ((value >= mMinY) && (value <= mMaxY))
  {
    // modified?
    if ((mCurveValues[index] != value) || (mCurveValidFlags[index] != true))
    {
      mCurveValues[index] = value;
      mCurveValidFlags[index] = true;
      mModified = true;
    }
  }
  else // out of range - mark value in-valid
  {
    // modified?
    if (mCurveValidFlags[index] != false)
    {
      mCurveValidFlags[index] = false;
      mModified = true;
    }
  }
}

/*****************************************************************************
* FUNCTION - SetCurveValueValid
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetCurveValueValid(int index, bool valid)
{
  // check index
  if ((index >= mSize) || (index < 0))
  {
    return;
  }

  // set new valid flag if modified
  if (mCurveValidFlags[index] != valid)
  {
    mCurveValidFlags[index] = valid;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - IsCurveValueValid
* DESCRIPTION: See header file
*****************************************************************************/
bool CurrentPointCurveData::IsCurveValueValid(int index)
{
  // check index
  if ((index >= mSize) || (index < 0))
  {
    return false;
  }

  return mCurveValidFlags[index];
}

/*****************************************************************************
* FUNCTION - GetCurveValue
* DESCRIPTION: See header file
*****************************************************************************/
float CurrentPointCurveData::GetCurveValue(int index)
{
  // check index and valid flag
  if ((index >= mSize) || (index < 0) || !mCurveValidFlags[index])
  {
    return 0.0f;
  }

  // return value
  return mCurveValues[index];
}

/*****************************************************************************
* FUNCTION - GetCurveValueAsPercent
* DESCRIPTION: See header file
*****************************************************************************/
float CurrentPointCurveData::GetCurveValueAsPercent(int index)
{
  const float range = (mMaxY - mMinY);

  // check index, valid flag and range
  if ((index >= mSize) || (index < 0) || !mCurveValidFlags[index] || (range <= 0.0))
  {
    return 0.0f;
  }

  // calculate and return value as percent
  return (mCurveValues[index] - mMinY) / range * 100.0f;
}

/*****************************************************************************
* FUNCTION - SetYQuantity
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetYQuantity(QUANTITY_TYPE quantity)
{
  if ( mYQuantity != quantity)
  {
    mYQuantity = quantity;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - SetXQuantity
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetXQuantity(QUANTITY_TYPE quantity)
{
  if (mXQuantity != quantity)
  {
    mXQuantity = quantity;
    mModified = true;
  }
}


/*****************************************************************************
* FUNCTION - GetYQuantity
* DESCRIPTION: See header file
*****************************************************************************/
QUANTITY_TYPE CurrentPointCurveData::GetYQuantity(void)
{
  return mYQuantity;
}

/*****************************************************************************
* FUNCTION - GetXQuantity
* DESCRIPTION: See header file
*****************************************************************************/
QUANTITY_TYPE CurrentPointCurveData::GetXQuantity(void)
{
  return mXQuantity;
}

/*****************************************************************************
* FUNCTION - GetCurrentXQuality
* DESCRIPTION: See header file
*****************************************************************************/
DP_QUALITY_TYPE CurrentPointCurveData::GetCurrentXQuality()
{
  return mCurrentXQuality;
}

/*****************************************************************************
* FUNCTION - GetCurrentXValue
* DESCRIPTION: See header file
*****************************************************************************/
float CurrentPointCurveData::GetCurrentXValue(void)
{
  return mCurrentXValue;
}

/*****************************************************************************
* FUNCTION - SetCurrentXQuality
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetCurrentXQuality(DP_QUALITY_TYPE quality)
{
  if (mCurrentXQuality != quality)
  {
    mCurrentXQuality = quality;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - SetCurrentXValue
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetCurrentXValue(float value)
{
  if ((mCurrentXValue != value) || (mCurrentXQuality != DP_AVAILABLE))
  {
    mCurrentXValue = value;
    mCurrentXQuality = DP_AVAILABLE;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - GetCurrentYQuality
* DESCRIPTION: See header file
*****************************************************************************/
DP_QUALITY_TYPE CurrentPointCurveData::GetCurrentYQuality()
{
  return mCurrentYQuality;
}

/*****************************************************************************
* FUNCTION - GetCurrentYValue
* DESCRIPTION: See header file
*****************************************************************************/
float CurrentPointCurveData::GetCurrentYValue(void)
{
  return mCurrentYValue;
}

/*****************************************************************************
* FUNCTION - SetCurrentYQuality
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetCurrentYQuality(DP_QUALITY_TYPE quality)
{
  if (mCurrentYQuality != quality)
  {
    mCurrentYQuality = quality;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - SetCurrentYValue
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetCurrentYValue(float value)
{
  if ((mCurrentYValue != value) || (mCurrentYQuality != DP_AVAILABLE))
  {
    mCurrentYValue = value;
    mCurrentYQuality = DP_AVAILABLE;
    mModified = true;
  }
}

/*****************************************************************************
* FUNCTION - SetCurrentQuality
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::SetCurrentQuality(DP_QUALITY_TYPE quality)
{
  if ((mCurrentXQuality != quality) || (mCurrentYQuality != quality))
  {
    mCurrentXQuality = quality;
    mCurrentYQuality = quality;
    mModified = true;
  }
}

/*****************************************************************************
  * FUNCTION - CopyValues
  * DESCRIPTION: copy values between to vectors of same type and size
  *****************************************************************************/
  void CurrentPointCurveData::CopyValues(CurrentPointCurveData* pSource)
  { 
    for (int i = 0; i < pSource->GetSize(); i++)
    {
      mCurveValues[i] = pSource->GetCurveValue(i);
      mCurveValidFlags[i] = pSource->IsCurveValueValid(i);
    }

    mMinX = pSource->GetMinXValue();
    mMinY = pSource->GetMinYValue();
    mMaxX = pSource->GetMaxXValue();
    mMaxY = pSource->GetMaxYValue();
    mXQuantity = pSource->GetXQuantity();
    mYQuantity = pSource->GetYQuantity();
    mCurrentXValue = pSource->GetCurrentXValue();
    mCurrentYValue = pSource->GetCurrentYValue();
    mCurrentXQuality = pSource->GetCurrentXQuality();
    mCurrentYQuality = pSource->GetCurrentYQuality();

    mModified = true;
    NotifyObservers();
  }


/*****************************************************************************
* FUNCTION - NotifyObservers
* DESCRIPTION: See header file
*****************************************************************************/
void CurrentPointCurveData::NotifyObservers(void)
{
  if (mModified)
  {
    mModified = false;
    Subject::NotifyObservers();
  }
}

