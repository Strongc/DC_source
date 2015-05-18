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
/* CLASS NAME       : WatchSmsAlarm                                         */
/*                                                                          */
/* FILE NAME        : WatchSmsAlarm.h                                       */
/*                                                                          */
/* CREATED DATE     : 10-08-2012 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Generates watch alarm sms when SCADA is down.   */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcWatchSmsAlarm_h
#define mrcWatchSmsAlarm_h

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
/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class WatchSmsAlarm : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    WatchSmsAlarm();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~WatchSmsAlarm();
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
    void HandelWatchAlarmSms();
    void SendAlarmSms( AlarmEvent*);
    STRING_ID GetUnitString( ERRONEOUS_UNIT_TYPE type, int number );    
    void SendWatchSms(AlarmEvent* pAlarmEvent, bool sendWatchSms);    
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<AlarmLog*>                        mpAlarmLog;
    SubjectPtr<EventDataPoint*>                  mpScadaStateUpdated;
    SubjectPtr<BoolDataPoint*>                   mpScadaWatchEnabled;
    SubjectPtr<BoolDataPoint*>                   mpScadaCallbackEnabled;
    SubjectPtr<AlarmDataPoint*>                  mpScadaCallbackAlarmObj;
    SubjectPtr<EnumDataPoint<SCADA_STATE_TYPE>*> mpScadaState;

    mpc::display::TimeText*    mpTimeText;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
