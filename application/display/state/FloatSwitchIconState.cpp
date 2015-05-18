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
/* CLASS NAME       : FloatSwitchIconState                                  */
/*                                                                          */
/* FILE NAME        : FloatSwitchIconState.cpp                              */
/*                                                                          */
/* CREATED DATE     : 2007-07-24                                            */
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
#include <FloatSwitchIconState.h>
#include "DataPoint.h"

#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmFloatSwitchOn;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmFloatSwitchOff;
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mFloatSwitchStatesIconIds[] = { 
        {DIGITAL_INPUT_FUNC_STATE_NOT_ACTIVE, &bmFloatSwitchOff},
        {DIGITAL_INPUT_FUNC_STATE_ACTIVE, &bmFloatSwitchOn}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    FloatSwitchIconState::FloatSwitchIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mFloatSwitchStatesIconIds;
      mIconIdCount = sizeof( mFloatSwitchStatesIconIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    FloatSwitchIconState::~FloatSwitchIconState()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * If the quality is DP_NEVER_AVAILABLE or DP_NOT_AVAILABLE this function shall return true
    *****************************************************************************/
    bool FloatSwitchIconState::IsNeverAvailable()
    {
      IDataPoint* pDP  = dynamic_cast<IDataPoint*>(GetSubject());
      
      if (pDP == NULL)
        return true;

      return pDP->GetQuality() != DP_AVAILABLE;
    }

  } // namespace display
} // namespace mpc


