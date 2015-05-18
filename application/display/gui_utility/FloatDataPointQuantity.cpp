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
/* FILE NAME        : FloatDataPointQuantity.cpp                            */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

#include <FloatDataPointQuantity.h>
#include <math.h>
#include <Languages.h>

namespace mpc
{
  namespace display
  {

  void FloatDataPointQuantity(char* str,FloatDataPoint* pData)
  {
    const char* the_string_to_show = Languages::GetInstance()->GetString(MpcUnits::GetInstance()->GetActualUnitString(pData->GetQuantity()));
    strcpy(str,the_string_to_show);
  }

  }
}




