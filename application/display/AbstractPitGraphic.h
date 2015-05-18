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
/* FILE NAME        : AbstractPitGraphic.h                                  */
/*                                                                          */
/* CREATED DATE     : 2007-08-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef _abstract_pit_graphic_h
#define _abstract_pit_graphic_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "SubjectPtr.h"
#include "BoolDataPoint.h"
#include "FloatDataPoint.h"
#include "U8DataPoint.h"
#include "EnumDataPoint.h"
#include "AppTypeDefs.h"
#include "MPCFonts.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AnimatedImage.h"
#include "AnimatedPump.h"
#include "Image.h"
#include "NumberQuantity.h"
#include "ObserverGroup.h"
#include "AvailabilityGroup.h"


namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    DEFINES
    *****************************************************************************/
    // sizes and offsets in pixels
    #define FLOW_NQ_WIDTH    70
    #define X_LEVEL_INDENT   8 // length of level-line to the left of the pit    
    #define NQ_WIDTH         40
    #define NQ_HEIGHT        15
    #define NO_OF_PIT_WIDTHS 3
    #define PIT_TOP_HEIGHT   12
    #define PUMP_HEIGHT      (AnimatedPump::HEIGHT)


    /*****************************************************************************
    EXTERNS
    *****************************************************************************/ 
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitLeft;       //   2,117
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitRight;      //  13,117
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPit1Pump;      //  64, 29
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPitWater;      //  75, 110 

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for 
    *
    *****************************************************************************/
    class AbstractPitGraphic : public ObserverGroup
    {
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
    protected:
      AbstractPitGraphic(Component* pParent = NULL);

    public:
      
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~AbstractPitGraphic();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void Run();

      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      virtual bool IsValid();

      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* pSubject);

      /* --------------------------------------------------
      * Called if subscription shall be canceled
      * --------------------------------------------------*/
      virtual void SubscribtionCancelled(Subject* pSubject);

      /* --------------------------------------------------
      * Called to set the subject pointer (used by class
      * factory)
      * --------------------------------------------------*/
      virtual void SetSubjectPointer(int Id,Subject* pSubject);

      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      virtual void ConnectToSubjects(void);

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Calculates and sets ClientAreas of all children
      * --------------------------------------------------*/
      virtual void CalculateClientAreas(void);
      void CalculateClientAreas(U8 numberOfInstalledPumps, U8 xOffset);
      
      virtual void UpdatePumpAnimations(void);
      void UpdateMixerAnimation(void);
      virtual void InvalidatePitContents(void);
      virtual void ChangeWaterLevelClientArea(int x1, int y1, int x2, int y2);

      U8 GetInternalPitWidth(void);

      U8 GetPitWidthIndex(U8 numberOfInstalledPumps);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      Image* mpImgPitTop[NO_OF_PIT_WIDTHS];
      Image* mpImgPitLeft;
      Image* mpImgPitRight;
      Image* mpImgPitBottom;
      Image* mpImgPitInlet;
      Image* mpImgWater;
      Image* mpImgPitUpperLevel;
      Image* mpImgPitLowerLevel;
      Image* mpImgPitMinVariationLevel;
      Image* mpImgPitMaxVariationLevel;

      Image* mpImgPitPumpManifold[NO_OF_PUMPS];  // used for DC2 only
      Image* mpImgPitPumpDisabled[NO_OF_PUMPS];  // used for DC2 only
      AnimatedImage*  mpAniFlowPump[NO_OF_PUMPS];// used for DC2 only

      AnimatedImage*  mpAniFlowPipe;
      AnimatedImage*  mpAniMixer;
      AnimatedPump*   mpPump[NO_OF_PUMPS];

      NumberQuantity*   mpNQInletFlow;

      bool mHideAllPumps;
      bool mUseRelease2layout;

      SubjectPtr<U8DataPoint*> mpDpNoOfPumpsInstalled;
      SubjectPtr<U8DataPoint*> mpDpNoOfPumpsRunning;
      SubjectPtr<EnumDataPoint<ALARM_STATE_TYPE>*> mpDpPumpAlarmState[NO_OF_PUMPS];
      SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpDpPumpOperationMode[NO_OF_PUMPS];
      SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpDpMixerOperationMode;
      SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mpDpSensorType;
      SubjectPtr<BoolDataPoint*>  mpDpSimulatorEnabled;
      

    };
  } // namespace display
} // namespace mpc

#endif
