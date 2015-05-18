/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : AlarmConfig                                           */
/*                                                                          */
/* FILE NAME        : AlarmConfig.h                                         */
/*                                                                          */
/* CREATED DATE     : 30-10-2007 (dd-mm-yyyy)                               */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_AlarmConfig_h
#define mrc_AlarmConfig_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <INumberDataPoint.h>
#include <AppTypeDefs.h>
#include <Subject.h>
#include <Observer.h>


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
  AC_DIGITAL,
  AC_INTEGER,
  AC_FLOAT
}AC_TYPE;

typedef struct
{
  bool AlarmEnabled;
  bool WarningEnabled;
  bool Sms1Enabled;
  bool Sms2Enabled;
  bool Sms3Enabled;
  bool Scada;
  bool CustomAlarmRelay;
  bool CustomWarningRelay;
  bool AlarmAutoAck;
  U32  WarningLimit;
  U32  AlarmLimit;
  float AlarmDelay;
  AC_TYPE AlarmConfigType;
} ALARM_CONFIG_GENI_TYPE;

/*****************************************************************************
 * CLASS: AlarmConfig
 *****************************************************************************/
class AlarmConfig : public Subject, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmConfig(SUBJECT_TYPE limitType, ALARM_CRITERIA_TYPE criteria);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AlarmConfig();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    AlarmConfig& operator=(const AlarmConfig& src);

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool GetSms1Enabled();
    bool GetSms2Enabled();
    bool GetSms3Enabled();
    bool GetScadaEnabled();
    bool GetCustomRelayForAlarm();
    bool GetCustomRelayForWarning();
    bool GetAlarmEnabled();
    bool GetWarningEnabled();
    bool GetAutoAcknowledge();
    float  GetAlarmDelay();
    bool GetLimitTypeIsFloat();
    AC_TYPE GetAlarmConfigType();

    INumberDataPoint* GetWarningLimit();
    INumberDataPoint* GetAlarmLimit();
    ALARM_CRITERIA_TYPE GetAlarmCriteria();

    bool SetSms1Enabled(bool smsEnabled);
    bool SetSms2Enabled(bool smsEnabled);
    bool SetSms3Enabled(bool smsEnabled);
    bool SetScadaEnabled(bool scadaEnabled);
    bool SetCustomRelayForAlarm(bool customRelayEnabledForAlarm);
    bool SetCustomRelayForWarning(bool customRelayEnabledForWarning);
    bool SetAlarmEnabled(bool alarmEnabled);
    bool SetWarningEnabled(bool warningEnabled);
    bool SetAutoAcknowledge(bool autoAcknowledge);
    bool SetAlarmDelay(float alarmDelayMs);
    bool SetAlarmConfigType(AC_TYPE alarmType);
    bool SetWarningLimit(float warningLimit);
    bool SetWarningLimit(int warningLimit);
    bool SetAlarmLimit(float alarmLimit);
    bool SetAlarmLimit(int alarmLimit);

    void SetMinLimit(float minLimit);
    void SetMinLimit(int minLimit);
    void SetMaxLimit(float maxLimit);
    void SetMaxLimit(int maxnLimit);
    void SetQuantity(QUANTITY_TYPE quantity);

    virtual FLASH_ID_TYPE GetFlashId(void);
  	virtual void SaveToFlash(IFlashWriter* pWrite, FLASH_SAVE_TYPE save);
  	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs);
    void AlarmConfigFromGeni(ALARM_CONFIG_GENI_TYPE *pAc);
    void AlarmConfigToGeni(ALARM_CONFIG_GENI_TYPE *pAc);

    virtual void SubscribtionCancelled(Subject* pSubject){};   //implementation not needed
    virtual void Update(Subject* pSubject);
    virtual void SetSubjectPointer(int Id,Subject* pSubject){};//implementation not needed
    virtual void ConnectToSubjects(void){};                    //implementation not needed

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    bool mSms1Enabled;
    bool mSms2Enabled;
    bool mSms3Enabled;
    bool mScadaEnabled;
    bool mCustomRelayForAlarmEnabled;
    bool mCustomRelayForWarningEnabled;
    bool mAlarmEnabled;
    bool mWarningEnabled;
    bool mAutoAck;
    float mAlarmDelay;
    INumberDataPoint* mpAlarmLimit;
    INumberDataPoint* mpWarningLimit;
    AC_TYPE mAlarmConfigType;

    // the following attributes are only set by factory
    ALARM_CRITERIA_TYPE mCriteria;

    bool mNotifyObserversEnabled;
    bool mLimitDatapointsChangedByAlarmConfig;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
