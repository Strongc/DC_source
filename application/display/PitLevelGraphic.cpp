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
/* CLASS NAME       : PitLevelGraphic                                       */
/*                                                                          */
/* FILE NAME        : PitLevelGraphic.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 2007-08-17                                            */
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
#include "PitLevelGraphic.h"

namespace mpc
{
  namespace display
  {

    /*****************************************************************************
    DEFINES
    *****************************************************************************/
    // sizes and offsets in pixels
    #define PLG_NQ_WIDTH          (NQ_WIDTH)
    #define PLG_ICON_SIZE         (bmTrendUp.XSize) // width = height
    #define PLG_HALF_NQ_HEIGHT     7
    #define PLG_Y_LEVEL_SPAN     110  
    #define PLG_Y_LEVEL_OFFSET     4
    #define PLG_Y_TRUNCATE         7 // truncate top vertical position of level NQ
    #define PLG_HALF_WAVE_HEIGHT   2
    // -5 compensates for mixer y-position
    #define PLG_PUMP_POSITION     (PLG_Y_LEVEL_OFFSET + PLG_Y_LEVEL_SPAN - bmPit1Pump.YSize - 5) 

    #define SURFACE_UPDATE_RATIO  3 // update surface level (if changed): each RATIO x 100 ms = 300 ms

    /*****************************************************************************
    EXTERNS
    *****************************************************************************/
                                                              // width,height
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitSonicSensor;//  22, 18
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmTrendUp;       //  13, 13
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLevelSensor;//   6, 35 
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitWater;      // 151, 110 

    /*****************************************************************************
    * Function...: PitLevelGraphic
    * DESCRIPTION: Constructor
    *****************************************************************************/
    PitLevelGraphic::PitLevelGraphic(Component* pParent) : AbstractPitGraphic(pParent)
    {
      // create Images
      mpImgPitLevelSensor = new Image(this);
      mpImgPitSonicSensor = new Image(this);
      mpImgPitLevelSensor->SetBitmap(&bmPitLevelSensor);
      mpImgPitSonicSensor->SetBitmap(&bmPitSonicSensor);
      mpImgPitLevelSensor->SetTransparent();
      mpImgPitSonicSensor->SetTransparent();

      // create trend icon
      mpIconSurfaceTrend = new TrendIconState(this);
      mpIconSurfaceTrend->SetWidth(PLG_ICON_SIZE);
      mpIconSurfaceTrend->SetHeight(PLG_ICON_SIZE);

      // create NQ
      mpNQSurfaceLevel = new NumberQuantity(this);
      mpNQUpperLevel = new NumberQuantity(this);   
      mpNQLowerLevel = new NumberQuantity(this);
      mpNQFlow = new NumberQuantity(this);

      mpNQSurfaceLevel->SetNumberOfDigits(3);
      mpNQSurfaceLevel->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQSurfaceLevel->SetAlign(GUI_TA_LEFT);
      mpNQSurfaceLevel->SetWidth(PLG_NQ_WIDTH);
      mpNQSurfaceLevel->SetHeight(NQ_HEIGHT);
      mpNQSurfaceLevel->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      
      mpNQUpperLevel->SetNumberOfDigits(3);
      mpNQUpperLevel->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQUpperLevel->SetAlign(GUI_TA_RIGHT);
      mpNQUpperLevel->SetWidth(PLG_NQ_WIDTH);
      mpNQUpperLevel->SetHeight(NQ_HEIGHT);
      mpNQUpperLevel->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpNQUpperLevel->SetTransparent();

      mpNQLowerLevel->SetNumberOfDigits(3);
      mpNQLowerLevel->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQLowerLevel->SetAlign(GUI_TA_RIGHT);
      mpNQLowerLevel->SetWidth(PLG_NQ_WIDTH);
      mpNQLowerLevel->SetHeight(NQ_HEIGHT);
      mpNQLowerLevel->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpNQLowerLevel->SetTransparent();
      
      mpNQFlow->SetNumberOfDigits(3);
      mpNQFlow->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQFlow->SetAlign(GUI_TA_LEFT);
      mpNQFlow->SetHeight(NQ_HEIGHT);
      mpNQFlow->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpNQFlow->HideOnNeverAvailable();
      
      //set visibility
      mpImgPitLevelSensor->SetVisible(false);
      mpImgPitSonicSensor->SetVisible(false);

      mpNQFlow->SetVisible();

      mpDpSurfaceLevel.SetUpdated();
      mpDpUpperLevel.SetUpdated();
      mpDpLowerLevel.SetUpdated();
      mpDpPitDepth.SetUpdated();
      
      AddChild(mpImgPitLevelSensor);
      AddChild(mpImgPitSonicSensor);
      AddChild(mpIconSurfaceTrend);
      AddChild(mpNQSurfaceLevel);
      AddChild(mpNQUpperLevel);
      AddChild(mpNQLowerLevel);
      AddChild(mpNQFlow);

      mNoOfRunsSinceSurfaceUpdate = 0;
    }

