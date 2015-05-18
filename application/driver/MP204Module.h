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
/* CLASS NAME       : MP204Module                                           */
/*                                                                          */
/* FILE NAME        : MP204Module.h                                         */
/*                                                                          */
/* CREATED DATE     : 10-08-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : MP204 driver                                    */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __MP204_MODULE_H__
#define __MP204_MODULE_H__

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
#include <AlarmDelay.h>
#include <AlarmDef.h>

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
  FIRST_MP204_FAULT_OBJ,
  MP204_FAULT_OBJ_GENI_COMM = FIRST_MP204_FAULT_OBJ ,
  MP204_FAULT_OBJ_ALARM                             ,
  MP204_FAULT_OBJ_WARNING_TIME_FOR_SERVICE          ,
  MP204_FAULT_OBJ_WARNING_INSULATION_RESISTANCE_LOW ,
  MP204_FAULT_OBJ_WARNING_TOO_MANY_STARTS_PER_HOUR  ,
  MP204_FAULT_OBJ_WARNING_LOAD_CONTINUES_DESPITE_OFF,
  MP204_FAULT_OBJ_WARNING_OVERVOLTAGE               ,
  MP204_FAULT_OBJ_WARNING_UNDERVOLTAGE              ,
  MP204_FAULT_OBJ_WARNING_OVERLOAD                  ,
  MP204_FAULT_OBJ_WARNING_UNDERLOAD                 ,
  MP204_FAULT_OBJ_WARNING_OVERTEMPERATURE_TEMPCON   ,
  MP204_FAULT_OBJ_WARNING_OVERTEMPERATURE_PT100     ,
  MP204_FAULT_OBJ_WARNING_TEMPCON_SENSOR_FAULT      ,
  MP204_FAULT_OBJ_WARNING_CURRENT_ASYMMETRY         ,
  MP204_FAULT_OBJ_WARNING_COS_PHI_MAX               ,
  MP204_FAULT_OBJ_WARNING_COS_PHI_MIN               ,
  MP204_FAULT_OBJ_WARNING_START_CAPACITOR_TOO_LOW   ,
  MP204_FAULT_OBJ_WARNING_RUN_CAPACITOR_TOO_LOW     ,
  MP204_FAULT_OBJ_WARNING_PT100_SENSOR_FAULT        ,
  MP204_FAULT_OBJ_WARNING_OTHER                     ,

  // Insert above
  NO_OF_MP204_FAULT_OBJ,
  LAST_MP204_FAULT_OBJ = NO_OF_MP204_FAULT_OBJ - 1
} MP204_FAULT_OBJ_TYPE;


/*****************************************************************************
  FORWARDS
 *****************************************************************************/
class GeniSlaveIf;

/*****************************************************************************
 * CLASS: MP204Module
 * DESCRIPTION: MP204 driver
 *****************************************************************************/
class MP204Module : public IO351, public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Constructor
    ********************************************************************/
    MP204Module(const IO351_NO_TYPE moduleNo);

    /********************************************************************
    MP204Module - Destructor
    ********************************************************************/
    ~MP204Module();

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
    void HandleMP204MeasuredValues();
    void HandleMP204Alarm(ALARM_ID_TYPE warning_code);
    void HandleMP204Warning(U32 warnings);
    void SetMP204DataAvailability(DP_QUALITY_TYPE quality);

    // IO351 class overrides
    virtual void ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType);

    /********************************************************************
    ATTRIBUTES
    ********************************************************************/

    // Config
    SubjectPtr<BoolDataPoint*>  mpMP204Installed;

    // Input
    SubjectPtr<EventDataPoint*> mpSystemAlarmResetEvent;
    SubjectPtr<EventDataPoint*> mpModuleAlarmResetEvent;

    // Output
    SubjectPtr<EnumDataPoint<IO_DEVICE_STATUS_TYPE>*> mpMP204DeviceStatus;
    SubjectPtr<BoolDataPoint*>  mpMP204CommunicationFlag;
    SubjectPtr<FloatDataPoint*> mpVoltage;
    SubjectPtr<FloatDataPoint*> mpCurrent;
    SubjectPtr<FloatDataPoint*> mpCurrentAsymmetry;
    SubjectPtr<FloatDataPoint*> mpCosPhi;
    SubjectPtr<FloatDataPoint*> mpPower;
    SubjectPtr<FloatDataPoint*> mpEnergy;
    SubjectPtr<FloatDataPoint*> mpInsulationResistance;
    SubjectPtr<BoolDataPoint*>  mpTemperaturePtc;
    SubjectPtr<FloatDataPoint*> mpTemperaturePt;
    SubjectPtr<FloatDataPoint*> mpTemperatureTempcon;

    // Alarms
    SubjectPtr<AlarmDataPoint*> mpMP204AlarmObj;  // To allow changing of alarm code since only one object covers all alarms
    AlarmDelay* mpMP204AlarmDelay[NO_OF_MP204_FAULT_OBJ];
    bool mMP204AlarmDelayCheckFlag[NO_OF_MP204_FAULT_OBJ];

    // Local variables
    U32 mSkipRunCounter;
    ALARM_ID_TYPE mExistingAlarmCode;
    U32           mExistingWarnings;

    GeniSlaveIf* mpGeniSlaveIf;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
   /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
