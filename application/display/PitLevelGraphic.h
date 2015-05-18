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
/* FILE NAME        : PitLevelGraphic.h                                     */
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
#ifndef _pit_level_graphic_h
#define _pit_level_graphic_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "SubjectPtr.h"
#include "FloatDataPoint.h"
#include "AppTypeDefs.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Image.h"
#include "NumberQuantity.h"
#include "ObserverGroup.h"
#include "AvailabilityGroup.h"
#include "AbstractPitGraphic.h"
#include "TrendIconState.h"

/*****************************************************************************
DEFINES
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
    class PitLevelGraphic : public AbstractPitGraphic
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      PitLevelGraphic(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~PitLevelGraphic();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void Run();

      virtual bool IsValid();

      virtual void Invalidate(void);

      virtual bool Redraw();

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
      virtual void InvalidatePitContents(void);
      void UpdateSurfaceLevelPosition(void);
      void UpdateUpperLevelPosition(void);
      void UpdateLowerLevelPosition(void);
      int GetLevelPosition(int YHalfLevelHeight, float MeasuredValue);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      int mNoOfRunsSinceSurfaceUpdate;

      Image* mpImgPitLevelSensor;
      Image* mpImgPitSonicSensor;

      NumberQuantity* mpNQSurfaceLevel;
      NumberQuantity* mpNQUpperLevel;    
      NumberQuantity* mpNQLowerLevel;
      NumberQuantity* mpNQFlow;

      TrendIconState* mpIconSurfaceTrend;

      SubjectPtr<FloatDataPoint*> mpDpSurfaceLevel;
      SubjectPtr<FloatDataPoint*> mpDpUpperLevel;
      SubjectPtr<FloatDataPoint*> mpDpLowerLevel;
      SubjectPtr<FloatDataPoint*> mpDpPitDepth;
      SubjectPtr<FloatDataPoint*> mpDpMinVariationLevel;
      SubjectPtr<FloatDataPoint*> mpDpMaxVariationLevel;

    };
  } // namespace display
} // namespace mpc

#endif
