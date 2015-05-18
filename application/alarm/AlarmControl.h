/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : AlarmControl                                          */
/*                                                                          */
/* FILE NAME        : AlarmControl.h                                        */
/*                                                                          */
/* CREATED DATE     : 16-09-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : AlarmControl observes sereval error sources.    */
/* If alarms are generated, then AlarmControl stores them in the Alarmlog.  */
/* AlarmControl also handles reset and clear of the AlarmLog. Finally       */
/* it determines the state og SystemMode, Operation Mode and AlarmState.    */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAlarmControl_h
#define mrcAlarmControl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <queue>
#include <vector>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <AlarmDef.h>
#include <SubTask.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <AlarmDataPoint.h>
#include <EventDataPoint.h>
#include <SwTimerBassClass.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AlarmLog.h>
#include <AlarmRelay.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  FORWARD DECLARATIONS
 *****************************************************************************/
class AlarmRelay;

/*****************************************************************************
 * CLASS: AlarmControl
 *****************************************************************************/
class AlarmControl : public Subject, public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmControl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AlarmControl();
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
    void PowerDown();
    void ReqCheckAlarmList();

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void NewAlarmEvent(AlarmDataPoint* pErrorSource);
    void UpdateAlarmRelayLists(AlarmDataPoint* pErrorSource);
    void CheckAlarmToScadaCallback(AlarmDataPoint* pErrorSource);
    void DetermineAlarmStates();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/


    SubjectPtr<AlarmLog*> mpAlarmLog;
    SubjectPtr<U32DataPoint*> mpAlarmLogChangedCnt;
    SubjectPtr<EventDataPoint*> mpAlarmResetEvent;
    SubjectPtr<EventDataPoint*> mpClearAlarmLogEvent;
    SubjectPtr<EventDataPoint*> mpAlarmToScadaArrived;
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpPumpOprMode[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*> mpAlarmEventNumber;
    SubjectPtr<BoolDataPoint*>  mpGroup2Enabled;
    SubjectPtr<U32DataPoint*>  mpFirstPumpInGroup2;


    OS_RSEMA mSemaErrorSourceQueue;
    std::vector<ALARM_STATE_TYPE> mAlarmDpErrorVector;
    std::vector<bool> mAlarmDpErrorUpdatedVector;
    std::vector< SubjectPtr<AlarmDataPoint*> > mErrorSourceVector;
    AlarmRelay* mpAlarmRelay[NO_OF_ALARM_RELAYS];

    bool mAlarmRelayCheckFlag[NO_OF_ALARM_RELAYS];
    bool mReqTaskTimeFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
