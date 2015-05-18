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
/* CLASS NAME       : Number                                                */
/*                                                                          */
/* FILE NAME        : Number.cpp                                            */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <stdlib.h>
#include <math.h>
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "display_task.h" // to get DISPLAY_SAMPLE_TIME

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Number.h"
#include "NumberQuantity.h"
#include "NumericalDataPointInterface.h"

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
    Number::Number(Component* pParent /*= NULL*/) : ObserverText(pParent)
    {
      mUpdateDelay = NUMBER_SAMPLE_TIME;
      mUpdateTimer = 0;
      mNumberOfDigits = 3;
      mToggle = false;
      mState = START;
      mChangeTimer = 0;
      mNumericalDpIf = NULL;
      mUpdate = false;
      mDecadeAccelerationLevel = 0;
      mTimesumChangeValue = 0;

      mLegalKeys = MPC_NO_KEY;
      MpcUnits::GetInstance()->Subscribe(this);
    }

    /*****************************************************************************
    * Destructor
    * DESCRIPTION:
    *****************************************************************************/
    Number::~Number()
    {
      MpcUnits::GetInstance()->Unsubscribe(this);
    }

    /*****************************************************************************
    * Function - SetUpdateDelay
    * DESCRIPTION: Returns previous delay value
    *****************************************************************************/
    U16 Number::SetUpdateDelay(U16 newUpdateDelay)
    {
      U16 oldDelay = mUpdateDelay;
      mUpdateDelay = newUpdateDelay;
      return oldDelay;
    }

    /*****************************************************************************
    * Function - SubscribtionCancelled
    * DESCRIPTION:
    *****************************************************************************/
    void Number::SubscribtionCancelled(Subject* pSubject)
    {
      ObserverText::SubscribtionCancelled(pSubject);
      if (mNumericalDpIf)
      {
        delete mNumericalDpIf;
        mNumericalDpIf = NULL;
      }
      Invalidate();
    }

    /*****************************************************************************
    * Function - ConnectToSubjects
    * DESCRIPTION:
    *****************************************************************************/
    void Number::ConnectToSubjects()
    {
      ObserverText::ConnectToSubjects();
      if (mNumericalDpIf)
      {
        delete mNumericalDpIf;
        mNumericalDpIf = NULL;
      }

      mNumericalDpIf = new NumericalDataPointInterface(GetSubject());

      mUpdate = true;
    }

    /*****************************************************************************
    * Function - Update
    * DESCRIPTION:
    *****************************************************************************/
    void Number::Update(Subject* pSubject)
    {
      mUpdate = true;

      NumberQuantity* parent_NQ = dynamic_cast<NumberQuantity*>(mpParent);

      if (parent_NQ)
      { 
        parent_NQ->Update(pSubject);
      }        

      IDataPoint* p  = dynamic_cast<IDataPoint*>(pSubject);
      if (p)
      {
        mDPWritable = p->GetWritable();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool Number::Redraw()
    {
      char the_string_to_show[MAX_NUMBER_STR_LEN];
      the_string_to_show[0] = '\0';

      if (IsVisible())
      {
        if (IsInEditMode())
        {
          ObserverText::SetColour(mToggle 
            ? GUI_COLOUR_TEXT_EDITMODE_FOREGROUND 
            : GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
          
          if (mNumericalDpIf != NULL)
          {
            mNumericalDpIf->GetCapturedDataPointAsString(the_string_to_show, mNumberOfDigits);
          }

          ObserverText::SetText(the_string_to_show);

          return ObserverText::Redraw();
        }
        else
        {
          return ObserverText::Redraw();
        }
      }
      return false;
    }

    /*****************************************************************************
    * Function - Run
    * DESCRIPTION:
    *****************************************************************************/
    void Number::Run()
    {
      if(!IsVisible())
        return;

      if (mUpdate && mNumericalDpIf != NULL && IsInEditMode() == false)
      {
        mUpdateTimer += DISPLAY_SAMPLE_TIME;
        if(mUpdateTimer >= mUpdateDelay)
        {
          mUpdateTimer -= mUpdateDelay;
          mUpdate = false;
          char the_string_to_show[MAX_NUMBER_STR_LEN];
          mNumericalDpIf->GetDataPointAsString(the_string_to_show,mNumberOfDigits);
          ObserverText::SetText(the_string_to_show); // Invalidates on change.
        }
      }


      switch (mState)
      {
        case START:
          break;
        case CHANGE: 
          mChangeTimer++;
			    if (mChangeTimer > FLASH_VALUE)
			    {
			      mChangeTimer = 0;
			      mToggle = !mToggle;
			      Invalidate();
			    }
			    break;
        default: 
          mState = START;
      }

      ObserverText::Run();
    }

    /*****************************************************************************
    * Function - SetNumberOfDigits
    * DESCRIPTION:
    *****************************************************************************/
    void Number::SetNumberOfDigits(int numberOfDigits)
    {
      if (mNumberOfDigits != numberOfDigits)
      {
        mNumberOfDigits = numberOfDigits;
        Invalidate();
      }
    }


    /*****************************************************************************
    * Function - GetNumberOfDigits
    * DESCRIPTION:
    *****************************************************************************/
    int Number::GetNumberOfDigits()
    {
      return mNumberOfDigits;
    }

    /*****************************************************************************
    * Function - HandleKeyEvent
    * DESCRIPTION:
    *****************************************************************************/
    bool Number::HandleKeyEvent(Keys KeyID)
    {
      if (IsReadOnly())
      {
        return ObserverText::HandleKeyEvent(KeyID);
      }

      if (IsInEditMode() && (mNumericalDpIf != NULL))
      {
        if (KeyID != MPC_PLUS_KEY_REP && KeyID != MPC_MINUS_KEY_REP)
        {
          mTimesumChangeValue = 0;
          mDecadeAccelerationLevel = 0;
          mNumericalDpIf->ResetAccelerationCounter();
        }

        if ( !mNumericalDpIf->IsCaptured()
          && (KeyID == MPC_PLUS_KEY 
          || KeyID == MPC_PLUS_KEY_REP
          || KeyID == MPC_MINUS_KEY
          || KeyID == MPC_MINUS_KEY_REP))
        {
          mNumericalDpIf->CaptureDataPoint();
        }

        if ( mNumericalDpIf->ChangeAccelerationOnDecades())
        {
          switch (KeyID)
          {
          case MPC_PLUS_KEY :
            {
              mNumericalDpIf->ChangeCapturedDataPoint(1, mNumberOfDigits);
              break;
            }
          case MPC_PLUS_KEY_REP :
            {
              int changeValue = ::pow((double)10, (mDecadeAccelerationLevel + 1));
              if ( mNumericalDpIf->IsReadyForAcceleration(changeValue, mNumberOfDigits) )
              {
                mDecadeAccelerationLevel++;
              }
              changeValue = ::pow((double)10, mDecadeAccelerationLevel);
              mNumericalDpIf->ChangeCapturedDataPoint(changeValue, mNumberOfDigits);

              break;
            }
          case MPC_MINUS_KEY :
            {
              mNumericalDpIf->ChangeCapturedDataPoint(-1, mNumberOfDigits);
              break;
            }
          case MPC_MINUS_KEY_REP :
            {
              int changeValue = - ::pow((double)10, (mDecadeAccelerationLevel + 1));

              if ( mNumericalDpIf->IsReadyForAcceleration(changeValue, mNumberOfDigits) )
              {
                mDecadeAccelerationLevel++;
              }

              changeValue = - ::pow((double)10, mDecadeAccelerationLevel);
              mNumericalDpIf->ChangeCapturedDataPoint(changeValue, mNumberOfDigits);

              break;
            }
          default :
            return ObserverText::HandleKeyEvent(KeyID);
          }
        }
        else
        {
          // use time sum change value
          switch (KeyID)
          {
          case MPC_PLUS_KEY :
          case MPC_PLUS_KEY_REP :
            {
              mTimesumChangeValue = mNumericalDpIf->GetCurrentTimesumChange(mTimesumChangeValue, mNumberOfDigits);
              mNumericalDpIf->ChangeCapturedDataPoint(mTimesumChangeValue,mNumberOfDigits);
              break;
            }
          case MPC_MINUS_KEY :
          case MPC_MINUS_KEY_REP :
            {
              mTimesumChangeValue = mNumericalDpIf->GetCurrentTimesumChange(mTimesumChangeValue, mNumberOfDigits);
              mNumericalDpIf->ChangeCapturedDataPoint(-mTimesumChangeValue, mNumberOfDigits);
              break;
            }
          default :
            return ObserverText::HandleKeyEvent(KeyID);
          }
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
    * Function - GetLegalKeys
    * DESCRIPTION:
    *****************************************************************************/
    Keys Number::GetLegalKeys()
    {
      if ( !IsReadOnly() )
      {
        SetAllowFlags(mLegalKeys, MPC_OK_KEY|MPC_ESC_KEY);
        if ( IsInEditMode() )
        {
          SetDenyFlags(mLegalKeys,MPC_UP_KEY |MPC_DOWN_KEY|MPC_HOME_KEY|MPC_MENU_KEY);

          if (mNumericalDpIf != NULL)
          {
            if ( mNumericalDpIf->IsCapturedDataPointMax() )
            {
              RemoveAllowFlags(mLegalKeys, MPC_PLUS_KEY | MPC_PLUS_KEY_REP);
            }
            else
            {
              SetAllowFlags(mLegalKeys, MPC_PLUS_KEY | MPC_PLUS_KEY_REP);
            }

            if ( mNumericalDpIf->IsCapturedDataPointMin() )
            {
              RemoveAllowFlags(mLegalKeys, MPC_MINUS_KEY | MPC_MINUS_KEY_REP);
            }
            else
            {
              SetAllowFlags(mLegalKeys, MPC_MINUS_KEY | MPC_MINUS_KEY_REP);
            }
          }

        }
        else
        {
          // Remove all flags that might have been set when we were in edit mode.
          RemoveDenyFlags(mLegalKeys, MPC_UP_KEY |MPC_DOWN_KEY|MPC_HOME_KEY|MPC_MENU_KEY);

          if (mNumericalDpIf != NULL)
          {
            if ( mNumericalDpIf->IsDataPointMax() )
            {
              RemoveAllowFlags(mLegalKeys, MPC_PLUS_KEY | MPC_PLUS_KEY_REP);
            }
            else
            {
              SetAllowFlags(mLegalKeys, MPC_PLUS_KEY | MPC_PLUS_KEY_REP);
            }

            if ( mNumericalDpIf->IsDataPointMin() )
            {
              RemoveAllowFlags(mLegalKeys, MPC_MINUS_KEY | MPC_MINUS_KEY_REP);
            }
            else
            {
              SetAllowFlags(mLegalKeys, MPC_MINUS_KEY | MPC_MINUS_KEY_REP);
            }
          }
        }
      }
      return ObserverText::GetLegalKeys();
    }

    /*****************************************************************************
    * Function - GetLedsStatus
    * DESCRIPTION:
    *****************************************************************************/
    Leds  Number::GetLedsStatus()
    {
      Leds temp_leds = NO_LED;

      if ( !IsReadOnly() )
      {
        if ( IsInEditMode() )
        {
          if (mNumericalDpIf != NULL)
          {
            if ( !mNumericalDpIf->IsCapturedDataPointMax() )
            {
              temp_leds |= PLUS_LED;
            }
            if ( !mNumericalDpIf->IsCapturedDataPointMin() )
            {
              temp_leds |= MINUS_LED;
            }
          }

          return COMBINE(UP_LED + DOWN_LED + MENU_LED + HOME_LED,temp_leds + ESC_LED + OK_LED);
        }
        else
        {
          if (mNumericalDpIf != NULL)
          {
            if ( !mNumericalDpIf->IsDataPointMax() )
            {
              temp_leds |= PLUS_LED;
            }
            if ( !mNumericalDpIf->IsDataPointMin() )
            {
              temp_leds |= MINUS_LED;
            }
          }

          return COMBINE(NO_LED,temp_leds + OK_LED);
        }
      }
      else
      {
        return COMBINE(NO_LED,NO_LED);
      }
    }


    /*****************************************************************************
    * Function - BeginEdit
    * DESCRIPTION:
    *****************************************************************************/
    bool Number::BeginEdit()
    {
      if (!ObserverText::BeginEdit())
      {
        return false;
      }

      Subject* pSubject = mNumericalDpIf->CaptureDataPoint();
      mState = CHANGE;
      mChangeTimer = 0;
      SetDenyFlags( mLegalKeys, MPC_HOME_KEY|MPC_UP_KEY|MPC_DOWN_KEY|MPC_MENU_KEY);

      Invalidate();

      return true;
    }

    /*****************************************************************************
    * Function - CancelEdit
    * DESCRIPTION: Cancels the edit mode started by a Call to BeginEdit.
    * The value being edited should return to the original.
    *****************************************************************************/
    bool Number::CancelEdit()
    {
      if( !ObserverText::CancelEdit() )
        return false;

      mNumericalDpIf->ReleaseDataPoint();
      mToggle = true;
      mState = START;
      mUpdate = true;
      mChangeTimer = 0;
      ObserverText::SetColour(GUI_COLOUR_TEXT_DEFAULT_FOREGROUND);

      Invalidate();
      RemoveDenyFlags( mLegalKeys, MPC_HOME_KEY|MPC_UP_KEY|MPC_DOWN_KEY|MPC_MENU_KEY);
      return true;
    }

    /*****************************************************************************
    * Function - EndEdit
    * DESCRIPTION: Ends edit mode and stores the new value.
    *****************************************************************************/
    bool Number::EndEdit()
    {
      if( !ObserverText::EndEdit() )
        return false;

      mNumericalDpIf->SetCapturedDataPoint();
      mNumericalDpIf->ReleaseDataPoint();
      mState = START;
      mToggle = true;
      mState = START;
      mChangeTimer = 0;
      ObserverText::SetColour(GUI_COLOUR_TEXT_DEFAULT_FOREGROUND);
      Invalidate();
      RemoveDenyFlags(mLegalKeys, MPC_HOME_KEY|MPC_UP_KEY|MPC_DOWN_KEY|MPC_MENU_KEY);
      return true;
    }

    /*****************************************************************************
    * Function - IsNeverAvailable
    * DESCRIPTION:
    *****************************************************************************/
    bool Number::IsNeverAvailable()
    {
      if (mNumericalDpIf != NULL)
      {
        return mNumericalDpIf->IsNeverAvailable();
      }
      return false;
    }


    /*****************************************************************************
    * Function - IsCaptured
    * DESCRIPTION:
    *****************************************************************************/
    bool Number::IsCaptured(void)
    {
      if (mNumericalDpIf)
      {
        return mNumericalDpIf->IsCaptured();
      }
      return false;
    }

    /*****************************************************************************
    *
    *
    *              PRIVATE FUNCTIONS
    *
    *
    ****************************************************************************/

    /*****************************************************************************
    *
    *
    *              PROTECTED FUNCTIONS
    *                 - RARE USED -
    *
    ****************************************************************************/
  } // namespace display
} // namespace mpc
