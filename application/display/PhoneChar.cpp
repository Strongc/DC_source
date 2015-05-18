/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : PhoneChar                                             */
/*                                                                          */
/* FILE NAME        : PhoneChar.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 2007-11-21                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for showing and editing a PhoneChar            */
/* ('0'-'9','+',' '). the space-char can be used for deleting digits        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <StringDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "PhoneChar.h"
#include <MpcFonts.h>

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
    PhoneChar::PhoneChar(Component* pParent) : ObserverText(pParent)
    {
      SetAlign(GUI_TA_VCENTER|GUI_TA_HCENTER);
      SetWordWrap(false);
      SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      SetLeftMargin(0);
      SetRightMargin(0);

      mCapturedCharIdx = 0;
      mOriginalCharIdx = 0;
      mMinCharIdx = 0;
      mMaxCharIdx = 0;

      mToggle = true;
      mFlashingState = NORMAL;
      mFlashingCounter = 0;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    PhoneChar::~PhoneChar()
    {
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void PhoneChar::Init(U8 position, bool includePlus)
    {
      if (position >= PHONENUMBER_MAX_LEN)
      {
        FatalErrorOccured("Phone no. position OOR");
      }

      mPosition = position;

      if (includePlus)
      {
        mMinCharIdx = 1;
        mMaxCharIdx = strlen( sPhoneChars ) - 1; 
      }
      else
      {
        mMinCharIdx = 2;// exclude '+'
        mMaxCharIdx = strlen( sPhoneChars ) - 2; 
      }

      UpdateCapturedValue(false);
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void PhoneChar::Run()
    {
      if (IsVisible())
      {
        ObserverText::Run();

        switch ( mFlashingState )
        {
          case NORMAL:
            break;
          case FLASHING: 
            mFlashingCounter++;
            if ( mFlashingCounter > PHONENUMBER_FLASH_VALUE )
            {
              mFlashingCounter = 0;
              mToggle = !mToggle;
              Invalidate();
            }
            break;
          default: 
            mFlashingState = NORMAL;
            break;
        }
      }//else do nothing
    }

    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool PhoneChar::Redraw()
    {
      char the_string_to_show[2];
      the_string_to_show[1] = '\0';
      if (IsVisible())
      {
        if (IsInEditMode())
        {
          
          ObserverText::SetColour(mToggle 
            ? GUI_COLOUR_TEXT_EDITMODE_FOREGROUND 
            : GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);      
        }

        the_string_to_show[0] = sPhoneChars[mCapturedCharIdx];

        if (mCapturedCharIdx == (strlen(sPhoneChars) - 1))
        {
          ObserverText::SetText(PHONENUMBER_NO_LETTER_ENTERED_CHAR);
        }
        else         
        {
          ObserverText::SetText(the_string_to_show);
        }

        return ObserverText::Redraw();
      }
      return false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void PhoneChar::UpdateCapturedValue(bool isInEditMode)
    {
      StringDataPoint* pDP  = dynamic_cast<StringDataPoint*>(GetSubject());  
      
      if (pDP != NULL)
      {
        char phone_number[PHONENUMBER_MAX_LEN + 1]; 

        strncpy(phone_number, pDP->GetValue(), PHONENUMBER_MAX_LEN);
        phone_number[mPosition + 1] = '\0';

        int char_idx = (strstr( sPhoneChars, &phone_number[mPosition]) - sPhoneChars);

        if (char_idx >= mMinCharIdx && char_idx <= mMaxCharIdx) 
        {
          mCapturedCharIdx = (U8) char_idx;
        }
        else
        {
          if (isInEditMode)
          {
            mCapturedCharIdx = strlen( sPhoneChars ) - 1; // select index of unused digit
          }
          else 
          {
            mCapturedCharIdx = strlen( sPhoneChars ) - 2; // select index of space
          }
        }
                  
      }
      else 
      {
        mCapturedCharIdx = mMinCharIdx;
      }

      if (mOriginalCharIdx != mCapturedCharIdx)
      {
        Invalidate();
      }

      mOriginalCharIdx = mCapturedCharIdx;      
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void PhoneChar::UpdateSubjectValue()
    {
      StringDataPoint* pDP  = dynamic_cast<StringDataPoint*>(GetSubject());  
      
      if (pDP != NULL)
      {

        char phone_number[PHONENUMBER_MAX_LEN + 1]; 

        strncpy(phone_number, pDP->GetValue(), PHONENUMBER_MAX_LEN);
        phone_number[PHONENUMBER_MAX_LEN] = '\0';

        phone_number[mPosition] = sPhoneChars[mCapturedCharIdx];

        pDP->SetValue(phone_number);
       

        mOriginalCharIdx = mCapturedCharIdx;
      }
    }

    void PhoneChar::Update(Subject* pSubject)
    {
      UpdateCapturedValue(true);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool PhoneChar::HandleKeyEvent(Keys KeyID)
    {
      if (IsReadOnly())
      {
        return ObserverText::HandleKeyEvent(KeyID);
      }

      if (IsInEditMode())
      {
         
        switch (KeyID)
        {
          case MPC_PLUS_KEY      :
          case MPC_PLUS_KEY_REP  :
            {
              mCapturedCharIdx++;

              if (mCapturedCharIdx > mMaxCharIdx){
                mCapturedCharIdx = mMinCharIdx;
              }
              break;
            }

          case MPC_MINUS_KEY     :
          case MPC_MINUS_KEY_REP :
            {
              mCapturedCharIdx--;

              if (mCapturedCharIdx < mMinCharIdx)
              {
                mCapturedCharIdx = mMaxCharIdx;
              }

              break;
            }

          default                :
            return ObserverText::HandleKeyEvent(KeyID);
        }

        Invalidate();
        return true;
      }
      else
      {
        return ObserverText::HandleKeyEvent(KeyID);
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    Keys PhoneChar::GetLegalKeys()
    {
      if ( !IsReadOnly() )
      {
        SetAllowFlags(mLegalKeys, MPC_UP_KEY | MPC_DOWN_KEY | MPC_OK_KEY | MPC_ESC_KEY | MPC_PLUS_KEY | MPC_MINUS_KEY | MPC_PLUS_KEY_REP | MPC_MINUS_KEY_REP);

        if ( IsInEditMode() )
        { 
          SetDenyFlags(mLegalKeys, MPC_HOME_KEY | MPC_MENU_KEY);

          SetAllowFlags(mLegalKeys, MPC_UP_KEY | MPC_DOWN_KEY);
        }
        else
        {
          // Remove all flags that might have been set when we were in edit mode.
          RemoveDenyFlags(mLegalKeys, MPC_HOME_KEY | MPC_MENU_KEY);

          RemoveAllowFlags(mLegalKeys, MPC_UP_KEY | MPC_DOWN_KEY);
        }
      }
      return ObserverText::GetLegalKeys();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    Leds  PhoneChar::GetLedsStatus()
    {
      if ( !IsReadOnly() )
      {
        if ( IsInEditMode() )
        {
          return COMBINE(MENU_LED + HOME_LED, UP_LED + DOWN_LED + PLUS_LED + MINUS_LED + ESC_LED + OK_LED);
        }
        else
        {
          return COMBINE(NO_LED, UP_LED + DOWN_LED + PLUS_LED + MINUS_LED + OK_LED);
        }
      }
      else
      {
        return COMBINE(NO_LED,NO_LED);
      }
    }

    /* --------------------------------------------------
    * Puts the component in edit mode.
    * --------------------------------------------------*/
    bool PhoneChar::BeginEdit()
    {
      if ( !ObserverText::BeginEdit() )
      {
        return false;
      }

      UpdateCapturedValue(true);

      mToggle = true;
      mFlashingState = FLASHING;
      mFlashingCounter = 0;
      SetDenyFlags( mLegalKeys, MPC_HOME_KEY | MPC_MENU_KEY);

      return true;
    }

    /* --------------------------------------------------
    * Cancels the edit mode started by a Call to BeginEdit.
    * The value being edited should return to the original.
    * --------------------------------------------------*/
    bool PhoneChar::CancelEdit()
    {
      if ( !ObserverText::CancelEdit() )
      {
        return false;
      }

      mCapturedCharIdx = mOriginalCharIdx;
      
      mFlashingState = NORMAL;
      ObserverText::SetColour(GUI_COLOUR_TEXT_DEFAULT_FOREGROUND);

      Invalidate();
      RemoveDenyFlags( mLegalKeys, MPC_HOME_KEY | MPC_MENU_KEY);
      return true;
    }

    /* --------------------------------------------------
    * Ends edit mode and stores the new value.
    * --------------------------------------------------*/
    bool PhoneChar::EndEdit()
    {
      if ( !ObserverText::EndEdit() )
      {
        return false;
      }

      UpdateSubjectValue();
      
      mFlashingState = NORMAL;
      ObserverText::SetColour(GUI_COLOUR_TEXT_DEFAULT_FOREGROUND);
      Invalidate();
      RemoveDenyFlags( mLegalKeys, MPC_HOME_KEY | MPC_MENU_KEY);
      return true;
    }


  } // namespace display
} // namespace mpc




