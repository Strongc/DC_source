/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : AnalogOutputFunctionState                             */
/*                                                                          */
/* FILE NAME        : AnalogOutputFunctionState.cpp                         */
/*                                                                          */
/* CREATED DATE     : 2009-05-18                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AnalogOutputFunctionState.   */
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
#include <AnaInOnIoCtrl.h>
#include "AnalogOutputFunctionState.h"


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
    StateStringId mAnalogOutputFunctionStatesStringIds[] =
    {
      { ANA_OUT_FUNC_NO_FUNCTION,    SID_NOT_USED                            },
      { ANA_OUT_FUNC_LEVEL,          SID_AO_LEVEL                            },
      { ANA_OUT_FUNC_VFD_1,          SID_AO_VFD_FREQUENCY_PUMP_1             },
      { ANA_OUT_FUNC_VFD_2,          SID_AO_VFD_FREQUENCY_PUMP_2             },
      { ANA_OUT_FUNC_VFD_3,          SID_AO_VFD_FREQUENCY_PUMP_3             },
      { ANA_OUT_FUNC_VFD_4,          SID_AO_VFD_FREQUENCY_PUMP_4             },
      { ANA_OUT_FUNC_VFD_5,          SID_AO_VFD_FREQUENCY_PUMP_5             },
      { ANA_OUT_FUNC_VFD_6,          SID_AO_VFD_FREQUENCY_PUMP_6             },
      { ANA_OUT_FUNC_USER_DEFINED_1, SID_AO_USER_DEFINED_1                   },
      { ANA_OUT_FUNC_USER_DEFINED_2, SID_AO_USER_DEFINED_2                   },
      { ANA_OUT_FUNC_USER_DEFINED_3, SID_AO_USER_DEFINED_3                   }

    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AnalogOutputFunctionState::AnalogOutputFunctionState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mAnalogOutputFunctionStatesStringIds;
      mStringIdCount = sizeof( mAnalogOutputFunctionStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AnalogOutputFunctionState::~AnalogOutputFunctionState()
    {
    }

  } // namespace display
} // namespace mpc




