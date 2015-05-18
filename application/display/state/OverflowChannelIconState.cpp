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
/* CLASS NAME       : OverflowChannelIconState                              */
/*                                                                          */
/* FILE NAME        : OverflowChannelIconState.cpp                          */
/*                                                                          */
/* CREATED DATE     : 2008-10-15                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for showing one of three images.               */
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
#include <OverflowChannelIconState.h>
#include "DataPoint.h"

#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowBemptyChannel;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowBlevelSensorInChannel;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowBflowSensorInChannel;
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mOverflowChannelIconStateIds[] = { 
        {0, &bmOverflowBemptyChannel}, // TODO applies to type 1-3
        {1, &bmOverflowBlevelSensorInChannel},// TODO applies to type 4-5
        {2, &bmOverflowBflowSensorInChannel}};// TODO applies to type 6-7

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    OverflowChannelIconState::OverflowChannelIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mOverflowChannelIconStateIds;
      mIconIdCount = sizeof( mOverflowChannelIconStateIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    OverflowChannelIconState::~OverflowChannelIconState()
    {
    }


  } // namespace display
} // namespace mpc


