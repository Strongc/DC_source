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
/* CLASS NAME       : U32DataPoint                                          */
/*                                                                          */
/* FILE NAME        : U32DataPoint.h                                        */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __U32DATAPOINT_H__
#define __U32DATAPOINT_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <NumberDataPoint.h>
#include <IIntegerDataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
* CLASS: U32DataPoint
* DESCRIPTION: U32 number data point
*****************************************************************************/
class U32DataPoint : public NumberDataPoint<U32>, public IIntegerDataPoint
{
public:
  /*****************************************************************************
  * LIFECYCLE - Constructor
  *****************************************************************************/
	U32DataPoint() : NumberDataPoint<U32>(false)
	{
	}

  /*****************************************************************************
  * LIFECYCLE - Destructor
  *****************************************************************************/
	virtual ~U32DataPoint() 
	{
	}
  
  /*****************************************************************************
  * INumberDataPoint::GetAsFloat implementation
  *****************************************************************************/
	virtual float GetAsFloat(void)
	{
		return (float)GetValue();
	}

  /*****************************************************************************
  * INumberDataPoint::SetAsFloat implementation
  *****************************************************************************/
	virtual bool SetAsFloat(float floatValue) 
	{
		return SetValue((U32)floatValue);
	}

  /*****************************************************************************
  * INumberDataPoint::GetAsInt / IIntegerDataPoint::GetAsInt implementation
  *****************************************************************************/
	virtual int GetAsInt(void)
	{
		return (int)GetValue();
	}

  /*****************************************************************************
  * INumberDataPoint::SetAsInt / IIntegerDataPoint::SetAsInt implementation
  *****************************************************************************/
	virtual bool SetAsInt(int intValue)
	{
		return SetValue((U32)intValue);
	}

  /*****************************************************************************
  * IIntegerDataPoint::GetAsBool implementation
  *****************************************************************************/
	virtual bool GetAsBool(void)
	{
		return GetValue() ? true : false;
	}

  /*****************************************************************************
  * IIntegerDataPoint::SetAsBool implementation
  *****************************************************************************/
	virtual bool SetAsBool(bool boolValue)
	{
	    return SetValue((U32)(boolValue ? 1 : 0));
	}

  /*****************************************************************************
  * Subject::GetFlashId implementation
  *****************************************************************************/
	virtual FLASH_ID_TYPE GetFlashId(void)
	{
		return FLASH_ID_U32_DATA_POINT;
	}

  /*****************************************************************************
  * Subject::SaveToFlash implementation
  *****************************************************************************/
	virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
	{
		switch (save)
		{
		case FLASH_SAVE_VALUE:
			pWriter->WriteU32(mValue);
			break;
		case FLASH_SAVE_ALL:
			pWriter->WriteU32(mMaxValue);
			pWriter->WriteU32(mMinValue);
			pWriter->WriteU32(mValue);
			pWriter->WriteI32(mQuantity);
			break;
		}
	}

  /*****************************************************************************
  * Subject::LoadFromFlash implementation
  *****************************************************************************/
	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
	{
		switch (savedAs)
		{
		case FLASH_SAVE_VALUE:
			SetValue(pReader->ReadU32(mValue));
			break;
		case FLASH_SAVE_ALL:
			SetMaxValue(pReader->ReadU32(mMaxValue));
			SetMinValue(pReader->ReadU32(mMinValue));
			SetValue(pReader->ReadU32(mValue));
			SetQuantity((QUANTITY_TYPE)pReader->ReadI32(mQuantity));
			break;
		}
	}
};

#endif
