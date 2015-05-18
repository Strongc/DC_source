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
/* CLASS NAME       : StartPumpCnt                                          */
/*                                                                          */
/* FILE NAME        : StartPumpCnt.h                                        */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcStartPumpCnt_h
#define mrcStartPumpCnt_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>
#include <deque>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include <Observer.h>
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <EnumDataPoint.h>
#include <U32DataPoint.h>
#include <AlarmDelay.h>
#include <AlarmDataPoint.h>

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
  FIRST_SPC_FAULT_OBJ,
  SPC_FAULT_OBJ_STARTS_PR_HOUR = FIRST_SPC_FAULT_OBJ,

  NO_OF_SPC_FAULT_OBJ,
  LAST_SPC_FAULT_OBJ = NO_OF_SPC_FAULT_OBJ - 1
}SPC_FAULT_OBJ_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class StartPumpCnt : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    StartPumpCnt();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~StartPumpCnt();
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
    void SetSubjectPointer(int Id, Subject* pSubject);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/


    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<U32DataPoint*> mpPumpStartCnt;
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpActualOperationMode;
    SubjectPtr<U32DataPoint*> mpStartPrHour;
    SubjectPtr<U32DataPoint*> mplastStartTime;

    AlarmDelay* mpStartPumpCntAlarmDelay[NO_OF_SPC_FAULT_OBJ];
    bool mStartPumpCntAlarmDelayCheckFlag[NO_OF_SPC_FAULT_OBJ];

    SubjectPtr<AlarmDataPoint*> mpStartPumpCntFaultObj[NO_OF_SPC_FAULT_OBJ];

    std::deque<int> mStartQue;

    int mActTime;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
