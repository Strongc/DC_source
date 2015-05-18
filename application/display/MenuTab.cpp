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
/* CLASS NAME       : MenuTab                                               */
/*                                                                          */
/* FILE NAME        : MenuTab.cpp                                           */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a MenuTab.                     */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <DisplayController.h>
#include <MPCFonts.h>
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "MenuTab.h"
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
    MenuTab::MenuTab(Component* pParent) : Label(pParent)
    {
      SetAlign(GUI_TA_VCENTER | GUI_TA_LEFT);
      SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mActive = false;
      mpDisplay = NULL;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    MenuTab::~MenuTab()
    {
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool MenuTab::Redraw()
    {
      bool rc = false;
      if(IsVisible())
      {
        if(!mActive)
        {
          Label::SetColour(GUI_COLOUR_TAB_FOREGROUND);
          Label::SetBackgroundColour(GUI_COLOUR_TAB_BACKGROUND);
        }
        else
        {
          Label::SetColour(GUI_COLOUR_TAB_SELECTED_FOREGROUND);
          Label::SetBackgroundColour(GUI_COLOUR_TAB_SELECTED_BACKGROUND);
        }
        Label::Redraw();
        rc = true;
      }
      return rc;
    }

    void MenuTab::SetActive(bool bActive /*= true*/)
    {
      if (bActive != mActive)
      {
        mActive = bActive;
        Invalidate();
      }

      // Always do this, if we are in a sub display the display should change
      // to the toplevel display.
      if (mpDisplay && mActive)
      {
        DisplayController::GetInstance()->ResetTo(mpDisplay);
      }
    }

    bool MenuTab::IsActive()
    {
      return mActive;
    }

  } // namespace display
} // namespace mpc


