/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : MpcLanguagesDataPoint                                 */
/*                                                                          */
/* FILE NAME        : MpcLanguagesdataPoint.cpp                             */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/

#include <Languages.h>
#include <LanguagesDataPoint.h>

 /*****************************************************************************
  CREATS AN OBJECT.
 ******************************************************************************/
LanguagesDataPoint* LanguagesDataPoint::mInstance = 0;


 /*****************************************************************************
  *
  *
  *              PUBLIC FUNCTIONS
  *
  *
  *****************************************************************************/
LanguagesDataPoint* LanguagesDataPoint::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new LanguagesDataPoint();
  }
  return mInstance;
}

 /*****************************************************************************
  * Function - Constructor
  * DESCRIPTION:
  *
  *****************************************************************************/
 LanguagesDataPoint::LanguagesDataPoint()
 {
    SetMaxValue(LAST_LANGUAGE);
    SetMinValue(FIRST_LANGUAGE);
    SetValue(DEFAULT_LANGUAGE);
 }
 /*****************************************************************************
  * Function - Destructor
  * DESCRIPTION:
  *
  ****************************************************************************/
 LanguagesDataPoint::~LanguagesDataPoint()
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



