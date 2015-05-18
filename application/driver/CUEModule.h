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
/* CLASS NAME       : CUEModule                                             */
/*                                                                          */
/* FILE NAME        : CUEModule.h                                           */
/*                                                                          */
/* CREATED DATE     : 6-4-2009 dd-mm-yyyy                                   */
/*                                                                          */
/* SHORT FILE DESCRIPTION : CUE driver                                      */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __CUE_MODULE_H__
#define __CUE_MODULE_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <EventDataPoint.h>
#include <FloatDataPoint.h>
#include <BoolDataPoint.h>
#include <AlarmDelay.h>

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
  FIRST_CUE_FAULT_OBJ,
  CUE_FAULT_OBJ_GENI_COMM = FIRST_CUE_FAULT_OBJ,
  CUE_FAULT_OBJ_ALARM,
  CUE_FAULT_OBJ_WARNING,

  NO_OF_CUE_FAULT_OBJ,
  LAST_CUE_FAULT_OBJ = NO_OF_CUE_FAULT_OBJ - 1
}CUE_FAULT_OBJ_TYPE;


/*****************************************************************************
  FORWARDS
 *****************************************************************************/
class GeniSlaveIf;

/*****************************************************************************
 * CLASS: CUEModule
 * DESCRIPTION: CUE driver
 *****************************************************************************/
class CUEModule : public IO351, public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Constructor
    ********************************************************************/
    CUEModule(const IO351_NO_TYPE moduleNo);

    /********************************************************************
    CUEModule - Destructor
    ********************************************************************/
    ~CUEModule();

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Subject
    void SetSubjectPointer(int Id, Subject* pSubject);
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);

    // SubTask
    virtual void InitSubTask(void);
    virtual void RunSubTask();

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void HandleCUECommands(bool cueReady);
    void CheckCommandResponse();
    void HandleCUEMeasuredValues();
    void HandleCUEStatus();
    void HandleCUEAlarm(ALARM_ID_TYPE new_code);
    void HandleCUEWarning(ALARM_ID_TYPE new_code);
    void SetCUEDataAvailability(DP_QUALITY_TYPE quality);

    // IO351 class overrides
    virtual void ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType);

    /********************************************************************
    ATTRIBUTES
    ********************************************************************/
    // Input
    SubjectPtr<BoolDataPoint*>  mpCUEInstalled;
    SubjectPtr<EventDataPoint*> mpResetEvent;
    SubjectPtr<EventDataPoint*> mpSystemAlarmResetEvent;
    SubjectPtr<EventDataPoint*> mpModuleAlarmResetEvent;
    SubjectPtr<EventDataPoint*> mpStopEvent;
    SubjectPtr<EventDataPoint*> mpStartEvent;
    SubjectPtr<EventDataPoint*> mpRemoteEvent;
    SubjectPtr<EventDataPoint*> mpConstFreqEvent;
    SubjectPtr<EventDataPoint*> mpForwardEvent;
    SubjectPtr<EventDataPoint*> mpReverseEvent;
    SubjectPtr<FloatDataPoint*> mpCUEReference;

    // Output
    SubjectPtr<FloatDataPoint*> mpVoltage;
    SubjectPtr<FloatDataPoint*> mpPower;
    SubjectPtr<FloatDataPoint*> mpEnergy;
    SubjectPtr<FloatDataPoint*> mpCurrent;
    SubjectPtr<FloatDataPoint*> mpFrequency;
    SubjectPtr<FloatDataPoint*> mpSpeed;
    SubjectPtr<FloatDataPoint*> mpTorque;
    SubjectPtr<FloatDataPoint*> mpRemoteTemperature;
    SubjectPtr<EnumDataPoint<CUE_OPERATION_MODE_TYPE>*> mpCUEOperationMode;
    SubjectPtr<EnumDataPoint<CUE_SYSTEM_MODE_TYPE>*> mpCUESystemMode;
    SubjectPtr<EnumDataPoint<CUE_LOOP_MODE_TYPE>*> mpCUEControlMode;
    SubjectPtr<EnumDataPoint<CUE_SOURCE_MODE_TYPE>*> mpCUESourceMode;
    SubjectPtr<BoolDataPoint*> mpCUEReverseStatus;
    SubjectPtr<EnumDataPoint<IO_DEVICE_STATUS_TYPE>*> mpCUEDeviceStatus;
    SubjectPtr<BoolDataPoint*> mpCUECommunicationFlag;

    SubjectPtr<AlarmDataPoint*> mpCUEAlarmObj;    // To allow changing of alarm code since only one object covers all alarms
    SubjectPtr<AlarmDataPoint*> mpCUEWarningObj;  // To allow changing of warn. code since only one object covers all warn's
    AlarmDelay* mpCUEAlarmDelay[NO_OF_CUE_FAULT_OBJ];
    bool mCUEAlarmDelayCheckFlag[NO_OF_CUE_FAULT_OBJ];
    ALARM_ID_TYPE mExistingAlarmCode;
    ALARM_ID_TYPE mExistingWarningCode;

    U32 mSkipRunCounter;
    bool mCUEStartRequest;
    bool mCUEReverseRequest;
    bool mCheckCommandResponseTimeout;

    GeniSlaveIf* mpGeniSlaveIf;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
   /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
