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
/* CLASS NAME       : FloatSwitchConfState                                  */
/*                                                                          */
/* FILE NAME        : FloatSwitchConfState.h                                */
/*                                                                          */
/* CREATED DATE     : 2007-07-24                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text.                        */
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
#include "DataPoint.h"
#include "FloatSwitchConfState.h"
#include <AppTypeDefs.h>

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
    StateStringId mFloatSwitchConfStatesStringIds[] = {
      {FSW_NOT_DEFINED, SID_FS_NOT_DEFINED},
      {FSW_DRY_RUN, SID_FS_DRY_RUN},
      {FSW_DRY_RUN_A, SID_FS_DRY_RUN},
      {FSW_STOP_ALL, SID_FS_STOP_ALL},
      {FSW_STOP, SID_FS_STOP},
      {FSW_STOP1, SID_FS_STOP1},
      {FSW_STOP2, SID_FS_STOP2},
      {FSW_START_STOP, SID_FS_START_STOP},
      {FSW_START1_STOP, SID_FS_START1_STOP},
      {FSW_START, SID_FS_START},
      {FSW_START1, SID_FS_START1},
      {FSW_START2, SID_FS_START2},
      {FSW_START_ALL, SID_FS_START_ALL},
      {FSW_ALARM, SID_FS_ALARM},
      {FSW_HIGH_WATER, SID_FS_HIGH_LEVEL},
      {FSW_HIGH_WATER_A, SID_FS_HIGH_LEVEL},
      {FSW_OVERFLOW, SID_FS_OVERFLOW},
      {FSW_ERROR, SID_FS_ERROR}
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    FloatSwitchConfState::FloatSwitchConfState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mFloatSwitchConfStatesStringIds;
      mStringIdCount = sizeof( mFloatSwitchConfStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    FloatSwitchConfState::~FloatSwitchConfState()
    {
    }

  } // namespace display
} // namespace mpc

