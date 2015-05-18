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
/* CLASS NAME       : TrendIconState                                        */
/*                                                                          */
/* FILE NAME        : TrendIconState.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 2007-10-16                                            */
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
#include <TrendIconState.h>
#include "DataPoint.h"

#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmTrendUp;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmTrendSteady;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmTrendDown;
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mTrendIconIds[] = { 
        {TREND_RISING, &bmTrendUp},
        {TREND_STEADY, &bmTrendSteady},
        {TREND_FALLING, &bmTrendDown}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    TrendIconState::TrendIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mTrendIconIds;
      mIconIdCount = sizeof( mTrendIconIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    TrendIconState::~TrendIconState()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * If the quality is DP_NEVER_AVAILABLE or DP_NOT_AVAILABLE this function shall return true
    *****************************************************************************/
    bool TrendIconState::IsNeverAvailable()
    {
      IDataPoint* pDP  = dynamic_cast<IDataPoint*>(GetSubject());
      
      if (pDP == NULL)
        return true;

      return pDP->GetQuality() != DP_AVAILABLE;
    }

  } // namespace display
} // namespace mpc


