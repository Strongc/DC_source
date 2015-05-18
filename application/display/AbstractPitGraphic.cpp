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
/* CLASS NAME       : AbstractPitGraphic                                    */
/*                                                                          */
/* FILE NAME        : AbstractPitGraphic.cpp                                */
/*                                                                          */
/* CREATED DATE     : 2007-08-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*  Shows a pit with 1-6 animated pumps.                                    */
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
#include "AbstractPitGraphic.h"
#include "MPCFonts.h"

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    DEFINES
    *****************************************************************************/
    // sizes and offsets in pixels
    #define HALF_NQ_HEIGHT    7
    #define HALF_WAVE_HEIGHT  2
    #define ANIMATION_SPEED_QUICK 100 //ms delay
    #define ANIMATION_SPEED_SLOW  300 //ms delay 

    /*****************************************************************************
    EXTERNS
    *****************************************************************************/
                                                              // width,height
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitTop2Pumps;  // 240, 12 
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitTop4Pumps;  // 240, 12 
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitTop6Pumps;  // 240, 12
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitBottom;
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmVerticalFlowAnimation;
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmMixerAnimation;
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLevelSensor;//   6, 35 
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLevel;      // 160,  1
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLevel2;     // 160,  1

    //release 2 bitmaps...
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitPumpDisabled;// 17, 22
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitInlet;      //  42, 12
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPit2Pumps;     //  64, 29
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPit3Pumps;     // 140, 29
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPit4Pumps;     // 140, 29
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPit5Pumps;     // 140, 29
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPit6Pumps;     // 140, 29
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmHorisontalFlowAnimation;


    /*****************************************************************************
    * Function...: AbstractPitGraphic
    * DESCRIPTION: Constructor
    *****************************************************************************/
    AbstractPitGraphic::AbstractPitGraphic(Component* pParent) : ObserverGroup(pParent)
    {
      mUseRelease2layout = true;
      #ifdef USE_TFT_COLOURS
      mUseRelease2layout = false;
      #endif

      mHideAllPumps = false;

      // create Images and Animations
      if (mUseRelease2layout)
      { 
        for (int i = 0; i < NO_OF_PUMPS; i++)
        {
          mpImgPitPumpManifold[i] = new Image(this);
          mpImgPitPumpManifold[i]->SetTransparent();

          mpImgPitPumpDisabled[i] = new Image(this);
          mpImgPitPumpDisabled[i]->SetBitmap(&bmPitPumpDisabled);
          mpImgPitPumpDisabled[i]->SetTransparent();

          mpAniFlowPump[i] = new AnimatedImage(this, false);
          mpAniFlowPump[i]->SetSize(13, 4);
          mpAniFlowPump[i]->SetBitmap(&bmHorisontalFlowAnimation);
        }
        mpImgPitPumpManifold[0]->SetBitmap(&bmPit1Pump);
        mpImgPitPumpManifold[1]->SetBitmap(&bmPit2Pumps);
        mpImgPitPumpManifold[2]->SetBitmap(&bmPit3Pumps);
        mpImgPitPumpManifold[3]->SetBitmap(&bmPit4Pumps);
        mpImgPitPumpManifold[4]->SetBitmap(&bmPit5Pumps);
        mpImgPitPumpManifold[5]->SetBitmap(&bmPit6Pumps);
      }
      else
      { 
        for (int i = 0; i < NO_OF_PUMPS; i++)
        {
          mpPump[i] = new AnimatedPump(this, i + 1);
        }
      }
      
      for (int i = 0; i < NO_OF_PIT_WIDTHS; i++)
      {
        mpImgPitTop[i] = new Image(this);
        mpImgPitTop[i]->SetTransparent();
        mpImgPitTop[i]->SetVisible(i == 0);
      }

      mpImgPitTop[0]->SetBitmap(&bmPitTop2Pumps);
      mpImgPitTop[1]->SetBitmap(&bmPitTop4Pumps);
      mpImgPitTop[2]->SetBitmap(&bmPitTop6Pumps);
       
      mpImgPitLeft = new Image(this);
      mpImgPitRight = new Image(this);
      mpImgPitBottom = new Image(this);
      mpImgPitInlet = new Image(this);
      mpImgWater = new Image(this);
      mpImgPitUpperLevel = new Image(this);
      mpImgPitLowerLevel = new Image(this);
      mpImgPitMinVariationLevel = new Image(this);
      mpImgPitMaxVariationLevel = new Image(this);

      mpImgPitLeft->SetBitmap(&bmPitLeft);
      mpImgPitRight->SetBitmap(&bmPitRight);
      mpImgPitBottom->SetBitmap(&bmPitBottom);
      mpImgPitInlet->SetBitmap(&bmPitInlet);
      mpImgWater->SetBitmap(&bmPitWater);
      mpImgPitUpperLevel->SetBitmap(&bmPitLevel);
      mpImgPitLowerLevel->SetBitmap(&bmPitLevel);
      mpImgPitMinVariationLevel->SetBitmap(&bmPitLevel2);
      mpImgPitMaxVariationLevel->SetBitmap(&bmPitLevel2);

      mpImgPitLeft->SetTransparent();
      mpImgPitRight->SetTransparent();
      mpImgPitInlet->SetTransparent();
      mpImgPitUpperLevel->SetTransparent();
      mpImgPitLowerLevel->SetTransparent();
      mpImgPitMinVariationLevel->SetTransparent();
      mpImgPitMaxVariationLevel->SetTransparent();

      mpImgWater->SetAlign(GUI_TA_LEFT | GUI_TA_TOP);
      mpImgWater->SetTransparent(!mUseRelease2layout);

      mpAniFlowPipe = new AnimatedImage(this, true);
      if (mUseRelease2layout)
      {
        mpAniFlowPipe->SetSize(5, 20);
      }
      else
      {
        mpAniFlowPipe->SetSize(3, 20);
      }

      mpAniFlowPipe->SetBitmap(&bmVerticalFlowAnimation);
      mpAniFlowPipe->SetNumberOfStoppedImages(1);

      mpAniMixer = new AnimatedImage(this, false);
      if (mUseRelease2layout)
      {
        mpAniMixer->SetSize(26, 22);
      }
      else
      {
        mpAniMixer->SetSize(18, 20);
        mpAniMixer->SetSpeed(500, 300);
        mpAniMixer->SetBackgroundBitmap(&bmPitWater);
      }
      mpAniMixer->SetBitmap(&bmMixerAnimation);
      mpAniMixer->SetTransparent(mUseRelease2layout);


      mpAniFlowPipe->SetVisible();
      mpAniMixer->SetVisible(false);

      //create NumberQuantity
      mpNQInletFlow = new NumberQuantity(this);
      mpNQInletFlow->SetNumberOfDigits(3);
      mpNQInletFlow->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQInletFlow->SetAlign(GUI_TA_LEFT);
      mpNQInletFlow->SetWidth(NQ_WIDTH);
      mpNQInletFlow->SetHeight(NQ_HEIGHT);
      mpNQInletFlow->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      
      //set visibility
      mpImgPitLeft->SetVisible();
      mpImgPitRight->SetVisible();
      if (!mUseRelease2layout)
      {
        mpImgPitBottom->SetVisible();
      }
      mpImgWater->SetVisible(false);
      mpImgPitUpperLevel->SetVisible(false);
      mpImgPitLowerLevel->SetVisible(false);
      mpImgPitMinVariationLevel->SetVisible(false);
      mpImgPitMaxVariationLevel->SetVisible(false);
      mpImgPitInlet->SetVisible(false);
      mpNQInletFlow->SetVisible(false);

      if (mUseRelease2layout)
      {
        for (int i = 0; i < NO_OF_PUMPS; i++)
        {
          mpImgPitPumpManifold[i]->SetVisible(i == 0);
          mpImgPitPumpDisabled[i]->SetVisible(false);
          mpAniFlowPump[i]->SetVisible(i == 0);
        }
      }

      for (int i = 0; i < NO_OF_PUMPS; i++)
      {
        mpDpPumpOperationMode[i].SetUpdated();
      }
      
      mpDpNoOfPumpsRunning.SetUpdated();
      mpDpNoOfPumpsInstalled.SetUpdated();
      mpDpMixerOperationMode.SetUpdated();
      mpDpSensorType.SetUpdated();
      mpDpSimulatorEnabled.SetUpdated();

      AddChild(mpImgPitLeft);

      for (int i = 0; i < NO_OF_PIT_WIDTHS; i++)
      {
        AddChild(mpImgPitTop[i]);
      }

      AddChild(mpImgWater);
      AddChild(mpImgPitInlet);
      AddChild(mpImgPitBottom);
      AddChild(mpImgPitRight);
      AddChild(mpImgPitMinVariationLevel);
      AddChild(mpImgPitMaxVariationLevel);
      AddChild(mpImgPitUpperLevel);
      AddChild(mpImgPitLowerLevel);

      if (mUseRelease2layout)
      {
        // first add all manifold bitmaps to use them as background layer
        for (int i = 0; i < NO_OF_PUMPS; i++)
        {
          AddChild(mpImgPitPumpManifold[i]);
        }
      
        // then add out-of-operation symbols and animations
        for (int i = 0; i < NO_OF_PUMPS; i++)
        {
          AddChild(mpImgPitPumpDisabled[i]);
          AddChild(mpAniFlowPump[i]);
        }
      }
      else
      {
        for (int i = 0; i < NO_OF_PUMPS; i++)
        {
          AddChild(mpPump[i]);
        }
      }
      AddChild(mpAniFlowPipe);
      AddChild(mpAniMixer);
      AddChild(mpNQInletFlow);

    }

    /*****************************************************************************
    * Function...: ~AbstractPitGraphic
    * DESCRIPTION: Dectructor
    *****************************************************************************/
    AbstractPitGraphic::~AbstractPitGraphic()
    {
    }

    /*****************************************************************************
    * Function...: CalculateClientAreas
    * DESCRIPTION: applies large x-offset for narrow pits to center the pit
    *****************************************************************************/
    void AbstractPitGraphic::CalculateClientAreas()
    {
      U8 pumps = mpDpNoOfPumpsInstalled->GetValue();
      if (mHideAllPumps)
      {// then show a empty pit with a with of a two pump pit
        pumps = 2;
      }

      U8 x_offset = (pumps < 3 ? 30 : 0);

      CalculateClientAreas(pumps, x_offset);
    }

    /*****************************************************************************
    * Function...: CalculateClientAreas
    * DESCRIPTION: Set ClientArea of images, animations and Flow NQ
    *****************************************************************************/
    void AbstractPitGraphic::CalculateClientAreas(U8 numberOfInstalledPumps, U8 xOffset)
    {
      int x1, y1, x2, y2;

      for (int i = 0; i < NO_OF_PUMPS; i++)
      {
        if (mUseRelease2layout)
        {
          if (i < numberOfInstalledPumps)
          {
            mpImgPitPumpManifold[i]->SetVisible((numberOfInstalledPumps == (i + 1)));
            mpAniFlowPump[i]->SetVisible();
          }
          else
          {
            mpImgPitPumpManifold[i]->SetVisible(false);
            mpAniFlowPump[i]->SetVisible(false);
            mpImgPitPumpDisabled[i]->SetVisible(false);
          }
        }
        else
        {
          mpPump[i]->SetVisible(i < numberOfInstalledPumps);
        }
      }

      U8 pit_width_index = GetPitWidthIndex(numberOfInstalledPumps);

      // The '-|' part of PitTopImage is 25 pixels wide
      x1 = xOffset + 25;
      y1 = 0;
      x2 = x1 + mpNQInletFlow->GetWidth() - 1;
      y2 = y1 + mpNQInletFlow->GetHeight() - 1;
      mpNQInletFlow->SetClientArea(x1, y1, x2, y2);

      for (int i = 0; i < NO_OF_PIT_WIDTHS; i++)
      {
        x1 = 0;
        y1 = NQ_HEIGHT;
        x2 = 239;
        y2 = y1 + PIT_TOP_HEIGHT - 1;
        mpImgPitTop[i]->SetClientArea(x1, y1, x2, y2);
        mpImgPitTop[i]->SetVisible(i == pit_width_index);
      }

      //draw ImgPitInlet on top of ImgPitTop
      x1 = xOffset + NQ_WIDTH + X_LEVEL_INDENT - 20;
      y1 = NQ_HEIGHT + 4;
      x2 = x1 + bmPitInlet.XSize - 1;
      y2 = y1 + bmPitInlet.YSize - 1;
      mpImgPitInlet->SetClientArea(x1, y1, x2, y2);

      x1 = xOffset + NQ_WIDTH + X_LEVEL_INDENT + (mUseRelease2layout ? -1 : 0);
      y1 = NQ_HEIGHT + PIT_TOP_HEIGHT;
      x2 = x1 + bmPitLeft.XSize - 1;
      y2 = y1 + bmPitLeft.YSize - 1;
      mpImgPitLeft->SetClientArea(x1, y1, x2, y2);

      x1 = mpImgPitLeft->GetChildPosX() + bmPitLeft.XSize + GetInternalPitWidth() - bmPitRight.XSize + 2;
      y1 = mpImgPitLeft->GetChildPosY();
      x2 = x1 + bmPitRight.XSize - 1;
      y2 = y1 + bmPitRight.YSize - 1;
      mpImgPitRight->SetClientArea(x1, y1, x2, y2);

      if (!mUseRelease2layout)
      {
        x1 = mpImgPitLeft->GetChildPosX() + bmPitLeft.XSize;
        y1 = mpImgPitLeft->GetChildPosY() + bmPitLeft.YSize - bmPitBottom.YSize;
        x2 = mpImgPitRight->GetChildPosX() - 1;
        y2 = y1 + bmPitBottom.YSize - 1;
        mpImgPitBottom->SetClientArea(x1, y1, x2, y2);
      }

      for (int i = 0; i < NO_OF_PUMPS; i++)
      {
        if (mUseRelease2layout)
        {
          x1 = mpImgPitLeft->GetChildPosX() + bmPitLeft.XSize;
          y1 = mpImgPitLeft->GetChildPosY() + bmPitLeft.YSize - mpImgPitPumpManifold[i]->GetHeight();
          x2 = x1 + mpImgPitPumpManifold[i]->GetWidth() - 1;
          y2 = y1 + mpImgPitPumpManifold[i]->GetHeight() - 1;
          mpImgPitPumpManifold[i]->SetClientArea(x1, y1, x2, y2);

          // The coordinates (4,20) are relative to topleft corner of PitPump.bmp
          x1 = mpImgPitPumpManifold[i]->GetChildPosX() + 4 + (i*19); 
          y1 = mpImgPitPumpManifold[i]->GetChildPosY() + 20;
          x2 = x1 + mpAniFlowPump[i]->GetWidth() - 1;
          y2 = y1 + mpAniFlowPump[i]->GetHeight() - 1;
          mpAniFlowPump[i]->SetClientArea(x1, y1, x2, y2);

          x1 = mpImgPitPumpManifold[i]->GetChildPosX() + (i*19);
          y1 = mpImgPitPumpManifold[i]->GetChildPosY() + 1;
          x2 = x1 + bmPitPumpDisabled.XSize - 1;
          y2 = y1 + bmPitPumpDisabled.YSize - 1;
          mpImgPitPumpDisabled[i]->SetClientArea(x1, y1, x2, y2);
        }
        else
        {
          if (!mHideAllPumps)
          {
            U8 x_offset = (numberOfInstalledPumps % 2 == 0 ? i : i + 1) * (AnimatedPump::WIDTH + 2);
            x1 = mpImgPitLeft->GetChildPosX() + bmPitLeft.XSize + bmPitLevelSensor.XSize + bmMixerAnimation.XSize + x_offset + 3;
            y1 = mpImgPitLeft->GetChildPosY() + bmPitLeft.YSize - bmPitBottom.YSize - AnimatedPump::HEIGHT - 2;
            x2 = x1 + AnimatedPump::WIDTH - 1;
            y2 = y1 + AnimatedPump::HEIGHT - 1;
            mpPump[i]->SetClientArea(x1, y1, x2, y2);
          }
        }
      }

      if (mUseRelease2layout)
      {
        // The coordinates (0,34) are relative to topleft corner of PitRight.bmp
        x1 = mpImgPitRight->GetChildPosX();
        y1 = mpImgPitRight->GetChildPosY() + 34;
        x2 = x1 + mpAniFlowPipe->GetWidth() - 1;
        y2 = y1 + mpAniFlowPipe->GetHeight() - 1;
      }
      else
      {        
        x1 = mpImgPitRight->GetChildPosX() + 5;
        y1 = mpImgPitRight->GetChildPosY() + 34;
        x2 = x1 + mpAniFlowPipe->GetWidth() - 1;
        y2 = y1 + mpAniFlowPipe->GetHeight() - 1;
      }
      mpAniFlowPipe->SetClientArea(x1, y1, x2, y2);

      
      if (mUseRelease2layout)
      {
        // The coordinates (-16,-5) are relative to topleft corner of PitRight.bmp
        x1 = mpImgPitRight->GetChildPosX() - 16;
        y1 = mpImgPitPumpManifold[0]->GetChildPosY() - 5;
        x2 = x1 + mpAniMixer->GetWidth() - 1;
        y2 = y1 + mpAniMixer->GetHeight() - 1;
      }
      else
      {
        x1 = mpImgPitLeft->GetChildPosX() + bmPitLeft.XSize;
        y1 = mpImgPitBottom->GetChildPosY() - AnimatedPump::HEIGHT;
        x2 = x1 + mpAniMixer->GetWidth() - 1;
        y2 = y1 + mpAniMixer->GetHeight() - 1;
      }
      mpAniMixer->SetClientArea(x1, y1, x2, y2);

      InvalidatePitContents();

      for (int i = 0; i < NO_OF_PIT_WIDTHS; i++)
      {
        mpImgPitTop[i]->Invalidate();
      }

      Invalidate();
    }



    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION: 
    *****************************************************************************/
    void AbstractPitGraphic::Run()
    {
      bool any_pump_changed = false;
      for (int i = 0; i < NO_OF_PUMPS; i++)
      {
        if (mpDpPumpOperationMode[i].IsUpdated()
          || mpDpPumpAlarmState[i].IsUpdated())
        {
          any_pump_changed = true;
        }
      }

      if (any_pump_changed)
      {
        UpdatePumpAnimations();       
      }

      // flow animation in vertical pipe
      if (mpDpNoOfPumpsRunning.IsUpdated())
      {
        if (mpDpNoOfPumpsRunning->GetValue() > 1)
        {
          mpAniFlowPipe->SetSpeed(ANIMATION_SPEED_QUICK, 0);
          mpAniFlowPipe->Start();
        }
        else if (mpDpNoOfPumpsRunning->GetValue() == 1)
        {
          mpAniFlowPipe->SetSpeed(ANIMATION_SPEED_SLOW, 0);
          mpAniFlowPipe->Start();
        }
        else
        {
          mpAniFlowPipe->Stop();
        }
      }

      // mixer animation
      if (mpDpMixerOperationMode.IsUpdated())
      {
        UpdateMixerAnimation();
      }

      // inlet image in simulation mode
      if (mpDpSimulatorEnabled.IsValid() && mpDpSimulatorEnabled.IsUpdated())
      {
        bool inlet_is_simulated = mpDpSimulatorEnabled->GetValue();
        mpNQInletFlow->SetVisible(inlet_is_simulated);
        mpImgPitInlet->SetVisible(inlet_is_simulated);

        Invalidate();
      }
    }

    /*****************************************************************************
    * Function...: UpdatePumpAnimations
    * DESCRIPTION: 
    *****************************************************************************/
    void AbstractPitGraphic::UpdatePumpAnimations()
    {
      for (int i = 0; i < NO_OF_PUMPS; i++)
      {
        if (mUseRelease2layout)
        {
          bool pump_disabled = (mpDpPumpOperationMode[i]->GetValue() == ACTUAL_OPERATION_MODE_DISABLED 
            || mpDpPumpOperationMode[i]->GetQuality() == DP_NOT_AVAILABLE);

          mpImgPitPumpDisabled[i]->SetVisible(pump_disabled);

          if (mpDpPumpOperationMode[i]->GetValue() == ACTUAL_OPERATION_MODE_STARTED
            && mpDpPumpOperationMode[i]->GetQuality() == DP_AVAILABLE)
          {
            mpAniFlowPump[i]->Start();
          }
          else
          {
            // don't stop a animation if it is not visible as this will make it visible
            if (mpAniFlowPump[i]->IsVisible())
            {
              mpAniFlowPump[i]->Stop();
            }
          }

          for (int j = 0; j < NO_OF_PUMPS; j++)
          {
            mpImgPitPumpManifold[j]->Invalidate();
            mpImgPitPumpDisabled[j]->Invalidate();
            mpAniFlowPump[j]->Invalidate();
          }
          mpAniMixer->Invalidate();
        }
        else
        {
          mpPump[i]->ChangeMode(mpDpPumpOperationMode[i]->GetValue(), mpDpPumpAlarmState[i]->GetValue());
        }
      }
    }

    /*****************************************************************************
    * Function...: UpdateMixerAnimation
    * DESCRIPTION: 
    *****************************************************************************/
    void AbstractPitGraphic::UpdateMixerAnimation()
    {
      bool mixer_installed = (mpDpMixerOperationMode->GetValue() != ACTUAL_OPERATION_MODE_NOT_INSTALLED) 
          && (mpDpMixerOperationMode->GetValue() != ACTUAL_OPERATION_MODE_DISABLED)
          && (mpDpMixerOperationMode->GetQuality() == DP_AVAILABLE);

      bool mixer_started = (mpDpMixerOperationMode->GetValue() == ACTUAL_OPERATION_MODE_STARTED) 
        && mixer_installed;

      if (mixer_started)
      {
        mpAniMixer->Start();
      }
      else
      {
        mpAniMixer->Stop();
      }

      mpAniMixer->SetVisible(mixer_installed);

      InvalidatePitContents();
    }

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION: Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool AbstractPitGraphic::Redraw()
    {
      return Group::Redraw();
    }

    /*****************************************************************************
    * Function...: IsValid
    * DESCRIPTION: 
    *****************************************************************************/
    bool AbstractPitGraphic::IsValid()
    {
      return mValid;
    }

    /*****************************************************************************
    * Function...: InvalidatePitContents
    * DESCRIPTION: 
    *****************************************************************************/
    void AbstractPitGraphic::InvalidatePitContents()
    {      
      mpImgPitInlet->Invalidate();
      mpImgWater->Invalidate();
      mpAniMixer->Invalidate();
      mpImgPitLeft->Invalidate();
      mpImgPitRight->Invalidate();
      mpAniFlowPipe->Invalidate();
      mpImgPitBottom->Invalidate();

      for (int i = 0; i < NO_OF_PUMPS; i++)
      {
        if (mUseRelease2layout)
        {
          mpImgPitPumpManifold[i]->Invalidate();
          mpImgPitPumpDisabled[i]->Invalidate();
          mpAniFlowPump[i]->Invalidate();
        }
        else
        {
          mpPump[i]->Invalidate();
        }
      }    
    }

    /*****************************************************************************
    * Function...: ChangeWaterLevelClientArea
    * DESCRIPTION: Updates client area of the water image its position is changed more
    * than some number of pixels
    *****************************************************************************/
    void AbstractPitGraphic::ChangeWaterLevelClientArea(int x1, int y1, int x2, int y2)
    {
      const int treshold = 3;
      
      int previous_x1 = mpImgWater->GetChildPosX();
      int previous_y1 = mpImgWater->GetChildPosY();
      int previous_x2 = previous_x1 + mpImgWater->GetWidth() - 1;

      // it is sufficient to compare treshold for y1, since this is the parameter that changes.
      if (abs(previous_y1 - y1) >= treshold
        || previous_x1 != x1
        || previous_x2 != x2)
      {
        mpImgWater->SetClientArea(x1, y1, x2, y2);
      }

    }

    /*****************************************************************************
    * Function...: GetInternalPitWidth
    * DESCRIPTION: returns integer
    *****************************************************************************/
    U8 AbstractPitGraphic::GetInternalPitWidth()
    {
      U8 width = 0;
      U8 number_of_pumps = mpDpNoOfPumpsInstalled->GetValue();

      if (mUseRelease2layout)
      {
        width = mpImgPitPumpManifold[number_of_pumps - 1]->GetWidth() + bmPitRight.XSize - 2;
      }
      else
      {
        U8 number_of_pump_slots = 6;

        if (number_of_pumps < 3)
        {
          number_of_pump_slots = 2;
        }
        else if (number_of_pumps < 5)
        {
          number_of_pump_slots = 4;
        }

        if (mHideAllPumps)
        {
          number_of_pump_slots = 2;
        }

        width = number_of_pump_slots * (AnimatedPump::WIDTH + 2) + bmPitLevelSensor.XSize + bmMixerAnimation.XSize + 12;
      }
      return width;
    }

    /*****************************************************************************
    * Function...: GetPitWidthIndex
    * DESCRIPTION: returns index for the pitTop image array, to select between wide and narrow pits
    *****************************************************************************/
    U8 AbstractPitGraphic::GetPitWidthIndex(U8 numberOfInstalledPumps)
    {
      return (numberOfInstalledPumps > 4 ? 2 : (numberOfInstalledPumps > 2 ? 1 : 0));
    }


    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void AbstractPitGraphic::Update(Subject* pSubject)
    {
      for (int i = 0; i < NO_OF_PUMPS; i++)
      {
        mpDpPumpOperationMode[i].Update(pSubject);
        mpDpPumpAlarmState[i].Update(pSubject);
      }

      mpDpNoOfPumpsInstalled.Update(pSubject);
      mpDpNoOfPumpsRunning.Update(pSubject);
      mpDpMixerOperationMode.Update(pSubject);
      mpDpSensorType.Update(pSubject);
      mpDpSimulatorEnabled.Update(pSubject);
    }

    /* --------------------------------------------------
    * Called if subscription shall be cancelled
    * --------------------------------------------------*/
    void AbstractPitGraphic::SubscribtionCancelled(Subject* pSubject)
    {
      for (int i = 0; i < NO_OF_PUMPS; i++)
      {
        mpDpPumpOperationMode[i].Detach(pSubject);
        mpDpPumpAlarmState[i].Detach(pSubject);
      }

      mpDpNoOfPumpsInstalled.Detach(pSubject);
      mpDpNoOfPumpsRunning.Detach(pSubject);
      mpDpMixerOperationMode.Detach(pSubject);
      mpDpSensorType.Detach(pSubject);
      mpDpSimulatorEnabled.Detach(pSubject);
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void AbstractPitGraphic::SetSubjectPointer(int id, Subject* pSubject)
    {
      switch (id)
      {
        case SP_PG_FSW_OPERATION_MODE_ACTUAL_PUMP_1:
        case SP_PG_L_OPERATION_MODE_ACTUAL_PUMP_1:
        case SP_PGMIX_OPERATION_MODE_ACTUAL_PUMP_1:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_1:
          mpDpPumpOperationMode[0].Attach(pSubject);
          break;
        case SP_PG_FSW_OPERATION_MODE_ACTUAL_PUMP_2:
        case SP_PG_L_OPERATION_MODE_ACTUAL_PUMP_2:
        case SP_PGMIX_OPERATION_MODE_ACTUAL_PUMP_2:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_2:
          mpDpPumpOperationMode[1].Attach(pSubject);
          break;
        case SP_PG_FSW_OPERATION_MODE_ACTUAL_PUMP_3:
        case SP_PG_L_OPERATION_MODE_ACTUAL_PUMP_3:
        case SP_PGMIX_OPERATION_MODE_ACTUAL_PUMP_3:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_3:
          mpDpPumpOperationMode[2].Attach(pSubject);
          break;
        case SP_PG_FSW_OPERATION_MODE_ACTUAL_PUMP_4:
        case SP_PG_L_OPERATION_MODE_ACTUAL_PUMP_4:
        case SP_PGMIX_OPERATION_MODE_ACTUAL_PUMP_4:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_4:
          mpDpPumpOperationMode[3].Attach(pSubject);
          break;
        case SP_PG_FSW_OPERATION_MODE_ACTUAL_PUMP_5:
        case SP_PG_L_OPERATION_MODE_ACTUAL_PUMP_5:
        case SP_PGMIX_OPERATION_MODE_ACTUAL_PUMP_5:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_5:
          mpDpPumpOperationMode[4].Attach(pSubject);
          break;
        case SP_PG_FSW_OPERATION_MODE_ACTUAL_PUMP_6:
        case SP_PG_L_OPERATION_MODE_ACTUAL_PUMP_6:
        case SP_PGMIX_OPERATION_MODE_ACTUAL_PUMP_6:
        case SP_PG_OPERATION_MODE_ACTUAL_PUMP_6:
          mpDpPumpOperationMode[5].Attach(pSubject);
          break;
        case SP_PG_FSW_ALARM_STATE_PUMP_1:
        case SP_PG_L_ALARM_STATE_PUMP_1:
        case SP_PGMIX_ALARM_STATE_PUMP_1:
        case SP_PG_ALARM_STATE_PUMP_1:
          mpDpPumpAlarmState[0].Attach(pSubject);
          break;
        case SP_PG_FSW_ALARM_STATE_PUMP_2:
        case SP_PG_L_ALARM_STATE_PUMP_2:
        case SP_PGMIX_ALARM_STATE_PUMP_2:
        case SP_PG_ALARM_STATE_PUMP_2:
          mpDpPumpAlarmState[1].Attach(pSubject);
          break;
        case SP_PG_FSW_ALARM_STATE_PUMP_3:
        case SP_PG_L_ALARM_STATE_PUMP_3:
        case SP_PGMIX_ALARM_STATE_PUMP_3:
        case SP_PG_ALARM_STATE_PUMP_3:
          mpDpPumpAlarmState[2].Attach(pSubject);
          break;
        case SP_PG_FSW_ALARM_STATE_PUMP_4:
        case SP_PG_L_ALARM_STATE_PUMP_4:
        case SP_PGMIX_ALARM_STATE_PUMP_4:
        case SP_PG_ALARM_STATE_PUMP_4:
          mpDpPumpAlarmState[3].Attach(pSubject);
          break;
        case SP_PG_FSW_ALARM_STATE_PUMP_5:
        case SP_PG_L_ALARM_STATE_PUMP_5:
        case SP_PGMIX_ALARM_STATE_PUMP_5:
        case SP_PG_ALARM_STATE_PUMP_5:
          mpDpPumpAlarmState[4].Attach(pSubject);
          break;
        case SP_PG_FSW_ALARM_STATE_PUMP_6:
        case SP_PG_L_ALARM_STATE_PUMP_6:
        case SP_PGMIX_ALARM_STATE_PUMP_6:
        case SP_PG_ALARM_STATE_PUMP_6:
          mpDpPumpAlarmState[5].Attach(pSubject);
          break;
        case SP_PG_FSW_NO_OF_PUMPS_INSTALLED:
        case SP_PG_L_NO_OF_PUMPS_INSTALLED:
        case SP_PGMIX_NO_OF_PUMPS_INSTALLED:
        case SP_PG_NO_OF_PUMPS_INSTALLED:
          mpDpNoOfPumpsInstalled.Attach(pSubject);
          break;
        case SP_PG_FSW_NO_OF_RUNNING_PUMPS:
        case SP_PG_L_NO_OF_RUNNING_PUMPS:
        case SP_PGMIX_NO_OF_RUNNING_PUMPS:
        case SP_PG_NO_OF_RUNNING_PUMPS:
          mpDpNoOfPumpsRunning.Attach(pSubject);
          break;
        case SP_PG_FSW_OPERATION_MODE_MIXER:
        case SP_PG_L_OPERATION_MODE_MIXER:
        case SP_PGMIX_OPERATION_MODE_MIXER:
        case SP_PG_OPERATION_MODE_MIXER:
          mpDpMixerOperationMode.Attach(pSubject);
          break;
        case SP_PG_FSW_SENSOR_TYPE:
        case SP_PG_L_SENSOR_TYPE:
        case SP_PGMIX_SENSOR_TYPE:
        case SP_PG_SENSOR_TYPE:
          mpDpSensorType.Attach(pSubject);
          break;
        case SP_PG_FSW_SIM_ENABLED:
        case SP_PG_L_SIM_ENABLED:
        case SP_PG_SIM_ENABLED:
          mpDpSimulatorEnabled.Attach(pSubject);
          break;
        case SP_PG_FSW_SIM_INLET_FLOW:
        case SP_PG_L_SIM_INLET_FLOW:
        case SP_PG_SIM_INLET_FLOW:
          mpNQInletFlow->SetSubjectPointer(id, pSubject);
          break;
      }
      
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void AbstractPitGraphic::ConnectToSubjects(void)
    {      
      for (int i = 0; i < NO_OF_PUMPS; i++)
      {
        mpDpPumpOperationMode[i]->Subscribe(this);
        mpDpPumpAlarmState[i]->Subscribe(this);
      }

      mpDpNoOfPumpsInstalled->Subscribe(this);
      mpDpNoOfPumpsRunning->Subscribe(this);
      mpDpMixerOperationMode->Subscribe(this);
      mpDpSensorType->Subscribe(this);
      mpDpSimulatorEnabled.Subscribe(this);

      if (mpNQInletFlow->GetSubject() != NULL)
      {
        mpNQInletFlow->ConnectToSubjects();
      }
   }

  } // namespace display
} // namespace mpc


