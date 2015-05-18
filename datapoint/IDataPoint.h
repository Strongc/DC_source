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
/* CLASS NAME       : IDataPoint                                            */
/*                                                                          */
/* FILE NAME        : IDataPoint.h                                          */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION: Data point interface                             */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __IDATAPOINT_H__
#define __IDATAPOINT_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <FactoryTypes.h>
#include <UnitTypes.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
typedef  enum
{
  DP_AVAILABLE,
  DP_NOT_AVAILABLE,
  DP_NEVER_AVAILABLE
} DP_QUALITY_TYPE;

class IDataPoint 
{
protected:
  IDataPoint() {};
  
public:
  virtual ~IDataPoint() {};
  
  virtual bool GetWritable() = 0;
  virtual bool SetWritable(bool writeable = true) = 0;
   
	virtual DP_QUALITY_TYPE GetQuality(void) = 0;
  virtual bool SetQuality(DP_QUALITY_TYPE quality) = 0;
  
  virtual QUANTITY_TYPE GetQuantity(void) = 0;
  virtual bool SetQuantity(QUANTITY_TYPE quantity) = 0;

  virtual bool IsAvailable(void) = 0;
  virtual bool IsNotAvailable(void) = 0;
  virtual bool IsNeverAvailable(void) = 0;
  
  virtual bool SetRedundancy(bool redundant) = 0;
  virtual bool IsRedundant(void) = 0;
};

#endif
