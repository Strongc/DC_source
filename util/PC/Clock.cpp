/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : Clock                                                 */
/*                                                                          */
/* FILE NAME        : Clock.cpp                                             */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

#ifndef __PC__
 #error "This file is only for PC"
#endif

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <time.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <Clock.h>
#include <MpcTime.h>
#include <ActTime.h>
#include <SwTimer.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
enum
    {
    INIT_TIMER_AFTER_INIT,
    ONE_SECONDS_TIMER
    };

/*****************************************************************************
  CREATS AN OBJECT.
 ******************************************************************************/
Clock* Clock::mInstance = 0;
const int century[]={20, 19};

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
Clock* Clock::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new Clock();
  }
  return mInstance;
}
/*****************************************************************************
 * Function GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
void Clock::UpdateTimeObj(MpcTime* pTimeObj)
{
  struct tm *newtime;
  time_t ltime;
  time( &ltime );
  newtime = localtime( &ltime ); /* Convert to local time. */

  pTimeObj->SetDate(YEAR, (newtime->tm_year)+1900);
  pTimeObj->SetDate(MONTH, (newtime->tm_mon)+1);
  pTimeObj->SetDate(DAY, newtime->tm_mday);
  pTimeObj->SetDate(DAY_OF_WEEK, newtime->tm_wday);

  pTimeObj->SetTime(SECONDS, newtime->tm_sec);
  pTimeObj->SetTime(MINUTES, newtime->tm_min);
  pTimeObj->SetTime(HOURS, newtime->tm_hour);
}

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
void Clock::InitClock(MpcTime* pTimeObj)
{
  SYSTEMTIME  st;

  st.wSecond = pTimeObj->GetTime(SECONDS);
  st.wMinute = pTimeObj->GetTime(MINUTES);
  st.wHour = pTimeObj->GetTime(HOURS);
  st.wMilliseconds = 0;

  st.wYear = pTimeObj->GetDate(YEAR);
  st.wMonth = pTimeObj->GetDate(MONTH);
  st.wDay = pTimeObj->GetDate(DAY);
  st.wDayOfWeek = pTimeObj->GetDate(DAY_OF_WEEK);;

  // Be carefull setting the Windows time :D
  // Uhrskov vil ikke ha det, det ødelægger hans VSS database :)
  // int i = SetLocalTime(&st);
}

void Clock::InitSubTask()
{
  UpdateTimeObj(ActTime::GetInstance());
}

/*****************************************************************************
 * Function GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
void Clock::RunSubTask()
{
  UpdateTimeObj(ActTime::GetInstance());
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 *****************************************************************************/
//void Clock::TimeOutFunc(int TimeOutType)
void Clock::Update(Subject* pSubject)
{
  if ( pSubject == mpTimerObjList[ONE_SECONDS_TIMER])
  {
    UpdateTimeObj(ActTime::GetInstance());//pActTimeObj);
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
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
Clock::Clock()
{
  mpTimerObjList[ONE_SECONDS_TIMER] = new SwTimer(1, S, true, true, this);
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
Clock::~Clock()
{
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
