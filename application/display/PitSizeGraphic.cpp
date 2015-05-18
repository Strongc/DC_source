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
/* CLASS NAME       : PitSizeGraphic                                        */
/*                                                                          */
/* FILE NAME        : PitSizeGraphic.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 2007-07-27                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*  Shows at Pit with variable lower and upper levels                       */
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
#include "PitSizeGraphic.h"
#include "MPCFonts.h"

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    DEFINES
    *****************************************************************************/
    // sizes and offsets in pixels
    #define X_OFFSET        0
    #define Y_OFFSET        0
    #define PSG_NQ_WIDTH   60
    #define PSG_NQ_HEIGHT  15
    #define HALF_NQ_HEIGHT  7
    #define Y_LEVEL_SPAN   85  
    #define Y_LEVEL_OFFSET 22
    #define Y_TRUNCATE      7 // truncate top vertical position of level NQ

    /*****************************************************************************
    EXTERNS
    *****************************************************************************/
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmLargePitLeft;
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmLargePitRight;
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitBottom;     
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLevel;   
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmSurfaceArea;

    /*****************************************************************************
    * Function...: PitSizeGraphic
    * DESCRIPTION: Constructor
    *****************************************************************************/
    PitSizeGraphic::PitSizeGraphic(Component* pParent) : ObserverGroup(pParent)
    {
      // create Images
      mpImgPitLeft = new Image(this);
      mpImgPitRight = new Image(this);
      mpImgPitBottom = new Image(this);
      mpImgPitUpperLevel = new Image(this);
      mpImgPitLowerLevel = new Image(this);
      mpImgPitSurfaceArea = new Image(this);

      mpImgPitLeft->SetBitmap(&bmLargePitLeft);
      mpImgPitRight->SetBitmap(&bmLargePitRight);
      mpImgPitBottom->SetBitmap(&bmPitBottom);
      mpImgPitUpperLevel->SetBitmap(&bmPitLevel);
      mpImgPitLowerLevel->SetBitmap(&bmPitLevel);
      mpImgPitSurfaceArea->SetBitmap(&bmSurfaceArea);
      
      mpImgPitUpperLevel->SetTransparent();
      mpImgPitLowerLevel->SetTransparent();
      
      // create NQ
      mpNQUpperLevel = new NumberQuantity(this);   
      mpNQLowerLevel = new NumberQuantity(this);
      mpNQPitDepth = new NumberQuantity(this);
      mpNQPitArea = new NumberQuantity(this);
      
      mpNQUpperLevel->SetNumberOfDigits(3);
      mpNQUpperLevel->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQUpperLevel->SetAlign(GUI_TA_RIGHT);
      mpNQUpperLevel->SetWidth(PSG_NQ_WIDTH);
      mpNQUpperLevel->SetHeight(PSG_NQ_HEIGHT);
      mpNQUpperLevel->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);

      mpNQLowerLevel->SetNumberOfDigits(3);
      mpNQLowerLevel->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQLowerLevel->SetAlign(GUI_TA_RIGHT);
      mpNQLowerLevel->SetWidth(PSG_NQ_WIDTH);
      mpNQLowerLevel->SetHeight(PSG_NQ_HEIGHT);
      mpNQLowerLevel->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);

      mpNQPitDepth->SetNumberOfDigits(4);
      mpNQPitDepth->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQPitDepth->SetAlign(GUI_TA_CENTER);
      mpNQPitDepth->SetWidth(PSG_NQ_WIDTH);
      mpNQPitDepth->SetHeight(PSG_NQ_HEIGHT);
      mpNQPitDepth->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);

      mpNQPitArea->SetNumberOfDigits(4);
      mpNQPitArea->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQPitArea->SetAlign(GUI_TA_CENTER);
      mpNQPitArea->SetWidth(PSG_NQ_WIDTH);
      mpNQPitArea->SetHeight(PSG_NQ_HEIGHT);
      mpNQPitArea->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);

      //create label
      mpLabelPitDepth = new Label(this);
      mpLabelPitDepth->SetStringId(SID_PIT_DEPTH);
      mpLabelPitDepth->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpLabelPitDepth->SetAlign(GUI_TA_CENTER);
      mpLabelPitDepth->SetWidth(200);
      mpLabelPitDepth->SetHeight(PSG_NQ_HEIGHT);
      mpLabelPitDepth->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      
      mpLabelPitArea = new Label(this);
      mpLabelPitArea->SetStringId(SID_PIT_AREA);
      mpLabelPitArea->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpLabelPitArea->SetAlign(GUI_TA_LEFT);
      mpLabelPitArea->SetWidth(PSG_NQ_WIDTH);
      mpLabelPitArea->SetHeight(PSG_NQ_HEIGHT);
      mpLabelPitArea->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);

      mpLabelLearning = new Label(this);
      mpLabelLearning->SetStringId(SID_LEARNING_IN_PROGRESS);
      mpLabelLearning->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpLabelLearning->SetAlign(GUI_TA_LEFT);
      mpLabelLearning->SetWordWrap();
      mpLabelLearning->SetWidth(PSG_NQ_WIDTH);
      mpLabelLearning->SetHeight(PSG_NQ_HEIGHT * 2);
      mpLabelLearning->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);

      //create yes-no state
      mpStateLearning = new YesNoState(this);
      mpStateLearning->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpStateLearning->SetAlign(GUI_TA_LEFT);
      mpStateLearning->SetWidth(PSG_NQ_WIDTH);
      mpStateLearning->SetHeight(PSG_NQ_HEIGHT);
      mpStateLearning->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);

      //calculate client areas
      SetStaticClientAreas();

      //set visibility
      mpImgPitLeft->SetVisible();
      mpImgPitRight->SetVisible();
      mpImgPitBottom->SetVisible();
      
      mpDpFlowCalculationType.SetUpdated();
      mpDpUpperLevel.SetUpdated();
      mpDpLowerLevel.SetUpdated();
      mpDpPitDepth.SetUpdated();
      mpDpPitArea.SetUpdated();
      mpDpLearningInProgress.SetUpdated();
      
      //add children
      AddChild(mpImgPitLeft);
      AddChild(mpImgPitRight);
      AddChild(mpImgPitBottom);
      AddChild(mpImgPitUpperLevel);
      AddChild(mpImgPitLowerLevel);
      AddChild(mpImgPitSurfaceArea);
      
      AddChild(mpNQUpperLevel);
      AddChild(mpNQLowerLevel);
      AddChild(mpNQPitDepth);
      AddChild(mpNQPitArea);
      AddChild(mpStateLearning);

      AddChild(mpLabelPitDepth);
      AddChild(mpLabelPitArea);
      AddChild(mpLabelLearning);

    }

    /*****************************************************************************
    * Function...: ~PitSizeGraphic
    * DESCRIPTION: Dectructor
    *****************************************************************************/
    PitSizeGraphic::~PitSizeGraphic()
    {      

    }

    /*****************************************************************************
    * Function...: SetStaticClientAreas
    * DESCRIPTION: Set ClientArea of images, animations and Flow NQ
    *****************************************************************************/
    void PitSizeGraphic::SetStaticClientAreas()
    {
      int x1, y1, x2, y2;

      x1 = X_OFFSET;
      y1 = 0;
      x2 = x1 + mpLabelPitDepth->GetWidth() - 1;
      y2 = y1 + mpLabelPitDepth->GetHeight() - 1;
      mpLabelPitDepth->SetClientArea(x1, y1, x2, y2);     

      x1 = X_OFFSET + PSG_NQ_WIDTH + bmLargePitLeft.XSize;
      y1 = mpLabelPitDepth->GetHeight();
      x2 = x1 + PSG_NQ_WIDTH - 1;
      y2 = y1 + PSG_NQ_HEIGHT - 1;
      mpNQPitDepth->SetClientArea(x1, y1, x2, y2);  

      x1 = X_OFFSET + PSG_NQ_WIDTH;
      y1 = Y_OFFSET + PSG_NQ_HEIGHT;
      x2 = x1 + bmLargePitLeft.XSize - 1;
      y2 = y1 + bmLargePitLeft.YSize - 1;
      mpImgPitLeft->SetClientArea(x1, y1, x2, y2);

      x1 = mpImgPitLeft->GetChildPosX() + bmLargePitLeft.XSize;
      y1 = mpImgPitLeft->GetChildPosY() + bmLargePitLeft.YSize - 3;
      x2 = x1 + bmSurfaceArea.XSize - 1;
      y2 = y1 + bmPitBottom.YSize - 1;
      mpImgPitBottom->SetClientArea(x1, y1, x2, y2);      

      x1 = mpImgPitLeft->GetChildPosX() + bmLargePitLeft.XSize + bmSurfaceArea.XSize;
      y1 = mpImgPitLeft->GetChildPosY();
      x2 = x1 + bmLargePitRight.XSize - 1;
      y2 = y1 + bmLargePitRight.YSize - 1;
      mpImgPitRight->SetClientArea(x1, y1, x2, y2);
      
      x1 = X_OFFSET;
      y1 = Y_OFFSET + PSG_NQ_HEIGHT + 40;
      x2 = x1 + mpLabelPitArea->GetWidth() - 1;
      y2 = y1 + mpLabelPitArea->GetHeight() - 1;
      mpLabelPitArea->SetClientArea(x1, y1, x2, y2);

      x1 = mpLabelPitArea->GetChildPosX();
      y1 = mpLabelPitArea->GetChildPosY() + mpLabelPitArea->GetHeight();
      x2 = x1 + mpNQPitArea->GetWidth() - 1;
      y2 = y1 + mpNQPitArea->GetHeight() - 1;
      mpNQPitArea->SetClientArea(x1, y1, x2, y2);

      x1 = mpImgPitLeft->GetChildPosX() + mpImgPitLeft->GetWidth();
      y1 = Y_OFFSET + PSG_NQ_HEIGHT + 40;
      x2 = x1 + bmSurfaceArea.XSize - 1;
      y2 = y1 + bmSurfaceArea.YSize - 1;
      mpImgPitSurfaceArea->SetClientArea(x1, y1, x2, y2);


      x1 = mpImgPitRight->GetChildPosX() + bmLargePitRight.XSize;
      y1 = Y_OFFSET + PSG_NQ_HEIGHT + 40;
      x2 = x1 + mpLabelLearning->GetWidth() - 1;
      y2 = y1 + mpLabelLearning->GetHeight() - 1;
      mpLabelLearning->SetClientArea(x1, y1, x2, y2);

      x1 = mpLabelLearning->GetChildPosX();
      y1 = mpLabelLearning->GetChildPosY() + mpLabelLearning->GetHeight();
      x2 = x1 + mpStateLearning->GetWidth() - 1;
      y2 = y1 + mpStateLearning->GetHeight() - 1;
      mpStateLearning->SetClientArea(x1, y1, x2, y2);
      

    }

    
    /*****************************************************************************
    * Function...: UpdateUpperLevelPosition
    * DESCRIPTION: Set ClientArea of UpperLevel line and NQ
    *****************************************************************************/
    void PitSizeGraphic::UpdateUpperLevelPosition()
    {
      int x1, y1, x2, y2, level;

      //calculate and set Image location 
      if (mpDpUpperLevel->GetQuality() == DP_AVAILABLE)
      {        
        level = Y_LEVEL_SPAN - ((mpDpUpperLevel->GetValueAsPercent() * Y_LEVEL_SPAN) / 100) + Y_LEVEL_OFFSET;
        mpNQUpperLevel->SetVisible();
        mpImgPitUpperLevel->SetVisible();
      }
      else
      {
        level = 0;
        mpNQUpperLevel->SetVisible(false);       
        mpImgPitUpperLevel->SetVisible(false);
      }

      x1 = X_OFFSET + PSG_NQ_WIDTH  + bmLargePitLeft.XSize;
      y1 = Y_OFFSET + PSG_NQ_HEIGHT + level;
      x2 = x1 + bmSurfaceArea.XSize - 1;
      y2 = y1 + bmPitLevel.YSize - 1;
      mpImgPitUpperLevel->SetClientArea(x1, y1, x2, y2);


      //calculate and set NQ location 
      level += HALF_NQ_HEIGHT; //move NQ downwards

      //truncate vertical position of NQ
      if (level < Y_TRUNCATE)
      {
        level = Y_TRUNCATE;
      }

      x1 = X_OFFSET;
      y1 = Y_OFFSET + level;

      if (mpDpLowerLevel->GetValue() <= mpDpUpperLevel->GetValue() && (mpNQLowerLevel->GetChildPosY() - y1) < PSG_NQ_HEIGHT)
      {
        y1 -= ((PSG_NQ_HEIGHT - (mpNQLowerLevel->GetChildPosY() - y1)) / 2) + 1;
        //truncate vertical position of NQ
        if (y1 < (Y_OFFSET + Y_TRUNCATE))
        {
          y1 = (Y_OFFSET + Y_TRUNCATE);
        }

        mpNQLowerLevel->SetClientArea(x1, y1 + PSG_NQ_HEIGHT, x1 + PSG_NQ_WIDTH - 1, y1 + PSG_NQ_HEIGHT + PSG_NQ_HEIGHT - 1);
      }
      else if (mpDpLowerLevel->GetValue() > mpDpUpperLevel->GetValue() && (y1 - mpNQLowerLevel->GetChildPosY()) < PSG_NQ_HEIGHT)
      {
        y1 += ((PSG_NQ_HEIGHT - (y1 - mpNQLowerLevel->GetChildPosY())) / 2) + 1;
        //truncate vertical position of NQ
        if (y1 < (Y_OFFSET + Y_TRUNCATE + PSG_NQ_HEIGHT))
        {
          y1 = (Y_OFFSET + Y_TRUNCATE + PSG_NQ_HEIGHT);
        }

        mpNQLowerLevel->SetClientArea(x1, y1 - PSG_NQ_HEIGHT, x1 + PSG_NQ_WIDTH - 1, y1 - 1);
      }
      else if (!mpNQUpperLevel->IsValid())
      {
        mpDpLowerLevel.SetUpdated();
      }

      x2 = x1 + PSG_NQ_WIDTH - 1;
      y2 = y1 + PSG_NQ_HEIGHT - 1;
      mpNQUpperLevel->SetClientArea(x1, y1, x2, y2);


      mpImgPitLowerLevel->Invalidate();
      mpNQUpperLevel->Invalidate();
    }
    
    /*****************************************************************************
    * Function...: UpdateLowerLevelPosition
    * DESCRIPTION: Set ClientArea of LowerLevel line and NQ. 
    * Sets ClientArea of UpperLevel NQ if NQ are close.
    *****************************************************************************/
    void PitSizeGraphic::UpdateLowerLevelPosition()
    {
      int x1, y1, x2, y2, level;

      //calculate and set Image location 
      if (mpDpLowerLevel->GetQuality() == DP_AVAILABLE)
      {        
        level = Y_LEVEL_SPAN - ((mpDpLowerLevel->GetValueAsPercent() * Y_LEVEL_SPAN) / 100) + Y_LEVEL_OFFSET;
        mpNQLowerLevel->SetVisible();
        mpImgPitLowerLevel->SetVisible();
      }
      else
      {
        level = 0;
        mpNQLowerLevel->SetVisible(false);       
        mpImgPitLowerLevel->SetVisible(false);
      }

      x1 = X_OFFSET + PSG_NQ_WIDTH  + bmLargePitLeft.XSize;
      y1 = Y_OFFSET + PSG_NQ_HEIGHT + level;
      x2 = x1 + bmSurfaceArea.XSize - 1;
      y2 = y1 + bmPitLevel.YSize - 1;
      mpImgPitLowerLevel->SetClientArea(x1, y1, x2, y2);

      //calculate and set NQ location 
      level += HALF_NQ_HEIGHT; //move NQ downwards

      //truncate vertical position of NQ
      if (level < Y_TRUNCATE)
      {
        level = Y_TRUNCATE;
      }

      x1 = X_OFFSET;
      y1 = Y_OFFSET + level;

      if (mpDpLowerLevel->GetValue() <= mpDpUpperLevel->GetValue() && (y1 - mpNQUpperLevel->GetChildPosY()) < PSG_NQ_HEIGHT)
      {
        y1 += ((PSG_NQ_HEIGHT - (y1 - mpNQUpperLevel->GetChildPosY())) / 2) + 1;
        //truncate vertical position of NQ
        if (y1 < (Y_OFFSET + Y_TRUNCATE + PSG_NQ_HEIGHT))
        {
          y1 = (Y_OFFSET + Y_TRUNCATE + PSG_NQ_HEIGHT);
        }
        
        mpNQUpperLevel->SetClientArea(x1, y1 - PSG_NQ_HEIGHT, x1 + PSG_NQ_WIDTH - 1, y1 - 1);
      }
      else if (mpDpLowerLevel->GetValue() > mpDpUpperLevel->GetValue() && (mpNQUpperLevel->GetChildPosY() - y1) < PSG_NQ_HEIGHT)
      {
        y1 -= ((PSG_NQ_HEIGHT - (mpNQUpperLevel->GetChildPosY() - y1)) / 2) + 1;
        //truncate vertical position of NQ
        if  (y1 < (Y_OFFSET + Y_TRUNCATE))
        {
          y1 = (Y_OFFSET + Y_TRUNCATE);
        }
        
        mpNQUpperLevel->SetClientArea(x1, y1 + PSG_NQ_HEIGHT, x1 + PSG_NQ_WIDTH - 1, y1 + PSG_NQ_HEIGHT + PSG_NQ_HEIGHT - 1);
      }
      else if (!mpNQLowerLevel->IsValid())
      {
        mpDpUpperLevel.SetUpdated();
      }


      x2 = x1 + PSG_NQ_WIDTH - 1;
      y2 = y1 + PSG_NQ_HEIGHT - 1;
      mpNQLowerLevel->SetClientArea(x1, y1, x2, y2);
     
      mpImgPitUpperLevel->Invalidate();
      mpNQLowerLevel->Invalidate();
    }

    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION: 
    *****************************************************************************/
    void PitSizeGraphic::Run()
    {
      bool is_pit_based = mpDpFlowCalculationType->GetValue() == FLOW_CALCULATION_PIT_BASED;
      bool is_advanced = mpDpFlowCalculationType->GetValue() == FLOW_CALCULATION_ADVANCED;

      if (mpDpFlowCalculationType.IsUpdated())
      {
        mpNQUpperLevel->SetVisible(is_pit_based);
        mpNQLowerLevel->SetVisible(is_pit_based);
        mpImgPitUpperLevel->SetVisible(is_pit_based);
        mpImgPitLowerLevel->SetVisible(is_pit_based);

        mpLabelPitArea->SetVisible(is_advanced);
        mpNQPitArea->SetVisible(is_advanced);
        mpLabelLearning->SetVisible(is_advanced);
        mpStateLearning->SetVisible(is_advanced);
        mpImgPitSurfaceArea->SetVisible(is_advanced);
      }

      if (is_pit_based)
      {
        if (mpDpUpperLevel.IsUpdated())
        {
          UpdateUpperLevelPosition();
        }

        if (mpDpLowerLevel.IsUpdated())
        {
          UpdateLowerLevelPosition();
        }
      }
      

      if (mpDpPitDepth.IsUpdated())
      {
        bool depth_visible = (mpDpPitDepth->GetQuality() == DP_AVAILABLE);

        mpLabelPitDepth->SetVisible(depth_visible);
        mpNQPitDepth->SetVisible(depth_visible);
      }
    
      Group::Run();
    }

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION: Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool PitSizeGraphic::Redraw()
    {
      if (mValid)
      {
        return true;
      }
           
      return Group::Redraw();
    }

    /*****************************************************************************
    * Function...: IsValid
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitSizeGraphic::IsValid()
    {
      return mValid;
    }

    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void PitSizeGraphic::Update(Subject* pSubject)
    {
      mpDpUpperLevel.Update(pSubject);
      mpDpLowerLevel.Update(pSubject);
      mpDpPitDepth.Update(pSubject);
      mpDpPitArea.Update(pSubject);
      mpDpLearningInProgress.Update(pSubject);
      mpDpFlowCalculationType.Update(pSubject);
    }

    /* --------------------------------------------------
    * Called if subscription shall be cancelled
    * --------------------------------------------------*/
    void PitSizeGraphic::SubscribtionCancelled(Subject* pSubject)
    {
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void PitSizeGraphic::SetSubjectPointer(int Id,Subject* pSubject)
    {
      switch (Id)
      {
        case SP_PSG_UPPER_LEVEL:
          mpDpUpperLevel.Attach(pSubject);
          mpNQUpperLevel->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PSG_LOWER_LEVEL:
          mpDpLowerLevel.Attach(pSubject);
          mpNQLowerLevel->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PSG_PIT_DEPTH:
          mpDpPitDepth.Attach(pSubject);
          mpNQPitDepth->SetSubjectPointer(Id, pSubject);
          break;       
        case SP_PSG_PIT_SURFACE_AREA:
          mpDpPitArea.Attach(pSubject);
          mpNQPitArea->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PSG_PIT_PARAMETER_LEARNING_IN_PROGRESS:
          mpDpLearningInProgress.Attach(pSubject);
          mpStateLearning->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PSG_FLOW_CALCULATION_TYPE:
          mpDpFlowCalculationType.Attach(pSubject);
          break;
      }
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void PitSizeGraphic::ConnectToSubjects(void)
    {
      
      mpDpPitArea->Subscribe(this);
      mpDpLearningInProgress->Subscribe(this);
      mpDpFlowCalculationType->Subscribe(this);

      mpDpUpperLevel->Subscribe(this);
      mpDpLowerLevel->Subscribe(this);
      mpDpPitDepth->Subscribe(this);
      mpNQUpperLevel->ConnectToSubjects();
      mpNQLowerLevel->ConnectToSubjects();
      mpNQPitDepth->ConnectToSubjects();
      mpNQPitArea->ConnectToSubjects();
      mpStateLearning->ConnectToSubjects();

   }

  } // namespace display
} // namespace mpc


