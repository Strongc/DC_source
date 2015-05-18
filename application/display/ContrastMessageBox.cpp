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
/* CLASS NAME       : ContrastMessageBox                                    */
/*                                                                          */
/* FILE NAME        : ContrastMessageBox.cpp                                */
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
#include <GUI.h>
#include "ContrastMessageBox.h"
#include <DisplayController.h>
#include <U32DataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
#define CMB_SLOW_INCREMENTATION 20
#define CMB_FAST_INCREMENTATION (CMB_SLOW_INCREMENTATION * 8)

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmContrast_icon;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmContrast_scale;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmBrightness_icon;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmBrightness_scale;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmCursor;

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
    * Function - Constructor
    * DESCRIPTION:
    *****************************************************************************/
    ContrastMessageBox::ContrastMessageBox(Component* pParent) : PopupBox( pParent )
    {
      mSubjectSubscribed = false;
      
      mControlBrightness = false;
      #ifdef TFT_16_BIT_LCD
      mControlBrightness = true;
      #endif


      SetClientArea(2,90,237,190);

      mpIconImage = new Image(this);
      mpIconImage->SetClientArea(200,2,231,33);

      if (mControlBrightness)
      {
        mpIconImage->SetBitmap(&bmBrightness_icon);
      }
      else
      {
        mpIconImage->SetBitmap(&bmContrast_icon);
      }

      mpIconImage->Invalidate();
      mpIconImage->SetVisible();
      AddChild(mpIconImage);


      mpSliderImage = new Image(this);
      mpSliderImage->SetClientArea(20,40,211,71);
      if (mControlBrightness)
      {
        mpSliderImage->SetBitmap(&bmBrightness_scale);
      }
      else
      {
        mpSliderImage->SetBitmap(&bmContrast_scale);
      }
      mpSliderImage->Invalidate();
      mpSliderImage->SetVisible();
      AddChild(mpSliderImage);

      mpCursorImage = new Image(this);
      mpCursorImage->SetClientArea(20+(192/2)-10,75,20+(192/2)-10+20,75+17);
      mpCursorImage->SetBitmap(&bmCursor);
      mpCursorImage->SetVisible(true);

      AddChild(mpCursorImage);

      mLedsStatus = PLUS_LED | MINUS_LED;
      mLegalKeys  = MPC_PLUS_KEY | MPC_MINUS_KEY | MPC_MINUS_KEY_REP | MPC_PLUS_KEY_REP;
    }

    /*****************************************************************************
    * Function - Deconstructor
    * DESCRIPTION:
    *****************************************************************************/
    ContrastMessageBox::~ContrastMessageBox()
    {
      delete mpIconImage;
      delete mpSliderImage;
      delete mpCursorImage;

      if (mSubjectSubscribed)
      {
        mpContrast->Unsubscribe(this);
      }
    }

    /*****************************************************************************
    * Function - HandleKeyEvent
    * DESCRIPTION:
    *****************************************************************************/
    bool ContrastMessageBox::HandleKeyEvent(Keys key)
    {
      int contrast_value = mpContrast->GetValue();
      int direction = mControlBrightness ? -1 : 1;      

      switch (key)
      {
      case MPC_PLUS_KEY:
        contrast_value += direction * CMB_SLOW_INCREMENTATION;
        break;
      case MPC_PLUS_KEY_REP :
        contrast_value += direction * CMB_FAST_INCREMENTATION;
        break;
      case MPC_MINUS_KEY:
        contrast_value -= direction * CMB_SLOW_INCREMENTATION;
        break;
      case MPC_MINUS_KEY_REP :
        contrast_value -= direction * CMB_FAST_INCREMENTATION;
        break;
      }

      mpContrast->SetValue(contrast_value);
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Update is part of the observer pattern
    *****************************************************************************/
    void ContrastMessageBox::Update(Subject* Object)
    {
      mpCursorImage->Invalidate();
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called if subscription shall be canceled
    *****************************************************************************/
    void ContrastMessageBox::SubscribtionCancelled(Subject* pSubject)
    {
      mSubjectSubscribed = false;
      mpContrast.Unsubscribe(this);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called to set the subject pointer (used by class
    * factory)
    *****************************************************************************/
    void ContrastMessageBox::SetSubjectPointer(int Id, Subject* pSubject)
    {
      mpContrast.Attach(pSubject);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called to indicate that subscription kan be made
    *****************************************************************************/
    void ContrastMessageBox::ConnectToSubjects()
    {
      if (mpContrast.IsValid() && !mSubjectSubscribed)
      {
        mpContrast.Subscribe(this);
        mSubjectSubscribed = true;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * The Run method
    *****************************************************************************/
    void ContrastMessageBox::Run()
    {
      bool valid = mpCursorImage->IsValid();
      if (!valid)
      {
        int value = mpContrast->GetValue();
        int min_value = mpContrast->GetMinValue();
        int max_value = mpContrast->GetMaxValue();

        int v_pos = ((192 * (value - min_value)) / (max_value - min_value));

        if (mControlBrightness)
        {
          v_pos = 192 - v_pos;
        }

        if (value == min_value)
        {
          mLedsStatus = mControlBrightness ? MINUS_LED : PLUS_LED;
        }
        else if (value == max_value)
        {
          mLedsStatus = mControlBrightness ? PLUS_LED : MINUS_LED;
        }
        else
        {
          mLedsStatus = MINUS_LED | PLUS_LED;
        }

        mpCursorImage->SetChildPos(10 + v_pos, 75);
        mpCursorImage->SetVisible(true);
        WM_InvalidateWindow(mpCursorImage->GetWMHandle());
      }
      Group::Run();
    }

  } // namespace display
} // namespace mpc
