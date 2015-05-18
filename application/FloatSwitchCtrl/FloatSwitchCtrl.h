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
/* CLASS NAME       : FloatSwitchCtrl                                       */
/*                                                                          */
/* FILE NAME        : FloatSwitchCtrl.h                                     */
/*                                                                          */
/* CREATED DATE     : 09-07-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class specification for FloatSwitchCtrl.        */
/*                          This class uses up to 5 float switches to       */
/*                          determine how many pumps to rum.                */
/*                          Generates alarms if a float switch is defect.   */
/*                          Generates high water alarm                      */
/*                          Generates low water alarm                       */
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef wwmrFloatSwitchCtrl_h
#define wwmrFloatSwitchCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <SubTask.h>
#include <DiFuncHandler.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <AlarmDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <FloatDataPoint.h>
#include <AppTypeDefs.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
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
  TA_STOP_BOTH_PUMPS,
  TA_STOP_PUMP_2,
  TA_STOP_PUMP_1,
  TA_STOP_NO_PUMPS,
  TA_FALSE = TA_STOP_BOTH_PUMPS,
  TA_NONE = 0x80, /* NO ACTION (OR 0) */
  TA_START_PUMP_1 = 0x81,
  TA_START_PUMP_2 = 0x82,
  TA_START_BOTH_PUMPS = 0x83,
  TA_TRUE = 0xFF
} TRANSISION_ACTION;

typedef struct
{
  FSW_TYPE fsw_config;
  TRANSISION_ACTION ta_pump_ref;
  TRANSISION_ACTION ta_high_level;
  TRANSISION_ACTION ta_dry_run;
  TRANSISION_ACTION ta_alarm_level;
} FSW_CONFIG_TRANSISION_ACTION;

typedef enum
{
  ALL_INACTIVE = 0x00,
  SW1_BIT_NO   = 0x01,
  SW2_BIT_NO   = 0x02,
  SW3_BIT_NO   = 0x04,
  SW4_BIT_NO   = 0x08,
  SW5_BIT_NO   = 0x10,
  SW6_BIT_NO   = 0x20,
  SW7_BIT_NO   = 0x40,
  SW8_BIT_NO   = 0x80,
  SW9_BIT_NO   = 0x100
} MRG_FSW_VALUE;



typedef struct
{
  FSW_TYPE fsw9_cfg;
  FSW_TYPE fsw8_cfg;
  FSW_TYPE fsw7_cfg;
  FSW_TYPE fsw6_cfg;
  FSW_TYPE fsw5_cfg;
  FSW_TYPE fsw4_cfg;
  FSW_TYPE fsw3_cfg;
  FSW_TYPE fsw2_cfg;
  FSW_TYPE fsw1_cfg;
} FSW_CFGS;


typedef enum
{
  FIRST_FSW_FAULT_OBJ,
  FSW_FAULT_OBJ_INCONSISTENCY = FIRST_FSW_FAULT_OBJ,
  FSW_FAULT_OBJ_HIGH_LEVEL,
  FSW_FAULT_OBJ_DRY_RUNNING,
  FSW_FAULT_OBJ_ALARM_LEVEL,
  FSW_FAULT_OBJ_SENSOR,
  FSW_FAULT_OBJ_CONFLICT_LEVELS,

  NO_OF_FSW_FAULT_OBJ,
  LAST_FSW_FAULT_OBJ = NO_OF_FSW_FAULT_OBJ - 1
}FSW_FAULT_OBJ_TYPE;

typedef enum
{
  FD_IDLE,
  FD_WAIT_FOR_PUMP_START,
  FD_WAIT_FOR_FOAM_DRAIN_LEVEL,
  FD_PRE_WAIT_FOR_FOAM_DRAIN_TIMEOUT,
  FD_WAIT_FOR_FOAM_DRAIN_TIMEOUT,
  FD_BELOW_DRY_RUN_LIMIT
}FOAM_DRAIN_STATE_TYPE;

