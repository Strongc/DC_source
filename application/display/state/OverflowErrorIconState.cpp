/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : OverflowErrorIconState                                */
/*                                                                          */
/* FILE NAME        : OverflowErrorIconState.cpp                            */
/*                                                                          */
/* CREATED DATE     : 2008-10-15                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for showing one of two images.                 */
/*                                                                          */
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
#include <OverflowErrorIconState.h>
#include "DataPoint.h"

#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowDerrorSwitch;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowDnoErrorSwitch;
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mOverflowErrorIconStateIds[] = { 
        {0, &bmOverflowDnoErrorSwitch}, // TODO applies to type 1-7 if error switch selected
        {1, &bmOverflowDerrorSwitch}}; // TODO applies to type 1-7 if error switch selected

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    OverflowErrorIconState::OverflowErrorIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mOverflowErrorIconStateIds;
      mIconIdCount = sizeof( mOverflowErrorIconStateIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    OverflowErrorIconState::~OverflowErrorIconState()
    {
    }


  } // namespace display
} // namespace mpc


