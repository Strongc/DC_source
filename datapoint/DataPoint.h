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
/* CLASS NAME       : DataPoint                                             */
/*                                                                          */
/* FILE NAME        : DataPoint.h                                           */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __DATAPOINT_H__
#define __DATAPOINT_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <rtos.h>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Subject.h>
#include <FactoryTypes.h>
#include <UnitTypes.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <IDataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
* CLASS: DataPoint
* DESCRIPTION: Data point template class 
*****************************************************************************/
class DataPoint : public Subject, public virtual IDataPoint
{
protected:
  /*****************************************************************************
  * LIFECYCLE - Default constructor.
  *****************************************************************************/
  DataPoint() 
  {
    mQuality = DP_NEVER_AVAILABLE; 
    mQuantity = Q_NO_UNIT; 
    mWritable = true;
    mRedundant = false;
  }

  /*****************************************************************************
  LIFECYCLE - Destructor.
  *****************************************************************************/
  virtual ~DataPoint() 
  {
  }

public:	
  /*****************************************************************************
  * PUBLIC METHODS
  *****************************************************************************/

  /*****************************************************************************
  * FUNCTION - GetQuality
  * DESCRIPTION:
  *****************************************************************************/
  virtual DP_QUALITY_TYPE GetQuality(void)
  {
    DP_QUALITY_TYPE ret_val;
    OS_EnterRegion();
    ret_val = mQuality;
    OS_LeaveRegion();
    return(ret_val);
  }

  /*****************************************************************************
  * FUNCTION - SetQuality
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetQuality(DP_QUALITY_TYPE quality)
  {
    return SetQuality(quality, true);
  }

  /*****************************************************************************
  * FUNCTION - IsAvailable
  * DESCRIPTION: returns true if quality is DP_AVAILABLE
  *****************************************************************************/
  virtual bool IsAvailable(void)
  {
    return (GetQuality() == DP_AVAILABLE) ? true : false;
  }

  /*****************************************************************************
  * FUNCTION - IsNotAvailable
  * DESCRIPTION: returns true if quality is DP_NOT_AVAILABLE
  *****************************************************************************/
  virtual bool IsNotAvailable(void)
  {
    return (GetQuality() == DP_NOT_AVAILABLE) ? true : false;
  }
  
  /*****************************************************************************
  * FUNCTION - IsNeverAvailable
  * DESCRIPTION: returns true if quality is DP_NEVER_AVAILABLE
  *****************************************************************************/
  virtual bool IsNeverAvailable(void)
  {
    return (GetQuality() == DP_NEVER_AVAILABLE) ? true : false;
  }

  /*****************************************************************************
  * FUNCTION - GetQuantity
  * DESCRIPTION:
  *****************************************************************************/
  virtual QUANTITY_TYPE GetQuantity(void)
  {
    QUANTITY_TYPE ret_val;
    OS_EnterRegion();
    ret_val = mQuantity;
    OS_LeaveRegion();
    return(ret_val);
  }

  /*****************************************************************************
  * FUNCTION - SetQuantity
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetQuantity(QUANTITY_TYPE quantity)
  {
    bool notify = false;
    OS_EnterRegion();
    if( mQuantity != quantity )
    {
      mQuantity = quantity;
      notify = true;
    }
    OS_LeaveRegion();

    if (notify)
      NotifyObservers();

    NotifyObserversE();
    
    return notify;
  }

  /*****************************************************************************
  * FUNCTION - GetWritable
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool GetWritable()
  {
    bool ret_val;
    OS_EnterRegion();
    ret_val = mWritable;
    OS_LeaveRegion();
    return(ret_val);
  }

  /*****************************************************************************
  * FUNCTION - SetWritable
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetWritable(bool writable = true)
  {
    bool notify = false;
		
    OS_EnterRegion();
		
    if (mWritable != writable)
    {
      mWritable = writable;
      notify = true;
    }
		
    OS_LeaveRegion();

    if (notify)
		{
      NotifyObservers();
		}

    NotifyObserversE();
		
		return notify;
  }

  /*****************************************************************************
  * FUNCTION - SetRedundancy
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetRedundancy(bool redundant)
  {
    bool notify = false;
		
    OS_EnterRegion();
		
    if (mRedundant != redundant)
    {
      mRedundant = redundant;
      notify = true;
    }
		
    OS_LeaveRegion();

    if (notify)
		{
      NotifyObservers();
		}

    NotifyObserversE();
		
		return notify;
  }
  
  /*****************************************************************************
  * FUNCTION - IsRedundant
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool IsRedundant(void)
  {
    bool ret_val;
    OS_EnterRegion();
    ret_val = mRedundant;
    OS_LeaveRegion();
    return(ret_val);
  }

protected:
  /*****************************************************************************
  * PROTECTED METHODS
  *****************************************************************************/
  
  /*****************************************************************************
  * FUNCTION - SetQuality
  * DESCRIPTION:
  *****************************************************************************/
  virtual bool SetQuality(DP_QUALITY_TYPE quality, bool notify)
  {
    bool ret_val = false;
    
    OS_EnterRegion();
    if (mQuality != quality)
    {
      mQuality = quality;
      ret_val = true;
    }
    OS_LeaveRegion();

    if (notify)
    {
      if (ret_val)
        NotifyObservers();

      NotifyObserversE();
    }
    
    return ret_val;
  }

  /*****************************************************************************
  * PROTECTED ATTRIBUTES
  *****************************************************************************/
  DP_QUALITY_TYPE mQuality;
  QUANTITY_TYPE   mQuantity;
  bool            mWritable;
  bool            mRedundant;
};

#endif
