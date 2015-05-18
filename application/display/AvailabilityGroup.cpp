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
/* CLASS NAME       : State                                                 */
/*                                                                          */
/* FILE NAME        : State.cpp                                             */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This is a group where the availability depends on the childrens          */
/* availability                                                             */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AvailabilityGroup.h"


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
    AvailabilityGroup::AvailabilityGroup(Component* pParent) : Group(pParent)
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AvailabilityGroup::~AvailabilityGroup()
    {
    }

    bool AvailabilityGroup::IsNeverAvailable()
    {
      if(Group::IsNeverAvailable())
        return true;

      Component* p_comp = GetFirstChild();
      while(p_comp != NULL)
      {
        if(p_comp->IsNeverAvailable())
          return true;
        p_comp = GetNextChild(p_comp);
      }
      return false;
    }
  } // namespace display
} // namespace mpc