    /*****************************************************************************
    * Function...: ~PitLevelGraphic
    * DESCRIPTION: Dectructor
    *****************************************************************************/
    PitLevelGraphic::~PitLevelGraphic()
    {
    }

    /*****************************************************************************
    * Function...: CalculateClientAreas
    * DESCRIPTION: Set ClientArea of images and Flow NQ
    *****************************************************************************/
    void PitLevelGraphic::CalculateClientAreas()
    {
      int x1, y1, x2, y2;

      AbstractPitGraphic::CalculateClientAreas();

      U8 pit_width_index = GetPitWidthIndex(mpDpNoOfPumpsInstalled->GetValue());
      
      x1 = mpImgPitLeft->GetChildPosX() + GetInternalPitWidth();
      x1 = (x1 > (239 - FLOW_NQ_WIDTH) ? (239 - FLOW_NQ_WIDTH) : x1);
      y1 = 0;
      x2 = x1 + FLOW_NQ_WIDTH  - 1;
      y2 = y1 + mpNQFlow->GetHeight() - 1;
      mpNQFlow->SetClientArea(x1, y1, x2, y2);

      if (mUseRelease2layout)
      {
        x1 = mpImgPitRight->GetChildPosX() - 23;
        y1 = mpImgPitLeft->GetChildPosY() + bmPitLeft.YSize - bmPitLevelSensor.YSize - 13;
      }
      else
      {
        x1 = mpAniMixer->GetChildPosX() + mpAniMixer->GetWidth() + 1;
        y1 = mpImgPitLeft->GetChildPosY() + bmPitLeft.YSize - bmPitLevelSensor.YSize - 2;
      }
      x2 = x1 + bmPitLevelSensor.XSize - 1;
      y2 = y1 + bmPitLevelSensor.YSize - 1;
      mpImgPitLevelSensor->SetClientArea(x1, y1, x2, y2);
     
      x1 = mpImgPitLeft->GetChildPosX() + (GetInternalPitWidth() - bmPitSonicSensor.XSize) / 2;
      y1 = mpImgPitTop[pit_width_index]->GetChildPosY() - 10;
      x2 = x1 + bmPitSonicSensor.XSize - 1;
      y2 = y1 + bmPitSonicSensor.YSize - 1;
      mpImgPitSonicSensor->SetClientArea(x1, y1, x2, y2);
      mpImgPitSonicSensor->Invalidate();

      InvalidatePitContents();
    }



