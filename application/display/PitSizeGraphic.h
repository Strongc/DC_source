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
/* FILE NAME        : PitSizeGraphic.h                                      */
/*                                                                          */
/* CREATED DATE     : 2007-07-27                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef _pit_size_graphics_h
#define _pit_size_graphics_h

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
#include "BoolDataPoint.h"
#include "EnumDataPoint.h"
#include "AppTypeDefs.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AnimatedImage.h"
#include "Image.h"
#include "NumberQuantity.h"
#include "ObserverGroup.h"
#include "Label.h"
#include "YesNoState.h"

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
    class PitSizeGraphic : public ObserverGroup
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      PitSizeGraphic(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~PitSizeGraphic();
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
      void UpdateUpperLevelPosition(void);
      void UpdateLowerLevelPosition(void);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      Image* mpImgPitLeft;
      Image* mpImgPitRight;
      Image* mpImgPitBottom;
      Image* mpImgPitUpperLevel;
      Image* mpImgPitLowerLevel;
      Image* mpImgPitSurfaceArea;

      NumberQuantity* mpNQUpperLevel;    
      NumberQuantity* mpNQLowerLevel;

      Label* mpLabelPitDepth;
      NumberQuantity* mpNQPitDepth;

      Label* mpLabelPitArea;
      NumberQuantity* mpNQPitArea;

      Label* mpLabelLearning;
      YesNoState* mpStateLearning;

      SubjectPtr<FloatDataPoint*> mpDpUpperLevel;
      SubjectPtr<FloatDataPoint*> mpDpLowerLevel;
      SubjectPtr<FloatDataPoint*> mpDpPitDepth;
      SubjectPtr<FloatDataPoint*> mpDpPitArea;

      SubjectPtr<BoolDataPoint*> mpDpLearningInProgress;
      SubjectPtr<EnumDataPoint<FLOW_CALCULATION_TYPE>*> mpDpFlowCalculationType;
    

    };
  } // namespace display
} // namespace mpc

#endif
