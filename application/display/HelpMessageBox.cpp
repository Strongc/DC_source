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
/* CLASS NAME       : HelpMessageBox                                        */
/*                                                                          */
/* FILE NAME        : HelpMessageBox.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a HelpMessageBox.              */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <EnglishLabel.h>
#include <MpcFonts.h>
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "HelpMessageBox.h"
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
    HelpMessageBox::HelpMessageBox(Component* pParent) : MessageBox(pParent, true)
    {
      SetClientArea(0,34,239,304);

      mpMessageArea = new Group(this);
      mpMessageArea->SetClientArea(6,40,236,269);
      mpMessageArea->SetVisible();
      mpMessageArea->AddChild(mpMessage);

      mpMessage->SetClientArea(0, 0, 230, GetMaxMessageHeight());
      mpMessage->SetColour(GUI_COLOUR_DEFAULT_FOREGROUND);
      mpMessage->SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      AddChild(mpMessageArea);

      mIsFirstRun = true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    HelpMessageBox::~HelpMessageBox()
    {
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void HelpMessageBox::Run()
    {
      MessageBox::Run();

      if (mIsFirstRun)
      {
        mIsFirstRun = false;

        mLedsStatus = COMBINE(NO_LED, OK_LED | ESC_LED | HELP_LED | MPC_HELP_KEY);
        SetAllowFlags(mLegalKeys, MPC_HELP_KEY);        
      }      
    }



    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    bool HelpMessageBox::HandleKeyEvent(Keys key)
    {
      switch (key)
      {
        case MPC_HELP_KEY:
          if (mpMessage->IsBeyondClientArea())
          {
            //use HELP key to show next page
            int height = GetMaxMessageHeight();
            if (mpMessage->GetChildPosY() >= 0)
            {
              mpMessage->SetClientArea(0, -height-1, 230, height);
            }
            else
            {
              mpMessage->SetClientArea(0, 0, 230, height);
            }
          }
          else
          {//use HELP key as Ok key
            key = MPC_OK_KEY;
          }
          
        return true;
      }

      return MessageBox::HandleKeyEvent(key);
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    int HelpMessageBox::GetMaxMessageHeight()
    {
      if (language_dep_font_11 == &Helvetica_57_11)
      {
        // helvetica font is used
        return 219;//= 20 lines * 11 pixel/line - 1
      }
      else
      { // not helvetica font. Assume font height of 13 pixels like chinese font
        return 220;//= 17 lines * 13 pixel/line - 1
      }
    }


  } // namespace display
} // namespace mpc


