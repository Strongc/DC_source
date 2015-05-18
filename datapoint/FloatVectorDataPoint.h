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
/* CLASS NAME       : FloatVectorDataPoint                                  */
/*                                                                          */
/* FILE NAME        : FloatVectorDataPoint.cpp                              */
/*                                                                          */
/* CREATED DATE     : 30-05-2007   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION : A datapoint containing a set of int values.     */
/*  The IntVectorDataPoint is protected by semaphores, so none of its       */
/*  functionallity should be called from within an interrupt rutine.        */
/*                                                                          */
/****************************************************************************/
#ifndef __FLOAT_VECTOR_DATA_POINT_H__
#define __FLOAT_VECTOR_DATA_POINT_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <rtos.h>
#include <vector>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
*****************************************************************************/
#include <VectorDataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPEDEFS
*****************************************************************************/

/*****************************************************************************
* CLASS: FloatVectorDataPoint
* DESCRIPTION: float vector data point
*****************************************************************************/
class FloatVectorDataPoint : public VectorDataPoint<float>
{
public:
  /*****************************************************************************
  * LIFECYCLE - Constructor
  * INPUT - initialSize: initial size of the vector
  *****************************************************************************/
	FloatVectorDataPoint(int initialSize, int maxSize, float defaultValue) : VectorDataPoint<float>(initialSize, maxSize, defaultValue)
  {
  }
  
  /*****************************************************************************
  * LIFECYCLE - Destructor
  *****************************************************************************/
	virtual ~FloatVectorDataPoint()
  {
  }
  
  /*****************************************************************************
  * Subject::GetFlashId implementation
  *****************************************************************************/
	virtual FLASH_ID_TYPE GetFlashId(void)
	{
		return FLASH_ID_FLOAT_VECTOR_DATA_POINT;
	}

  /*****************************************************************************
  * Subject::SaveToFlash implementation
  *****************************************************************************/
	virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
	{
    const int maxSize = GetMaxSize();
    const int size = GetSize();
    const I32 defaultValue = GetDefaultValue();
    
    pWriter->WriteI32(maxSize);
    pWriter->WriteI32(size);
    
    for (int i = 0; i < maxSize; i++)
    {
      if (i < size)
      {
        pWriter->WriteFloat(GetValue(i));
      }
      else
      {
        pWriter->WriteFloat(defaultValue);
      }
    }
	}

  /*****************************************************************************
  * Subject::LoadFromFlash implementation
  *****************************************************************************/
	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
	{
    const int savedMaxSize = pReader->ReadI32(-1);
    const int savedSize = pReader->ReadI32(-1);
    const I32 defaultValue = GetDefaultValue();

    Clear();
    
    for (int i = 0; i < savedMaxSize; i++)
    {
      if (i < savedSize)
      {
        SetValue(i, pReader->ReadFloat(defaultValue));
      }
      else
      {
        pReader->ReadFloat(defaultValue);
      }
    }
	}

};

#endif


