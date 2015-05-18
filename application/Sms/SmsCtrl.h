/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : SmsCtrl                                               */
/*                                                                          */
/* FILE NAME        : SmsCtrl.h                                             */
/*                                                                          */
/* CREATED DATE     : 30-11-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Interprete incomming sms                        */
/*                          Controls transmitting of outgoing sms           */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcSmsCtrl_h
#define mrcSmsCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <subtask.h>
#include <Sms.h>
#include <rtos.h>
#include <U32DataPoint.h>
#include <U16DataPoint.h>
#include <U8DataPoint.h>
#include <BoolDataPoint.h>
#include <FloatDataPoint.h>
#include <StringDataPoint.h>
#include <EventDataPoint.h>
#include <EnumDataPoint.h>


/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define SMS_BUF_SIZE 24  //Max number og smses in quque

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

 typedef enum
 {
   SMS_IDLE,
   SMS_SYNCRONIZING,
   SMS_TRANSMITTING

 }SMS_SENDER_STATE_TYPE;

 /*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class SmsCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SmsCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~SmsCtrl();
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

    void SendSms(SmsOut* aSms);
    void SendDirectAlarmSms(SmsOut* aSms);
    void SendWatchAlarmSms(SmsOut* aSms);

    static SmsCtrl* GetInstance();
    void AckSms(void);
    SmsOut* GetActiveSms(void);
    void HandleSmsIn( SmsIn* );

    SmsOut* GetStatusSms( int );

    // SubTask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void strupr(char* pStr);
    void strnupr(char* pStr, unsigned int len);
    const char* GetInstallationName();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<U32DataPoint*>    mpSmsSentId;
    SubjectPtr<U32DataPoint*>    mpSmsOutSeqNumber;
    SubjectPtr<BoolDataPoint*>   mpPendingSms;
    SubjectPtr<StringDataPoint*> mpPrimaryNumber;
    SubjectPtr<StringDataPoint*> mpSecondaryNumber;
    SubjectPtr<StringDataPoint*> mpWatchPhoneNumber;
    SubjectPtr<StringDataPoint*> mpInstallationName;
    SubjectPtr<StringDataPoint*> mpSmsInstallationName;
    SubjectPtr<EventDataPoint*>  mpAlarmResetEvent;
    SubjectPtr<EventDataPoint*>  mpAutoEvent;
    SubjectPtr<EventDataPoint*>  mpInterlockEvent;
    SubjectPtr<U32DataPoint*>    mpInterlockTimeout;
    SubjectPtr<EventDataPoint*>  mpGetAlarmsCmdEvent;
    SubjectPtr<FloatDataPoint*>  mpSurfaceLevel;
    SubjectPtr<U32DataPoint*>    mpRetransTime;
    SubjectPtr<EnumDataPoint<SMS_AUTHORISE_METHOD_TYPE>*> mpSmsAuthoriseMethod;
    SubjectPtr<U16DataPoint*>    mpPinCode;
	SubjectPtr<BoolDataPoint*>   mpblockreplySms;
    SubjectPtr<BoolDataPoint*>   mpCustomRelay;
    SubjectPtr<EnumDataPoint<GSM_SIGNAL_LEVEL_TYPE>*> mpSignalLevel;

    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mpPitLevelCtrlType;
    SubjectPtr<U8DataPoint*>     mpNoOfPumps;

    /* Measured data */
    SubjectPtr<FloatDataPoint*> mpSysFlow;
    SubjectPtr<FloatDataPoint*>   mpSysVolume;
    SubjectPtr<U32DataPoint*>   mpFloatSwitchWaterLevel;
    SubjectPtr<U32DataPoint*>   mpPumpOperatingTime[MAX_NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>   mpPumpNoOfStarts[MAX_NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpPumpFlow[MAX_NO_OF_PUMPS];

    /* Settable data */
    SubjectPtr<FloatDataPoint*> mpAnaOutUser1;
    SubjectPtr<FloatDataPoint*> mpAnaOutUser2;
    SubjectPtr<FloatDataPoint*> mpAnaOutUser3;
    
    SmsOut* mSmsBuf[SMS_BUF_SIZE];
    SmsOut* mpStatusSms;
    char mAlarmSmsPhoneNumber[PHONO_NO_MAX_LENGTH+1];
    OS_RSEMA mSemSmsBuf;
    int mActiveSms;
    SMS_SENDER_STATE_TYPE mSenderState;
    bool mPendingSms;
    static SmsCtrl* mInstance;
    int mSmsMaxCntAccount;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
