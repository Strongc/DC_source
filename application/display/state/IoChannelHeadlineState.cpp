/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : IoChannelHeadlineState                                */
/*                                                                          */
/* FILE NAME        : IoChannelHeadlineState.cpp                            */
/*                                                                          */
/* CREATED DATE     : 2009-01-05                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a IoChannelHeadlineState.      */
/*                                                                          */
/****************************************************************************/
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
#include "IoChannelHeadlineState.h"
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
    StateStringId mIoChannelHeadlineStatesStringIds[] = {
      {USER_FUNC_SOURCE_1_1,    SID_SET_UP_SOURCE_1},
      {USER_FUNC_SOURCE_2_1,    SID_SET_UP_SOURCE_1},
      {USER_FUNC_SOURCE_3_1,    SID_SET_UP_SOURCE_1},
      {USER_FUNC_SOURCE_4_1,    SID_SET_UP_SOURCE_1},
      {USER_FUNC_SOURCE_5_1,    SID_SET_UP_SOURCE_1},
      {USER_FUNC_SOURCE_6_1,    SID_SET_UP_SOURCE_1},
      {USER_FUNC_SOURCE_7_1,    SID_SET_UP_SOURCE_1},
      {USER_FUNC_SOURCE_8_1,    SID_SET_UP_SOURCE_1},
      {USER_FUNC_SOURCE_1_2,    SID_SET_UP_SOURCE_2},
      {USER_FUNC_SOURCE_2_2,    SID_SET_UP_SOURCE_2},
      {USER_FUNC_SOURCE_3_2,    SID_SET_UP_SOURCE_2},
      {USER_FUNC_SOURCE_4_2,    SID_SET_UP_SOURCE_2},
      {USER_FUNC_SOURCE_5_2,    SID_SET_UP_SOURCE_2},
      {USER_FUNC_SOURCE_6_2,    SID_SET_UP_SOURCE_2},
      {USER_FUNC_SOURCE_7_2,    SID_SET_UP_SOURCE_2},
      {USER_FUNC_SOURCE_8_2,    SID_SET_UP_SOURCE_2}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    IoChannelHeadlineState::IoChannelHeadlineState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mIoChannelHeadlineStatesStringIds;
      mStringIdCount = sizeof( mIoChannelHeadlineStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    IoChannelHeadlineState::~IoChannelHeadlineState()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * protected GetStateStringId made public 
    *****************************************************************************/    
    U16 IoChannelHeadlineState::GetStateStringId(int state)
    {
      return State::GetStateStringId(state);
    }


  } // namespace display
} // namespace mpc


