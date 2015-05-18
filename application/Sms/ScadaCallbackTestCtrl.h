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
/* CLASS NAME       : ScadaCallbackTestCtrl                                 */
/*                                                                          */
/* FILE NAME        : ScadaCallbackTestCtrl.h                               */
/*                                                                          */
/* CREATED DATE     : 12-07-2012 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Tests SCADA call back functionality             */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcScadaCallbackTestCtrl_h
#define mrcScadaCallbackTestCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Languages.h>
#include <EnumDataPoint.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>
#include <StringDataPoint.h>
#include <AlarmDelay.h>
#include <SubjectPtr.h>
#include <EventDataPoint.h>

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
  FIRST_SCBTC_ALARM_OBJ,
  SCBTC_ALARM_OBJ_CALLBACK_TEST = FIRST_SCBTC_ALARM_OBJ,

  NO_OF_SCBTC_ALARM_OBJ,
  LAST_SCBTC_ALARM_OBJ = NO_OF_SCBTC_ALARM_OBJ - 1
} SCBTC_ALARM_OBJ_TYPE;

typedef enum
{
  FIRST_CALLBACK_TEST_STATE = 0,
  CALLBACK_TEST_STATE_NOT_STARTED = FIRST_CALLBACK_TEST_STATE,
  CALLBACK_TEST_STATE_WAITING_ACK,
  CALLBACK_TEST_STATE_ACK_FAILED,
  CALLBACK_TEST_STATE_SENT_FAILED,
  CALLBACK_TEST_STATE_SUCCESS,

  // insert states above
  NO_OF_CALLBACK_TEST_STATES,
  LAST_CALLBACK_TEST_STATE = NO_OF_CALLBACK_TEST_STATES - 1
}CALLBACK_TEST_STATE_TYPE;


 /*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class ScadaCallbackTestCtrl : public SubTask,  public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    ScadaCallbackTestCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~ScadaCallbackTestCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Subject.
    void SetSubjectPointer( int Id, Subject* pSubject );
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled( Subject* pSubject );

    // SubTask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void RaiseCallbackTestAlarm(bool alarmRequest);
    void UpdateStatusDisplay();
    void InitSettings();
    void CheckIsAlarmStabilized();
    void CheckCallbackStatus();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<U32DataPoint*>                    mpTimeStampLastScadaAck;
    SubjectPtr<BoolDataPoint*>                   mpCallbackTestEnabled;
    SubjectPtr<StringDataPoint*>                 mpCallbackTestReqString;
    SubjectPtr<StringDataPoint*>                 mpCallbackTestAckString;
    SubjectPtr<EnumDataPoint<SCADA_STATE_TYPE>*> mpScadaState;
    SubjectPtr<BoolDataPoint*>                   mpScadaCallBackEnabled;
    AlarmDelay*                                  mpCallbaclTestAlarmDelay;
    SubjectPtr<BoolDataPoint*>                   mpCiuCardFault;
    SubjectPtr<EventDataPoint*>                  mpCallbackTestEvent;

    bool                      mCallbackTestAlarmDelayCheckFlag;
    bool                      mRunRequestedFlag;        
    bool                      mAlarmStabilizationTimerRestart;
    bool                      mCallbackAckWaitingTimerRestart;
    bool                      mAlarmStabilized;
    bool                      mStabilizationTimerStarted;
    CALLBACK_TEST_STATE_TYPE  mCallbackTestState;



  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