    /*****************************************************************************
    * Function...: UpdateSurfaceLevelPosition
    * DESCRIPTION: Set ClientArea of SurfaceLevel waves and NQ
    *****************************************************************************/
    void PitLevelGraphic::UpdateSurfaceLevelPosition()
    {
      //positions in pixels
      int x1, y1, x2, y2, level;
      bool level_is_visible;
      U8 number_of_installed_pumps = mpDpNoOfPumpsInstalled->GetValue();
      U8 pit_width_index = GetPitWidthIndex(number_of_installed_pumps);

      //calculate and set Image location 
      if (mpDpSurfaceLevel->IsAvailable())
      { 
        level = GetLevelPosition(PLG_HALF_WAVE_HEIGHT, mpDpSurfaceLevel->GetValue());

        mpIconSurfaceTrend->SetVisible();
        mpNQSurfaceLevel->SetVisible();

        level_is_visible = (mUseRelease2layout 
          ? (level <= (PLG_PUMP_POSITION - PLG_HALF_WAVE_HEIGHT)) : true);

      }
      else
      {
        level = GetLevelPosition(0, 0);
        mpIconSurfaceTrend->SetVisible(false);
        mpNQSurfaceLevel->SetVisible(false); 

        level_is_visible = false;
      }

      mpImgWater->SetVisible(level_is_visible);

      x1 = mpImgPitLeft->GetChildPosX() + mpImgPitLeft->GetWidth() + (mUseRelease2layout ? 0 : -1);
      y1 = NQ_HEIGHT + PIT_TOP_HEIGHT + level;
      x2 = x1 + GetInternalPitWidth() - 1;
      
      if (mUseRelease2layout)
      {
        y2 = y1 + 3;
      }
      else
      {
        y2 = mpImgPitBottom->GetChildPosY() - 1;
      }

      if (level_is_visible)
      {
        ChangeWaterLevelClientArea(x1, y1, x2, y2);
        
        x1 = mpImgWater->GetChildPosX() - mpAniMixer->GetChildPosX();
        y1 = mpImgWater->GetChildPosY() - mpAniMixer->GetChildPosY();
        mpAniMixer->SetBackgroundOffset(x1, y1);
      }
      else
      {
        mpAniMixer->SetBackgroundOffset(0, mpAniMixer->GetHeight());
      }

      if (!mpImgWater->IsValid() || !level_is_visible)
      {
        InvalidatePitContents();
      }

      //calculate and set NQ location 
      int nq_level = level + PLG_HALF_NQ_HEIGHT; //move NQ downwards

      //truncate vertical position of NQ (5 pixels added for horisontal pipe)
      if (nq_level < (PLG_Y_LEVEL_OFFSET + PLG_Y_TRUNCATE + 5))
      {
        nq_level = (PLG_Y_LEVEL_OFFSET + PLG_Y_TRUNCATE + 5);
      }

      x1 = mpImgPitRight->GetChildPosX() + mpImgPitRight->GetWidth();
      y1 = PIT_TOP_HEIGHT + nq_level + 1; // + 1: to center vertically with text
      x2 = x1 + PLG_ICON_SIZE - 1;
      y2 = y1 + PLG_ICON_SIZE - 1;
      mpIconSurfaceTrend->SetClientArea(x1, y1, x2, y2);

      if (GetPitWidthIndex(number_of_installed_pumps) < 2)
      {
        // show NQ outside pit, in right side
        x1 = mpIconSurfaceTrend->GetChildPosX() + mpIconSurfaceTrend->GetWidth();
        y1 = PIT_TOP_HEIGHT + nq_level;
      }
      else
      {
        // show NQ inside pit, close to surface level
        x1 = mpImgPitLeft->GetChildPosX() + (GetInternalPitWidth() - PLG_NQ_WIDTH) / 2;

        int top_position = mpImgPitTop[0]->GetChildPosY() + mpImgPitTop[0]->GetHeight();
        if (level < NQ_HEIGHT)
        {
          top_position += (NQ_HEIGHT + 4);
        }
        y1 = top_position;
      }

      x2 = x1 + PLG_NQ_WIDTH - 1;
      y2 = y1 + NQ_HEIGHT - 1;
      mpNQSurfaceLevel->SetClientArea(x1, y1, x2, y2);
      
    }


    
    /*****************************************************************************
    * Function...: UpdateUpperLevelPosition
    * DESCRIPTION: Set ClientArea of UpperLevel line and NQ
    *****************************************************************************/
    void PitLevelGraphic::UpdateUpperLevelPosition()
    {
      //positions in pixels     
      int x1, y1, x2, y2, level;
      bool level_above_pumps;
      
      //calculate and set Image location 
      if (mpDpUpperLevel->GetQuality() == DP_AVAILABLE)
      {        
        level = GetLevelPosition(0, mpDpUpperLevel->GetValue());
        
        mpNQUpperLevel->SetVisible();
        mpImgPitUpperLevel->SetVisible();
      }
      else
      {
        level = GetLevelPosition(0, 0);
        mpNQUpperLevel->SetVisible(false);       
        mpImgPitUpperLevel->SetVisible(false);
        mpImgPitMinVariationLevel->SetVisible(false);
        mpImgPitMaxVariationLevel->SetVisible(false);
      }

      level_above_pumps = (mUseRelease2layout ? level <= PLG_PUMP_POSITION : true);

      x1 = mpImgPitLeft->GetChildPosX() - X_LEVEL_INDENT;
      y1 = NQ_HEIGHT + PIT_TOP_HEIGHT + level;
      x2 = x1 + X_LEVEL_INDENT + (level_above_pumps ? mpImgPitLeft->GetWidth() + GetInternalPitWidth() - 1 : 0);
      y2 = y1;
      
      mpImgPitUpperLevel->SetClientArea(x1, y1, x2, y2);
     
      bool show_min_max = (mpDpMaxVariationLevel->GetQuality() == DP_AVAILABLE);
      int min_level = GetLevelPosition(0, mpDpMinVariationLevel->GetValue());
      int max_level = GetLevelPosition(0, mpDpMaxVariationLevel->GetValue());

      if ((mUseRelease2layout ? min_level <= PLG_PUMP_POSITION : true) && show_min_max)
      {
        int min_y = NQ_HEIGHT + PIT_TOP_HEIGHT + min_level;
        mpImgPitMinVariationLevel->SetClientArea(x1, min_y, x2, min_y);
        mpImgPitMinVariationLevel->SetVisible();
      }
      else
      {
        mpImgPitMinVariationLevel->SetVisible(false);
      }

      if ((mUseRelease2layout ? max_level <= PLG_PUMP_POSITION : true) && show_min_max)
      {
        int max_y = NQ_HEIGHT + PIT_TOP_HEIGHT + max_level;
        mpImgPitMaxVariationLevel->SetClientArea(x1, max_y, x2, max_y);
        mpImgPitMaxVariationLevel->SetVisible();
      }
      else
      {
        mpImgPitMaxVariationLevel->SetVisible(false);
      }

      //calculate and set NQ location 
      level += PLG_HALF_NQ_HEIGHT; //move NQ downwards

      //truncate vertical position of NQ
      if (level < PLG_Y_TRUNCATE)
      {
        level = PLG_Y_TRUNCATE;
      }

      x1 = mpImgPitUpperLevel->GetChildPosX() - PLG_NQ_WIDTH - 2;
      y1 = PIT_TOP_HEIGHT + level;

      if (mpDpLowerLevel->GetQuality() == DP_AVAILABLE 
        && mpDpUpperLevel->GetQuality() == DP_AVAILABLE)
      {
        // check if upper and lower limit labels collide, and adjust if nessasary

        if (mpDpLowerLevel->GetValue() <= mpDpUpperLevel->GetValue() 
          && ((mpNQLowerLevel->GetChildPosY() - y1) < NQ_HEIGHT))
        {
          y1 -= ((NQ_HEIGHT - (mpNQLowerLevel->GetChildPosY() - y1)) / 2) + 1;
          //truncate vertical position of NQ
          if (y1 < (PIT_TOP_HEIGHT + PLG_Y_TRUNCATE))
            y1 = (PIT_TOP_HEIGHT + PLG_Y_TRUNCATE);

          mpNQLowerLevel->SetClientArea(x1, y1 + NQ_HEIGHT, x1 + PLG_NQ_WIDTH - 1, y1 + NQ_HEIGHT + NQ_HEIGHT - 1);
        }
        else if (mpDpLowerLevel->GetValue() > mpDpUpperLevel->GetValue() 
          && ((y1 - mpNQLowerLevel->GetChildPosY()) < NQ_HEIGHT))
        {
          y1 += ((NQ_HEIGHT - (y1 - mpNQLowerLevel->GetChildPosY())) / 2) + 1;
          //truncate vertical position of NQ
          if (y1 < (PIT_TOP_HEIGHT + PLG_Y_TRUNCATE + NQ_HEIGHT))
            y1 = (PIT_TOP_HEIGHT + PLG_Y_TRUNCATE + NQ_HEIGHT);

          mpNQLowerLevel->SetClientArea(x1, y1 - NQ_HEIGHT, x1 + PLG_NQ_WIDTH - 1, y1 - 1);
        }
        else if (!mpNQUpperLevel->IsValid())
        {
          mpDpLowerLevel.SetUpdated();
        }
      }
      else if (mpDpLowerLevel->GetQuality() == DP_AVAILABLE)
      {
        mpDpLowerLevel.SetUpdated();
      }

      x2 = x1 + PLG_NQ_WIDTH - 1;
      y2 = y1 + NQ_HEIGHT - 1;
      mpNQUpperLevel->SetClientArea(x1, y1, x2, y2);

      InvalidatePitContents();
    }
    
