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
/* CLASS NAME       : SmsAlarm                                              */
/*                                                                          */
/* FILE NAME        : SmsAlarm.h                                            */
/*                                                                          */
/* CREATED DATE     : 06-12-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Generates alarm sms at the right times          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcSmsAlarm_h
#define mrcSmsAlarm_h

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
#include <AlarmLog.h>
#include <AppTypeDefs.h>
#include <EnumDataPoint.h>
#include <EventDataPoint.h>
#include <TimeText.h>
#include <Factory.h>
#include <BoolDataPoint.h>

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
   SMS_PRI_SEC,             //Primary or Secondry   
   SMS_DIRECT               //Direct request

 }SMS_REQUEST_TYPE;

 /*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class SmsAlarm : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SmsAlarm();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~SmsAlarm();
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
    void HandelScheduledAlarmSms();
    void HandelCmdEventAlarmSms();
    void SendAlarmSms( AlarmEvent*, SMS_REQUEST_TYPE reqType);
    STRING_ID GetUnitString( ERRONEOUS_UNIT_TYPE type, int number );
    bool CheckCurrentSmsCategory( AlarmEvent* pAlarmEvent );
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<AlarmLog*> mpAlarmLog;
    SubjectPtr<EnumDataPoint<SMS_CATEGORY_TYPE>*> mpCurrentSmsCategory;
    SubjectPtr<EnumDataPoint<SMS_RECIPIENT_TYPE>*> mpSmsRecipient;
    SubjectPtr<EventDataPoint*>  mpGetAlarmsCmdEvent;

    mpc::display::TimeText* mpTimeText;  

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
