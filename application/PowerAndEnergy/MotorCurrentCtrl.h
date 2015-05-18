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
/* CLASS NAME       : MotorCurrentCtrl                                      */
/*                                                                          */
/* FILE NAME        : MotorCurrentCtrl.h                                    */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                    Read the motor current sensor.                        */
/*                    Calculate a filtered value and handle alarms.         */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcMotorCurrentCtrl_h
#define mrcMotorCurrentCtrl_h

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
  FIRST_MOC_ALARM_OBJ,
  MOC_OVERLOAD_ALARM_OBJ = FIRST_MOC_ALARM_OBJ,
  MOC_UNDERLOAD_ALARM_OBJ,

  NO_OF_MOC_ALARM_OBJ,
  LAST_MOC_ALARM_OBJ = NO_OF_MOC_ALARM_OBJ - 1
} MOC_ALARM_OBJ_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class MotorCurrentCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    MotorCurrentCtrl(const int pumpNo);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~MotorCurrentCtrl();
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
    void UpdateCurrent();
    void CheckAlarms();


    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<U32DataPoint*>     mpDelayAfterStart;
    SubjectPtr<FloatDataPoint*>   mpFilterTime;

    // Variable inputs:
    SubjectPtr<FloatDataPoint*>   mpMeasuredValueCurrent;
    SubjectPtr<FloatDataPoint*>   mpCUECurrent;
    SubjectPtr<FloatDataPoint*>   mpMP204Current;
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpActualOperationMode;
    SubjectPtr<BoolDataPoint*>    mpUnderLowestStopLevel;
    SubjectPtr<BoolDataPoint*>    mpFoamDrainingRequested;

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mpLatestValueCurrent;
    SubjectPtr<FloatDataPoint*>   mpFilteredValueCurrent;
    SubjectPtr<FloatDataPoint*>   mpResultingValueCurrent;

    // Alarms
    SubjectPtr<AlarmDataPoint*>   mpMocAlarmObj[NO_OF_MOC_ALARM_OBJ];
    AlarmDelay*                   mpMocAlarmDelay[NO_OF_MOC_ALARM_OBJ];
    bool                          mMocAlarmDelayCheckFlag[NO_OF_MOC_ALARM_OBJ];

    // Class variables
    U32   mSkipRunCounter;
    U32   mAwaitStartDelayCount;
    bool  mStartDelayElapsed;
    bool  mFilteredCurrentHasBeenCalculated;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
