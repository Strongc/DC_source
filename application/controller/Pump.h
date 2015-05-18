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
/* CLASS NAME       : Pump                                                  */
/*                                                                          */
/* FILE NAME        : Pump.h                                                */
/*                                                                          */
/* CREATED DATE     : 05-07-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Reprensents the physical pump                   */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcPump_h
#define mrcPump_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <AlarmDelay.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <subtask.h>
#include <pump.h>

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
  FIRST_PUMP_FAULT_OBJ,
  PUMP_FAULT_OBJ_CONTACTOR_FEEDBACK = FIRST_PUMP_FAULT_OBJ,

  NO_OF_PUMP_FAULT_OBJ,
  LAST_PUMP_FAULT_OBJ = NO_OF_PUMP_FAULT_OBJ - 1
}PUMP_FAULT_OBJ_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class Pump : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    Pump();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~Pump();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Subject.
    void SetSubjectPointer(int Id, Subject* pSubject);
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);

    // SubTask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void NormalRun(void);
    void SetContactorAlarm(bool alarm_present);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<U8DataPoint*> mpPumpNo;
    SubjectPtr<U8DataPoint*> mpNoOfPumps;

    SubjectPtr<U32DataPoint*> mpStartStartDelayCnf;
    SubjectPtr<U32DataPoint*> mpStartStopStartDelayCnf;
    SubjectPtr<U32DataPoint*> mpStopStopDelayCnf;

    SubjectPtr<EnumDataPoint<REQ_OPERATION_MODE_TYPE>*> mpOprModeReq;
    SubjectPtr<EnumDataPoint<PUMP_OPERATION_MODE_TYPE>*> mpOprModeLevelCtrl;
    SubjectPtr<EnumDataPoint<ANTI_BLOCKING_REQUEST_TYPE>*> mpAntiBlockingRequest;
    SubjectPtr<EnumDataPoint<VFD_OPERATION_MODE_TYPE>*> mpVfdState;
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpActualOperationMode;
    SubjectPtr<EnumDataPoint<CONTROL_SOURCE_TYPE>*> mpCtrlSource;
    SubjectPtr<BoolDataPoint*> mpPumpAlarmFlag;

    SubjectPtr<BoolDataPoint*> mpPumpStartRelay;
    SubjectPtr<BoolDataPoint*> mpPumpReverseRelay;
    SubjectPtr<BoolDataPoint*> mpPumpEnable;
    SubjectPtr<BoolDataPoint*> mpPumpReadyForAutoOpr;
    SubjectPtr<BoolDataPoint*> mpPumpAvailable;
    SubjectPtr<BoolDataPoint*> mpStartStartDelayActive;
    SubjectPtr<BoolDataPoint*> mpStartStopStartDelayActive;
    SubjectPtr<BoolDataPoint*> mpStopStopDelayActive;
    SubjectPtr<BoolDataPoint*> mpUnderLowestStopLevel;

    SubjectPtr<BoolDataPoint*> mpVfdPumpStartRequest;

    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpPumpContactorFeedback;
    SubjectPtr<BoolDataPoint*> mpReferenceForContactorFeedbackAvailable;
    SubjectPtr<AlarmDataPoint*>   mpContactorAlarmObj;
    AlarmDelay* mpPumpAlarmDelay[NO_OF_PUMP_FAULT_OBJ];
    bool mPumpAlarmDelayCheckFlag[NO_OF_PUMP_FAULT_OBJ];
    bool mContactorAlarmFlag;

    ACTUAL_OPERATION_MODE_TYPE mPumpOperationModeAct;
    ACTUAL_OPERATION_MODE_TYPE mPumpOperationModeReq;
    bool mPumpEnabled;
    bool mPumpInstalled;
    static int mLastEvent;                    //0=stop, 1=start

    bool mStartStartTimeOut;
    bool mStartStopStartTimeOut;
    bool mStopStopTimeOut;
    bool mIgnoreDelays;

    bool mRunRequestedFlag;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
