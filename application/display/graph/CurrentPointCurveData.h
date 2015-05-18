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
/* FILE NAME        : CurrentPointCurveData.h                               */
/*                                                                          */
/* CREATED DATE     : 05-07-2007   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Data for the CurrentPointCurveGraph                                      */
/****************************************************************************/
#ifndef __CURRENT_POINT_CURVE_DATA_H__
#define __CURRENT_POINT_CURVE_DATA_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <rtos.h>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Subject.h>
#include <MpcUnit/MpcUnits.h>

/*****************************************************************************
LOCAL INCLUDES
*****************************************************************************/

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPEDEFS
*****************************************************************************/

/*****************************************************************************
* CLASS: CurrentPointCurveData
* DESCRIPTION: Data class for CurrentPointCurveGraph
*****************************************************************************/
class CurrentPointCurveData : public Subject
{
public:
  /*****************************************************************************
  * LIFECYCLE - Constructor
  * INPUT - initialSize: initial size of the circulat buffer
  *****************************************************************************/
  CurrentPointCurveData(int initialSize);

  /*****************************************************************************
  * LIFECYCLE - Destructor
  *****************************************************************************/
  virtual ~CurrentPointCurveData();

public:
  /*****************************************************************************
  * PUBLIC METHODS
  *****************************************************************************/

  /*****************************************************************************
  * FUNCTION - GetSize
  * DESCRIPTION: Returns the size of the buffer
  *****************************************************************************/
  virtual int GetSize(void);

  /*****************************************************************************
  * FUNCTION - Lock
  * DESCRIPTION: Locks access to the buffer.
  *****************************************************************************/
  virtual void Lock(void);

  /*****************************************************************************
  * FUNCTION - UnLock
  * DESCRIPTION: Un-locks access to the buffer.
  *****************************************************************************/
  virtual void UnLock(void);

  /*****************************************************************************
  * FUNCTION - SetMinXValue
  * DESCRIPTION: Sets the min x value for the buffer.
  *****************************************************************************/
  virtual void SetMinXValue(const float value);

  /*****************************************************************************
  * FUNCTION - SetMaxXValue
  * DESCRIPTION: Sets the max x value for the buffer.
  *****************************************************************************/
  virtual void SetMaxXValue(const float value);

  /*****************************************************************************
  * FUNCTION - GetMinXValue
  * DESCRIPTION: Gets the min x value for the buffer.
  *****************************************************************************/
  virtual float GetMinXValue(void);

  /*****************************************************************************
  * FUNCTION - GetMaxXValue
  * DESCRIPTION: Gets the max x value for the buffer.
  *****************************************************************************/
  virtual float GetMaxXValue(void);

  /*****************************************************************************
  * FUNCTION - SetMinYValue
  * DESCRIPTION: Sets the min y value for the buffer.
  *****************************************************************************/
  virtual void SetMinYValue(const float value);

  /*****************************************************************************
  * FUNCTION - SetMaxYValue
  * DESCRIPTION: Sets the max y value for the buffer.
  *****************************************************************************/
  virtual void SetMaxYValue(const float value);

  /*****************************************************************************
  * FUNCTION - GetMinYValue
  * DESCRIPTION: Gets the min y value for the buffer.
  *****************************************************************************/
  virtual float GetMinYValue(void);

  /*****************************************************************************
  * FUNCTION - GetMaxYValue
  * DESCRIPTION: Gets the max y value for the buffer.
  *****************************************************************************/
  virtual float GetMaxYValue(void);

  /*****************************************************************************
  * FUNCTION - SetXQuantity
  * DESCRIPTION: Sets quantity of the x value
  *****************************************************************************/
  virtual void SetXQuantity(QUANTITY_TYPE quantity);

  /*****************************************************************************
  * FUNCTION - SetYQuantity
  * DESCRIPTION: Sets quantity of the y value
  *****************************************************************************/
  virtual void SetYQuantity(QUANTITY_TYPE quantity);

  /*****************************************************************************
  * FUNCTION - GetXQuantity
  * DESCRIPTION: Gets quantity of the x value
  *****************************************************************************/
  virtual QUANTITY_TYPE GetXQuantity(void);

  /*****************************************************************************
  * FUNCTION - GetYQuantity
  * DESCRIPTION: Gets quantity of the y value
  *****************************************************************************/
  virtual QUANTITY_TYPE GetYQuantity(void);

  DP_QUALITY_TYPE GetCurrentXQuality();
  float GetCurrentXValue(void);
  void SetCurrentXQuality(DP_QUALITY_TYPE quality);
  void SetCurrentXValue(float value);

  DP_QUALITY_TYPE GetCurrentYQuality();
  float GetCurrentYValue(void);
  void SetCurrentYQuality(DP_QUALITY_TYPE quality);
  void SetCurrentYValue(float value);

  void SetCurrentQuality(DP_QUALITY_TYPE quality);

  /*****************************************************************************
  * FUNCTION - ClearCurveValues
  * DESCRIPTION: Cleans the validity in all the curve data values (set to false).
  *****************************************************************************/
  virtual void ClearCurveValues(void);

  /*****************************************************************************
  * FUNCTION - SetCurveValue
  * DESCRIPTION: Sets the value at the specified index and sets valid flag true
  *****************************************************************************/
  virtual void SetCurveValue(int index, float value);

  /*****************************************************************************
  * FUNCTION - SetCurveValueValid
  * DESCRIPTION: Sets the validity of the value at the specified index
  *****************************************************************************/
  virtual void SetCurveValueValid(int index, bool valid);

  /*****************************************************************************
  * FUNCTION - IsCurveValueValid
  * DESCRIPTION: Returns the validity of the curve value at the index given
  * or false if out of bounce
  *****************************************************************************/
  virtual bool IsCurveValueValid(int index);

  /*****************************************************************************
  * FUNCTION - GetCurveValue
  * DESCRIPTION: Returns the value at the index given or 0.0
  * if index out of bounds or value at index isn't valid
  *****************************************************************************/
  virtual float GetCurveValue(int index);

  /*****************************************************************************
  * FUNCTION - GetCurveValueAsPercent
  * DESCRIPTION: Returns the value in percent at the index given or 0.0
  * if index out of bounds or value at index isn't valid
  *****************************************************************************/
  virtual float GetCurveValueAsPercent(int index);

  /*****************************************************************************
  * FUNCTION - CopyValues
  * DESCRIPTION: copies curve data from a source
  *****************************************************************************/
  virtual void CopyValues(CurrentPointCurveData* pSource);

  /*****************************************************************************
  * FUNCTION - NotifyObservers
  * DESCRIPTION: See header file
  *****************************************************************************/
  virtual void NotifyObservers(void);

private:
  /*****************************************************************************
  * PRIVATE METHODS
  *****************************************************************************/

  /*****************************************************************************
  * PRIVATE ATTRIBUTES
  *****************************************************************************/
  const int mSize;
  float     mMaxY;
  float     mMinY;
  QUANTITY_TYPE mYQuantity;
  float     mMaxX;
  float     mMinX;
  QUANTITY_TYPE mXQuantity;
  DP_QUALITY_TYPE mCurrentXQuality;
  DP_QUALITY_TYPE mCurrentYQuality;
  float     mCurrentXValue;
  float     mCurrentYValue;
  bool*     mCurveValidFlags;
  float*    mCurveValues;
  bool      mModified;

};

#endif


