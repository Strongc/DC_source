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
/* CLASS NAME       : MultiString                                           */
/*                                                                          */
/* FILE NAME        : MultiString.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 2008-02-11                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for editing a string                           */
/*                                                                          */
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
#include "MultiString.h"
#include <MpcFonts.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
#define ARROW_WIDTH             7
#define FRAME_WIDTH             8
#define SELECTED_FRAME_WIDTH   16

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
EXTERNS
*****************************************************************************/
                                                         // width,height
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmArrowLeft;    // 5, 9
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmArrowRight;   // 5, 9


namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    MultiString::MultiString(Component* pParent) : Group(pParent)
    {
      mpFrames = NULL;
      mpMultiStringChars = NULL;
      mCurrentField = 0;

      mpImgMoreCharsToTheLeft = new Image(this);
      mpImgMoreCharsToTheLeft->SetBitmap(&bmArrowLeft);
      mpImgMoreCharsToTheLeft->SetVisible(false);

      mpImgMoreCharsToTheRight = new Image(this);
      mpImgMoreCharsToTheRight->SetBitmap(&bmArrowRight);
      mpImgMoreCharsToTheRight->SetVisible(false);

      mpNonEditableText = new DataPointText();
      mpNonEditableText->SetAlign(GUI_TA_LEFT|GUI_TA_BOTTOM);      
      mpNonEditableText->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpNonEditableText->SetVisible(false);
      
      AddChild(mpImgMoreCharsToTheLeft);
      AddChild(mpImgMoreCharsToTheRight);
      AddChild(mpNonEditableText);
      
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    MultiString::~MultiString()
    {
      if (mpFrames != NULL)
      {
        for (int i=0; i<mNoOfChars; i++)
        {
          RemoveChild(&mpFrames[i]);
        }
        delete[] mpFrames;
      }
      if (mpMultiStringChars != NULL)
      {
        for (int i=0; i<mNoOfChars; i++)
        {
          RemoveChild(&mpMultiStringChars[i]);
        }
        delete[] mpMultiStringChars;
      }

      if (mpImgMoreCharsToTheLeft != NULL) 
      {
        RemoveChild(mpImgMoreCharsToTheLeft);
        delete mpImgMoreCharsToTheLeft;
      }

      if (mpImgMoreCharsToTheRight != NULL) 
      {
        RemoveChild(mpImgMoreCharsToTheRight);
        delete mpImgMoreCharsToTheRight;
      }

      if (mpNonEditableText != NULL) 
      {
        RemoveChild(mpNonEditableText);
        delete mpNonEditableText;
      }

      mCurrentField = 0;

      mpDpSubject.UnsubscribeAndDetach(this);
    }

    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    bool MultiString::SetReadOnly(bool readOnly /*= true*/)
    {
      Group::SetReadOnly(readOnly);

      if (mpMultiStringChars!=NULL)
      {
        int i;
        for (i=0; i<mNoOfChars; i++)
        {
          mpMultiStringChars[i].SetReadOnly(readOnly);
          mpFrames[i].SetReadOnly(readOnly);
        }
      }
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void MultiString::SetClientArea(int x1, int y1, int x2, int y2)
    {
      Group::SetClientArea(x1, y1, x2, y2);

      mpNonEditableText->SetClientArea(0, 0, x2 - x1, y2 - y1);
    }

    /*****************************************************************************
    * Function - SetColour
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void MultiString::SetColour(U32 colour, bool forced)
    {
      Group::SetColour(colour, forced);
      mpNonEditableText->SetColour(colour, forced);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void MultiString::Run()
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
    bool MultiString::IsValid()
    {
      return Component::IsValid();
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. A frame is drawn for the currently selected field
    * String is appended with '<', '>' and/or '...' when editing a long string
    *****************************************************************************/
    bool MultiString::Redraw()
    {
      if (IsInEditMode())
      {
        const int height = GetHeight();
        const int width = GetWidth();

        bool more_to_the_left = false;
        bool more_to_the_right_reached = false;

        int x_pos = 1;
        int frame_width = FRAME_WIDTH;
        
        int first_visible_char = 0;

        int max_no_of_visible_fields = ((width - 2 - SELECTED_FRAME_WIDTH)/ frame_width) ;

        mpNonEditableText->SetVisible(false);

        for (int i=0; i<mNoOfChars; i++)
        {
          (&mpMultiStringChars[i])->SetVisible(true);
        }
      
        if (mCurrentField >= max_no_of_visible_fields)
        {
          more_to_the_left = true;
          first_visible_char = mCurrentField - max_no_of_visible_fields + 1;

          for (int i = 0; i < first_visible_char; i++)
          {
            mpFrames[i].SetVisible(false);
          }
        }
        else
        {
          first_visible_char = 0;
        }

        if (more_to_the_left)
        {
          mpImgMoreCharsToTheLeft->SetClientArea(x_pos, 1, x_pos + bmArrowLeft.XSize-1, height-1);
          x_pos += (bmArrowLeft.XSize + 1);
        }

        for (int i = first_visible_char; i < mNoOfChars; i++)
        {
          bool is_selected_field = (i == mCurrentField && HasFocus() && !mReadOnly);

          frame_width = (is_selected_field ? SELECTED_FRAME_WIDTH : FRAME_WIDTH);

          mpFrames[i].SetClientArea(x_pos, 0, x_pos + frame_width-1, height-1);

          if (is_selected_field)
          {
            mpMultiStringChars[i].SetClientArea(2, 1, frame_width-2, height-2);
          }
          else
          {
            mpMultiStringChars[i].SetClientArea(0, 1, frame_width-1, height-2);
          }

          mpFrames[i].SetVisible(!more_to_the_right_reached);

          x_pos += frame_width;

          mpFrames[i].SetFrame(is_selected_field);

          if (!more_to_the_right_reached && x_pos > (width - ARROW_WIDTH - ARROW_WIDTH))
          {
            //hide all chars to the right
            more_to_the_right_reached = true;

            mpImgMoreCharsToTheRight->SetClientArea(x_pos, 1, x_pos + ARROW_WIDTH - 1, height-2);
            x_pos += ARROW_WIDTH;
                    
          } 
        }

        bool is_at_last_field = (mCurrentField == mNoOfChars-1);

        mpImgMoreCharsToTheLeft->SetVisible(more_to_the_left);     
        mpImgMoreCharsToTheRight->SetVisible(more_to_the_right_reached && !is_at_last_field);
        
        
      }
      else //not in edit mode
      {
        mpNonEditableText->SetVisible(true);

        for (int i=0; i<mNoOfChars; i++)
        {
          (&mpFrames[i])->SetVisible(false);
          (&mpMultiStringChars[i])->SetVisible(false);
        }
        mpImgMoreCharsToTheLeft->SetVisible(false);
        mpImgMoreCharsToTheRight->SetVisible(false);
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
    bool MultiString::HandleKeyEvent(Keys KeyID)
    {
      if (mReadOnly)
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

            mpMultiStringChars[mCurrentField].BeginEdit();
            mpFrames[mCurrentField].SetFrame(true);
            SetCurrentChild(&mpFrames[mCurrentField]);
          }
          Invalidate();
          return true;

        case MPC_UP_KEY:
          if (IsInEditMode())
          {// select next field
            mpFrames[mCurrentField].SetFrame(false);
            mpMultiStringChars[mCurrentField].EndEdit();

            if (mCurrentField < mNoOfChars-1)
            {
              mCurrentField++;
            }
            else
            {
              mCurrentField = 0;
            }

            mpMultiStringChars[mCurrentField].BeginEdit();
            mpFrames[mCurrentField].SetFrame(true);
            SetCurrentChild(&mpFrames[mCurrentField]);
          }
          Invalidate();
          return true;

        case MPC_DOWN_KEY:
          if (IsInEditMode())
          {// select previous field
            mpFrames[mCurrentField].SetFrame(false);
            mpMultiStringChars[mCurrentField].EndEdit();

            if (mCurrentField > 0)
            {
              mCurrentField--;
            }
            else
            {
              mCurrentField = mNoOfChars - 1;
            }

            mpMultiStringChars[mCurrentField].BeginEdit();
            mpFrames[mCurrentField].SetFrame(true);
            SetCurrentChild(&mpFrames[mCurrentField]);
          }
          Invalidate();
          return true;

        case MPC_OK_KEY:
          if (IsInEditMode())
          {// end edit mode
            mpFrames[mCurrentField].SetFrame(false);
            mpMultiStringChars[mCurrentField].EndEdit();
            mCurrentField = 0;
            if (mpDpSubject.IsValid())
            {
              StripCommitAndRecapture();
            }
          }
          else
          {// enter edit mode
            PadAndCaptureSubject(true);

            mpMultiStringChars[mCurrentField].BeginEdit();
            mpFrames[mCurrentField].SetFrame(true);
            SetCurrentChild(&mpFrames[mCurrentField]);
          }
          Invalidate();
          return true;

        case MPC_ESC_KEY:
          if (IsInEditMode())
          {// cancel edit mode
            mpFrames[mCurrentField].SetFrame(false);
            mpMultiStringChars[mCurrentField].EndEdit();

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
    Leds  MultiString::GetLedsStatus()
    {
      if (!mReadOnly)
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
    void MultiString::SubscribtionCancelled(Subject* pSubject)
    {
      mpDpSubject.Detach(pSubject);
    }
		
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * observer pattern method
    *****************************************************************************/
    void MultiString::Update(Subject* pSubject)
    {
      mpDpSubject.Update(pSubject);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * observer pattern method
    *****************************************************************************/
    void MultiString::SetSubjectPointer(int Id,Subject* pSubject)
    {
      mpDpSubject.UnsubscribeAndDetach(this);
      mpDpSubject.Attach(pSubject); 
      mpDpSubject.SetUpdated();

      mpNonEditableText->SetSubjectPointer(0, pSubject);

      StringDataPoint* strDp = dynamic_cast<StringDataPoint*>(pSubject);

      if (strDp != NULL)
      {
        Init((U8)strDp->GetMaxNoOfChars());
      }
      else
      {
        FatalErrorOccured("Subject of MultiString is not a StringDataPoint!");
      }
    }
		
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * observer pattern method
    *****************************************************************************/
    void MultiString::ConnectToSubjects(void)
    {
      if (mpDpSubject.IsValid())
      {
        mpDpSubject->Subscribe(this);
        mpNonEditableText->ConnectToSubjects();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * create frames and char components
    *****************************************************************************/
    void MultiString::Init(U8 noOfChars)
    {
      mNoOfChars = noOfChars;

      mpDpCapturedString = new StringDataPoint("", mNoOfChars);

      mpFrames = new Frame[mNoOfChars];
      mpMultiStringChars = new MultiStringChar[mNoOfChars];

      for (int i=0; i<mNoOfChars; i++)
      {
        mpFrames[i].SetVisible();
        mpFrames[i].SetReadOnly(false);
        mpFrames[i].SetFrame(false);
        AddChild(&mpFrames[i]);
        
        mpMultiStringChars[i].SetSubjectPointer(0, mpDpCapturedString);
        mpMultiStringChars[i].ConnectToSubjects();
        mpMultiStringChars[i].Init(i, mNoOfChars);
        mpMultiStringChars[i].SetReadOnly(mReadOnly);
        mpMultiStringChars[i].SetVisible();
        mpFrames[i].AddChild(&mpMultiStringChars[i]);
        mpFrames[i].SetCurrentChild(&mpMultiStringChars[i]);

        if (mReadOnly)
        {
          mpMultiStringChars[i].SetReadOnly(true);
          mpFrames[i].SetReadOnly(true);
        }
      }
      mpFrames[0].SetFrame(true);
      mpMultiStringChars[0].SetFocus();
      SetCurrentChild(&mpFrames[0]);
      Invalidate();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Updates the captured string with string from StringDataPoint. 
    * The string is padded with dots to achieve its full length.
    *****************************************************************************/
    void MultiString::PadAndCaptureSubject(bool isInEditMode)
    {
      // reset current field
      mCurrentField = 0;

      if (mpDpSubject.IsValid())
      {
        char padded_str[MULTISTRING_MAX_STRING_LEN + 1];
        
        strncpy(padded_str, mpDpSubject->GetValue(), MULTISTRING_MAX_STRING_LEN);
        padded_str[MULTISTRING_MAX_STRING_LEN] = '\0';
        
        int pos = strlen(padded_str);

        for (; pos<mNoOfChars; pos++)
        {
          padded_str[pos] = sMultiStringChars[0];
        }

        padded_str[mNoOfChars] = '\0'; 

        mpDpCapturedString->SetValue(padded_str);

        for (int i=0; i<mNoOfChars; i++)
        {
          mpMultiStringChars[i].Update(mpDpCapturedString);
          mpMultiStringChars[i].UpdateCapturedValue(isInEditMode);
        }

        Invalidate();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Updates the captured string.
    *****************************************************************************/
    void MultiString::StripCommitAndRecapture()
    {
      int pos = 0;
      char stripped_str[MULTISTRING_MAX_STRING_LEN];
      const char* subject_str = mpDpCapturedString->GetValue();

      for (int i=0; i<mNoOfChars; i++)
      {
        if (subject_str[i] != sMultiStringChars[ 0 ]
          && subject_str[i] != sMultiStringChars[ strlen(sMultiStringChars)-1 ]
          && subject_str[i] != '\0')
        {
          stripped_str[pos] = subject_str[i];
          pos++;
        }

        mpMultiStringChars[i].Invalidate();
      }

      //remove trailing spaces
      while(pos > 0 && stripped_str[pos-1] == ' ')
      {
        pos--;
      }

      stripped_str[pos] = 0;

      // store the stripped string
      mpDpSubject->SetValue(stripped_str);

      // recapture the stripped string
      PadAndCaptureSubject(false);
    }

  }
}

