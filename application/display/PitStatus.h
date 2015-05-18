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
/* FILE NAME        : PitStatus.h                                           */
/*                                                                          */
/* CREATED DATE     : 2007-06-29                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef _pit_status_h
#define _pit_status_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "SubjectPtr.h"
#include "FloatDataPoint.h"
#include "U8DataPoint.h"
#include "EnumDataPoint.h"
#include "AppTypeDefs.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Frame.h"
#include "AnimatedPumpPipe.h"
#include "AnimatedPipe.h"
#include "AnimatedMixer.h"
#include "Image.h"
#include "NumberQuantity.h"
#include "ObserverGroup.h"
#include "AvailabilityGroup.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for 
    *
    *****************************************************************************/
    class PitStatus : public ObserverGroup//Observer, public Frame
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      PitStatus(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~PitStatus();
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
      void SetStaticClientAreas(void);
      void UpdateSurfaceLevelPosition(void);
      void UpdateUpperLevelPosition(void);
      void UpdateLowerLevelPosition(void);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      Image* mpImgPitTop;
      Image* mpImgPitLeft;
      Image* mpImgPitRight;
      Image* mpImgPitPumps;
      Image* mpImgPitSinglePump;
      Image* mpImgPitSurface;
      Image* mpImgPitUpperLevel;
      Image* mpImgPitLowerLevel;
      Image* mpImgPitLevelSensor;
      Image* mpImgPitPump1Disabled;
      Image* mpImgPitPump2Disabled;

      AnimatedPumpPipe* mpAniFlowPump1;
      AnimatedPumpPipe* mpAniFlowPump2;
      AnimatedPipe*     mpAniFlowPipe;
      AnimatedMixer*    mpAniMixer;

      NumberQuantity* mpNQSurfaceLevel;
      NumberQuantity* mpNQUpperLevel;    
      NumberQuantity* mpNQLowerLevel;
      NumberQuantity* mpNQFlow;

      SubjectPtr<FloatDataPoint*> mpDpSurfaceLevel;
      SubjectPtr<FloatDataPoint*> mpDpUpperLevel;
      SubjectPtr<FloatDataPoint*> mpDpLowerLevel;

      SubjectPtr<U8DataPoint*> mpDpNoOfRunningPumps;

      SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpDpPump1OperationMode;
      SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpDpPump2OperationMode;

      SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpDpMixerOperationMode;
      



    };
  } // namespace display
} // namespace mpc

#endif
