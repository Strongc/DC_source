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
/* CLASS NAME       : SwitchIconState                                       */
/*                                                                          */
/* FILE NAME        : SwitchIconState.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 2007-09-19                                            */
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
#include <SwitchIconState.h>
#include "DataPoint.h"

#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmOpenSwitch;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmClosedSwitch;
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mSwitchIconStatesIds[] = { 
        {0, &bmOpenSwitch},
        {1, &bmClosedSwitch}
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    SwitchIconState::SwitchIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mSwitchIconStatesIds;
      mIconIdCount = sizeof( mSwitchIconStatesIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    SwitchIconState::~SwitchIconState()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * If the quality is DP_NEVER_AVAILABLE or DP_NOT_AVAILABLE this function shall return true
    *****************************************************************************/
    bool SwitchIconState::IsNeverAvailable()
    {
      IDataPoint* pDP  = dynamic_cast<IDataPoint*>(GetSubject());
      
      if (pDP == NULL)
        return true;

      return pDP->GetQuality() != DP_AVAILABLE;
    }

  } // namespace display
} // namespace mpc


