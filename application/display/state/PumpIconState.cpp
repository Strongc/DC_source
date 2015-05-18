/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MidRange                                      */
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
/* CLASS NAME       : PumpIconState                                         */
/*                                                                          */
/* FILE NAME        : PumpIconState.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 2008-03-26                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "PumpIconState.h"


/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
EXTERNS
*****************************************************************************/
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmStatusManual;
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmStatusAlarm;
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmStatusWarning;

namespace mpc
{
  namespace display
  {

    #define ICON_ANIMATION_RATE 6 // refreshes every ICON_ANIMATION_RATE x 100 ms

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    PumpIconState::PumpIconState(Component* pParent) : Image(pParent)
    {
      SetWidth(bmStatusAlarm.XSize);
      SetHeight(bmStatusAlarm.YSize);

      mAnimationCounter = 0;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    PumpIconState::~PumpIconState()
    {
    }

    
    /*****************************************************************************
    * Function...: RunSubTask
    * DESCRIPTION:
    * Run animation or image based on alarm state and operation mode
    *****************************************************************************/
    void PumpIconState::Run()
    {
      const GUI_BITMAP* orginal_image = mpBitmap;

      if (mDpResetIconEvent.IsUpdated())
      {
        mAnimationCounter = ICON_ANIMATION_RATE;
        if (mDpOperationMode->GetValue() != REQ_OPERATION_MODE_AUTO)
        {
          mpBitmap = &bmStatusManual;
        }
      }

      if (IsVisible())
      {
        bool alarm_active = (mDpAlarmState->GetValue() == ALARM_STATE_ALARM);
        bool warning_active = (mDpAlarmState->GetValue() == ALARM_STATE_WARNING);
        bool manual_active = (mDpOperationMode->GetValue() != REQ_OPERATION_MODE_AUTO);

        if (manual_active && (alarm_active || warning_active))
        {
          // toggle between alarm/warning and manual

          mAnimationCounter++;

          if (mpBitmap == NULL)
          {
            mpBitmap = &bmStatusManual;
          }
          else if (mAnimationCounter >= ICON_ANIMATION_RATE)
          {
            //time to toggle image
            mAnimationCounter = 0;
            
            if (mpBitmap == &bmStatusAlarm || mpBitmap == &bmStatusWarning )
            {
              mpBitmap = &bmStatusManual;
            }
            else if (alarm_active)
            {
              mpBitmap = &bmStatusAlarm;
            }
            else // warning_active
            {
              mpBitmap = &bmStatusWarning;
            }
          }
          // else keep the current image
        }
        else if (alarm_active)
        {
          mpBitmap = &bmStatusAlarm;
        }
        else if (warning_active)
        {
          mpBitmap = &bmStatusWarning;
        }
        else if (manual_active)
        {
          mpBitmap = &bmStatusManual;
        }
        else
        {
          mpBitmap = NULL;
        } 

      }// if (IsVisible())

      if (orginal_image != mpBitmap)
        Invalidate();

      Image::Run();

    }


    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION:
    * 
    *****************************************************************************/
    bool PumpIconState::Redraw()
    {
      ClearArea();
      return Image::Redraw();
    }


    /*****************************************************************************
    * Function - ConnectToSubjects
    * DESCRIPTION: Subscribe to subjects.
    *
    *****************************************************************************/
    void PumpIconState::ConnectToSubjects()
    {
      mDpAlarmState.Subscribe(this);
      mDpOperationMode.Subscribe(this);
      mDpResetIconEvent.Subscribe(this);
    }

    /*****************************************************************************
    * Function - Update
    * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
    * If it is then put the pointer in queue and request task time for sub task.
    *
    *****************************************************************************/
    void PumpIconState::Update(Subject* pSubject)
    {
      if (!mDpResetIconEvent.Update(pSubject))
      {
        mDpResetIconEvent->SetEvent();
      }

      Invalidate();
    }

    /*****************************************************************************
    * Function - SubscribtionCancelled
    * DESCRIPTION:
    *
    *****************************************************************************/
    void PumpIconState::SubscribtionCancelled(Subject* pSubject)
    {
      mDpAlarmState.Unsubscribe(this);
      mDpOperationMode.Unsubscribe(this);
      mDpResetIconEvent.Unsubscribe(this);
    }

    /*****************************************************************************
    * Function - SetSubjectPointer
    * DESCRIPTION: If the id is equal to a id for a subject observed.
    * Then take a copy of pSubjet to the member pointer for this subject.
    *
    *****************************************************************************/
    void PumpIconState::SetSubjectPointer(int id, Subject* pSubject)
    {
      switch (id)
      {
        case SP_PIS_ALARM_STATE:
          mDpAlarmState.Attach(pSubject);
          break;
        case SP_PIS_OPERATION_MODE:
          mDpOperationMode.Attach(pSubject);
          break;
        case SP_PIS_RESET_ICON_EVENT:
          mDpResetIconEvent.Attach(pSubject);
          break;
        default:
          break;

      }
    }

    
  } // namespace display
} // namespace mpc


