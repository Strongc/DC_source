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
/* CLASS NAME       : ListView                                              */
/*                                                                          */
/* FILE NAME        : ListView.cpp                                          */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Draws a list of Component in a  row/column style.                        */
/* The list can be navigated with a row selection bar.                      */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef __PC__
#include <crtdbg.h>
extern bool g_is_calculating_strings;
#endif

#include "WM.h"
#include "leds.h"
#include "keys.h"
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "ListView.h"
#include <DisplayController.h>
#include <Display.h>
#include <LowerStatusLine.h>
#include "AnalogInputSetupListView.h"
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/

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
    ListView::ListView(Component* pParent /*= NULL*/) : Component(pParent)
    {
      mColumnCount = 0;
      mSelectedRow  = -1;
      mLastSelectedRow = -1;
      mRowHeight = 15;
      mpScrollBar = new ScrollBar(this);
      mIsInitialized = false;
      mpNextList = NULL;
      mpPrevList = NULL;
      mCallingChild = false;

      SetAllowFlags(mLegalKeys,MPC_UP_KEY);
      SetAllowFlags(mLegalKeys,MPC_DOWN_KEY);
      SetAllowFlags(mLedsStatus,UP_LED);
      SetAllowFlags(mLedsStatus,DOWN_LED);

      mSelColour = GUI_COLOUR_SELECTION_FOREGROUND;
		  mSelBgColour = GUI_COLOUR_SELECTION_BACKGROUND;
		  mSelFrameColour = GUI_COLOUR_SELECTION_FRAME; 
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ListView::~ListView()
    {
      delete mpScrollBar;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Adds a column to the listview.
    *****************************************************************************/
    bool ListView::InsertColumn(U16 columnIndex)
    {
      // Insert or add the column to all the rows
      for (int i = 0; i < mRows.size(); i++)
      {
        mRows[i]->InsertItem(columnIndex, NULL);
      }

      if (columnIndex >= mColumnCount )
        mColumnWidths.push_back(20);
      else
        mColumnWidths[columnIndex] = 20;

      mColumnCount++;
      Invalidate();
      return true;
    }

    /*************************************************************************
    * Function - GetColCount
    * DESCRIPTION:
    *************************************************************************/
    U16 ListView::GetColCount()
    {
      return mColumnCount;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Inserts a item to the listview
    *****************************************************************************/
    int ListView::InsertItem(U16 itemIndex, Component* comp)
    {
      // Create a new component for the row.
      ListViewItem* plist_view_item = new ListViewItem( this );
      plist_view_item->SetVisible(IsVisible());

      // Add the Component as the first column in the row
      plist_view_item->InsertItem( 0, comp );

      return InsertItem(itemIndex, plist_view_item);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Inserts a item to the listview
    *****************************************************************************/
    int ListView::InsertItem(U16 itemIndex, ListViewItem* item)
    {
      // Create dummies for the rest of the columns
      for (U16 i = item->GetColumnCount(); i < mColumnCount; i++)
      {
        item->InsertItem(i, NULL);
      }

      // Insert or add the new row.
      int index;
      if (itemIndex < mRows.size())
      {
        mRows[itemIndex] = item;
        index = itemIndex;
      }
      else
      {
        mRows.push_back(item);
        index = mRows.size() - 1;
      }

      item->SetParent(this);
      Invalidate();
      return index;
    }

    /*************************************************************************
    * Function - SetNextList
    * DESCRIPTION:
    *************************************************************************/
    void ListView::SetNextList(ListView* pList)
    {
      mpNextList = pList;
    }

    /*************************************************************************
    * Function - SetPrevList
    * DESCRIPTION:
    *************************************************************************/
    void ListView::SetPrevList(ListView* pList)
    {
      mpPrevList = pList;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets/replaces an item
    *****************************************************************************/
    bool ListView::SetItem(U16 itemIndex, U16 columnIndex, Component* comp)
    {
      ListViewItem* p_lvi = GetListViewItem(itemIndex);
      if(p_lvi)
      {
        p_lvi->SetItem(columnIndex,comp);
        Invalidate();
        return true;
      }
      return false;
    }

    /*****************************************************************************
    * Function - GetItem
    * DESCRIPTION:
    * Gets an item.
    *****************************************************************************/
    Component* ListView::GetItem(U16 itemIndex, U16 columnIndex)
    {
      ListViewItem* p_lvi = GetListViewItem(itemIndex);
      if( p_lvi == NULL)
        return NULL;

      return p_lvi->GetItem(columnIndex);
    }

    /*************************************************************************
    * Function - GetListViewItem
    * DESCRIPTION:
    *************************************************************************/
    ListViewItem* ListView::GetListViewItem(U16 itemIndex)
    {
      if (itemIndex < mRows.size())
      {
        return mRows[itemIndex];
      }
      return NULL;
    }

    /*****************************************************************************
    * Function - SetSelection
    * DESCRIPTION:
    * Set the selected row
    *****************************************************************************/
    bool ListView::SetSelection(int itemIndex)
    {
      if (itemIndex >= (int)mRows.size())
      {
        return false;
      }
      
      for (int i = 0; i < mRows.size(); i++)
      {
        mRows[i]->SetSelected(false);
      }

      if (itemIndex < 0)
      {
        mLastSelectedRow = mSelectedRow;
        mSelectedRow = itemIndex;
        return true;
      }

      mLastSelectedRow = mSelectedRow;
      mSelectedRow = itemIndex;
      mRows[itemIndex]->SetSelected(true);
      
      return true;
    }

    /*************************************************************************
    * Function - SelectFirstVisibleItem
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::SelectFirstVisibleItem()
    {
      int i = GetFirstVisibleItem();
      bool rc = SetSelection(i);
      // Nothing to select in this list select the next list and give it focus
      if (i == -1)
      {
        return false;
      }
      return rc;
    }

    /*************************************************************************
    * Function - SelectLastVisibleItem
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::SelectLastVisibleItem()
    {
      int i = GetLastVisibleItem();
      bool rc = SetSelection(i);
      // Nothing to select in this list select the next list and give it focus
      if (i == -1)
      {
        return false;
      }
      return rc;
    }

    /*************************************************************************
    * Function - SelectNextAvailable
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::SelectNextAvailable(int itemIndex)
    {
      const int number_of_rows = (int)(mRows.size());
      int index = itemIndex;

      if (index < 0)
      {
        index = 0;
      }

      while (index < number_of_rows 
        && (mRows[index]->IsNeverAvailable() || !mRows[index]->IsSelectable()))
      {
        index++;
      }

      // Scroll to top of list or focus next list
      if (index >= number_of_rows 
        || mRows[index]->IsNeverAvailable() 
        || !mRows[index]->IsSelectable() 
        || SetSelection(index) == false )
      {
        if (mpNextList != NULL)
        {
          ListView* p_list = mpNextList;
          while (p_list->SelectFirstVisibleItem() == false && p_list != this)
          {
            if (p_list->mpNextList == NULL)
            {
              break;
            }
            p_list = p_list->mpNextList;
          }

          p_list->SetFocus();

          if (p_list != this)
          {
            SetSelection(-1);
            SetFocus(false);
          }
          return true;
        }
        
        
        index = 0;
        while (index < number_of_rows 
          && (mRows[index]->IsNeverAvailable() || !mRows[index]->IsSelectable()) )
        {
          index++;
        }
        return SetSelection(index);
        
      }
      return true;
    }

    /*************************************************************************
    * Function - SelectPrevAvailable
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::SelectPrevAvailable(int itemIndex)
    {
      int index = itemIndex;

      while (index > 0 
        && (mRows[index]->IsNeverAvailable() || !(mRows[index]->IsSelectable())) )
      {
        index--;
      }

      // Scroll to bottom or prev display
      if (index < 0 
        || mRows[index]->IsNeverAvailable() 
        || !mRows[index]->IsSelectable() 
        || SetSelection(index) == false)
      {
        if (mpPrevList != NULL)
        {
          ListView* p_list = mpPrevList;
          while (p_list->SelectLastVisibleItem() == false && p_list != this)
          {
            if (p_list->mpPrevList == NULL)
            {
              break;
            }

            p_list = p_list->mpPrevList;
          }
          p_list->SetFocus();
          if (p_list != this)
          {
            SetSelection(-1);
            SetFocus(false);
          }

          return false;
        }
        index = mRows.size() - 1;
        while (index > 0 && 
          (mRows[index]->IsNeverAvailable() || !mRows[index]->IsSelectable()))
        {
          index--;
        }
        return SetSelection(index);
      }
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns the selected row.
    *****************************************************************************/
    int ListView::GetSelection()
    {
      return mSelectedRow;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the column width of all items in a column.
    *****************************************************************************/
    bool ListView::SetColumnWidth(U16 columnIndex, U16 width)
    {
      mColumnWidths[columnIndex] = width;
      Invalidate();

      return true;
    }

    /*************************************************************************
    * Function - GetColumnWidth
    * DESCRIPTION:
    * Gets the column width.
    *************************************************************************/
    U16 ListView::GetColumnWidth(U16 columnIndex)
    {
      return mColumnWidths[columnIndex];
    }

    /*************************************************************************
    * Function - Redraw
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::Redraw()
    {
      bool rc = true;
      if (IsVisible())
      {
        bool valid = mValid;

        // Calculate the number of fully visible rows in the view
        U16 visible_items = (GetHeight() / mRowHeight);

        const int no_of_rows = (int) mRows.size();

        // Select another row if the current selected row isn't available
        if (GetSelection() >= 0)
        {
          int org_selection = GetSelection();
          
          if (mRows[org_selection]->IsSelected() && mRows[org_selection]->IsNeverAvailable())
          {
            SetSelection(-1);
            for (int i = org_selection; i < no_of_rows; i++)
            {
              if (mRows[i]->IsNeverAvailable() == false)
              {
                SetSelection(i);
                break;
              }
            }

            if (GetSelection() == -1)
            {
              for (int i = org_selection; i >= 0; i--)
              {
                if (mRows[i]->IsNeverAvailable() == false)
                {
                  SetSelection(i);
                  break;
                }
              }
            }
            if (GetSelection() == -1)
            {
              SetSelection(0);
            }
          }
        }
		
        if (!mIsInitialized)
        {
          mIsInitialized = true;

          if (mSelectedRow > 0)
          {
            mFirstVisibleRow = mSelectedRow;
          }
          else
          {
            mFirstVisibleRow = 0;
          }
          
          mLastVisibleRow = mFirstVisibleRow;
          for (int i = visible_items; i > 0 && mLastVisibleRow < no_of_rows; i--)
          {
		        mLastVisibleRow++;
          }
        }
		
        if (GetRowCount() < visible_items)
        {
          mFirstVisibleRow = 0;
          mLastVisibleRow = no_of_rows - 1;
        }
        else
        {
          if (mSelectedRow >= 0 && mSelectedRow < mFirstVisibleRow)
          {
            mFirstVisibleRow = mSelectedRow;

            //if the previous row is not selectable, make sure it becomes visible 
            if (mSelectedRow > 0 && !(mRows[mSelectedRow - 1]->IsSelectable()) )
            {
              mFirstVisibleRow--;
            }

            mLastVisibleRow = mFirstVisibleRow + visible_items;

            for (int i = mFirstVisibleRow; i <= mLastVisibleRow && mLastVisibleRow < no_of_rows; i++)
            {
              if (mRows[i]->IsNeverAvailable())
              {
                mLastVisibleRow++;
              }
            }
          }
          else
          {
            mLastVisibleRow = mFirstVisibleRow + visible_items;

            // Append the number of hidden items between first visible and last visible to the last visible.
            for(int i = mFirstVisibleRow; i <= mLastVisibleRow && mLastVisibleRow < no_of_rows; i++)
            {
              if (mRows[i]->IsNeverAvailable())
              {
                mLastVisibleRow++;
              }
            }
            
			      //if the previous row is not selectable, make sure it becomes visible 
            if (mSelectedRow == 1 && !mRows[0]->IsSelectable() )
            {
              mFirstVisibleRow--;
            }
			
            // Scrolling down to a position outsite the last view
            if (mSelectedRow >= 0 && mSelectedRow >= mLastVisibleRow)
            {
              mLastVisibleRow = mSelectedRow + 1;

              if (mLastVisibleRow >= no_of_rows)
              {
                mLastVisibleRow = no_of_rows;
              }
              mFirstVisibleRow =  mLastVisibleRow - visible_items;

              for (int i = mLastVisibleRow - 1; i >= 0 && i >= mFirstVisibleRow; i--)
              {
                if (mRows[i]->IsNeverAvailable())
                {
                  mFirstVisibleRow--;
                }
              }
            }
          }
        }
        // Safe check :)
        if (mFirstVisibleRow < 0)
        {
          mFirstVisibleRow = 0;
        }
        if (mLastVisibleRow >= no_of_rows)
        {
          mLastVisibleRow = no_of_rows - 1;
        }


        U16 x = GetChildPosX();
        U16 y = GetChildPosY();
        U16 width = GetWidth();
        U16 max_y = GetHeight();

        // Setup the scrollbar
        const U16 scroll_width = 11; // must be a even value
        if( GetRowCount() > visible_items )
        {
          mpScrollBar->SetClientArea(width - scroll_width, 0, width - 1, GetHeight() - 1 );
          mpScrollBar->SetRange(0, GetRowCount() - visible_items);
          mpScrollBar->SetSliderSize( mpScrollBar->GetSliderAreaSize() * (visible_items / (float)GetRowCount()));
          mpScrollBar->SetVisible(IsVisible());
          width = width - scroll_width - 1;
        }
        else
        {
          mpScrollBar->SetVisible(false);
        }
        width = width - 1;
        U16 height = 0;
        x = 0;
        y = 0;


        // Move the scrollbar slider.
        // Count number of hidden rows until the first visible row
        U16 hidden_items = 0;
        for (int i = 0; i < mFirstVisibleRow; i++)
        {
          if (mRows[i]->IsNeverAvailable())
          {
            hidden_items++;
          }
        }

        int i_pos = mFirstVisibleRow - hidden_items;
        mpScrollBar->SetScrollPos(i_pos);
        

        // Set row attributes.
        ListViewItem* p_item;
        bool bVisible = false;
        for (int i = 0; i < no_of_rows; i++)
        {
          p_item = mRows[i];

          // Is the row in the visible area else hide it.
          if( i >= mFirstVisibleRow &&
              i <= mLastVisibleRow)
          {
            if (p_item->IsNeverAvailable())
            {
              bVisible = false;

              if (p_item->IsVisible())
              {
                // about to hide a visible item, invalidate whole listview
                Invalidate();
              }
            }
            else
            {
              bVisible = true;
              height += mRowHeight;
              p_item->SetClientArea(x, y, width, height - 1);
              y = height;

              // Do a full redraw if the entire listview is invalidated.
              if(!valid)
              {
                p_item->Invalidate();
              }
            }

          }
          else
          {
            bVisible = false;
          }
          
          p_item->SetParent(this);
          // if the listview item is beeing hided cancel edit and selection.
          if (p_item->IsVisible() && !bVisible && p_item->IsInEditMode() )
          {
            p_item->CancelEdit();
          }
          
          p_item->SetVisible( bVisible );     
        }
        

        // Redraw scrollbar.
        if(!valid && mpScrollBar != 0)
        {
          mpScrollBar->Invalidate();
        }

        if (HasFocus())
        {
          ListViewItem* p_row;
          Display*    p_disp = NULL;
          if (mSelectedRow >= 0 && mSelectedRow < no_of_rows)
          {
            p_row = mRows[mSelectedRow];
            int cols = GetColCount();
            Component*  p_comp = NULL;
            for(U16 i = 0; i < cols; ++i)
            {
              p_comp = p_row->GetItem(i);
              if(p_comp != NULL)
              {
                p_disp = p_comp->GetDisplay();
                if(p_disp != NULL)
                {
                  break;
                }
              }
            }
          }
        }

        // Clears Area and validate
        Component::Redraw();
      } // if( IsVisible() )

      return rc;
    }

    /*************************************************************************
    * Function - SetRowHeight
    * DESCRIPTION:
    *************************************************************************/
    U16 ListView::SetRowHeight(U16 newHeight)
    {
      U16 oldHeight = mRowHeight;
      mRowHeight = newHeight;
      Invalidate();
      return oldHeight;
    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the foreground colour of the selected item
    *************************************************************************/
    void ListView::SetSelColour(U32 colour)
    {
      if (mSelColour != colour)
      {
        mSelColour = colour;
        for (int i = 0; i < mRows.size(); i++)
        {
          if (mRows[i] != NULL)
          {
            mRows[i]->SetSelColour(mSelColour);
          }
        }
        Invalidate();
      }
    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the background colour of the selected item
    *************************************************************************/
    void ListView::SetSelBgColour(U32 colour)
    {
      if (mSelBgColour!=colour)
      {
        mSelBgColour = colour;
        for (int i = 0; i < mRows.size(); i++)
        {
          if (mRows[i] != NULL)
          {
            mRows[i]->SetSelBgColour(mSelBgColour);
          }
        }
        Invalidate();
      }
    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the frame colour of the rectangle surrounding the selected item.
    *************************************************************************/
    void ListView::SetSelFrameColour(U32 colour)
    {
      if (mSelFrameColour != colour)
      {
        mSelFrameColour = colour;
        for (int i = 0; i < mRows.size(); i++)
        {
          if (mRows[i] != NULL)
          {
            mRows[i]->SetSelFrameColour(mSelFrameColour);
          }
        }
        Invalidate();
      }
    }

    /*************************************************************************
    * Function SetColour
    * DESCRIPTION:
    * Sets foreground colour on all rows.
    *************************************************************************/
    void ListView::SetColour(U32 colour, bool forced)
    {
      if (mColour != colour || forced)
      {
        for (int i = 0; i < mRows.size(); i++)
        {
          if (mRows[i] != NULL)
          {
            mRows[i]->SetColour(colour, forced);
          }
        }
      }
      Component::SetColour(colour, forced);
    }

    /*************************************************************************
    * Function - GetFirstVisibleItem
    * DESCRIPTION:
    *************************************************************************/
    int ListView::GetFirstVisibleItem()
    {
      for (int i = 0; i < mRows.size(); i++)
      {
        if (mRows[i] != NULL 
          && !mRows[i]->IsNeverAvailable() 
          && mRows[i]->IsSelectable())
        {
          return i;
        }
      }

      return -1;
    }

    /*************************************************************************
    * Function - GetLastVisibleItem
    * DESCRIPTION:
    *************************************************************************/
    int ListView::GetLastVisibleItem()
    {
      if (mRows.size() == 0)
        return -1;

      for (int i = mRows.size() - 1; i >= 0; i--)
      {
        if (!mRows[i]->IsNeverAvailable() && mRows[i]->IsSelectable())
          return i;
      }

      return -1;
    }

    /*************************************************************************
    * Function - GetLedsStatus
    * DESCRIPTION:
    * Gets the LEDs which this element and the ones below, wants to be on
    * or off. Normaly this is also the keys to react, but it doesn't have
    * to be this way ...
    *************************************************************************/
    Leds ListView::GetLedsStatus()
    {
      int selected_index = GetSelection();
      Component* selected_item;

      Leds re = Component::GetLedsStatus();

      // if listview only contains a single row, and has no linked listview thwen remove led for up/down arrows
      if ( mpNextList == NULL && mpPrevList == NULL && mRows.size() == 1 )
      {
        RemoveAllowFlags(re, UP_LED);
        RemoveAllowFlags(re, DOWN_LED);
      }

      if ((selected_item = GetRow(selected_index)) == NULL)
      {
        return re;
      }

      return ((ListViewItem*)selected_item)->GetLedsStatus() | re;
    }

    /*************************************************************************
    * Function - GetLegalKeys
    * DESCRIPTION:
    * Gets the keys to which we should react in this element and the
    * elements below. Thereby this is the element to send key messages
    * to for the given keys.
    *************************************************************************/
    Keys ListView::GetLegalKeys()
    {
      int selected_index = GetSelection();
      Component* selected_item;

      Keys re = Component::GetLegalKeys();
      if ((selected_item = GetRow(selected_index)) != NULL)
      {
        re |= ((ListViewItem*)selected_item)->GetLegalKeys();
      }
      else
      {
        SetSelection(0);
      }
      return re;
    }

    /*************************************************************************
    * Function - HandleKeyEvent
    * DESCRIPTION:
    * Handle this key event. Return true if it is taken care of
    *************************************************************************/
    bool ListView::HandleKeyEvent(Keys KeyID)
    {
      int selected_index = GetSelection();
      Component* selected_item;

      Keys legal_keys = GetLegalKeys();
      if (!IsFlagInSet(legal_keys, KeyID))
        return false;


      // Send key to the current selected item
      if ((selected_item = GetRow(selected_index)) != NULL)
      {
        if (((ListViewItem*)selected_item)->HandleKeyEvent(KeyID))
          return true;
      }


      // Scroll the list (if selected item has not handled the key)
      if (KeyID == MPC_UP_KEY)
      {
          --selected_index;
          return SelectPrevAvailable(selected_index);
      }
      else if (KeyID == MPC_DOWN_KEY)
      {
          ++selected_index;
          return SelectNextAvailable(selected_index);
      }
      

      return (IsFlagInSet(mLegalKeys,KeyID) && Component::HandleKeyEvent(KeyID));
    }

    /*************************************************************************
    * Function - GetRow
    * DESCRIPTION:
    *************************************************************************/
    ListViewItem* ListView::GetRow(U16 itemIndex)
    {
      if (itemIndex < mRows.size())
      {
        return (ListViewItem*) mRows[itemIndex];
      }
      return NULL;
    }

    /*************************************************************************
    * Function - IsValid
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::IsValid()
    {
      if (Component::IsValid() == false)
        return false;

      // Ask each row (and their components) if they are valid
      Component*  p_comp;

      for (int i = 0; i < mRows.size(); i++)
      {
        p_comp = mRows[i];
        if (p_comp->IsValid() == false)
        {
          return false;
        }
      }

      // If we get to this line all is valid :-)
      return true;
    }

    /*************************************************************************
    * Function - Run
    * DESCRIPTION:
    *************************************************************************/
    void ListView::Run( )
    {
      if (!mValid && mSelectedRow >= 0)
      {
        SelectNextAvailable(mSelectedRow);
      }

      Component::Run();

      // Redrawing the scrollbar first gives a better user experience.
      mpScrollBar->Run();

      // Give runtime to each row in the list view.
      for (int i = 0; i < mRows.size(); i++)
      {
        if (mRows[i] != NULL)
        {
          mRows[i]->Run();
        }
      }
    }

    /*************************************************************************
    * Function - IsReadOnly
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::IsReadOnly()
    {
      if(mCallingChild)
        return false;

      if(Component::IsReadOnly())
        return true;

      int selected_index = GetSelection();
      ListViewItem* selected_item = GetRow(selected_index);
      if (selected_item == NULL)
      {
        return false;
      }
      mCallingChild = true;
      bool rc = selected_item->IsReadOnly();
      mCallingChild = false;
      return rc;
    }

    /*************************************************************************
    * Function - IsInEditMode
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::IsInEditMode()
    {
      if( IsReadOnly() )
        return false;

      int selected_index = GetSelection();
      ListViewItem* selected_item = GetRow(selected_index);
      if (selected_item == NULL)
      {
        return false;
      }
      return selected_item->IsInEditMode();
    }

    /*************************************************************************
    * Function - BeginEdit
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::BeginEdit()
    {
      if( IsReadOnly() )
        return false;

      int selected_index = GetSelection();
      ListViewItem* selected_item = GetRow(selected_index);
      if (selected_item == NULL)
      {
        return false;
      }
      return selected_item->BeginEdit();
    }

    /*************************************************************************
    * Function - CancelEdit
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::CancelEdit()
    {
      if( IsReadOnly() )
        return false;

      int selected_index = GetSelection();
      ListViewItem* selected_item = GetRow(selected_index);
      if (selected_item == NULL)
      {
        return false;
      }
      return selected_item->CancelEdit();
    }

    /*************************************************************************
    * Function - EndEdit
    * DESCRIPTION:
    *************************************************************************/
    bool ListView::EndEdit()
    {
      if( IsReadOnly() )
        return false;

      int selected_index = GetSelection();
      ListViewItem* selected_item = GetRow(selected_index);
      if (selected_item == NULL)
      {
        return false;
      }
      return selected_item->EndEdit();
    }

    /*************************************************************************
    * Function - GetRowCount
    * DESCRIPTION:
    *************************************************************************/
    U16 ListView::GetRowCount()
    {
      U16 row_count = 0;
      for (int i = 0; i < mRows.size(); i++)
      {
        if (mRows[i]->IsNeverAvailable() == false)
        {
          row_count++;
        }
      }
      return row_count;
    }

    /*************************************************************************
    * Function - Destroy
    * DESCRIPTION:
    * Destroys the WM window allocated by Initialize.
    * The component object remains, allocated.
    *************************************************************************/
    void ListView::Destroy()
    {
      for (int i = 0; i < mRows.size(); i++)
      {
        if (mRows[i] != NULL)
        {
          mRows[i]->Destroy();
        }
      }
      
      mpScrollBar->Destroy();
      Component::Destroy();
    }

    /*************************************************************************
    * Function - SetFocus
    * DESCRIPTION:
    *************************************************************************/
    void ListView::SetFocus(bool focus)
    {
      Component* parent = Component::GetParent();
      if (parent != NULL && !parent->HasFocus())
      {
        // If this listview is a child of an AnalogInputSetupListView (group), make sure it has a selected child
        AnalogInputSetupListView* analog_list_view = (dynamic_cast<AnalogInputSetupListView*> (parent));
        if (analog_list_view != NULL && analog_list_view->GetCurrentChild() == NULL)
        {
          analog_list_view->SetCurrentChild(analog_list_view->GetFirstListView());
        }

        parent->SetFocus(focus);
        mpScrollBar->SetFocus(focus);
      }

      Component::SetFocus(focus);
    }


#ifdef __PC__
    /*************************************************************************
    * Function - CalculateStringWidths
    * DESCRIPTION:
    *************************************************************************/
    void ListView::CalculateStringWidths(bool forceVisible)
    {
      Component::CalculateStringWidths(forceVisible);

      for (int i = 0; i < mRows.size(); i++)
      {
        if (mRows[i] != NULL)
        {
          if (forceVisible)
          {
            mRows[i]->SetVisible();
          }
          mRows[i]->Run();
          mRows[i]->Redraw();
          mRows[i]->CalculateStringWidths(forceVisible);

          if (forceVisible && mRows[i]->IsSelectable())
          {
            SelectNextAvailable(1 + GetSelection());
            this->Run();
          }
        }
      }
    }


#endif

  } // namespace display
} // namespace mpc
