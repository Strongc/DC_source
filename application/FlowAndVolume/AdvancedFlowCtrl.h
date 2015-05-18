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
/* CLASS NAME       : AdvancedFlowCtrl                                      */
/*                                                                          */
/* FILE NAME        : AdvancedFlowCtrl.h                                    */
/*                                                                          */
/* CREATED DATE     : 29-10-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Calculates flow based on pressure measured      */
/*  in pit and outlet pipe and power consumption of pumps.                  */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAdvancedFlowCtrl_h
#define mrcAdvancedFlowCtrl_h

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
#include <BoolDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <EnumDataPoint.h>
#include <EventDataPoint.h>
#include <FloatDataPoint.h>
#include <FloatVectorDataPoint.h>
#include <DoubleVectorDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <FlowLearnControl.h>
#include <SystemFlow.h>
#include <PumpEvaluation.h>
#include <PumpCurves.h>
#include <PitFlow.h>
#include <ParIdent.h>
#include <MotorModel.h>
#include <KalmanCalc.h>
#include <DynparIdent.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  FLOW_LEARNING_IDLE,
  FLOW_LEARNING_RUNNING
} FLOW_LEARNING_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class AdvancedFlowCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AdvancedFlowCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AdvancedFlowCtrl();
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
    void CreateFlowCalculationObjects();
    void InitializeFlowCalculationObjects();

    void HandleAdvancedFlowCalculation();
    void HandleTrainingRestart();
    void UpdateIsAbleAndReadyFlags();
    void UpdatePumpState(PUMP_TYPE pumpNo);
    void UpdateTrainingState(PUMP_TYPE pumpNo);
    void UpdateTrainingEnabled(PUMP_TYPE pumpNo);
    void UpdateTrainingFrequency(PUMP_TYPE pumpNo);
    void UpdateFlowLearnControl(PUMP_TYPE pumpNo);

    void RunPeriodFlowCalculationTasks();
    void CalculateTotalFlow();
    void HandleParameterStorage();

    bool IsTrainingFinishedForPump(PUMP_TYPE pumpNo);

    float GetPitArea();
    float GetPressureDifference();
    float GetPressureInPit();
    float GetPressureInOutlet();

    float GetTorque(int pumpIndex);
    float GetSpeed(int pumpIndex);
    float GetFrequency(int pumpIndex);

    void LoadFlowParametersFromFlash();
    void StoreFlowParametersInFlash();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<EnumDataPoint<FLOW_CALCULATION_TYPE>*> mpFlowCalculationType;
    SubjectPtr<FloatDataPoint*>    mpPitArea;
    SubjectPtr<EventDataPoint*>    mpStartLearningEvent;
    SubjectPtr<FloatDataPoint*>    mpNominalPower;
    SubjectPtr<FloatDataPoint*>    mpNominalPressure;
    SubjectPtr<FloatDataPoint*>    mpNominalFlow;
    SubjectPtr<BoolDataPoint*>     mpVfdInstalled[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>    mpVfdMinFrequency[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>    mpVfdMaxFrequency[NO_OF_PUMPS];
    SubjectPtr<DoubleVectorDataPoint*> mpLearningParameters[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>    mpPitStartLevel;
    SubjectPtr<FloatDataPoint*>    mpPitStopLevel;
    SubjectPtr<FloatDataPoint*>    mpAdvFlowTempStopLevel;
    SubjectPtr<U32DataPoint*>      mpAdvFlowTempStopPeriod;

    // Variable inputs:
    SubjectPtr<U8DataPoint*>       mpNoOfRunningPumps;
    SubjectPtr<FloatDataPoint*>    mpSurfaceLevel;
    SubjectPtr<FloatDataPoint*>    mpAiOutletPressure;

    SubjectPtr<EnumDataPoint<VFD_OPERATION_MODE_TYPE>*> mpVfdState[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpOperationMode[NO_OF_PUMPS];

    SubjectPtr<FloatDataPoint*>    mpPower[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>    mpVfdFrequency[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>    mpCUEFrequency[NO_OF_PUMPS];

    // Outputs:
    SubjectPtr<BoolDataPoint*>     mpIsLearningInProgress;
    SubjectPtr<BoolDataPoint*>     mpIsReadyToLearn;
    SubjectPtr<FloatDataPoint*>    mpVfdTrainingFrequency[NO_OF_PUMPS];
    SubjectPtr<U8DataPoint*>       mpAdvFlowTempStopRequest;
    SubjectPtr<FloatDataPoint*>    mpCalculatedFlow;

    // Internal members:
    bool                           mIsConfiguredForAdvancedFlow;
    bool                           mIsAbleToLearn[NO_OF_PUMPS];

    FLOW_LEARNING_TYPE             mPumpLearningState[NO_OF_PUMPS];
    AFC_TRAINING_STATE_TYPE        mTrainingEnabledState[NO_OF_PUMPS];
    AFC_PUMP_STATE_TYPE            mPumpState[NO_OF_PUMPS];
    int                            mAwaitPumpRampUpTime[NO_OF_PUMPS];

    bool                           mRequestTrainingEvent[NO_OF_PUMPS];
    bool                           mPumpStartedEvent[NO_OF_PUMPS];
    bool                           mNewParametersLearnedEvent;
    bool                           mConfigurationIsUpdated[NO_OF_PUMPS];

    int                            mSecondsSinceParameterStorage;

    // Advanced flow objects:
    FlowLearnControl*              mpFlowLearnControl[NO_OF_PUMPS];
    PumpEvaluation*                mpPumpEvaluations[NO_OF_PUMPS];
    ParIdent*                      mpParameterIdentifiers[NO_OF_PUMPS];
    MotorModel*                    mpMotorModel;
    PumpCurves*                    mpPumpCurves;
    PitFlow*                       mpPitFlow;
    KalmanCalc*                    mpKalmanCalculator;
    SystemFlow*                    mpSystemFlow;
    DynparIdent*                   mpDynamicParameterIdentifier;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
