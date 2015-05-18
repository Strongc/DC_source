/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/*                                                                          */
/* FILE NAME        : PcSimulatorService.h                                  */
/*                                                                          */
/* CREATED DATE     : 28-07-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_PcSimulatorService_h
#define mrc_PcSimulatorService_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <StringDataPoint.h>
#include <FloatDataPoint.h>
#include <EventDataPoint.h>
#include <EnumDataPoint.h>
#include <U8DataPoint.h>
#include <BoolDataPoint.h>
#include <U32DataPoint.h>
#include <IO351.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
struct DiParameters
{
  int index;
  int isHigh;
};

struct AiParameters
{
  int index;
  float value;
};

struct AlarmParameters
{
  int alarmDataPointId;
  bool alarmActive;
  bool warningActive;
};

struct Io111Parameters
{
  int   moduleNo;
  float temperature;
  float temperaturePt100;
  float temperaturePt1000;
  float temperatureMainBearing;
  float temperatureSupportBearing;
  float waterInOil;
  float resistance;
  float vibration;
  int  moisture;
  int  thermalSwitch;
};

#define NO_OF_AI (3 + NO_OF_IO351_IOM_NO * 2)
#define NO_OF_DI (3 + NO_OF_IO351_IOM_NO * 9)
#define NO_OF_DO (2 + NO_OF_IO351_IOM_NO * 7)

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PcSimulatorService
{
public:
  static PcSimulatorService* GetInstance(void);

  BSTR GetSimulatorValues(void);
  BSTR GetAIValues(void);
  BSTR GetDIValues(void);
  BSTR GetDOValues(void);
  BSTR GetIO111Values(int moduleNo);

  int SetDiValue(DiParameters* diParameters);
  int SetAiValueInPercent(AiParameters* aiParameters);
  int SetAiValueInInternalUnit(AiParameters* aiParameters);
  int SetIO111Values(Io111Parameters* io111Parameters);
  int SimulateAlarm(AlarmParameters* param);
 
private:
  PcSimulatorService();
  ~PcSimulatorService(){}

  static PcSimulatorService* mInstance;

  EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>* mpDpPumpOm[6];
  EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>* mpDpMixerOm;
  FloatDataPoint*   mpDpSurface;
  INumberDataPoint* mpDpHigh;
  INumberDataPoint* mpDpAlarm;
  INumberDataPoint* mpDpDryrun;
  FloatDataPoint*   mpDpPitDepth;
  FloatDataPoint* mpDpUpper;
  FloatDataPoint* mpDpLower;
  FloatDataPoint* mpDpDelta;
  EnumDataPoint<REQ_OPERATION_MODE_TYPE>* mpDpPump1ReqOm;
  EnumDataPoint<REQ_OPERATION_MODE_TYPE>* mpDpPump2ReqOm;
  EnumDataPoint<SENSOR_TYPE_TYPE>* mpDpPitLevelCtrlType;
  EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>* mpDpFswStates[5];
  EnumDataPoint<FSW_TYPE>* mpDpFswTypes[5];
  BoolDataPoint* mpDpVfdInstalled[6];
  FloatDataPoint* mpDpVfdFrequency[6];
  
  FloatDataPoint*   mpDpAiValues[NO_OF_AI];
  EnumDataPoint<MEASURED_VALUE_TYPE>* mpDpAiFuncs[NO_OF_AI];

  BoolDataPoint* mpDpDiLogics[NO_OF_DI];
  EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>* mpDpDiStates[NO_OF_DI];
  EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>* mpDpDiFuncs[NO_OF_DI];

  BoolDataPoint* mpDpDoStates[NO_OF_DO];
  EnumDataPoint<RELAY_FUNC_TYPE>* mpDpDoFuncs[NO_OF_DO];

  EnumDataPoint<SENSOR_ELECTRIC_TYPE>* mpDpAiElec[NO_OF_AI];
  FloatDataPoint* mpDpAiMin[NO_OF_AI];
  FloatDataPoint* mpDpAiMax[NO_OF_AI];
  U32DataPoint* mpDpAiSim[6];

  FloatDataPoint* mpDpIO351AI[NO_OF_AI - 3];

  U32DataPoint* mpDpCu361DiStatus;
  U32DataPoint* mpDpIo351DiStatus[NO_OF_IO351_IOM_NO];
  FloatDataPoint* mpDpIO111Temperature[NO_OF_IO111_NO];
  FloatDataPoint* mpDpIO111WaterInOil[NO_OF_IO111_NO];
  FloatDataPoint* mpDpIO111Resistance[NO_OF_IO111_NO];
  BoolDataPoint*  mpDpIO111Moisture[NO_OF_IO111_NO];
  FloatDataPoint* mpDpIO111TemperatureMainBearing[NO_OF_IO111_NO];
  FloatDataPoint* mpDpIO111TemperaturePt100[NO_OF_IO111_NO];
  FloatDataPoint* mpDpIO111TemperaturePt1000[NO_OF_IO111_NO];
  FloatDataPoint* mpDpIO111Vibration[NO_OF_IO111_NO];
  FloatDataPoint* mpDpIO111TemperatureSupportBearing[NO_OF_IO111_NO];
  BoolDataPoint*  mpDpIO111ThermalSwitch[NO_OF_IO111_NO];

};

#endif
