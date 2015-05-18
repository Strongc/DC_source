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
/* CLASS NAME       : UserIoState                                           */
/*                                                                          */
/* FILE NAME        : UserIoState.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 2008-12-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a UserIoState.                 */
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
#include "UserIoState.h"
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
    StateStringId mUserIoStatesStringIds[] = {
      {USER_IO_1,    SID_USERDEFINED_FUNCTION_1},
      {USER_IO_2,    SID_USERDEFINED_FUNCTION_2},
      {USER_IO_3,    SID_USERDEFINED_FUNCTION_3},
      {USER_IO_4,    SID_USERDEFINED_FUNCTION_4},
      {USER_IO_5,    SID_USERDEFINED_FUNCTION_5},
      {USER_IO_6,    SID_USERDEFINED_FUNCTION_6},
      {USER_IO_7,    SID_USERDEFINED_FUNCTION_7},
      {USER_IO_8,    SID_USERDEFINED_FUNCTION_8}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    UserIoState::UserIoState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mUserIoStatesStringIds;
      mStringIdCount = sizeof( mUserIoStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    UserIoState::~UserIoState()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * protected GetStateStringId made public 
    *****************************************************************************/    
    U16 UserIoState::GetStateStringId(int state)
    {
      return State::GetStateStringId(state);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * returns display number used by slip point control to set dynamic menu title
    *****************************************************************************/    
    void UserIoState::GetStateDisplayNumber(int state, char* displayNumber)
    {
      if (state >= FIRST_USER_IO && state <= LAST_USER_IO)
      {
        sprintf(displayNumber, "4.2.9.%i", state); // Settings->adv. functions->userIO
      }
      else 
      {
        FatalErrorOccured("User IO state OOR error");
      }      
    }

  } // namespace display
} // namespace mpc


