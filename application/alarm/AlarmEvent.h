/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : AlarmEvent                                            */
/*                                                                          */
/* FILE NAME        : AlarmEvent.h                                          */
/*                                                                          */
/* CREATED DATE     : 17-09-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : AlarmEvent is a used as element in the AlarmLog.*/
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcAlarmEvent_h
#define mpcAlarmEvent_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <AlarmDataPoint.h>
#include <MpcTime.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AlarmDef.h>
#include <AlarmLogDataHistory.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: AlarmEvent
 *****************************************************************************/
class AlarmEvent
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmEvent(U32 alarmEventNumber);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AlarmEvent();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    AlarmEvent& operator=(const AlarmEvent& src);

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    U32 GetAlarmEventNumber();
    ALARM_ID_TYPE GetAlarmId();
    ALARM_STATE_TYPE GetAlarmType();
    bool GetAcknowledge();
    MpcTime* GetArrivalTime();
    MpcTime* GetDepartureTime();
    ERRONEOUS_UNIT_TYPE GetErroneousUnit();
    int GetErroneousUnitNumber();
    AlarmDataPoint* GetErrorSource();
    U32 GetErrorSourceSubjectId();
    void SetAlarmId(ALARM_ID_TYPE alarmId);
    void SetAlarmType(ALARM_STATE_TYPE alarmType);
    void SetAcknowledge(bool acknowledge);
    void SetArrivalTime();
    void SetArrivalTime(MpcTime* pArrivalTime);
    void SetArrivalTimeNotAvailable();
    void SetDepartureTime();
    void SetDepartureTime(MpcTime* pDepartureTime);
    void SetDepartureTimeNotAvailable();
    void SetErroneousUnit(ERRONEOUS_UNIT_TYPE erroneousUnit);
    void SetErroneousUnitNumber(int erroneousUnitNumber);
    void SetErrorSource(int errorSourceSubject);
    bool GetSmsArrivalSent(void);
    void SetSmsArrivalSent(bool);
    bool GetSmsDepartureSent(void);
    void SetSmsDepartureSent(bool);
    bool GetWatchSmsArrivalSent(void);
    void SetWatchSmsArrivalSent(bool);
    bool GetWatchSmsDepartureSent(void);
    void SetWatchSmsDepartureSent(bool);

    void SetSnapShotData(const AlarmLogDataHistory::DataHistoryEntry& data);
    const AlarmLogDataHistory::DataHistoryEntry& GetSnapShotData() const;
    
//     bool GetSurfaceLevelReady() const;
//     float GetSurfaceLevel() const;
//     float GetAverageFlow() const;
//     ACTUAL_OPERATION_MODE_TYPE GetPumpOperatingMode() const;
//     float GetPumpVoltage() const;
//     float GetPumpCurrent() const;
//     float GetPumpCosPhi() const;
//     float GetPumpPower() const;
//     float GetPumpFlow() const;
//     float GetPumpTemperature() const;

//     DP_QUALITY_TYPE GetSurfaceLevelReadyQuality() const;
//     DP_QUALITY_TYPE GetSurfaceLevelQuality() const;
//     DP_QUALITY_TYPE GetAverageFlowQuality() const;
//     DP_QUALITY_TYPE GetPumpOperatingModeQuality() const;
//     DP_QUALITY_TYPE GetPumpVoltageQuality() const;
//     DP_QUALITY_TYPE GetPumpCurrentQuality() const;
//     DP_QUALITY_TYPE GetPumpCosPhiQuality() const;
//     DP_QUALITY_TYPE GetPumpPowerQuality() const;
//     DP_QUALITY_TYPE GetPumpFlowQuality() const;
//     DP_QUALITY_TYPE GetPumpTemperatureQuality() const;

//     void SetSurfaceLevelReady(bool);
//     void SetSurfaceLevel(float);
//     void SetAverageFlow(float);
//     void SetPumpOperatingMode(ACTUAL_OPERATION_MODE_TYPE);
//     void SetPumpVoltage(float);
//     void SetPumpCurrent(float);
//     void SetPumpCosPhi(float);
//     void SetPumpPower(float);
//     void SetPumpFlow(float);
//     void SetPumpTemperature(float);

//     void SetSurfaceLevelReadyQuality(DP_QUALITY_TYPE);
//     void SetSurfaceLevelQuality(DP_QUALITY_TYPE);
//     void SetAverageFlowQuality(DP_QUALITY_TYPE);
//     void SetPumpOperatingModeQuality(DP_QUALITY_TYPE);
//     void SetPumpVoltageQuality(DP_QUALITY_TYPE);
//     void SetPumpCurrentQuality(DP_QUALITY_TYPE);
//     void SetPumpCosPhiQuality(DP_QUALITY_TYPE);
//     void SetPumpPowerQuality(DP_QUALITY_TYPE);
//     void SetPumpFlowQuality(DP_QUALITY_TYPE);
//     void SetPumpTemperatureQuality(DP_QUALITY_TYPE);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    U32 mAlarmEventNumber;
    ALARM_ID_TYPE mAlarmId;
    ALARM_STATE_TYPE mAlarmType;
    bool mAcknowledge;
    bool mSmsArrivalSent;
    bool mSmsDepartureSent;
    bool mWatchSmsArrivalSent;
    bool mWatchSmsDepartureSent;    
    MpcTime mArrivalTime;
    MpcTime mDepartureTime;
    ERRONEOUS_UNIT_TYPE mErroneousUnit;
    int mErroneousUnitNumber;
    int mAlarmDataPointSubjectId;

    AlarmLogDataHistory::DataHistoryEntry mSnapShotData;
//     bool mSurfaceLevelReady;
//     float mSurfaceLevel;
//     float mAverageFlow;
//     ACTUAL_OPERATION_MODE_TYPE mPumpOperatingMode;
//     float mPumpVoltage;
//     float mPumpCurrent;
//     float mPumpCosPhi;
//     float mPumpPower;
//     float mPumpFlow;
//     float mPumpTemperature;

//     DP_QUALITY_TYPE mSurfaceLevelReadyQuality;
//     DP_QUALITY_TYPE mSurfaceLevelQuality;
//     DP_QUALITY_TYPE mAverageFlowQuality;
//     DP_QUALITY_TYPE mPumpOperatingModeQuality;
//     DP_QUALITY_TYPE mPumpVoltageQuality;
//     DP_QUALITY_TYPE mPumpCurrentQuality;
//     DP_QUALITY_TYPE mPumpCosPhiQuality;
//     DP_QUALITY_TYPE mPumpPowerQuality;
//     DP_QUALITY_TYPE mPumpFlowQuality;
//     DP_QUALITY_TYPE mPumpTemperatureQuality;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
