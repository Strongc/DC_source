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
/* CLASS NAME       : PumpGroupGraphic                                      */
/*                                                                          */
/* FILE NAME        : PumpGroupGraphic.h                                    */
/*                                                                          */
/* CREATED DATE     : 2009-05-19                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*  Shows how the pumps are divided into groups.                            */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mrcPumpGroupGraphic_h
#define mrcPumpGroupGraphic_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "SubjectPtr.h"
#include "BoolDataPoint.h"
#include "U8DataPoint.h"
#include "U32DataPoint.h"
#include "AppTypeDefs.h"
#include "MPCFonts.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Image.h"
#include "Label.h"
#include "ObserverGroup.h"

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    DEFINES
    *****************************************************************************/
    /*****************************************************************************
    EXTERNS
    *****************************************************************************/
    extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPumpGroups;     //

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for
    *
    *****************************************************************************/
    class PumpGroupGraphic : public ObserverGroup
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      PumpGroupGraphic(Component* pParent = NULL);

      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~PumpGroupGraphic();

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
      virtual void CalculateClientAreas();

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      Image* mpImgPumps;

      Label* mpLabelGroupName[NO_OF_PUMP_GROUPS];

      SubjectPtr<U8DataPoint*>   mpDpNoOfPumps;
      SubjectPtr<BoolDataPoint*> mpDpGroupsEnabled;
      SubjectPtr<U32DataPoint*>   mpDpFirstPumpInGroupTwo;

      int mGroupDividerPosition;
      int mVisibleImageWidth;


    };
  } // namespace display
} // namespace mpc

#endif
