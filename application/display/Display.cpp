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
/* CLASS NAME       : DisplayText                                           */
/*                                                                          */
/* FILE NAME        : DisplayText.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <RTOS.h>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#ifdef __PC__
#include <direct.h>
#endif

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <GUI_Utility.h>
#include <GUI.h>
#include <GUIConf.h>

#include "leds.h"
#include "keys.h"

#include "Display.h"

#ifdef __PC__
#include "MenuBar.h"
#include "DisplayController.h"
#include "UpperStatusLine.h"
#endif
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
    Display::Display( display::Group* pRoot, STRING_ID Name, U16 Id, bool AbleToShow /*= true*/, bool Show /*= false*/ )
    {
      mpRoot = pRoot;
      mName = Name;
      mId = Id;
      SetVisible(Show);
      mPasswordId = 0;
      mAbleToShow = AbleToShow;
      strcpy(mDisplayNumber,"1.1");
      mpTitleText = "";
      mFadeX = 240;
      mFadeY = 20;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    Display::~Display()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    int Display::GetId()
    {
      return mId;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    STRING_ID Display::GetName()
    {
      return mName;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void Display::SetName(STRING_ID DisplayTitleId)
    {
      mName = DisplayTitleId;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void Display::SetTitleText(char* pText)
    {
      mpTitleText = pText;
    }

     
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    const char* Display::GetTitleText(void)
    {
      return mpTitleText;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void Display::SetVisible(bool Visible /*= true*/)
    {
      mVisible = Visible;

      // set visible will allocate new WMHandles if the display was previously destroyed
      mpRoot->SetVisible(mVisible);

      if (mVisible)
      {
        WM_BringToTop(mpRoot->GetWMHandle());
      }
      else
      {
        // call destroy to release WMHandles used by the display and its children
        mpRoot->Destroy();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    bool Display::Show()
    {
      SetVisible(true);
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    bool Display::Hide()
    {
      SetVisible(false);
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the AbleToShow flag.
    *****************************************************************************/
    void Display::SetAbleToShow(bool AbleToShow /*= true*/)
    {
      mAbleToShow = AbleToShow;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the AbleToShow flag.
    *****************************************************************************/
    bool Display::GetAbleToShow()
    {
      return mAbleToShow;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool Display::Redraw()
    {
      return mpRoot->Redraw();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the LEDs to turn on
    *****************************************************************************/
    Leds Display::GetLedsStatus(void)
    {
      if (mId == 1) //is home
      {
        Leds leds = mpRoot->GetLedsStatus();
        SetDenyFlags(leds, HOME_LED);
        return leds;
      }
      else
        return mpRoot->GetLedsStatus();      
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the legal keys
    *****************************************************************************/
    Keys Display::GetLegalKeys(void)
    {
      if (mId == 1) //is home
      {
        Keys keys = mpRoot->GetLegalKeys();
        SetDenyFlags(keys, MPC_HOME_KEY);
        return keys;
      }
      else
        return mpRoot->GetLegalKeys();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * A key (with a key id) has been pressed
    *****************************************************************************/
    void Display::HandleKeyEvent(KeyId Key)
    {
      mpRoot->HandleKeyEvent(Key);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void Display::Run(void)
    {
      mpRoot->Run();
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    Group*  Display::GetRoot()
    {
      return mpRoot;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    int Display::GetPasswordId()
    {
      return mPasswordId;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void Display::SetPasswordId( int pw )
    {
      mPasswordId = pw;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void Display::SetDisplayNumber(const char* Number)
    {
      strcpy(mDisplayNumber,Number);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    const char* Display::GetDisplayNumber(void)
    {
      return mDisplayNumber;
    }

#ifdef __FADE__
    void Display::FadeIn(int startx, int starty)
    {
      mFadeX = startx;
      mFadeY = starty;
      /*
      WM_SelectWindow(WM_GetDesktopWindow());
      GUI_ClearRect(0,0,240,320);
      WM_DisableMemdev(mpRoot->GetWMHandle());
      */
      GUI_LOCK();
      /*
      GUI_MEMDEV_Handle h_mem_dev_in = GUI_MEMDEV_CreateEx(0, 33, mpRoot->GetWidth(), mpRoot->GetHeight(), 0);
      GUI_MEMDEV_CopyFromLCD(h_mem_dev_in);
      GUI_MEMDEV_Select(h_mem_dev_in);
      mpRoot->SetChildPos(0, 33);

      Show();

      mpRoot->Invalidate();
      mpRoot->SetStayOnTop();
      mpRoot->ClearArea();
      Run();
      Run();
      Run();
      WM_Paint(mpRoot->GetWMHandle());
      WM_Paint(mpRoot->GetWMHandle());
      WM_SelectWindow(mpRoot->GetWMHandle());
      WM_BringToTop(mpRoot->GetWMHandle());
      //      GUI_SelectLCD();
      GUI_MEMDEV_Select(0);
      */

      mpRoot->SetChildPos(startx,starty);
      Show();

      int y_len = starty - 33;
      int x_pixels = 1;
      for(int i = startx; i > x_pixels; i -= x_pixels)
      {
        mpRoot->SetChildPos(i, 33 + (y_len * ((float)i/startx)));
        mpRoot->Run();
        /*
        WM_Paint(mpRoot->GetWMHandle());
        GUI_ClearRect(i, 33 + (y_len * ((float)i/startx)), 240, 320);
        GUI_MEMDEV_WriteAt(h_mem_dev_in, i, 33 + (y_len * ((float)i/startx)));
        */
        if(x_pixels < 32)
          x_pixels *= 2;
        GUI_Delay(50);
      }
      /*
      GUI_SelectLCD();
      //      WM_EnableMemdev(mpRoot->GetWMHandle());
      GUI_MEMDEV_CopyToLCD(h_mem_dev_in);
      GUI_MEMDEV_Delete(h_mem_dev_in);
      */
      GUI_UNLOCK();
      mpRoot->SetChildPos(0,33);
      mpRoot->Invalidate();
    }

    void Display::FadeOut(Display* to_display)
    {
      Group* root = to_display->GetRoot();
      root->SetVisible(true);
      bool sot = mpRoot->GetStayOnTop();
      mpRoot->SetStayOnTop(true);
      int startx = mpRoot->GetChildPosX();
      int y_len = mFadeY - 33;
      int x_pixels = 1;
      for(int i = startx; i < mFadeX - x_pixels; i += x_pixels)
      {
        mpRoot->SetChildPos(i, 33 + (y_len * ((float)i/mFadeX)));
        root->Invalidate();
        root->Run();
        mpRoot->Run();
        if(x_pixels < 32)
          x_pixels *= 2;
        OS_Delay(50);
      }
      mpRoot->SetStayOnTop(sot);
      root->Invalidate();
    }
#endif // __FADE__



#ifdef __PC__
    void Display::CalculateStringWidths(bool forceVisible)
    {
      // calculate length of display name, shown in upper status line
      UpperStatusLine* upper_status_line = DisplayController::GetInstance()->GetUpperStatusLine();

      upper_status_line->CalculateStringWidths(mName);

      mpRoot->CalculateStringWidths(forceVisible);
    }
#endif

  }

}

