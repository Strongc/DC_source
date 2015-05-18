/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/* CLASS NAME       : MultiNumber                                           */
/*                                                                          */
/* FILE NAME        : MultiNumber.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a MultiNumber.                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <math.h>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "MultiNumber.h"
#include <Frame.h>
#include <Number.h>
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
    MultiNumber::MultiNumber(Component* pParent) : Group(pParent)
    {
      mpDps = NULL;
      mpFrames = NULL;
      mpNumbers = NULL;
      mFieldCount = 0;
      mCurrentField = 0;
      mFieldMinValue = 0;
      mFieldMaxValue = 9;
      mMaxMinChanged = true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    MultiNumber::~MultiNumber()
    {
      CleanUp();
      mpDpSubject.UnsubscribeAndDetach(this);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiNumber::CleanUp()
    {
      if (mpFrames != NULL)
      {
        for (int i=0; i<mFieldCount; i++)
        {
          RemoveChild(&mpFrames[i]);
        }
        delete[] mpFrames;
      }
      if (mpDps != NULL)
      {
        delete[] mpDps;
      }
      if (mpNumbers != NULL)
      {
        delete[] mpNumbers;
      }
      mCurrentField = 0;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void  MultiNumber::SetFieldCount(const int fields, int defaultValue /* = 0 */)
    {
      CleanUp();

      mFieldCount = fields;
      mpDps = new U32DataPoint[mFieldCount];
      mpFrames = new Frame[mFieldCount];
      mpNumbers = new Number[mFieldCount];

      for (int i=0; i<mFieldCount; i++)
      {
        mpDps[i].SetMaxValue(0x7fffffff);
        mpDps[i].SetValue(defaultValue);
        mpDps[i].SetQuality(DP_AVAILABLE);
        mpDps[i].SetQuantity(Q_NO_UNIT);

        mpFrames[i].SetVisible();
        mpFrames[i].SetReadOnly(false);
        mpFrames[i].SetFrame(false);
        AddChild(&mpFrames[i]);

        mpNumbers[i].SetNumberOfDigits(1);
        mpNumbers[i].SetSubjectPointer(0,&mpDps[i]);
        mpNumbers[i].ConnectToSubjects();

        mpNumbers[i].SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
        mpNumbers[i].SetLeftMargin(1);
        mpNumbers[i].SetRightMargin(1);
        mpNumbers[i].SetReadOnly(IsReadOnly());
        mpNumbers[i].SetWordWrap(false);
        mpNumbers[i].SetVisible();
        mpNumbers[i].SetId(i+30000);
        mpFrames[i].AddChild(&mpNumbers[i]);
        mpFrames[i].SetCurrentChild(&mpNumbers[i]);
      }
      mpNumbers[0].SetFocus();
      SetCurrentChild(&mpFrames[0]);
      mpFrames[0].SetCurrentChild(&mpNumbers[0]);
      Invalidate();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiNumber::SetFieldMaxValue(const int maxValue)
    {
      mMaxMinChanged  = true;
      mFieldMaxValue = maxValue;
      Invalidate();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiNumber::SetFieldMinValue(const int minValue)
    {
      mMaxMinChanged  = true;
      mFieldMinValue = minValue;
      Invalidate();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool MultiNumber::SetReadOnly(bool readOnly /*= true*/)
    {
      Group::SetReadOnly(readOnly);
      for (int i=0; i<mFieldCount;  i++)
      {
        mpNumbers[i].SetReadOnly(readOnly);
        mpFrames[i].SetReadOnly(readOnly);
      }
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiNumber::Run()
    {
      if (mMaxMinChanged)
      {
        mMaxMinChanged = false;
        for (int i=0; i<mFieldCount; i++)
        {
          mpDps[i].SetMaxValue(mFieldMaxValue);
          mpDps[i].SetMinValue(mFieldMinValue);
        }
      }
			
      if (mpDpSubject.IsUpdated() && !IsInEditMode())
      {
        UpdateDpsFromSubject();
        Invalidate();
      }
      
      Group::Run();
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool MultiNumber::Redraw()
    {
     const int frame_width = (GetWidth() - mFieldCount + 1) / mFieldCount;
     const int height = GetHeight();
     int  x_pos = 2;
     for (int i=0; i<mFieldCount; i++)
     {
       mpFrames[i].SetClientArea(x_pos, 0, x_pos+frame_width-1,height-1);
       mpNumbers[i].SetClientArea(1,1,frame_width-3,height-3);
       x_pos += (frame_width + 1);

       mpFrames[i].SetFrame(i == mCurrentField && HasFocus() && !IsReadOnly() && IsInEditMode());
     }
     return Group::Redraw();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool MultiNumber::HandleKeyEvent(Keys KeyID)
    {
      if (!IsReadOnly())
      {
        switch(KeyID)
        {
          case MPC_UP_KEY:
            if (IsInEditMode())
            {
              mpNumbers[mCurrentField].EndEdit();
              ++mCurrentField;

              if (mCurrentField >= mFieldCount)
              {
                mCurrentField = mFieldCount - 1;
              }

              mpNumbers[mCurrentField].BeginEdit();
              SetCurrentChild(&mpFrames[mCurrentField]);
              return true;
            }

          case MPC_DOWN_KEY:
            if (IsInEditMode())
            {
              mpNumbers[mCurrentField].EndEdit();
              --mCurrentField;
              if (mCurrentField < 0)
              {
                mCurrentField = 0;
              }
              mpNumbers[mCurrentField].BeginEdit();
              SetCurrentChild(&mpFrames[mCurrentField]);
              return true;
            }

          case MPC_OK_KEY:
            if (IsInEditMode())
            {
              mpNumbers[mCurrentField].EndEdit();

              mCurrentField = 0;
              if (mpDpSubject.IsValid())
              {
                int the_value = 0;
                int number_sys = mpDps[0].GetMaxValue() - mpDps[0].GetMinValue() + 1;
                for (int i = 0; i < mFieldCount; i++)
                {
                  the_value += mpDps[i].GetValue() * ::pow((double)number_sys, mFieldCount - i - 1);
                }
                mpDpSubject->SetAsInt(the_value);
              }
            }
            else
            {
              if (BeginEdit())
              {
                mpNumbers[mCurrentField].BeginEdit();
              }
            }
            SetCurrentChild(&mpFrames[mCurrentField]);

            return true;

          case MPC_ESC_KEY:
            mpNumbers[mCurrentField].EndEdit();

            mCurrentField = 0;
            
            UpdateDpsFromSubject();
            CancelEdit();

            SetCurrentChild(&mpFrames[mCurrentField]);
            
            return true;
        }
      }
      return Group::HandleKeyEvent(KeyID);
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    Leds  MultiNumber::GetLedsStatus()
    {
      if ( !IsReadOnly() )
      {
        Leds leds = Group::GetLedsStatus();
        SetAllowFlags(leds, UP_LED | DOWN_LED | OK_LED | ESC_LED );
        
        if ( IsInEditMode() )
        {
          
          if (mCurrentField == 0)
          {
            SetDenyFlags(leds, DOWN_LED);
          }
          else
          {
            RemoveDenyFlags(leds, DOWN_LED);
          }

          if (mCurrentField == (mFieldCount - 1))
          {
            SetDenyFlags(leds, UP_LED);
          }
          else
          {
            RemoveDenyFlags(leds, UP_LED);
          }
        }
        else
        {
          RemoveDenyFlags(leds, UP_LED);
          RemoveDenyFlags(leds, DOWN_LED);

          SetAllowFlags(leds, PLUS_LED | MINUS_LED );
        }

        return leds;
        
      }
      else
      {
        return COMBINE(NO_LED, NO_LED);
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    Keys MultiNumber::GetLegalKeys()
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
      return mLegalKeys;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiNumber::SubscribtionCancelled(Subject* pSubject)
    {
      mpDpSubject.Detach(pSubject);
    }
		
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiNumber::Update(Subject* pSubject)
    {
      mpDpSubject.Update(pSubject);
    }
		
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiNumber::SetSubjectPointer(int Id,Subject* pSubject)
    {
      mpDpSubject.UnsubscribeAndDetach(this);
      mpDpSubject.Attach(pSubject); 
      mpDpSubject.SetUpdated();
    }
		
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiNumber::ConnectToSubjects(void)
    {
      if (mpDpSubject.IsValid())
      {
        mpDpSubject->Subscribe(this);
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void MultiNumber::UpdateDpsFromSubject()
    {
      if (mpDpSubject.IsValid())
      {
        unsigned int value = (unsigned int)mpDpSubject->GetAsInt();
        int the_value = 0;
        int number_sys = mpDps[0].GetMaxValue() - mpDps[0].GetMinValue() + 1;
        for (int i = 0; i < mFieldCount; i++)
        {
          the_value = (int)(value / pow((double)number_sys, mFieldCount - i - 1));
          if (the_value < 0)
          {
            the_value += number_sys - 1;
          }
          value -= (int)(the_value * pow((double)number_sys, mFieldCount - i - 1));
          mpDps[i].SetValue(the_value);
        }
      }
    }

  }
}

