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
/* CLASS NAME       : AppModeCtrl                                           */
/*                                                                          */
/* FILE NAME        : AppModeCtrl.h                                         */
/*                                                                          */
/* CREATED DATE     : 22-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Sets the application mode for the display       */
/*                          based upon information about pump ctrl mode,    */
/*                          operation mode, alarms and the startup delay    */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAppModeCtrl_h
#define mrcAppModeCtrl_h

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
#include <AlarmDataPoint.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>

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
class AppModeCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AppModeCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AppModeCtrl();
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
    bool AllPumpsOutOfOperation(void);
    bool AllPumpsManual(void);


    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<EnumDataPoint<APPLICATION_MODE_TYPE>*> mpAppMode;

    SubjectPtr<BoolDataPoint*> mpStartupDelay;
    SubjectPtr<AlarmDataPoint*> mpMainsFailure;
    SubjectPtr<BoolDataPoint*> mpAllPumpAlarmFlag;
    SubjectPtr<EnumDataPoint<REQ_OPERATION_MODE_TYPE>*> mpOperationModeReqPump[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpOperationModeActualPump[NO_OF_PUMPS];
    SubjectPtr<AlarmDataPoint*> mpConflictingLevelsAlarmObj;
    SubjectPtr<AlarmDataPoint*> mpLevelSensorAlarmObj;
    SubjectPtr<BoolDataPoint*> mpDailyEmptyingRequested;
    SubjectPtr<BoolDataPoint*> mpFoamDrainingRequested;
    SubjectPtr<EnumDataPoint<APPLICATION_MODE_TYPE>*> mpPumpCtrlMode;
    SubjectPtr<BoolDataPoint*> mpServiceModeEnabled;

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
