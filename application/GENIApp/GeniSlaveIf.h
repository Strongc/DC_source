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
/* CLASS NAME       : GeniSlaveIf                                           */
/*                                                                          */
/* FILE NAME        : GeniSlaveIf.h                                         */
/*                                                                          */
/* CREATED DATE     : 01-04-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class definition for GeniSlaveIf                */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __GENI_SLAVE_IF_H__
#define __GENI_SLAVE_IF_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <SubTask.h>
#include <AlarmDef.h>
#include <AnaInOnIOCtrl.h>
#include <IO351.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "SlaveData.h"
#include <common.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define GSC_APDU_LEN  23                /* Space for 10 sets of id, data */
#define NO_OF_DEVICES_TO_CONFIG 2

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef struct
{
  U8 GscNo;
  GENI_DEVICE_TYPE GeniDevice;
  U8 Apdu[GSC_APDU_LEN];
} DEVICE_CONFIG_TYPE;

/*****************************************************************************
 * CLASS: GeniSlaveIf
 * DESCRIPTION: This class is a singleton and is encapsulating GENI slaves
 *
 *****************************************************************************/
class GeniSlaveIf : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/

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


    // MP204 methods
    bool ConnectMP204(IO351_NO_TYPE moduleNo);
    void DisconnectMP204(IO351_NO_TYPE moduleNo);
    void MP204Reset(IO351_NO_TYPE moduleNo);
    void MP204AlarmReset(IO351_NO_TYPE moduleNo);
    bool GetMP204AlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue);
    bool GetMP204WarningCode(IO351_NO_TYPE moduleNo, U32* pValue);
    bool GetMP204Voltage(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetMP204Current(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetMP204CurrentAsymmetry(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetMP204CosPhi(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetMP204Power(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetMP204Energy(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetMP204InsulationResistance(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetMP204TemperaturePtc(IO351_NO_TYPE moduleNo, bool* pValue);
    bool GetMP204TemperatureTempcon(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetMP204TemperaturePt(IO351_NO_TYPE moduleNo, float* pValue);


    // CUE methods
    bool ConnectCUE(IO351_NO_TYPE moduleNo);
    void DisconnectCUE(IO351_NO_TYPE moduleNo);
    void CUEReset(IO351_NO_TYPE moduleNo);
    void CUEAlarmReset(IO351_NO_TYPE moduleNo);
    void CUEStop(IO351_NO_TYPE moduleNo);
    void CUEStart(IO351_NO_TYPE moduleNo);
    void CUERemote(IO351_NO_TYPE moduleNo);
    void CUELocal(IO351_NO_TYPE moduleNo);
    void CUEConstFreq(IO351_NO_TYPE moduleNo);
    void CUEForward(IO351_NO_TYPE moduleNo);
    void CUEReverse(IO351_NO_TYPE moduleNo);
    bool GetCUEAlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue);
    bool GetCUEWarningCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue);
    bool GetCUEVoltage(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetCUEPower(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetCUEEnergy(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetCUECurrent(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetCUEFrequency(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetCUESpeed(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetCUETorque(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetCUERemoteTemperature(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetCUEOperationMode(IO351_NO_TYPE moduleNo, CUE_OPERATION_MODE_TYPE* pValue);
    bool GetCUESystemMode(IO351_NO_TYPE moduleNo, CUE_SYSTEM_MODE_TYPE* pValue);
    bool GetCUEControlMode(IO351_NO_TYPE moduleNo, CUE_LOOP_MODE_TYPE* pValue);
    bool GetCUESourceMode(IO351_NO_TYPE moduleNo, CUE_SOURCE_MODE_TYPE* pValue);
    bool GetCUEReverseStatus(IO351_NO_TYPE moduleNo, bool* pValue);
    bool SetCUEReference(IO351_NO_TYPE moduleNo, float outValue);


    // IO111 methods
    bool ConnectIO111(IO351_NO_TYPE moduleNo);
    void DisconnectIO111(IO351_NO_TYPE moduleNo);
    void IO111AlarmReset(IO351_NO_TYPE moduleNo);
    bool GetIO111TemperatureSupportBearing(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetIO111TemperatureMainBearing(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetIO111TemperaturePT100(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetIO111TemperaturePT1000(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetIO111InsulationResistance(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetIO111Vibration(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetIO111WaterInOil(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetIO111MoistureSwitch(IO351_NO_TYPE moduleNo, bool* pValue);
    bool GetIO111ThermalSwitch(IO351_NO_TYPE moduleNo, bool* pValue);
    bool GetIO111AlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue);
    bool GetIO111WarningCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue);

    bool GetIO11xUnitType(IO351_NO_TYPE moduleNo, U8* pValue);
    bool GetIO113Speed(IO351_NO_TYPE moduleNo, U16* pValue);

    // IO351 methods
    U8 GetUnitAddress(IO351_NO_TYPE moduleNo) const;
    void SetIO351Pointer(IO351_NO_TYPE moduleNo, IO351* pIO351);
    bool ConnectIO351(IO351_NO_TYPE moduleNo);
    void DisconnectIO351(IO351_NO_TYPE moduleNo);
    bool ResetIO351(IO351_NO_TYPE moduleNo);
    bool SetIO351AnalogInputConfig(IO351_NO_TYPE moduleNo, IO351_ANA_IN_NO_TYPE anaInNo, SENSOR_ELECTRIC_TYPE type);
    bool GetIO351ModuleConfig(IO351_NO_TYPE moduleNo);
    bool SetIO351ModuleConfig(IO351_NO_TYPE moduleNo, int noOfPumps, int noOfVfds, int addrOffset, int systemType);
    bool GetIO351PowerOnCnt(IO351_NO_TYPE moduleNo, U16* pValue);
    bool GetIO351AlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue);
    bool GetIO351DigitalInputCounter(IO351_NO_TYPE moduleNo, IO351_DIG_IN_NO_TYPE digInNo, U32* pCounter);
    bool GetIO351DigitalInputStatus(IO351_NO_TYPE moduleNo, U16* pStatus);
    bool GetIO351PTCStatus(IO351_NO_TYPE moduleNo, U8* pStatus);
    bool GetIO351DigitalOutputStatus(IO351_NO_TYPE moduleNo, U8* pStatus);
    bool SetIO351DigitalOutputStatus(IO351_NO_TYPE moduleNo, IO351_DIG_OUT_NO_TYPE digOutNo, bool status);
    bool GetIO351AnalogInputValue(IO351_NO_TYPE moduleNo, IO351_ANA_IN_NO_TYPE anaInNo, float* pValue);
    bool SetIO351AnalogOutput(IO351_NO_TYPE moduleNo, IO351_ANA_OUT_NO_TYPE anaOutNo, float outValue);


   // DDA methods
    bool ConnectDDA(IO351_NO_TYPE moduleNo);
    void DisconnectDDA(IO351_NO_TYPE moduleNo);
    void DDAReset(IO351_NO_TYPE moduleNo);
    void DDA_AlarmReset(IO351_NO_TYPE moduleNo);
    bool GetDDA_pressure_max(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetDDA_dosing_cap_max(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetDDA_flow_mon_dosing_cap(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetDDA_flow_mon_press(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetDDA_volume_total(IO351_NO_TYPE moduleNo, float* pValue);
    bool GetDDA_system_mode(IO351_NO_TYPE moduleNo, U8* pStatus);
    bool GetDDA_operating_mode(IO351_NO_TYPE moduleNo, U8* pStatus);
    bool GetDDA_control_mode(IO351_NO_TYPE moduleNo, U8* pStatus);
    bool GetDDA_stop_ctr_state(IO351_NO_TYPE moduleNo, U8* pStatus);
    bool GetDDA_ctr_source(IO351_NO_TYPE moduleNo, U8* pStatus);
    bool GetDDA_pumping_state(IO351_NO_TYPE moduleNo, bool* pStatus);
    bool GetDDA_alarm_code(IO351_NO_TYPE moduleNo, U8* pStatus);
    bool GetDDA_warn_code(IO351_NO_TYPE moduleNo, U8* pStatus);
    bool SetDDA_bus_ctr_dosing_cap(IO351_NO_TYPE moduleNo, U32* pValue);
    void DDA_RequestStop(IO351_NO_TYPE moduleNo);
    void DDA_RequestStart(IO351_NO_TYPE moduleNo);
    void DDA_SetToUserMode(IO351_NO_TYPE moduleNo);
    void DDA_SetToManualDosing(IO351_NO_TYPE moduleNo);
    void DDA_SetToAnalogueDosing(IO351_NO_TYPE moduleNo);
    void DDA_SetToPulseDosing(IO351_NO_TYPE moduleNo);
    void DDA_PressStartKey(IO351_NO_TYPE moduleNo);
    void DDA_PressStopKey(IO351_NO_TYPE moduleNo);
    
   
    void IncomingIO351ModuleConfig(void);

    static GeniSlaveIf* GetInstance();

    void SetGeniError(unsigned char nw_index, unsigned char fault_state);
    void IncomingSlaveInfo(void);
    bool ChangeSlaveGeniAddress(unsigned char target_address, unsigned char new_address);

  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    GeniSlaveIf();

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    GeniSlaveIf& operator=(const GeniSlaveIf&);

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    GeniSlaveIf(const GeniSlaveIf&);
    U8   ConnectDevice(IO351_NO_TYPE moduleNo, GENI_DEVICE_TYPE device);
    void DisconnectDevice(IO351_NO_TYPE moduleNo, GENI_DEVICE_TYPE device);
    void InitSlaveData();
    void InitSlaveInfo();
    void SetSlaveData(U8* id, U8 value);
    void SetSlaveInfo(U8* id, U8 value);
    bool Scale8BitGeniValue(U8 geni_value, U8* info, float* val);
    bool Scale16BitGeniValue(U16 geni_value, U8* info, float* val);
    bool Scale24BitGeniValue(U32 geni_value, U8* info, float* val);
    bool ScaleExtendedGeniValue(U32 geni_value, U8* info, float* val, U8 no_of_bytes);
    bool GetScaleFactor(U8 info_unit, float* p_scale_factor);
    bool GetDevice(U8 unit_index, GENI_DEVICE_TYPE* device);
    void CheckForInfoRequest(void);
    void CheckForAutoPollEnableDisable(void);
    void InitDeviceConfiguration();
    void StoreDeviceConfiguration(int device_index);
    bool SendDeviceConfiguration(U8 addr, GENI_DEVICE_TYPE device);
    bool GetDeviceConfiguration(GENI_DEVICE_TYPE device, U8** p_apdu);
    void CheckForPendingUnitConfiguration();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static GeniSlaveIf* mInstance;
    bool mUnitCommError[MAX_NUMBER_OF_UNITS];
    bool mUnitInfoWanted[MAX_NUMBER_OF_UNITS];
    bool mUnitInfoReceived[MAX_NUMBER_OF_UNITS];
    bool mUnitConfigPending[MAX_NUMBER_OF_UNITS];
    bool mInfoRequestSent;
    int mNextInfoToRequest;
    int mInfoUnitIndex;

    IO351* mpIO351IOModules[NO_OF_IO351_IOM_NO];

    GENI_DEVICE_TYPE mUnit2Device[MAX_NUMBER_OF_UNITS];
    DEVICE_CONFIG_TYPE mConfiguration[NO_OF_DEVICES_TO_CONFIG];

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif

