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
#include <FloatDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
class GeniSlaveIf;


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

typedef enum
{
  FIRST_DDAC_FAULT_OBJ,
  DDA_FAULT_OBJ_H2S = FIRST_DDAC_FAULT_OBJ,

  NO_OF_DDAC_FAULT_OBJ,
  LAST_DDAC_FAULT_OBJ = NO_OF_DDAC_FAULT_OBJ - 1
}DDAC_FAULT_OBJ_TYPE;
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

    SubjectPtr<BoolDataPoint*> mpDosingPumpInstalled;
    SubjectPtr<BoolDataPoint*> mpDDAInstalled;
    SubjectPtr<FloatDataPoint*> mpSetDosingRef;
    SubjectPtr<U32DataPoint*> mpDDARef;
    SubjectPtr<U16DataPoint*> mpSetH2SFault;
    SubjectPtr<U32DataPoint*> mpH2SLevelAct;
    SubjectPtr<U32DataPoint*> mpSetH2SLevel;
    SubjectPtr<U32DataPoint*> mpDDALevelToday;
    SubjectPtr<U32DataPoint*> mpDDALevelYesterday;
    SubjectPtr<FloatDataPoint*> mpMeasuredValue;
    SubjectPtr<FloatDataPoint*> mpDDADosingFeedTankLevel;
    SubjectPtr<EnumDataPoint<DOSING_PUMP_TYPE_TYPE>*>  mpDosingPumpType;

    //For test

    //DOSING_PUMP_TYPE_TYPE mDosingPumpType;

    bool mDDAWaitTimerFlag;
    bool mDDARunTimerFlag;
    bool mRunRequestedFlag;

    SubjectPtr<AlarmDataPoint*> mAlarms[NO_OF_DDAC_FAULT_OBJ];
    AlarmDelay* mpAlarmDelay[NO_OF_DDAC_FAULT_OBJ];
    bool mAlarmDelayCheckFlag[NO_OF_DDAC_FAULT_OBJ];

    GeniSlaveIf*  mpGeniSlaveIf;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
