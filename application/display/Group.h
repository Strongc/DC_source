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

#ifndef mpc_displayGroup
#define mpc_displayGroup

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
  // FOWARD declarations
  namespace display
  {
    class Group : public Component
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      Group(Component* pParent = NULL);

      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~Group();

      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual int AddChild(Component* child);
      virtual bool RemoveChild(Component* child);
      virtual Component* GetFirstChild();
      virtual Component* GetNextChild(Component* child);
      virtual Component* GetCurrentChild();
      virtual bool SetCurrentChild(Component* child);
      virtual U16 GetChildCount();

      virtual bool Redraw();

      /* --------------------------------------------------
      * Sets the background colour of the component and all childs.
      * --------------------------------------------------*/
      virtual void SetBackgroundColourRecursive(U32 Colour);
      /* --------------------------------------------------
      * Sets the colour of the component and all childs.
      * --------------------------------------------------*/
      virtual void SetColourRecursive(U32 Colour, bool forced = false);
      virtual void SetColour(U32 colour, bool forced = false);

     /* --------------------------------------------------
      * Validates the component and all its childs.
      * --------------------------------------------------*/
      virtual void ValidateRecursively();

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
      * Called by operating system to give time to redraw
      * --------------------------------------------------*/
      virtual void Run(void);


      /* --------------------------------------------------
      * Returns false if the group or any of its childs 
      * needs redrawing 
      * --------------------------------------------------*/
      virtual bool IsValid();

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

      virtual Display* GetDisplay();

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

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      std::vector< Component* > mChildren;     // All children.
      Component*                mCurrentChild; // Child with focus.
      bool                      mCallingChild;  // flag to avoid parent call child, child call parent stack overflow
    };
  } // namespace display
} // namespace mpc

#endif // mpcGroup
