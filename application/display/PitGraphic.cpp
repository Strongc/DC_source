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
/* CLASS NAME       : PitGraphic                                            */
/*                                                                          */
/* FILE NAME        : PitGraphic.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 2007-08-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*  Show pit with either levels or float switches                           */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "PitGraphic.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
EXTERNS
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Function...: PitGraphic
    * DESCRIPTION: Constructor
    *****************************************************************************/
    PitGraphic::PitGraphic(Component* pParent) : ObserverGroup(pParent)
    {
      mpPitLevelGraphic = new PitLevelGraphic(this);
      mpPitFloatSwitchGraphic = new PitFloatSwitchGraphic(this);

      mpPitLevelGraphic->SetVisible(false);
      mpPitFloatSwitchGraphic->SetVisible(false);

      mpDpSensorType.SetUpdated();

      AddChild(mpPitLevelGraphic);
      AddChild(mpPitFloatSwitchGraphic);

      SetVisible();
    }

    /*****************************************************************************
    * Function...: ~PitGraphic
    * DESCRIPTION: Dectructor
    *****************************************************************************/
    PitGraphic::~PitGraphic()
    {
      delete mpPitLevelGraphic;
      delete mpPitFloatSwitchGraphic;     
    }


    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION: 
    *****************************************************************************/
    void PitGraphic::Run()
    {
      if (mpDpSensorType.IsUpdated())
      {
        bool level_controlled = (mpDpSensorType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES);

        mpPitLevelGraphic->SetVisible(level_controlled);
        mpPitFloatSwitchGraphic->SetVisible(!level_controlled);
      }

      if (mpPitLevelGraphic->IsVisible())
      {
        mpPitLevelGraphic->Run();
      }
      else if (mpPitFloatSwitchGraphic->IsVisible())
      {
        mpPitFloatSwitchGraphic->Run(); 
      }

      if (!mValid)
      {
        Redraw();
      }

    }

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitGraphic::Redraw()
    {
      return Component::Redraw();
    }

    /*****************************************************************************
    * Function...: IsValid
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitGraphic::IsValid()
    {
      if (mpPitLevelGraphic->IsVisible())
      {
        mValid = mpPitLevelGraphic->IsValid();
      }
      else if (mpPitFloatSwitchGraphic->IsVisible())
      {
        mValid = mpPitFloatSwitchGraphic->IsValid(); 
      }

      return mValid;
    }

    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void PitGraphic::Update(Subject* pSubject)
    {
       mpDpSensorType.Update(pSubject);
    }

    /* --------------------------------------------------
    * Called if subscription shall be cancelled
    * --------------------------------------------------*/
    void PitGraphic::SubscribtionCancelled(Subject* pSubject)
    {
      mpDpSensorType.Detach(pSubject);
      mpPitLevelGraphic->SubscribtionCancelled(pSubject);
      mpPitFloatSwitchGraphic->SubscribtionCancelled(pSubject);
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void PitGraphic::SetSubjectPointer(int Id,Subject* pSubject)
    {

      switch (Id)
      {
        case SP_PG_SURFACE_TREND:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_SURFACE_TREND, pSubject);
          break;
        case SP_PG_SURFACE_LEVEL:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_SURFACE_LEVEL, pSubject);
          break;
        case SP_PG_UPPER_LEVEL:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_UPPER_LEVEL, pSubject);
          break;
        case SP_PG_LOWER_LEVEL:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_LOWER_LEVEL, pSubject);
          break;
        case SP_PG_MIN_VARIATION_LEVEL:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_MIN_VARIATION_LEVEL, pSubject);
          break;
        case SP_PG_MAX_VARIATION_LEVEL:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_MAX_VARIATION_LEVEL, pSubject);
          break;
        case SP_PG_FLOW:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_FLOW, pSubject);
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_FLOW, pSubject);
          break;
        case SP_PG_PIT_DEPTH:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_PIT_DEPTH, pSubject);
          break;
        case SP_PG_FSW_1:
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_FSW_1, pSubject);
          break;
        case SP_PG_FSW_2:
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_FSW_2, pSubject);
          break;
        case SP_PG_FSW_3:
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_FSW_3, pSubject);
          break;
        case SP_PG_FSW_4:
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_FSW_4, pSubject);
          break;
        case SP_PG_FSW_5:
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_FSW_5, pSubject);
          break;
        case SP_PG_FLOAT_SWITCH_SURFACE_LEVEL:
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_FLOAT_SWITCH_SURFACE_LEVEL, pSubject);
          break;
        case SP_PG_NO_OF_FSW:
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_NO_OF_FSW, pSubject);
          break;

        case SP_PG_SENSOR_TYPE:
          mpDpSensorType.Attach(pSubject);
          mpPitLevelGraphic->SetSubjectPointer(Id, pSubject);
          mpPitFloatSwitchGraphic->SetSubjectPointer(Id, pSubject);
          break;

        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_1:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_2:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_3:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_4:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_5:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_6:
        case SP_PG_ALARM_STATE_PUMP_1:
        case SP_PG_ALARM_STATE_PUMP_2:
        case SP_PG_ALARM_STATE_PUMP_3:
        case SP_PG_ALARM_STATE_PUMP_4:
        case SP_PG_ALARM_STATE_PUMP_5:
        case SP_PG_ALARM_STATE_PUMP_6:
        case SP_PG_NO_OF_PUMPS_INSTALLED:
        case SP_PG_NO_OF_RUNNING_PUMPS:
        case SP_PG_OPERATION_MODE_MIXER:
          mpPitLevelGraphic->SetSubjectPointer(Id, pSubject);
          mpPitFloatSwitchGraphic->SetSubjectPointer(Id, pSubject);
          break;

        case SP_PG_SIM_ENABLED:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_SIM_ENABLED, pSubject);
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_SIM_ENABLED, pSubject);
          break;
        case SP_PG_SIM_INLET_FLOW:
          mpPitLevelGraphic->SetSubjectPointer(SP_PG_L_SIM_INLET_FLOW, pSubject);
          mpPitFloatSwitchGraphic->SetSubjectPointer(SP_PG_FSW_SIM_INLET_FLOW, pSubject);
          break;
      }

    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void PitGraphic::ConnectToSubjects(void)
    {
      mpDpSensorType.Subscribe(this);
      mpPitLevelGraphic->ConnectToSubjects();
      mpPitFloatSwitchGraphic->ConnectToSubjects();
    }

    
    /*****************************************************************************
    * Function...: SetClientArea
    * DESCRIPTION:
    * SetClientArea on all children
    *****************************************************************************/
    void PitGraphic::SetClientArea(int x1, int y1, int x2, int y2)
    {
      int width, height;
      width = abs(x2 - x1) + 1;
      height = abs(y2 - y1) + 1;

      SetWidth(width);
      SetHeight(height);
      SetChildPos(x1,y1);
      
      mpPitLevelGraphic->SetClientArea(0, 0, width - 1, height - 1);
      mpPitFloatSwitchGraphic->SetClientArea(0, 0, width - 1, height - 1);
    }



    /*****************************************************************************
    * Function...: Invalidate
    * DESCRIPTION:
    * Invalidate all children
    *****************************************************************************/
    void PitGraphic::Invalidate()
    {
      Component::Invalidate();
      mpPitLevelGraphic->Invalidate();
      mpPitFloatSwitchGraphic->Invalidate();
    }


  } // namespace display
} // namespace mpc


