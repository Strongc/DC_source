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
/* FILE NAME        : SmsAlarm.cpp                                          */
/*                                                                          */
/* CREATED DATE     : 06-12-2007 dd-mm-yyyy                                 */
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
#include <SmsAlarm.h>
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
SmsAlarm::SmsAlarm()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
SmsAlarm::~SmsAlarm()
{
  delete mpTimeText;
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsAlarm::InitSubTask()
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
void SmsAlarm::RunSubTask()
{
  /* Check if an alarm sms has to be sent */
  if (mpAlarmLog.IsUpdated() || mpCurrentSmsCategory.IsUpdated())
  {
    HandelScheduledAlarmSms();
  }

  /* Send an sms alarm for each active alarm */
  if (mpGetAlarmsCmdEvent.IsUpdated())
  {
    HandelCmdEventAlarmSms();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void SmsAlarm::ConnectToSubjects()
{
  mpAlarmLog->Subscribe(this);
  mpGetAlarmsCmdEvent->Subscribe(this);
  mpCurrentSmsCategory->Subscribe(this);   
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void SmsAlarm::Update(Subject* pSubject)
{
  mpAlarmLog.Update(pSubject);
  mpCurrentSmsCategory.Update(pSubject);
  mpGetAlarmsCmdEvent.Update(pSubject); 
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsAlarm::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void SmsAlarm::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_SA_ALARM_LOG:
      mpAlarmLog.Attach(pSubject);
      break;
    case SP_SA_ACTIVE_SMS_CATEGORY:
      mpCurrentSmsCategory.Attach(pSubject);
      break;
    case SP_SA_SMS_RECIPIENT:
      mpSmsRecipient.Attach(pSubject);
      break;
    case SP_SA_GET_ALARMS_CMD:
      mpGetAlarmsCmdEvent.Attach(pSubject);
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
void SmsAlarm::SendAlarmSms(AlarmEvent* pAlarmEvent, SMS_REQUEST_TYPE reqType)
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

  switch (reqType)
  {
  case SMS_PRI_SEC:
    my_sms->SetSendTo(mpSmsRecipient->GetValue());          //Primary, or both primary and secondary number
    SmsCtrl::GetInstance()->SendSms( my_sms );              //Send sms to SmsCtrl
    break;
  case SMS_DIRECT:
    SmsCtrl::GetInstance()->SendDirectAlarmSms(my_sms);
    break;
  }
}

/*****************************************************************************
 * Function - GetUnitString
 * DESCRIPTION: Get string telling the alarm source and number
 *
 ****************************************************************************/
STRING_ID SmsAlarm::GetUnitString(ERRONEOUS_UNIT_TYPE type, int number)
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
 * Function - CheckCurrentSmsCategory
 * DESCRIPTION: Returns true if the current alarm event is enabled for the
 *              actual work/off/sleep
 ****************************************************************************/
bool SmsAlarm::CheckCurrentSmsCategory( AlarmEvent* pAlarmEvent)
{
  bool ret_val = false;
  AlarmDataPoint * pa;

  pa = pAlarmEvent->GetErrorSource();

  if(pa==NULL)
    return ret_val;

  switch (mpCurrentSmsCategory->GetValue())
  {
    case SMS_CATEGORY_WORK :
      if( pa->GetAlarmConfig()->GetSms1Enabled() )
        ret_val = true;
      break;
    case SMS_CATEGORY_OFF :
      if( pa->GetAlarmConfig()->GetSms2Enabled() )
        ret_val = true;
      break;
    case SMS_CATEGORY_SLEEP :
      if( pa->GetAlarmConfig()->GetSms3Enabled() )
        ret_val = true;
      break;
    case SMS_CATEGORY_NONE :
      break;
  }
  return ret_val;
}

/*****************************************************************************
 * Function - HandelScheduledAlarmSms
 * DESCRIPTION: 
 ****************************************************************************/
void SmsAlarm::HandelScheduledAlarmSms()
{
  AlarmEvent* p_alarm_event;
  mpAlarmLog->UseAlarmLog();
  for (int i = 0; i < ALARM_LOG_SIZE; i++)
  {
    /* Get event */
    p_alarm_event = mpAlarmLog->GetAlarmLogElement(i);

    if (p_alarm_event->GetAlarmId() != ALARM_ID_NO_ALARM) //Check if alarme event is active
    {
      //Check if actual alarm is configured to send an sms at this time.
      if( CheckCurrentSmsCategory( p_alarm_event ) )
      {
        if( p_alarm_event->GetSmsArrivalSent() == false )            //Check if Sms has been sent for this event
        {
          SendAlarmSms(p_alarm_event, SMS_PRI_SEC);                  //Create an alarm sms for this alarm
          p_alarm_event->SetSmsArrivalSent(true);                    //Set flag for sms sent
          if(p_alarm_event->GetDepartureTime()->IsValid())           //Alarm has all ready disappeared
            p_alarm_event->SetSmsDepartureSent(true);                //Set flag for alarm departured sms sent
        }
        else if(p_alarm_event->GetDepartureTime()->IsValid())        //Alarm has disappeared
        {
          if( p_alarm_event->GetSmsDepartureSent() == false )
          {
            SendAlarmSms( p_alarm_event, SMS_PRI_SEC);               //Create an alarm sms for this alarm
            p_alarm_event->SetSmsDepartureSent( true );              //Set flag for alarm departured sms sent
          }
        }
      }
      else
      {
        /* If all sms's are disabled, mark alarmeventsms as sent, to ensure that old alrmsms are not sent then sms is enabled */
        AlarmDataPoint * pa;
        pa = p_alarm_event->GetErrorSource();
        if (pa)
        {
          bool enabled = false;
          if ( pa->GetAlarmConfig()->GetSms1Enabled() )
          {
            enabled = true;
          }
          if (pa->GetAlarmConfig()->GetSms2Enabled() )
          {
            enabled = true;
          }
          if (pa->GetAlarmConfig()->GetSms3Enabled() )
          {
            enabled = true;
          }
          if (enabled==false)
          {
            p_alarm_event->SetSmsArrivalSent(true);          //Set flag for sms sent
            p_alarm_event->SetSmsDepartureSent( true );      //Set flag for alarm departured sms sent
          }
        }
      }
    }
  }
  mpAlarmLog->UnuseAlarmLog();    
}
/*****************************************************************************
 * Function - HandelCmdEventAlarmSms
 * DESCRIPTION: 
 ****************************************************************************/
void SmsAlarm::HandelCmdEventAlarmSms()
{
  AlarmEvent* p_alarm_event;
  bool alarm_found = false;
  mpAlarmLog->UseAlarmLog();
  for (int i=0; i<ALARM_LOG_SIZE; i++)
  {
    /* Get event */
    p_alarm_event = mpAlarmLog->GetAlarmLogElement(i);

    if(p_alarm_event->GetAlarmId()!= ALARM_ID_NO_ALARM)
    {
      if(p_alarm_event->GetDepartureTime()->IsValid()==false) //Alarm has all ready disappeared
      {
        alarm_found=true;
        SendAlarmSms(p_alarm_event, SMS_DIRECT);                 //Create an alarm sms for this alarm
      }
    }
  }
  mpAlarmLog->UnuseAlarmLog();
  if(alarm_found==false)
  {
    /* Return a sms */
    SmsOut *my_sms = new SmsOut();
    my_sms->SetSmsMessage( (char*)Languages::GetInstance()->GetString( SID_SMS_NO_ACTIVE_ALARMS ) );  //"No active alarms"
    my_sms->SetSendTo(SMS_RECIPIENT_PRI);
    SmsCtrl::GetInstance()->SendDirectAlarmSms( my_sms );
  }
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
