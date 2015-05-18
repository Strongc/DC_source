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
/* CLASS NAME       : PitStatus                                             */
/*                                                                          */
/* FILE NAME        : PitStatus.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 2007-06-29                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*  Shows at Pit with two animated pumps, a variable surface level and      */
/*  two surveillance levels (upper and lower). Number-Quantities of all     */
/*  levels together with the pipe flow is shown when available.             */
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
#include "PitStatus.h"
#include "MPCFonts.h"
#include <display_task.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
// sizes and offsets in pixels
#define X_OFFSET   0
#define Y_OFFSET   0
#define NQ_WIDTH  60 
#define NQ_HEIGHT 15
#define HALF_NQ_HEIGHT 7
#define Y_LEVEL_SPAN   76  
#define Y_LEVEL_OFFSET 4
#define Y_TRUNCATE 7 // truncate top vertical position of level NQ
#define HALF_WAVE_HEIGHT 2


#define ANIMATION_SPEED_QUICK 100 //ms delay
#define ANIMATION_SPEED_SLOW  300 //ms delay 

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
EXTERNS
*****************************************************************************/
                                                          // width,height
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitTop;        // 142, 12 
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLeft;       //   2,117
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitRight;      //  13,117
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitPumps;      //  64, 29
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitSinglePump; //  64, 29
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLevel;      //  75,  1
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitSurface;    //  75,  4
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLevelSensor;//   6, 35 
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitPumpDisabled;// 17, 22

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Function...: PitStatus
    * DESCRIPTION: Constructor
    *****************************************************************************/
    PitStatus::PitStatus(Component* pParent) : ObserverGroup(pParent) // Frame(true, true, pParent)
    {
      // create Images
      mpImgPitTop = new Image(this);
      mpImgPitLeft = new Image(this);
      mpImgPitRight = new Image(this);
      mpImgPitPumps = new Image(this);
      mpImgPitSinglePump = new Image(this);
      mpImgPitSurface = new Image(this);
      mpImgPitUpperLevel = new Image(this);
      mpImgPitLowerLevel = new Image(this);
      mpImgPitLevelSensor = new Image(this);
      mpImgPitPump1Disabled = new Image(this);
      mpImgPitPump2Disabled = new Image(this);

      mpImgPitTop->SetBitmap(&bmPitTop);
      mpImgPitLeft->SetBitmap(&bmPitLeft);
      mpImgPitRight->SetBitmap(&bmPitRight);
      mpImgPitPumps->SetBitmap(&bmPitPumps);
      mpImgPitSinglePump->SetBitmap(&bmPitSinglePump);

      mpImgPitSurface->SetBitmap(&bmPitSurface);
      mpImgPitUpperLevel->SetBitmap(&bmPitLevel);
      mpImgPitLowerLevel->SetBitmap(&bmPitLevel);
      mpImgPitLevelSensor->SetBitmap(&bmPitLevelSensor);
      mpImgPitPump1Disabled->SetBitmap(&bmPitPumpDisabled);
      mpImgPitPump2Disabled->SetBitmap(&bmPitPumpDisabled);
      
      mpImgPitRight->SetTransparent();
      mpImgPitPumps->SetTransparent();
      mpImgPitSinglePump->SetTransparent();
      mpImgPitSurface->SetTransparent();
      mpImgPitUpperLevel->SetTransparent();
      mpImgPitLowerLevel->SetTransparent();
      mpImgPitLevelSensor->SetTransparent();
      mpImgPitPump1Disabled->SetTransparent();
      mpImgPitPump2Disabled->SetTransparent();

      // create Animations
      mpAniFlowPump1 = new AnimatedPumpPipe(this);
      mpAniFlowPump2 = new AnimatedPumpPipe(this);
      mpAniFlowPipe  = new AnimatedPipe(this);
      mpAniMixer     = new AnimatedMixer(this);

      mpAniFlowPump1->SetVisible();
      mpAniFlowPump2->SetVisible(false);
      mpAniFlowPipe->SetVisible();
      mpAniMixer->SetVisible(false);

      mpAniFlowPump1->Stop();

      // create NQ
      mpNQSurfaceLevel = new NumberQuantity(this);
      mpNQUpperLevel = new NumberQuantity(this);   
      mpNQLowerLevel = new NumberQuantity(this);
      mpNQFlow = new NumberQuantity(this);

      mpNQSurfaceLevel->SetNumberOfDigits(3);
      mpNQSurfaceLevel->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQSurfaceLevel->SetAlign(GUI_TA_LEFT);
      mpNQSurfaceLevel->SetWidth( NQ_WIDTH );
      mpNQSurfaceLevel->SetHeight( NQ_HEIGHT );
      
      mpNQUpperLevel->SetNumberOfDigits(3);
      mpNQUpperLevel->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQUpperLevel->SetAlign(GUI_TA_RIGHT);
      mpNQUpperLevel->SetWidth( NQ_WIDTH );
      mpNQUpperLevel->SetHeight( NQ_HEIGHT );

      mpNQLowerLevel->SetNumberOfDigits(3);
      mpNQLowerLevel->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQLowerLevel->SetAlign(GUI_TA_RIGHT);
      mpNQLowerLevel->SetWidth( NQ_WIDTH );
      mpNQLowerLevel->SetHeight( NQ_HEIGHT );
      
      mpNQFlow->SetNumberOfDigits(3);
      mpNQFlow->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      mpNQFlow->SetAlign(GUI_TA_LEFT);
      mpNQFlow->SetWidth( NQ_WIDTH );
      mpNQFlow->SetHeight( NQ_HEIGHT );
      mpNQFlow->HideOnNeverAvailable();
      
      //calculate client areas
      SetStaticClientAreas();

      mpDpPump1OperationMode.SetUpdated();
      mpDpPump2OperationMode.SetUpdated();

      //set visibility
      mpImgPitTop->SetVisible();
      mpImgPitLeft->SetVisible();
      mpImgPitRight->SetVisible();
      mpImgPitPumps->SetVisible(false);

    
      mpImgPitSinglePump->SetVisible();
      mpImgPitLevelSensor->SetVisible(false);
      mpImgPitPump1Disabled->SetVisible(false);
      mpImgPitPump2Disabled->SetVisible(false);

      mpNQFlow->SetVisible();

      mpDpSurfaceLevel.SetUpdated();
      mpDpUpperLevel.SetUpdated();
      mpDpLowerLevel.SetUpdated();
      mpDpNoOfRunningPumps.SetUpdated();
      mpDpMixerOperationMode.SetUpdated();

      AddChild(mpImgPitLeft);
      AddChild(mpImgPitTop);
      AddChild(mpImgPitRight);
      AddChild(mpImgPitPumps);
      AddChild(mpImgPitSinglePump);
      AddChild(mpImgPitSurface);
      AddChild(mpImgPitUpperLevel);
      AddChild(mpImgPitLowerLevel);
      AddChild(mpImgPitLevelSensor);
      AddChild(mpImgPitPump1Disabled);
      AddChild(mpImgPitPump2Disabled);
      AddChild(mpAniFlowPump1);
      AddChild(mpAniFlowPump2);
      AddChild(mpAniFlowPipe);
      AddChild(mpAniMixer);
      AddChild(mpNQSurfaceLevel);
      AddChild(mpNQUpperLevel);
      AddChild(mpNQLowerLevel);
      AddChild(mpNQFlow);

    }

    /*****************************************************************************
    * Function...: ~PitStatus
    * DESCRIPTION: Dectructor
    *****************************************************************************/
    PitStatus::~PitStatus()
    {
      RemoveChild(mpImgPitTop);
      delete mpImgPitTop;
      RemoveChild(mpImgPitLeft);
      delete mpImgPitLeft;
      RemoveChild(mpImgPitRight);
      delete mpImgPitRight;
      RemoveChild(mpImgPitPumps);
      delete mpImgPitPumps;
      RemoveChild(mpImgPitSinglePump);
      delete mpImgPitSinglePump;
      RemoveChild(mpImgPitSurface);
      delete mpImgPitSurface;
      RemoveChild(mpImgPitUpperLevel);
      delete mpImgPitUpperLevel;
      RemoveChild(mpImgPitLowerLevel);
      delete mpImgPitLowerLevel;
      RemoveChild(mpImgPitLevelSensor);
      delete mpImgPitLevelSensor;
      RemoveChild(mpImgPitPump1Disabled);
      delete mpImgPitPump1Disabled;
      RemoveChild(mpImgPitPump2Disabled);
      delete mpImgPitPump2Disabled;    

      RemoveChild(mpAniFlowPump1);
      delete mpAniFlowPump1;
      RemoveChild(mpAniFlowPump2);
      delete mpAniFlowPump2;
      RemoveChild(mpAniFlowPipe);
      delete mpAniFlowPipe;
      RemoveChild(mpAniMixer);
      delete mpAniMixer;
      
      RemoveChild(mpNQSurfaceLevel);
      delete mpNQSurfaceLevel;
      RemoveChild(mpNQUpperLevel);
      delete mpNQUpperLevel;
      RemoveChild(mpNQLowerLevel);
      delete mpNQLowerLevel;
      RemoveChild(mpNQFlow);
      delete mpNQFlow;
      
    }

    /*****************************************************************************
    * Function...: SetStaticClientAreas
    * DESCRIPTION: Set ClientArea of images, animations and Flow NQ
    *****************************************************************************/
    void PitStatus::SetStaticClientAreas()
    {
      int x1, y1, x2, y2;

      // The '-|' part of PitTopImage is 25 pixels wide
      x1 = X_OFFSET + NQ_WIDTH - 25;
      y1 = Y_OFFSET + NQ_HEIGHT;
      x2 = x1 + bmPitTop.XSize - 1;
      y2 = y1 + bmPitTop.YSize - 1;
      mpImgPitTop->SetClientArea(x1, y1, x2, y2);

      x1 = X_OFFSET + NQ_WIDTH;
      y1 = Y_OFFSET + NQ_HEIGHT + bmPitTop.YSize;
      x2 = x1 + bmPitLeft.XSize - 1;
      y2 = y1 + bmPitLeft.YSize - 1;
      mpImgPitLeft->SetClientArea(x1, y1, x2, y2);

      x1 = mpImgPitLeft->GetChildPosX() + bmPitLeft.XSize + bmPitPumps.XSize;
      y1 = mpImgPitLeft->GetChildPosY();
      x2 = x1 + bmPitRight.XSize - 1;
      y2 = y1 + bmPitRight.YSize - 1;
      mpImgPitRight->SetClientArea(x1, y1, x2, y2);
      
      x1 = mpImgPitTop->GetChildPosX()+ bmPitTop.XSize - NQ_WIDTH;
      y1 = Y_OFFSET;
      x2 = x1 + mpNQFlow->GetWidth() - 1;
      y2 = y1 + mpNQFlow->GetHeight() - 1;
      mpNQFlow->SetClientArea(x1, y1, x2, y2);

      x1 = mpImgPitLeft->GetChildPosX() + bmPitLeft.XSize;
      y1 = mpImgPitLeft->GetChildPosY() + bmPitLeft.YSize - bmPitPumps.YSize;
      x2 = x1 + bmPitPumps.XSize - 1;
      y2 = y1 + bmPitPumps.YSize - 1;
      mpImgPitPumps->SetClientArea(x1, y1, x2, y2);
      mpImgPitSinglePump->SetClientArea(x1, y1, x2, y2);

      // The coordinates (4,20) are relative to topleft corner of PitPump.bmp
      x1 = mpImgPitPumps->GetChildPosX() + 4; 
      y1 = mpImgPitPumps->GetChildPosY() + 20;
      x2 = x1 + mpAniFlowPump1->GetWidth() - 1;
      y2 = y1 + mpAniFlowPump1->GetHeight() - 1;
      mpAniFlowPump1->SetClientArea(x1, y1, x2, y2);

      // The coordinates (23,20) are relative to topleft corner of PitPump.bmp
      x1 = mpImgPitPumps->GetChildPosX() + 23;
      y1 = mpImgPitPumps->GetChildPosY() + 20;
      x2 = x1 + mpAniFlowPump2->GetWidth() - 1;
      y2 = y1 + mpAniFlowPump2->GetHeight() - 1;
      mpAniFlowPump2->SetClientArea(x1, y1, x2, y2);

      // The coordinates (0,34) are relative to topleft corner of PitRight.bmp
      x1 = mpImgPitRight->GetChildPosX();
      y1 = mpImgPitRight->GetChildPosY() + 34;
      x2 = x1 + mpAniFlowPipe->GetWidth() - 1;
      y2 = y1 + mpAniFlowPipe->GetHeight() - 1;
      mpAniFlowPipe->SetClientArea(x1, y1, x2, y2);

      // The coordinates (40,-20) are relative to topleft corner of PitPump.bmp
      x1 = mpImgPitPumps->GetChildPosX() + 40;
      y1 = mpImgPitPumps->GetChildPosY() - 20;
      x2 = x1 + bmPitLevelSensor.XSize - 1;
      y2 = y1 + bmPitLevelSensor.YSize - 1;
      mpImgPitLevelSensor->SetClientArea(x1, y1, x2, y2);

      // The coordinates (0,1) are relative to topleft corner of PitPump.bmp
      x1 = mpImgPitPumps->GetChildPosX();
      y1 = mpImgPitPumps->GetChildPosY() + 1 ;
      x2 = x1 + bmPitPumpDisabled.XSize - 1;
      y2 = y1 + bmPitPumpDisabled.YSize - 1;
      mpImgPitPump1Disabled->SetClientArea(x1, y1, x2, y2);

      // The coordinates (19,1) are relative to topleft corner of PitPump.bmp
      x1 = mpImgPitPumps->GetChildPosX() + 19;
      y1 = mpImgPitPumps->GetChildPosY() + 1;
      x2 = x1 + bmPitPumpDisabled.XSize - 1;
      y2 = y1 + bmPitPumpDisabled.YSize - 1;
      mpImgPitPump2Disabled->SetClientArea(x1, y1, x2, y2);

      // The coordinates (48,-5) are relative to topleft corner of PitPump.bmp
      x1 = mpImgPitPumps->GetChildPosX() + 48;
      y1 = mpImgPitPumps->GetChildPosY() - 5;
      x2 = x1 + mpAniMixer->GetWidth() - 1;
      y2 = y1 + mpAniMixer->GetHeight() - 1;
      mpAniMixer->SetClientArea(x1, y1, x2, y2);

    }



    /*****************************************************************************
    * Function...: UpdateSurfaceLevelPosition
    * DESCRIPTION: Set ClientArea of SurfaceLevel waves and NQ
    *****************************************************************************/
    void PitStatus::UpdateSurfaceLevelPosition()
    {
      int x1, y1, x2, y2, level;

      //calculate and set Image location 
      if(mpDpSurfaceLevel->GetQuality() == DP_AVAILABLE)
      {        
        level = Y_LEVEL_SPAN - ((mpDpSurfaceLevel->GetValueAsPercent() * Y_LEVEL_SPAN) / 100) + Y_LEVEL_OFFSET - HALF_WAVE_HEIGHT;
        mpNQSurfaceLevel->SetVisible();
        mpImgPitSurface->SetVisible();
      }
      else
      {
        level = 0;
        mpNQSurfaceLevel->SetVisible(false);       
        mpImgPitSurface->SetVisible(false);
      }

      x1 = X_OFFSET + NQ_WIDTH  + bmPitLeft.XSize;
      y1 = Y_OFFSET + NQ_HEIGHT + bmPitTop.YSize + level;
      x2 = x1 + bmPitSurface.XSize - 1;
      y2 = y1 + bmPitSurface.YSize - 1;
      mpImgPitSurface->SetClientArea(x1, y1, x2, y2);

      //calculate and set NQ location 
      level += HALF_NQ_HEIGHT; //move NQ downwards

      //truncate vertical position of NQ (5 pixels added for horisontal pipe)
      if(level < (Y_LEVEL_OFFSET+Y_TRUNCATE+5))
        level = (Y_LEVEL_OFFSET+Y_TRUNCATE+5);

      x1 = X_OFFSET + NQ_WIDTH + bmPitLeft.XSize * 2 + bmPitSurface.XSize;
      y1 = Y_OFFSET + bmPitTop.YSize + level;
      x2 = x1 + NQ_WIDTH-1;
      y2 = y1 + NQ_HEIGHT-1;
      mpNQSurfaceLevel->SetClientArea(x1, y1, x2, y2);

      mpImgPitLevelSensor->Invalidate();
      mpImgPitUpperLevel->Invalidate();
      mpImgPitLowerLevel->Invalidate();
      mpImgPitTop->Invalidate();
      mpImgPitRight->Invalidate();
      mpNQSurfaceLevel->Invalidate();
    }


    
    /*****************************************************************************
    * Function...: UpdateUpperLevelPosition
    * DESCRIPTION: Set ClientArea of UpperLevel line and NQ
    *****************************************************************************/
    void PitStatus::UpdateUpperLevelPosition()
    {
      int x1, y1, x2, y2, level;

      //calculate and set Image location 
      if(mpDpUpperLevel->GetQuality() == DP_AVAILABLE)
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

      x1 = X_OFFSET + NQ_WIDTH  + bmPitLeft.XSize;
      y1 = Y_OFFSET + NQ_HEIGHT + bmPitTop.YSize + level;
      x2 = x1 + bmPitLevel.XSize - 1;
      y2 = y1 + bmPitLevel.YSize - 1;
      mpImgPitUpperLevel->SetClientArea(x1, y1, x2, y2);


      //calculate and set NQ location 
      level += HALF_NQ_HEIGHT; //move NQ downwards

      //truncate vertical position of NQ
      if(level < Y_TRUNCATE)
        level = Y_TRUNCATE;

      x1 = X_OFFSET;
      y1 = Y_OFFSET + bmPitTop.YSize + level;

      if ( mpDpLowerLevel->GetValue() <= mpDpUpperLevel->GetValue() && (mpNQLowerLevel->GetChildPosY() - y1) < NQ_HEIGHT)
      {
        y1 -= ((NQ_HEIGHT - (mpNQLowerLevel->GetChildPosY() - y1)) / 2) + 1;
        //truncate vertical position of NQ
        if (y1 < (Y_OFFSET + bmPitTop.YSize + Y_TRUNCATE))
          y1 = (Y_OFFSET + bmPitTop.YSize + Y_TRUNCATE);

        mpNQLowerLevel->SetClientArea(x1, y1 + NQ_HEIGHT, x1 + NQ_WIDTH-1, y1 + NQ_HEIGHT + NQ_HEIGHT-1);
      }
      else if ( mpDpLowerLevel->GetValue() > mpDpUpperLevel->GetValue() && (y1 - mpNQLowerLevel->GetChildPosY()) < NQ_HEIGHT)
      {
        y1 += ((NQ_HEIGHT - (y1 - mpNQLowerLevel->GetChildPosY())) / 2) + 1;
        //truncate vertical position of NQ
        if (y1 < (Y_OFFSET + bmPitTop.YSize + Y_TRUNCATE + NQ_HEIGHT))
          y1 = (Y_OFFSET + bmPitTop.YSize + Y_TRUNCATE + NQ_HEIGHT);

        mpNQLowerLevel->SetClientArea(x1, y1 - NQ_HEIGHT, x1 + NQ_WIDTH-1, y1 -1);
      }
      else if ( !mpNQUpperLevel->IsValid() )
        mpDpLowerLevel.SetUpdated();

      x2 = x1 + NQ_WIDTH-1;
      y2 = y1 + NQ_HEIGHT-1;
      mpNQUpperLevel->SetClientArea(x1, y1, x2, y2);


      mpImgPitSurface->Invalidate();
      mpImgPitLowerLevel->Invalidate();
      mpImgPitLevelSensor->Invalidate();
      mpImgPitTop->Invalidate();
      mpImgPitRight->Invalidate();
      mpNQUpperLevel->Invalidate();
    }
    
    /*****************************************************************************
    * Function...: UpdateLowerLevelPosition
    * DESCRIPTION: Set ClientArea of LowerLevel line and NQ. 
    * Sets ClientArea of UpperLevel NQ if NQ are close.
    *****************************************************************************/
    void PitStatus::UpdateLowerLevelPosition()
    {
      int x1, y1, x2, y2, level;

      //calculate and set Image location 
      if(mpDpLowerLevel->GetQuality() == DP_AVAILABLE)
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

      x1 = X_OFFSET + NQ_WIDTH  + bmPitLeft.XSize;
      y1 = Y_OFFSET + NQ_HEIGHT + bmPitTop.YSize + level;
      x2 = x1 + bmPitLevel.XSize - 1;
      y2 = y1 + bmPitLevel.YSize - 1;
      mpImgPitLowerLevel->SetClientArea(x1, y1, x2, y2);

      //calculate and set NQ location 
      level += HALF_NQ_HEIGHT; //move NQ downwards

      //truncate vertical position of NQ
      if(level < Y_TRUNCATE)
        level = Y_TRUNCATE;

      x1 = X_OFFSET;
      y1 = Y_OFFSET + bmPitTop.YSize + level;

      if ( mpDpLowerLevel->GetValue() <= mpDpUpperLevel->GetValue() && (y1 - mpNQUpperLevel->GetChildPosY()) < NQ_HEIGHT)
      {
        y1 += ((NQ_HEIGHT - (y1 - mpNQUpperLevel->GetChildPosY())) / 2) + 1;
        //truncate vertical position of NQ
        if (y1 < (Y_OFFSET + bmPitTop.YSize + Y_TRUNCATE + NQ_HEIGHT))
          y1 = (Y_OFFSET + bmPitTop.YSize + Y_TRUNCATE + NQ_HEIGHT);
        
        mpNQUpperLevel->SetClientArea(x1, y1 - NQ_HEIGHT, x1 + NQ_WIDTH-1, y1 -1);
      }
      else if ( mpDpLowerLevel->GetValue() > mpDpUpperLevel->GetValue() && (mpNQUpperLevel->GetChildPosY() - y1) < NQ_HEIGHT)
      {
        y1 -= ((NQ_HEIGHT - (mpNQUpperLevel->GetChildPosY() - y1)) / 2) + 1;
        //truncate vertical position of NQ
        if (y1 < (Y_OFFSET + bmPitTop.YSize + Y_TRUNCATE))
          y1 = (Y_OFFSET + bmPitTop.YSize + Y_TRUNCATE);
        
        mpNQUpperLevel->SetClientArea(x1, y1 + NQ_HEIGHT, x1 + NQ_WIDTH-1, y1 + NQ_HEIGHT + NQ_HEIGHT-1);
      }
      else if ( !mpNQLowerLevel->IsValid() )
        mpDpUpperLevel.SetUpdated();


      x2 = x1 + NQ_WIDTH-1;
      y2 = y1 + NQ_HEIGHT-1;
      mpNQLowerLevel->SetClientArea(x1, y1, x2, y2);

      mpImgPitSurface->Invalidate();
      mpImgPitUpperLevel->Invalidate();
      mpImgPitLevelSensor->Invalidate();
      mpImgPitTop->Invalidate();
      mpImgPitRight->Invalidate();
      mpNQLowerLevel->Invalidate();
    }

    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION: 
    *****************************************************************************/
    void PitStatus::Run()
    {

      if(mpDpPump1OperationMode.IsUpdated())
      {
        bool pump1_disabled = (mpDpPump1OperationMode->GetValue() == ACTUAL_OPERATION_MODE_DISABLED 
          || mpDpPump1OperationMode->GetQuality() == DP_NOT_AVAILABLE);

        if( mpDpPump1OperationMode->GetValue() == ACTUAL_OPERATION_MODE_STARTED
          && !pump1_disabled
          && mpDpPump1OperationMode->GetQuality() == DP_AVAILABLE)
        {
          mpAniFlowPump1->Start();
        }
        else
        {
          mpAniFlowPump1->Stop();
        }

        mpImgPitPump1Disabled->SetVisible( pump1_disabled );        

        mpImgPitSinglePump->Invalidate();
        mpImgPitPumps->Invalidate();
        mpAniFlowPump1->Invalidate();
      }

      if(mpDpPump2OperationMode.IsUpdated())
      {
        bool is_single_pump_system = (mpDpPump2OperationMode->GetValue() == ACTUAL_OPERATION_MODE_NOT_INSTALLED
          || mpDpPump2OperationMode->GetQuality() == DP_NEVER_AVAILABLE);
        
        bool pump1_disabled = (mpDpPump1OperationMode->GetValue() == ACTUAL_OPERATION_MODE_DISABLED 
          || mpDpPump1OperationMode->GetQuality() == DP_NOT_AVAILABLE);
        
        bool pump2_disabled = (mpDpPump2OperationMode->GetValue() == ACTUAL_OPERATION_MODE_DISABLED
          || mpDpPump2OperationMode->GetQuality() == DP_NOT_AVAILABLE);

        if( mpDpPump2OperationMode->GetValue() == ACTUAL_OPERATION_MODE_STARTED && !pump2_disabled)
        {
          mpAniFlowPump2->Start();
        }
        else
          mpAniFlowPump2->Stop();
        
        mpImgPitSinglePump->SetVisible( is_single_pump_system );
        mpImgPitPumps->SetVisible( !is_single_pump_system );
        mpAniFlowPump2->SetVisible( !is_single_pump_system );

        mpImgPitPump1Disabled->SetVisible( pump1_disabled );
        mpImgPitPump2Disabled->SetVisible( pump2_disabled && !is_single_pump_system );

        mpImgPitSinglePump->Invalidate();
        mpImgPitPumps->Invalidate();
        mpAniFlowPump1->Invalidate();
        mpAniFlowPump2->Invalidate();
        mpImgPitPump1Disabled->Invalidate();
        mpImgPitPump2Disabled->Invalidate();

        mpImgPitLevelSensor->Invalidate();
      }

      if (mpDpNoOfRunningPumps.IsUpdated())
      {
        if(mpDpNoOfRunningPumps->GetValue() > 1)
        {
          mpAniFlowPipe->SetDelay(ANIMATION_SPEED_QUICK);
          mpAniFlowPipe->Start();
        }
        else if ( mpDpNoOfRunningPumps->GetValue() == 1)
        {
          mpAniFlowPipe->SetDelay(ANIMATION_SPEED_SLOW);
          mpAniFlowPipe->Start();
        }
        else
          mpAniFlowPipe->Stop();
      }

      if (mpDpSurfaceLevel.IsUpdated())
      {
        mpImgPitLevelSensor->SetVisible( !mpNQSurfaceLevel->IsNeverAvailable());

        UpdateSurfaceLevelPosition();
      }

      if (mpDpUpperLevel.IsUpdated())
      {
        UpdateUpperLevelPosition();
      }

      if (mpDpLowerLevel.IsUpdated())
      {
        UpdateLowerLevelPosition();
      }

      if (mpDpMixerOperationMode.IsUpdated())
      {
        bool mixer_installed = (mpDpMixerOperationMode->GetValue() != ACTUAL_OPERATION_MODE_NOT_INSTALLED) 
          && (mpDpMixerOperationMode->GetValue() != ACTUAL_OPERATION_MODE_DISABLED)
          && (mpDpMixerOperationMode->GetQuality() == DP_AVAILABLE);

        bool mixer_started = (mpDpMixerOperationMode->GetValue() == ACTUAL_OPERATION_MODE_STARTED) 
          && mixer_installed;

        if( mixer_started )
          mpAniMixer->Start();
        else
          mpAniMixer->Stop();

        mpAniMixer->SetVisible( mixer_installed );

        if (mpImgPitSinglePump->IsVisible())
          mpImgPitSinglePump->Invalidate();
        else if (mpImgPitPumps->IsVisible())
          mpImgPitPumps->Invalidate();

        mpImgPitRight->Invalidate();
        
      }

      Group::Run();
    }

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION: Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool PitStatus::Redraw()
    {
      if(mValid)
        return true;
           
      return Group::Redraw();
    }

    /*****************************************************************************
    * Function...: IsValid
    * DESCRIPTION: 
    *****************************************************************************/
    bool PitStatus::IsValid()
    {
      return mValid;
    }

    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void PitStatus::Update(Subject* pSubject)
    {
      mpDpSurfaceLevel.Update(pSubject);
      mpDpUpperLevel.Update(pSubject);
      mpDpLowerLevel.Update(pSubject);
      mpDpNoOfRunningPumps.Update(pSubject);
      mpDpPump1OperationMode.Update(pSubject);
      mpDpPump2OperationMode.Update(pSubject);
      mpDpMixerOperationMode.Update(pSubject);
    }

    /* --------------------------------------------------
    * Called if subscription shall be cancelled
    * --------------------------------------------------*/
    void PitStatus::SubscribtionCancelled(Subject* pSubject)
    {
      mpDpSurfaceLevel.Detach(pSubject);
      mpDpUpperLevel.Detach(pSubject);
      mpDpLowerLevel.Detach(pSubject);
      mpDpNoOfRunningPumps.Detach(pSubject);
      mpDpPump1OperationMode.Detach(pSubject);
      mpDpPump2OperationMode.Detach(pSubject);
      
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void PitStatus::SetSubjectPointer(int Id,Subject* pSubject)
    {
      
      switch ( Id )
      {
        case SP_PS_SURFACE_LEVEL:
          mpDpSurfaceLevel.Attach(pSubject);
          mpNQSurfaceLevel->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PS_UPPER_LEVEL:
          mpDpUpperLevel.Attach(pSubject);
          mpNQUpperLevel->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PS_LOWER_LEVEL:
          mpDpLowerLevel.Attach(pSubject);
          mpNQLowerLevel->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PS_FLOW:
          mpNQFlow->SetSubjectPointer(Id, pSubject);
          break;
        case SP_PS_OPERATION_MODE_ACTUAL_PUMP_1:
          mpDpPump1OperationMode.Attach(pSubject);
          break;
        case SP_PS_OPERATION_MODE_ACTUAL_PUMP_2:
          mpDpPump2OperationMode.Attach(pSubject);
          break;
        case SP_PS_NO_OF_RUNNING_PUMPS:
          mpDpNoOfRunningPumps.Attach(pSubject);
          break;
        case SP_PS_OPERATION_MODE_MIXER:
          mpDpMixerOperationMode.Attach(pSubject);
          break;

      }
      
      
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void PitStatus::ConnectToSubjects(void)
    {
      mpDpSurfaceLevel->Subscribe(this);
      mpDpUpperLevel->Subscribe(this);
      mpDpLowerLevel->Subscribe(this);
      mpDpNoOfRunningPumps->Subscribe(this);
      mpDpPump1OperationMode->Subscribe(this);
      mpDpPump2OperationMode->Subscribe(this);
      mpDpMixerOperationMode->Subscribe(this);
      
      mpNQSurfaceLevel->ConnectToSubjects();
      mpNQUpperLevel->ConnectToSubjects();
      mpNQLowerLevel->ConnectToSubjects();
      mpNQFlow->ConnectToSubjects();
   }

  } // namespace display
} // namespace mpc


