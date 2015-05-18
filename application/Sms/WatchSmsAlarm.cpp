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
/* FILE NAME        : WatchSmsAlarm.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 10-08-2012 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AlarmText.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <WatchSmsAlarm.h>
#include <SmsCtrl.h>
#include <ActualAlarmString.h>
#include <TimeFormatDataPoint.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
WatchSmsAlarm::WatchSmsAlarm()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
WatchSmsAlarm::~WatchSmsAlarm()
{
  delete mpTimeText;
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void WatchSmsAlarm::InitSubTask()
{
  mpTimeText = new mpc::display::TimeText();
  TimeFormatDataPoint* pDpTimePreference = TimeFormatDataPoint::GetInstance();
  mpTimeText->SetSubjectPointer(0,pDpTimePreference);
  mpTimeText->ConnectToSubjects();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void WatchSmsAlarm::RunSubTask()
{
  //Enable SCADA callback for SCADA callback error alarm
  if (mpScadaWatchEnabled.IsUpdated())
  {
    if (mpScadaWatchEnabled->GetValue())
    {
      mpScadaCallbackAlarmObj->GetAlarmConfig()->SetAlarmEnabled(true);
      mpScadaCallbackAlarmObj->GetAlarmConfig()->SetScadaEnabled(true);
    }
    mpScadaWatchEnabled.SetUpdated();
  }

  /* Check if a watch alarm sms has to be sent */ 
  if (mpScadaStateUpdated.IsUpdated() ||
      mpScadaWatchEnabled.IsUpdated() ||
      mpScadaCallbackEnabled.IsUpdated())
  {
    HandelWatchAlarmSms(); 
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void WatchSmsAlarm::ConnectToSubjects()
{
  mpScadaStateUpdated->Subscribe(this);
  mpScadaWatchEnabled->Subscribe(this);
  mpScadaCallbackEnabled->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void WatchSmsAlarm::Update(Subject* pSubject)
{
  mpScadaStateUpdated.Update(pSubject);
  mpScadaWatchEnabled.Update(pSubject);
  mpScadaCallbackEnabled.Update(pSubject);
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void WatchSmsAlarm::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void WatchSmsAlarm::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    //inputs
    case SP_WSA_ALARM_LOG:
      mpAlarmLog.Attach(pSubject);
      break;
    case SP_WSA_SCADA_CALLBACK_ENABLED:
      mpScadaCallbackEnabled.Attach(pSubject);
      break;
    case SP_WSA_SCADA_WATCH_ENABLED:
      mpScadaWatchEnabled.Attach(pSubject);
      break;
    case SP_WSA_SCADA_STATE:
      mpScadaState.Attach(pSubject);
      break;
    case SP_WSA_SCADA_STATE_UPDATED:
      mpScadaStateUpdated.Attach(pSubject);
      break;

      //outputs
    case SP_WSA_SCADA_CALLBACK_ALARM_OBJ:
      mpScadaCallbackAlarmObj.Attach(pSubject);
      break;
    default:
      break;
  }
}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - SendAlarmSms
 * DESCRIPTION: Create an send an alarm sms
 *
 *****************************************************************************/
void WatchSmsAlarm::SendAlarmSms(AlarmEvent* pAlarmEvent)
{
  #ifdef STR_MAX
    #error"AlarmSms"
  #else
    #define STR_MAX 900
  #endif
  char s1[STR_MAX]; //En sms må ikke være mere end 210tegn(tegn=1-4bytes), udfør evt tjek i nedenstående der sikrer mod overindeksering
  int str_id;
  int max_len;
  TimeFormatDataPoint* pDpTimePreference = TimeFormatDataPoint::GetInstance();
  mpc::display::ActualAlarmString alarm_String;

  SmsOut* my_sms = new SmsOut();  //Create a sms object, SmsCtrl will take care of deleting the object

  /* Create the message */
  s1[0]=0;
  if(pAlarmEvent->GetAlarmType() == ALARM_STATE_WARNING)
    strncat(s1, Languages::GetInstance()->GetString( SID_WARNING ),STR_MAX);  //Warning
  else if (pAlarmEvent->GetAlarmType() == ALARM_STATE_ALARM)
    strncat(s1, Languages::GetInstance()->GetString( SID_FS_ALARM ),STR_MAX); //Alarm
  max_len = STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, "\n", max_len);                                          // CR LF
  max_len = STR_MAX - strlen(s1);
  str_id = GetUnitString(pAlarmEvent->GetErroneousUnit(), pAlarmEvent->GetErroneousUnitNumber());//Alarm source
  if( max_len>0 )
    strncat(s1, Languages::GetInstance()->GetString( str_id ), max_len); //Unit type and number
  max_len = STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, "\n", max_len);                                          // CR LF
  max_len = STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, AlarmText::GetInstance()->GetString(pAlarmEvent->GetAlarmId(), pAlarmEvent->GetErroneousUnitNumber()), max_len); //Alarm text
  max_len = STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, "\n", max_len);                                          // CR LF
  max_len = STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, Languages::GetInstance()->GetString( SID_ARRIVAL_TIME ), max_len);  //Occurred at
  max_len = STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, "\n", max_len);                                          // CR LF
  max_len = STR_MAX - strlen(s1);
  mpTimeText->SetTime(*pAlarmEvent->GetArrivalTime());
  if( max_len>0 )
    strncat(s1, mpTimeText->GetText(), max_len);                         //Time
  max_len = STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, "\n", max_len);                                           // CR LF
  max_len = STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, Languages::GetInstance()->GetString( SID_DEPARTURE_TIME ), max_len);//Disappeared at
  strcat(s1, "\n");                                                     // CR LF
  max_len = STR_MAX - strlen(s1);
  mpTimeText->SetTime(*pAlarmEvent->GetDepartureTime());
  if( max_len>0 )
    strncat(s1, mpTimeText->GetText(), max_len);                                    //Time

  /* Copy messge to sms object */
  my_sms->SetSmsMessage( s1 );

  SmsCtrl::GetInstance()->SendWatchAlarmSms(my_sms);      
}

