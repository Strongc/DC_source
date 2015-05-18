/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : MinuteDataPoint, HourDataPoint, DayDataPoint,         */
/*                    MonthDataPoint, YearDataPoint, DateTimeChangeEvent    */
/*                                                                          */
/* FILE NAME        : TimeDataPoint.cpp                                     */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <mpctime.h>
#include <acttime.h>
#include <clock.h>
#include <TimeDataPoint.h>
#include <UnitTypes.h>

 /*****************************************************************************
  CREATS AN OBJECT.
 ******************************************************************************/
MinuteDataPoint* MinuteDataPoint::mInstance = 0;
HourDataPoint* HourDataPoint::mInstance = 0;
DayDataPoint* DayDataPoint::mInstance = 0;
MonthDataPoint* MonthDataPoint::mInstance = 0;
YearDataPoint* YearDataPoint::mInstance = 0;
DateTimeChangeEvent* DateTimeChangeEvent::mInstance = 0;

bool LeapYear(int year)
{
	int modFour = year % 4;
	int modHundred = year % 100;

    if(((modFour == 0) && (modHundred != 0)) || ((modHundred == 0) && (modHundred % 4 == 0)))
    {
      return true;
    }
    else
    {
      return false;
    }
}

int LastdayInMonth(int month, int year)
{
  switch ( month )
  {
    case 1   :
    case 3   :
    case 5   :
    case 7   :
    case 8   :
    case 10  :
    case 12  :
               return 31;
    case 4   :
    case 6   :
    case 9   :
    case 11  :
               return 30;
    case  2  :
               if( LeapYear(year) )
               {
                 return 29;
               }
               else
               {
                 return 28;
               }
   default   : return 0;
  }
}

 /*****************************************************************************
  *
  *
  *              PUBLIC FUNCTIONS
  *
  *
  *****************************************************************************/
MinuteDataPoint* MinuteDataPoint::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new MinuteDataPoint();
  }
  return mInstance;
}

HourDataPoint* HourDataPoint::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new HourDataPoint();
  }
  return mInstance;
}

DayDataPoint* DayDataPoint::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new DayDataPoint();
  }
  return mInstance;
}

MonthDataPoint* MonthDataPoint::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new MonthDataPoint();
  }
  return mInstance;
}

YearDataPoint* YearDataPoint::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new YearDataPoint();
  }
  return mInstance;
}

DateTimeChangeEvent* DateTimeChangeEvent::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new DateTimeChangeEvent();
  }
  return mInstance;
}

 /*****************************************************************************
  * Function - Constructor
  * DESCRIPTION:
  *
  *****************************************************************************/
 MinuteDataPoint::MinuteDataPoint()
 {
    mMaxValue = 59;
    mMinValue = 0;
    mValue = 0;
    mQuantity = Q_CLOCK_MINUTE;
    mQuality = DP_AVAILABLE;
    ActTime::GetInstance()->Subscribe(this);
 }

 HourDataPoint::HourDataPoint()
 {
    mMaxValue = 23;
    mMinValue = 0;
    mValue = 0;
    mQuantity = Q_CLOCK_HOUR;
    mQuality = DP_AVAILABLE;
    ActTime::GetInstance()->Subscribe(this);
 }

 DayDataPoint::DayDataPoint()
 {
    mMaxValue = 31;
    mMinValue = 1;
    mValue = 1;
    mQuantity = Q_CLOCK_DAY;
    mQuality = DP_AVAILABLE;
    ActTime::GetInstance()->Subscribe(this);
 }

 MonthDataPoint::MonthDataPoint()
 {
    mMaxValue = 12;
    mMinValue = 1;
    mValue = 1;
    mQuantity = Q_CLOCK_MONTH;
    mQuality = DP_AVAILABLE;
    ActTime::GetInstance()->Subscribe(this);
 }

 YearDataPoint::YearDataPoint()
 {
    mMaxValue = 2099;
    mMinValue = 2000;
    mValue = 2000;
    mQuantity = Q_CLOCK_YEAR;
    mQuality = DP_AVAILABLE;
    ActTime::GetInstance()->Subscribe(this);
 }

 /*****************************************************************************
  * Function - Destructor
  * DESCRIPTION:
  *
  ****************************************************************************/
