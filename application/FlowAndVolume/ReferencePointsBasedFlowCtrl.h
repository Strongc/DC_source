/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : ReferencePointsBasedFlowCtrl                          */
/*                                                                          */
/* FILE NAME        : ReferencePointsBasedFlowCtrl.h                        */
/*                                                                          */
/* CREATED DATE     : 07-11-2011 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                 Calculate the in flow based on changes in pit level      */
/*                 using reference points relative to the level where       */
/*                 overflow switch is triggered.                            */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcReferencePointsBasedFlowCtrl_h
#define mrcReferencePointsBasedFlowCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include <Observer.h>
#include <SubTask.h>
#include <EnumDataPoint.h>
#include <BoolDataPoint.h>
#include <FloatDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define NO_OF_REF_POINTS 10
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class ReferencePointsBasedFlowCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    ReferencePointsBasedFlowCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~ReferencePointsBasedFlowCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void InitSubTask();
    void RunSubTask();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);


  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void HandleConfigurationChange(void);
    void HandleReferencePointsBasedFlow(void);
    float GetFlowAtLevel(float level);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:   
    SubjectPtr<BoolDataPoint*>    mRefPointsEnabled[NO_OF_REF_POINTS];
    SubjectPtr<FloatDataPoint*>   mRefPointsLevel[NO_OF_REF_POINTS];
    SubjectPtr<FloatDataPoint*>   mRefPointsFlow[NO_OF_REF_POINTS];
    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mControlType;

    // Variable inputs:
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mDiOverflowFuncState;
    SubjectPtr<FloatDataPoint*>   mSurfaceLevel;

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mReferencePointsBasedFlow;
   
    // Class variables
    float mOverflowLevelOffset;
    U8 mNumberOfReferencePoints;
    float mSortedLevels[NO_OF_REF_POINTS];
    float mFlowsOfLevels[NO_OF_REF_POINTS];
    bool mConfigurationChanged;

    DIGITAL_INPUT_FUNC_STATE_TYPE mPreviousSwitchState;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
