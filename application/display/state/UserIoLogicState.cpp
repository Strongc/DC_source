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
/* CLASS NAME       : UserIoLogicState                                      */
/*                                                                          */
/* FILE NAME        : UserIoLogicState.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 2009-01-05                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a UserIoLogicState.            */
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
#include "UserIoLogicState.h"
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
    StateStringId mUserIoLogicStatesStringIds[] = {
      {USER_IO_LOGIC_AND,               SID_AND},
      {USER_IO_LOGIC_OR,                SID_OR},
      {USER_IO_LOGIC_XOR,               SID_XOR},
      {USER_IO_LOGIC_SET_RESET_LATCH,   SID_SR_LATCH},
      {USER_IO_LOGIC_RESET_SET_LATCH,   SID_RS_LATCH},
      {USER_IO_LOGIC_TOGGLE_LATCH,      SID_T_LATCH}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    UserIoLogicState::UserIoLogicState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mUserIoLogicStatesStringIds;
      mStringIdCount = sizeof( mUserIoLogicStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    UserIoLogicState::~UserIoLogicState()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * protected GetStateStringId made public 
    *****************************************************************************/    
    U16 UserIoLogicState::GetStateStringId(int state)
    {
      return State::GetStateStringId(state);
    }


  } // namespace display
} // namespace mpc