    /*****************************************************************************
    * Function...: UpdateLowerLevelPosition
    * DESCRIPTION: Set ClientArea of LowerLevel line and NQ. 
    * Sets ClientArea of UpperLevel NQ if NQ are close.
    *****************************************************************************/
    void PitLevelGraphic::UpdateLowerLevelPosition()
    {
      //positions in pixels
      int x1, y1, x2, y2, level;
      bool level_above_pumps;

      //calculate and set Image location 
      if (mpDpLowerLevel->GetQuality() == DP_AVAILABLE)
      { 
        level = GetLevelPosition(0, mpDpLowerLevel->GetValue());

        mpNQLowerLevel->SetVisible();
        mpImgPitLowerLevel->SetVisible();
      }
      else
      {
        level = GetLevelPosition(0, 0);
        mpNQLowerLevel->SetVisible(false);       
        mpImgPitLowerLevel->SetVisible(false);
      }

      level_above_pumps = (mUseRelease2layout ? level <= PLG_PUMP_POSITION : true);

      x1 = mpImgPitLeft->GetChildPosX() - X_LEVEL_INDENT;
      y1 = NQ_HEIGHT + PIT_TOP_HEIGHT + level;
      x2 = x1 + X_LEVEL_INDENT + (level_above_pumps ? mpImgPitLeft->GetWidth() + GetInternalPitWidth() - 1 : 0);
      y2 = y1;
      
      mpImgPitLowerLevel->SetClientArea(x1, y1, x2, y2);

      //calculate and set NQ location 
      level += PLG_HALF_NQ_HEIGHT; //move NQ downwards

      //truncate vertical position of NQ
      if (level < PLG_Y_TRUNCATE)
        level = PLG_Y_TRUNCATE;

      x1 -= (PLG_NQ_WIDTH + 2);
      y1 = PIT_TOP_HEIGHT + level;

      if (mpDpLowerLevel->GetQuality() == DP_AVAILABLE 
        && mpDpUpperLevel->GetQuality() == DP_AVAILABLE)
      {
        // check if upper and lower limit labels collide, and adjust if nessasary

        if (mpDpLowerLevel->GetValue() <= mpDpUpperLevel->GetValue() 
          && ((y1 - mpNQUpperLevel->GetChildPosY()) < NQ_HEIGHT))
        {
          y1 += ((NQ_HEIGHT - (y1 - mpNQUpperLevel->GetChildPosY())) / 2) + 1;
          //truncate vertical position of NQ
          if (y1 < (PIT_TOP_HEIGHT + PLG_Y_TRUNCATE + NQ_HEIGHT))
            y1 = (PIT_TOP_HEIGHT + PLG_Y_TRUNCATE + NQ_HEIGHT);
          
          mpNQUpperLevel->SetClientArea(x1, y1 - NQ_HEIGHT, x1 + PLG_NQ_WIDTH - 1, y1 - 1);
        }
        else if (mpDpLowerLevel->GetValue() > mpDpUpperLevel->GetValue() 
          && ((mpNQUpperLevel->GetChildPosY() - y1) < NQ_HEIGHT))
        {
          y1 -= ((NQ_HEIGHT - (mpNQUpperLevel->GetChildPosY() - y1)) / 2) + 1;
          //truncate vertical position of NQ
          if (y1 < (PIT_TOP_HEIGHT + PLG_Y_TRUNCATE))
            y1 = (PIT_TOP_HEIGHT + PLG_Y_TRUNCATE);
          
          mpNQUpperLevel->SetClientArea(x1, y1 + NQ_HEIGHT, x1 + PLG_NQ_WIDTH - 1, y1 + NQ_HEIGHT + NQ_HEIGHT - 1);
        }
        else if (!mpNQLowerLevel->IsValid())
        {
          mpDpUpperLevel.SetUpdated();
        }
      }
      if (mpDpUpperLevel->GetQuality() == DP_AVAILABLE)
      {
        mpDpUpperLevel.SetUpdated();
      }


      x2 = x1 + PLG_NQ_WIDTH - 1;
      y2 = y1 + NQ_HEIGHT - 1;
      mpNQLowerLevel->SetClientArea(x1, y1, x2, y2);

      InvalidatePitContents();
    }

