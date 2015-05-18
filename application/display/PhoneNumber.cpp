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
/* CLASS NAME       : PhoneNumber                                           */
/*                                                                          */
/* FILE NAME        : PhoneNumber.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 2007-11-21                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for editing a phone number                     */
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

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "PhoneNumber.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define INDEX_OF_FIRST_EDITABLE_DIGIT 1
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
    PhoneNumber::PhoneNumber(Component* pParent) : Group(pParent)
    {
      mpFrames = NULL;
      mpPhoneChars = NULL;
      mCurrentField = INDEX_OF_FIRST_EDITABLE_DIGIT;
      mpDpCapturedString = new StringDataPoint("", PHONENUMBER_MAX_LEN);

      mpFrames = new Frame[PHONENUMBER_MAX_LEN];
      mpPhoneChars = new PhoneChar[PHONENUMBER_MAX_LEN];

      for (int i=0; i<PHONENUMBER_MAX_LEN; i++)
      {
        mpFrames[i].SetVisible();
        mpFrames[i].SetReadOnly(false);
        mpFrames[i].SetFrame(false);
        AddChild(&mpFrames[i]);
        
        mpPhoneChars[i].SetSubjectPointer(0, mpDpCapturedString);
        mpPhoneChars[i].ConnectToSubjects();
        mpPhoneChars[i].Init(i, i==0 );
        mpPhoneChars[i].SetReadOnly(IsReadOnly());
        mpPhoneChars[i].SetVisible();
        mpFrames[i].AddChild(&mpPhoneChars[i]);
        mpFrames[i].SetCurrentChild(&mpPhoneChars[i]);
      }
      //set focus on second char, since first is '+' (read-only)
      mpFrames[1].SetFrame(true);
      mpPhoneChars[1].SetFocus();
      SetCurrentChild(&mpFrames[1]);
      Invalidate();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    PhoneNumber::~PhoneNumber()
    {
      if (mpFrames != NULL)
      {
        for (int i=0; i<PHONENUMBER_MAX_LEN; i++)
        {
          RemoveChild(&mpFrames[i]);
        }
        delete[] mpFrames;
      }
      if (mpPhoneChars != NULL)
      {
        delete[] mpPhoneChars;
      }
      mCurrentField = INDEX_OF_FIRST_EDITABLE_DIGIT;

      mpDpSubject.UnsubscribeAndDetach(this);
    }

    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    bool PhoneNumber::SetReadOnly(bool readOnly /*= true*/)
    {
      Group::SetReadOnly(readOnly);
      int i;
      for (i=0; i<PHONENUMBER_MAX_LEN; i++)
      {
        mpPhoneChars[i].SetReadOnly(readOnly);
        mpFrames[i].SetReadOnly(readOnly);
      }
      return true;
    }

    /*****************************************************************************
    * Function - SetColour
    * DESCRIPTION:
    * 
    *****************************************************************************/
    /*void PhoneNumber::SetColour(U32 colour, bool forced)
    {
      Group::SetColour(colour, forced);
    }*/

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void PhoneNumber::Run()
    {
      if (mpDpSubject.IsUpdated() && !IsInEditMode())
      {
        PadAndCaptureSubject(false);
      }
      
      Group::Run();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    bool PhoneNumber::IsValid()
    {
      return Component::IsValid();
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. A frame is drawn for the currently selected field
    *****************************************************************************/
    bool PhoneNumber::Redraw()
    {
      const int height = GetHeight();
      int x_pos = 1;
      int frame_width = 7;

      //make right alignment, when not in edit mode
      if (!IsInEditMode())
      {
        int no_of_empty_digits = mpDpSubject->GetMaxNoOfChars() - strlen(mpDpSubject->GetValue());
        x_pos += no_of_empty_digits * frame_width;
      }
      

      for (int i = 0; i<PHONENUMBER_MAX_LEN; i++)
      {
        bool is_selected_field = (i == mCurrentField && HasFocus() && !IsReadOnly() && IsInEditMode());

        if ( is_selected_field )
        {
          frame_width = 16;
        }
        else
        {
          frame_width = 7;
        }

        mpFrames[i].SetClientArea(x_pos, 0, x_pos + frame_width-1, height-1);

        if ( is_selected_field )
        {
          mpPhoneChars[i].SetClientArea(2, 1, frame_width-2, height-2);
        }
        else
        {
          mpPhoneChars[i].SetClientArea(0, 1, frame_width-1, height-2);
        }

        x_pos += frame_width;

        mpFrames[i].SetFrame( is_selected_field );
      }
      
      return Group::Redraw();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * +, - and 'ok' puts the string in edit mode. 
    * ^ and v selects char position (in edit mode)
    * + and - are used for char selection (in edit mode). 
    * 'esc' cancels changes and ends edit mode. 
    * 'ok' commits changes and ends edit mode.
    *****************************************************************************/
    bool PhoneNumber::HandleKeyEvent(Keys KeyID)
    {
      if (IsReadOnly())
      {
        return Group::HandleKeyEvent(KeyID);
      }

      switch(KeyID)
      {
        case MPC_PLUS_KEY:
        case MPC_PLUS_KEY_REP:
        case MPC_MINUS_KEY:
        case MPC_MINUS_KEY_REP:
          if (IsInEditMode())
          {
            return Group::HandleKeyEvent(KeyID);
          }
          else
          {// enter edit mode
            PadAndCaptureSubject(true);

            mpPhoneChars[mCurrentField].BeginEdit();
            mpFrames[mCurrentField].SetFrame(true);
            SetCurrentChild(&mpFrames[mCurrentField]);
          }
          Invalidate();
          return true;

        case MPC_UP_KEY:
          if (IsInEditMode())
          {// select next field
            mpFrames[mCurrentField].SetFrame(false);
            mpPhoneChars[mCurrentField].EndEdit();

            if (mCurrentField < PHONENUMBER_MAX_LEN - 1)
            {
              mCurrentField++;
            }
            else
            {
              mCurrentField = 0;
            }

            mpPhoneChars[mCurrentField].BeginEdit();
            mpFrames[mCurrentField].SetFrame(true);
            SetCurrentChild(&mpFrames[mCurrentField]);
          }
          Invalidate();
          return true;

        case MPC_DOWN_KEY:
          if (IsInEditMode())
          {// select previous field
            mpFrames[mCurrentField].SetFrame(false);
            mpPhoneChars[mCurrentField].EndEdit();

            if (mCurrentField > INDEX_OF_FIRST_EDITABLE_DIGIT)
            {
              mCurrentField--;
            }
            else
            {
              mCurrentField = PHONENUMBER_MAX_LEN - 1;
            }

            mpPhoneChars[mCurrentField].BeginEdit();
            mpFrames[mCurrentField].SetFrame(true);
            SetCurrentChild(&mpFrames[mCurrentField]);
          }
          Invalidate();
          return true;


        case MPC_OK_KEY:
          if (IsInEditMode())
          {// end edit mode
            mpFrames[mCurrentField].SetFrame(false);
            mpPhoneChars[mCurrentField].EndEdit();
            mCurrentField = INDEX_OF_FIRST_EDITABLE_DIGIT;
            if (mpDpSubject.IsValid())
            {
              StripCommitAndRecapture();
            }
          }
          else
          {// enter edit mode
            PadAndCaptureSubject(true);

            mpPhoneChars[mCurrentField].BeginEdit();
            mpFrames[mCurrentField].SetFrame(true);
            SetCurrentChild(&mpFrames[mCurrentField]);
          }
          Invalidate();
          return true;

        case MPC_ESC_KEY:
          if (IsInEditMode())
          {// cancel edit mode
            mpFrames[mCurrentField].SetFrame(false);
            mpPhoneChars[mCurrentField].EndEdit();

            PadAndCaptureSubject(false);

            SetCurrentChild(&mpFrames[mCurrentField]);
          }
          Invalidate();
          return true;

        default:
          return false;
      }

    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    Leds  PhoneNumber::GetLedsStatus()
    {
      if (!IsReadOnly())
      {
        return Group::GetLedsStatus();
      }
      else
      {
        return COMBINE(NO_LED,NO_LED);
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * observer pattern method
    *****************************************************************************/
    void PhoneNumber::SubscribtionCancelled(Subject* pSubject)
    {
      mpDpSubject.Detach(pSubject);
    }
		
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * observer pattern method
    *****************************************************************************/
    void PhoneNumber::Update(Subject* pSubject)
    {
      mpDpSubject.Update(pSubject);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * observer pattern method
    *****************************************************************************/
    void PhoneNumber::SetSubjectPointer(int Id,Subject* pSubject)
    {
      mpDpSubject.UnsubscribeAndDetach(this);
      mpDpSubject.Attach(pSubject); 
      mpDpSubject.SetUpdated();
    }
		
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * observer pattern method
    *****************************************************************************/
    void PhoneNumber::ConnectToSubjects(void)
    {
      if (mpDpSubject.IsValid())
      {
        mpDpSubject->Subscribe(this);
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Updates the captured string with string from StringDataPoint. 
    * The string is padded with spaces to achieve its full length.
    *****************************************************************************/
    void PhoneNumber::PadAndCaptureSubject(bool isInEditMode)
    {
      mCurrentField = INDEX_OF_FIRST_EDITABLE_DIGIT;

      if (mpDpSubject.IsValid())
      {
        char padded_str[PHONENUMBER_MAX_LEN + 1];
        
        strncpy(padded_str, mpDpSubject->GetValue(), PHONENUMBER_MAX_LEN);
        padded_str[PHONENUMBER_MAX_LEN] = '\0';
        
        int pos = strlen(padded_str);

        if (isInEditMode || pos > 1)
        {
          if (pos==0)
          {
            pos = 1;
          }
          padded_str[0] = sPhoneChars[PHONENUMBER_INDEX_OF_PLUS_PREFIX];
        }
        else
        {
          padded_str[0] = sPhoneChars[0];
        }


        for (; pos<PHONENUMBER_MAX_LEN; pos++)
        {
          padded_str[pos] = sPhoneChars[0];
        }

        padded_str[PHONENUMBER_MAX_LEN] = '\0'; 

        mpDpCapturedString->SetValue( padded_str );

        for (int i=0; i<PHONENUMBER_MAX_LEN; i++)
        {
          mpPhoneChars[i].Update( mpDpCapturedString );
          mpPhoneChars[i].UpdateCapturedValue(isInEditMode);
        }
      }
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Removes all chars except '0'-'9' and '+' and saves it in the StringDatapoint.
    * Updates the captured string with a space-padded version.
    *****************************************************************************/
    void PhoneNumber::StripCommitAndRecapture()
    {
      // pos 0 is reserved for international prefix ('+')
      int pos = 1; 
      char stripped_str[PHONENUMBER_MAX_LEN + 1];
      const char* subject_str = mpDpCapturedString->GetValue();
      
      for (int i=1; i<PHONENUMBER_MAX_LEN; i++)
      {
        if (subject_str[i] >= '0' && subject_str[i] <= '9')
        {
          stripped_str[pos] = subject_str[i];
          pos++;
        }

        mpPhoneChars[i].Invalidate();
      }
      stripped_str[pos] = '\0';

      // start the number with international prefix ('+') if a number was entered
      stripped_str[0] = (strlen(stripped_str+1) > 0 ? '+' : '\0');

      // store the stripped string
      mpDpSubject->SetValue( stripped_str );

      // recapture the stripped string
      PadAndCaptureSubject(false);
    }

  }
}

