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
/* CLASS NAME       : TimeNumber                                            */
/*                                                                          */
/* FILE NAME        : TimeNumber.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a time (hours and minutes)     */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "TimeNumber.h"
#include "DataPoint.h"
#include "Languages.h"
#include <TimeFormatDataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

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
    TimeNumber::TimeNumber(Component* pParent) : ObserverText(pParent)
    {
      mColon = ':';
      strcpy(mStringToFormat,"%02d%c%02d");
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    TimeNumber::~TimeNumber()
    {
			mpTime1.UnsubscribeAndDetach(this);
			mpTime2.UnsubscribeAndDetach(this);
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool TimeNumber::Redraw()
    {
      // Skip redawing if the component isn't visible.
      if(!IsVisible())
        return false;

      char sz_time[30];
      // new time format.
      int time_preferences = mpDpTimePreference->GetAsInt();

      if (mpTime1->GetQuantity() == Q_CLOCK_HOUR)
      {
        if (time_preferences == 2) // US format
        {
            int hours = mpTime1->GetAsInt();
            bool am = true;
            if ( hours == 0 )                         // 00:00 -> 00:59
            {
              hours = 12;
              am = true;
            }
            else if ( (hours > 0) && (hours <= 11) )  // 01:00 -> 11:59
            {
              am = true;
            }
            else if ( hours == 12 )                  // 12:00 -> 12:59
            {
              am = false;
            }
            else if ( hours >= 13 )                  // 13:00 -> 23:59
            {
              hours -= 12;
              am = false;
            }
            sprintf(sz_time,"%d:%02d %s",hours,mpTime2->GetAsInt(),am ? "am" : "pm");
        }
        else
        {
          sprintf(sz_time,"%02d:%02d",mpTime1->GetAsInt(),mpTime2->GetAsInt());
        }
        
      }
      else if (mpTime2->GetQuantity() == Q_CLOCK_HOUR)
      {
        if (time_preferences == 2) // US format
        {
            int hours = mpTime1->GetAsInt();
            bool am = true;
            if ( hours == 0 )                         // 00:00 -> 00:59
            {
              hours = 12;
              am = true;
            }
            else if ( (hours > 0) && (hours <= 11) )  // 01:00 -> 11:59
            {
              am = true;
            }
            else if ( hours == 12 )                  // 12:00 -> 12:59
            {
              am = false;
            }
            else if ( hours >= 13 )                  // 13:00 -> 23:59
            {
              hours -= 12;
              am = false;
            }
            sprintf(sz_time,"%d:%02d %s",hours,mpTime1->GetAsInt(),am ? "am" : "pm");
        }
        else
        {
          sprintf(sz_time,"%02d:%02d",mpTime2->GetAsInt(),mpTime1->GetAsInt());
        }
        
      }



      SetText(sz_time);

      return ObserverText::Redraw();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Update is part of the observer pattern
    *****************************************************************************/
    void TimeNumber::Update(Subject* pSubject)
    {
      Invalidate();
      ObserverText::Update(pSubject);
    }

      /* --------------------------------------------------
      * Called if subscription shall be canceled
      * --------------------------------------------------*/
      void TimeNumber::SubscribtionCancelled(Subject* pSubject)
      {
        mpDpTimePreference = NULL;
        mpTime1.Detach(pSubject);
        mpTime2.Detach(pSubject);
      }
      /* --------------------------------------------------
      * Called to set the subject pointer (used by class
      * factory)
      * --------------------------------------------------*/
      void TimeNumber::SetSubjectPointer(int Id,Subject* pSubject)
      {
        mpDpTimePreference = TimeFormatDataPoint::GetInstance();
        if (!mpTime1.IsValid())
        {
          mpTime1.Attach(pSubject);
        }
        else if (!mpTime2.IsValid())
        {
          mpTime2.Attach(pSubject);
        }
      }
      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      void TimeNumber::ConnectToSubjects(void)
      {
        mpDpTimePreference->Subscribe(this);
        mpTime1->Subscribe(this);
        mpTime2->Subscribe(this);
      }

      bool TimeNumber::IsNeverAvailable()
      {
        if ((mpTime1->GetQuality() == DP_NEVER_AVAILABLE) && (mpTime1->GetQuality() == DP_NEVER_AVAILABLE))
        {
          return true;
        }
        else
        {
          return false;
        }
      }




  } // namespace display
} // namespace mpc


