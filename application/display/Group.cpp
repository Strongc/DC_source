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
/* CLASS NAME       : Group                                                 */
/*                                                                          */
/* FILE NAME        : Group.h                                               */
/*                                                                          */
/* CREATED DATE     : 2004-10-18                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for grouping and drawing displaycomponents.    */
/*                                                                          */
/****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <algorithm>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "WM.h"
#include <keys.h>
#include <Leds.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Group.h"


namespace mpc
{
  namespace display
  {
    Group::Group(Component* pParent /*= NULL*/) : Component( pParent )
    {
      mCurrentChild = NULL;
      mLedsStatus = NO_LED;
      mLegalKeys = MPC_NO_KEY;
      mCallingChild = false;
      mColour = GUI_COLOUR_TEXT_DEFAULT_FOREGROUND;
    }

    Group::~Group()
    {
    }

    int Group::AddChild(Component* child)
    {
      std::vector< Component* >::iterator iter = std::find(mChildren.begin(), mChildren.end(), child);
      if (iter != mChildren.end())
      {
        return false;
      }
      
      //set mCurrentChild to support nested groups with focusable components for writable groups.
      if (!mReadOnly)
      {
        mCurrentChild = child;
      }
      
      mChildren.push_back(child);
      child->SetParent(this);
      Invalidate();
      return mChildren.size() - 1;
    }

    bool Group::RemoveChild( Component* child )
    {
      std::vector< Component* >::iterator iter = std::find(mChildren.begin(), mChildren.end(), child);
      if (iter != mChildren.end())
      {
        mChildren.erase(iter);
      }
      Invalidate();
      return true;
    }

    Component* Group::GetFirstChild()
    {
      std::vector< Component* >::iterator iter = mChildren.begin();
      if (mChildren.size() > 0)
        return *iter;
      else
        return NULL;
    }

    Component* Group::GetNextChild( Component* child )
    {
      std::vector< Component* >::iterator iter = mChildren.begin();
      while(iter != mChildren.end() && *iter != child)
      {
        ++iter;
      }

      ++iter;
      if( iter >= mChildren.end() )
      {
        return NULL;
      }
      return *iter;
    }

    Component* Group::GetCurrentChild()
    {
      return mCurrentChild;
    }

    bool Group::SetCurrentChild( Component* child )
    {
      if(mCurrentChild != NULL)
      {
        mCurrentChild->SetFocus(false);
        mCurrentChild->Invalidate();
      }

      std::vector< Component* >::iterator iter = std::find(mChildren.begin(), mChildren.end(), child);
      if (iter != mChildren.end())
      {
        mCurrentChild = child;
        mCurrentChild->SetFocus();
        mCurrentChild->Invalidate();
        return true;
      }
      return false;
    }

    /* --------------------------------------------------
    * Sets the background colour of the component and all childs.
    * --------------------------------------------------*/
    void Group::SetBackgroundColourRecursive(U32 Colour)
    {
      Component* p_comp = GetFirstChild();
      while(p_comp)
      {
        p_comp->SetBackgroundColour(Colour);
        p_comp = GetNextChild(p_comp);
      }
    }

    /* --------------------------------------------------
    * Sets the colour of the component and all childs.
    * --------------------------------------------------*/
    void Group::SetColourRecursive(U32 colour, bool forced)
    {
      Component* p_comp = GetFirstChild();
      while (p_comp)
      {
        p_comp->SetColour(colour, forced);
        p_comp = GetNextChild(p_comp);
      }
    }

    void Group::SetColour(U32 colour, bool forced)
    {
      Component::SetColour(colour, forced);
      SetColourRecursive(colour, forced);
    }

    void Group::ValidateRecursively()
    {
      Component::Validate();
      Component* p_comp = GetFirstChild();
      while (p_comp)
      {
        p_comp->Validate();
        p_comp = GetNextChild(p_comp);
      }
    }

    bool Group::Redraw()
    {
      bool rc = true;
      bool valid = mValid;

      if (!valid && IsVisible())
      {
        std::vector< Component* >::iterator iter = mChildren.begin();
        std::vector< Component* >::iterator iterEnd = mChildren.end();
        for( ; iter != iterEnd; ++iter )
        {
          (*iter)->Invalidate();
        }
      }
      Component::Redraw();
      return rc;
    }

    U16 Group::GetChildCount()
    {
      return (U16)mChildren.size();
    }


    /* --------------------------------------------------
    * Gets the LEDs which this element and the ones below, wants to be on
    * or off. Normaly this is also the keys to react, but it doesn't have
    * to be this way ...
    * --------------------------------------------------*/
    Leds Group::GetLedsStatus()
    {
      Leds leds = mLedsStatus;
      Component* p_child = GetCurrentChild();
      if(p_child != NULL)
      {
        return leds | p_child->GetLedsStatus();
      }
      return mLedsStatus;
    }

    /* --------------------------------------------------
    * Gets the keys to which we should react in this element and the
    * elements below. Thereby this is the element to send key messages
    * to for the given keys.
    * --------------------------------------------------*/
    Keys Group::GetLegalKeys()
    {
      Keys keys = Component::GetLegalKeys();
      Component* p_child = GetCurrentChild();
      if(p_child != NULL)
      {
        keys |= p_child->GetLegalKeys();
      }
      return keys;
    }

    /* --------------------------------------------------
    * Handle this key event. Return true if it is taken care of
    * --------------------------------------------------*/
    bool Group::HandleKeyEvent(Keys KeyID)
    {
      Component* p_child = GetCurrentChild();
      if(p_child != NULL)
      {
        if(p_child->HandleKeyEvent(KeyID))
          return true;
      }
      return Component::HandleKeyEvent(KeyID);
    }

    /* --------------------------------------------------
    * Called by operating system to give time to redraw
    * --------------------------------------------------*/
    void Group::Run(void)
    {
      if(!IsVisible())
        return;

      bool valid = mValid;
      if (!valid || !IsValid())
      {
        Component::Run();
      }

      Component*  p_child = NULL;
      std::vector< Component* >::iterator iter = mChildren.begin();
      std::vector< Component* >::iterator iterEnd = mChildren.end();
      for( ; iter != iterEnd; ++iter )
      {
        p_child = *iter;
        if(GetCurrentChild() != NULL && p_child->HasFocus() && p_child != GetCurrentChild())
        {
          SetCurrentChild(p_child);
        }

        if(!valid)
          p_child->Invalidate();
        p_child->Run();
      }
    }

    /* --------------------------------------------------
    * Returns false if the group or any of its childs
    * needs redrawing
    * --------------------------------------------------*/
    bool Group::IsValid()
    {
      if (IsNeverAvailable())
      {
        return true;
      }
      if (!mValid && IsVisible())
      {
        return false;
      }

      if(IsVisible())
      {
        std::vector< Component* >::iterator iter = mChildren.begin();
        std::vector< Component* >::iterator iterEnd = mChildren.end();
        for( ; iter != iterEnd; ++iter )
        {
          if( !(*iter)->IsValid() )
          {
            return false;
          }
        }
      }
      // If we get down here all children are valid or group is not visible.
      return true;
    }


    /* --------------------------------------------------
    * Redirect call to the current child.
    * --------------------------------------------------*/
    bool Group::IsReadOnly()
    {
      if(mCallingChild)
        return false;

      if(Component::IsReadOnly())
        return true;

      Component*  p_cur_child = GetCurrentChild();
      if( p_cur_child != NULL )
      {
        mCallingChild = true;
        bool rc = p_cur_child->IsReadOnly();
        mCallingChild = false;
        return rc;
      }

      return false;
    }

    /* --------------------------------------------------
    * Redirect call to the current child.
    * --------------------------------------------------*/
    bool Group::IsInEditMode()
    {
      if( IsReadOnly() )
        return false;
      Component*  p_child = GetCurrentChild();
      if( p_child != NULL )
        return p_child->IsInEditMode();

      p_child = GetFirstChild();
      while(p_child != NULL)
      {
        if(p_child->IsInEditMode())
        {
          return true;
        }
        p_child = GetNextChild(p_child);
      }
      return mEditing;
    }

    /* --------------------------------------------------
    * Redirect call to the current child.
    * --------------------------------------------------*/
    bool Group::BeginEdit()
    {
      if( IsReadOnly() )
        return false;
      Component*  p_cur_child = GetCurrentChild();
      if( p_cur_child != NULL )
        return p_cur_child->BeginEdit();
      return mEditing;
    }

    /* --------------------------------------------------
    * Redirect call to the current child.
    * --------------------------------------------------*/
    bool Group::CancelEdit()
    {
      if( IsReadOnly() )
        return false;
      Component*  p_cur_child = GetCurrentChild();
      if( p_cur_child != NULL )
        return p_cur_child->CancelEdit();
      return mEditing;
    }

    /* --------------------------------------------------
    * Redirect call to the current child.
    * --------------------------------------------------*/
    bool Group::EndEdit()
    {
      if( IsReadOnly() )
        return false;
      Component*  p_cur_child = GetCurrentChild();
      if( p_cur_child != NULL )
        return p_cur_child->EndEdit();
      return mEditing;
    }


    Display* Group::GetDisplay()
    {
      if(mpDisplay != NULL)
        return mpDisplay;


      Display* disp = NULL;
      std::vector< Component* >::iterator iter = mChildren.begin();
      std::vector< Component* >::iterator iterEnd = mChildren.end();
      for( ; iter != iterEnd; ++iter )
      {
        disp = (*iter)->GetDisplay();
        if(disp != NULL)
        {
          return disp;
        }
      }
      return NULL;
    }

    void Group::Destroy()
    {
      Component* p_comp;
      std::vector< Component* >::iterator iter = mChildren.begin();
      std::vector< Component* >::iterator iterEnd = mChildren.end();
      for( ; iter != iterEnd; ++iter )
      {
        p_comp = (*iter);
        if(p_comp != NULL)
          p_comp->Destroy();
      }
      Component::Destroy();
    }

#ifdef __PC__
    void Group::CalculateStringWidths(bool forceVisible)
    {
      Component::CalculateStringWidths(forceVisible);
      Component* p_child = GetFirstChild();
      while (p_child)
      {
        p_child->CalculateStringWidths(forceVisible);
        p_child = GetNextChild(p_child);
      }
    }
#endif // #ifdef __PC__

  } // namespace display
} // namespace mpc
