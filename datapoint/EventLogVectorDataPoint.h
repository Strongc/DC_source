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
/* CLASS NAME       : EventLogVectorDataPoint                               */
/*                                                                          */
/* FILE NAME        : EventLogVectorDataPoint.cpp                           */
/*                                                                          */
/* CREATED DATE     : 28-04-2008   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION : A datapoint containing a set of Event loggings. */
/*  The VectorDataPoint is protected by semaphores, so none of its          */
/*  functionallity should be called from within an interrupt rutine.        */
/*                                                                          */
/****************************************************************************/
#ifndef __EVENT_LOG_VECTOR_DATA_POINT_H__
#define __EVENT_LOG_VECTOR_DATA_POINT_H__

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
class EventLogRecord
{
public:
  /********************************************************************
  * LIFECYCLE - Constructor
  ********************************************************************/
  EventLogRecord()
  {
    EventId         = 0;
    EventCode       = 0;
    EventSource     = 0;
    EventType       = 0;
    EventTimeValue  = 0;
  }

  /*****************************************************************************
  * Overload: - Equal operator
  ****************************************************************************/
  bool operator==(const EventLogRecord &right) const
  {
    return (this->EventId         == right.EventId
         && this->EventCode       == right.EventCode
         && this->EventSource     == right.EventSource
         && this->EventType       == right.EventType
         && this->EventTimeValue  == right.EventTimeValue);
  } // ==

  /*****************************************************************************
  * Overload: - Not equal operator
  ****************************************************************************/
  bool operator!=(const EventLogRecord &right) const
  {
    return !(*this == right);
  } // !=

public:
  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
    U16 EventId;
    U8  EventCode;
    U8  EventSource;
    U8  EventType;
    U32 EventTimeValue;
};

/*****************************************************************************
* CLASS: EventLogVectorDataPoint
* DESCRIPTION: I32 vector data point
*****************************************************************************/
class EventLogVectorDataPoint : public VectorDataPoint<EventLogRecord>
{
public:
  /*****************************************************************************
  * LIFECYCLE - Constructor
  * INPUT - initialSize: initial size of the vector
  *****************************************************************************/
  EventLogVectorDataPoint(int initialSize, int maxSize, EventLogRecord defaultValue) : VectorDataPoint<EventLogRecord>(initialSize, maxSize, defaultValue)
  {
  }
  EventLogVectorDataPoint(int initialSize, int maxSize, int initValue) : VectorDataPoint<EventLogRecord>(initialSize, maxSize)
  {
    EventLogRecord init_value;

    init_value.EventCode = (U8)initValue;
    SetDefaultValue(init_value);
    for (int i = 0; i < initialSize; i++)
    {
      SetValue(i, init_value);
    }
  }

  /*****************************************************************************
  * LIFECYCLE - Destructor
  *****************************************************************************/
  virtual ~EventLogVectorDataPoint()
  {
  }

  /*****************************************************************************
  * Subject::GetFlashId implementation
  *****************************************************************************/
  virtual FLASH_ID_TYPE GetFlashId(void)
  {
    return FLASH_ID_EVENT_LOG_VECTOR_DATA_POINT;
  }

  /*****************************************************************************
  * Subject::SaveToFlash implementation
  *****************************************************************************/
  virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
  {
    const int maxSize = GetMaxSize();
    const int size = GetSize();
    const EventLogRecord default_value = GetDefaultValue();
    EventLogRecord temp_value;

    // Write a version code of the data type
    pWriter->WriteU8(1);
    // Write vector sizes
    pWriter->WriteI32(maxSize);
    pWriter->WriteI32(size);
    // Write vector data
    for (int i = 0; i < maxSize; i++)
    {
      if (i < size)
      {
        temp_value = GetValue(i);
        pWriter->WriteU16(temp_value.EventId);
        pWriter->WriteU8( temp_value.EventCode);
        pWriter->WriteU8( temp_value.EventSource);
        pWriter->WriteU8( temp_value.EventType);
        pWriter->WriteU32(temp_value.EventTimeValue);
      }
      else
      {
        pWriter->WriteU16(default_value.EventId);
        pWriter->WriteU8( default_value.EventCode);
        pWriter->WriteU8( default_value.EventSource);
        pWriter->WriteU8( default_value.EventType);
        pWriter->WriteU32(default_value.EventTimeValue);
      }
    }
  }

  /*****************************************************************************
  * Subject::LoadFromFlash implementation
  *****************************************************************************/
  virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
  {
    const U8 version = pReader->ReadU8(0);
    const int savedMaxSize = pReader->ReadI32(-1);
    const int savedSize = pReader->ReadI32(-1);
    EventLogRecord temp_value;

    if (version != 1)
    {
      // Do nothing, since the version code is unknown.
      // This check can be used for backward compatibility handling if a new version of the record is made
    }
    else for (int i = 0; i < savedMaxSize; i++)
    {
      temp_value.EventId        = pReader->ReadU16(0);
      temp_value.EventCode      = pReader->ReadU8(0);
      temp_value.EventSource    = pReader->ReadU8(0);
      temp_value.EventType      = pReader->ReadU8(0);
      temp_value.EventTimeValue = pReader->ReadU32(0);
      if (i < savedSize)
      {
        SetValue(i, temp_value);
      }
    }
  }
};

#endif
