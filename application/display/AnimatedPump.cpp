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
/* CLASS NAME       : AnimatedPump                                          */
/*                                                                          */
/* FILE NAME        : AnimatedPump.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 2012-03-28                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "MPCFonts.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AnimatedPump.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
EXTERNS
*****************************************************************************/
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPumpTop;
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPumpAnimationDisabled;
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPumpAnimationReady;
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPumpAnimationAlarm;
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPumpAnimationWarning;

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Constructor
    * DESCRIPTION:
    *****************************************************************************/
    AnimatedPump::AnimatedPump(Component* pParent, U8 pumpNumber /*=0*/) : Group(pParent)
    {
      mPumpNumber[0] = '0' + pumpNumber;
      mPumpNumber[1] = '\0';

      SetTransparent();

      mpImgPumpTop = new Image(this);
      mpImgPumpTop->SetBitmap(&bmPumpTop);
      mpImgPumpTop->SetTransparent();
      mpImgPumpTop->SetVisible(false);
      AddChild(mpImgPumpTop);

      mpNumberText = new Text(this);
      mpNumberText->SetAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);
      mpNumberText->SetTransparent();
      mpNumberText->SetText(mPumpNumber);
      mpNumberText->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);
      mpNumberText->SetSize(bmPumpTop.XSize, bmPumpTop.YSize - 2);
      mpNumberText->SetChildPos(1, 1);
      mpNumberText->SetVisible(false);

      AddChild(mpNumberText);
      
      mpDisabled = new Image(this);
      mpDisabled->SetBitmap(&bmPumpAnimationDisabled);
      mpDisabled->SetTransparent();
      mpDisabled->SetChildPos(0, bmPumpTop.YSize);
      mpDisabled->SetVisible(false);
      AddChild(mpDisabled);

      for (int i = FIRST_ALARM_STATE; i <= LAST_ALARM_STATE; i++)
      {
        mpAnimation[i] = new AnimatedImage(this);
        mpAnimation[i]->SetChildPos(0, bmPumpTop.YSize);
        mpAnimation[i]->SetTransparent();
        mpAnimation[i]->SetVisible(false);
        mpAnimation[i]->SetSize(AnimatedPump::WIDTH, AnimatedPump::HEIGHT);
        AddChild(mpAnimation[i]);
      }

      mpAnimation[ALARM_STATE_ALARM]->SetBitmap(&bmPumpAnimationAlarm);
      mpAnimation[ALARM_STATE_WARNING]->SetBitmap(&bmPumpAnimationWarning);
      mpAnimation[ALARM_STATE_READY]->SetBitmap(&bmPumpAnimationReady);

      mpAnimation[ALARM_STATE_ALARM]->SetNumberOfStoppedImages(6);
      mpAnimation[ALARM_STATE_WARNING]->SetNumberOfStoppedImages(1);
      mpAnimation[ALARM_STATE_READY]->SetNumberOfStoppedImages(1);

      mpAnimation[ALARM_STATE_ALARM]->SetSpeed(500, 300);
      mpAnimation[ALARM_STATE_WARNING]->SetSpeed(500, 300);
      mpAnimation[ALARM_STATE_READY]->SetSpeed(500, 300);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AnimatedPump::~AnimatedPump()
    {
    }

    
    /*****************************************************************************
    * Function...: ChangeMode
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedPump::ChangeMode(ACTUAL_OPERATION_MODE_TYPE operationMode, ALARM_STATE_TYPE alarmState)
    {
      if (alarmState != ALARM_STATE_ALARM)
      {
        mpAnimation[ALARM_STATE_ALARM]->SetVisible(false);
      }
      if (alarmState != ALARM_STATE_WARNING)
      {
        mpAnimation[ALARM_STATE_WARNING]->SetVisible(false);
      }
      if (alarmState != ALARM_STATE_READY)
      {
        mpAnimation[ALARM_STATE_READY]->SetVisible(false);
      }

      switch (operationMode)
      {
      case ACTUAL_OPERATION_MODE_STARTED:
        SetVisible();
        mpAnimation[alarmState]->Start();
        mpAnimation[alarmState]->SetVisible();
        mpAnimation[alarmState]->Reset();
        break;
      case ACTUAL_OPERATION_MODE_STOPPED:
        SetVisible();
        mpAnimation[alarmState]->Stop();
        mpAnimation[alarmState]->SetVisible();
        break;
      }
      
      mpAnimation[alarmState]->Invalidate();

      if (operationMode == ACTUAL_OPERATION_MODE_DISABLED)
      {
        mpAnimation[alarmState]->SetVisible(false);
      }

      mpDisabled->SetVisible(operationMode == ACTUAL_OPERATION_MODE_DISABLED);
      SetVisible(operationMode != ACTUAL_OPERATION_MODE_NOT_INSTALLED);

      Invalidate();
    }

    /*****************************************************************************
    * Function...: SetVisible
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedPump::SetVisible(bool set /* = true*/)
    {
      Group::SetVisible(set);
      mpImgPumpTop->SetVisible(set);
      mpNumberText->SetVisible(set);
    }

    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION:
    * Run animation based on started-state
    *****************************************************************************/
    void AnimatedPump::Run()
    {
      if (!mValid)
      {
        mpImgPumpTop->Invalidate();
      }

      Group::Run();
    }

 

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION:
    *****************************************************************************/
    bool AnimatedPump::Redraw()
    {
      return Group::Redraw();
    }
    

  } // namespace display
} // namespace mpc


