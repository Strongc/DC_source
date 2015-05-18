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
/* CLASS NAME       : VfdEnergyTestCtrl                                     */
/*                                                                          */
/* FILE NAME        : VfdEnergyTestCtrl.h                                   */
/*                                                                          */
/* CREATED DATE     : 14-05-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class to find the energy optimum vfd frequency  */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcVfdEnergyTestCtrl_h
#define mrcVfdEnergyTestCtrl_h

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
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <EventDataPoint.h>
#include <FloatDataPoint.h>
#include <FloatVectorDataPoint.h>
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

typedef enum
{
  FIRST_ENERGY_TEST_MASTER_STATE = 0,
  ENERGY_TEST_MASTER_STATE_IDLE = FIRST_OPERATION_MODE,
  ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_RAISED,
  ENERGY_TEST_MASTER_STATE_AWAIT_LEVEL_OK,
  ENERGY_TEST_MASTER_STATE_RUNNING,

  NO_OF_ENERGY_TEST_MASTER_STATE,
  LAST_ENERGY_TEST_MASTER_STATE = NO_OF_ENERGY_TEST_MASTER_STATE - 1
} ENERGY_TEST_MASTER_STATE_TYPE;

typedef enum
{
  FIRST_ENERGY_TEST_RUN_STATE = 0,
  ENERGY_TEST_RUN_STATE_IDLE = FIRST_OPERATION_MODE,
  ENERGY_TEST_RUN_STATE_ECO_START,
  ENERGY_TEST_RUN_STATE_ECO_INCREASING,
  ENERGY_TEST_RUN_STATE_ECO_DECREASING,
  ENERGY_TEST_RUN_STATE_SWEEP_START,
  ENERGY_TEST_RUN_STATE_SWEEP_DECREASING,

  NO_OF_ENERGY_TEST_RUN_STATE,
  LAST_ENERGY_TEST_RUN_STATE = NO_OF_ENERGY_TEST_RUN_STATE - 1
} ENERGY_TEST_RUN_STATE_TYPE;

typedef enum
{
  FIRST_TEST_IN_PROGRESS = 0,
  TEST_IN_PROGRESS_IDLE = FIRST_OPERATION_MODE,
  TEST_IN_PROGRESS_ENERGY_TEST,
  TEST_IN_PROGRESS_SELF_LEARNING,

  NO_OF_TEST_IN_PROGRESS,
  LAST_TEST_IN_PROGRESS = NO_OF_TEST_IN_PROGRESS - 1
} TEST_IN_PROGRESS_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class VfdEnergyTestCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    VfdEnergyTestCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~VfdEnergyTestCtrl();
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
    void  PrepareEnergyTest(void);
    void  StartEnergyTestStep(float testFrequency);
    bool  RunEnergyStep(float specificEnergy);
    bool  RunEnergyTest(float specificEnergy);
    bool  IsVelocityConstraintFulfilled(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<BoolDataPoint*>    mpVfdInstalled;
    SubjectPtr<BoolDataPoint*>    mpVfdEconomySelfLearningEnabled;
    SubjectPtr<U32DataPoint*>     mpVfdEconomySelfLearningInterval;
    SubjectPtr<EnumDataPoint<VFD_RUN_MODE_TYPE>*> mpVfdRunMode;
    SubjectPtr<FloatDataPoint*>   mpVfdMinFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdMaxFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdEconomyLevel;
    SubjectPtr<U32DataPoint*>     mpVfdEnergyTestSettlingTime;
    SubjectPtr<FloatDataPoint*>   mpVfdEnergyTestLevelRange;
    SubjectPtr<FloatDataPoint*>   mpLowestStartLevel;
    SubjectPtr<BoolDataPoint*>    mpMinVelocityEnabled;
    SubjectPtr<FloatDataPoint*>   mpMinVelocity;
    SubjectPtr<FloatDataPoint*>   mpPipeDiameter;

    // Variable inputs:
    SubjectPtr<EnumDataPoint<VFD_OPERATION_MODE_TYPE>*> mpVfdState;
    SubjectPtr<FloatDataPoint*>   mpMeasuredValueFlow;
    SubjectPtr<FloatDataPoint*>   mpEstimatedFlow;
    SubjectPtr<FloatDataPoint*>   mpMeasuredValuePower;
    SubjectPtr<FloatDataPoint*>   mpPumpPower;
    SubjectPtr<FloatDataPoint*>   mpSurfaceLevel;
    SubjectPtr<U8DataPoint*>      mpNoOfRunningPumps;
    SubjectPtr<EventDataPoint*>   mpVfdEnergyTestStartTest;

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mpVfdEnergyTestFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdEnergyTestSpecificEnergy;
    SubjectPtr<U32DataPoint*>     mpVfdEnergyTestTimeStamp;
    SubjectPtr<FloatDataPoint*>   mpVfdEconomyFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdEconomyMinFrequency;
    SubjectPtr<FloatVectorDataPoint*> mpSpecificEnergyTestResult;

    // Local variables:
    ENERGY_TEST_MASTER_STATE_TYPE mEnergyTestMasterState;
    ENERGY_TEST_RUN_STATE_TYPE    mEnergyTestRunState;
    TEST_IN_PROGRESS_TYPE         mTestInProgress;
    U32   mTimeSinceSelfLearning;
    U32   mPumpRunningInTestTime;
    I32   mAveragingTime;
    float mAccumulatedSpecificEnergy;
    float mSpecificEnergyAtMax;
    float mLastSpecificEnergy;
    float mTestFrequency;
    float mLastTestFrequency;
    float mFilteredFlow;
    
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
