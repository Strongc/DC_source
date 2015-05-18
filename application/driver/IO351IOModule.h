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
/* CLASS NAME       : IO351IOModule                                         */
/*                                                                          */
/* FILE NAME        : IO351IOModule.h                                       */
/*                                                                          */
/* CREATED DATE     : 04-03-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : IO 351 type C driver                            */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __IO351_IO_MODULE_H__
#define __IO351_IO_MODULE_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <IIntegerDataPoint.h>
#include <U32DataPoint.h>
#include <U16DataPoint.h>
#include <U8DataPoint.h>
#include <FloatDataPoint.h>
#include <AlarmDataPoint.h>
#include <EventDataPoint.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <HttpResponse.h>
#include <AlarmDelay.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <io351.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
const int IO351_IOM_NO_OF_COUNTERS = 2;
const int IO351_IOM_NO_OF_DIG_IN = 9;
const int IO351_IOM_NO_OF_DIG_OUT = 7;
const int IO351_IOM_NO_OF_ANA_IN = 2;

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  FIRST_IO_MODULE_CONFIG_STATE,

  IOM_CONFIG_STATE_CHECK_CONFIG = FIRST_IO_MODULE_CONFIG_STATE,
  IOM_CONFIG_STATE_READ_CONFIG,
  IOM_CONFIG_STATE_SEND_CONFIG,
  IOM_CONFIG_STATE_DELAY,
  IOM_CONFIG_STATE_CONFIG_DONE,

  // insert state above
  NO_OF_IO_MODULE_CONFIG_STATE,
  LAST_IO_MODULE_CONFIG_STATE = NO_OF_IO_MODULE_CONFIG_STATE - 1
} IO_MODULE_CONFIG_STATE_TYPE;

typedef enum
{
  IO_MODULE_DIG_OUT_STATE_INITIAL,
  IO_MODULE_DIG_OUT_STATE_SET,
  IO_MODULE_DIG_OUT_STATE_RESET,
  IO_MODULE_DIG_OUT_STATE_IS_SET,
  IO_MODULE_DIG_OUT_STATE_IS_RESET
} IO_MODULE_DIG_OUT_STATE_TYPE;

/* Enum for alarms in this module */
typedef enum
{
  FIRST_IO351_FAULT_OBJ,
  IO351_FAULT_OBJ_GENI_COMM = FIRST_IO351_FAULT_OBJ,
  IO351_FAULT_OBJ_HW_ERROR,

  NO_OF_IO351_FAULT_OBJ,
  LAST_IO351_FAULT_OBJ = NO_OF_IO351_FAULT_OBJ - 1
}IO351_FAULT_OBJ_TYPE;

/*****************************************************************************
  FORWARDS
 *****************************************************************************/
class GeniSlaveIf;

/*****************************************************************************
 * CLASS: IO351IOModule
 * DESCRIPTION: IO 351 type C driver
 *****************************************************************************/
class IO351IOModule : public IO351, public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Constructor
    ********************************************************************/
    IO351IOModule(const IO351_NO_TYPE moduleNo);

    /********************************************************************
    I0351Module - Destructor
    ********************************************************************/
    ~IO351IOModule();

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
    void SubscribtionCancelled(Subject* pSubject){};

    // SubTask
    virtual void InitSubTask(void);
    virtual void RunSubTask();

    // WebIfHandler
    void webifGetNoOfIOModules(std::ostream& res);
    void webifSetNoOfIOModules(int noOfIOModules);
    void webifMakeStatusHtml(HttpResponse& res);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void HandleIoModuleConfig();
    void DetermineIoModuleConfig();
    void HandleIoConfigDataPoints();
    void HandleDigitalInputCounters();
    void HandleDigitalInputs();
    void HandlePTCInputs();
    void HandleDigitalOutputs();
    void HandleAnalogInputs();
    void HandleChangeOfAiSettings();
    U8 SelectComCardForDemoSuitcase();

    // IO351 class overrides
    virtual void ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType);

    /********************************************************************
    ATTRIBUTES
    ********************************************************************/

    /* Variables for alarm handling */
    AlarmDelay* mpIO351AlarmDelay[NO_OF_IO351_FAULT_OBJ];
    bool mIO351AlarmDelayCheckFlag[NO_OF_IO351_FAULT_OBJ];

    SubjectPtr<IIntegerDataPoint*> mpNoOfIo351;
    SubjectPtr<BoolDataPoint*> mpModuleReady;
    SubjectPtr<EventDataPoint*> mpRestartedEvent;

    // digital input DPs
    SubjectPtr<U32DataPoint*> mpCounterValue[IO351_IOM_NO_OF_COUNTERS];
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*> mpDigInConfDigitalInputFunc[IO351_IOM_NO_OF_DIG_IN];
    SubjectPtr<U16DataPoint*> mpDigInStatusBits;

    // PTC input DPs
    SubjectPtr<U32DataPoint*> mpPtcBits;

    // digital output DPs
    SubjectPtr<U8DataPoint*> mpDigOutStatusBits;
    SubjectPtr<EnumDataPoint<RELAY_FUNC_TYPE>*> mpDigOutConfRelayFunc[IO351_IOM_NO_OF_DIG_OUT];

    // analog input DPs
    SubjectPtr<EnumDataPoint<SENSOR_ELECTRIC_TYPE>*> mpAnaInConfSensorElectric[IO351_IOM_NO_OF_ANA_IN];
    SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*> mpAnaInConfMeasuredValue[IO351_IOM_NO_OF_ANA_IN];
    SubjectPtr<FloatDataPoint*> mpAnaInValue[IO351_IOM_NO_OF_ANA_IN];
    SubjectPtr<AlarmDataPoint*> mpAnaInAlarmObj[IO351_IOM_NO_OF_ANA_IN];

    // Demo suitcase
    SubjectPtr<BoolDataPoint*> mpDemoSuitcaseModeEnabled;
    SubjectPtr<EnumDataPoint<COM_CARD_TYPE>*> mpCommunicationCard;

    GeniSlaveIf* mpGeniSlaveIf;
    bool mThisIoModuleExpectedFlag;
    bool mPowerOnCntValidFlag;
    U16 mPowerOnCntActual;
    bool mConfigReceivedFlag;
    bool mDoConfigFlag;
    bool mTimeOutFlag;
    bool mCheckDigOutTimeout;
    IO_MODULE_CONFIG_STATE_TYPE mIoModuleConfigState;
    IO_MODULE_DIG_OUT_STATE_TYPE mDigOutState[IO351_IOM_NO_OF_DIG_OUT];

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
   /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
