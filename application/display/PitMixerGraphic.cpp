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
/* CLASS NAME       : PitMixerGraphic                                       */
/*                                                                          */
/* FILE NAME        : PitMixerGraphic.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 2007-09-14                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*  Shows a Pit with two animated pumps, a variable surface level and       */
/*  two surveillance levels (upper and lower). Number-Quantities of all     */
/*  levels together with the pipe flow is shown when available.             */
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
#include "PitMixerGraphic.h"

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    DEFINES
    *****************************************************************************/
    // sizes and offsets in pixels
    #define PGM_NQ_WIDTH          50 
    #define PGM_NQ_HEIGHT         15
    #define PGM_L_HEIGHT          15
    #define PGM_HALF_NQ_HEIGHT     7

    #define PGM_Y_LEVEL_OFFSET    (PGM_NQ_HEIGHT + PGM_NQ_HEIGHT)
    #define PGM_Y_LEVEL_SPAN     110

    #define PGM_TRUNCATE_TOP      (PGM_Y_LEVEL_OFFSET)
    #define PGM_TRUNCATE_BOTTOM   (PGM_Y_LEVEL_OFFSET + PGM_Y_LEVEL_SPAN - PGM_HALF_NQ_HEIGHT) 
    // -2 compensates for mixer y-position
    #define PGM_TRUNCATE_PUMPS    (PGM_Y_LEVEL_OFFSET + PGM_Y_LEVEL_SPAN - AnimatedPump::HEIGHT - 2) 

    /*****************************************************************************
    EXTERNS
    *****************************************************************************/
                                                              // width,height
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitTopDash;    // 240, 12 
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLevel;      //  75,  1
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitBottom;

    /*****************************************************************************
    * Function...: PitMixerGraphic
    * DESCRIPTION: Constructor
    *****************************************************************************/
    PitMixerGraphic::PitMixerGraphic(Component* pParent) : AbstractPitGraphic(pParent)
    {
      mHideAllPumps = true;

      //calculated datapoint initialized in SetSubjectPointer
      mpDpStartLevelMixer = new FloatDataPoint();
      mpDpStartLevelMixer->SetQuantity(Q_HEIGHT);
      mpDpStartLevelMixer->SetWritable();

      mpAniMixer->SetBackgroundOffset(0, mpAniMixer->GetHeight());
      
      // create Images
      mpImgStartLevel1 = new Image(this);
      mpImgStartLevelMixer = new Image(this);
      mpImgStopLevelMixer = new Image(this);
      mpImgStartLevel1Ext = new Image(this);
      mpImgStartLevelMixerExt = new Image(this);
      mpImgStopLevelMixerExt = new Image(this);

      mpImgStartLevel1->SetBitmap(&bmPitLevel);
      mpImgStartLevelMixer->SetBitmap(&bmPitLevel);
      mpImgStopLevelMixer->SetBitmap(&bmPitLevel);
      mpImgStartLevel1Ext->SetBitmap(&bmPitLevel);
      mpImgStartLevelMixerExt->SetBitmap(&bmPitLevel);
      mpImgStopLevelMixerExt->SetBitmap(&bmPitLevel);

      for (int i = 0; i < NO_OF_PIT_WIDTHS; i++)
      {
        mpImgPitTop[i]->SetBitmap(&bmPitTopDash);
      }
      
      mpImgStartLevel1->SetTransparent();
      mpImgStartLevelMixer->SetTransparent();
      mpImgStopLevelMixer->SetTransparent();
      mpImgStartLevel1Ext->SetTransparent();
      mpImgStartLevelMixerExt->SetTransparent();
      mpImgStopLevelMixerExt->SetTransparent();
      mpImgPitTop[0]->SetTransparent();


      // create NQ
      mpNQStartLevel1 = new NumberQuantity(this);
      mpNQStartLevelMixer = new NumberQuantity(this);   
      mpNQStopLevelMixer = new NumberQuantity(this);

      mpNQStartLevel1->SetNumberOfDigits(3);
      mpNQStartLevel1->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQStartLevel1->SetAlign(GUI_TA_RIGHT);
      mpNQStartLevel1->SetWidth(PGM_NQ_WIDTH);
      mpNQStartLevel1->SetHeight(PGM_NQ_HEIGHT);
      mpNQStartLevel1->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpNQStartLevel1->SetTransparent();
      mpNQStartLevel1->SetVisible();

      mpNQStartLevelMixer->SetNumberOfDigits(3);
      mpNQStartLevelMixer->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQStartLevelMixer->SetAlign(GUI_TA_RIGHT);
      mpNQStartLevelMixer->SetWidth(PGM_NQ_WIDTH);
      mpNQStartLevelMixer->SetHeight(PGM_NQ_HEIGHT);
      mpNQStartLevelMixer->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpNQStartLevelMixer->SetTransparent();
      mpNQStartLevelMixer->SetVisible();
      mpNQStartLevelMixer->SetSubjectPointer(-1, mpDpStartLevelMixer);

      mpNQStopLevelMixer->SetNumberOfDigits(3);
      mpNQStopLevelMixer->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQStopLevelMixer->SetAlign(GUI_TA_RIGHT);
      mpNQStopLevelMixer->SetWidth(PGM_NQ_WIDTH);
      mpNQStopLevelMixer->SetHeight(PGM_NQ_HEIGHT);
      mpNQStopLevelMixer->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpNQStopLevelMixer->SetTransparent();
      mpNQStopLevelMixer->SetVisible();
      
      // create labels
      mpLStartLevel1 = new Label(this);
      mpLStartLevelMixer = new Label(this);
      mpLStopLevelMixer = new Label(this);

      mpLStartLevel1->SetFont(DEFAULT_FONT_11_LANGUAGE_DEP);
      mpLStartLevel1->SetAlign(GUI_TA_VCENTER|GUI_TA_LEFT);
      mpLStartLevel1->SetLeftMargin(0);
      mpLStartLevel1->SetRightMargin(0);
      mpLStartLevel1->SetVisible(true);
      mpLStartLevel1->SetStringId(SID_START_LEVEL_1);

      mpLStartLevelMixer->SetFont(DEFAULT_FONT_11_LANGUAGE_DEP);
      mpLStartLevelMixer->SetAlign(GUI_TA_VCENTER|GUI_TA_LEFT);
      mpLStartLevelMixer->SetLeftMargin(0);
      mpLStartLevelMixer->SetRightMargin(0);
      mpLStartLevelMixer->SetVisible(true);
      mpLStartLevelMixer->SetStringId(SID_START_LEVEL_MIXER);

      mpLStopLevelMixer->SetFont(DEFAULT_FONT_11_LANGUAGE_DEP);
      mpLStopLevelMixer->SetAlign(GUI_TA_VCENTER|GUI_TA_LEFT);
      mpLStopLevelMixer->SetLeftMargin(0);
      mpLStopLevelMixer->SetRightMargin(0);
      mpLStopLevelMixer->SetVisible(true);
      mpLStopLevelMixer->SetStringId(SID_STOP_LEVEL_MIXER);

      //set visibility
      mpImgStartLevel1->SetVisible();
      mpImgStartLevelMixer->SetVisible();
      mpImgStopLevelMixer->SetVisible();
      mpImgStartLevel1Ext->SetVisible();
      mpImgStartLevelMixerExt->SetVisible();
      mpImgStopLevelMixerExt->SetVisible();
      
      AddChild(mpNQStartLevel1);
      AddChild(mpNQStartLevelMixer);
      AddChild(mpNQStopLevelMixer);
      
      AddChild(mpImgStartLevel1);
      AddChild(mpImgStartLevelMixer);
      AddChild(mpImgStopLevelMixer);

      AddChild(mpImgStartLevel1Ext);
      AddChild(mpImgStartLevelMixerExt);
      AddChild(mpImgStopLevelMixerExt);

      AddChild(mpLStartLevel1);
      AddChild(mpLStartLevelMixer);
      AddChild(mpLStopLevelMixer);

    }

    /*****************************************************************************
    * Function...: ~PitMixerGraphic
    * DESCRIPTION: Dectructor
    *****************************************************************************/
    PitMixerGraphic::~PitMixerGraphic()
    {
    }

    /*****************************************************************************
    * Function...: CalculateClientAreas
    * DESCRIPTION: Set ClientArea of start pump level 1 (label + image + NQ)
    *****************************************************************************/
    void PitMixerGraphic::CalculateClientAreas()
    {
      int x1, y1, x2, y2;

      U8 pumps = (mpDpNoOfPumpsInstalled->GetValue() > 1 ? 2 : 1);
      
      AbstractPitGraphic::CalculateClientAreas(pumps, 15);

      x1 = mpImgPitLeft->GetChildPosX() + mpImgPitLeft->GetWidth() - 1;
      y1 = PGM_Y_LEVEL_OFFSET;
      x2 = mpImgPitRight->GetChildPosX() + mpImgPitRight->GetWidth() - 2;
      y2 = y1;
      mpImgStartLevel1->SetClientArea(x1, y1, x2, y2);
      mpImgStartLevel1Ext->SetClientArea(x1 - X_LEVEL_INDENT, y1, x1 - bmPitLeft.XSize - 1, y2);

      x1 = mpImgPitRight->GetChildPosX() + mpImgPitRight->GetWidth() + 2;
      y1 = PGM_NQ_HEIGHT;
      x2 = this->GetWidth() - 1;
      y2 = y1 + PGM_L_HEIGHT - 1;
      mpLStartLevel1->SetClientArea(x1, y1, x2, y2);

      x1 = mpImgStartLevel1Ext->GetChildPosX() - PGM_NQ_WIDTH;
      y1 = PGM_NQ_HEIGHT;    
      x2 = x1 + PGM_NQ_WIDTH - 1;
      y2 = y1 + PGM_NQ_HEIGHT - 1;
      mpNQStartLevel1->SetClientArea(x1, y1, x2, y2);
    }


    
    /*****************************************************************************
    * Function...: UpdateStartLevelMixerPosition
    * DESCRIPTION: Set ClientArea of start level mixer (label + image + NQ)
    *****************************************************************************/
    void PitMixerGraphic::UpdateStartLevelMixerPosition()
    {
      //positions in pixels     
      int x1, y1, x2, y2, line_level, label_level;
     
      //heights in measured values
      float flevel;

      if (mpDpStartLevelMixer->GetQuality() == DP_AVAILABLE 
        && (mpDpStartLevelMixer->GetAsFloat() <= mpDpStartLevel1->GetAsFloat())
        && (mpDpStartLevelOffset->GetAsFloat() <= mpDpStartLevel1->GetAsFloat()))
      { 
        flevel = mpDpStartLevelMixer->GetAsFloat();
        line_level = GetLevelPosition(flevel);
        
        mpNQStartLevelMixer->SetVisible();
        mpImgStartLevelMixer->SetVisible((!mUseRelease2layout || line_level <= PGM_TRUNCATE_PUMPS) && (line_level > 0));
        mpImgStartLevelMixerExt->SetVisible();
        mpLStartLevelMixer->SetVisible();
      }
      else
      {
        line_level = 0;
        mpNQStartLevelMixer->SetVisible(false);       
        mpImgStartLevelMixer->SetVisible(false);
        mpImgStartLevelMixerExt->SetVisible(false);
        mpLStartLevelMixer->SetVisible(false);
      }

      //calculate NQ location 
      label_level = line_level - PGM_HALF_NQ_HEIGHT; //move NQ

      x1 = mpImgPitLeft->GetChildPosX() + mpImgPitLeft->GetWidth() - 1;
      y1 = line_level;
      x2 = mpImgPitRight->GetChildPosX() + mpImgPitRight->GetWidth() - 2;
      y2 = y1;
      if (mpImgStartLevelMixer->IsVisible())
      {
        mpImgStartLevelMixer->SetClientArea(x1, y1, x2, y2);
      }

      mpImgStartLevelMixerExt->SetClientArea(x1 - X_LEVEL_INDENT, y1, x1 - bmPitLeft.XSize - 1, y2);

      //truncate vertical position of NQ
      if (label_level < PGM_TRUNCATE_TOP)
      {
        label_level = PGM_TRUNCATE_TOP;
      }
      else if (label_level > (PGM_TRUNCATE_BOTTOM))
      {
        label_level = PGM_TRUNCATE_BOTTOM;
      }

      x1 = mpImgStartLevelMixerExt->GetChildPosX() - PGM_NQ_WIDTH;
      y1 = label_level;

      if (mpDpStopLevelMixer->GetQuality() == DP_AVAILABLE 
        && mpDpStartLevelMixer->GetQuality() == DP_AVAILABLE)
      {
        float fstart_level = mpDpStartLevelMixer->GetAsFloat();
        float fstop_level = mpDpStopLevelMixer->GetAsFloat();

        // check if limit labels collide, and adjust if nessasary
        y1 = AdjustPosition(x1, y1, fstart_level, fstop_level, mpNQStopLevelMixer, mpLStopLevelMixer);

      }
      else if (mpDpStopLevelMixer->GetQuality() == DP_AVAILABLE)
      {
        mpDpStopLevelMixer.SetUpdated();
      }

      x2 = x1 + PGM_NQ_WIDTH - 1;
      y2 = y1 + PGM_NQ_HEIGHT - 1;
      mpNQStartLevelMixer->SetClientArea(x1, y1, x2, y2);
      x1 = mpImgPitRight->GetChildPosX() + mpImgPitRight->GetWidth() + 2;
      x2 = this->GetWidth() - 1;
      mpLStartLevelMixer->SetClientArea(x1, y1, x2, y2);

      InvalidatePitContents();
    }
    
    /*****************************************************************************
    * Function...: UpdateStopLevelMixerPosition
    * DESCRIPTION: Sets ClientArea of stop level. 
    *****************************************************************************/
    void PitMixerGraphic::UpdateStopLevelMixerPosition()
    {
      //positions in pixels
      int x1, y1, x2, y2, line_level, label_level;

      //calculate and set Image location 
      if (mpDpStopLevelMixer->GetQuality() == DP_AVAILABLE 
        && (mpDpStopLevelMixer->GetAsFloat() <= mpDpStartLevel1->GetAsFloat()))
      { 
        line_level = GetLevelPosition(mpDpStopLevelMixer->GetValue());

        mpNQStopLevelMixer->SetVisible();
        // hide line below pumps
        mpImgStopLevelMixer->SetVisible((!mUseRelease2layout || line_level <= PGM_TRUNCATE_PUMPS) && (line_level > 0));
        mpImgStopLevelMixerExt->SetVisible();
        mpLStopLevelMixer->SetVisible();
      }
      else
      {
        line_level = 0;
        mpNQStopLevelMixer->SetVisible(false);       
        mpImgStopLevelMixer->SetVisible(false);
        mpImgStopLevelMixerExt->SetVisible(false);
        mpLStopLevelMixer->SetVisible(false);
      }

      //calculate NQ location 
      label_level = line_level - PGM_HALF_NQ_HEIGHT; //move NQ
      
      x1 = mpImgPitLeft->GetChildPosX() + mpImgPitLeft->GetWidth() - 1;
      y1 = line_level;
      x2 = mpImgPitRight->GetChildPosX() + mpImgPitRight->GetWidth() - 2;
      y2 = y1;
      if (mpImgStopLevelMixer->IsVisible())
      {
        mpImgStopLevelMixer->SetClientArea(x1, y1, x2, y2);
      }

      mpImgStopLevelMixerExt->SetClientArea(x1 - X_LEVEL_INDENT, y1, x1 - bmPitLeft.XSize - 1, y2);

      //truncate vertical position of NQ
      if (label_level < PGM_TRUNCATE_TOP)
      {
        label_level = PGM_TRUNCATE_TOP;
      }
      else if (label_level > (PGM_TRUNCATE_BOTTOM))
      {
        label_level = PGM_TRUNCATE_BOTTOM;
      }

      x1 = mpImgStopLevelMixerExt->GetChildPosX() - PGM_NQ_WIDTH;
      y1 = label_level;

      if (mpDpStopLevelMixer->GetQuality() == DP_AVAILABLE 
        && mpDpStartLevelMixer->GetQuality() == DP_AVAILABLE)
      {
        float start_level = mpDpStartLevelMixer->GetAsFloat();
        float stop_level = mpDpStopLevelMixer->GetAsFloat();

        // check if limit labels collide, and adjust if nessasary
        y1 = AdjustPosition(x1, y1, stop_level, start_level, mpNQStartLevelMixer, mpLStartLevelMixer);

      }
      else if (mpDpStartLevelOffset->GetQuality() == DP_AVAILABLE)
      {
        mpDpStartLevelOffset.SetUpdated();
      }

      x2 = x1 + PGM_NQ_WIDTH - 1;
      y2 = y1 + PGM_NQ_HEIGHT - 1;
      mpNQStopLevelMixer->SetClientArea(x1, y1, x2, y2);

      x1 = mpImgPitRight->GetChildPosX() + mpImgPitRight->GetWidth() + 2;
      x2 = this->GetWidth() - 1;
      mpLStopLevelMixer->SetClientArea(x1, y1, x2, y2);


      InvalidatePitContents();
      
    }

    /*****************************************************************************
    * Function...: AdjustPosition
    * DESCRIPTION: Adjust labels and NQ of two levels close to each other
    *****************************************************************************/
    int PitMixerGraphic::AdjustPosition(int x1, int y1, float ValueA, float ValueB, NumberQuantity* pBNumQ, Label* pBLabel)
    {
      float b_y1 = pBNumQ->GetChildPosY();
      int delta_y = abs((int)(b_y1 - y1));
      int delta_levels = abs((int)(mpImgStartLevelMixer->GetChildPosY() - mpImgStopLevelMixer->GetChildPosY()));

      int label_x1 = mpImgPitRight->GetChildPosX() + mpImgPitRight->GetWidth() + 2;
      int label_x2 = this->GetWidth() - 1;

      if ((ValueB < ValueA) && (delta_y <= (PGM_NQ_HEIGHT + 5)))
      {
        y1 -= ((PGM_NQ_HEIGHT + 2 - delta_y) / 2);

        //truncate vertical position of NQ and Label
        if (y1 < PGM_TRUNCATE_TOP)
        {
          y1 = PGM_TRUNCATE_TOP;
        }
        else if (y1 > (PGM_TRUNCATE_BOTTOM - PGM_NQ_HEIGHT))
        {
          y1 = PGM_TRUNCATE_BOTTOM - PGM_NQ_HEIGHT;
        }

        if (delta_levels <= PGM_NQ_HEIGHT)
        {
          pBNumQ->SetClientArea(x1, y1 + PGM_NQ_HEIGHT, x1 + PGM_NQ_WIDTH - 1, y1 + PGM_NQ_HEIGHT + PGM_NQ_HEIGHT - 1);
          pBLabel->SetClientArea(label_x1, y1 + PGM_NQ_HEIGHT, label_x2, y1 + PGM_NQ_HEIGHT + PGM_L_HEIGHT - 1);
        }
      }
      else if ((ValueB >= ValueA) && (delta_y <= (PGM_NQ_HEIGHT + 5)))
      {
        y1 += ((PGM_NQ_HEIGHT + 2 - delta_y) / 2);

        //truncate vertical position of NQ and Label
        if (y1 < (PGM_TRUNCATE_TOP + PGM_NQ_HEIGHT))
        {
          y1 = (PGM_TRUNCATE_TOP + PGM_NQ_HEIGHT);
        }
        else if (y1 > (PGM_TRUNCATE_BOTTOM))
        {
          y1 = PGM_TRUNCATE_BOTTOM;
        }

        if (delta_levels <= PGM_NQ_HEIGHT)
        {
          pBNumQ->SetClientArea(x1, y1 - PGM_NQ_HEIGHT, x1 + PGM_NQ_WIDTH - 1, y1 - 1);
          pBLabel->SetClientArea(label_x1, y1 - PGM_NQ_HEIGHT, label_x2, y1 - 1);
        }
      }

      return y1;
    }


    /*****************************************************************************
    * Function...: GetLevelPosition
    * DESCRIPTION: Calculate pixel position of a measured level value
    *****************************************************************************/
    int PitMixerGraphic::GetLevelPosition(float MeasuredValue)
    {
      //heights in measured values
      float fspan;

      //position in pixels
      int level;

      fspan = mpDpStartLevel1->GetAsFloat();

      //set level to top position
      level = PGM_Y_LEVEL_OFFSET;

      if (MeasuredValue < fspan)
      {
        level += PGM_Y_LEVEL_SPAN - (int)(((float)PGM_Y_LEVEL_SPAN) * (MeasuredValue / fspan));
      }//else use top position

      return level;
    }


    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION: 
    *****************************************************************************/
    void PitMixerGraphic::Run()
    {
      AbstractPitGraphic::Run();

      bool update_start_level = false;
      bool update_stop_level = false;

      if (mpDpStartLevel1.IsUpdated())
      {
        update_start_level = true;
        update_stop_level = true;
      }

      if (mpDpStartLevelOffset.IsUpdated())
      {
        update_start_level = true;
      }
      
      if (mpDpStopLevelMixer.IsUpdated())
      {
        update_stop_level = true;
      }

      if (mpDpNoOfPumpsInstalled.IsUpdated())
      {
        CalculateClientAreas();
      }

      if (update_start_level)
      {
        float value = (mpDpStartLevel1->GetAsFloat() - mpDpStartLevelOffset->GetAsFloat());

        mpDpStartLevelMixer->SetQuality(mpDpStartLevelOffset->GetQuality());
        mpDpStartLevelMixer->SetMaxValue(mpDpStartLevelOffset->GetMaxValue());
        mpDpStartLevelMixer->SetMinValue(mpDpStopLevelMixer->GetMinValue());

        mpDpStartLevelMixer->SetValue(value) ;
        mpNQStartLevelMixer->Update(mpDpStartLevelMixer);
 
        UpdateStartLevelMixerPosition();
      }

      if (update_stop_level)
      {
        UpdateStopLevelMixerPosition();
      }

      
      Group::Run();      
    }

    
    /*****************************************************************************
    * Function...: InvalidatePitContents
    * DESCRIPTION: 
    *****************************************************************************/
    void PitMixerGraphic::InvalidatePitContents()
    {
      AbstractPitGraphic::InvalidatePitContents();

      mpNQStartLevelMixer->Invalidate();
      mpImgStartLevelMixer->Invalidate();
      mpLStartLevelMixer->Invalidate();

      mpImgPitRight->Invalidate();
      mpAniFlowPipe->Invalidate();
      mpNQStopLevelMixer->Invalidate();

      mpImgStartLevel1->Invalidate();
      mpImgStopLevelMixer->Invalidate();

      mpLStopLevelMixer->Invalidate();

      mpImgStartLevelMixerExt->Invalidate();
      mpImgStartLevel1Ext->Invalidate();
      mpImgStopLevelMixerExt->Invalidate();
    }

    /*****************************************************************************
    * Function...: IsValid
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitMixerGraphic::IsValid()
    {
      return Component::IsValid();
    }

    /*****************************************************************************
    * Function...: Invalidate
    * DESCRIPTION:
    * Invalidate this component only
    *****************************************************************************/
    void PitMixerGraphic::Invalidate()
    {
      Component::Invalidate();
    }

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitMixerGraphic::Redraw()
    {
      return Component::Redraw();
    }

    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void PitMixerGraphic::Update(Subject* pSubject)
    {
      AbstractPitGraphic::Update(pSubject);

      mpDpStartLevel1.Update(pSubject);
      mpDpStartLevelOffset.Update(pSubject);
      mpDpStopLevelMixer.Update(pSubject);
    }

    /* --------------------------------------------------
    * Called if subscription shall be cancelled
    * --------------------------------------------------*/
    void PitMixerGraphic::SubscribtionCancelled(Subject* pSubject)
    {
      AbstractPitGraphic::SubscribtionCancelled(pSubject);

      mpDpStartLevel1.Detach(pSubject);
      mpDpStartLevelOffset.Detach(pSubject);
      mpDpStopLevelMixer.Detach(pSubject);
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void PitMixerGraphic::SetSubjectPointer(int Id,Subject* pSubject)
    {
      AbstractPitGraphic::SetSubjectPointer(Id, pSubject);
      
      switch (Id)
      {
        case SP_PGMIX_START_LEVEL_1:
          mpDpStartLevel1.Attach(pSubject);
          mpNQStartLevel1->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PGMIX_START_LEVEL_OFFSET:
          mpDpStartLevelOffset.Attach(pSubject);
          break;
        case SP_PGMIX_STOP_LEVEL:
          mpDpStopLevelMixer.Attach(pSubject);
          mpNQStopLevelMixer->SetSubjectPointer(Id, pSubject);
          break;
      }
            
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void PitMixerGraphic::ConnectToSubjects(void)
    {
      AbstractPitGraphic::ConnectToSubjects();

      mpDpStartLevel1->Subscribe(this);
      mpDpStartLevelOffset->Subscribe(this);
      mpDpStopLevelMixer->Subscribe(this);
      
      mpNQStartLevel1->ConnectToSubjects();
      mpNQStopLevelMixer->ConnectToSubjects();
      mpNQStartLevelMixer->ConnectToSubjects();

      mpDpStartLevelOffset.SetUpdated();
      mpDpStartLevel1.SetUpdated();      
   }


  } // namespace display
} // namespace mpc