    /*****************************************************************************
    * Function...: GetLevelPosition
    * DESCRIPTION: Calculate pixel position of a measured level value
    *****************************************************************************/
    int PitLevelGraphic::GetLevelPosition(int YHalfLevelHeight, float MeasuredValue)
    {
      //heights in measured values
      float fdepth, fspan;

      //position in pixels
      int level;

      fdepth = mpDpPitDepth->GetValue();
      fspan = fdepth;

      //set level to top position
      level = - YHalfLevelHeight; 

      if (MeasuredValue < fspan)
      {
        level += PLG_Y_LEVEL_SPAN - (int)(((float)PLG_Y_LEVEL_SPAN) * (MeasuredValue / fspan));
      }//else use top position

      if (!mUseRelease2layout && level < 0)
      {
        level = 0;
      }

      return level;
    }


    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION: 
    *****************************************************************************/
    void PitLevelGraphic::Run()
    {
      if (mpDpNoOfPumpsInstalled.IsUpdated())
      { 
        //when number of installed pumps is changed redraw everything
        CalculateClientAreas();
        UpdateUpperLevelPosition();
        UpdateLowerLevelPosition();
        UpdateSurfaceLevelPosition();
      }

      bool pit_depth_changed = mpDpPitDepth.IsUpdated();

      mNoOfRunsSinceSurfaceUpdate++;

      if (mpDpSurfaceLevel.IsUpdated(false) || pit_depth_changed)
      {
        if (mNoOfRunsSinceSurfaceUpdate >= SURFACE_UPDATE_RATIO)
        {
          mNoOfRunsSinceSurfaceUpdate = 0;
          mpDpSurfaceLevel.ResetUpdated();
          UpdateSurfaceLevelPosition();
        }
      }

      if (mpDpSensorType.IsUpdated())
      {
        mpImgPitLevelSensor->SetVisible(mpDpSensorType->GetValue() == SENSOR_TYPE_PRESSURE);
        mpImgPitSonicSensor->SetVisible(mpDpSensorType->GetValue() == SENSOR_TYPE_ULTRA_SONIC);
      }

      if (mpDpUpperLevel.IsUpdated() || pit_depth_changed)
      {
        UpdateUpperLevelPosition();
      }

      if (mpDpLowerLevel.IsUpdated() || pit_depth_changed)
      {
        UpdateLowerLevelPosition();
      }

      
      AbstractPitGraphic::Run();
      Group::Run();      
    }


