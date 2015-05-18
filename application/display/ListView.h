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
/* FILE NAME        : ListView.h                                            */
/*                                                                          */
/* CREATED DATE     : 2004-10-18                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Draws a list of DisplayComponent in a  row/column style.                 */
/* The list can be navigated with a row selection bar.                      */
/*                                                                          */
/****************************************************************************/

#ifndef mpc_displayListView
#define mpc_displayListView

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <vector>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Component.h"
#include "ListViewItem.h"
#include "ScrollBar.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
* CLASS:
* DESCRIPTION:
*
* This Class is responsible for grouping and drawing displaycomponents.
*
*****************************************************************************/

namespace mpc
{
  namespace display
  {
  // FOWARD declarations
  class ListView : public Component
  {
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    ListView(Component* pParent = NULL);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~ListView();

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual bool InsertColumn(U16 columnIndex);
    virtual int InsertItem(U16 itemIndex, Component* comp);
    virtual int InsertItem(U16 itemIndex, ListViewItem* comp);

    virtual void SetNextList(ListView* pList);
    virtual void SetPrevList(ListView* pList);

    virtual bool SetItem(U16 itemIndex, U16 columnIndex, Component* comp);
    virtual Component* GetItem(U16 itemIndex, U16 columnIndex);
    virtual ListViewItem* GetListViewItem(U16 itemIndex);

    virtual bool SetColumnWidth(U16 columnIndex, U16 width);
    virtual U16 GetColumnWidth(U16 columnIndex);
    virtual U16 GetColCount();

    virtual bool SetSelection(int itemIndex);
    virtual int GetSelection( );
    virtual bool SelectFirstVisibleItem();
    virtual bool SelectLastVisibleItem();

    virtual bool SelectNextAvailable(int itemIndex);
    virtual bool SelectPrevAvailable(int itemIndex);

    virtual U16 SetRowHeight(U16 newHeight);
    virtual bool Redraw();

    virtual void SetSelColour(U32 colour);
    virtual void SetSelBgColour(U32 colour);
    virtual void SetSelFrameColour(U32 colour);
    virtual void SetColour(U32 colour, bool forced = false);

    virtual void Run( );

    virtual bool IsValid();

    /* --------------------------------------------------
    * Gets the LEDs which this element and the ones below, wants to be on
    * or off. Normaly this is also the keys to react, but it doesn't have
    * to be this way ...
    * --------------------------------------------------*/
    virtual Leds GetLedsStatus();
    /* --------------------------------------------------
    * Gets the keys to which we should react in this element and the
    * elements below. Thereby this is the element to send key messages
    * to for the given keys.
    * --------------------------------------------------*/
    virtual Keys GetLegalKeys();
    /* --------------------------------------------------
    * Handle this key event. Return true if it is taken care of
    * --------------------------------------------------*/
    virtual bool HandleKeyEvent(Keys KeyID);

    /* --------------------------------------------------
    * Returns whether this element is readable/writeable
    * --------------------------------------------------*/
    virtual bool IsReadOnly();

    /* --------------------------------------------------
    * Returns true if the component is currently being
    * edited.
    * --------------------------------------------------*/
    virtual bool IsInEditMode();

    /* --------------------------------------------------
    * Puts the component in edit mode.
    * --------------------------------------------------*/
    virtual bool BeginEdit();
    /* --------------------------------------------------
    * Cancels the edit mode started by a Call to BeginEdit.
    * The value being edited should return to the original.
    * --------------------------------------------------*/
    virtual bool CancelEdit();
    /* --------------------------------------------------
    * Ends edit mode and stores the new value.
    * --------------------------------------------------*/
    virtual bool EndEdit();

    /* --------------------------------------------------
    * Destroys the WM window allocated by Initialize.
    * The component object remains, allocated.
    * --------------------------------------------------*/
    virtual void Destroy();

    /* --------------------------------------------------
    * Sets focus on itself and its parent
    * --------------------------------------------------*/ 
    virtual void SetFocus(bool focus = true);

#ifdef __PC__
      virtual void CalculateStringWidths(bool forceVisible);
#endif
  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    int GetFirstVisibleItem();
    int GetLastVisibleItem();
    ListViewItem* GetRow(U16 itemIndex);
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    ScrollBar*  mpScrollBar;
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    U16 GetRowCount();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    bool  mCallingChild;  // flag to avoid parent call child, child call parent stack overflow
    U16   mColumnCount;
    U16   mRowHeight;
    int   mSelectedRow;
    int   mLastSelectedRow;

    bool  mIsInitialized;
    int   mFirstVisibleRow;
    int   mLastVisibleRow;

    std::vector< ListViewItem* > mRows;
    std::vector< int >  mColumnWidths;

    U32   mSelColour;
    U32   mSelBgColour;
    U32   mSelFrameColour;

    ListView* mpNextList;
    ListView* mpPrevList;

  };
  } // namespace display
} // namespace mpc

#endif // mpcDisplayListView
