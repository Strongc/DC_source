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
/* CLASS NAME       : MpcTime                                               */
/*                                                                          */
/* FILE NAME        : MpcTime.cpp                                           */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
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
#include <MpcTime.h>
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

/***************************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 **************************************************************************************/
MpcTime::MpcTime(U32 secondsSince1Jan1970)
{
  mHours    = INVALID_TIME;
  mMinutes  = INVALID_TIME;
  mSecunds  = INVALID_TIME;

  mYear   = INVALID_DATE;
  mMonth  = INVALID_DATE;
  mDay    = INVALID_DATE;
  mDayOfWeek = INVALID_DATE;
  SetSecondsSince1Jan1970(secondsSince1Jan1970);
}

/***************************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 * If we construct a MpcTime obj, and set currentTime == true, the object is created
 * with the time, when created, otherwise Invalid time
 **************************************************************************************/
MpcTime::MpcTime(bool currentTime)
{
  if ( currentTime == true)
  {
    ActTime* pActTime = ActTime::GetInstance();
    mHours = pActTime->GetTime(HOURS);
    mMinutes = pActTime->GetTime(MINUTES);
    mSecunds = pActTime->GetTime(SECONDS);

    mDay = pActTime->GetDate(DAY);
    mDayOfWeek = pActTime->GetDate(DAY_OF_WEEK);
    mMonth = pActTime->GetDate(MONTH);
    mYear = pActTime->GetDate(YEAR);
  }
  else
  {
    mHours    = INVALID_TIME;
    mMinutes  = INVALID_TIME;
    mSecunds  = INVALID_TIME;

    mYear   = INVALID_DATE;
    mMonth  = INVALID_DATE;
    mDay    = INVALID_DATE;
    mDayOfWeek = INVALID_DATE;
  }
  CalcSecondsSince1Jan1970();
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
MpcTime::~MpcTime()
{
}

/*****************************************************************************
 * Function Operator Overloead
 * DESCRIPTION:
 *
 ****************************************************************************/
MpcTime::MpcTime(const MpcTime& src)
{
  *this = src; // call operator=
}

/*****************************************************************************
 * Function - Operator Overloead
 * DESCRIPTION:
 *
 ****************************************************************************/
MpcTime& MpcTime::operator=(const MpcTime& src)
{
  mHours = src.mHours;
  mMinutes = src.mMinutes;
  mSecunds = src.mSecunds;

  mYear = src.mYear;
  mMonth = src.mMonth;
  mDay = src.mDay;
  mDayOfWeek = src.mDayOfWeek;
  CalcSecondsSince1Jan1970();
  return *this;
}
/*****************************************************************************
 * Function - Operator Overloead
 * DESCRIPTION:
 *
 ****************************************************************************/
bool MpcTime::operator==(const MpcTime& src)
{
    return  mSecunds == src.mSecunds &&
            mMinutes == src.mMinutes &&
            mHours == src.mHours &&
            mDay == src.mDay &&
            mMonth == src.mMonth &&
            mYear == src.mYear &&
            mDayOfWeek == src.mDayOfWeek;
}

/*****************************************************************************
 * Function  Operator Overloead
 * DESCRIPTION:
 *
 ****************************************************************************/
bool MpcTime::operator!=(const MpcTime& src)
{
  return !(*this == src);
}

/*****************************************************************************
 * Function - SetTime
 * DESCRIPTION:
 *
 ****************************************************************************/
void MpcTime::SetTime(TIME_TYPE type, int time)
{
  switch (type)
  {
    case SECONDS:
      mSecunds = time;
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
void MpcTime::SetDate(DATE_TYPE type, int value)
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
void MpcTime::Set(MpcTime* tempObj)
{
  mHours = tempObj->mHours;
  mMinutes = tempObj->mMinutes;
  mSecunds = tempObj->mSecunds;

  mDay = tempObj->mDay;
  mMonth = tempObj->mMonth;
  mYear = tempObj->mYear;
  mDayOfWeek = tempObj->mDayOfWeek;
  CalcSecondsSince1Jan1970();
}

/*****************************************************************************
 * Function - GetTime
 * DESCRIPTION:
 *
 ****************************************************************************/
int MpcTime::GetTime(TIME_TYPE type)
{
  int returnValue;
  switch (type)
  {
    case SECONDS:
      returnValue = mSecunds;
      break;
    case MINUTES:
      returnValue = mMinutes;
      break;
    case HOURS:
      returnValue = mHours;
      break;
    case SECONDS_SINCE_MIDNIGHT:
      returnValue = mHours*3600 + mMinutes * 60 + mSecunds;
      break;
  }
  return returnValue;
}

/*****************************************************************************
 * Function - GetDate
 * DESCRIPTION:
 *
 ****************************************************************************/
int MpcTime::GetDate(DATE_TYPE type)
{
  int returnValue;
  switch (type)
  {
    case DAY:
      returnValue = mDay;
      break;
    case MONTH:
      returnValue = mMonth;
      break;
    case YEAR:
      returnValue = mYear;
      break;
    case DAY_OF_WEEK:
      CalcWeekday();
      returnValue = mDayOfWeek;
      break;
    case DAY_OF_YEAR:
      CalcDayOfYear();
      returnValue = mDayOfYear;
      break;
  }
  return returnValue;
}

/*****************************************************************************
 * Function - SetTimeDateSince1Jan1970
 * DESCRIPTION:
 *
 ****************************************************************************/
void MpcTime::SetSecondsSince1Jan1970(U32 secondsSince1Jan1970)
{
  U32 sum = secondsSince1Jan1970;
  bool found;

  if(sum>946684800)// 60*60*24*366*7 + 60*60*24*365*23
  {
    /* Find year */
    sum-=946684800;
    mYear = 2000;
    found = false;
    while(found==false)
    {
      int y_s;
      if(LeapYear(mYear))
        y_s = 31622400;
      else
        y_s = 31536000;

      if(sum>y_s)
      {
        sum-=y_s;
        mYear++;
      }
      else
      {
        found=true;
      }
    }

    /* Find month */
    found = false;
    mMonth = 1;
    while(found==false)
    {
      int m_s;
      m_s = LastDayInMonth(mMonth, mYear)*86400; //60*60*24*DAYS
      if(sum>m_s)
      {
        sum-=m_s;
        mMonth++;
      }
      else
      {
        found=true;
      }
    }
    /* Find day */
    mDay = 1 + (sum / 86400);       //60*60*24  [sec/day]
    sum -= (mDay-1)*86400;

    /* Find hour */
    mHours = (sum / 3600);          //60*60  [sec/hour]
    sum -= (mHours)*3600;

    /* Find minute */
    mMinutes = (sum / 60);          //60  [sec/minute]
    sum -= (mMinutes)*60;

    /* Find second */
    mSecunds = sum;

    CalcWeekday();
  }
}

/*****************************************************************************
 * Function - GetTimeDateSince1Jan1970
 * DESCRIPTION:
 *
 ****************************************************************************/
U32 MpcTime::GetSecondsSince1Jan1970()
{
  return mSecSince1970;
}


/*****************************************************************************
 * Function - CalcTimeDateSince1Jan1970
 * DESCRIPTION:
 *
 ****************************************************************************/
void MpcTime::CalcSecondsSince1Jan1970()
{
  U32 time = 0; //0 is used as illegal time
  int i;

  if( IsValid() )
  {
    /* Handle years */
    time = 946684800;// 60*60*24*366*7 + 60*60*24*365*23
    for( i=2000; i<mYear; i++ )
    {
      if(LeapYear(i))
        time+=31622400; //60*60*24*366
      else
        time+=31536000; //60*60*24*365
    }

    /* Handle months */
    for( i=1; i<mMonth; i++ )
    {
      time+=LastDayInMonth(i, mYear)*86400; //60*60*24*DAYS
    }

    /* Handle Days */
    time+=(mDay-1)*86400; //60*60*24*DAYS

    /* Handle hours */
    time += mHours*3600;          //60*60*HOUR

    /* Handle minutes */
    time += mMinutes*60;          //60*MINUTES

    /* Handle seconds */
    time += mSecunds;             //SECONDS
  }
  mSecSince1970 = time;
}

/*****************************************************************************
 * Function - IsValid
 * DESCRIPTION:
 * Returns true if all of DAY MONTH YEAR HOURS MINUTES and SECUNDS have been
 * set.
 ****************************************************************************/
bool MpcTime::IsValid(void)
{
   return IsDateValid() && IsTimeValid();
}

/*****************************************************************************
 * Function - IsDateValid
 * DESCRIPTION:
 * Returns true if all of DAY MONTH and YEAR have been
 * set.
 ****************************************************************************/
bool MpcTime::IsDateValid(void)
{
  return mYear != INVALID_DATE
    && mMonth != INVALID_DATE
    && mDay != INVALID_DATE;
}

/*****************************************************************************
 * Function - IsTimeValid
 * DESCRIPTION:
 * Returns true if all of HOURS MINUTES and SECUNDS have been
 * set.
 ****************************************************************************/
bool MpcTime::IsTimeValid(void)
{
  return mHours != INVALID_TIME
    && mMinutes != INVALID_TIME
    && mSecunds != INVALID_TIME;
}

/*****************************************************************************
 * Function - SetDateInValid
 * DESCRIPTION:
 * Marks date part as IN-VALID
 ****************************************************************************/
void MpcTime::SetDateInValid(void)
{
  mYear = INVALID_DATE;
  mMonth = INVALID_DATE;
  mDay = INVALID_DATE;
  mSecSince1970=0;
}

/*****************************************************************************
 * Function - SetTimeInValid
 * DESCRIPTION:
 * Marks time part as IN-VALID
 ****************************************************************************/
void MpcTime::SetTimeInValid(void)
{
  mHours = INVALID_TIME;
  mMinutes = INVALID_TIME;
  mSecunds = INVALID_TIME;
  mSecSince1970=0;
}

/*****************************************************************************
 * Function - IsTimeLegal
 * DESCRIPTION:
 *
 ****************************************************************************/
bool MpcTime::IsTimeLegal()
{
  bool ret_val = true;

  if( mSecunds<0 || mSecunds>59 )
    ret_val = false;
  if( mMinutes<0 || mMinutes>59 )
    ret_val = false;
  if( mHours<0 || mHours>23 )
    ret_val = false;
  if( mDay<1 || mDay>LastDayInMonth(mMonth, mYear) )
    ret_val = false;
  if( mMonth<1 || mMonth>12 )
    ret_val = false;
  if( mYear<2000 || mYear>2099 )
    ret_val = false;

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
 * Function - LastDayInMonth
 * DESCRIPTION:
 *
 ****************************************************************************/
int MpcTime::LastDayInMonth(int month, int year)
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
 * Function - CalcWeekday
 * DESCRIPTION:
 *
 ****************************************************************************/
int MpcTime::CalcWeekday()
{
	int r = -1;
	int x = 0, y = 0;
	int ddcentury = -1;
  int ddmonth = DoomsdayMonth(mYear, mMonth);
  int century = mYear - (mYear % 100);
  ddcentury = DoomsdayCentury(century);
  if( ddcentury < 0 ) return -1;
  if( ddmonth < 0 ) return -1;
  if( ddmonth > mDay )
  {
    mDayOfWeek = (7 - ((ddmonth-mDay) % 7 ) + ddmonth);
  }
  else
  {
    mDayOfWeek = mDay;
  }
  x = (mDayOfWeek - ddmonth);
  x %= 7;
  //TETS JSM y = ddcentury + (mYear-century) + (floor((mYear-century)/4));
  y = ddcentury + (mYear-century) + ((mYear-century)/4);
  y %= 7;
  r = (x+y)%7;
  mDayOfWeek = r;
  return 0;
}

/*****************************************************************************
 * Function - CalcDayOfYear
 * DESCRIPTION:
 *
 ****************************************************************************/
void MpcTime::CalcDayOfYear()
{
  int d;
  int i;

  d=0;

  for( i=1; i<mMonth; i++ )
  {
    d+=LastDayInMonth(mMonth, mYear);
  }
  d+=mDay;

  mDayOfYear = d;
}

/*****************************************************************************
 * Function - LeapYear
 * DESCRIPTION:
 *
 ****************************************************************************/
bool MpcTime::LeapYear(int year)
{
	bool r = false;
	int modFour = year % 4;
	int modHundred = year % 100;
  if(((modFour == 0) && (modHundred != 0)) || ((modHundred == 0) && (modHundred % 4 == 0)))
    r = true;
	return r;
}

/*****************************************************************************
 * Function - DoomsdayMonth
 * DESCRIPTION:
 *
 ****************************************************************************/
int MpcTime::DoomsdayMonth(int year, int month)
{
	int r = -1;
	switch(month)
	{
    case 1: //JAN
      if( LeapYear(year) )
        r = 32;
      else
        r = 31;
      break;
    case 2: //FEB
      if( LeapYear(year) )
        r = 29;
      else
        r = 28;
		break;
      case 3: //MAR
      r = 7;
      break;
    case 4: //APR
      r = 4;
      break;
    case 5: //MAY
      r = 9;
      break;
    case 6: //JUN
      r = 6;
      break;
    case 7: //JUL
      r = 11;
      break;
    case 8: //AUG
      r = 8;
      break;
    case 9: //SEP
      r = 5;
      break;
    case 10: //OCT
      r = 10;
      break;
    case 11: //NOV
      r = 7;
      break;
    case 12: //DEC
      r = 12;
      break;
	}
	return r;
}

/*****************************************************************************
 * Function - DoomsdayCentury
 * DESCRIPTION:
 *
 ****************************************************************************/
int MpcTime::DoomsdayCentury(int century)
{
	int r = -1;
  switch( century % 400 )
	{
	case 0:
		r = 2;
		break;
	case 100:
		r = 0;
		break;
	case 200:
		r = 5;
		break;
	case 300:
		r = 3;
		break;
	}
	return r;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
