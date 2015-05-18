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
/* CLASS NAME       : FloatSwitchCtrlTest                                   */
/*                                                                          */
/* FILE NAME        : FloatSwitchCtrlTest.h                                 */
/*                                                                          */
/* CREATED DATE     : 07-08-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class specification for FloatSwitchCtrlTest.    */
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef wwmrFloatSwitchCtrlTest_h
#define wwmrFloatSwitchCtrlTest_h

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
#include <FloatDataPoint.h>
#include <AppTypeDefs.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "FloatSwitchCtrl.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef struct
{
  DIGITAL_INPUT_FUNC_STATE_TYPE fsw9;
  DIGITAL_INPUT_FUNC_STATE_TYPE fsw8;
  DIGITAL_INPUT_FUNC_STATE_TYPE fsw7;
  DIGITAL_INPUT_FUNC_STATE_TYPE fsw6;
  DIGITAL_INPUT_FUNC_STATE_TYPE fsw5;
  DIGITAL_INPUT_FUNC_STATE_TYPE fsw4;
  DIGITAL_INPUT_FUNC_STATE_TYPE fsw3;
  DIGITAL_INPUT_FUNC_STATE_TYPE fsw2;
  DIGITAL_INPUT_FUNC_STATE_TYPE fsw1;
} FSW_VALUES;

typedef struct
{
  FSW_CFGS c;
  FSW_VALUES v;
  GF_UINT8 l;        /* mpFloatSwitchWaterLevel             */
  GF_UINT8 lp;       /* mpFloatSwitchWaterLevelPct          */
  GF_UINT8 r;        /* mpPumpRef                           */
  GF_UINT8 i;        /* mpFloatSwitchInconsistencyFaultObj  */
  GF_UINT8 d;        /* mpFloatSwitchDryRunFaultObj         */
  GF_UINT8 h;        /* mpFloatSwitchHighLevelFaultObj      */
  GF_UINT8 a;        /* mpFloatSwitchAlarmLevelFaultObj     */
  GF_UINT8 s;        /* mpUnderLowestStopLevel              */
} FSW_TV;


typedef struct
{
  GF_UINT8 hw_a;
  GF_UINT8 a_a;
  GF_UINT8 sta2_a;
  GF_UINT8 sta1_a;
  GF_UINT8 sto2_a;
  GF_UINT8 sto1_a;
  GF_UINT8 dr_a;
  FSW_TV fsw_tv;
} ANALOG_FSW_TV;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class FloatSwitchCtrlTest : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    FloatSwitchCtrlTest(void);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~FloatSwitchCtrlTest(void);

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
    void CheckResults(void *tv, GF_UINT16 i, GF_UINT8 format);
    void FSW_assign(void *tv, GF_UINT16 i, GF_UINT8 format);
    /********************************************************************
    ATTRIBUTES
    ********************************************************************/
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpFloatSwitch[MAX_NO_OF_FLOAT_SWITCHES];
    SubjectPtr<EnumDataPoint<FSW_TYPE>*> mpFloatSwitchCnf[MAX_NO_OF_FLOAT_SWITCHES];
    SubjectPtr<AlarmDataPoint*> mpFloatSwitchInconsistencyFaultObj;
    SubjectPtr<AlarmDataPoint*> mpFloatSwitchHighLevelFaultObj;
    SubjectPtr<AlarmDataPoint*> mpFloatSwitchDryRunFaultObj;
    SubjectPtr<AlarmDataPoint*> mpFloatSwitchAlarmLevelFaultObj;
    SubjectPtr<FloatDataPoint*> mpFloatSwitchWaterLevelPct;
    SubjectPtr<U32DataPoint*> mpFloatSwitchWaterLevel;
    SubjectPtr<BoolDataPoint*> mpUnderLowestStopLevel;
    SubjectPtr<EnumDataPoint<PUMP_REF_TYPE>*> mpPumpRef;
    SubjectPtr<FloatDataPoint*> mpStartLevelPump[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpStopLevelPump[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpAlarmLevel;
    SubjectPtr<FloatDataPoint*> mpDryRunLevel;
    SubjectPtr<FloatDataPoint*> mpHighLevel;
    SubjectPtr<FloatDataPoint*> mpPitLevelMeasured;
    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mpPitLevelCtrlType;

    bool mReqTaskTimeFlag;
    int mTestState;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif

