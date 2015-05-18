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
/* CLASS NAME       : NumberQuantity                                        */
/*                                                                          */
/* FILE NAME        : NumberQuantity.h                                      */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayNumberQuantity_h
#define mpc_displayNumberQuantity_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverText.h"
#include "Number.h"
#include "Quantity.h"
#include "GUI_Utility/string_id.h"

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
    *****************************************************************************/
    class NumberQuantity : public ObserverText
    {
    public:
      NumberQuantity(Component* pParent = NULL);
      virtual ~NumberQuantity();

      /********************************************************************
      OPERATIONS
      ********************************************************************/

      virtual void Validate(void);
      virtual void SetColour(U32 colour, bool forced = false);
      virtual void SetBackgroundColour(U32 colour);
      virtual void SetVisible(bool set = true);

      virtual void SetQuantityType(QUANTITY_TYPE type);
      virtual QUANTITY_TYPE GetQuantityType();
      virtual void SetNumberOfDigits(int numberOfDigits);
      virtual int GetNumberOfDigits();

      virtual void ClearArea();
      /* --------------------------------------------------
      * Sets font for both the Number and the graphics component
      * --------------------------------------------------*/
      virtual void SetFont(const GUI_FONT** Font);
      /* --------------------------------------------------
      * Sets font for both the Number and the Quantity component
      * --------------------------------------------------*/
      virtual void SetFont(const GUI_FONT** NumberFont,const GUI_FONT** QuantityFont);
      /* --------------------------------------------------
      * Sets font for the Number component
      * --------------------------------------------------*/
      virtual void SetNumberFont(const GUI_FONT** Font);
      /* --------------------------------------------------
      * Sets font for the Quantity component
      * --------------------------------------------------*/
      virtual void SetQuantityFont(const GUI_FONT** Font);

      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* Object);
      /* --------------------------------------------------
      * Called if subscription shall be canceled
      * --------------------------------------------------*/
      virtual void SubscribtionCancelled(Subject* pSubject);
      /* --------------------------------------------------
      * Called to set the subject pointer (used by class
      * factory)
      * --------------------------------------------------*/
      virtual void SetSubjectPointer(int Id,Subject* pSubject);

      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      virtual void ConnectToSubjects(void);
      /* --------------------------------------------------
      * Handle this key event. Return true if it is taken care of
      * --------------------------------------------------*/
      virtual bool HandleKeyEvent(Keys KeyID);
      /* --------------------------------------------------
      * Gets the keys to which we should react in this element and the
      * elements below. Thereby this is the element to send key messages
      * to for the given keys.
      * --------------------------------------------------*/
      virtual Keys GetLegalKeys();

      /* --------------------------------------------------
      * Called by operating system to give time to redraw
      * --------------------------------------------------*/
      virtual void Run(void);
      /*****************************************************************************
      * Function
      * DESCRIPTION:
      * Sets or resets the readonly flag, and invlidates the component. The
      * function returns the previous readonly state.
      *****************************************************************************/
      bool SetReadOnly(bool readOnly = true);
      /* --------------------------------------------------
      * Gets the LEDs which this element and the ones below, wants to be on
      * or off. Normaly this is also the keys to react, but it doesn't have
      * to be this way ...
      * --------------------------------------------------*/
      virtual Leds GetLedsStatus();
      /* --------------------------------------------------
      * Sets the alignment of the element
      * --------------------------------------------------*/
      virtual void SetAlign(int Align);

      /* --------------------------------------------------
      * Indicate that the DataPoint connected to this NumberQuantity is captured
      * --------------------------------------------------*/
      virtual bool IsCaptured(void);

      virtual bool IsValid();

      /* --------------------------------------------------
      * If the subject is a DataPoint and the quality is
      * DP_NEVER_AVAILABLE this function shall return true
      * --------------------------------------------------*/
      virtual bool IsNeverAvailable();

      /* --------------------------------------------------
      * Sets the id of this component and the Number
      * and Quantity components contained within this cmp.
      * --------------------------------------------------*/
      virtual void SetId(int id);

      /* --------------------------------------------------
      * Cancels the edit mode started by a Call to BeginEdit.
      * The value being edited should return to the original.
      * --------------------------------------------------*/
      virtual bool CancelEdit();

      virtual bool IsInEditMode();

      /* --------------------------------------------------
      * Destroys the WM window allocated by Initialize.
      * The component object remains, allocated.
      * --------------------------------------------------*/
      virtual void Destroy();

      virtual U16 SetUpdateDelay(U16 newUpdateDelay);


      void HideOnNeverAvailable(bool hide = true);


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
      void AdjustClientAreas(void);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      Number* mpNumber;
      Quantity* mpQuantity;

      bool mHideOnNeverAvailable;
    };
  } // namespace display
} // namespace mpc

#endif
