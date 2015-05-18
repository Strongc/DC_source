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
/* CLASS NAME       : LevelCtrl                                             */
/*                                                                          */
/* FILE NAME        : LevelCtrl.h                                           */
/*                                                                          */
/* CREATED DATE     : 20-05-2009  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class specification for LevelCtrl.              */
/*                          This class uses an analog level sensor to       */
/*                          determine how many pumps to rum.                */
/*                          Generates alarms if the sensor is defect.       */
/*                          Generates high water alarm                      */
/*                          Generates low water alarm                       */
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef wwmrLevelCtrl_h
#define wwmrLevelCtrl_h

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

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class LevelCtrl : public SubTask, public Observer// public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    LevelCtrl(void);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~LevelCtrl(void);

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
    typedef enum
    {
      FIRST_LEVEL_FAULT,
      LEVEL_FAULT_HIGH_LEVEL = FIRST_LEVEL_FAULT,
      LEVEL_FAULT_DRY_RUNNING,
      LEVEL_FAULT_ALARM_LEVEL,
      LEVEL_FAULT_SENSOR,
      LEVEL_FAULT_CONFLICT_LEVELS,
      LEVEL_FAULT_FSW_INCONSISTENCY,

      NO_OF_LEVEL_FAULT,
      LAST_LEVEL_FAULT = NO_OF_LEVEL_FAULT - 1
    } LEVEL_FAULT_TYPE;


    /********************************************************************
    OPERATIONS
    ********************************************************************/


    /********************************************************************
    ATTRIBUTES
    ********************************************************************/
    // Input values
    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mpPitLevelCtrlType;
    SubjectPtr<BoolDataPoint*> mpSurfaceLevelReady;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpFloatSwitch[2];
    SubjectPtr<EnumDataPoint<FSW_TYPE>*> mpFloatSwitchCnf[2];
    SubjectPtr<BoolDataPoint*> mpFoamDrainActive;
    SubjectPtr<FloatDataPoint*> mpSurfaceLevel;
    SubjectPtr<FloatDataPoint*> mpStartLevelPump[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpStopLevelPump[NO_OF_PUMPS];
    SubjectPtr<U8DataPoint*> mpNoOfPumps;
    SubjectPtr<U32DataPoint*> mpAfterRunHighLevelDelay;

    SubjectPtr<AlarmConfig*> mpDryRunAlarmConf;
    SubjectPtr<AlarmConfig*> mpHighLevelAlarmConf;
    SubjectPtr<AlarmConfig*> mpAlarmLevelAlarmConf;

    // Output values
    SubjectPtr<BoolDataPoint*> mpHighLevelSwitchActivated;
    SubjectPtr<U8DataPoint*> mpFloatSwitchDiStatus;
    SubjectPtr<BoolDataPoint*> mpUnderDryRunLevel;
    SubjectPtr<BoolDataPoint*> mpUnderLowestStopLevel;
    SubjectPtr<BoolDataPoint*> mpHighLevelStateDetected;
    SubjectPtr<BoolDataPoint*> mpPumpRunForLowestStopLevel[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*> mpPumpRefLevel[NO_OF_PUMPS];

    SubjectPtr<AlarmDataPoint*> mAlarms[NO_OF_LEVEL_FAULT];
    AlarmDelay* mAlarmDelays[NO_OF_LEVEL_FAULT];
    bool mAlarmDelayCheckFlags[NO_OF_LEVEL_FAULT];

    SwTimer* mpDryRunAlarmBlockedByFoamDrainTimer;
    SwTimer* mpDryRunWarningBlockedByFoamDrainTimer;
    SwTimer* mpHighLevelAfterRunTimer;

    GF_UINT8 mOldNoOfPumps;
    bool mOldHighLevel;

    bool mFirstRunInAnalogueMode;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif

