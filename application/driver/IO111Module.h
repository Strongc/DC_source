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
/* CLASS NAME       : IO111IOModule                                         */
/*                                                                          */
/* FILE NAME        : IO111IOModule.h                                       */
/*                                                                          */
/* CREATED DATE     : 28-11-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : IO 111 driver                                   */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __IO111_MODULE_H__
#define __IO111_MODULE_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <SubTask.h>
#include <EventDataPoint.h>
#include <FloatDataPoint.h>
#include <BoolDataPoint.h>
#include <IIntegerDataPoint.h>
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
  FIRST_IO111_FAULT_OBJ,
  IO111_FAULT_OBJ_GENI_COMM = FIRST_IO111_FAULT_OBJ,
  IO111_FAULT_OBJ_ALARM,
  IO111_FAULT_OBJ_WARNING,

  NO_OF_IO111_FAULT_OBJ,
  LAST_IO111_FAULT_OBJ = NO_OF_IO111_FAULT_OBJ - 1
}IO111_FAULT_OBJ_TYPE;








/*****************************************************************************
  FORWARDS
 *****************************************************************************/
class GeniSlaveIf;

/*****************************************************************************
 * CLASS: IO111Module
 * DESCRIPTION: IO 111 driver
 *****************************************************************************/
class IO111Module : public IO351, public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Constructor
    ********************************************************************/
    IO111Module(const IO351_NO_TYPE moduleNo);

    /********************************************************************
    I0351Module - Destructor
    ********************************************************************/
    ~IO111Module();

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Subject
    void SetSubjectPointer(int Id, Subject* pSubject);
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);


    // SubTask
    virtual void InitSubTask(void);
    virtual void RunSubTask();

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void HandleIO111MeasuredValues();
    void HandleIO111Alarm(ALARM_ID_TYPE new_code);
    void HandleIO111Warning(ALARM_ID_TYPE new_code);
    void SetIO111DataAvailability(DP_QUALITY_TYPE quality);

    // IO351 class overrides
    virtual void ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType);

    /********************************************************************
    ATTRIBUTES
    ********************************************************************/
    SubjectPtr<BoolDataPoint*>  mpIo111Installed;

    SubjectPtr<EventDataPoint*> mpSystemAlarmResetEvent;
    SubjectPtr<EventDataPoint*> mpModuleAlarmResetEvent;

    SubjectPtr<FloatDataPoint*> mpTemperatureSupportBearing;
    SubjectPtr<FloatDataPoint*> mpTemperatureMainBearing;
    SubjectPtr<FloatDataPoint*> mpTemperaturePT100;
    SubjectPtr<FloatDataPoint*> mpTemperaturePT1000;
    SubjectPtr<FloatDataPoint*> mpTemperature;
    SubjectPtr<FloatDataPoint*> mpInsulationResistance;
    SubjectPtr<FloatDataPoint*> mpVibration;
    SubjectPtr<FloatDataPoint*> mpWaterInOil;
    SubjectPtr<BoolDataPoint*>  mpMoistureSwitch;
    SubjectPtr<BoolDataPoint*>  mpThermalSwitch;

    SubjectPtr<EnumDataPoint<IO_DEVICE_STATUS_TYPE>*> mpIO111DeviceStatus;

    SubjectPtr<AlarmDataPoint*> mpIO111AlarmObj;    // To allow changing of alarm code since only one object covers all alarms
    SubjectPtr<AlarmDataPoint*> mpIO111WarningObj;  // To allow changing of warn. code since only one object covers all warn's
    AlarmDelay* mpIO111AlarmDelay[NO_OF_IO111_FAULT_OBJ];
    bool mIO111AlarmDelayCheckFlag[NO_OF_IO111_FAULT_OBJ];

    ALARM_ID_TYPE mExistingAlarmCode;
    ALARM_ID_TYPE mExistingWarningCode;
    U32 mSkipRunCounter;

    GeniSlaveIf* mpGeniSlaveIf;

    // Needed for IO11x type recognition.
    SubjectPtr<EnumDataPoint<IO11X_UNIT_TYPE_TYPE>*> mpUnitType;

    // IO113 datapoints.
    SubjectPtr<U16DataPoint*> mpSpeed; // Pump or mixer speed.

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
   /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
