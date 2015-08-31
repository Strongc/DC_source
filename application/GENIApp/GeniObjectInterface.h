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
/* CLASS NAME       : GeniObjectInterface                                   */
/*                                                                          */
/* FILE NAME        : GeniObjectInterface.h                                 */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcGeniObjectInterface_h
#define mpcGeniObjectInterface_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <EventDataPoint.h>
#include <U16VectorDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define LOG_HEADER_SIZE 5                     // 5 words
#define MAX_LOG_VALUES  8000                  // Max length (words) for log series data
#define MAX_OBJ_LENGTH  (6+2*MAX_LOG_VALUES)  // Max length (bytes) for log series objects
#define NO_OF_SESSION_BUFFERS 5

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  Forward declaration
 *****************************************************************************/
 class SessionBuf;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class GeniObjectInterface : public Observer, public SubTask
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static GeniObjectInterface* GetInstance();

    void InitSubTask(void);
    void RunSubTask(void);
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects(void);
    void SetSubjectPointer(int Id, Subject* pSubject);

    void ObjectRequest(U8 id, U16 sub_id,   U8 sourceAddress); //Event from GENIpro
    void ObjectDelivered(U8 id, U16 sub_id, U8 sourceAddress); //Event from GENIpro

  private:
    /********************************************************************
    LIFECYCLE - Default constructor. Private because it is a Singleton
    ********************************************************************/
    GeniObjectInterface();
    /********************************************************************
    LIFECYCLE - Destructor. Private because it is a Singleton
    ********************************************************************/
    ~GeniObjectInterface();
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void HandleObjectRequest(void);
    void HandleObjectDelivered(void);
    bool FirstSubIdInObject(U8 id, U16 subId);
    bool GetObject(U8 id, U16 subId);
    bool HandleReceivedObj(U8 id, U16 subId);
    int FindActualSession(void);
    int FindFreeSession(void);

    void HandleAlarmConfig(int subject);
    void GetAlarmConfig(int subject);
    void SetAlarmRelayConfig(int no);
    void GetAlarmRelayConfig(int no);
    void GetCombiAlarmSourceConfig(int no);
    void SetCombiAlarmSourceConfig(int no);
    void GetCombiAlarmName(int no);
    void GetUserDefinedDigitalInputName(int no);

    void GetTimeSum(U16 subject);
    void SetTimeSum(U16 subject);
    void GetNumberU8(U16 subject);
    void SetNumberU8(U16 subject);
    void GetNumber(U16 subject);
    void SetNumber(U16 subject);

    void GetRtc(void);
    void SetRtc(void);
    void GetWizardSetup(void);
    void SetWizardSetup(void);
    void GetPumpStatus(int pump_no);
    void GetVfdStatus(int pump_no);
    void GetMp204Status(int pump_no);
    void GetControlApplication(void);
    void SetControlApplication(void);
    void GetUltrasonicSensorSetup(void);
    void SetUltrasonicSensorSetup(void);
    void GetSmsAuthentification(void);
    void SetSmsAuthentification(void);
    void GetPhoneNumber(U16 subject);
    void SetStringDataPoint(U16 subject);
    void GetCounter(U16 subject);
    void SetCounter(U16 subject);
    void GetVolumeCounter(U16 subject);
    void SetVolumeCounter(U16 subject);
    void GetCounterFloat(U16 subject);
    void SetCounterFloat(U16 subject);
    void GetInstallationName(void);
    void GetSmsOut(void);
    void SetSmsIn(void);
    void GetCimGprsSetup(void);
    void SetGsmStatus(void);
    void GetAnaInConfig(int number);
    void SetAnaInConfig(int number);
    void GetSmsNoList(void);
    void GetSmsPeriodeConfig(int no);
    void SetSmsPeriodeConfig(int no);
    void GetSmsDutyRosterConfig(int no);
    void SetSmsDutyRosterConfig(int no);
    void GetSmsBasicSetup(void);
    void SetSmsBasicSetup(void);
    void GetHeartBeatConfig(void);
    void SetHeartBeatConfig(void);
    void GetInterlockSetup(int no);
    void SetInterlockSetup(int no);
    void GetInterlockBasicSetup(void);
    void SetInterlockBasicSetup(void);
    void GetInstalledPumpSetup(void);
    void SetInstalledPumpSetup(void);
    void GetDigOutConfig(int);
    void SetDigOutConfig(int);
    void GetPtcInConfig(int);
    void SetPtcInConfig(int);
    void GetDigInConfig(int);
    void SetDigInConfig(int);
    void GetDigOutElectricalOverview(int digOutNo);
    void GetDigInElectricalOverview(int digInNo);
    void GetAnaInElectricalOverview(int anaInNo);
    void InsertDigInInObjBuf(int* j, int digInNo);
    void GetDigInFromObjBuf(int* j, int digInNo);
    void GetMixerConfig(void);
    void SetMixerConfig(void);
    void GetUserDefinedCounterConfig(int CntNo);
    void SetUserDefinedCounterConfig(int CntNo);
    void GetExtraOverFlowConfig(void);
    void SetExtraOverFlowConfig(void);
    void GetConfigSetupIDCode(void);
    void SetConfigSetupIDCode(void);
    void GetFlowCalcSetup(void);
    void SetFlowCalcSetup(void);
    void GetFoamDrainConfig(void);
    void SetFoamDrainConfig(void);
    void GetAntiSeizingConfig(void);
    void SetAntiSeizingConfig(void);
    void GetFloatSwitchConfig(void);
    void SetFloatSwitchConfig(void);
    void GetPitLevelsConfig(void);
    void SetPitLevelsConfig(void);
    void GetUnitSetup(void);
    void SetUnitSetup(void);
    void GetPumpDelayConfig(void);
    void SetPumpDelayConfig(void);
    void GetDailyEmptyingConfig(void);
    void SetDailyEmptyingConfig(void);
    void GetPitLevelCtrlType(void);
    void SetPitLevelCtrlType(void);
    void GetLanguageConfig(void);
    void SetLanguageConfig(void);
    void GetPassword(U16 subjectEnabled, U16 subjectValue);
    void SetPassword(U16 subjectEnabled, U16 subjectValue);
    void GetDateFormatConfig(void);
    void SetDateFormatConfig(void);
    void GetCounterSetup(void);
    void SetCounterSetup(void);
    void GetSystemBasicConfig(void);
    void SetSystemBasicConfig(void);
    void GetEthernetSetup(void);
    void SetEthernetSetup(void);
    void GetComCardSetup(void);
    void SetComCardSetup(void);
    void GetGprsSetup(void);
    void SetGprsSetup(void);
    void GetModbusSetup(void);
    void SetModbusSetup(void);
    void GetScadaCommSetup(void);
    void SetScadaCommSetup(void);
    void GetSimCardSetup(void);
    void SetSimCardSetup(void);
    void GetPinCode(void);
    void GetPukCode(void);
    void GetGeniNumber(U16 subject);
    void SetGeniNumber(U16 subject);

    void GetLogI32Min(U16 subject);
    void GetLogUnscaled(U16 subject);
    void GetLogEfficiency(U16 subject);
    void GetLogVolume8(U16 subject);
    void GetLogVolume16(U16 subject);
    void GetLogEnergy(U16 subject);
    void GetLogFlow(U16 subject);
    void GetLogCurrent(U16 subject);

    void GetLogSeriesConfig(U16 subId);
    void SetLogSeriesConfig(U16 subId);
    void GetLogSeriesSurvey(void);
    U16  InsertLogSeriesDataHeader(U16 logItem, U16 sampleRate, U16 noOfSamples, U32 timeStamp);
    U16  GetLogValue(int offset);
    bool GetLogValues(int offset, U16 no_of_values, U8 *p_destination);
    void GetLogSeriesData(U16 subId);

    void GetPitStatus(void);
    void GetAlarmLog(void);
    void GetEventLog(void);
    void GetGsmGprsStatus(void);
    void GetBatteryStatus(void);
    void GetSystemStatus(void);
    void GetAdvFlowCalculationStatus(void);
    void GetSpecificEnergyStatus(int pump_no);
    void GetAntiBlockingStatus(int pump_no);

    void InsertObjectHeader(int len, U16 type, U8 version);
    void InsertBoolInObjBuf(int *i, bool value);
    void InsertByteInObjBuf(int *i, U8 value);
    void InsertWordInObjBuf(int *i, U16 value);
    void InsertU32InObjBuf(int *i, U32 value);

    bool GetBoolFromObjBuf(int *i);
    U16 GetWordFromObjBuf(int *i);
    U32 GetU32FromObjBuf(int *i);
    I32 GetI32FromObjBuf(int *i);
    float GetFloatFromObjBuf(int *i);

    void InsertFloatInObjBuf(int *i, U16 subject, bool checkQuality);
    void GetFloatFromObjBuf(int *i, U16 subject);
    void InsertU32InObjBuf(int *i, U16 subject, bool checkQuality);
    void GetU32FromObjBuf(int *i, U16 subject);
    void InsertU16InObjBuf(int *i, U16 subject, bool checkQuality);
    void GetU16FromObjBuf(int *i, U16 subject);
    void InsertU8InObjBuf(int *i, U16 subject, bool checkQuality);
    void GetU8FromObjBuf(int *i, U16 subject);
    void InsertBoolInObjBuf(int *i, U16 subject, bool checkQuality);
    void GetBoolFromObjBuf(int *i, U16 subject);
    void InsertI32InObjBuf(int *i, U16 subject, bool checkQuality);
    void GetI32FromObjBuf(int *i, U16 subject);
    void InsertPhoneNumberObjBuf(int *i, U16 subject, bool checkQuality);
    void GetPhoneNumberFromObjBuf(int *i, U16 subject);
    void InsertEnumInObjBuf(int *i, U16 subject, bool checkQuality);
    void GetEnumFromObjBuf(int *i, U16 subject);
    void InsertStringInObjBuf(int *i, U16 subject, U16 length);
    void GetStringFromObjBuf(int *i, U16 subject, U16 length);
    void GetIntString(int int_str);
    void GetString(const char *str);
    void GetU32DpString(U16);
    void InsertI32As16InObjBuf (int *i, U16 subject, bool checkQuality);
    void GetI32As16FromObjBuf (int *i, U16 subject);
    void GetU32As16FromObjBuf(int *i, U16 subject);
    void InsertAlarmLimitInBuf(int *i, U16 subject);
    void GetScadaNumber(void);
    void GetSmsServiceCenterNumber(void);

    void CheckSmsAlarmStringsState(void);

    void GetBasicPumpSetup(int pumpNo);
    void SetBasicPumpSetup(int pumpNo);

    void GetVfdSetup(int pumpNo);
    void SetVfdSetup(int pumpNo);

    void GetSpecificEnergyTestSetup(int pumpNo);
    void SetSpecificEnergyTestSetup(int pumpNo);

    void GetAntiBlockingSetup();
    void SetAntiBlockingSetup();

    void GetH2SControlSetup();
    void SetH2SControlSetup();

    void GetBasicPumpGroupSetup();
    void SetBasicPumpGroupSetup();

    void GetAnalogueOutputSetup(int number);
    void SetAnalogueOutputSetup(int number);

    void GetUserIoSetup(int channel);
    void SetUserIoSetup(int channel);

    void GetAnalogueOutElectricalOverview(int no);
    void GetUserIoElectricalOverview(int no);

    void GetFirmwareInfo();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EventDataPoint*> mpSmsAlarmStringsReadEvent;
    SubjectPtr<EventDataPoint*> mpSmsAlarmNewStringsEvent;
    SubjectPtr<EventDataPoint*> mpNoListReadEvent;
    SubjectPtr<EventDataPoint*> mpScadaNumberReadEvent;
    SubjectPtr<BoolDataPoint*>  mpAnaInSetupFromGeniFlag;
    SubjectPtr<BoolDataPoint*>  mpAnaOutSetupFromGeniFlag;
    SubjectPtr<EventDataPoint*> mpGprsSetupReadEvent;
    U16VectorDataPoint*         *mpLogSeriesData;

    static GeniObjectInterface* mInstance;
    U8 mId;
    U16 mSubId;
    U8 mMasterAddress;
    U32 mSecTimer;
    U32 mSecTimerPre;
    U8* mpObjectBufferAct;
    bool mObjectError;
    bool mBatEmptyStringRead;
    bool mChangeBatStringRead;
    bool mNoPowerStringRead;
    bool mNoConnectStringRead;
    bool mPowerOnStringRead;
    bool mMainsReturnedStringRead;
    SessionBuf* mpSessionBuf[NO_OF_SESSION_BUFFERS];
    U16  mTempWordArray[MAX_LOG_VALUES];

    protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

 /*****************************************************************************
 * CLASS:  SessionBuf
 * DESCRIPTION: Controls a object session
 *****************************************************************************/
class SessionBuf
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SessionBuf();

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~SessionBuf(){};

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void CopyData(U8* data);
    void CalcNumberOfSubIds(void);
    void CalcLastSubIdId(void);
    bool LastSubIdInObject();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    int mObjLength;                     // Length of the actual object
    int mId;                            // Id for the actual object
    int mSubId;                         // First SubId for the object
    int mLastSubId ;                    // Last sub id in the actual object, used to validation
    int mNextExpectedSubId;             // Next sub id in sequence, only used in receive object
    bool mBusy;                         // Tells if the session is in use
    U8 mData[MAX_OBJ_LENGTH];           // Object buffer for the session
    U32 mDataIndex;                     // Index used to receive data
    U8 mAddress;                        // Address of the master, uses to validate object
    U32 mSesTimer;                      // Timer used to control session timeout

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    int mNumberOfSubIds;                // Number of sub ids in the actual object

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

};


#endif
