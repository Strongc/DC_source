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
/* CLASS NAME       : CurrentSmsCategoryCtrl                                */
/*                                                                          */
/* FILE NAME        : CurrentSmsCategoryCtrl.cpp                            */
/*                                                                          */
/* CREATED DATE     : 07-12-2007 dd-mm-yyyy                                 */
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
#include <ActTime.h>

#ifdef TEST_SCHEDULE
  #include <stdio.h>
#endif



/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <CurrentSmsCategoryCtrl.h>
#include <EventList.h>

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
CurrentSmsCategoryCtrl::CurrentSmsCategoryCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
CurrentSmsCategoryCtrl::~CurrentSmsCategoryCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CurrentSmsCategoryCtrl::InitSubTask()
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
void CurrentSmsCategoryCtrl::RunSubTask()
{
  int current_day_of_week;
  EventList e_list;
  int time_in_week_act;
  int time_in_week_next;
  int act_value;
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
      if(mDpCategoryEnabled[j]->GetValue(i))
      {
        e_list.AddEvent( mDpCategoryStarttime[j]->GetValue(i) + ONE_DAY_IN_SEC*i,j);
      }
    }
  }
  e_list.SortEvents();

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
  act_value = e_list.GetValue( time_in_week_act );
  if(act_value==-1)       //No events in list < act_time_in_week
  {
    act_value = e_list.GetValue( 7*ONE_DAY_IN_SEC );//Get value for the end of week
  }

  /* Find time for next event */
  time_in_week_next = e_list.GetTimeForNextEvent(time_in_week_act);
  if(time_in_week_next==-1)
  {
    time_in_week_next = e_list.GetTimeForNextEvent(-1);//GetFirstEvent
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
  mDpCurrentCategory->SetValue((SMS_CATEGORY_TYPE) act_value);


  #ifdef TEST_SCHEDULE
  FILE *stream;
  char a[1000];
  int len;
  sprintf(a, "ActTime: %i  NextTime: %i  Cat: %i\n",time_in_week_act,time_in_week_next, act_value  ),
  len=strlen(a);
  /* Open for write */
  stream = fopen( "test.txt", "a" );
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
void CurrentSmsCategoryCtrl::ConnectToSubjects()
{
  mDpCategoryEnabled[SMS_CATEGORY_WORK]->Subscribe(this);
  mDpCategoryEnabled[SMS_CATEGORY_OFF]->Subscribe(this);
  mDpCategoryEnabled[SMS_CATEGORY_SLEEP]->Subscribe(this);
  mDpCategoryStarttime[SMS_CATEGORY_WORK]->Subscribe(this);
  mDpCategoryStarttime[SMS_CATEGORY_OFF]->Subscribe(this);
  mDpCategoryStarttime[SMS_CATEGORY_SLEEP]->Subscribe(this);

  mpCpMon2AllDays->Subscribe(this);

  // to update sms category if time is changed by user
  DateTimeChangeEvent::GetInstance()->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void CurrentSmsCategoryCtrl::Update(Subject* pSubject)
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
void CurrentSmsCategoryCtrl::SubscribtionCancelled(Subject* pSubject)
{
  //not needed, thus not implemented
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void CurrentSmsCategoryCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_CSCC_CURRENT_CATEGORY:
      mDpCurrentCategory.Attach(pSubject);
      break;
    case SP_CSCC_WORK_ENABLED:
      mDpCategoryEnabled[SMS_CATEGORY_WORK].Attach(pSubject);
      break;
    case SP_CSCC_OFF_ENABLED:
      mDpCategoryEnabled[SMS_CATEGORY_OFF].Attach(pSubject);
      break;
    case SP_CSCC_SLEEP_ENABLED:
      mDpCategoryEnabled[SMS_CATEGORY_SLEEP].Attach(pSubject);
      break;
    case SP_CSCC_WORK_STARTTIME:
      mDpCategoryStarttime[SMS_CATEGORY_WORK].Attach(pSubject);
      break;
    case SP_CSCC_OFF_STARTTIME:
      mDpCategoryStarttime[SMS_CATEGORY_OFF].Attach(pSubject);
      break;
    case SP_CSCC_SLEEP_STARTTIME:
      mDpCategoryStarttime[SMS_CATEGORY_SLEEP].Attach(pSubject);
      break;
    case SP_CSCC_CP_MON_TO_ALL_DAYS_EVENT:
      mpCpMon2AllDays.Attach(pSubject);
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
 * Function - CpMonToAllDays
 * DESCRIPTION:
 *
 *****************************************************************************/
void CurrentSmsCategoryCtrl::CpMonToAllDays(void)
{
  int i, j;

  for ( j=FIRST_SMS_SCHEDULE_PERIOD; j<=LAST_SMS_SCHEDULE_PERIOD; j++)
  {
    bool enabled = mDpCategoryEnabled[j]->GetValue( WEEKDAY_MONDAY );
    I32 i32 = mDpCategoryStarttime[j]->GetValue( WEEKDAY_MONDAY );

    for ( i=WEEKDAY_TUESDAY; i<=WEEKDAY_SUNDAY; i++ )
    {
      mDpCategoryEnabled[j]->SetValue(i, enabled);
      mDpCategoryStarttime[j]->SetValue(i, i32);
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