typedef enum
{
  FSW_NO_CHANGE,
  FSW_BELOW_LEVEL,
  FSW_ABOVE_LEVEL
} FSW_LEVEL_FLAG;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class FloatSwitchCtrl : public SubTask,  public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    FloatSwitchCtrl(void);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~FloatSwitchCtrl(void);

    /********************************************************************
    ASSIGNMENT OPERATORS
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Functions from Subtask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

    // Functions from Observer
    virtual void SetSubjectPointer(int Id, Subject* pSubject);
    virtual void ConnectToSubjects();
    virtual void Update(Subject* pSubject);
    virtual void SubscribtionCancelled(Subject* pSubject);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    MRG_FSW_VALUE MergeFloatSwitches(void);
    GF_UINT8 GetDiStatusOfFloatSwitches(void);
    MRG_FSW_VALUE Update2ValidFSW(MRG_FSW_VALUE fsw);
    GF_UINT8 CommonTransisionAction(TRANSISION_ACTION ta, GF_UINT8 present_value);
    void UpdateUnderLowestStopLevel(MRG_FSW_VALUE fsw);
    void UpdateUnderDryRunAlarmLevel(MRG_FSW_VALUE fsw);
    void CheckForFSWInconsistency(MRG_FSW_VALUE fsw);
    void UpdateWaterLevelOutputs(MRG_FSW_VALUE fsw);
    void UpdateFSWOutputs(const FSW_CONFIG_TRANSISION_ACTION *ta_ptr);

    PUMP_REF_TYPE GetPumpRef();
    void SetPumpRef(PUMP_REF_TYPE ref);


    /********************************************************************
    ATTRIBUTES
    ********************************************************************/
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpFloatSwitch[MAX_NO_OF_FLOAT_SWITCHES];
    SubjectPtr<EnumDataPoint<FSW_TYPE>*> mpFloatSwitchCnf[MAX_NO_OF_FLOAT_SWITCHES];
    SubjectPtr<AlarmDataPoint*> mpFloatSwitchFaultObj[NO_OF_FSW_FAULT_OBJ];
    SubjectPtr<FloatDataPoint*> mpFloatSwitchWaterLevelPct;
    SubjectPtr<U32DataPoint*> mpFloatSwitchWaterLevel;
    SubjectPtr<BoolDataPoint*> mpUnderLowestStopLevel;
    SubjectPtr<BoolDataPoint*> mpHighLevelStateDetected;
    SubjectPtr<BoolDataPoint*> mpPumpRefLevel[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpStartLevelPump[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpStopLevelPump[NO_OF_PUMPS];
    SubjectPtr<AlarmConfig*> mpAlarmLevelConf;
    SubjectPtr<AlarmConfig*> mpDryRunLevelConf;
    SubjectPtr<AlarmConfig*> mpHighLevelConf;
    SubjectPtr<FloatDataPoint*> mpPitLevelMeasured;
    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mpPitLevelCtrlType;
    SubjectPtr<U32DataPoint*> mpAfterRunDelay;
    SubjectPtr<U32DataPoint*> mpAfterRunHighLevelDelay;
    SubjectPtr<FloatDataPoint*>mpFoamDrainLevel;
    SubjectPtr<U32DataPoint*> mpFoamDrainIntervalTime;
    SubjectPtr<U32DataPoint*> mpFoamDrainTime;
    SubjectPtr<BoolDataPoint*> mpFoamDrainEnabled;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpFoamDrainDigInRequest;
    SubjectPtr<BoolDataPoint*> mpFoamDrainRequest;
    SubjectPtr<BoolDataPoint*> mpHighLevelSwitchActivated;
    SubjectPtr<U8DataPoint*> mpFloatSwitchDiStatus;
    SubjectPtr<BoolDataPoint*> mpFloatSwitchInputMoved;

    SubjectPtr<BoolDataPoint*> mpSurfaceLevelReadyFlag;
    SubjectPtr<U8DataPoint*> mpNoOfPumps;

    AlarmDelay* mpFloatSwitchAlarmDelay[NO_OF_FSW_FAULT_OBJ];
    bool mFloatSwitchAlarmDelayCheckFlag[NO_OF_FSW_FAULT_OBJ];


    bool mPitLevelMeasuredSubscribed;
    bool mReqTaskTimeFlag;
    bool mFloatSwitchCnfChanged;
    bool mFloatSwitchChanged;
    bool mRunSubTaskCausedBySwitchChange;
    bool mRunSubTaskCausedByConfChange;
    MRG_FSW_VALUE mFsw1; /* floatswitch value one sample back in time */
    PUMP_REF_TYPE mLocalFswPumpRef;
    GF_UINT8 mNoOfFsw;
    GF_UINT8 mNoFaultDetectedCnt;     // Counter is reset every time an level conflict is detected. It is increased every
                                        // time no conflict is present. It has to rise above a certain value before an
                                        // level conflict is cleared.
    FSW_TYPE mFswCfg[9]; /* Local configuration */
    bool mPreviousHighWater;

    /* FoamDrainPar */
    FOAM_DRAIN_STATE_TYPE mFoamDrainState;
    bool mFoamDrainIntervalTimeOut;
    bool mFoamDrainTimeOut;
    bool mFoamDrainCrossedDryRunAlarm;
    bool mFoamDrainBlockDryRunAlarm;
    bool mFoamDrainCrossedDryRunWarn;
    bool mFoamDrainBlockDryRunWarn;
    bool mUnderDryRunAlarmLevel;

    /* Flags for deciding High Level and Dry Running alarms from two sources (analog level and float switch) */
    FSW_LEVEL_FLAG mPreviousHighLevelAnalogFlag;
    FSW_LEVEL_FLAG mPreviousHighLevelFloatSwitchFlag;
    FSW_LEVEL_FLAG mHighLevelAnalogFlag;
    FSW_LEVEL_FLAG mHighLevelFloatSwitchFlag;
    FSW_LEVEL_FLAG mPreviousDryRunAnalogFlag;
    FSW_LEVEL_FLAG mPreviousDryRunFloatSwitchFlag;
    FSW_LEVEL_FLAG mDryRunAnalogFlag;
    FSW_LEVEL_FLAG mDryRunFloatSwitchFlag;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif

