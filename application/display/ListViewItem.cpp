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
/* This Class is responsible for holding and drawing a single row in a      */
/* ListView.                                                                */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef __PC__
#include <crtdbg.h>
extern bool g_is_calculating_strings;
#endif

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <GUI_Utility.h>
#include <GUI.h>
#include <GUIConf.h>
#include <keys.h>
#include "ListViewItem.h"
#include <ListView.h>
#include <ObserverText.h>
#include <Group.h>
#include <Label.h>
#include <State.h>
#include <Number.h>
#include <NumberQuantity.h>
#include <AvalibleIfSet.h>
#include <AvalibleIfNotSet.h>
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
    ListViewItem::ListViewItem(Component* pParent /*= NULL*/) : Component(pParent)
    {
      mHideState = 0;
      mSelected = false;
      mNextSelectedColumnIndex = 0;
      mUseSingleCellEditStyle = false;
      mIsEditStyleDetermined = false;
      mReadOnly = false;
      mCallingChild = false;
      mIsUnselectedColoursSet = false;
      
      mSelectedColourForeground = GUI_COLOUR_SELECTION_FOREGROUND;
	    mSelectedColourBackground = GUI_COLOUR_SELECTION_BACKGROUND;
      mUnselectedColourForeground = GUI_COLOUR_TEXT_DEFAULT_FOREGROUND;
      mUnselectedColourBackground = GUI_COLOUR_DEFAULT_BACKGROUND;
      mFrameColour = GUI_COLOUR_SELECTION_FRAME; 
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ListViewItem::~ListViewItem()
    {
      mpDataPoint.UnsubscribeAndDetach(this);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Insert/adds a column to this item.
    *****************************************************************************/
    bool ListViewItem::InsertItem(U16 itemIndex, Component* comp)
    {
      if (itemIndex >= mColumns.size())
      {
        mColumns.push_back(comp);
      }
      else
      {
        mColumns[itemIndex] = comp;
      }

      Invalidate();
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Replaces a column.
    *****************************************************************************/
    bool ListViewItem::SetItem(U16 columnIndex, Component* comp)
    {
      if (columnIndex >= mColumns.size())
      {
        return false;
      }

      mColumns[columnIndex] = comp;

      if (comp)
      {
        comp->SetParent(this);
      }

      Invalidate();
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns a item or NULL if the item don't exist
    *****************************************************************************/
    Component* ListViewItem::GetItem(U16 columnIndex)
    {
      if (columnIndex < mColumns.size())
      {
        return mColumns[columnIndex];
      }
      return NULL;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns the number of items in this component
    *****************************************************************************/
    int ListViewItem::GetColumnCount()
    {
      return mColumns.size();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool ListViewItem::Redraw()
    {
      bool rc = true;
      if (IsVisible() == false)
      {
        return Component::Redraw();
      }
      else
      {
        bool valid = mValid;

        int current_column_index = 0;

        U16 max_x = GetChildPosX() + GetWidth() - 1;

        U16 height = GetHeight() - 2;

        U16 x1 = 1;
        U16 x2 = 1;

        ListView* p_parent = (ListView*)GetParent();
        Component*  p_child;
        int number_of_columns = mColumns.size();
        int last_index = number_of_columns - 1;
        for (int i = 0; i < number_of_columns; i++)
        {
          p_child = mColumns[i];
          x2 = x1 + p_parent->GetColumnWidth(current_column_index);

          // if the next column(s) are not used (childless), then merge the columns into one
          if (i != last_index && mColumns[i] != NULL)
          {
            int next_column_to_try_merging = current_column_index;
            while (i != last_index)
            {
              i++;
              next_column_to_try_merging++;

              int next_width = p_parent->GetColumnWidth(next_column_to_try_merging);

              if (i == last_index 
                && (mColumns[i] == NULL || next_width == 0))
              {
                x2 = max_x;
              }
              else if (mColumns[i] == NULL)
              {
                x2 += next_width;
              }
              else if (next_width == 0)
              {
                continue;
              }
              else // next column with visible content was found
              {
                break;
              }
            }

            i = current_column_index;
          }
          
          // if the column is empty, then hide the column by setting its width to zero
          if (mColumns[i] == NULL && i > 0)
          {
            x2 = x1;
          }

          if ((x2 > max_x) || (i == last_index)) // Ensure we aren't drawing outside the assigned area.
          {
            x2 = max_x; // Expand the last column to the right.
          }

          if (p_child != NULL)
          {
            p_child->SetParent(this);
            p_child->SetClientArea(x1, 1, x2 - 1, height);
            p_child->SetFocus(mSelected && mColumns[i]->IsReadOnly() == false);

            if (!valid)
            {
              p_child->Invalidate();
            }
          }
          x1 = x2;
          current_column_index++;
        }

        ClearArea();
        if (mSelected)
        {
          GUI_SetColor(mFrameColour);
          int x = GetWidth();
          int y = GetHeight();
          GUI_DrawLine(1, 0, x - 2, 0);        // top
          GUI_DrawLine(x - 1, 1, x - 1, y - 2);// right side
          GUI_DrawLine(x - 2, y - 1, 1, y - 1);// bottom
          GUI_DrawLine(0, y - 2, 0, 1);        // left side
        }

        Validate();
        
      }
      return rc;
    }

    /* --------------------------------------------------
    * Gets the LEDs which this element and the ones below, wants to be on
    * or off. Normaly this is also the keys to react, but it doesn't have
    * to be this way ...
    * --------------------------------------------------*/
    Leds ListViewItem::GetLedsStatus()
    {
      Leds re = NO_LED;

      int number_of_columns = mColumns.size();
      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL)
        {
          re |= mColumns[i]->GetLedsStatus();
        }
      }
      return re;
    }

    /* --------------------------------------------------
    * Gets the keys to which we should react in this element and the
    * elements below. Thereby this is the element to send key messages
    * to for the given keys.
    * --------------------------------------------------*/
    Keys ListViewItem::GetLegalKeys()
    {
      Keys keys = MPC_NO_KEY;

      int number_of_columns = mColumns.size();
      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL)
        {
          keys |= mColumns[i]->GetLegalKeys();
        }
      }
      return keys;
    }

    /* --------------------------------------------------
    * Handle this key event. Return true if it is taken care of
    * --------------------------------------------------*/
    bool ListViewItem::HandleKeyEvent(Keys KeyID)
    { 
      bool key_handled = false;
      bool in_edit_mode = IsInEditMode();
      int number_of_columns = mColumns.size();

      DetermineEditStyle();

      if (mUseSingleCellEditStyle)
      {
        int columnIndex = -1;

        // Lookup the current column/cell in edit mode.
        for( int i = 0 ; i < number_of_columns; i++)
        {
          Component* p_comp = mColumns[i];

          if (p_comp->IsInEditMode()==true)
          {
            columnIndex = i; // Found it!
            break;
          }
        }

        if (columnIndex == -1 )
        {
          // If no columnIndex/cell in edit mode, then find the next candidate.
          mNextSelectedColumnIndex = (mNextSelectedColumnIndex > number_of_columns ? 0 : mNextSelectedColumnIndex);

          for( int i = mNextSelectedColumnIndex ; i < number_of_columns; i++)
          {
            Component* p_comp = mColumns[i];
            if (p_comp->IsReadOnly() == false && IsFlagInSet(p_comp->GetLegalKeys(), KeyID))
            {
              columnIndex = i; // Found it!
              break;
            }
          }
        }

        if (columnIndex != -1)
        {
          // Forward the key to the column/cell.
          Component* p_comp = mColumns[columnIndex];

          key_handled = p_comp->HandleKeyEvent(KeyID);

          if (key_handled && p_comp->IsInEditMode() == false)
          {
            // If the column is no longer in edit mode, then set the next to be selected.
            mNextSelectedColumnIndex = columnIndex + 1;
          }
        }

      }
      else
      {
        // Key must be forwarded to all columns/cells if they accept it.
        for (int i = 0; i < number_of_columns; i++)
        {
          Component* p_comp = mColumns[i];

          if (p_comp != NULL)
          {
            if (p_comp->IsInEditMode() || IsFlagInSet(p_comp->GetLegalKeys(), KeyID))
            {
              if(p_comp->HandleKeyEvent(KeyID))
              {
                key_handled = true;
              }
            }
          }
        }
      }

      return key_handled;
    }

    /* --------------------------------------------------
    * Test all components in this row.
    * --------------------------------------------------*/
    bool ListViewItem::IsValid()
    {
      if (mValid == false)
        return false;

      int number_of_columns = mColumns.size();
      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL && mColumns[i]->IsValid() == false)
        {
          return false;
        }
      }

      bool bNeverAvailable = IsNeverAvailable();
      if (bNeverAvailable != mLastNeverAvailable)
      {
        mLastNeverAvailable = bNeverAvailable;

        Invalidate();
      }
      return mValid;
    }


    void ListViewItem::Run()
    {
      Component::Run();

      int number_of_columns = mColumns.size();

      for (int i = 0; i < number_of_columns; i++)
      {
        Component* p_comp = mColumns[i];
        if (p_comp != NULL)
        {
          //get unselected colors if not set
          if (!mIsUnselectedColoursSet)
          {
            Text* p_text = dynamic_cast<Text*>(p_comp);
            if (p_text != NULL)
            {
              mUnselectedColourForeground = p_comp->GetColour();
              mUnselectedColourBackground = p_comp->GetBackgroundColour();
              mIsUnselectedColoursSet = true;
            }
          }

          if (p_comp->GetParent() == NULL)
          {
            p_comp->SetParent(this);
          }

          if (mSelected)
          {
            p_comp->SetColour(mSelectedColourForeground);
            p_comp->SetBackgroundColour(mSelectedColourBackground);
          }
          else
          {
            p_comp->SetColour(mUnselectedColourForeground);
            p_comp->SetBackgroundColour(mUnselectedColourBackground);
          }
          
          p_comp->Run();

        }
      }
    }

    void ListViewItem::SetSelected(bool select/*=true*/)
    {
      if (mSelected != select)
      {
        mSelected = select;
        mNextSelectedColumnIndex = 0;
        Invalidate();
      }
    }

    bool ListViewItem::IsSelected()
    {
      return mSelected;
    }


    void ListViewItem::SetSelColour(U32 colour)
    {
      if (mSelectedColourForeground != colour)
      {
        mSelectedColourForeground = colour;
        Invalidate();
      }
    }

    void ListViewItem::SetSelBgColour(U32 colour)
    {
      if (mSelectedColourBackground != colour)
      {
        mSelectedColourBackground = colour;
        Invalidate();
      }
    }

    void ListViewItem::SetSelFrameColour(U32 colour)
    {
      if (mFrameColour != colour)
      {
        mFrameColour = colour;
        Invalidate();
      }
    }


    void ListViewItem::SetColour(U32 colour, bool forced)
    {
      if (mColour != colour || forced)
      {
        for (int i = 0; i < mColumns.size(); i++)
        {
          if (mColumns[i] != NULL)
          {
            mColumns[i]->SetColour(colour, forced);
          }
        }
      }
      Component::SetColour(colour, forced);
    }
    /* --------------------------------------------------
    * Returns whether this element is readable/writeable
    * --------------------------------------------------*/
    bool ListViewItem::IsReadOnly()
    {
      if (mCallingChild)
        return false;

      if (Component::IsReadOnly())
        return true;

      mCallingChild = true;
      bool rc = true;
      
      int number_of_columns = mColumns.size();
      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL && !mColumns[i]->IsReadOnly())
        {
          rc = false;
          break;
        }

      }
      mCallingChild = false;
      return rc;
    }

    /* --------------------------------------------------
    * Returns true if the component is currently being
    * edited.
    * --------------------------------------------------*/
    bool ListViewItem::IsInEditMode()
    {
      int number_of_columns = mColumns.size();
      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL && mColumns[i]->IsInEditMode())
        {
          return true;
        }
      }

      // Nothing is editing.
      return false;
    }

    /* --------------------------------------------------
    * Puts the component in edit mode.
    * --------------------------------------------------*/
    bool ListViewItem::BeginEdit()
    {
      int number_of_columns = mColumns.size();

      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL && !mColumns[i]->IsReadOnly())
        {
          return mColumns[i]->BeginEdit();
        }
      }
     
      // Nothing is writeable
      return false;
    }
    /* --------------------------------------------------
    * Cancels the edit mode started by a Call to BeginEdit.
    * The value being edited should return to the original.
    * --------------------------------------------------*/
    bool ListViewItem::CancelEdit()
    {
      int number_of_columns = mColumns.size();

      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL && !mColumns[i]->IsReadOnly())
        {
          return mColumns[i]->CancelEdit();
        }
      }

      // Nothing is writeable
      return false;
    }

    /* --------------------------------------------------
    * Ends edit mode and stores the new value.
    * --------------------------------------------------*/
    bool ListViewItem::EndEdit()
    {
      int number_of_columns = mColumns.size();

      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL && !mColumns[i]->IsReadOnly())
        {
          return mColumns[i]->EndEdit();
        }
      }
      
      // Nothing is writeable
      return false;
    }

    bool ListViewItem::IsSelectable()
    {
      int number_of_columns = mColumns.size();
      for (int i = 0; i < number_of_columns; i++)
      {
        Component* p_component = mColumns[i];
        if (p_component != NULL)
        {
          if ( dynamic_cast<AvalibleIfNotSet*>(p_component) != NULL 
            || dynamic_cast<AvalibleIfSet*>(p_component) != NULL )
          {//ignore columns with availability checks
            continue;
          }
          else if (i == 0)
          {
            if (!p_component->IsSelectable())
              return false;

            // make labels of first column without left margin for selection frame unselectable
            Label* label = dynamic_cast<Label*>(p_component);
            if (label != NULL && label->GetLeftMargin() == 0)
              return false;

            // make state-labels of first column without left margin for selection frame unselectable
            State* state = dynamic_cast<State*>(p_component);
            if (state != NULL && state->GetLeftMargin() == 0)
              return false;
          } 

          return true;
        }
      }
      return false;
    }

    bool ListViewItem::IsNeverAvailable()
    {
#ifdef __PC__
      if (g_is_calculating_strings)
        return false;
#endif
      if(mpDataPoint.IsValid())
      {
        return mpDataPoint->GetAsInt() == mHideState;
      }
      else
      {
        int number_of_columns = mColumns.size();
        for (int i = 0; i < number_of_columns; i++)
        {
          if (mColumns[i] != NULL 
            && mColumns[i]->IsNeverAvailable() == true)
          {
            return true;
          }
        }
      }
      return false;
    }


    void ListViewItem::SetHideState(int state)
    {
      mHideState = state;
    }

    void ListViewItem::Update(Subject* pSubject)
    {
      if (mpDataPoint.Update(pSubject))
		{
			mpDataPoint.ResetUpdated();
			Invalidate();
		}
    }

    void ListViewItem::SetSubjectPointer(int id, Subject* pSubject)
    {
      mpDataPoint.UnsubscribeAndDetach(this);
      mpDataPoint.Attach(pSubject);
    }

    void ListViewItem::ConnectToSubjects(void)
    {
      mpDataPoint.Subscribe(this);
    }

    void ListViewItem::SubscribtionCancelled(Subject* pSubject)
    {
      mpDataPoint.Detach(pSubject);
    }

    /* --------------------------------------------------
    * Destroys the WM window allocated by Initialize.
    * The component object remains, allocated.
    * --------------------------------------------------*/
    void ListViewItem::Destroy()
    {
      int number_of_columns = mColumns.size();
      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL)
        {
          mColumns[i]->Destroy();
        }
      }
      
      Component::Destroy();
    }

    void ListViewItem::DetermineEditStyle(void)
    {
      if (!mIsEditStyleDetermined)
      {
        mIsEditStyleDetermined = true;
      
        int number_of_editable_number_cells = 0;

        int number_of_columns = mColumns.size();
        for (int i = 0; i < number_of_columns; i++)
        {
          Component* p_comp = mColumns[i];
          if (p_comp != NULL && !p_comp->IsReadOnlySet())
          {
            Number* p_n = dynamic_cast<Number*>(p_comp);
            NumberQuantity* p_nq = dynamic_cast<NumberQuantity*>(p_comp);

            if (p_n != NULL || p_nq != NULL)
            {
              number_of_editable_number_cells++;
            }
          }
        }

        mUseSingleCellEditStyle = number_of_editable_number_cells > 1;
      }
    }

#ifdef __PC__
    void ListViewItem::CalculateStringWidths(bool forceVisible)
    {
      Component::CalculateStringWidths(forceVisible);
      Component* p_comp;
      int number_of_columns = mColumns.size();
      for (int i = 0; i < number_of_columns; i++)
      {
        if (mColumns[i] != NULL)
        {
          p_comp = mColumns[i];
          bool isHidden = !p_comp->IsVisible();

          if (forceVisible)
          {
            p_comp->SetVisible();
          }

          p_comp->CalculateStringWidths(forceVisible);

          if (forceVisible && isHidden)
          {
            p_comp->SetVisible(false);
          }
        }
      }
    }
#endif

  } // namespace display
} // namespace mpc

