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
/* CLASS NAME       : PitBasedFlowCtrl                                      */
/*                                                                          */
/* FILE NAME        : PitBasedFlowCtrl.h                                    */
/*                                                                          */
/* CREATED DATE     : 11-01-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                    Calculate the in flow based on changes in pit level.  */
/*                    Estimate the pump flow when one pump is running.      */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcPitBasedFlowCtrl_h
#define mrcPitBasedFlowCtrl_h

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
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <FloatDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

// Pit measuring states
typedef enum
{
  FIRST_PIT_STATE = 0,
  PIT_STATE_AWAIT_FILLING = FIRST_PIT_STATE,
  PIT_STATE_FILLING,
  PIT_STATE_AWAIT_EMPTYING,
  PIT_STATE_EMPTYING,

  // Insert new items above
  NO_OF_PIT_STATE,
  LAST_PIT_STATE = NO_OF_PIT_STATE - 1
} PIT_STATE_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PitBasedFlowCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PitBasedFlowCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PitBasedFlowCtrl();
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
    void HandlePitBasedFlow();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<EnumDataPoint<FLOW_CALCULATION_TYPE>*>    mpFlowCalculationType;
    SubjectPtr<FloatDataPoint*>   mpUpperMeasurementLevel;
    SubjectPtr<FloatDataPoint*>   mpLowerMeasurementLevel;
    SubjectPtr<FloatDataPoint*>   mpVolumeForFlowMeasurement;
    SubjectPtr<U32DataPoint*>     mpMaxFlowMeasurementTime;
    SubjectPtr<U32DataPoint*>     mpFlowMinMultiply;
    SubjectPtr<U32DataPoint*>     mpFlowMaxMultiply;

    // Variable inputs:
    SubjectPtr<FloatDataPoint*>   mpSurfaceLevel;
    SubjectPtr<U8DataPoint*>      mpNoOfRunningPumps;

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mpPitBasedFlow;
    SubjectPtr<FloatDataPoint*>   mpPitBasedPumpFlow;
    SubjectPtr<U32DataPoint*>     mpInFlowMeasurementTime;

    // Class variables
    PIT_STATE_TYPE                mPitState;
    U32                           mFillingTime;
    U32                           mEmptyingTime;
    U32                           mInflowResetTime;
    U32                           mOldNoOfPumps;
    float                         mOldPitLevel;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
