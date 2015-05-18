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
/* FILE NAME        : NumberQuantity.cpp                                    */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "NumberQuantity.h"
#include "Languages.h"
#include "MpcUnit/MpcUnits.h"
#include <math.h>
/*****************************************************************************
DEFINES
*****************************************************************************/
#define AVERAGE_CHAR_WIDTH 5.5f //pixels

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
  /*****************************************************************************
  *
  *
  *              PUBLIC FUNCTIONS
  *
  *
  *****************************************************************************/

    /*****************************************************************************
    * Constructor
    * DESCRIPTION:
    *****************************************************************************/
    NumberQuantity::NumberQuantity(Component* pParent) : ObserverText( pParent )
    {
      mpNumber = new Number(this);
      mpNumber->SetAlign(GUI_TA_RIGHT);
      mpNumber->SetLeftMargin(0);
      mpNumber->SetWordWrap(false);
      mpQuantity = new Quantity(this);
      mpQuantity->SetAlign(GUI_TA_LEFT);
      mpQuantity->SetLeftMargin(1);
      mpQuantity->SetWordWrap(false);
      SetWordWrap(false);

      mHideOnNeverAvailable = false;
    }

    /*****************************************************************************
    * Destructor
    * DESCRIPTION:
    *****************************************************************************/
    NumberQuantity::~NumberQuantity()
    {
      delete mpNumber;
      delete mpQuantity;
    }

    /*****************************************************************************
    * Function - SetId
    * DESCRIPTION:
    *****************************************************************************/
    void NumberQuantity::SetId(int id)
    {
      mpNumber->SetId(id);
      mpQuantity->SetId(id);
      Component::SetId(id);
    }

    /*****************************************************************************
    * Function - SetFont
    * DESCRIPTION: Sets font for both the Number and the Quantity component
    *****************************************************************************/
    void NumberQuantity::SetFont(const GUI_FONT** font)
    {
      SetFont(font, font);
    }

    /*****************************************************************************
    * Function - SetFont
    * DESCRIPTION: Sets font for both the Number and the Quantity component
    *****************************************************************************/
    void NumberQuantity::SetFont(const GUI_FONT** numberFont, const GUI_FONT** quantityFont)
    {
      SetNumberFont(numberFont);
      SetQuantityFont(quantityFont);
    }

    /*****************************************************************************
    * Function - SetNumberFont
    * DESCRIPTION: Sets font for the Number component
    *****************************************************************************/
    void NumberQuantity::SetNumberFont(const GUI_FONT** Font)
    {
      mpNumber->SetFont(Font);
    }

    /*****************************************************************************
    * Function - SetQuantityFont
    * DESCRIPTION: Sets font for the Quantity component
    *****************************************************************************/
    void NumberQuantity::SetQuantityFont(const GUI_FONT** Font)
    {
      mpQuantity->SetFont(Font);
    }

    /*****************************************************************************
    * Function - Run
    * DESCRIPTION: Called by operating system to give time to redraw
    *****************************************************************************/
    void NumberQuantity::Run(void)
    {
      if (!IsVisible())
      {
        return;
      }
      
      // Only quantity should be transparent since number may frequently change
      mpQuantity->SetTransparent(IsTransparent());

      AdjustClientAreas();

      if (!mValid)
      {
        mpNumber->Invalidate();
        mpQuantity->Invalidate();
        mValid = true;
      }
      mpNumber->Run();
      mpQuantity->Run();
        
    }


    /*****************************************************************************
    * Function - Validate
    * DESCRIPTION: Validates both the Number and the Quantity component
    *****************************************************************************/
    void NumberQuantity::Validate()
    {
      ObserverText::Validate();
      mpNumber->Validate();
      mpQuantity->Validate();
    }

    /*****************************************************************************
    * Function - SetColour
    * DESCRIPTION: Set colour on both the Number and the Quantity component
    *****************************************************************************/
    void NumberQuantity::SetColour(U32 colour, bool forced /*= false*/)
    {
      ObserverText::SetColour(colour, forced);
      mpNumber->SetColour(colour, forced);
      mpQuantity->SetColour(colour, forced);
    }

    /*****************************************************************************
    * Function - SetBackgroundColour
    * DESCRIPTION: Set colour on both the Number and the Quantity component
    *****************************************************************************/
    void NumberQuantity::SetBackgroundColour(U32 colour)
    {
      ObserverText::SetBackgroundColour(colour);
      mpNumber->SetBackgroundColour(colour);
      mpQuantity->SetBackgroundColour(colour);
    }

    /*****************************************************************************
    * Function - SetVisible
    * DESCRIPTION: Set visible on both the Number and the Quantity component
    *****************************************************************************/
    void NumberQuantity::SetVisible(bool set /*= true*/)
    {
      ObserverText::SetVisible(set);
      mpNumber->SetVisible(set);
      mpQuantity->SetVisible(set);
    }

    /*****************************************************************************
    * Function - SetQuantityType
    * DESCRIPTION:
    *****************************************************************************/
    void NumberQuantity::SetQuantityType(QUANTITY_TYPE type)
    {
      mpQuantity->SetQuantityType(type);
      Invalidate();
    }

    /*****************************************************************************
    * Function - GetQuantityType
    * DESCRIPTION:
    *****************************************************************************/
    QUANTITY_TYPE NumberQuantity::GetQuantityType()
    {
      return mpQuantity->GetQuantityType();
    }

    /*****************************************************************************
    * Function - SetNumberOfDigits
    * DESCRIPTION:
    *****************************************************************************/
    void NumberQuantity::SetNumberOfDigits(int numberOfDigits)
    {
      mpNumber->SetNumberOfDigits(numberOfDigits);
    }

    /*****************************************************************************
    * Function - GetNumberOfDigits
    * DESCRIPTION:
    *****************************************************************************/
    int NumberQuantity::GetNumberOfDigits()
    {
      return mpNumber->GetNumberOfDigits();
    }

    /*****************************************************************************
    * Function - Update
    * DESCRIPTION:
    * Update is part of the observer pattern
    *****************************************************************************/
    void NumberQuantity::Update(Subject* Object)
    {
      if (mHideOnNeverAvailable)
      {
        SetVisible(!IsNeverAvailable());
      }
    }
    /*****************************************************************************
    * Function - SubscribtionCancelled
    * DESCRIPTION:
    * Called if subscription shall be canceled
    *****************************************************************************/
    void NumberQuantity::SubscribtionCancelled(Subject* pSubject)
    {
      mpNumber->SubscribtionCancelled(pSubject);
      mpQuantity->SubscribtionCancelled(pSubject);
    }

    /*****************************************************************************
    * Function - SetSubjectPointer
    * DESCRIPTION:
    * Called to set the subject pointer (used by class
    * factory)
    *****************************************************************************/
    void NumberQuantity::SetSubjectPointer(int id, Subject* pSubject)
    {
      mpNumber->SetSubjectPointer(id, pSubject);
      mpQuantity->SetSubjectPointer(id, pSubject);
      ObserverText::SetSubjectPointer(id, pSubject);
    }

    /*****************************************************************************
    * Function - ConnectToSubjects
    * DESCRIPTION:
    *****************************************************************************/
    void NumberQuantity::ConnectToSubjects(void)
    {
      mpNumber->ConnectToSubjects();
      mpQuantity->ConnectToSubjects();
    }

    /*****************************************************************************
    * Function - HandleKeyEvent
    * DESCRIPTION:
    * Handle this key event. Return true if it is taken care of
    *****************************************************************************/
    bool NumberQuantity::HandleKeyEvent(Keys KeyID)
    {
       if (mpNumber->HandleKeyEvent(KeyID))
       {
         return true;
       }
       return ObserverText::HandleKeyEvent(KeyID);
    }

    /*****************************************************************************
    * Function - SetReadOnly
    * DESCRIPTION:
    * Sets or resets the readonly flag, and invalidates the component. The
    * function returns the previous readonly state.
    *****************************************************************************/
    bool NumberQuantity::SetReadOnly(bool readOnly /*= true*/)
    {
      bool prev = Component::SetReadOnly(readOnly);
      mpNumber->SetReadOnly(readOnly);
      return prev;
    }

    /*****************************************************************************
    * Function - GetLedsStatus
    * DESCRIPTION:
    * Gets the LEDs which this element and the ones below, wants to be on
    * or off. Normaly this is also the keys to react, but it doesn't have
    * to be this way ...
    *****************************************************************************/
    Leds NumberQuantity::GetLedsStatus()
    {
      return mpNumber->GetLedsStatus() | ObserverText::GetLedsStatus();
    }

    /*****************************************************************************
    * Function - IsInEditMode
    * DESCRIPTION:
    *****************************************************************************/
    bool NumberQuantity::IsInEditMode()
    {
      return mpNumber->IsInEditMode();
    }

    /*****************************************************************************
    * Function - GetLegalKeys
    * DESCRIPTION:
    * Gets the keys to which we should react in this element and the
    * elements below. Thereby this is the element to send key messages
    * to for the given keys.
    *****************************************************************************/
    Keys NumberQuantity::GetLegalKeys()
    {
      return mpNumber->GetLegalKeys() | ObserverText::GetLegalKeys();
    }

    /*****************************************************************************
    * Function - SetAlign
    * DESCRIPTION:
    * Sets the alignment of the element
    *****************************************************************************/
    void NumberQuantity::SetAlign(int Align)
    {
       mpNumber->SetAlign(GUI_TA_VCENTER | GUI_TA_RIGHT);
       mpQuantity->SetAlign(GUI_TA_VCENTER | GUI_TA_LEFT);
    }

    /*****************************************************************************
    * Function - IsValid
    * DESCRIPTION:
    *****************************************************************************/
    bool NumberQuantity::IsValid()
    {
      if (!mVisible)
      {
        return true;
      }

      return mpNumber->IsValid() && mpQuantity->IsValid() && Component::IsValid();
    }

    /*****************************************************************************
    * Function - IsNeverAvailable
    * DESCRIPTION:
    *****************************************************************************/
    bool NumberQuantity::IsNeverAvailable()
    {
      return mpNumber->IsNeverAvailable() || mpQuantity->IsNeverAvailable();
    }

    /*****************************************************************************
    * Function - IsCaptured
    * DESCRIPTION:
    *****************************************************************************/
    bool NumberQuantity::IsCaptured(void)
    {
      if (mpNumber)
      {
        return mpNumber->IsCaptured();
      }
      return false;
    }

    /*****************************************************************************
    * Function - CancelEdit
    * DESCRIPTION:
    * Cancels the edit mode started by a Call to BeginEdit.
    * The value being edited should return to the original.
    *****************************************************************************/
    bool NumberQuantity::CancelEdit()
    {
      mpNumber->CancelEdit();
      mpQuantity->CancelEdit();
      Invalidate();
      return true;
    }

    /*****************************************************************************
    * Function - Destroy
    * DESCRIPTION: Destroys the WM window allocated by Initialize.
    * The component object remains, allocated.
    *****************************************************************************/
    void NumberQuantity::Destroy()
    {
      mpNumber->Destroy();
      mpQuantity->Destroy();
      ObserverText::Destroy();
    }

    /*****************************************************************************
    * Function - SetUpdateDelay
    * DESCRIPTION:
    *****************************************************************************/
    U16 NumberQuantity::SetUpdateDelay(U16 newUpdateDelay)
    {
      return mpNumber->SetUpdateDelay(newUpdateDelay);
    }

    /*****************************************************************************
    * Function - ClearArea
    * DESCRIPTION:
    *****************************************************************************/
    void NumberQuantity::ClearArea()
    {
      ObserverText::ClearArea();
      mpNumber->ClearArea();
      mpQuantity->ClearArea();
    }

    void NumberQuantity::HideOnNeverAvailable(bool hide /*=true*/)
    {
      mHideOnNeverAvailable = hide;
    }

    void NumberQuantity::AdjustClientAreas()
    {
      float extra_number_pixels = 0;
      
      int max_char_count = (GetWidth()/(2*AVERAGE_CHAR_WIDTH)) - 1 ;

      if (mpQuantity->GetText() != NULL)
      {
        int char_count = strlen(mpQuantity->GetText()) - 1;

        // make extra space for large quantities like kWh/m3 (specific energy)            
        if (char_count > max_char_count)
        {
          extra_number_pixels -= AVERAGE_CHAR_WIDTH * (char_count - max_char_count);
        }
      }

      if (mpNumber->GetText() != NULL)
      {
        int char_count = strlen(mpNumber->GetText());

        // make extra space for large numbers like 500000 h (operating hours)
        if (char_count == max_char_count)
        {
          extra_number_pixels = 0;
        }
        else if (char_count > max_char_count)
        {
          extra_number_pixels += AVERAGE_CHAR_WIDTH * (char_count - max_char_count);
        }
      }

      if (abs(extra_number_pixels) > (GetWidth()/4))
      {
        extra_number_pixels = (extra_number_pixels > 0 ? GetWidth()/4 : -GetWidth()/4);
      }

      mpNumber->SetClientArea(0, 0, GetWidth()/2 + (int)extra_number_pixels, GetHeight());
      mpQuantity->SetClientArea(GetWidth()/2 + 1 + (int)extra_number_pixels, 0, GetWidth(), GetHeight());

    }

#ifdef __PC__
    /*****************************************************************************
    * Function - CalculateStringWidths
    * DESCRIPTION:
    *****************************************************************************/
    void NumberQuantity::CalculateStringWidths(bool forceVisible)
    {
      Component::CalculateStringWidths(forceVisible);
      mpQuantity->CalculateStringWidths(forceVisible);
    }
#endif

  } // namespace display
} // namespace mpc