MinuteDataPoint::~MinuteDataPoint()
{
}

bool MinuteDataPoint::SetValue(I32 min)
{
  MpcTime time_obj;

  mValue = min;
  
  time_obj.SetTime(SECONDS, 0);
  time_obj.SetTime(MINUTES, mValue);
  Clock::GetInstance()->InitClock(&time_obj);
  DateTimeChangeEvent::GetInstance()->SetEvent();
  return true;
}

void MinuteDataPoint::Update(Subject* pSubject)
{
  I32 value = ActTime::GetInstance()->GetTime(MINUTES);
  if (mValue != value)
  {
    mValue = value;
    NotifyObservers();
  }
}


///////////////////////////////////////////////////////////////////////////////

HourDataPoint::~HourDataPoint()
{
}

bool HourDataPoint::SetValue(I32 hour)
{
  MpcTime time_obj;
  
  mValue = hour;

  time_obj.SetTime(HOURS, mValue);
  Clock::GetInstance()->InitClock(&time_obj);
  DateTimeChangeEvent::GetInstance()->SetEvent();
  return true;
}

void HourDataPoint::Update(Subject* pSubject)
{
  I32 value = ActTime::GetInstance()->GetTime(HOURS);
  if (mValue != value)
  {
    mValue = value;
    NotifyObservers();
  }
}

///////////////////////////////////////////////////////////////////////////////


DayDataPoint::~DayDataPoint()
{
}

I32 DayDataPoint::GetValue(void)
{
  I32 current_value = mValue;

  // max value might have changed due to change in month or year
  if( SetMaxValue( LastdayInMonth(MonthDataPoint::GetInstance()->GetAsInt(), YearDataPoint::GetInstance()->GetAsInt()) ) )
  {
    if (current_value != mValue) 
    {
      //be sure to set clock if current value has changed
      SetValue(mValue);
    }
  }
    
  return mValue;
}

bool DayDataPoint::SetValue(I32 day)
{
  MpcTime time_obj;

  mValue = day;
  
  time_obj.SetDate(DAY, mValue);
  Clock::GetInstance()->InitClock(&time_obj);
  DateTimeChangeEvent::GetInstance()->SetEvent();

  return true;
}

void DayDataPoint::Update(Subject* pSubject)
{
  I32 value = ActTime::GetInstance()->GetDate(DAY);
  if (mValue != value)
  {
    mValue = value;
    NotifyObservers();
  }
}


///////////////////////////////////////////////////////////////////////////////

MonthDataPoint::~MonthDataPoint()
{
}

bool MonthDataPoint::SetValue(I32 month)
{
  MpcTime time_obj;

  mValue = month;
  
  DayDataPoint::GetInstance()->SetMaxValue(LastdayInMonth(month,ActTime::GetInstance()->GetDate(YEAR)));

  time_obj.SetDate(MONTH, mValue);
  time_obj.SetDate(DAY,DayDataPoint::GetInstance()->GetValue());
  Clock::GetInstance()->InitClock(&time_obj);
  DateTimeChangeEvent::GetInstance()->SetEvent();

  return true;
}

void MonthDataPoint::Update(Subject* pSubject)
{
  I32 value = ActTime::GetInstance()->GetDate(MONTH);
  if (mValue != value)
  {
    mValue = value;
    NotifyObservers();
  }
}

///////////////////////////////////////////////////////////////////////////////

YearDataPoint::~YearDataPoint()
{
}


bool YearDataPoint::SetValue(I32 year)
{
  MpcTime time_obj;

  mValue = year;
  
  DayDataPoint::GetInstance()->SetMaxValue(LastdayInMonth(ActTime::GetInstance()->GetDate(MONTH), mValue));

  time_obj.SetDate(YEAR, mValue);
  time_obj.SetDate(DAY, DayDataPoint::GetInstance()->GetValue());
  Clock::GetInstance()->InitClock(&time_obj);

  DateTimeChangeEvent::GetInstance()->SetEvent();
  return true;
}

void YearDataPoint::Update(Subject* pSubject)
{
  I32 value = ActTime::GetInstance()->GetDate(YEAR);
  if (mValue != value)
  {
    mValue = value;
    NotifyObservers();
  }
}


///////////////////////////////////////////////////////////////////////////////



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
