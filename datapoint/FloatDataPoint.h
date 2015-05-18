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
/* CLASS NAME       : FloatDataPoint                                        */
/*                                                                          */
/* FILE NAME        : FloatDataPoint.h                                      */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __FLOATDATAPOINT_H__
#define __FLOATDATAPOINT_H__

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

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
* CLASS: FloatDataPoint
* DESCRIPTION: Float number data point
*****************************************************************************/
class FloatDataPoint : public NumberDataPoint<float>
{
public:
  /********************************************************************
  * LIFECYCLE - Constructor
  ********************************************************************/
	FloatDataPoint() : NumberDataPoint<float>(true)
	{
	}

  /********************************************************************
  * LIFECYCLE - Destructor
  ********************************************************************/
	virtual ~FloatDataPoint()
	{
	}
  
  /*****************************************************************************
  * INumberDataPoint::GetAsFloat implementation
  *****************************************************************************/
	virtual float GetAsFloat(void)
	{
		return GetValue();
	}

  /*****************************************************************************
  * INumberDataPoint::SetAsFloat implementation
  *****************************************************************************/
	virtual bool SetAsFloat(float floatValue) 
	{
		return SetValue(floatValue);
	}

  /*****************************************************************************
  * INumberDataPoint::GetAsInt implementation
  *****************************************************************************/
	virtual int GetAsInt(void)
	{
		return (int)GetValue();
	}

  /*****************************************************************************
  * INumberDataPoint::GetAsInt implementation
  *****************************************************************************/
	virtual bool SetAsInt(int intValue)
	{
		return SetValue((float)intValue);
	}

  /*****************************************************************************
  * Subject::GetFlashId implementation
  *****************************************************************************/
	virtual FLASH_ID_TYPE GetFlashId(void)
	{
		return FLASH_ID_FLOAT_DATA_POINT;
	}

  /*****************************************************************************
  * Subject::SaveToFlash implementation
  *****************************************************************************/
	virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
	{
		switch (save)
		{
		case FLASH_SAVE_VALUE:
			pWriter->WriteFloat(mValue);
			break;
		case FLASH_SAVE_ALL:
			pWriter->WriteFloat(mMaxValue);
			pWriter->WriteFloat(mMinValue);
			pWriter->WriteFloat(mValue);
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
			SetValue(pReader->ReadFloat(mValue));
			break;
		case FLASH_SAVE_ALL:
			SetMaxValue(pReader->ReadFloat(mMaxValue));
			SetMinValue(pReader->ReadFloat(mMinValue));
			SetValue(pReader->ReadFloat(mValue));
			SetQuantity((QUANTITY_TYPE)pReader->ReadI32(mQuantity));
			break;
		}
	}
};

#endif
