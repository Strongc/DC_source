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
/* CLASS NAME       : DoubleVectorDataPoint                                 */
/*                                                                          */
/* FILE NAME        : DoubleVectorDataPoint.cpp                             */
/*                                                                          */
/* CREATED DATE     : 02-03-2012   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION : A datapoint containing a set of double values.  */
/*  The DoubleVectorDataPoint is protected by semaphores, so none of its    */
/*  functionallity should be called from within an interrupt rutine.        */
/*                                                                          */
/****************************************************************************/
#ifndef mrcDoubleVectorDataPoint_h
#define mrcDoubleVectorDataPoint_h

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
* CLASS: DoubleVectorDataPoint
* DESCRIPTION: float vector data point
*****************************************************************************/
class DoubleVectorDataPoint : public VectorDataPoint<double>
{
public:
  /*****************************************************************************
  * LIFECYCLE - Constructor
  * INPUT - initialSize: initial size of the vector
  *****************************************************************************/
	DoubleVectorDataPoint(int initialSize, int maxSize, double defaultValue) : VectorDataPoint<double>(initialSize, maxSize, defaultValue)
  {
  }
  
  /*****************************************************************************
  * LIFECYCLE - Destructor
  *****************************************************************************/
	virtual ~DoubleVectorDataPoint()
  {
  }
  
  /*****************************************************************************
  * Subject::GetFlashId implementation
  *****************************************************************************/
	virtual FLASH_ID_TYPE GetFlashId(void)
	{
		return FLASH_ID_DOUBLE_VECTOR_DATA_POINT;
	}

  /*****************************************************************************
  * Subject::SaveToFlash implementation
  *****************************************************************************/
	virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
	{
    const int maxSize = GetMaxSize();
    const int size = GetSize();
    const double defaultValue = GetDefaultValue();
    
    pWriter->WriteI32(maxSize);
    pWriter->WriteI32(size);
    
    for (int i = 0; i < maxSize; i++)
    {
      if (i < size)
      {
        pWriter->WriteDouble(GetValue(i));
      }
      else
      {
        pWriter->WriteDouble(defaultValue);
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
    const double defaultValue = GetDefaultValue();

    Clear();
    
    for (int i = 0; i < savedMaxSize; i++)
    {
      if (i < savedSize)
      {
        SetValue(i, pReader->ReadDouble(defaultValue));
      }
      else
      {
        pReader->ReadDouble(defaultValue);
      }
    }
	}

};

#endif


