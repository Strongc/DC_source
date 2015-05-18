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
/* CLASS NAME       : SimulatedAlarmCtrl                                    */
/*                                                                          */
/* FILE NAME        : SimulatedAlarmCtrl.h                                  */
/*                                                                          */
/* CREATED DATE     : 03-06-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Controls an alarm object that can be set to     */
/*                          any alarm via GENI commands.                    */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcSimulatedAlarmCtrl_h
#define mrcSimulatedAlarmCtrl_h

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
#include <U8DataPoint.h>
#include <AlarmDataPoint.h>
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
  FIRST_SAC_FAULT_OBJ,
  SAC_FAULT_OBJ_SIMULATED = FIRST_SAC_FAULT_OBJ,

  NO_OF_SAC_FAULT_OBJ,
  LAST_SAC_FAULT_OBJ = NO_OF_SAC_FAULT_OBJ - 1
}SAC_FAULT_OBJ_TYPE;



/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class SimulatedAlarmCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SimulatedAlarmCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~SimulatedAlarmCtrl();
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

    SubjectPtr<U8DataPoint*> mpSimActivate;
    SubjectPtr<U8DataPoint*> mpAlarmCodeSim;
    SubjectPtr<U8DataPoint*> mpDeviceTypeCodeSim;
    SubjectPtr<U8DataPoint*> mpDeviceNoSim;
    SubjectPtr<U8DataPoint*> mpResetTypeSim;
    SubjectPtr<U8DataPoint*> mpActionTypeSim;

    AlarmDelay* mpSimulatedAlarmDelay[NO_OF_SAC_FAULT_OBJ];
    bool mSimulatedAlarmDelayCheckFlag[NO_OF_SAC_FAULT_OBJ];

    SubjectPtr<AlarmDataPoint*> mpSimulatedAlarmObj[NO_OF_SAC_FAULT_OBJ];

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
