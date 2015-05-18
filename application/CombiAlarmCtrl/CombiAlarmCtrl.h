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
/* CLASS NAME       : CombiAlarmCtrl                                        */
/*                                                                          */
/* FILE NAME        : CombiAlarmCtrl.h                                      */
/*                                                                          */
/* CREATED DATE     : 29-01-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcCombiAlarmCtrl_h
#define mrcCombiAlarmCtrl_h

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
  FIRST_CAC_FAULT_OBJ,
  CAC_FAULT_OBJ_COMBI_ALARM = FIRST_CAC_FAULT_OBJ,

  NO_OF_CAC_FAULT_OBJ,
  LAST_CAC_FAULT_OBJ = NO_OF_CAC_FAULT_OBJ - 1
}CAC_FAULT_OBJ_TYPE;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class CombiAlarmCtrl : public Observer, public SubTask

{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    CombiAlarmCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~CombiAlarmCtrl();
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

    void SetSourcePointer(EnumDataPoint<ALARM_TYPE>* source, BoolDataPoint** alarm);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<BoolDataPoint*> mpPumpAlarmFlag[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*> mpAllPumpAlarmFlag;
    SubjectPtr<BoolDataPoint*> mpAnyPumpAlarmFlag;
    SubjectPtr<BoolDataPoint*> mpPumpGeniIO111AlarmFlag[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*> mpOverflowAlarmFlag;
    SubjectPtr<BoolDataPoint*> mpLevelAlarmFlag;
    SubjectPtr<BoolDataPoint*> mpHighLevelAlarmFlag;
    SubjectPtr<BoolDataPoint*> mpUserFuncFlag[NO_OF_USER_IO + 1];
    

    BoolDataPoint* mpSource1AlarmFlag;
    BoolDataPoint* mpSource2AlarmFlag;
    SubjectPtr<EnumDataPoint<ALARM_TYPE>*> mpSource1;
    SubjectPtr<EnumDataPoint<ALARM_TYPE>*> mpSource2;

    SubjectPtr<BoolDataPoint*> mpNoAlarmSelectedFlag;

    AlarmDelay* mpCombiAlarmDelay[NO_OF_CAC_FAULT_OBJ];
    bool mCombiAlarmDelayCheckFlag[NO_OF_CAC_FAULT_OBJ];

    bool mRunRequestedFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
