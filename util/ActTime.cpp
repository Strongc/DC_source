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
/* CLASS NAME       : ActTime                                               */
/*                                                                          */
/* FILE NAME        : ActTime.cpp                                           */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : This is a signleton that contaions the actual   */
/*                          time. And is read from the RTC 2 times each day */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <ActTime.h>
#include <MPCTime.h>

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
ActTime* ActTime::mpInstance = 0;

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 ****************************************************************************/
ActTime* ActTime::GetInstance()
{
  if (!mpInstance)
  {
    mpInstance = new ActTime(false);
  }
  return mpInstance;
}

/*****************************************************************************
 * Function - SetTime
 * DESCRIPTION:
 *
 ****************************************************************************/
void ActTime::SetTime(TIME_TYPE type, int time)
{
  switch (type)
  {
    case SECONDS:
      mSecunds = time;
      NotifyObservers();
      break;
    case MINUTES:
      mMinutes = time;
      break;
    case HOURS:
      mHours = time;
      break;
    case SECONDS_SINCE_MIDNIGHT:
      mHours = time/3600;
      mMinutes = (time - mHours*3600)/60;
      mSecunds = time - mHours*3600 - mMinutes*60;
      break;
  }
  CalcSecondsSince1Jan1970();
}
/*****************************************************************************
 * Function - SetDate
 * DESCRIPTION:
 *
 ****************************************************************************/
void ActTime::SetDate(DATE_TYPE type, int value)
{
  switch (type)
  {
    case DAY:
      mDay = value;
      break;
    case MONTH:
      mMonth = value;
      break;
    case YEAR:
      mYear = value;
      break;
    case DAY_OF_WEEK:
      mDayOfWeek = value;
      break;
  }
  CalcSecondsSince1Jan1970();
}
/*****************************************************************************
 * Function - Set
 * DESCRIPTION:
 *
 ****************************************************************************/
void ActTime::Set(MpcTime* tempObj)
{
  OS_Use(&mSemaActTime);
  mHours = tempObj->GetTime(HOURS);
  mMinutes = tempObj->GetTime(MINUTES);
  mSecunds = tempObj->GetTime(SECONDS);

  mDay = tempObj->GetDate(DAY);
  mMonth = tempObj->GetDate(MONTH);
  mYear = tempObj->GetDate(YEAR);
  mDayOfWeek = tempObj->GetDate(DAY_OF_WEEK);
  LimitTime();
  OS_Unuse(&mSemaActTime);

  CalcSecondsSince1Jan1970();
  NotifyObservers();
}

/*****************************************************************************
 * Function - UpdateTimeObj
 * DESCRIPTION:
 *
 ****************************************************************************/
void ActTime::UpdateTimeObj(MpcTime* pTimeObj)
{
  OS_Use(&mSemaActTime);
  pTimeObj->SetTime(HOURS, mHours);
  pTimeObj->SetTime(MINUTES, mMinutes);
  pTimeObj->SetTime(SECONDS, mSecunds);

  pTimeObj->SetDate(DAY, mDay);
  pTimeObj->SetDate(MONTH, mMonth);
  pTimeObj->SetDate(YEAR, mYear);
  pTimeObj->SetDate(DAY_OF_WEEK, mDayOfWeek);
  OS_Unuse(&mSemaActTime);
}

/*****************************************************************************
 * Function - IncSec
 * DESCRIPTION:
 * Increment the time with one secund
 ****************************************************************************/
bool ActTime::IncSec()
{
  bool ret_val;

  OS_Use(&mSemaActTime);
  if(IsValid())
  {
    ret_val = true;
    mSecSince1970++;
    if(++mSecunds>59)
    {
      mSecunds=0;
      if(++mMinutes>59)
      {
        mMinutes=0;
        if(++mHours>23)
        {
          mHours=0;
          if(++mDay>LastDayInMonth( mMonth, mYear))
          {
            mDay=1;
            if(++mMonth>12)
            {
              mMonth=1;
              mYear++;
            }
          }
        }
      }
    }
  }
  else
    ret_val = false;

  LimitTime();
  OS_Unuse(&mSemaActTime);
  NotifyObservers();
  return ret_val;
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
ActTime::ActTime(bool currentTime) : MpcTime(currentTime)
{
  OS_CREATERSEMA(&mSemaActTime);
}
/*****************************************************************************
 * Function - ActTime
 * DESCRIPTION:
 *
 ****************************************************************************/
ActTime::~ActTime()
{

}

/*****************************************************************************
 * Function - LimitTime
 * DESCRIPTION:
 *
 *****************************************************************************/
void ActTime::LimitTime(void)
{
  if( mYear>2099 )
  {
    mYear    = 2099;
    mMonth   = 12;
    mDay     = 31;
    mHours   = 23;
    mMinutes = 59;
    mSecunds = 59;
  }
  else if( mYear==1900 )
  {
    mYear    = 2099;
    mMonth   = 12;
    mDay     = 31;
    mHours   = 23;
    mMinutes = 59;
    mSecunds = 59;
  }
  else if( mYear<2000 )
  {
    mYear    = 2000;
    mMonth   = 1;
    mDay     = 1;
    mHours   = 0;
    mMinutes = 0;
    mSecunds = 0;
  }
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
