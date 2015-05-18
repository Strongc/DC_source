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
/* CLASS NAME       : CurrentSmsNoCtrl                                      */
/*                                                                          */
/* FILE NAME        : CurrentSmsNoCtrl.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 05-12-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/

//TEST JSM
//#define TEST_SCHEDULE

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <TimeDataPoint.h>
#ifdef TEST_SCHEDULE
  #include <stdio.h>
#endif

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <CurrentSmsNoCtrl.h>
#include <EventList.h>
#include <ActTime.h>


/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define ONE_DAY_IN_SEC (24*60*60)

// SW Timers
enum
{
  NEXT_EVENT_TIMER
};

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
CurrentSmsNoCtrl* CurrentSmsNoCtrl::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/


/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CurrentSmsNoCtrl::InitSubTask()
{
  mpTimerObjList[NEXT_EVENT_TIMER] = new SwTimer(5, S, false, false, this);

  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CurrentSmsNoCtrl::RunSubTask()
{
  int current_day_of_week;
  EventList e_list_pri, e_list_sec;
  int time_in_week_act;
  int time_in_week_next;
  int act_value_pri, act_value_sec;
  int sec_to_wait;
  int i,j;

  mRunRequestedFlag = false;

  if( mpCpMon2AllDays.IsUpdated() )
  {
    CpMonToAllDays();
  }

  /* Convert event configuration to a list of events */
  for ( i=0; i<7; i++ )
  {
    for ( j=FIRST_SMS_SCHEDULE_PERIOD; j<=LAST_SMS_SCHEDULE_PERIOD; j++)
    {
      e_list_pri.AddEvent( mDpChangeSmsNoStartimes[j]->GetValue(i)+ONE_DAY_IN_SEC*i, mDpPrimaryNoId[j]->GetValue(i));
      e_list_sec.AddEvent( mDpChangeSmsNoStartimes[j]->GetValue(i)+ONE_DAY_IN_SEC*i, mDpSecondaryNoId[j]->GetValue(i));
    }
  }
  e_list_pri.SortEvents();
  e_list_sec.SortEvents();

  /* Find actual time i week */
  time_in_week_act = ActTime::GetInstance()->GetTime(SECONDS_SINCE_MIDNIGHT);
  current_day_of_week = ActTime::GetInstance()->GetDate(DAY_OF_WEEK);
  if (current_day_of_week==0)// need monday=0 for vector-datapoints, but GetDate(DAY_OF_WEEK) starts with sunday = 0
  {
    current_day_of_week = 6;
  }
  else
  {
    current_day_of_week--;
  }
  time_in_week_act += current_day_of_week*ONE_DAY_IN_SEC ;


  /* Find actual value/category */
  act_value_pri = e_list_pri.GetValue( time_in_week_act );
  if(act_value_pri==-1)       //No events in list < act_time_in_week
  {
    act_value_pri = e_list_pri.GetValue( 7*ONE_DAY_IN_SEC );//Get value for the end of week
  }
  act_value_sec = e_list_sec.GetValue( time_in_week_act );
  if(act_value_sec==-1)       //No events in list < act_time_in_week
  {
    act_value_sec = e_list_sec.GetValue( 7*ONE_DAY_IN_SEC );//Get value for the end of week
  }

  /* Find time for next event, just look at primary list, the two lists have same event times */
  time_in_week_next = e_list_pri.GetTimeForNextEvent(time_in_week_act);
  if(time_in_week_next==-1)
  {
    time_in_week_next = e_list_pri.GetTimeForNextEvent(-1);//GetFirstEvent
  }

  if(time_in_week_next!=-1)
  {
    /* Find time to wait for next event */
    sec_to_wait = time_in_week_next - time_in_week_act;
    if(sec_to_wait<0)
    {
      sec_to_wait+=7*ONE_DAY_IN_SEC;
    }
#ifdef TEST_SCHEDULE
    mpTimerObjList[NEXT_EVENT_TIMER]->SetSwTimerPeriod(sec_to_wait, MS, false);
#else
    mpTimerObjList[NEXT_EVENT_TIMER]->SetSwTimerPeriod(sec_to_wait, S, false);
#endif
    mpTimerObjList[NEXT_EVENT_TIMER]->RetriggerSwTimer();
  }

  /* Handle output */
  mDpCurrentPrimarySmsNo->SetValue( mDpPhoneNo[ act_value_pri]->GetValue() );
  mDpCurrentSecondarySmsNo->SetValue( mDpPhoneNo[ act_value_sec]->GetValue() );

#ifdef TEST_SCHEDULE
  FILE *stream;
  char a[1000];
  int len;
  sprintf(a, "ActTime: %i  NextTime: %i  Pri: %i  Sec:%i\n",time_in_week_act,time_in_week_next, act_value_pri, act_value_sec ),
  len=strlen(a);
  /* Open for write */
  stream = fopen( "PriSec.txt", "a" );
  fwrite(a, sizeof( char ),len, stream );

  /* Close stream */
  fclose( stream );
