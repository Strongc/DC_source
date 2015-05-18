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
/* CLASS NAME       : StateBracket                                          */
/*                                                                          */
/* FILE NAME        : StateBracket.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a StateBracket.                */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <string.h>
#include "StateBracket.h"
#include "IIntegerDataPoint.h"
#include "Languages.h"

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

    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    StateBracket::StateBracket(Component* pParent): State(pParent)
    {
    }
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    StateBracket::~StateBracket()
    {
    }

    /*****************************************************************************
    * Function - Redraw
    * DESCRIPTION:
    *****************************************************************************/
    bool StateBracket::Redraw()
    {
      char text[MAX_STATE_TEXT_LENGTH];
      Component::Redraw();
      if (mVisible)
      {
        strcpy(text, Languages::GetInstance()->GetString(SID_DIGITAL_STATE_OPEN_BRACKET));
        IIntegerDataPoint* pDP  = dynamic_cast<IIntegerDataPoint*>(GetSubject()); 

        if (pDP != NULL && pDP->IsAvailable())
        {
          const char* state_as_string = GetStateAsString(pDP->GetAsInt());
		      strcat(text, state_as_string);
        }
        else
        {
   			  strcat(text, "--");
        }

        strcat(text, Languages::GetInstance()->GetString(SID_DIGITAL_STATE_CLOSING_BRACKET));
        SetText(text);

        ObserverText::Redraw();
        return true;
      }
      return false;
    }

  } // namespace display
} // namespace mpc


