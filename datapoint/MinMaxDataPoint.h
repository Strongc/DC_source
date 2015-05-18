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
/* CLASS NAME       : MinMaxDataPoint                                       */
/*                                                                          */
/* FILE NAME        : MinMaxDataPoint.h                                     */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __MINMAXDATAPOINT_H__
#define __MINMAXDATAPOINT_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <rtos.h>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <factory.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <DataPoint.h>
#include <IMinMaxDataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
* CLASS: MinMaxDataPoint
* DESCRIPTION: Min max data point template class
*****************************************************************************/
template<typename VAL_TYPE>
class MinMaxDataPoint : public DataPoint, public virtual IMinMaxDataPoint
{
protected:
  /*****************************************************************************
  * LIFECYCLE - Default constructor.
  *****************************************************************************/
  MinMaxDataPoint()
  {
  }

  /*****************************************************************************
  * LIFECYCLE - Destructor.
  *****************************************************************************/
  virtual ~MinMaxDataPoint()
  {
  }

public:
  /*****************************************************************************
  * PUBLIC METHODS
  *****************************************************************************/

  /*****************************************************************************
  * FUNCTION - GetValue
  * DESCRIPTION:
  *****************************************************************************/
  virtual VAL_TYPE GetValue(void)
  {
    VAL_TYPE ret_val;
    OS_EnterRegion();
    ret_val = mValue;
    OS_LeaveRegion();
    return(ret_val);
  };

  /*****************************************************************************
  * FUNCTION - SetValue
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetValue(VAL_TYPE newValue)
  {
    bool ret_val;

    // set new value WITHOUT notifying
    ret_val = SetValuePure(newValue, false);

    // ensure quality is set to available, WITHOUT notifying
    ret_val |= SetQuality(DP_AVAILABLE, false);

    // notify if changed
    if (ret_val)
      NotifyObservers();

    NotifyObserversE();

    return (ret_val);
  }

  /*****************************************************************************
  * FUNCTION - GetMinValue
  * DESCRIPTION:
  *****************************************************************************/
  virtual VAL_TYPE GetMinValue(void)
  {
    VAL_TYPE ret_val;
    OS_EnterRegion();
    ret_val = mMinValue;
    OS_LeaveRegion();
    return(ret_val);
  }

  /*****************************************************************************
  * FUNCTION - SetMinValue
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetMinValue(VAL_TYPE newMinValue)
  {
    bool ret_val;
    OS_EnterRegion();
    if (mMinValue != newMinValue)
    {
      mMinValue = newMinValue;
      if (mValue < mMinValue)
      {
        mValue = mMinValue;
      }
      ret_val = true;
    }
    else
    {
      ret_val = false;
    }
    OS_LeaveRegion();

    if (ret_val)
      NotifyObservers();

    NotifyObserversE();

    return(ret_val);
  }

  /*****************************************************************************
  * FUNCTION - GetMaxValue
  * DESCRIPTION:
  *****************************************************************************/
  virtual VAL_TYPE GetMaxValue(void)
  {
    VAL_TYPE ret_val;
    OS_EnterRegion();
    ret_val = mMaxValue;
    OS_LeaveRegion();
    return(ret_val);
  }

