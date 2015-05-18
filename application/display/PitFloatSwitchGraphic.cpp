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
/* CLASS NAME       : PitFloatSwitchGraphic                                 */
/*                                                                          */
/* FILE NAME        : PitFloatSwitchGraphic.cpp                             */
/*                                                                          */
/* CREATED DATE     : 2007-08-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*  Shows a Pit with two animated pumps, and 5 float switch icons          */
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
#include "PitFloatSwitchGraphic.h"

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    DEFINES
    *****************************************************************************/
    // sizes and offsets in pixels
    #define X_FSW_OFFSET      7 // offset used for float switch icons
    #define Y_FSW_OFFSET      5 // offset used for float switch icons
    #define ROW_HEIGHT       18 // height used for float switch icons
    #define PFG_NQ_Y_OFFSET   0
    #define PIT_DEPTH        90
    #define LABEL_HEIGHT     15
    #define LABEL_WIDTH      15

    /*****************************************************************************
    EXTERNS
    *****************************************************************************/
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmFloatSwitchOn;

    /*****************************************************************************
    * Function...: PitFloatSwitchGraphic
    * DESCRIPTION: Constructor
    *****************************************************************************/
    PitFloatSwitchGraphic::PitFloatSwitchGraphic(Component* pParent) : AbstractPitGraphic(pParent)
    {

      for (int i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
      {
        mpFloatSwitchGroup[i] = new AvailabilityGroup(this);
        mpFloatSwitchGroup[i]->SetVisible();
        mpFloatSwitchGroup[i]->SetTransparent();

        mpFloatSwitchIcons[i] = new FloatSwitchIconState(mpFloatSwitchGroup[i]);
        mpFloatSwitchIcons[i]->SetVisible(false);
        mpFloatSwitchIcons[i]->SetTransparent(false);

        if (!mUseRelease2layout)
        {
          mpFloatSwitchIcons[i]->SetBackgroundBitmap(&bmPitWater);
        }

        mpFloatSwitchLabels[i] = new Label(mpFloatSwitchGroup[i]);
        mpFloatSwitchLabels[i]->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);
        mpFloatSwitchLabels[i]->SetAlign(GUI_TA_HCENTER|GUI_TA_RIGHT);
        mpFloatSwitchLabels[i]->SetLeftMargin(0);
        mpFloatSwitchLabels[i]->SetRightMargin(0);
        mpFloatSwitchLabels[i]->SetVisible(false);
        mpFloatSwitchLabels[i]->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
        mpFloatSwitchLabels[i]->SetTransparent();

        mpDpFloatSwitchStates[i].SetUpdated();
      }

      mpFloatSwitchLabels[0]->SetStringId(SID_5);
      mpFloatSwitchLabels[1]->SetStringId(SID_4);
      mpFloatSwitchLabels[2]->SetStringId(SID_3);
      mpFloatSwitchLabels[3]->SetStringId(SID_2);
      mpFloatSwitchLabels[4]->SetStringId(SID_1);

      mpNQFlow = new NumberQuantity(this);
      mpNQFlow->SetNumberOfDigits(3);
      mpNQFlow->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQFlow->SetAlign(GUI_TA_LEFT);
      mpNQFlow->SetHeight(NQ_HEIGHT);
      mpNQFlow->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpNQFlow->HideOnNeverAvailable();
      
      mpNQFlow->SetVisible();

      mpDpSurfaceLevel.SetUpdated();

      mpDpNoOfFloatSwitches.SetUpdated();

      for (int i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
      {
        mpFloatSwitchGroup[i]->AddChild(mpFloatSwitchIcons[i]);
        mpFloatSwitchGroup[i]->AddChild(mpFloatSwitchLabels[i]);

        AddChild(mpFloatSwitchGroup[i]);
      }
      AddChild(mpNQFlow);
    }

    /*****************************************************************************
    * Function...: ~PitFloatSwitchGraphic
    * DESCRIPTION: Dectructor
    *****************************************************************************/
    PitFloatSwitchGraphic::~PitFloatSwitchGraphic()
    {
    }

    /*****************************************************************************
    * Function...: CalculateClientAreas
    * DESCRIPTION: Set ClientArea of images, animations and Flow NQ
    *****************************************************************************/
    void PitFloatSwitchGraphic::CalculateClientAreas()
    {
      int x1, y1, x2, y2;

      AbstractPitGraphic::CalculateClientAreas();
      
      U8 pit_width_index = GetPitWidthIndex(mpDpNoOfPumpsInstalled->GetValue());
      
      x1 = mpImgPitLeft->GetChildPosX() + GetInternalPitWidth();
      x1 = (x1 > (239 - FLOW_NQ_WIDTH) ? (239 - FLOW_NQ_WIDTH) : x1);
      y1 = PFG_NQ_Y_OFFSET;
      x2 = x1 + FLOW_NQ_WIDTH  - 1;
      y2 = y1 + mpNQFlow->GetHeight() - 1;
      mpNQFlow->SetClientArea(x1, y1, x2, y2);

      for (int i = 0; i < MAX_NO_OF_FLOAT_SWITCHES; i++)
      {
        x1 = X_FSW_OFFSET + mpImgPitLeft->GetChildPosX();
        y1 = Y_FSW_OFFSET + mpImgPitLeft->GetChildPosY() + (i * ROW_HEIGHT) + (mUseRelease2layout ? -10 : 0) ;
        x2 = x1 + bmFloatSwitchOn.XSize + LABEL_WIDTH + 5 - 2;
        y2 = y1 + bmFloatSwitchOn.YSize - 1;
        mpFloatSwitchGroup[i]->SetClientArea(x1, y1, x2, y2);

        x1 = 0;
        y1 = 0;
        x2 = x1 + bmFloatSwitchOn.XSize - 1;
        y2 = y1 + bmFloatSwitchOn.YSize - 1;
        mpFloatSwitchIcons[i]->SetClientArea(x1, y1, x2, y2);

        x1 = x2 + 5;
        //y1 is not changed
        x2 = x1 + LABEL_WIDTH - 1;
        y2 = y1 + LABEL_HEIGHT - 1;

        mpFloatSwitchLabels[i]->SetClientArea(x1, y1, x2, y2);
      }

      InvalidatePitContents();

    }

    
    /*****************************************************************************
    * Function...: InvalidatePitContents
    * DESCRIPTION: 
    *****************************************************************************/
    void PitFloatSwitchGraphic::InvalidatePitContents()
    {
      AbstractPitGraphic::InvalidatePitContents();
      for (int i = 0; i < MAX_NO_OF_FLOAT_SWITCHES; i++)
      {
        mpFloatSwitchGroup[i]->Invalidate();

        int y = mpImgWater->GetChildPosY() - mpFloatSwitchGroup[i]->GetChildPosY();
        mpFloatSwitchIcons[i]->SetBackgroundOffset(0, y);
      }
    }

    /*****************************************************************************
    * Function...: UpdateSurfaceLevelPosition
    * DESCRIPTION: Set ClientArea of SurfaceLevel waves and NQ
    *****************************************************************************/
    void PitFloatSwitchGraphic::UpdateSurfaceLevelPosition()
    {
      int x1, y1, x2, y2, level;
      bool surface_level_available = mpDpSurfaceLevel->IsAvailable();

      //calculate image y location 
      if (surface_level_available)
      {        
        level = PIT_DEPTH - ROW_HEIGHT * mpDpSurfaceLevel->GetAsInt() + (mUseRelease2layout ? -10 : 0);
      }
      else
      {
        level = 0;
      }
      
      mpImgWater->SetVisible(surface_level_available);

      x1 = mpImgPitLeft->GetChildPosX() + mpImgPitLeft->GetWidth() + (mUseRelease2layout ? 0 : -1);
      y1 = mpImgPitLeft->GetChildPosY() + level;
      if (mUseRelease2layout)
      {
        x2 = x1 + GetInternalPitWidth();
        y2 = y1 + 3;
      }
      else
      {
        x2 = x1 + GetInternalPitWidth();
        y2 = mpImgPitBottom->GetChildPosY() - 1;
      }
      mpImgWater->SetClientArea(x1, y1, x2, y2);
      if (!mpImgWater->IsValid())
      {
        InvalidatePitContents();
      }



    }

    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION: 
    *****************************************************************************/
    void PitFloatSwitchGraphic::Run()
    {

      for (int i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
      {
        if (mpDpFloatSwitchStates[i].IsUpdated() || !mValid)
        {
          bool available = ((mpDpFloatSwitchStates[i]->GetValue()) != DIGITAL_INPUT_FUNC_STATE_NOT_CONFIGURED);
          
          mpFloatSwitchIcons[i]->SetVisible(available);
          mpFloatSwitchLabels[i]->SetVisible(available);
        }
      }

      if (mpDpNoOfFloatSwitches.IsUpdated())
      {
        for (int i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
        {          
          mpFloatSwitchGroup[i]->SetVisible(mpDpNoOfFloatSwitches->GetAsInt() >= (MAX_NO_OF_FLOAT_SWITCHES - i));
        }
      }

      if (mpDpSurfaceLevel.IsUpdated())
      {
        UpdateSurfaceLevelPosition();
      }

      if (mpDpNoOfPumpsInstalled.IsUpdated())
      {
        CalculateClientAreas();
        UpdateSurfaceLevelPosition();
      }

      AbstractPitGraphic::Run();
      Group::Run();
    }

    /*****************************************************************************
    * Function...: IsValid
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitFloatSwitchGraphic::IsValid()
    {
      return Component::IsValid();
    }

    /*****************************************************************************
    * Function...: Invalidate
    * DESCRIPTION:
    * Invalidate this component only
    *****************************************************************************/
    void PitFloatSwitchGraphic::Invalidate()
    {
      Component::Invalidate();
    }

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitFloatSwitchGraphic::Redraw()
    {
      return Component::Redraw();
    }

    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void PitFloatSwitchGraphic::Update(Subject* pSubject)
    {
      AbstractPitGraphic::Update(pSubject);

      for (int i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
      {
        mpDpFloatSwitchStates[i].Update(pSubject);        
      }

      mpDpSurfaceLevel.Update(pSubject);
      mpDpNoOfFloatSwitches.Update(pSubject);
    }

    /* --------------------------------------------------
    * Called if subscription shall be cancelled
    * --------------------------------------------------*/
    void PitFloatSwitchGraphic::SubscribtionCancelled(Subject* pSubject)
    {
      AbstractPitGraphic::SubscribtionCancelled(pSubject);

      for (int i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
      {
        mpDpFloatSwitchStates[i].Detach(pSubject);
      }

      mpDpSurfaceLevel.Detach(pSubject);
      mpDpNoOfFloatSwitches.Detach(pSubject);
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void PitFloatSwitchGraphic::SetSubjectPointer(int Id,Subject* pSubject)
    {
      AbstractPitGraphic::SetSubjectPointer(Id, pSubject);
      
      switch (Id)
      {
        case SP_PG_FSW_FSW_1:
          mpDpFloatSwitchStates[4].Attach(pSubject);
          mpFloatSwitchIcons[4]->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_FSW_FSW_2:
          mpDpFloatSwitchStates[3].Attach(pSubject);
          mpFloatSwitchIcons[3]->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_FSW_FSW_3:
          mpDpFloatSwitchStates[2].Attach(pSubject);
          mpFloatSwitchIcons[2]->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_FSW_FSW_4:
          mpDpFloatSwitchStates[1].Attach(pSubject);
          mpFloatSwitchIcons[1]->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_FSW_FSW_5:
          mpDpFloatSwitchStates[0].Attach(pSubject);
          mpFloatSwitchIcons[0]->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_FSW_FLOAT_SWITCH_SURFACE_LEVEL:
          mpDpSurfaceLevel.Attach(pSubject);
          break;
        case SP_PG_FSW_NO_OF_FSW:
          mpDpNoOfFloatSwitches.Attach(pSubject);
          break;
        case SP_PG_FSW_FLOW:
          mpNQFlow->SetSubjectPointer(Id, pSubject);
          break;
      }
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void PitFloatSwitchGraphic::ConnectToSubjects(void)
    {
      AbstractPitGraphic::ConnectToSubjects();

      for (int i = 0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
      {
        mpDpFloatSwitchStates[i]->Subscribe(this);

        mpFloatSwitchIcons[i]->ConnectToSubjects();
      }

      mpDpSurfaceLevel.Subscribe(this);
      mpDpNoOfFloatSwitches.Subscribe(this);

      mpNQFlow->ConnectToSubjects();
    }


  } // namespace display
} // namespace mpc