/*****************************************************************************
 * Function - GetUnitString
 * DESCRIPTION: Get string telling the alarm source and number
 *
 ****************************************************************************/
STRING_ID WatchSmsAlarm::GetUnitString(ERRONEOUS_UNIT_TYPE type, int number)
{
  for(int i = 0; i < DISPLAY_UNIT_STRINGS_CNT; ++i)
  {
    if( DISPLAY_UNIT_STRINGS[i].UnitType == type
        && DISPLAY_UNIT_STRINGS[i].UnitNumber == number )
    {
        return (*(DISPLAY_UNIT_STRINGS+i)).StringId;
    }
  }
  return SID_UNIT_UNKNOWN;
}
/*****************************************************************************
 * Function - HandelWatchAlarmSms
 * DESCRIPTION: 
 *
 ****************************************************************************/
void WatchSmsAlarm::HandelWatchAlarmSms()
{
  AlarmEvent* p_alarm_event;
  bool alarm_scada_enabled = false;
  bool watch_scada_enabled = (mpScadaWatchEnabled->GetValue() && mpScadaCallbackEnabled->GetValue());  // Watch Alarm SMS enabled and Master SCADA callback enabled
  
  mpAlarmLog->UseAlarmLog();
  for (int i = 0; i < ALARM_LOG_SIZE; i++)
  {
    /* Get event */
    p_alarm_event = mpAlarmLog->GetAlarmLogElement(i);    

    if (p_alarm_event->GetAlarmId() != ALARM_ID_NO_ALARM)       
    {
      alarm_scada_enabled = ((watch_scada_enabled) && (p_alarm_event->GetErrorSource()->GetAlarmConfig()->GetScadaEnabled()));  // Alarm SCADA callback enabled

      if ((alarm_scada_enabled) && (mpScadaState->GetValue() == SCADA_STATE_NOT_CONNECTED)) // SCADA disconnected and watch is enabled
      {
        SendWatchSms(p_alarm_event, true);
      }
      else if ((!alarm_scada_enabled) || (mpScadaState->GetValue() == SCADA_STATE_CONNECTED)) //Suppress the alarm even watch disabled.
      {
        SendWatchSms(p_alarm_event, false);
      }
      //else do nothing
    } 
  }
  mpAlarmLog->UnuseAlarmLog();   
}
/*****************************************************************************
 * Function - SendWatchSms
 * DESCRIPTION: 
 *
 ****************************************************************************/
void WatchSmsAlarm::SendWatchSms(AlarmEvent* pAlarmEvent, bool sendWatchSms)
{  
  if (pAlarmEvent->GetWatchSmsArrivalSent() == false)        //Check if Sms has been sent for this event
  {
    if (sendWatchSms)
    {
      SendAlarmSms(pAlarmEvent);                             //Create an alarm sms for this alarm
    }
    pAlarmEvent->SetWatchSmsArrivalSent(true);               //Set flag for sms sent
    if (pAlarmEvent->GetDepartureTime()->IsValid())          //Alarm has all ready disappeared
      pAlarmEvent->SetWatchSmsDepartureSent(true);           //Set flag for alarm departured sms sent
  }
  else if (pAlarmEvent->GetDepartureTime()->IsValid())       //Alarm has disappeared
  {
    if (pAlarmEvent->GetWatchSmsDepartureSent() == false)
    {
      if (sendWatchSms)
      {
        SendAlarmSms(pAlarmEvent);          //Create an alarm sms for this alarm
      }
      pAlarmEvent->SetWatchSmsDepartureSent(true);           //Set flag for alarm departured sms sent
    }
  }
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
