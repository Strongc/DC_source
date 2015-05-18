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
/* CLASS NAME       : NumberDataPoint                                       */
/*                                                                          */
/* FILE NAME        : NumberDataPoint.h                                     */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __NUMBERDATAPOINT_H__
#define __NUMBERDATAPOINT_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <rtos.h>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <MinMaxDataPoint.h>
#include <INumberDataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
* CLASS: NumberDataPoint
* DESCRIPTION: Number data point template class 
*****************************************************************************/
template<typename VAL_TYPE>
class NumberDataPoint : public MinMaxDataPoint<VAL_TYPE>, public INumberDataPoint
{
protected:
  /********************************************************************
  * LIFECYCLE - Constructor
  * INPUT - isFloat: true if the template VAL_TYPE is a float
  *                  (set by derived impl. classes: I32DataPoint...
  ********************************************************************/
	NumberDataPoint(bool isFloat) : MinMaxDataPoint<VAL_TYPE>()
	{
    mIsFloat = isFloat;
	}

public:
  /********************************************************************
  * LIFECYCLE - Destructor
  * INPUT - isFloat: true if the template VAL_TYPE is a float
  ********************************************************************/
	virtual ~NumberDataPoint() 
	{
	}
  
public:
  /*****************************************************************************
  * PUBLIC METHODS
  *****************************************************************************/

  /*****************************************************************************
  * FUNCTION - IsFloat
  * DESCRIPTION: Returns true if the template VAL_TYPE is a float, 
  *              see constructor
  *****************************************************************************/
 	virtual bool IsFloat(void)
  {
    return mIsFloat;
  }

  /*****************************************************************************
  * FUNCTION - GetValueAsPercent
  * DESCRIPTION: Returns the value as a percentage (min/max)
  *****************************************************************************/
  virtual float GetValueAsPercent(void)
  {
    float ret_val;
		VAL_TYPE delta;

    OS_EnterRegion();
		delta = mMaxValue - mMinValue;

    if (delta > 0)
      ret_val = ((mValue - mMinValue) * 100.0f / delta);
    else
      ret_val = 0.0f;
    OS_LeaveRegion();

    return ret_val;
  }

  /*****************************************************************************
  * FUNCTION - SetValueAsPercent
  * DESCRIPTION: Sets the value as a percentage (min/max)
  *****************************************************************************/
  virtual bool SetValueAsPercent(float newPercentage)
  {
    float val;
    OS_EnterRegion();
    val = (newPercentage * (mMaxValue - mMinValue) / 100.0f + mMinValue);
    OS_LeaveRegion();
    return SetValue((VAL_TYPE)val);
  }

private:
  /*****************************************************************************
  * PUBLIC ATTRIBUTES
  *****************************************************************************/
  bool mIsFloat;
};

#endif
