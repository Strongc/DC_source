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
/* CLASS NAME       : DDA                                           */
/*                                                                          */
/* FILE NAME        : DDAFuncHandler.h                                         */
/*                                                                          */
/* CREATED DATE     : 10-08-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : MP204 driver                                    */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __DDA_MODULE_H__
#define __DDA_MODULE_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <AlarmDelay.h>
#include <EventDataPoint.h>
#include <EnumDataPoint.h>
#include <BoolDataPoint.h>
#include <U32DataPoint.h>
#include <U16DataPoint.h>
#include <U8DataPoint.h>
#include <FloatDataPoint.h>


/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
typedef enum
{
  FIRST_DDA_FAULT_OBJ,
  DDA_FAULT_OBJ_GENI_COMM = FIRST_DDA_FAULT_OBJ,
  DDA_FAULT_OBJ_ALARM,

  NO_OF_DDA_FAULT_OBJ,
  LAST_DDA_FAULT_OBJ = NO_OF_DDA_FAULT_OBJ - 1
} DDA_FAULT_OBJ_TYPE;

typedef enum
{
  DDA_INIT,
  DDA_INIT_WAITING,
  DDA_START,
  DDA_RUNNING
} DDA_STATUS;

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/


/*****************************************************************************
  FORWARDS
 *****************************************************************************/
class GeniSlaveIf;

/*****************************************************************************
 * CLASS: MP204Module
 * DESCRIPTION: MP204 driver
 *****************************************************************************/
class DDA : public IO351, public SubTask, public SwTimerBaseClass 
{
  public:
    /********************************************************************
    LIFECYCLE - Constructor
    ********************************************************************/
    DDA(const IO351_NO_TYPE moduleNo);

    /********************************************************************
    DDAModule - Destructor
    ********************************************************************/
    ~DDA();

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
    void InitSubTask(void);
    void RunSubTask();

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void RunDDA();
    void HandleDDAAlarm(ALARM_ID_TYPE warning_code);
    void DDAInit();
    bool CheckInitRespond();
    bool CheckRunRespond();
    bool ValidateSetpoint();
    void HandleDDAMeasuredValues();
    void SetDDADataAvailability(DP_QUALITY_TYPE quality);
    void UpdateDosingVolume();

    // IO351 class overrides
    virtual void ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType);

    /********************************************************************
    ATTRIBUTES
    ********************************************************************/

    // Config
    
    // Input
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpOprModeDosingPump;
    SubjectPtr<EventDataPoint*>     mpSystemAlarmResetEvent;
    SubjectPtr<FloatDataPoint*>     mpDosingRefAct;
    SubjectPtr<BoolDataPoint*>      mpDDAInstalled;
    SubjectPtr<FloatDataPoint*>     mpChemicalTotalDosed;
    SubjectPtr<U32DataPoint*>       mpRunningDosingVolume;
    SubjectPtr<FloatDataPoint*>     mpDosingVolumeTotalLog;


    /* Variables for alarm handling */
    SubjectPtr<AlarmDataPoint*> mDDAAlarms[NO_OF_DDA_FAULT_OBJ];
    AlarmDelay* mpDDAAlarmDelay[NO_OF_DDA_FAULT_OBJ];
    bool mDDAAlarmDelayCheckFlag[NO_OF_DDA_FAULT_OBJ];

    // Local variables
    bool          mInitTimeOutFlag;
    bool          mRunTimeOutFlag;
    GeniSlaveIf*  mpGeniSlaveIf;
    DDA_STATUS    mDDAStatus;
    ALARM_ID_TYPE mExistingAlarmCode;
    U32           mExistingWarnings;
    U32           mMaxDosingCapacity;
    float         mLastChemicalTotalDosed;
    U32           mDosingVolumeWhenStart;
  
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
   /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
