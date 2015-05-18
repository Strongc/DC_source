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
/* CLASS NAME       : OverflowPitTopIconState                               */
/*                                                                          */
/* FILE NAME        : OverflowPitTopIconState.cpp                           */
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
#include <OverflowPitTopIconState.h>
#include "DataPoint.h"

#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowAnoSwitch;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowAhighlevelSwitch;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOverflowAoverflowSwitch;
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mOverflowPitTopIconStateIds[] = { 
        {0, &bmOverflowAnoSwitch},// TODO applies to type 2 + 4 + 6
        {1, &bmOverflowAhighlevelSwitch},// TODO applies to type 1
        {2, &bmOverflowAoverflowSwitch}};// TODO applies to type 3 + 5 + 7

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    OverflowPitTopIconState::OverflowPitTopIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mOverflowPitTopIconStateIds;
      mIconIdCount = sizeof( mOverflowPitTopIconStateIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    OverflowPitTopIconState::~OverflowPitTopIconState()
    {
    }


  } // namespace display
} // namespace mpc


