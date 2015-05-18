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
/* CLASS NAME       : BoolDataPoint                                         */
/*                                                                          */
/* FILE NAME        : BoolDataPoint.h                                       */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __BOOLDATAPOINT_H__
#define __BOOLDATAPOINT_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <MinMaxDataPoint.h>
#include <IIntegerDataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
* CLASS: BoolDataPoint
* DESCRIPTION: Bool data point
*****************************************************************************/
class BoolDataPoint : public MinMaxDataPoint<bool>, public IIntegerDataPoint
{
public:
  /********************************************************************
  * LIFECYCLE - Constructor
  ********************************************************************/
	BoolDataPoint() : MinMaxDataPoint<bool>()
	{
		mMinValue = false;
		mMaxValue = true;
	}

  /********************************************************************
  * LIFECYCLE - Destructor
  ********************************************************************/
	virtual ~BoolDataPoint()
	{
	}
  
  /*****************************************************************************
  * IIntegerDataPoint::GetAsInt implementation
  *****************************************************************************/
	virtual int GetAsInt(void)
	{
		return GetValue() ? 1 : 0;
	}

  /*****************************************************************************
  * IIntegerDataPoint::SetAsInt implementation
  *****************************************************************************/
	virtual bool SetAsInt(int intValue)
	{
		return SetValue(intValue ? true : false);
	}

  /*****************************************************************************
  * IIntegerDataPoint::GetAsBool implementation
  *****************************************************************************/
	virtual bool GetAsBool(void)
	{
		return GetValue();
	}

  /*****************************************************************************
  * IIntegerDataPoint::SetAsBool implementation
  *****************************************************************************/
	virtual bool SetAsBool(bool boolValue)
	{
		return SetValue(boolValue);
	}
	
  /*****************************************************************************
  * Subject::GetFlashId implementation
  *****************************************************************************/
	virtual FLASH_ID_TYPE GetFlashId(void)
	{
		return FLASH_ID_BOOL_DATA_POINT;
	}

  /*****************************************************************************
  * Subject::SaveToFlash implementation
  *****************************************************************************/
	virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
	{
		pWriter->WriteBool(mValue);
	}

  /*****************************************************************************
  * Subject::LoadFromFlash implementation
  *****************************************************************************/
	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
	{
		SetValue(pReader->ReadBool(mValue));
	}

private:
  /*****************************************************************************
  * MinMaxDataPoint::SetMinValue override
  * It makes no sense to adjust min value on a bool so let's make it private
  *****************************************************************************/
  virtual bool SetMinValue(bool newMinValue)
	{
    return false;
	}

  /*****************************************************************************
  * MinMaxDataPoint::SetMaxValue override
  * It makes no sense to adjust max value on a bool so let's make it private
  *****************************************************************************/
  virtual bool SetMaxValue(bool newMaxValue)
	{
    return false;
	}
};

#endif
