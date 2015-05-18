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
/* FILE NAME        : PitFloatSwitchGraphic.h                               */
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
#ifndef _pit_float_switch_graphic_h
#define _pit_float_switch_graphic_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "SubjectPtr.h"
#include "U32DataPoint.h"
#include "U8DataPoint.h"
#include "EnumDataPoint.h"
#include "AppTypeDefs.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AbstractPitGraphic.h"
#include "FloatSwitchIconState.h"
#include "Label.h"
#include "Image.h"
#include "AvailabilityGroup.h"

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
    class PitFloatSwitchGraphic : public AbstractPitGraphic
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      PitFloatSwitchGraphic(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~PitFloatSwitchGraphic();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void Run();

      virtual bool IsValid();
      
      virtual void Invalidate();
      
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

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/

      FloatSwitchIconState* mpFloatSwitchIcons[MAX_NO_OF_FLOAT_SWITCHES];

      Label* mpFloatSwitchLabels[MAX_NO_OF_FLOAT_SWITCHES];

      AvailabilityGroup* mpFloatSwitchGroup[MAX_NO_OF_FLOAT_SWITCHES];

      NumberQuantity* mpNQFlow;

      SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpDpFloatSwitchStates[MAX_NO_OF_FLOAT_SWITCHES];

      SubjectPtr<U32DataPoint*> mpDpSurfaceLevel;
      SubjectPtr<U8DataPoint*> mpDpNoOfFloatSwitches;
      
    };
  } // namespace display
} // namespace mpc

#endif
