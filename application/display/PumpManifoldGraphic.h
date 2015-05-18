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
/* CLASS NAME       : PumpManifoldGraphic                                   */
/*                                                                          */
/* FILE NAME        : PumpManifoldGraphic.h                                 */
/*                                                                          */
/* CREATED DATE     : 2009-06-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for showing all pumps with animated flows.     */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mrc_PumpManifoldGraphic_h
#define mrc_PumpManifoldGraphic_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "AppTypeDefs.h"
#include "EnumDataPoint.h"
#include "ObserverGroup.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Image.h"
#include "AnimatedImage.h"
#include "AnimatedPump.h"
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
    * This Class is responsible for showing all pumps with animated flow
    *
    *****************************************************************************/
    class PumpManifoldGraphic : public ObserverGroup
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      PumpManifoldGraphic(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~PumpManifoldGraphic();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void Run(void);

      virtual bool Redraw(void);

      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* Object);
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
      void CreateAnimatedPumpFlowLayout(void);
      void RunWithAnimatedPumpFlow(void);

      void CreateAnimatedPumpsLayout(void);
      void RunWithAnimatedPumps(void);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      Image*            mpImgPumpManifold;
      Image*            mpImgPumpDisabled[NO_OF_PUMPS];
      AnimatedImage*    mpAniFlowPump[NO_OF_PUMPS];

      AnimatedPump*     mpAnimatedPump[NO_OF_PUMPS];

      SubjectPtr<EnumDataPoint<ALARM_STATE_TYPE>*> mpDpPumpAlarmState[NO_OF_PUMPS];
      SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mDpOperationMode[NO_OF_PUMPS];

      bool mUseRelease2layout;
      
    };
  } // namespace display
} // namespace mpc

#endif
