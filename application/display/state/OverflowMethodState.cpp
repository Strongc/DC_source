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
/* CLASS NAME       : OverflowMethodState                                   */
/*                                                                          */
/* FILE NAME        : OverflowMethodState.cpp                               */
/*                                                                          */
/* CREATED DATE     : 2008-11-18                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a OverflowMethodState.         */
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
#include "OverflowMethodState.h"
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
    StateStringId mOverflowMethodStatesStringIds[] = {
      {OVERFLOW_METHOD_NONE,         SID_NOT_USED},
      {OVERFLOW_METHOD_1,            SID_OC_TYPE_1},
      {OVERFLOW_METHOD_2,            SID_OC_TYPE_2},
      {OVERFLOW_METHOD_3,            SID_OC_TYPE_3},
      {OVERFLOW_METHOD_4,            SID_OC_TYPE_4},
      {OVERFLOW_METHOD_5,            SID_OC_TYPE_5},
      {OVERFLOW_METHOD_6,            SID_OC_TYPE_6},
      {OVERFLOW_METHOD_7,            SID_OC_TYPE_7}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    OverflowMethodState::OverflowMethodState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mOverflowMethodStatesStringIds;
      mStringIdCount = sizeof( mOverflowMethodStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    OverflowMethodState::~OverflowMethodState()
    {
    }

  } // namespace display
} // namespace mpc


