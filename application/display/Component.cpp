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
/* CLASS NAME       : Component                                             */
/*                                                                          */
/* FILE NAME        : Component.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 2004-08-30                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a given element.               */
/* All graphical elements are sub-types of this class and they form the     */
/* basic GUI elements all responsible for drawing themselves acoording to   */
/* the given actual situation.                                              */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef __PC__
#include "StringWidthCalculator.h"
#endif

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <list>
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "WM.h"
#include <MpcFonts.h>
#include "GUI_Utility.h"
#include "Leds.h"
#include "Keys.h"
#include "String_id.h"
#include "Observer.h"
#include "Subject.h"

/*****************************************************************************
LOCAL INCLUDES
**************************************************************************/
#include "Component.h"
#include <Display.h>
#include <DisplayController.h>
#include <HelpMessageBox.h>

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
    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    **************************************************************************/
    Component::Component( Component* pParent /*= NULL*/)
    {
      mWMHandle = 0;
      mComponentId = -1;
      mHelpString = SID_NONE;
      mLegalKeys = MPC_ESC_KEY;
      mLedsStatus = NO_LED;
      mReadOnly = true;
      mValid = false;
      mFocus = false;
      mpDisplay = NULL;
      mEditing = false;
      mpParent = NULL;       // Because SetParent depent on that !!

      if (pParent != NULL)
      {
        SetParent(pParent);
      }
      
      if (pParent)
      {
        SetColour(pParent->GetColour());
        SetBackgroundColour(pParent->GetBackgroundColour());
      }
      else
      {
        SetColour(GUI_COLOUR_DEFAULT_FOREGROUND);
        SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      }

      mVisible = false;
      mTransparent = false;
      mClientRect.x0 = 0;
      mClientRect.y0 = 0;
      mClientRect.x1 = 0;
      mClientRect.y1 = 0;
      mStayOnTop = false;
      mSelectable = true;
    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *************************************************************************/
    Component::~Component()
    {
      Destroy();
    }

    /*************************************************************************
    * Function IsInitialized
    * DESCRIPTION:
    * Returns true if a window is created in the Window Manager for this 
    * component
    *************************************************************************/
    bool Component::IsInitialized()
    {
      return mWMHandle != 0;
    }

    /*************************************************************************
    * Function Initialize
    * DESCRIPTION:
    * Creates a window in the Window Manager for this component.
    * Usually called by GetWMHandle.
    *************************************************************************/
    void Component::Initialize()
    {
      if(IsInitialized())
        return;

      if(mpParent != NULL && !mpParent->IsInitialized())
      {
        mpParent->Initialize();
      }

      U16 style = WM_CF_DISABLED|WM_CF_CONST_OUTLINE|WM_CF_LATE_CLIP|WM_CF_MEMDEV|WM_CF_MEMDEV_ON_REDRAW;
      if(mVisible == false)
      {
        style |= WM_CF_HIDE;
      }
      else
      {
        style |= WM_CF_SHOW;
      }
      
      if(mStayOnTop)
      {
        style |= WM_CF_STAYONTOP;
      }


      if(mTransparent)
      {
        style |= WM_CF_HASTRANS;
      }
      WM_HWIN hParent = NULL;
      if(mpParent != NULL)
      {
        hParent = mpParent->GetWMHandle();
      }
      else
      {
        hParent = WM_GetDesktopWindow();
      }
      mWMHandle = WM_CreateWindowAsChild(mClientRect.x0,mClientRect.y0,GetWidth(),GetHeight(),hParent,style,NULL,0);
      if(mWMHandle == 0)
      {
        WM_SelectWindow(WM_GetDesktopWindow());
        WM_HWIN win = WM_GetFirstChild(WM_GetDesktopWindow());
        WM_HWIN win_next;
        while(win != NULL)
        {
          win_next = WM_GetNextSibling(win);
          WM_DeleteWindow(win);
          win = win_next;
        }

        WM_SetStayOnTop(WM_GetDesktopWindow(),1);
        WM_BringToTop(WM_GetDesktopWindow());
        GUI_SetBkColor(GUI_COLOUR_DEFAULT_BACKGROUND);
        GUI_SetColor(GUI_COLOUR_DEFAULT_FOREGROUND);
        GUI_SetFont(*DEFAULT_FONT_18_LANGUAGE_DEP);
        GUI_Clear();

        const char* sz_msg = "!!! Out of memory !!!\n\nPlease raise GUI_ALLOC_SIZE";

        GUI_RECT  rect;
        
        rect.x0 = 0;
        rect.y0 = 0;
        rect.x1 = 239;
        rect.y1 = 319;
        while(1)
        {
        rect.x0 = 0;
        rect.y0 = 0;
        rect.x1 = 239;
        rect.y1 = 319;
          GUI_DispStringInRect(sz_msg,&rect,GUI_TA_VCENTER|GUI_TA_HCENTER);
          GUI_Delay(700);
          GUI_Clear();
          GUI_Delay(200);
        }
      }

      Invalidate();
    }


    void Component::Destroy()
    {
      if(!IsInitialized())
        return;


      if(WM_IsWindow(mWMHandle))
      {
        /* Detach child windows to make sure their WM handles are NOT
        destroyd by the call to WM_DeleteWindow. The child windows
        should only be destroyed by a call to Destroy, to make the mWMHandle
        member variable invalid. */
/*
        WM_HWIN win = WM_GetFirstChild(mWMHandle);
        WM_HWIN win_next;
        while(win != NULL)
        {
          win_next = WM_GetNextSibling(win);
          WM_DetachWindow(win);
          win = win_next;
        }*/
        WM_DeleteWindow(mWMHandle);
      }
      mWMHandle = NULL; // Handle invalid
    }

    /*************************************************************************
    * Function SelectWindow
    * DESCRIPTION:
    * Selects the WM window for drawing operations.
    *************************************************************************/
    void Component::SelectWindow()
    {
      if(!IsInitialized())
        Initialize();
      WM_SelectWindow(mWMHandle);
    }

    /*************************************************************************
    * Function SetId
    * DESCRIPTION:
    * Sets the component Id from the DisplayComponent table
    *************************************************************************/
    void  Component::SetId(int id)
    {
      mComponentId = id;
    }

    int   Component::GetId()
    {
      return mComponentId;
    }

    /*************************************************************************
    * Function SetId
    * DESCRIPTION:
    * Sets the stayontop flag for the component.
    *************************************************************************/
    void Component::SetStayOnTop(bool stayOnTop)
    {
      if(stayOnTop != mStayOnTop)
      {
        mStayOnTop = stayOnTop;
        if(IsInitialized())
        {
          WM_SetStayOnTop(mWMHandle,mStayOnTop);
        }
      }
    }

    bool Component::GetStayOnTop()
    {
      return mStayOnTop;
    }


    void Component::SetClientArea(int x1, int y1, int x2, int y2)
    {
      SetChildPos(x1,y1);
      SetSize(abs(x2-x1+1),abs(y2-y1 + 1));
    }

    void Component::SetChildPos(int childX, int childY)
    {
      if(childX != GetChildPosX() || childY != GetChildPosY())
      {
        if(IsInitialized())
          ClearArea();
        int width = GetWidth();
        int height = GetHeight();
        mClientRect.x0 = childX;
        mClientRect.y0 = childY;
        mClientRect.x1 = childX + width - 1;
        mClientRect.y1 = childY + height - 1;
        if(IsInitialized())
          WM_MoveChildTo(mWMHandle, mClientRect.x0, mClientRect.y0);
        Invalidate();
      }
    }
    int Component::GetChildPosX()
    {
      return mClientRect.x0;
    }

    int Component::GetChildPosY()
    {
      return mClientRect.y0;
    }

      /* --------------------------------------------------
      * Get/Set child size
      * --------------------------------------------------*/
    void Component::SetSize(int width, int height)
    {
      if(width != GetWidth() || height != GetHeight())
      {
        if(IsInitialized())
          ClearArea();
        mClientRect.x1 = mClientRect.x0 + width - 1;
        mClientRect.y1 = mClientRect.y0 + height - 1;
        if(IsInitialized())
          WM_SetSize(mWMHandle, width, height);
        Invalidate();
      }
    }

    void Component::SetWidth(int width)
    {
      if(GetWidth() != width)
      {
        if(IsInitialized())
          ClearArea();
        mClientRect.x1 = mClientRect.x0 + width - 1;
        if(IsInitialized())
          WM_SetXSize(mWMHandle,width);
        Invalidate();
      }
    }

    void Component::SetHeight(int height)
    {
      if(GetHeight() != height)
      {
        if(IsInitialized())
          ClearArea();
        mClientRect.y1 = mClientRect.y0 + height - 1;
        if(IsInitialized())
          WM_SetYSize(mWMHandle,height);
        Invalidate();
      }
    }

    int Component::GetWidth()
    {
      return abs(mClientRect.x1 - mClientRect.x0 + 1);
    }

    int Component::GetHeight()
    {
      return abs(mClientRect.y1 - mClientRect.y0 + 1);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the LEDs which this element and the ones below, wants to be on
    * or off. Normaly this is also the keys to react, but it doesn't have
    * to be this way ...
    *****************************************************************************/
    Leds Component::GetLedsStatus()
    {
      return mLedsStatus;
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the keys to which we should react in this element and the
    * elements below. Thereby this is the element to send key messages
    * to for the given keys.
    *****************************************************************************/
    Keys Component::GetLegalKeys()
    {
      return mLegalKeys;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Handle this key event. Return true if it is taken care of
    *****************************************************************************/
    bool Component::HandleKeyEvent(Keys KeyID)
    {
      if(KeyID == MPC_HELP_KEY)
      {
        if(ShowHelp())
        {
          return true;
        }
      }

      if( IsReadOnly() )
      {
        // Nothing to edit, switch display.
        switch( KeyID )
        {
          case MPC_OK_KEY:
            if( mpDisplay )
            {
#ifdef __FADE__
              DisplayController::GetInstance()->Push(mpDisplay, 224, WM_GetWindowOrgY(mWMHandle));
#else
              DisplayController::GetInstance()->Push(mpDisplay);
#endif
              return true;
            }
          break;
          case MPC_ESC_KEY:
            DisplayController::GetInstance()->Pop();
            return true;
            // break;
          default:
            return false;
        }

      }
      else
      {
        if( IsInEditMode() )
        {
          switch( KeyID )
          {
            case MPC_OK_KEY :
              return EndEdit();
              // break;
            case MPC_ESC_KEY :
              return CancelEdit();
              // break;
            default :
              return false;
          }
        }
        else
        {
          // Start edit.
          switch( KeyID )
          {
            case MPC_OK_KEY        :
              if(mpDisplay != NULL)
              {
#ifdef __FADE__
                DisplayController::GetInstance()->Push(mpDisplay, 224, WM_GetWindowOrgY(mWMHandle));
#else
                DisplayController::GetInstance()->Push(mpDisplay);
#endif
                return true;
              }
              // fall through
            case MPC_PLUS_KEY      :
            case MPC_PLUS_KEY_REP  :
            case MPC_MINUS_KEY     :
            case MPC_MINUS_KEY_REP :
              return BeginEdit();
              // break;
            case MPC_ESC_KEY:
              DisplayController::GetInstance()->Pop();
              return true;
              // break;
            default:
              return false;
          }
        }
      }
      return false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns whether this element is readable/writeable
    *****************************************************************************/
    bool Component::IsReadOnly()
    {
      return mReadOnly || (mpParent != NULL && mpParent->IsReadOnly());
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets or resets the readonly flag, and invlidates the component. The
    * function returns the previous readonly state.
    *****************************************************************************/
    bool Component::SetReadOnly(bool readOnly /*= true*/)
    {
      bool prev = mReadOnly;
      if(mReadOnly != readOnly)
      {
        mReadOnly = readOnly;
        Invalidate();
      }
      return prev;
    }


    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the method returns
    * false. You should call this as the first thing in inherited classes.
    * The WM window is shown or hidden, and selected for drawing
    * operations. The WM parent is set to ensure the window hierarchy.
    *************************************************************************/
    bool Component::Redraw()
    {
#ifdef __PC__
//      _RPT1( _CRT_WARN, "Component::Redraw() called instance: %lX\r\n", (unsigned long)this );
#endif

      Validate();
      
      if (!IsVisible())
      {
        return true;
      }

      if (!IsInitialized())
      {
        Initialize();
        Validate();
      }

      if (!IsTransparent())
      {
        //XHM: ClearArea selects the window
        ClearArea();
      }
      else
      {
        SelectWindow();
        GUI_SetColor(mColour);
        GUI_SetBkColor(mBackgroundColour);
      }
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns whether this element is visible or not
    *****************************************************************************/
    bool Component::IsVisible()
    {
      if(mpParent)
      {
        if(!mVisible)
          return false;
        return  mpParent->IsVisible();
      }
      else
      {
        return mVisible;
      }
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns whether this element is visible or not
    *****************************************************************************/
    void Component::SetVisible(bool Set)
    {
      if(mVisible != Set )
      {
        if(IsInitialized())
        {
          if(Set)
          {
            WM_ShowWindow(mWMHandle);
  //          WM_BringToTop(mWMHandle);
            Invalidate();
          }
          else
          {
            ClearArea();
            WM_BringToBottom(mWMHandle);
            WM_HideWindow(mWMHandle);
          }
        }
        mVisible = Set;
        Invalidate();
      }
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called by operating system to give time to redraw
    *****************************************************************************/
    void Component::Run()
    {
      if (!IsValid())
      {
        Redraw();
      }

    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns the background colour of the component
    *****************************************************************************/
    U32 Component::GetBackgroundColour()
    {
      return mBackgroundColour;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Does this component have a background or is it transparent?
    *****************************************************************************/
    bool Component::IsTransparent()
    {
      return mTransparent;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Holds the colour of the component
    *****************************************************************************/
    U32 Component::GetColour()
    {
      return mColour;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns the background colour of the component
    *****************************************************************************/
    void Component::SetBackgroundColour(U32 Colour)
    {
      if (mBackgroundColour != Colour)
      {
        mBackgroundColour = Colour;
        Invalidate();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Does this component have a background or is it transparent?
    *****************************************************************************/
    void Component::SetTransparent(bool Set)
    {
      if (IsTransparent() != Set)
      {
        mTransparent = Set;
        if(IsInitialized())
        {
          if(Set)
            WM_SetHasTrans(mWMHandle);
          else
            WM_ClrHasTrans(mWMHandle);
        }
        Invalidate();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Holds the colour of the component
    *****************************************************************************/
    void Component::SetColour(U32 Colour, bool forced)
    {
      if (mColour != Colour || forced)
      {
        mColour = Colour;
        Invalidate();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the parent
    *****************************************************************************/
    void Component::SetParent( Component* pParent )
    {
      if(mpParent == pParent)
        return;

      mpParent = pParent;
      if(IsInitialized())
      {
        if( mpParent )
        {
          WM_AttachWindow(mWMHandle,mpParent->GetWMHandle());
        }
        else
        {
          WM_AttachWindow(mWMHandle, WM_GetDesktopWindow() );
        }
      }
      Invalidate();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the parent
    *****************************************************************************/
    Component* Component::GetParent( )
    {
      return mpParent;
    }

    /* --------------------------------------------------
    * Sets the input foces to this component.
    * --------------------------------------------------*/
    void Component::SetFocus(bool focus /*= true*/)
    {
      if(mFocus != focus)
      {
        mFocus = focus;
        Invalidate();
      }
    }

    /* --------------------------------------------------
    * Returns true if this component has the input focus.
    * --------------------------------------------------*/
    bool Component::HasFocus()
    {
      return mFocus;
    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Clears the client area using the background colour. The foreground
    * colour is set as well.
    *************************************************************************/
    void Component::ClearArea()
    {
      if(IsVisible())
      {
        SelectWindow();
        GUI_SetColor(mColour);
        GUI_SetBkColor(mBackgroundColour);
        GUI_Clear();
      }
    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Returns the allocated WM handle.
    *************************************************************************/
    WM_HWIN Component::GetWMHandle()
    {
      if(!IsInitialized())
        Initialize();
      return mWMHandle;
    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Invalidates the component, and notify the parent.
    *************************************************************************/
    void Component::Invalidate()
    {
//      WM_InvalidateWindow(mWMHandle);
      mValid = false;
    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Marks this component as valid. When a component is valid it is not
    * Redrawn.
    *************************************************************************/
    void Component::Validate()
    {
      mValid = true;
    }

    bool Component::IsValid()
    {
      return mValid || !mVisible;
    }

    void Component::SetDisplay(Display* pDisplay)
    {
      mpDisplay = pDisplay;
      if( mpDisplay != NULL )
      {
        SetAllowFlags(mLegalKeys,MPC_OK_KEY);
        SetAllowFlags(mLedsStatus,OK_LED);
      }
      else
      {
        if(mReadOnly)
        {
          RemoveAllowFlags(mLegalKeys,MPC_OK_KEY);
          RemoveAllowFlags(mLedsStatus,OK_LED);
        }
      }
      Invalidate();
    }

    Display* Component::GetDisplay()
    {
      return mpDisplay;
    }


    /* --------------------------------------------------
    * Returns true if the component is currently being
    * edited.
    * --------------------------------------------------*/
    bool Component::IsInEditMode()
    {
      return mEditing;
    }

    /* --------------------------------------------------
    * Puts the component in edit mode.
    * --------------------------------------------------*/
    bool Component::BeginEdit()
    {
      if( IsReadOnly() )
        return false;

      mEditing = true;
      Invalidate();
      return true;
    }

    /* --------------------------------------------------
    * Cancels the edit mode started by a Call to BeginEdit.
    * The value being edited should return to the original.
    * --------------------------------------------------*/
    bool  Component::CancelEdit()
    {
      if( !IsInEditMode()|| IsReadOnly() )
        return false;
      mEditing = false;
      Invalidate();
      return true;
    }

    /* --------------------------------------------------
    * Ends edit mode, validates and stores the new value.
    * --------------------------------------------------*/
    bool Component::EndEdit()
    {
      if( !IsInEditMode()|| IsReadOnly() )
        return false;
      mEditing = false;
      Invalidate();
      return true;
    }


    /* --------------------------------------------------
    * Sets or resets the help string for this component.
    * --------------------------------------------------*/
    void Component::SetHelpString(STRING_ID helpString)
    {
      mHelpString = helpString;
      if(mHelpString != 0)
      {
        SetAllowFlags(mLegalKeys,MPC_HELP_KEY);
        SetAllowFlags(mLedsStatus,HELP_LED);
      }
    }

    /* --------------------------------------------------
    * Shows a help messagebox, if a help string is set
    * for the Component.
    * --------------------------------------------------*/
    bool Component::ShowHelp()
    {
      if(mHelpString == SID_NONE)
        return false;

      // Note MessageBox will autodelete it self.
      display::HelpMessageBox* p_msg = new display::HelpMessageBox();
      p_msg->SetTitle(SID_NONE);
      p_msg->SetMessage(mHelpString);
      p_msg->SetStyle(MB_STYLE_HELP);
      display::DisplayController::GetInstance()->PushPopupBox(p_msg);
      return true;
    }

    /* --------------------------------------------------
    * If the subject is a DataPoint and the quality is
    * DP_NEVER_AVAILABLE this function shall return true
    * I'm a little lazzy, so I implemented a default ver
    * of the function in Component that return false.
    * The correct design would be to make it pure virtual.
    * --------------------------------------------------*/
    bool Component::IsNeverAvailable()
    {
      return false;
    }

    /* --------------------------------------------------
    * Sets whether the component should be selectable 
    * when used in a listview. Default is true.
    * --------------------------------------------------*/
    void Component::SetSelectable(bool selectable)
    {
      mSelectable = selectable;
    }

    bool Component::IsSelectable()
    {
      return mSelectable;
    }

#ifdef __PC__
    void Component::CalculateStringWidths(bool forceVisible)
    {
      Invalidate();
      Run();

      if (!forceVisible && mHelpString != SID_NONE)
      {
        StringWidthCalculator::GetInstance()->WriteHelpRefToCSV(mHelpString);
      }
    }
#endif
  } // namespace display
}  // namespace mpc

