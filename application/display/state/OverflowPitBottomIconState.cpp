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
/* CLASS NAME       : OverflowPitBottomIconState                            */
/*                                                                          */
/* FILE NAME        : OverflowPitBottomIconState.cpp                        */
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
#include <OverflowPitBottomIconState.h>
#include "DataPoint.h"

#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowCemptyPit;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowClevelSensorInPit;
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mOverflowPitBottomIconStateIds[] = { 
        {0, &bmOverflowCemptyPit},// TODO applies to type 1
        {1, &bmOverflowClevelSensorInPit}};// TODO applies to type 2-7

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    OverflowPitBottomIconState::OverflowPitBottomIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mOverflowPitBottomIconStateIds;
      mIconIdCount = sizeof( mOverflowPitBottomIconStateIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    OverflowPitBottomIconState::~OverflowPitBottomIconState()
    {
    }


  } // namespace display
} // namespace mpc


