/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: H2S Prevention                                     */
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
/* CLASS NAME       : DDACtrl                                               */
/*                                                                          */
/* FILE NAME        : DDACtrl.h                                     */
/*                                                                          */
/* CREATED DATE     : 05-07-2013 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Controls the DDA functionality */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcDDACtrl_h
#define mrcDDACtrl_h

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
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <EnumDataPoint.h>
#include <BoolDataPoint.h>
#include <U32DataPoint.h>
#include <U16DataPoint.h>
#include <AlarmDelay.h>
#include <U8DataPoint.h>

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
  DDA_PREVENTED,
  DDA_LEGAL
} DDA_STATE;


typedef enum
{
  DDA_LEGAL_WAITING,
  DDA_LEGAL_REQUESTED,
  DDA_LEGAL_RUNNING
} DDA_LEGAL_STATE;


typedef enum
{
  FIRST_DDA_FAULT_OBJ,
  DDA_FAULT_OBJ = FIRST_DDA_FAULT_OBJ,
  //DDA_FAULT_OBJ_EMPTY_TANK,

  NO_OF_DDA_FAULT_OBJ,
  LAST_DDA_FAULT_OBJ = NO_OF_DDA_FAULT_OBJ - 1
}DDA_FAULT_OBJ_TYPE;

typedef enum
{
  FIRST_DOSING_PUMP_FAULT_OBJ,
  DOSING_PUMP_FAULT_OBJ = FIRST_DOSING_PUMP_FAULT_OBJ,
  //DDA_FAULT_OBJ_EMPTY_TANK,

  NO_OF_DOSING_PUMP_FAULT_OBJ,
  LAST_DOSING_PUMP_FAULT_OBJ = NO_OF_DOSING_PUMP_FAULT_OBJ - 1
}DOSING_PUMP_FAULT_OBJ_TYPE;

typedef struct
{
 U8 Pumping_Status;
 U8 Chemical_Status;
 U8 Alarm_status;

}DDA_STATUS;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class DDACtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    DDACtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~DDACtrl();
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
    void StopTimers(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<BoolDataPoint*> mpDDAed;
    SubjectPtr<U32DataPoint*> mpDDALevelAct;
    SubjectPtr<U32DataPoint*> mpDDADosingFeedTankLevel;
    SubjectPtr<U32DataPoint*> mpDDAChemicalTotalDosed;
    SubjectPtr<EnumDataPoint<DOSING_PUMP_TYPE_TYPE>*>  mpDosingPumpType;

    DDA_STATE mDDAState;
    DDA_LEGAL_STATE mDDALegalState;
    //DDA_STATUS *dda_pump_status;
    //DOSING_PUMP_TYPE_TYPE mDosingPumpType;

    bool mDDAWaitTimerFlag;
    bool mDDARunTimerFlag;
    bool mRunRequestedFlag;

    /* Variables for alarm handling */
    SubjectPtr<AlarmDataPoint*> mDDAAlarms[NO_OF_DDA_FAULT_OBJ];
    AlarmDelay* mpDDAAlarmDelay[NO_OF_DDA_FAULT_OBJ];
    bool mDDAAlarmDelayCheckFlag[NO_OF_DDA_FAULT_OBJ];
    SubjectPtr<AlarmDataPoint*> mDosingPumpAlarms[NO_OF_DOSING_PUMP_FAULT_OBJ];
    AlarmDelay* mpDosingPumpAlarmDelay[NO_OF_DOSING_PUMP_FAULT_OBJ];
    bool mDosingPumpAlarmDelayCheckFlag[NO_OF_DOSING_PUMP_FAULT_OBJ];
    static int counter;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
