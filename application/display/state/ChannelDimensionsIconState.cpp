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
/* CLASS NAME       : ChannelDimensionsIconState                            */
/*                                                                          */
/* FILE NAME        : ChannelDimensionsIconState.cpp                        */
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
#include <ChannelDimensionsIconState.h>
#include "DataPoint.h"

#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowDimensionsRect;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowDimensionsTrap;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowDimensionsCirc;
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mChannelDimensionsIconStateIds[] = { 
        {0, &bmOverflowDimensionsRect},
        {1, &bmOverflowDimensionsTrap},
        {2, &bmOverflowDimensionsCirc}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    ChannelDimensionsIconState::ChannelDimensionsIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mChannelDimensionsIconStateIds;
      mIconIdCount = sizeof( mChannelDimensionsIconStateIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ChannelDimensionsIconState::~ChannelDimensionsIconState()
    {
    }


  } // namespace display
} // namespace mpc


