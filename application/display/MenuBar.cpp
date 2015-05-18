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
/* CLASS NAME       : MenuBar                                               */
/*                                                                          */
/* FILE NAME        : MenuBar.cpp                                           */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a MenuBar.                     */
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
#include "MenuBar.h"
#include "DataPoint.h"
#include "Languages.h"
#include "DisplayController.h"

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
    MenuBar::MenuBar(Component* pParent) : Group(pParent)
    {
      SetBackgroundColour(GUI_COLOUR_UPPER_STATUSLINE_BACKGROUND);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    MenuBar::~MenuBar()
    {
    }
    
    void MenuBar::Run()
    {
      if(!IsValid())
      {
        Redraw();
        MenuTab*  p_child = (MenuTab*)Group::GetFirstChild();
        while(p_child)
        {
          p_child->Run();
          p_child = (MenuTab*)Group::GetNextChild(p_child);
        }
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool MenuBar::Redraw()
    {
      bool valid = mValid;
      if(IsVisible())
      {
        WM_BringToTop(GetWMHandle());
        const U16 child_count = Group::GetChildCount();
        const float child_width = GetWidth() / (float)child_count;
        const U16 child_height = GetHeight();

        float x = 0;

        MenuTab*  pChild = (MenuTab*)Group::GetFirstChild();
        while(pChild)
        {
/*          if(pChild->IsActive())
          {
            pChild->SetClientArea( Area( x, 1, x + child_width, child_height) );
          }
          else*/
          {
            pChild->SetClientArea( x, 0, x + child_width, child_height);
          }
          pChild->SetVisible();
          x += child_width;
          if( !valid )
          {
            pChild->Invalidate();
          }
          pChild = (MenuTab*)Group::GetNextChild(pChild);
        }
        Group::Redraw();
        return true;
      }
      Group::Redraw();
      
      return false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Adds a tab to this component.
    *****************************************************************************/
    int MenuBar::AddMenuTab(MenuTab* tab)
    {
      int rc = Group::AddChild(tab);
      if(GetChildCount() == 1)
      {
        tab->SetActive();
        SetCurrentChild(tab);
      }
      Invalidate();
      return rc;
    }

    bool MenuBar::SetActiveTab(MenuTab* tab)
    {
      bool found = false;
      MenuTab*  pChild = (MenuTab*)Group::GetFirstChild();
      int i=0;
      while(pChild)
      {
        ++i;
        if(pChild == tab)
        {
          pChild->SetActive(true);
          SetCurrentChild(pChild);
          found = true;
        }
        else
        {
          pChild->SetActive(false);
        }
        pChild = (MenuTab*)GetNextChild(pChild);
      }
      // Activate first tab if none where found.
      if(!found)
      {
        pChild = (MenuTab*)Group::GetFirstChild();
        pChild->SetActive();
        SetCurrentChild(pChild);
      }
      return found;
    }

    /*****************************************************************************
    * Function - SetActiveTab
    * DESCRIPTION:
    *****************************************************************************/
    bool MenuBar::SetActiveTab(int index)
    {
      MenuTab*  pChild = (MenuTab*)Group::GetFirstChild();
      for (int i = 0; i < index && pChild != NULL; i++)
      {
        pChild = (MenuTab*)Group::GetNextChild(pChild);
      }

      if (pChild)
      {
        return SetActiveTab(pChild);
      }
      return false;
    }

    /*****************************************************************************
    * Function - GetActiveTabIndex
    * DESCRIPTION:
    *****************************************************************************/
    int MenuBar::GetActiveTabIndex()
    {
      int index = 0;
      
      for (; index < mChildren.size(); index++)
      {
        if (mCurrentChild == mChildren[index])
        {
          break;
        }
      }

      return index;
    }

    /* --------------------------------------------------
    * Gets the keys to which we should react in this element and the
    * elements below. Thereby this is the element to send key messages
    * to for the given keys.
    * --------------------------------------------------*/
    Keys MenuBar::GetLegalKeys()
    {
      return MPC_MENU_KEY | MPC_HOME_KEY;
    }



    /* --------------------------------------------------
    * Gets the LEDs which this element and the ones below, wants to be on
    * or off. Normaly this is also the keys to react, but it doesn't have
    * to be this way ...
    * --------------------------------------------------*/
    Leds MenuBar::GetLedsStatus()
    {
      return MENU_LED | HOME_LED;
    }

    /* --------------------------------------------------
    * Handle this key event. Return true if it is taken care of
    * --------------------------------------------------*/
    bool MenuBar::HandleKeyEvent(Keys KeyID)
    {
      switch(KeyID)
      {
        case MPC_MENU_KEY:
        {
          Component* child = GetCurrentChild();
          child = GetNextChild(child);
          SetActiveTab((MenuTab*)child);
          return true;
        }

        case MPC_HOME_KEY:
        {
          DisplayController::GetInstance()->ResetToHome(); 
          return true;
        }
        
        default:
          return false;

      }
    }
  } // namespace display
} // namespace mpc