  /*****************************************************************************
  * FUNCTION - SetMaxValue
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetMaxValue(VAL_TYPE newMaxValue)
  {
    bool ret_val;
    OS_EnterRegion();
    if (mMaxValue != newMaxValue)
    {
      mMaxValue = newMaxValue;
      if (mValue > mMaxValue)
      {
        mValue = mMaxValue;
      }
      ret_val = true;
    }
    else
    {
      ret_val = false;
    }
    OS_LeaveRegion();

    if (ret_val)
      NotifyObservers();

    NotifyObserversE();
    return(ret_val);
  }

  /*****************************************************************************
  * FUNCTION - SetValuePure
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetValuePure(VAL_TYPE newValue)
  {
    return SetValuePure(newValue, true);
  }

  /*****************************************************************************
  * FUNCTION - SetValueUnLtd
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetValueUnLtd(VAL_TYPE newValue)
  {
    bool ret_val;

    OS_EnterRegion();
    if(mValue != newValue)
    {
      mValue = newValue;
      ret_val = true;
    }
    else
    {
      ret_val = false;
    }
    OS_LeaveRegion();

    if (ret_val)
      NotifyObservers();

    NotifyObserversE();
    return(ret_val);
  }

  /*****************************************************************************
  * FUNCTION - CopyValues
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool CopyValues(IMinMaxDataPoint* pSource_)
  {
    bool notify = false;
    VAL_TYPE val;
    DP_QUALITY_TYPE quality;
    QUANTITY_TYPE quantity;
    bool writable;
    MinMaxDataPoint* pSource = dynamic_cast<MinMaxDataPoint*>(pSource_);

    if (pSource == NULL)
    {
      FatalErrorOccured("Missing pointer to MinMaxDataPoint!!");
    }

    OS_EnterRegion();
    val = pSource->GetMinValue();
    if (val != mMinValue)
    {
      notify |= true;
      mMinValue = val;
    }

    val = pSource->GetMaxValue();
    if (val != mMaxValue)
    {
      notify |= true;
      mMaxValue = val;
    }

    val = pSource->GetValue();
    if (val != mValue)
    {
      notify |= true;
      mValue = val;
    }

    quality = pSource->GetQuality();
    if (quality != mQuality)
    {
      notify |= true;
      mQuality = quality;
    }

    quantity = pSource->GetQuantity();
    if (quantity != mQuantity)
    {
      notify |= true;
      mQuantity = quantity;
    }

    writable = pSource->GetWritable();
    if (writable != mWritable)
    {
      notify |= true;
      mWritable = writable;
    }
    OS_LeaveRegion();

    if (notify)
      NotifyObservers();

    NotifyObserversE();

		return notify;
  }

  /*****************************************************************************
  * IMinMaxDataPoint::IsAtMax implementation
  *****************************************************************************/
  virtual bool IsAtMax(void)
  {
    bool rc;

    OS_EnterRegion();

    rc = (mValue >= mMaxValue) ? true : false;

    OS_LeaveRegion();

    return rc;
  }

  /*****************************************************************************
  * IMinMaxDataPoint::IsAtMin implementation
  *****************************************************************************/
  virtual bool IsAtMin(void)
  {
    bool rc;

    OS_EnterRegion();

    rc = (mValue <= mMinValue) ? true : false;

    OS_LeaveRegion();

    return rc;
  }

  /*****************************************************************************
  * IMinMaxDataPoint::GetMinAsInt implementation
  *****************************************************************************/
	virtual int GetMinAsInt(void)
  {
    return (int)GetMinValue();
  }

  /*****************************************************************************
  * IMinMaxDataPoint::GetMaxAsInt implementation
  *****************************************************************************/
	virtual int GetMaxAsInt(void)
  {
    return (int)GetMaxValue();
  }

  /*****************************************************************************
  * IMinMaxDataPoint::GetMinAsFloat implementation
  *****************************************************************************/
	virtual float GetMinAsFloat(void)
  {
    return (float)GetMinValue();
  }

  /*****************************************************************************
  * IMinMaxDataPoint::GetMaxAsFloat implementation
  *****************************************************************************/
	virtual float GetMaxAsFloat(void)
  {
    return (float)GetMaxValue();
  }

  /*****************************************************************************
  * IMinMaxDataPoint::SetMaxAsInt implementation
  *****************************************************************************/
	virtual bool SetMaxAsInt(int newValue)
  {
    return SetMaxValue((VAL_TYPE) newValue);
  }

  /*****************************************************************************
  * IMinMaxDataPoint::SetMinAsInt implementation
  *****************************************************************************/
	virtual bool SetMinAsInt(int newValue)
  {
    return SetMinValue((VAL_TYPE) newValue);
  }


private:
  /********************************************************************
  * PRIVATE METHODS
  ********************************************************************/
  bool SetValuePure(VAL_TYPE newValue, bool notify)
  {
    bool ret_val;

    OS_EnterRegion();
    if(newValue < mMinValue)
    {
      newValue = mMinValue;
    }
    if(newValue > mMaxValue)
    {
      newValue = mMaxValue;
    }
    if(mValue != newValue)
    {
      mValue = newValue;
      ret_val = true;
    }
    else
    {
      ret_val = false;
    }
    OS_LeaveRegion();

    if (notify)
    {
      if (ret_val)
        NotifyObservers();

      NotifyObserversE();
    }

    return(ret_val);
  }

protected:
  /********************************************************************
  * PROTECTED ATTRIBUTES
  ********************************************************************/
  VAL_TYPE mValue;
  VAL_TYPE mMaxValue;
  VAL_TYPE mMinValue;
};

#endif
