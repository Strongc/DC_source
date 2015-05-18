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
/* CLASS NAME       : AnaInMeasureValueCtrl                                 */
/*                                                                          */
/* FILE NAME        : AnaInMeasureValueCtrl.h                               */
/*                                                                          */
/* CREATED DATE     : 17-02-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : This is the header file for a single class      */
/*                            AnaInMeasureValueCtrl                         */
/*                          This class performs the following duties based  */
/*                          on the 7 AI's and what has been configured for  */
/*                          them:                                           */
/*                          - Updates all 35 (at the time of writing)       */
/*                            measured values                               */
/*                          - Updates sensor output (value of the primary   */
/*                            sensor                                        */
/*                          - Updates 7 dp's showing measured value and     */
/*                            value for the 7 AI's, used for electrical     */
/*                            overview                                      */
/*                          - controls redundant sensors: If 2 sensors are  */
/*                            redundant only the best is used for the       */
/*                            corredsponding measured value                 */
/*                          - controls warnings for redundant sensors       */
/*                          - controls error for the primary sensor         */
/*                          Note: This class collects functionality         */
/*                          previously (release 1) to be found in class     */
/*                          SensorCtrl, which is now extinct                */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __ANA_IN_MEASURE_VALUE_CTRL_H__
#define __ANA_IN_MEASURE_VALUE_CTRL_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <Subject.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <SubTask.h>
#include <EnumDataPoint.h>
#include <FloatDataPoint.h>
#include <BoolDataPoint.h>
#include <AlarmDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/

/*****************************************************************************
DEFINES
*****************************************************************************/
const int AIMVC_NO_OF_ANA_IN = 9;
const int NO_OF_REDUNDANCY_TIMERS = AIMVC_NO_OF_ANA_IN - 1;

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
typedef enum
{
  NO_REDUNDANT_ERROR,
  CHECK_REDUNDANT_ERROR,
  CHECK_REDUNDANT_OK,
  REDUNDANT_ERROR
} REDUNDANT_SENSOR_ERROR_STATUS_TYPE;

/*****************************************************************************
* CLASS:
* DESCRIPTION:
*
*****************************************************************************/
class AnaInMeasureValueCtrl : public SwTimerBaseClass, public SubTask
{
public:
  /********************************************************************
  LIFECYCLE - Default constructor.
  ********************************************************************/
  AnaInMeasureValueCtrl();
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  ~AnaInMeasureValueCtrl();
  /********************************************************************
  ASSIGNMENT OPERATOR
  ********************************************************************/
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  // Subject.
  void SetSubjectPointer(int Id, Subject* pSubject);
  void ConnectToSubjects();
  void Update(Subject* pSubject);
  void SubscribtionCancelled(Subject* pSubject);

  virtual void RunSubTask();
  virtual void InitSubTask(void);
private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  bool IsThereToBigDifferenceBetween2RedundantSensors(FloatDataPoint* pSensor1 , FloatDataPoint* pSensor2);
  void HandleRedundantSensor(unsigned int AnaInFirst, unsigned int AnaInSecond, const int MeasuredValueNo);
  void HandleNotRedundantSensor(const unsigned int AnaInNo, const int MeasuredValueNo);
  void UpdateNotRedundantSensor(const unsigned int AnaInNo, const int MeasuredValueNo);
  void DisableMeasuredValueAI(const unsigned int AnaInNo);
  void EnableMeasuredValueAI(const unsigned int AnaInNo);
  void WriteToMeasuredValueAI(const unsigned int AnaInNo);
  void HandleRedundancyStateMachineEvents(const unsigned int RedundantTimerNo);
  void ResetRedundancyStateMachine(const unsigned int RedundantTimerNo);

  /********************************************************************
  ATTRIBUTE
  ********************************************************************/

  SubjectPtr<FloatDataPoint*> mpAnaInValue[AIMVC_NO_OF_ANA_IN]; // input values from ADC / IO 351
  bool mAnaInValueUpdated; // true if one of mpAnaInValue has been updated

  SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*> mpConfMeasuredValue[AIMVC_NO_OF_ANA_IN];
  SubjectPtr<EnumDataPoint<SENSOR_ELECTRIC_TYPE>*> mpConfSensorElectric[AIMVC_NO_OF_ANA_IN];
  SubjectPtr<FloatDataPoint*> mpConfSensorMinValue[AIMVC_NO_OF_ANA_IN];
  SubjectPtr<FloatDataPoint*> mpConfSensorMaxValue[AIMVC_NO_OF_ANA_IN];
  SubjectPtr<BoolDataPoint*> mpAnaInSetupFromGeniFlag;

  SubjectPtr<BoolDataPoint*> mpAnalogValuesMeasuredFlag;

  bool mSensorConfigUpdated; // true if one of the mpConf... data point has been updated

  SubjectPtr<FloatDataPoint*> mpElectricalValueAI[AIMVC_NO_OF_ANA_IN];  // value in [V] or [A]
  SubjectPtr<FloatDataPoint*> mpMeasuredValueAI[AIMVC_NO_OF_ANA_IN];  // measured values
  SubjectPtr<FloatDataPoint*> mpOutputList[NO_OF_MEASURED_VALUE];  // specific measured output values

  bool mListOfRedundancyTimerEvents[NO_OF_REDUNDANCY_TIMERS];
  int mRedundantPartner[AIMVC_NO_OF_ANA_IN];

  bool mHandleRedundancyTimers;

  SubjectPtr<AlarmDataPoint*> mpRedundantSensorWarningObj[NO_OF_REDUNDANCY_TIMERS]; // This error obj from the redundant sensor.
  REDUNDANT_SENSOR_ERROR_STATUS_TYPE mRedundantErrorStatus[NO_OF_REDUNDANCY_TIMERS];

  bool mReqTaskTime;   //flag to control requesting of task time

protected:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
};
#endif
