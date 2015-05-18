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
/* CLASS NAME       : PumpManifoldGraphic                                   */
/*                                                                          */
/* FILE NAME        : PumpManifoldGraphic.cpp                               */
/*                                                                          */
/* CREATED DATE     : 2009-06-17                                            */
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
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "PumpManifoldGraphic.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define PMG_PUMP_INTERVAL 41
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    EXTERNS
    *****************************************************************************/ 
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPumpGroups;
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPump;
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitPumpDisabled;
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmHorisontalFlowAnimation;

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    PumpManifoldGraphic::PumpManifoldGraphic(Component* pParent) : ObserverGroup(pParent)
    {
      mUseRelease2layout = true;
      #ifdef USE_TFT_COLOURS
      mUseRelease2layout = false;
      #endif

      if (mUseRelease2layout)
      {
        CreateAnimatedPumpFlowLayout();
      }
      else
      {
        CreateAnimatedPumpsLayout();
      }
      
      mDpOperationMode[PUMP_1].SetUpdated();
      Invalidate();
     
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    PumpManifoldGraphic::~PumpManifoldGraphic()
    {
      // ignore - the object is never deleted
    }

    
    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION:
    * Run animation or image based on alarm state and operation mode
    *****************************************************************************/
    void PumpManifoldGraphic::Run()
    {
      if (mUseRelease2layout)
      {
        RunWithAnimatedPumpFlow();
      }
      else
      {
        RunWithAnimatedPumps();
      }       
    }


    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION:
    * 
    *****************************************************************************/
    bool PumpManifoldGraphic::Redraw()
    {
      if (mValid)
      {
        return true;
      }
           
      return Group::Redraw();
    }


    /*****************************************************************************
    * Function - ConnectToSubjects
    * DESCRIPTION: 
    *****************************************************************************/
    void PumpManifoldGraphic::ConnectToSubjects()
    {
      for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
      {
        mDpOperationMode[i].Subscribe(this);
        mpDpPumpAlarmState[i].Subscribe(this);
      }
    }

    /*****************************************************************************
    * Function - Update
    * DESCRIPTION: 
    *****************************************************************************/
    void PumpManifoldGraphic::Update(Subject* pSubject)
    {
      for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
      {
        if (mDpOperationMode[i].Update(pSubject)) break;
        if (mpDpPumpAlarmState[i].Update(pSubject)) break;
      }

      Invalidate();
    }

    /*****************************************************************************
    * Function - SubscribtionCancelled
    * DESCRIPTION:
    *
    *****************************************************************************/
    void PumpManifoldGraphic::SubscribtionCancelled(Subject* pSubject)
    {
      //ignore - this never happens
    }

    /*****************************************************************************
    * Function - SetSubjectPointer
    * DESCRIPTION: 
    *****************************************************************************/
    void PumpManifoldGraphic::SetSubjectPointer(int id, Subject* pSubject)
    {
      switch (id)
      {
        case SP_PMG_OPERATION_MODE_PUMP_1:
          mDpOperationMode[PUMP_1].Attach(pSubject);
          break; 
        case SP_PMG_OPERATION_MODE_PUMP_2:
          mDpOperationMode[PUMP_2].Attach(pSubject);
          break; 
        case SP_PMG_OPERATION_MODE_PUMP_3:
          mDpOperationMode[PUMP_3].Attach(pSubject);
          break; 
        case SP_PMG_OPERATION_MODE_PUMP_4:
          mDpOperationMode[PUMP_4].Attach(pSubject);
          break; 
        case SP_PMG_OPERATION_MODE_PUMP_5:
          mDpOperationMode[PUMP_5].Attach(pSubject);
          break; 
        case SP_PMG_OPERATION_MODE_PUMP_6:
          mDpOperationMode[PUMP_6].Attach(pSubject);
          break;
        case SP_PMG_ALARM_STATE_PUMP_1:
          mpDpPumpAlarmState[PUMP_1].Attach(pSubject);
          break;
        case SP_PMG_ALARM_STATE_PUMP_2:
          mpDpPumpAlarmState[PUMP_2].Attach(pSubject);
          break;
        case SP_PMG_ALARM_STATE_PUMP_3:
          mpDpPumpAlarmState[PUMP_3].Attach(pSubject);
          break;
        case SP_PMG_ALARM_STATE_PUMP_4:
          mpDpPumpAlarmState[PUMP_4].Attach(pSubject);
          break;
        case SP_PMG_ALARM_STATE_PUMP_5:
          mpDpPumpAlarmState[PUMP_5].Attach(pSubject);
          break;
        case SP_PMG_ALARM_STATE_PUMP_6:
          mpDpPumpAlarmState[PUMP_6].Attach(pSubject);
          break;
      }
    }

    /*****************************************************************************
    * Function - CreateAnimatedPumpFlowLayout
    * DESCRIPTION: 
    *****************************************************************************/
    void PumpManifoldGraphic::CreateAnimatedPumpFlowLayout()
    {
      int x1, y1, x2, y2;

      mpImgPumpManifold = new Image(this);
      mpImgPumpManifold->SetBitmap(&bmPumpGroups);
      mpImgPumpManifold->SetTransparent();
      mpImgPumpManifold->SetAlign(GUI_TA_LEFT + GUI_TA_TOP);
      mpImgPumpManifold->SetVisible();
      
      x1 = 1; 
      y1 = 0;
      x2 = x1 + bmPumpGroups.XSize - 1;
      y2 = y1 + bmPumpGroups.YSize - 1;
      mpImgPumpManifold->SetClientArea(x1, y1, x2, y2);

      AddChild(mpImgPumpManifold);

      for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
      {
        mpAniFlowPump[i] = new AnimatedImage(this, false);
        mpAniFlowPump[i]->SetSize(13, 4);
        mpAniFlowPump[i]->SetBitmap(&bmHorisontalFlowAnimation);

        mpAniFlowPump[i]->SetVisible(false);

        x1 = 3 + (i * PMG_PUMP_INTERVAL); 
        y1 = bmPumpGroups.YSize - mpAniFlowPump[i]->GetHeight();
        x2 = x1 + mpAniFlowPump[i]->GetWidth() - 1;
        y2 = bmPumpGroups.YSize - 1;
        mpAniFlowPump[i]->SetClientArea(x1, y1, x2, y2);

        AddChild(mpAniFlowPump[i]);

        mpImgPumpDisabled[i] = new Image(this);
        mpImgPumpDisabled[i]->SetBitmap(&bmPitPumpDisabled);
        mpImgPumpDisabled[i]->SetTransparent();
        mpImgPumpDisabled[i]->SetVisible(false);

        x1 = (i * PMG_PUMP_INTERVAL); 
        y1 = 1;
        x2 = x1 + bmPitPumpDisabled.XSize - 1;
        y2 = y1 + bmPitPumpDisabled.YSize - 1;
        mpImgPumpDisabled[i]->SetClientArea(x1, y1, x2, y2);
        
        AddChild(mpImgPumpDisabled[i]);
      }
    }

    
    /*****************************************************************************
    * Function...: RunWithAnimatedPumpFlow
    * DESCRIPTION:
    * Run method for release 2 layout
    *****************************************************************************/
    void PumpManifoldGraphic::RunWithAnimatedPumpFlow()
    {
      bool last_pump_found = false;

      for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
      {
        bool pump_running = (mDpOperationMode[i]->GetValue() == ACTUAL_OPERATION_MODE_STARTED
          && mDpOperationMode[i]->GetQuality() == DP_AVAILABLE);

        bool pump_stopped = (mDpOperationMode[i]->GetValue() == ACTUAL_OPERATION_MODE_STOPPED
          && mpAniFlowPump[i]->IsVisible());

        bool pump_disabled = (mDpOperationMode[i]->GetValue() == ACTUAL_OPERATION_MODE_DISABLED 
          || mDpOperationMode[i]->GetQuality() == DP_NOT_AVAILABLE);

        bool not_installed = (mDpOperationMode[i]->GetValue() == ACTUAL_OPERATION_MODE_NOT_INSTALLED
          || mDpOperationMode[i]->GetQuality() == DP_NEVER_AVAILABLE);


        if (pump_running)
        {
          mpAniFlowPump[i]->Start();
        }
        else if (pump_stopped)
        {
          mpAniFlowPump[i]->Stop();
        }
        
        if (pump_disabled || not_installed)
        {
          mpAniFlowPump[i]->SetVisible(false);
        }

        if (not_installed)
        {
          mpImgPumpDisabled[i]->SetVisible(false);

          if (!last_pump_found)
          {
            last_pump_found = true;

            if (i > 0)
            {
              mpImgPumpManifold->SetClientArea(1, 0, PMG_PUMP_INTERVAL * i, bmPumpGroups.YSize - 1);
            }
            else
            {
              mpImgPumpManifold->SetClientArea(0, 0, 0, 0);
            }
          }
        }
        else if (!last_pump_found)
        {
          mpImgPumpDisabled[i]->SetVisible(pump_disabled);
        }
      }

      if (!last_pump_found)
      {
        mpImgPumpManifold->SetClientArea(1, 0, bmPumpGroups.XSize, bmPumpGroups.YSize - 1);
      }

      Group::Run();
    }


    /*****************************************************************************
    * Function - CreateAnimatedPumpsLayout
    * DESCRIPTION: 
    *****************************************************************************/
    void PumpManifoldGraphic::CreateAnimatedPumpsLayout()
    {
      int x0, y0, x1, y1;
      for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
      {
        x0 = i * PMG_PUMP_INTERVAL;
        y0 = 0;
        x1 = x0 + AnimatedPump::WIDTH;
        y1 = y0 + AnimatedPump::HEIGHT;

        mpAnimatedPump[i] = new AnimatedPump(this, i + 1);
        mpAnimatedPump[i]->SetClientArea(x0, y0, x1, y1);
        mpAnimatedPump[i]->SetVisible(false);
        AddChild(mpAnimatedPump[i]);
      }
    }

    /*****************************************************************************
    * Function...: RunWithAnimatedPumps
    * DESCRIPTION:
    * Run method used beyond release 2
    *****************************************************************************/
    void PumpManifoldGraphic::RunWithAnimatedPumps()
    {
      bool change_mode = false;
      for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
      {
        if (mDpOperationMode[i].IsUpdated()
          || mpDpPumpAlarmState[i].IsUpdated())
        {
          change_mode = true;
        }
      }

      if (change_mode)
      {
         for (int i = FIRST_PUMP; i <= LAST_PUMP; i++)
         {
           mpAnimatedPump[i]->ChangeMode(mDpOperationMode[i]->GetValue(), mpDpPumpAlarmState[i]->GetValue());
         }
      }
      
      Group::Run();
    }

    
  } // namespace display
} // namespace mpc


