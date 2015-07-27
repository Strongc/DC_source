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
/* CLASS NAME       : LoggingCtrl                                           */
/*                                                                          */
/* FILE NAME        : LoggingCtrl.h                                         */
/*                                                                          */
/* CREATED DATE     : 20-02-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :  Class to handle the hourly + daily log values. */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcLoggingCtrl_h
#define mrcLoggingCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Observer.h>
#include <SubTask.h>
#include <MpcTimeCmpCtrl.h>
#include <EventDataPoint.h>
#include <U32DataPoint.h>
#include <FloatDataPoint.h>
#include <I32VectorDataPoint.h>
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



/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class LoggingCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    LoggingCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~LoggingCtrl();
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
    void UpdateU32Accumulator(SubjectPtr<U32DataPoint*> newValueDp, U32 &oldValue,
                              SubjectPtr<U32DataPoint*> p1HourAcc, SubjectPtr<U32DataPoint*> pTodayAcc);
    void UpdateFloatAccumulator(SubjectPtr<U32DataPoint*> newValueDp, U32 &oldValue,
                                SubjectPtr<U32DataPoint*> p1HourAcc, SubjectPtr<FloatDataPoint*> pTodayAcc, float scaleFactor);
    void UpdateRunningAccumulators();
    void UpdateHourLog();
    void MarkHourLogInvalid(U32 noOfHours);
    void UpdateDayLog();
    void UpdateTodayAverages(U32 noOfHours);
    void UpdateTodayAccumulators(U32 noOfHours);
    void FillValuesForTest();


    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration / power on handling
    SubjectPtr<U32DataPoint*>           mpDayLoggingStartHour;
    SubjectPtr<U32DataPoint*>           mpHourLogTimestamp;

    // Inputs:
    SubjectPtr<EventDataPoint*>         mpFillSimulatedLogData;

    SubjectPtr<U32DataPoint*>           mpFreeRunningPumpedVolume;
    SubjectPtr<U32DataPoint*>           mpFreeRunningOverflowVolume;
    SubjectPtr<U32DataPoint*>           mpOverflowCount;
    SubjectPtr<U32DataPoint*>           mpOverflowTime;
    SubjectPtr<U32DataPoint*>           mpParallelOperationTime;
    SubjectPtr<U32DataPoint*>           mpFreeRunningEnergy;
    SubjectPtr<U32DataPoint*>           mpRunningDosingVolume;

    SubjectPtr<U32DataPoint*>           mpOperationTime[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>           mpNoOfStarts[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>         mpFilteredFlow[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>         mpFilteredCurrent[NO_OF_PUMPS];

    // Outputs:
    SubjectPtr<U32DataPoint*>           mpPumpedVolume1hAcc;
    SubjectPtr<FloatVectorDataPoint*>   mpPumpedVolume72hLog;
    SubjectPtr<FloatDataPoint*>         mpPumpedVolumeTodayLog;
    SubjectPtr<FloatDataPoint*>         mpPumpedVolumeYesterdayLog;
    SubjectPtr<U32DataPoint*>           mpOverflowVolume1hAcc;
    SubjectPtr<FloatVectorDataPoint*>   mpOverflowVolume72hLog;
    SubjectPtr<FloatDataPoint*>         mpOverflowVolumeTodayLog;
    SubjectPtr<FloatDataPoint*>         mpOverflowVolumeYesterdayLog;
    SubjectPtr<U32DataPoint*>           mpOverflowCount1hAcc;
    SubjectPtr<I32VectorDataPoint*>     mpOverflowCount72hLog;
    SubjectPtr<U32DataPoint*>           mpOverflowCountTodayLog;
    SubjectPtr<U32DataPoint*>           mpOverflowCountYesterdayLog;
    SubjectPtr<U32DataPoint*>           mpOverflowTime1hAcc;
    SubjectPtr<I32VectorDataPoint*>     mpOverflowTime72hLog;
    SubjectPtr<U32DataPoint*>           mpOverflowTimeTodayLog;
    SubjectPtr<U32DataPoint*>           mpOverflowTimeYesterdayLog;
    SubjectPtr<U32DataPoint*>           mpParallelOperationTime1hAcc;
    SubjectPtr<I32VectorDataPoint*>     mpParallelOperationTime72hLog;
    SubjectPtr<U32DataPoint*>           mpParallelOperationTimeTodayLog;
    SubjectPtr<U32DataPoint*>           mpParallelOperationTimeYesterdayLog;
    SubjectPtr<U32DataPoint*>           mpEnergyConsumption1hAcc;
    SubjectPtr<FloatVectorDataPoint*>   mpEnergyConsumption72hLog;
    SubjectPtr<FloatDataPoint*>         mpEnergyConsumptionTodayLog;
    SubjectPtr<FloatDataPoint*>         mpEnergyConsumptionYesterdayLog;
    SubjectPtr<FloatVectorDataPoint*>   mpEfficiency72hLog;
    SubjectPtr<FloatDataPoint*>         mpEfficiencyTodayLog;
    SubjectPtr<FloatDataPoint*>         mpEfficiencyYesterdayLog;
    SubjectPtr<U32DataPoint*>           mpDosingVolume1hAcc;
    SubjectPtr<I32VectorDataPoint*>     mpDosingVolume72hLog;
    SubjectPtr<FloatDataPoint*>         mpDosingVolumeTodayLog;
    SubjectPtr<FloatDataPoint*>         mpDosingVolumeYesterdayLog;

    SubjectPtr<U32DataPoint*>           mpOperationTime1hAcc[NO_OF_PUMPS];
    SubjectPtr<I32VectorDataPoint*>     mpOperationTime72hLog[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>           mpOperationTimeTodayLog[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>           mpOperationTimeYesterdayLog[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>           mpNoOfStarts1hAcc[NO_OF_PUMPS];
    SubjectPtr<I32VectorDataPoint*>     mpNoOfStarts72hLog[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>           mpNoOfStartsTodayLog[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>           mpNoOfStartsYesterdayLog[NO_OF_PUMPS];
    SubjectPtr<FloatVectorDataPoint*>   mpFilteredFlow72hLog[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>         mpFilteredFlowTodayLog[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>         mpFilteredFlowYesterdayLog[NO_OF_PUMPS];
    SubjectPtr<FloatVectorDataPoint*>   mpFilteredCurrent72hLog[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>         mpFilteredCurrentTodayLog[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>         mpFilteredCurrentYesterdayLog[NO_OF_PUMPS];

    U32   mLastFreeRunningPumpedVolume;
    U32   mLastFreeRunningOverflowVolume;
    U32   mLastOverflowCount;
    U32   mLastOverflowTime;
    U32   mLastParallelOperationTime;
    U32   mLastFreeRunningEnergy;
    U32   mLastDosingVolume;
    U32   mLastOperationTime[NO_OF_PUMPS];
    U32   mLastNoOfStarts[NO_OF_PUMPS];

    SubjectPtr<U32DataPoint*>           mPumpRunTimeXPumps[NO_OF_PUMPS + 1];
    SubjectPtr<U32DataPoint*>           mPumpRunTimeXPumps1hAcc[NO_OF_PUMPS + 1];
    SubjectPtr<I32VectorDataPoint*>     mPumpRunTimeXPumps72hLog[NO_OF_PUMPS + 1];
    SubjectPtr<U32DataPoint*>           mPumpRunTimeXPumpsTodayLog[NO_OF_PUMPS + 1];
    SubjectPtr<U32DataPoint*>           mPumpRunTimeXPumpsYesterdayLog[NO_OF_PUMPS + 1];
    U32                                 mLastPumpRunTimeXPumps[NO_OF_PUMPS + 1];

    MpcTime*          mTimeChangeObj;
    MpcTimeCmpCtrl*   mTimeCompareObs;

    bool  mLogDataSimulation;
    bool  mFirstTimeToRun;
    bool  mRunRequestedFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