    /*****************************************************************************
    * Function...: IsValid
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitLevelGraphic::IsValid()
    {
      return Component::IsValid();
    }
    
    /*****************************************************************************
    * Function...: InvalidatePitContents
    * DESCRIPTION: 
    *****************************************************************************/
    void PitLevelGraphic::InvalidatePitContents()
    {
      AbstractPitGraphic::InvalidatePitContents();

      mpImgPitLevelSensor->Invalidate();
      mpImgPitUpperLevel->Invalidate();
      mpImgPitLowerLevel->Invalidate();
      mpImgPitMinVariationLevel->Invalidate();
      mpImgPitMaxVariationLevel->Invalidate();
      
      mpNQUpperLevel->Invalidate();
      mpNQLowerLevel->Invalidate();
      mpNQSurfaceLevel->Invalidate();
    }


    /*****************************************************************************
    * Function...: Invalidate
    * DESCRIPTION:
    * Invalidate this component only
    *****************************************************************************/
    void PitLevelGraphic::Invalidate()
    {
      Component::Invalidate();
    }

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitLevelGraphic::Redraw()
    {
      return Component::Redraw();
    }

    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void PitLevelGraphic::Update(Subject* pSubject)
    {
      AbstractPitGraphic::Update(pSubject);

      mpDpSurfaceLevel.Update(pSubject);
      mpDpUpperLevel.Update(pSubject);
      mpDpLowerLevel.Update(pSubject);
      mpDpPitDepth.Update(pSubject);

      if (mpDpMinVariationLevel.GetSubject() == pSubject
        || mpDpMaxVariationLevel.GetSubject() == pSubject)
      {
        mpDpUpperLevel.SetUpdated();
      }
    }

