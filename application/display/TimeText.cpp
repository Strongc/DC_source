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
/* CLASS NAME       : TimeText                                              */
/*                                                                          */
/* FILE NAME        : TimeText.cpp                                          */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a TimeText.                    */
/*                                                                          */
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
#include "TimeText.h"
#include "TimeFormatDataPoint.h"
#include "MPCFonts.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define MAX_FORMAT_SIZE  50
#define MAX_TEXT_SIZE    30
#define TIMEFORMAT_YMD   "%04d-%02d-%02d  %02d%c%02d"
#define TIMEFORMAT_DMY   "%02d-%02d-%04d  %02d%c%02d"
#define TIMEFORMAT_MDY   "%02d/%02d/%04d  %d%c%02d %s"
#define TIMEFORMAT_YMD_S "%04d-%02d-%02d %02d%c%02d%c%02d"
#define TIMEFORMAT_DMY_S "%02d-%02d-%04d %02d%c%02d%c%02d"
#define TIMEFORMAT_MDY_S "%02d/%02d/%04d %d%c%02d%c%02d %s"

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    TimeText::TimeText(Component* pParent) : ObserverText(pParent)
    {
      mColon = ':';
      mFormat = DATETIME_FORMAT_YMD_ISO; //Default to ISO
      mShowSeconds = false;
      mpFont = DEFAULT_FONT_13_LANGUAGE_INDEP;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    TimeText::~TimeText()
    {
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool TimeText::Redraw()
    {
      // Skip redawing if the component isn't visible.
      if(!IsVisible())
      {
        return false;
      }
      
      return ObserverText::Redraw();
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Update is part of the observer pattern
    *****************************************************************************/
    void TimeText::Update(Subject* pSubject)
    {
      if ( GetSubject() == pSubject )
      {
        // new time format.
        TimeFormatDataPoint* p_time_pre = dynamic_cast<TimeFormatDataPoint*>(pSubject);

        if (p_time_pre != NULL)
        {
          DATETIME_FORMAT_TYPE format = DATETIME_FORMAT_TYPE(p_time_pre->GetAsInt());
          if (mFormat != format)
          {
            mFormat = format;
            Invalidate();
          }
        }
        else
        {
          FatalErrorOccured("TimeText should observe a TimeFormatDataPoint!");
        }
      }
      ObserverText::Update(pSubject);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the time member and invalidate the component 
    * if the new value differs from the previous one
    *****************************************************************************/
    void TimeText::SetTime(MpcTime& newTime)
    {
      if (mTime != newTime)
      {
        mTime = newTime;
        Invalidate();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * return textual representation of the time member 
    *****************************************************************************/
    const char* TimeText::GetText()
    {
      ConvertTimeToText();

      return Text::GetText();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * set true to include seconds in time format. Seconds are hidden as default.
    *****************************************************************************/
    void TimeText::ShowSeconds(bool showSeconds)
    {
      mShowSeconds = showSeconds;
    }

    
    /*****************************************************************************
    *
    *
    *              PROTECTED FUNCTIONS
    *
    *
    ****************************************************************************/

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Formats the time as text
    *****************************************************************************/
    void TimeText::ConvertTimeToText()
    {
      char sz_time[MAX_TEXT_SIZE];

      if (mTime.IsValid())
      {
        int y = mTime.GetDate(YEAR);
        int m = mTime.GetDate(MONTH);
        int d = mTime.GetDate(DAY);
        int h = mTime.GetTime(HOURS);
        int min = mTime.GetTime(MINUTES);
        int s = mTime.GetTime(SECONDS);
        bool isAm = false;
        int hours_offset = 0;
        GetAs12HourClock(&isAm, &hours_offset);
        int ampm_hours = h + hours_offset;

        if (mShowSeconds)
        {
          switch (mFormat)
          {
            case DATETIME_FORMAT_YMD_ISO : // yyyy-mm-dd (ISO)
              sprintf(sz_time, TIMEFORMAT_YMD_S, y, m, d, h, mColon, min, mColon, s);
              break;
            case DATETIME_FORMAT_DMY : // dd-mm-yyyy
              sprintf(sz_time, TIMEFORMAT_DMY_S, d, m, y, h, mColon, min, mColon, s);
              break;
            case DATETIME_FORMAT_MDY_AMPM : // mm/dd-yyyy          
              sprintf(sz_time, TIMEFORMAT_MDY_S, m, d, y, ampm_hours, mColon, min, mColon, s, isAm ? "am" : "pm");
              break;
            default:  // YYYY-MM-DD
              sprintf(sz_time, TIMEFORMAT_YMD_S, y, m, d, h, mColon, min, mColon, s);
          }
        }
        else
        {
          switch (mFormat)
          {
            case DATETIME_FORMAT_YMD_ISO : // yyyy-mm-dd (ISO)
              sprintf(sz_time, TIMEFORMAT_YMD, y, m, d, h, mColon, min);
              break;
            case DATETIME_FORMAT_DMY : // dd-mm-yyyy
              sprintf(sz_time, TIMEFORMAT_DMY, d, m, y, h, mColon, min);
              break;
            case DATETIME_FORMAT_MDY_AMPM : // mm/dd-yyyy
              sprintf(sz_time, TIMEFORMAT_MDY, m, d, y, ampm_hours, mColon, min, isAm ? "am" : "pm");
              break;
            default:  // YYYY-MM-DD
              sprintf(sz_time, TIMEFORMAT_YMD, y, m, d, h, mColon, min);
          }
        }      
      }
      else
      {
        sprintf(sz_time, "%10s  %5s", "--", "--");
      }
      SetText(sz_time);
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Get AM/PM state and hour offset
    *****************************************************************************/
    void TimeText::GetAs12HourClock(bool* isAm, int* hoursOffset)
    {
      *isAm = true;
      *hoursOffset = 0;

      if (mTime.IsValid())
      {
        int hours = mTime.GetTime(HOURS);
        
        if (hours == 0)                        // 00:00 -> 00:59
        {
          *hoursOffset = 12;
          *isAm = true;
        }
        else if ((hours > 0) && (hours <= 11)) // 01:00 -> 11:59
        {
          *isAm = true;
        }
        else if (hours == 12)                  // 12:00 -> 12:59
        {
          *isAm = false;
        }
        else if (hours >= 13)                  // 13:00 -> 23:59
        {
          *hoursOffset = -12;
          *isAm = false;
        }
      }
    }


  } // namespace display
} // namespace mpc


