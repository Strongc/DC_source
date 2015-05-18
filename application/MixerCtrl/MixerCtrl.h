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
/* CLASS NAME       : MixerCtrl                                             */
/*                                                                          */
/* FILE NAME        : MixerCtrl.h                                           */
/*                                                                          */
/* CREATED DATE     : 5-09-2007 dd-mm-yyyy                                  */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcMixerCtrl_h
#define mrcMixerCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <subtask.h>
#include <U32DataPoint.h>
#include <U8DataPoint.h>
#include <FloatDataPoint.h>
//#include <AlarmDataPoint.h>
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
  MM_PRE_DISABLED,
  MM_DISABLED,
  MM_PRE_WAIT_FOR_RATIO,
  MM_WAIT_FOR_RATIO,
  MM_WAIT_FOR_START_LEVEL,
  MM_PRE_STARTED,
  MM_STARTED,
  MM_WAIT_FOR_STOP_LEVEL
} MIXER_CTRL_MODE_TYPE;

typedef enum
{
  FIRST_MM_FAULT_OBJ,
  MM_FAULT_OBJ_CONTACTOR_FEEDBACK = FIRST_MM_FAULT_OBJ,

  NO_OF_MM_FAULT_OBJ,
  LAST_MM_FAULT_OBJ = NO_OF_MM_FAULT_OBJ - 1
}MM_FAULT_OBJ_TYPE;


 /*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class MixerCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    MixerCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~MixerCtrl();
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
    void StartMixer(void);
    void StopMixer(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpOprModeMixer;
    SubjectPtr<BoolDataPoint*> mpMixerRelay;
    SubjectPtr<BoolDataPoint*> mpMixerEnabled;
    SubjectPtr<BoolDataPoint*> mpMixWhilePumping;
    SubjectPtr<U32DataPoint*> mpMixerRatio;
    SubjectPtr<U32DataPoint*> mpMixerMaxRunTimeCnf;
    SubjectPtr<U8DataPoint*> mpNumberOfRunningPumps;
    SubjectPtr<FloatDataPoint*> mpPitLevel;
    SubjectPtr<FloatDataPoint*> mpStartLevel_1;
    SubjectPtr<FloatDataPoint*> mpMixerStopLevel;
    SubjectPtr<FloatDataPoint*> mpMixerStartLevelOffset;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpMixerContactorFeedback;
    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mpPitLevelCtrlType;
    SubjectPtr<U8DataPoint*> mpNoOfAvailablePumps;
	SubjectPtr<BoolDataPoint*> mpMixFallingLevelOnly;

    AlarmDelay* mpMixerAlarmDelay[NO_OF_MM_FAULT_OBJ];
    bool mMixerAlarmDelayCheckFlag[NO_OF_MM_FAULT_OBJ];

    SubjectPtr<AlarmDataPoint*> mpMaxStartsPerHourAlarm;

    bool mRunRequestedFlag;
    bool mMixingTimeout;
    MIXER_CTRL_MODE_TYPE mMixerCtrlMode;
    int mRatioCnt;
	float mprepitlevel;
	bool  mprepitinitlevel;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};


#endif