#endif

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void CurrentSmsNoCtrl::ConnectToSubjects()
{
  mDpChangeSmsNoStartimes[SMS_SCHEDULE_PERIOD_1]->Subscribe(this);
  mDpChangeSmsNoStartimes[SMS_SCHEDULE_PERIOD_2]->Subscribe(this);
  mDpChangeSmsNoStartimes[SMS_SCHEDULE_PERIOD_3]->Subscribe(this);

  mDpPhoneNo[PHONE_NO_1]->Subscribe(this);
  mDpPhoneNo[PHONE_NO_2]->Subscribe(this);
  mDpPhoneNo[PHONE_NO_3]->Subscribe(this);

  mDpPrimaryNoId[SMS_SCHEDULE_PERIOD_1]->Subscribe(this);
  mDpPrimaryNoId[SMS_SCHEDULE_PERIOD_2]->Subscribe(this);
  mDpPrimaryNoId[SMS_SCHEDULE_PERIOD_1]->Subscribe(this);

  mDpSecondaryNoId[SMS_SCHEDULE_PERIOD_1]->Subscribe(this);
  mDpSecondaryNoId[SMS_SCHEDULE_PERIOD_2]->Subscribe(this);
  mDpSecondaryNoId[SMS_SCHEDULE_PERIOD_3]->Subscribe(this);

  mpCpMon2AllDays->Subscribe(this);

  // to update sms numbers when date/time is changed by user
  DateTimeChangeEvent::GetInstance()->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 *****************************************************************************/
void CurrentSmsNoCtrl::Update(Subject* pSubject)
{
  mpCpMon2AllDays.Update( pSubject );

  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void CurrentSmsNoCtrl::SubscribtionCancelled(Subject* pSubject)
{
  //not needed, thus not implemented
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void CurrentSmsNoCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_CSNC_CURRENT_PRIMARY_SMS_NO:
      mDpCurrentPrimarySmsNo.Attach(pSubject);
      break;
    case SP_CSNC_CURRENT_SECONDARY_SMS_NO:
      mDpCurrentSecondarySmsNo.Attach(pSubject);
      break;
    case SP_CSNC_1ST_STARTTIME:
      mDpChangeSmsNoStartimes[SMS_SCHEDULE_PERIOD_1].Attach(pSubject);
      break;
    case SP_CSNC_2ND_STARTTIME:
      mDpChangeSmsNoStartimes[SMS_SCHEDULE_PERIOD_2].Attach(pSubject);
      break;
    case SP_CSNC_3TH_STARTTIME:
      mDpChangeSmsNoStartimes[SMS_SCHEDULE_PERIOD_3].Attach(pSubject);
      break;
    case SP_CSNC_1P_NO:
      mDpPrimaryNoId[SMS_SCHEDULE_PERIOD_1].Attach(pSubject);
      break;
    case SP_CSNC_2P_NO:
      mDpPrimaryNoId[SMS_SCHEDULE_PERIOD_2].Attach(pSubject);
      break;
    case SP_CSNC_3P_NO:
      mDpPrimaryNoId[SMS_SCHEDULE_PERIOD_3].Attach(pSubject);
      break;
    case SP_CSNC_1S_NO:
      mDpSecondaryNoId[SMS_SCHEDULE_PERIOD_1].Attach(pSubject);
      break;
    case SP_CSNC_2S_NO:
      mDpSecondaryNoId[SMS_SCHEDULE_PERIOD_2].Attach(pSubject);
      break;
    case SP_CSNC_3S_NO:
      mDpSecondaryNoId[SMS_SCHEDULE_PERIOD_3].Attach(pSubject);
      break;
    case SP_CSNC_PHONE_NO_1:
      mDpPhoneNo[PHONE_NO_1].Attach(pSubject);
      break;
    case SP_CSNC_PHONE_NO_2:
      mDpPhoneNo[PHONE_NO_2].Attach(pSubject);
      break;
    case SP_CSNC_PHONE_NO_3:
      mDpPhoneNo[PHONE_NO_3].Attach(pSubject);
      break;
    case SP_CSNC_CP_MON_TO_ALL_DAYS_EVENT:
      mpCpMon2AllDays.Attach(pSubject);
      break;
    default:
      break;
  }
}

/*****************************************************************************
 * Function - NumberInPhoneBook
 * DESCRIPTION:
 *
 *****************************************************************************/
bool CurrentSmsNoCtrl::NumberInPhoneBook(const char* nr)
{
  bool ret_val = false;
  int c;

  for(int i=FIRST_PHONE_NO;i<NO_OF_PHONE_NO;i++)
  {
    c = strcmp(mDpPhoneNo[i]->GetValue(), nr);
    if(c==0)
    {
      ret_val = true;
      break;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION: Returns pointer to singleton object of CurrentSmsNoCtrl.
 *
 *****************************************************************************/
CurrentSmsNoCtrl* CurrentSmsNoCtrl::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new CurrentSmsNoCtrl();
  }
  return mInstance;
}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
CurrentSmsNoCtrl::CurrentSmsNoCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
CurrentSmsNoCtrl::~CurrentSmsNoCtrl()
{
}

/*****************************************************************************
 * Function - CpMonToAllDays
 * DESCRIPTION:
 *
 *****************************************************************************/
void CurrentSmsNoCtrl::CpMonToAllDays(void)
{
  int i, j;

  for ( j=FIRST_SMS_SCHEDULE_PERIOD; j<=LAST_SMS_SCHEDULE_PERIOD; j++)
  {
    I32 i32;
    U8 id_pri, id_sec;

    i32 = mDpChangeSmsNoStartimes[j]->GetValue( WEEKDAY_MONDAY );
    id_pri = mDpPrimaryNoId[j]->GetValue( WEEKDAY_MONDAY );
    id_sec = mDpSecondaryNoId[j]->GetValue( WEEKDAY_MONDAY );

    for ( i=WEEKDAY_TUESDAY; i<=WEEKDAY_SUNDAY; i++ )
    {
      mDpChangeSmsNoStartimes[j]->SetValue(i, i32);
      mDpPrimaryNoId[j]->SetValue(i, id_pri);
      mDpSecondaryNoId[j]->SetValue(i, id_sec);
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
