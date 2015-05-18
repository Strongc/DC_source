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
/* CLASS NAME       : WaterInOilCtrl                                        */
/*                                                                          */
/* FILE NAME        : WaterInOilCtrl.h                                      */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                    Read the Water in Oil sensor.                         */
/*                    Calculate an average value and handle alarms.         */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcWaterInOilCtrl_h
#define mrcWaterInOilCtrl_h

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
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
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
  FIRST_WIO_ALARM_OBJ,
  WIO_ALARM_OBJ_LIMIT = FIRST_WIO_ALARM_OBJ,
  WIO_ALARM_OBJ_SENSOR,

  NO_OF_WIO_ALARM_OBJ,
  LAST_WIO_ALARM_OBJ = NO_OF_WIO_ALARM_OBJ - 1
} WIO_ALARM_OBJ_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class WaterInOilCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    WaterInOilCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~WaterInOilCtrl();
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
    void CheckWioSensorPresent();
    void HandleWio(float wio_value);
    void CheckWioLimits();


    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<U32DataPoint*>     mpDelayAfterStart;
    SubjectPtr<U32DataPoint*>     mpAveragingTime;

    // Variable inputs:
    SubjectPtr<FloatDataPoint*>   mpMeasuredValueWaterInOil;
    SubjectPtr<FloatDataPoint*>   mpIO111WaterInOil;
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpActualOperationMode;

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mpFilteredValueWaterInOil;
    SubjectPtr<FloatDataPoint*>   mpResultingValueWaterInOil;

    // Alarms
    SubjectPtr<AlarmDataPoint*>   mpWioAlarmObj[NO_OF_WIO_ALARM_OBJ];
    AlarmDelay*                   mpWioAlarmDelay[NO_OF_WIO_ALARM_OBJ];
    bool                          mWioAlarmDelayCheckFlag[NO_OF_WIO_ALARM_OBJ];

    // Class members
    U32   mOneSecondCounter;
    float mOneSecondAccumulator;
    U32   mStartDelayCounter;
    U32   mAveragingCounter;
    float mAveragingAccumulator;
    bool  mSensorConnected;
    bool  mWioLimitAlarmAllowed;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
