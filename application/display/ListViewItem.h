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
/* CLASS NAME       : DisplayGroup                                          */
/*                                                                          */
/* FILE NAME        : DisplayGroup.h                                        */
/*                                                                          */
/* CREATED DATE     : 2004-10-18                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for holding and drawing a single row in a      */
/* ListView.                                                                */
/*                                                                          */
/****************************************************************************/

#ifndef mpcListViewItem
#define mpcListViewItem

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <vector>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Observer.h>
#include <IIntegerDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Component.h"

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
    class ListViewItem : public Component, public Observer
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      ListViewItem(Component* pParent = NULL);

      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~ListViewItem();

      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual bool InsertItem(U16 itemIndex, Component* comp);
      virtual bool SetItem(U16 itemIndex, Component* comp);
      virtual Component* GetItem(U16 itemIndex);
      virtual int GetColumnCount();

      virtual void SetSelected(bool select=true);
      virtual bool IsSelected();
      virtual bool Redraw();

      virtual void Run(void);
      virtual bool IsValid();

      virtual void SetSelColour(U32 colour);
      virtual void SetSelBgColour(U32 colour);
      virtual void SetSelFrameColour(U32 colour);
      virtual void SetColour(U32 colour, bool forced = false);

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
      * Returns true if a subject is connected and has a 
      * given value.
      * If a subject isn't connected, true is only returned
      * if one of the Components of type ObserverText in the 
      * ListViewItem returns true from a call to 
      * ObserverText::IsNeverAvailible.
      * 
      * The ListView only draws the ListViewItem if
      * IsNeverAvailible return false.
      * --------------------------------------------------*/
      virtual bool IsNeverAvailable();

      bool IsSelectable(void);
      /**
      * Sets the value wich hiddes the row if the subjects
      * of type DataPoint<int> GetValue equals the state.
      */
      virtual void SetHideState(int state);

      virtual void SubscribtionCancelled(Subject* pSubject);
      virtual void Update(Subject* pSubject);
      virtual void SetSubjectPointer(int id,Subject* pSubject);
      virtual void ConnectToSubjects(void);

      /* --------------------------------------------------
      * Destroys the WM window allocated by Initialize.
      * The component object remains, allocated.
      * --------------------------------------------------*/
      virtual void Destroy();

#ifdef __PC__
      virtual void CalculateStringWidths(bool forceVisible);
#endif
    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      bool  mLastNeverAvailable;

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      void DetermineEditStyle(void);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      std::vector< Component* > mColumns;
      bool  mSelected;
      int   mNextSelectedColumnIndex;
      bool  mIsEditStyleDetermined;
      bool  mUseSingleCellEditStyle;
      bool  mIsUnselectedColoursSet;

      U32   mSelectedColourForeground;
      U32   mSelectedColourBackground;
      U32   mUnselectedColourForeground;
      U32   mUnselectedColourBackground;
      U32   mFrameColour;

      int   mHideState;
      SubjectPtr<IIntegerDataPoint*> mpDataPoint;
      bool  mCallingChild;  // flag to avoid parent call child, child call parent stack overflow

    };
  } // namespace display
} // namespace mpc

#endif // mpcDisplayGroup
