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
/* CLASS NAME       : TimeFormatDataPoint                                   */
/*                                                                          */
/* FILE NAME        : TimeFormatDataPoint.cpp                               */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/

#include <TimeFormatDataPoint.h>

 /*****************************************************************************
  CREATS AN OBJECT.
 ******************************************************************************/
TimeFormatDataPoint* TimeFormatDataPoint::mInstance = 0;


 /*****************************************************************************
  *
  *
  *              PUBLIC FUNCTIONS
  *
  *
  *****************************************************************************/
TimeFormatDataPoint* TimeFormatDataPoint::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new TimeFormatDataPoint();
  }
  return mInstance;
}

 /*****************************************************************************
  * Function - Constructor
  * DESCRIPTION:
  *
  *****************************************************************************/
 TimeFormatDataPoint::TimeFormatDataPoint()
 {
    SetMaxValue(2);
    SetMinValue(0);
    SetValue(0);
 }
 /*****************************************************************************
  * Function - Destructor
  * DESCRIPTION:
  *
  ****************************************************************************/
 TimeFormatDataPoint::~TimeFormatDataPoint()
 {
 }

 /*****************************************************************************
  *
  *
  *              PRIVATE FUNCTIONS
  *
  *
  ****************************************************************************/

 /*****************************************************************************
  *
  *
  *              PROTECTED FUNCTIONS
  *                 - RARE USED -
  *
  ****************************************************************************/



