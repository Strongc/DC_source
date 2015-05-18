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
/* CLASS NAME       : PumpFlowCtrl                                          */
/*                                                                          */
/* FILE NAME        : PumpFlowCtrl.h                                        */
/*                                                                          */
/* CREATED DATE     : 11-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Calculate the pump flow and check flow alarms   */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcPumpFlowCtrl_h
#define mrcPumpFlowCtrl_h

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
#include <BoolDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
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
  FIRST_PFC_ALARM_OBJ,
  PFC_ALARM_OBJ_LOW_FLOW = FIRST_PFC_ALARM_OBJ,

  NO_OF_PFC_ALARM_OBJ,
  LAST_PFC_ALARM_OBJ = NO_OF_PFC_ALARM_OBJ - 1
} PFC_ALARM_OBJ_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PumpFlowCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PumpFlowCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PumpFlowCtrl();
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
    void UpdatePumpFlow();
    void CheckLowFlowAlarm();


    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<U32DataPoint*>     mpDelayAfterStart;
    SubjectPtr<FloatDataPoint*>   mpFilterFactor;

    // Variable inputs:
    SubjectPtr<FloatDataPoint*>   mpSystemFlow;
    SubjectPtr<FloatDataPoint*>   mpPitBasedPumpFlow;
    SubjectPtr<U8DataPoint*>      mpNoOfRunningPumps;
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpActualOperationMode;
    SubjectPtr<EnumDataPoint<FLOW_QUALITY_TYPE>*> mpFlowQuality;
    SubjectPtr<BoolDataPoint*>    mpUnderLowestStopLevel;

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mpCalculatedPumpFlowRaw;
    SubjectPtr<FloatDataPoint*>   mpCalculatedPumpFlowFiltered;
    SubjectPtr<U32DataPoint*>     mpFlowCalculationTimestamp;
    SubjectPtr<U32DataPoint*>     mpNoOfPumpFlowCalculations;

    // Alarms
    SubjectPtr<AlarmDataPoint*>   mpPfcAlarmObj[NO_OF_PFC_ALARM_OBJ];
    AlarmDelay*                   mpPfcAlarmDelay[NO_OF_PFC_ALARM_OBJ];
    bool                          mPfcAlarmDelayCheckFlag[NO_OF_PFC_ALARM_OBJ];

    // Class privates
    U32                           mStartDelayCounter;
    float                         mHighestFlowDuringPumping;
    bool                          mPumpWasRunningAlone;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
