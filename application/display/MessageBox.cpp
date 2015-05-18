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
/* CLASS NAME       : MessageBox                                            */
/*                                                                          */
/* FILE NAME        : MessageBox.cpp                                        */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
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
#include "MessageBox.h"
#include <Factory.h>
#include <DisplayController.h>
#include <MpcFonts.h>
#include <Label.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
EXTERNS
*****************************************************************************/
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmHelp;

namespace mpc
{
  namespace display
  {
  /*****************************************************************************
  *
  *
  *              PUBLIC FUNCTIONS
  *
  *
    *****************************************************************************/
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    MessageBox::MessageBox(Component* pParent, bool autoDelete /*=true*/) : PopupBox( pParent )
    {
      mAutoDelete = autoDelete; // Delete on OK or ESC key

      SetClientArea(2,34,237,304);

      mpTitle = new Label(this);
      mpTitle->SetStringId(SID_NONE);
      mpTitle->SetClientArea(3,3,195,39);
      mpTitle->SetWordWrap();
      mpTitle->SetVisible();
      AddChild(mpTitle);

      mpImage = new Image(this);
      mpImage->SetClientArea(200,2,231,39);
      mpImage->SetVisible();
      AddChild(mpImage);

      mpMessage = new Label(this);
      mpMessage->SetStringId(SID_NONE);
      mpMessage->SetClientArea(3,40,234,269);
      mpMessage->SetWordWrap();
      mpMessage->SetFont(DEFAULT_FONT_11_LANGUAGE_DEP);
      mpMessage->SetAlign(GUI_TA_TOP|GUI_TA_LEFT);
      mpMessage->SetVisible();
      AddChild(mpMessage);

      SetAllowFlags(mLegalKeys, MPC_OK_KEY | MPC_ESC_KEY);
      mLedsStatus = COMBINE(NO_LED,OK_LED | ESC_LED);
    }
    
    MessageBox::~MessageBox()
    {
      delete mpTitle;
      delete mpImage;
      delete mpMessage;
    }


    void MessageBox::SetTitle(STRING_ID title)
    {
      mpTitle->SetStringId(title);
    }

    void MessageBox::SetMessage(STRING_ID message)
    {
      mpMessage->SetStringId(message);

    }

    void MessageBox::SetStyle(MB_STYLE style)
    {
      switch(style)
      {
        case MB_STYLE_NONE:
          mpImage->SetBitmap(NULL);
        break;
        case MB_STYLE_HELP:
          mpImage->SetBitmap(&bmHelp);
        break;
        case MB_STYLE_INFORMATION:
          break;
        case MB_STYLE_WARNING:
          break;
        case MB_STYLE_ERROR: // Fall through
//        case MB_STYLE_CRITICAL:
          break;
      }
    }

    bool MessageBox::HandleKeyEvent(Keys KeyID)
    {
      switch(KeyID)
      {
        case MPC_OK_KEY:
        case MPC_ESC_KEY:
          DisplayController::GetInstance()->PopPopupBox();
          if(mAutoDelete)
          {
            delete this;
          }
          return true;
      }
      return false;
    }
    
  } // namespace display
} // namespace mpc
