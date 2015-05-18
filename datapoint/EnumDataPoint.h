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
/* CLASS NAME       : EnumDataPoint                                         */
/*                                                                          */
/* FILE NAME        : EnumDataPoint.h                                       */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __ENUMDATAPOINT_H__
#define __ENUMDATAPOINT_H__

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
* CLASS: EnumDataPoint
* DESCRIPTION: Enum data point template class
*****************************************************************************/
template<typename VAL_TYPE>
class EnumDataPoint : public MinMaxDataPoint<VAL_TYPE>, public IIntegerDataPoint
{
public:
  /********************************************************************
  * LIFECYCLE - Constructor
  ********************************************************************/
 	EnumDataPoint() : MinMaxDataPoint<VAL_TYPE>()
	{
		mEnumType = ENUM_TYPE_UNKNOWN;
	}

  /********************************************************************
  * LIFECYCLE - Constructor
  ********************************************************************/
	EnumDataPoint(ENUM_TYPE enumType) : MinMaxDataPoint<VAL_TYPE>()
	{
		mEnumType = enumType;
	}

  /********************************************************************
  * LIFECYCLE - Destructor
  ********************************************************************/
	virtual ~EnumDataPoint()
	{
	}

  /*****************************************************************************
  * PUBLIC METHODS
  *****************************************************************************/

  /*****************************************************************************
  * FUNCTION - GetEnumType
  * DESCRIPTION: Returns the enum type or ENUM_TYPE_UNKNOWN if the data point
  * was constructed using the default constructor.
  * The ENUM_TYPE enumeration is created by the FactoryGenerator.
  *****************************************************************************/
	ENUM_TYPE GetEnumType(void)
	{
	    return mEnumType;
	}

  /*****************************************************************************
  * IIntegerDataPoint::GetAsInt implementation
  *****************************************************************************/
	virtual int GetAsInt(void)
	{
		return (int)GetValue();
	}

  /*****************************************************************************
  * IIntegerDataPoint::SetAsInt implementation
  *****************************************************************************/
	virtual bool SetAsInt(int intValue)
	{
		return SetValue((VAL_TYPE)intValue);
	}

  /*****************************************************************************
  * IIntegerDataPoint::GetAsBool implementation
  *****************************************************************************/
	virtual bool GetAsBool(void)
	{
        FatalErrorOccured("Getting enum as a bool!");
        return 0;  // Return value required - 0 is returned to avoid compiler error.
	}

  /*****************************************************************************
  * IIntegerDataPoint::SetAsBool implementation
  *****************************************************************************/
	virtual bool SetAsBool(bool boolValue)
	{
        FatalErrorOccured("Setting enum as a bool!");
        return 0;  // Return value required - 0 is indicating error.
	}

    /*****************************************************************************
  * Subject::GetFlashId implementation
  *****************************************************************************/
	virtual FLASH_ID_TYPE GetFlashId(void)
	{
		return FLASH_ID_ENUM_DATA_POINT;
	}

  /*****************************************************************************
  * Subject::SaveToFlash implementation
  *****************************************************************************/
	virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
	{
    pWriter->WriteI32(mValue);
	}

  /*****************************************************************************
  * Subject::LoadFromFlash implementation
  *****************************************************************************/
	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
	{
    SetValue((VAL_TYPE)pReader->ReadI32((I32)mValue));
	}

  /*****************************************************************************
  * PRIVATE ATTRIBUTES
  *****************************************************************************/
private:
	ENUM_TYPE mEnumType;
};

#endif