    /* --------------------------------------------------
    * Called if subscription shall be cancelled
    * --------------------------------------------------*/
    void PitLevelGraphic::SubscribtionCancelled(Subject* pSubject)
    {
      AbstractPitGraphic::SubscribtionCancelled(pSubject);

      mpDpSurfaceLevel.Detach(pSubject);
      mpDpUpperLevel.Detach(pSubject);
      mpDpLowerLevel.Detach(pSubject);
      mpDpPitDepth.Detach(pSubject);
      mpDpMinVariationLevel.Detach(pSubject);
      mpDpMaxVariationLevel.Detach(pSubject);
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void PitLevelGraphic::SetSubjectPointer(int Id,Subject* pSubject)
    {
      AbstractPitGraphic::SetSubjectPointer(Id, pSubject);
      
      switch (Id)
      {
        case SP_PG_L_SURFACE_TREND:
          mpIconSurfaceTrend->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_L_SURFACE_LEVEL:
          mpDpSurfaceLevel.Attach(pSubject);
          mpNQSurfaceLevel->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_L_UPPER_LEVEL:
          mpDpUpperLevel.Attach(pSubject);
          mpNQUpperLevel->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_L_LOWER_LEVEL:
          mpDpLowerLevel.Attach(pSubject);
          mpNQLowerLevel->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_L_FLOW:
          mpNQFlow->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PG_L_PIT_DEPTH:
          mpDpPitDepth.Attach(pSubject);
          break;
        case SP_PG_L_MIN_VARIATION_LEVEL:
          mpDpMinVariationLevel.Attach(pSubject);
          break;
        case SP_PG_L_MAX_VARIATION_LEVEL:
          mpDpMaxVariationLevel.Attach(pSubject);
          break;
      }
            
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void PitLevelGraphic::ConnectToSubjects(void)
    {
      AbstractPitGraphic::ConnectToSubjects();

      mpDpSurfaceLevel->Subscribe(this);
      mpDpUpperLevel->Subscribe(this);
      mpDpLowerLevel->Subscribe(this);
      mpDpPitDepth->Subscribe(this);
      mpDpMinVariationLevel->Subscribe(this);
      mpDpMaxVariationLevel->Subscribe(this);
      
      mpIconSurfaceTrend->ConnectToSubjects();
      mpNQSurfaceLevel->ConnectToSubjects();
      mpNQUpperLevel->ConnectToSubjects();
      mpNQLowerLevel->ConnectToSubjects();
      mpNQFlow->ConnectToSubjects();
   }


  } // namespace display
} // namespace mpc


