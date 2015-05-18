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
/* CLASS NAME       : U8DataPoint                                           */
/*                                                                          */
/* FILE NAME        : U8DataPoint.h                                         */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __U8DATAPOINT_H__
#define __U8DATAPOINT_H__

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
* CLASS: U8DataPoint
* DESCRIPTION: U8 number data point
*****************************************************************************/
class U8DataPoint : public NumberDataPoint<U8>, public IIntegerDataPoint
{
public:
  /********************************************************************
  * LIFECYCLE - Constructor
  ********************************************************************/
	U8DataPoint() : NumberDataPoint<U8>(false)
	{
	}
	
  /********************************************************************
  * LIFECYCLE - Destructor
  ********************************************************************/
	virtual ~U8DataPoint() 
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
		return SetValue((U8)floatValue);
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
		return SetValue((U8)intValue);
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
	    return SetValue((U8)(boolValue ? 1 : 0));
	}

  /*****************************************************************************
  * Subject::GetFlashId implementation
  *****************************************************************************/
	virtual FLASH_ID_TYPE GetFlashId(void)
	{
		return FLASH_ID_U8_DATA_POINT;
	}

  /*****************************************************************************
  * Subject::ToFlash implementation
  *****************************************************************************/
	virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
	{
		switch (save)
		{
		case FLASH_SAVE_VALUE:
			pWriter->WriteU8(mValue);
			break;
		case FLASH_SAVE_ALL:
			pWriter->WriteU8(mMaxValue);
			pWriter->WriteU8(mMinValue);
			pWriter->WriteU8(mValue);
			pWriter->WriteI32(mQuantity);
			break;
		}
	}

  /*****************************************************************************
  * Subject::FromFlash implementation
  *****************************************************************************/
	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
	{
		switch (savedAs)
		{
		case FLASH_SAVE_VALUE:
			SetValue(pReader->ReadU8(mValue));
			break;
		case FLASH_SAVE_ALL:
			SetMaxValue(pReader->ReadU8(mMaxValue));
			SetMinValue(pReader->ReadU8(mMinValue));
			SetValue(pReader->ReadU8(mValue));
			SetQuantity((QUANTITY_TYPE)pReader->ReadI32(mQuantity));
			break;
		}
	}
};

#endif
