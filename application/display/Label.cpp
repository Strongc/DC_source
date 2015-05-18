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
/* CLASS NAME       : Label                                                 */
/*                                                                          */
/* FILE NAME        : Label.cpp                                             */
/*                                                                          */
/* CREATED DATE     : 2004-09-14                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a label.                       */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef __PC__
extern bool g_show_string_ids;
#include "DisplayDumper.h"
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
#include <GUI_Utility.h>
#include <DisplayController.h>
#include <Display.h>
#include <Leds.h>
#include <Keys.h>
#include <String_id.h>
#include <Observer.h>
#include <Subject.h>
#include <DataPoint.h>
#include <Languages.h>
#include "Label.h"
#include <GUI.h>
#include <GUIConf.h>
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
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    Label::Label(Component* pParent /*=NULL*/) : ObserverText(pParent)
    {
      mStringId = SID_NONE;
      Languages::GetInstance()->Subscribe(this);
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    Label::~Label()
    {
      Languages::GetInstance()->Unsubscribe(this);
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool Label::Redraw()
    {
      if(!mValid && IsVisible())
      {
#ifdef __PC__
        if(g_show_string_ids)
        {
          char szTmp[200];
          sprintf(szTmp, "%d", mStringId);
          ObserverText::SetText(szTmp);
        }
        else
        {
#endif
        const char* the_string_to_show = Languages::GetInstance()->GetString(mStringId);
        ObserverText::SetText(the_string_to_show);
#ifdef __PC__
        }
#endif
      }
      return ObserverText::Redraw();
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    STRING_ID Label::GetStringId()
    {
      return mStringId;
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void Label::SetStringId(STRING_ID StringId)
    {
      if(mStringId != StringId)
      {
        mStringId = StringId;
        const char* the_string_to_show = Languages::GetInstance()->GetString(mStringId);
        ObserverText::SetText(the_string_to_show);
      }
    }

    void Label::SetText(const char* pText)
    {
      ObserverText::SetText(pText);
    }


#ifdef __PC__
    void Label::CalculateStringWidths(bool forceVisible)
    {
      Text::CalculateStringWidths(forceVisible, mStringId);
    }
#endif

  } // namespace display
} // namespace mpc
