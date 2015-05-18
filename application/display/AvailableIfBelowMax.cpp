/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW-Midrange                                      */
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
/* CLASS NAME       : AvailableIfBelowMax                                   */
/*                                                                          */
/* FILE NAME        : AvailableIfBelowMax.cpp                               */
/*                                                                          */
/* CREATED DATE     : 2007-07-24                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible...                                             */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AvailableIfBelowMax.h"
#include <DataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AvailableIfBelowMax::AvailableIfBelowMax(Component* pParent) : AvalibleIfSet(pParent)
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AvailableIfBelowMax::~AvailableIfBelowMax()
    {
    }

    bool AvailableIfBelowMax::IsNeverAvailable()
    {
      if (mpDataPoint.IsValid())
      {
        bool is_set = false;
        std::vector< int >::iterator iter = mCheckStates.begin();
        std::vector< int >::iterator iterEnd = mCheckStates.end();
        for(; iter != iterEnd; ++iter )
        {
          if(mpDataPoint->GetMaxAsInt() < *iter)
          {
            is_set = true;
            break;
          }
        }

        if (mInverted)
          return !is_set;
        else
          return is_set;
      }
      
      return false;
    }

  } // namespace display
} // namespace mpc


