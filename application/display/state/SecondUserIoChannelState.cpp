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
/* CLASS NAME       : SecondUserIoChannelState                              */
/*                                                                          */
/* FILE NAME        : SecondUserIoChannelState.cpp                          */
/*                                                                          */
/* CREATED DATE     : 2009-02-24                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a SecondUserIoChannelState.    */
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
#include "SecondUserIoChannelState.h"

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
    /*****************************************************************************
    * Function - GetStateStringId
    * DESCRIPTION:
    *****************************************************************************/
    U16 SecondUserIoChannelState::GetStateStringId(CHANNEL_SOURCE_TYPE source, U8 index)
    {
      USER_FUNC_SOURCE_TYPE channel = mpUserIoConfig->GetSecondSourceIndex();
      IoChannelConfig* pConfig = mpIoChannelConfigs[channel].GetSubject();

      if (mpChannelConfig.GetSubject() != pConfig)
      {
        mpChannelConfig.UnsubscribeAndDetach(this);
        mpChannelConfig.Attach(pConfig);
      }

      return UserIoSourceState::GetStateStringId(pConfig->GetSource(), pConfig->GetSourceIndex());
    }


  } // namespace display
} // namespace mpc


