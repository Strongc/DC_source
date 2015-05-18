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
/* CLASS NAME       : OverflowCtrl                                          */
/*                                                                          */
/* FILE NAME        : OverflowCtrl.h                                        */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                    Detection of overflow and handling of                 */
/*                    overflow counters and alarm.                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcOverflowCtrl_h
#define mrcOverflowCtrl_h

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
#include <EnumDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>
#include <FloatDataPoint.h>
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
  FIRST_OFC_ALARM_OBJ,
  OFC_ALARM_OBJ_OVERFLOW = FIRST_OFC_ALARM_OBJ,

  NO_OF_OFC_ALARM_OBJ,
  LAST_OFC_ALARM_OBJ = NO_OF_OFC_ALARM_OBJ - 1
} OFC_ALARM_OBJ_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class OverflowCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    OverflowCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~OverflowCtrl();
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
    void HandleOverflowAlarm();
    void CheckOverflowPresent();
    void UpdateOverflowVolume();
    void UpdateOverflowCounters();
    void SetFloatSwitchStatusGENI();


    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Variable inputs:
    SubjectPtr<BoolDataPoint*>    mpHighLevelSwitchActivated;
    SubjectPtr<FloatDataPoint*>   mpSurfaceLevel;
    SubjectPtr<FloatDataPoint*>   mpPitBasedFlow;
    SubjectPtr<FloatDataPoint*>   mpReferencePointsBasedFlow;
    SubjectPtr<U32DataPoint*>     mpReactivationDelay;
    SubjectPtr<U32DataPoint*>     mpActivationDelay;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpDiOverflowFuncState;

    // Outputs:
    SubjectPtr<U32DataPoint*>     mpOverflowTime;
    SubjectPtr<U32DataPoint*>     mpOverflowVolumeLitreForLog;
    SubjectPtr<FloatDataPoint*>   mpOverflowVolumeM3ForDisplay;

    SubjectPtr<U32DataPoint*>     mpFreeRunningOverflowVolume;
    SubjectPtr<U32DataPoint*>     mpNoOfOverflow;
    SubjectPtr<U8DataPoint*>      mpFloatSwitchDiStatus;

    // Alarms
    SubjectPtr<AlarmDataPoint*>   mpOfcHighLevelAlarmObj;
    SubjectPtr<AlarmDataPoint*>   mpOfcAlarmObj[NO_OF_OFC_ALARM_OBJ];
    AlarmDelay*                   mpOfcAlarmDelay[NO_OF_OFC_ALARM_OBJ];
    bool                          mOfcAlarmDelayCheckFlag[NO_OF_OFC_ALARM_OBJ];

    // Local data
    bool                          mOverflowIsActive;
    bool                          mConditionalOverflowIsActive;
    bool                          mUpdatingOverflowVolume;
    U32                           mOverflowStartingCountDown;
    U32                           mOverflowEndingCountDown;
    U32                           mOverflowRestartingCountDown;
    U32                           mPreConditionalOverflowTimeInSeconds;
    U32                           mDeltaVolumeInLitre;
    float                         mDeltaVolumeRemainsInLitre;
    U32                           mFreeRunningDeltaVolumeInLitre;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
