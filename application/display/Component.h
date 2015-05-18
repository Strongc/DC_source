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
/* FILE NAME        : Component.h                                           */
/*                                                                          */
/* CREATED DATE     : 2004-08-03                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a given element.               */
/* All graphical elements are sub-types of this class and they form the     */
/* basic GUI elements all responsible for drawing themselves acoording to   */
/* the given actual situation.                                              */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayCOMPONENT_h
#define mpc_displayCOMPONENT_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <list>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <WM.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "GUI_Utility/GUI_Utility.h"
#include "GUI_Utility/Leds.h"
#include "GUI_Utility/Keys.h"
#include "GUI_Utility/string_id.h"
#include "GUI_Utility/Colours.h"

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
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a given element.
    * All graphical elements are sub-types of this class and they form a tree
    * of GUI elements all responsible for drawing themselves acoording to the
    * given actual situation.
    *
    *****************************************************************************/

    class Display;

    class Component
    {
    public:
    /********************************************************************
    LIFECYCLE - Default constructor.
      ********************************************************************/
      Component(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~Component();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void  SetId(int id);
      virtual int   GetId();


      virtual void SetClientArea(int x1, int y1, int x2, int y2);
      /* --------------------------------------------------
      * Get/Set child position in cordinates of the parent window.
      * --------------------------------------------------*/
      virtual void SetChildPos(int childX, int childY);
      virtual int GetChildPosX();
      virtual int GetChildPosY();

      /* --------------------------------------------------
      * Get/Set child size
      * --------------------------------------------------*/
      virtual void SetSize(int width, int height);
      virtual void SetWidth(int width);
      virtual void SetHeight(int height);
      virtual int GetWidth();
      virtual int GetHeight();

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
      * Returns whether this element or its parent is readable/writeable
      * --------------------------------------------------*/
      virtual bool IsReadOnly();

      /* --------------------------------------------------
      * Returns whether this element is readable/writeable
      * --------------------------------------------------*/
      bool IsReadOnlySet() {return mReadOnly;}

      virtual bool SetReadOnly(bool readOnly = true);

      virtual void ClearArea();
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      /* --------------------------------------------------
      * Called by operating system to give time to redraw
      * --------------------------------------------------*/
      virtual void Run(void);

      /* --------------------------------------------------
      * Sets the parent
      * --------------------------------------------------*/
      virtual void SetParent(Component* pParent);

      /* --------------------------------------------------
      * Gets the parent
      * --------------------------------------------------*/
      virtual Component* GetParent( );

      /* --------------------------------------------------
      * Sets the input foces to this component.
      * --------------------------------------------------*/
      virtual void SetFocus(bool focus = true);

      /* --------------------------------------------------
      * Returns true if this component has the input focus.
      * --------------------------------------------------*/
      virtual bool HasFocus();

      /* --------------------------------------------------
      * Returns whether this elementis visible or not
      * --------------------------------------------------*/
      virtual bool IsVisible();
      /* --------------------------------------------------
      * Sets or resets the visible property
      * --------------------------------------------------*/
      virtual void SetVisible(bool Set = true);
      /* --------------------------------------------------
      * Returns the background colour of the component
      * --------------------------------------------------*/
      virtual U32 GetBackgroundColour();
      /* --------------------------------------------------
      * Does this component have a background or is it
      * transparent?
      * --------------------------------------------------*/
      virtual bool IsTransparent();

      /* --------------------------------------------------
      * Returns the colour of the component
      * --------------------------------------------------*/
      virtual U32 GetColour();
      /* --------------------------------------------------
      * Returns the background colour of the component
      * --------------------------------------------------*/
      virtual void SetBackgroundColour(U32 Colour);

      /* --------------------------------------------------
      * Does this component have a background or is it
      * transparent?
      * --------------------------------------------------*/
      virtual void SetTransparent(bool Set = true);

      /* --------------------------------------------------
      * Sets the stay on top flag. Components with this 
      * flag set stays on top of components without the 
      * flag set.
      * --------------------------------------------------*/
      virtual void SetStayOnTop(bool stayOnTop = true);
      /* --------------------------------------------------
      * Returns the stay on top flag status
      * --------------------------------------------------*/
      virtual bool GetStayOnTop();

      /* --------------------------------------------------
      * Holds the colour of the component
      * --------------------------------------------------*/
      virtual void SetColour(U32 Colour, bool forced = false);

      /* --------------------------------------------------
      * Invalidates the component, and marks it for
      * redrawing.
      * --------------------------------------------------*/
      virtual void Invalidate();
      /* --------------------------------------------------
      * Validate the component.
      * --------------------------------------------------*/
      virtual void Validate();

      virtual bool IsValid();

      /* --------------------------------------------------
      * Sets the component to be displayed when this tab
      * is active.
      * --------------------------------------------------*/
      virtual void SetDisplay(Display* pDisplay);
      virtual Display* GetDisplay();

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
      * Returns/creates the WM handle of this component.
      * Calls Initialize if a WM handle needs to be allocated.
      * --------------------------------------------------*/
      WM_HWIN GetWMHandle();

      /* --------------------------------------------------
      * Sets or resets the help string for this component.
      * --------------------------------------------------*/
      virtual void SetHelpString(STRING_ID helpString);

      /* --------------------------------------------------
      * Shows a help messagebox, if a help string is set
      * for the Component.
      * --------------------------------------------------*/
      virtual bool ShowHelp();

      /* --------------------------------------------------
      * If the subject is a DataPoint and the quality is
      * DP_NEVER_AVAILABLE this function shall return true
      *
      * The correct design would be to make it pure virtual.
      * --------------------------------------------------*/
      virtual bool IsNeverAvailable();


      /* --------------------------------------------------
      * Creates a WM window used for drawing operations
      * on this object.
      * --------------------------------------------------*/
      virtual void Initialize();
      virtual bool IsInitialized();

      /* --------------------------------------------------
      * Destroys the WM window allocated by Initialize.
      * The component object remains, allocated.
      * --------------------------------------------------*/
      virtual void Destroy();

      /* --------------------------------------------------
      * Selects the WM window for drawing operations. 
      * Calls Initialize if a WM window handle dosn't exist
      * for this component.
      * --------------------------------------------------*/
      virtual void SelectWindow();

      /* --------------------------------------------------
      * Sets whether the component should be selectable 
      * when used in a listview. Default is true.
      * --------------------------------------------------*/
      virtual void SetSelectable(bool selectable = true);

      virtual bool IsSelectable(void);

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

      WM_HWIN mWMHandle;        /* Handle to the WM window allocated for
                                this component. Inherited classes should
                                call GetWMHandle to access this member 
                                variable, since GetWMHandle ensures that
                                a handle is created.
                                */

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/

      int mComponentId;

      Component* mpParent;        // Holds the parent component.
                                  // If this paremeter is NULL (default)
                                  // the entire screen is the parent.

      Display*  mpDisplay;        // Display this component can jump to if any
      STRING_ID mHelpString;
      bool mFocus;
      bool mReadOnly;             // Holds information about
                                  // whether this element is
                                  // read-only or read-write
      bool mValid;
      bool mEditing;
      Keys mLegalKeys;            // Holds information about which
                                  // keys this element wants to
                                  // recive information about if
                                  // pressed
      Leds mLedsStatus;           // Holds information about which
                                  // LEDs this element wants to
                                  // lit
      U32 mBackgroundColour;      // Holds the colour of the background
      U32 mColour;                // Holds the colour of the component
      GUI_RECT    mClientRect;
      bool        mTransparent;
      bool        mVisible;
      bool        mStayOnTop;
      bool        mSelectable;


    }; // class component
  } // namespace display
} // namespace mpc

#endif
