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
/* CLASS NAME       : U16DataPoint                                          */
/*                                                                          */
/* FILE NAME        : U16DataPoint.h                                        */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __U16DATAPOINT_H__
#define __U16DATAPOINT_H__

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
* CLASS: I16DataPoint
* DESCRIPTION: I16 number data point
*****************************************************************************/
class U16DataPoint : public NumberDataPoint<U16>, public IIntegerDataPoint
{
public:
  /*****************************************************************************
  * LIFECYCLE - Constructor
  *****************************************************************************/
	U16DataPoint() : NumberDataPoint<U16>(false)
	{
	}
	
  /*****************************************************************************
  * LIFECYCLE - Destructor
  *****************************************************************************/
	virtual ~U16DataPoint() 
	{
	}

public:  
  /*****************************************************************************
  * PUBLIC METHODS
  *****************************************************************************/
  
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
		return SetValue((U16)floatValue);
	}

  /*****************************************************************************
  * IIntegerDataPoint::GetAsInt / INumberDataPoint::GetAsInt implementation
  *****************************************************************************/
	virtual int GetAsInt(void)
	{
		return (int)GetValue();
	}

  /*****************************************************************************
  * IIntegerDataPoint::SetAsInt / INumberDataPoint::SetAsInt implementation
  *****************************************************************************/
	virtual bool SetAsInt(int intValue)
	{
		return SetValue((U16)intValue);
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
	  return SetValue((U16)(boolValue ? 1 : 0));
	}

  /*****************************************************************************
  * Subject::GetFlashId implementation
  *****************************************************************************/
	virtual FLASH_ID_TYPE GetFlashId(void)
	{
		return FLASH_ID_U16_DATA_POINT;
	}

  /*****************************************************************************
  * Subject::SaveToFlash implementation
  *****************************************************************************/
	virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
	{
		switch (save)
		{
		case FLASH_SAVE_VALUE:
			pWriter->WriteU16(mValue);
			break;
		case FLASH_SAVE_ALL:
			pWriter->WriteU16(mMaxValue);
			pWriter->WriteU16(mMinValue);
			pWriter->WriteU16(mValue);
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
			SetValue(pReader->ReadU16(mValue));
			break;
		case FLASH_SAVE_ALL:
			SetMaxValue(pReader->ReadU16(mMaxValue));
			SetMinValue(pReader->ReadU16(mMinValue));
			SetValue(pReader->ReadU16(mValue));
			SetQuantity((QUANTITY_TYPE)pReader->ReadI32(mQuantity));
			break;
		}
	}
};

#endif
