/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : AnaInOnIOCtrl                                         */
/*                                                                          */
/* FILE NAME        : AnaInOnIOCtrl.H                                       */
/*                                                                          */
/* CREATED DATE     : 17-02-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ANAINONIOCTRL_H__
#define __ANAINONIOCTRL_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <IobComDrv.h>
#include <ControllerTypes.h>
#include <SwTimerBassClass.h>
#include <SubTask.h>
#include <U8Datapoint.h>
#include <FloatDatapoint.h>
#include <BoolDatapoint.h>
#include <EnumDataPoint.h>
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
    NO_ADC_ERROR,
    CHECK_LOW_ADC_CURRENT_ERROR,
    CHECK_NO_ADC_ERROR,
    CHECK_HIGH_ADC_ERROR,
    LOW_CURRENT_ADC_ERROR,
    HIGH_ADC_ERROR,
    CHECK_ADC_OVER_CURRENT,
    HIGH_ADC_ERROR_CURRENT_PROTECTION,
    WAIT_FOR_STABLE_CURRENT,
    WAIT_FOR_SET_BACK_TO_CURRENT
} ADC_ALARM_STATUS_TYPE;

typedef enum
{
  FIRST_AI_ALARM_ID,
  AnaInOk = FIRST_AI_ALARM_ID,
  SENSOR_ERROR,
  AnaInOverCurrent,
  AnaInUnderCurrent,
  AnaInOverVoltage,
  LAST_AI_ALARM_ID = AnaInOverVoltage
} AI_ALARM_ID_TYPE;

/*****************************************************************************
 * CLASS: AnaInOnIOCtrl
 * DESCRIPTION:
 *
 *****************************************************************************/
class AnaInOnIOCtrl : public SwTimerBaseClass, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Constructor
    ********************************************************************/
    AnaInOnIOCtrl(const int noOnIoBoard);
    /********************************************************************
    LIFECYCLE - Destructor
    ********************************************************************/
    ~AnaInOnIOCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    /********************************************************************
    OPERATIONS
    ********************************************************************/
     // OBSERVER VIRTUAL FUNCTIONS
     virtual void SubscribtionCancelled(Subject* pSubject) {};
     virtual void Update(Subject* pSubject);
     virtual void SetSubjectPointer(int Id,Subject* pSubject);
     virtual void ConnectToSubjects();
     //SubTask
     virtual void RunSubTask();
     virtual void InitSubTask();

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EnumDataPoint<SENSOR_ELECTRIC_TYPE>*> mpAnaInConfSensorElectric; // the electrical type of the Ana In, incl. desabled
    SubjectPtr<FloatDataPoint*> mpAnaInValue; // the dp where the value from the datapoint is placed.
    SubjectPtr<BoolDataPoint*> mpCurrentProtectionAllowed;

    // THE FOLLOWING DP, is set in the INIT, from the -> mpAnaInOnIoBoardNo
    EnumDataPoint<IOB_DIG_OUT_NO_TYPE>* mpVoltageCurrentDOSelectorNo;
    EnumDataPoint<IOB_ANA_IN_NO_TYPE>* mpVoltageAINoOnIOBoard;
    EnumDataPoint<IOB_ANA_IN_NO_TYPE>* mpCurrentAINoOnIOBoard;

    SubjectPtr<AlarmDataPoint*> mpAnaInAlarmObj; // This error obj from the ANA in. Remark, different from sensor error obj.
    EnumDataPoint<AI_ALARM_ID_TYPE>* mpAnaInErrorObj; // internal error obj.

    const int mNoOnIoBoard; // The NO of the AnaIn on the IO board (1-3)
    float mA,mB;
    bool mAvailable;

    float mLowAlarmLimit_1;
    float mLowAlarmLimit_2;
    float mHighAlarmLimit_1;
    float mHighAlarmLimit_2;

    bool mNewValueToDataPoint;

    ADC_ALARM_STATUS_TYPE mAdcAlarmStatus;
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
