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
/* CLASS NAME       : GeniAppRtcCtrl                                        */
/*                                                                          */
/* FILE NAME        : GeniAppRtcCtrl.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 27-03-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <GeniAppRtcCtrl.h>
#include <ActTime.h>

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
GeniAppRtcCtrl::GeniAppRtcCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
GeniAppRtcCtrl::~GeniAppRtcCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniAppRtcCtrl::InitSubTask()
{
  mRunRequestedFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniAppRtcCtrl::RunSubTask()
{
  MpcTime new_time(true);

  mRunRequestedFlag = false;
  if (mpRtcSecondsSince1970Set.IsUpdated())
  {
    new_time.SetSecondsSince1Jan1970(mpRtcSecondsSince1970Set->GetValue());
    Clock::GetInstance()->InitClock(&new_time);
  }
  else if (mpRtcUseNewTimeEvent.IsUpdated())
  {
    new_time.SetDate(YEAR, 2000+mpRtcYearSet->GetValue());
    new_time.SetDate(MONTH, mpRtcMonthSet->GetValue());
    new_time.SetDate(DAY, mpRtcDaySet->GetValue());
    new_time.SetTime(HOURS, mpRtcHourSet->GetValue());
    new_time.SetTime(MINUTES, mpRtcMinuteSet->GetValue());
    new_time.SetTime(SECONDS, mpRtcSecondSet->GetValue());
    Clock::GetInstance()->InitClock(&new_time);
  }
  mpRtcSecond->SetValue(new_time.GetTime(SECONDS));
  mpRtcMinute->SetValue(new_time.GetTime(MINUTES));
  mpRtcHour->SetValue(new_time.GetTime(HOURS));
  mpRtcDayOfMonth->SetValue(new_time.GetDate(DAY));
  mpRtcMonthOfYear->SetValue(new_time.GetDate(MONTH));
  mpRtcYear->SetValue(new_time.GetDate(YEAR)-2000); // Offset = year 2000
  mpRtcDayOfWeek->SetValue(new_time.GetDate(DAY_OF_WEEK));
  mpRtcDayOfYear->SetValue(new_time.GetDate(DAY_OF_YEAR));
  mpRtcSecondsSince1970Act->SetValue(new_time.GetSecondsSince1Jan1970());
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void GeniAppRtcCtrl::ConnectToSubjects()
{
  // Subscribe to the ActTime object. Should update once per second
  ActTime::GetInstance()->Subscribe(this);

  mpRtcSecondsSince1970Set->Subscribe(this);
  mpRtcUseNewTimeEvent->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void GeniAppRtcCtrl::Update(Subject* pSubject)
{
  if (mpRtcSecondsSince1970Set.Update(pSubject))
  {
    // nop
  }
  else if (mpRtcUseNewTimeEvent.Update(pSubject))
  {
    // nop
  }

  if (mRunRequestedFlag == false)
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
void GeniAppRtcCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void GeniAppRtcCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_RTC_RTC_SECOND:
      mpRtcSecond.Attach(pSubject);
      break;
    case SP_RTC_RTC_MINUTE:
      mpRtcMinute.Attach(pSubject);
      break;
    case SP_RTC_RTC_HOUR:
      mpRtcHour.Attach(pSubject);
      break;
    case SP_RTC_RTC_DAY_OF_MONTH:
      mpRtcDayOfMonth.Attach(pSubject);
      break;
    case SP_RTC_RTC_MONTH:
      mpRtcMonthOfYear.Attach(pSubject);
      break;
    case SP_RTC_RTC_YEAR:
      mpRtcYear.Attach(pSubject);
      break;
    case SP_RTC_RTC_DAY_OF_WEEK:
      mpRtcDayOfWeek.Attach(pSubject);
      break;
    case SP_RTC_RTC_DAY_OF_YEAR:
      mpRtcDayOfYear.Attach(pSubject);
      break;
    case SP_RTC_RTC_SECONDS_SINCE_1970_ACT:
      mpRtcSecondsSince1970Act.Attach(pSubject);
      break;
    case SP_RTC_RTC_SECONDS_SINCE_1970_SET:
      mpRtcSecondsSince1970Set.Attach(pSubject);
      break;
    case SP_RTC_YEAR_SET:
      mpRtcYearSet.Attach(pSubject);
      break;
    case SP_RTC_MONTH_SET:
      mpRtcMonthSet.Attach(pSubject);
      break;
    case SP_RTC_DAY_SET:
      mpRtcDaySet.Attach(pSubject);
      break;
    case SP_RTC_HOUR_SET:
      mpRtcHourSet.Attach(pSubject);
      break;
    case SP_RTC_MINUTE_SET:
      mpRtcMinuteSet.Attach(pSubject);
      break;
    case SP_RTC_SECOND_SET:
      mpRtcSecondSet.Attach(pSubject);
      break;
    case SP_RTC_USE_NEW_TIME_EVENT:
      mpRtcUseNewTimeEvent.Attach(pSubject);
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
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
