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
/* CLASS NAME       : MultiStringChar                                       */
/*                                                                          */
/* FILE NAME        : MultiStringChar.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 2008-02-11                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for showing and editing a MultiStringChar      */
/* (a-zA-Z0-9.,-/# ). the space-char can be used for deleting chars         */
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
#include "MultiStringChar.h"
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
    MultiStringChar::MultiStringChar(Component* pParent) : ObserverText(pParent),
      mMinCharIdx(1),
      mMaxCharIdx(strlen(sMultiStringChars) - 1)
    {
      SetAlign(GUI_TA_VCENTER|GUI_TA_HCENTER);
      SetWordWrap(false);
      SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      SetLeftMargin(0);
      SetRightMargin(0);

      mCapturedCharIdx = 0;
      mOriginalCharIdx = 0;

      mToggle = true;
      mFlashingState = NORMAL;
      mFlashingCounter = 0;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    MultiStringChar::~MultiStringChar()
    {
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiStringChar::Init(U8 position, U8 noOfChars)
    {
      if (position >= noOfChars)
      {
        FatalErrorOccured("MultiString: position OOR");
      }

      mPosition = position;
      mNoOfChars = noOfChars;
      mIsEdited = false;
      
      UpdateCapturedValue(false);
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiStringChar::Run()
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
            if ( mFlashingCounter > MULTISTRING_FLASH_VALUE )
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
    bool MultiStringChar::Redraw()
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

        the_string_to_show[0] = sMultiStringChars[mCapturedCharIdx];
        
        if (mCapturedCharIdx == strlen( sMultiStringChars ) - 1)
        {
          ObserverText::SetText(MULTISTRING_NO_LETTER_ENTERED_CHAR);
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
    void MultiStringChar::UpdateCapturedValue(bool isInEditMode)
    {
      StringDataPoint* pDP  = dynamic_cast<StringDataPoint*>(GetSubject());  
      
      if (pDP != NULL)
      {
        char* p_string = new char[mNoOfChars + 1];
        strncpy(p_string, pDP->GetValue(), mNoOfChars);
        p_string[mNoOfChars] = '\0';
        
        p_string[mPosition + 1] = '\0';

        int char_idx = (strstr( sMultiStringChars, &p_string[mPosition]) - sMultiStringChars);

        delete p_string;

        if (char_idx >= mMinCharIdx && char_idx <= mMaxCharIdx) 
        {
          mCapturedCharIdx = (U8) char_idx;
        }
        else
        {
          if (isInEditMode)
          {
            mCapturedCharIdx = strlen( sMultiStringChars ) - 1; // select index of unused digit
          }
          else 
          {
            mCapturedCharIdx = strlen( sMultiStringChars ) - 2; // select index of space
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
    void MultiStringChar::UpdateSubjectValue()
    {  
      if(mIsEdited)
      {
        mIsEdited = false;
        StringDataPoint* pDP  = dynamic_cast<StringDataPoint*>(GetSubject());  
        if (pDP != NULL)
        {
          char* p_string = new char[mNoOfChars + 1];
          
          strncpy(p_string, pDP->GetValue(), mNoOfChars);
          p_string[mNoOfChars] = '\0';
          
          p_string[mPosition] = sMultiStringChars[mCapturedCharIdx];
          mOriginalCharIdx = mCapturedCharIdx;

          pDP->SetValue(p_string);
          pDP->ValidateStringUTF8(true, MULTISTRING_REPLACE_ILLEGAL_CHAR);

          delete p_string;
        }        
      }
    }

    void MultiStringChar::Update(Subject* pSubject)
    {
      UpdateCapturedValue(true);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool MultiStringChar::HandleKeyEvent(Keys KeyID)
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
              mIsEdited = true;

              if (mCapturedCharIdx > mMaxCharIdx)
              {
                mCapturedCharIdx = mMinCharIdx;
              }
              break;
            }

          case MPC_MINUS_KEY     :
          case MPC_MINUS_KEY_REP :
            {
              mCapturedCharIdx--;
              mIsEdited = true;

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
    Keys MultiStringChar::GetLegalKeys()
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
    Leds  MultiStringChar::GetLedsStatus()
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
    bool MultiStringChar::BeginEdit()
    {
      mIsEdited = false;
      if ( !ObserverText::BeginEdit() )
      {
        return false;
      }

      UpdateCapturedValue(true);
      
      mFlashingState = FLASHING;
      mFlashingCounter = 0;
      SetDenyFlags( mLegalKeys, MPC_HOME_KEY | MPC_MENU_KEY);

      return true;
    }

    /* --------------------------------------------------
    * Cancels the edit mode started by a Call to BeginEdit.
    * The value being edited should return to the original.
    * --------------------------------------------------*/
    bool MultiStringChar::CancelEdit()
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
    bool MultiStringChar::EndEdit()
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




