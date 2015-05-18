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
/* CLASS NAME       : CheckBox                                              */
/*                                                                          */
/* FILE NAME        : CheckBox.cpp                                          */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a CheckBox.                    */
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
#include "CheckBox.h"
#include <Image.h>
/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

extern "C" GUI_CONST_STORAGE GUI_BITMAP bmCheck;

namespace mpc
{
  namespace display
  {

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    CheckBox::CheckBox(Component* pParent) : Group(pParent)
    {
      mChecked = false;
      mpCheckImage = new Image(this);
      mpCheckImage->SetBitmap(&bmCheck);
      AddChild(mpCheckImage);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    CheckBox::~CheckBox()
    {
      delete mpCheckImage;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the check state of the check box. Returning the previous check state.
    *****************************************************************************/
    bool CheckBox::SetCheck(bool check /*= true*/)
    {
      if( mChecked != check )
      {
        bool oldCheck = mChecked;
        mChecked = check;
        mpCheckImage->SetVisible(mChecked);
        Invalidate();
        return oldCheck;
      }
      return mChecked;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns the current checkstate of the checkbox.
    *****************************************************************************/
    bool CheckBox::IsChecked()
    {
      return mChecked;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool CheckBox::Redraw()
    {
      if(IsVisible())
      {
        Component::Redraw();
        int x1 = GetWidth() / 2 - 9;
        int y1 = GetHeight() / 2 - 4;
        int x2 = x1 + 9;
        int y2 = y1 + 9;

        mpCheckImage->SetClientArea(x1,y1,x2,y2);
        mpCheckImage->SetReadOnly(mReadOnly);

        x1 -= 2;
        y1 -= 1;
        x2 += 1;
        y2 += 1;
        SelectWindow();
        GUI_SetColor(GetColour());
        GUI_DrawRect(x1,y1,x2,y2);
        return true;
      }
      return false;
    }

    Keys CheckBox::GetLegalKeys()
    {
      if(IsReadOnly())
        return MPC_NO_KEY;

      mLegalKeys = MPC_OK_KEY;
      return mLegalKeys;
    }

    bool CheckBox::HandleKeyEvent(Keys keyId)
    {
      if(IsReadOnly())
        return false;
      switch(keyId)
      {
        case MPC_OK_KEY:
          SetCheck(!IsChecked());
          return true;
      }
      return Component::HandleKeyEvent(keyId);
    }


    Leds CheckBox::GetLedsStatus()
    {
      if(IsReadOnly())
      {
        return COMBINE(NO_LED,NO_LED);
      }
      return COMBINE(NO_LED,OK_LED);
    }



  } // namespace display
} // namespace mpc


