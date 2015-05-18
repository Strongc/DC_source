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
/* CLASS NAME       : DataPointTime                                         */
/*                                                                          */
/* FILE NAME        : DataPointTime.CPP                                     */
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
#include "DataPointTime.h"
#include <ActTime.h>
#include <display_task.h>
#include <MPCFonts.h>
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
    DataPointTime::DataPointTime()
    {
      GUI_SetFont(*DEFAULT_FONT_11_LANGUAGE_INDEP);   // Select Font to use
      SetClientArea(0,0,GUI_GetFontSizeY(),50);
      SetAlign(GUI_TA_LEFT + GUI_TA_BOTTOM);

      mSecCounter = 0;
      mShowColon = false;
      mColon = ' ';
      mLastColonPosition = -1;
     
      ActTime::GetInstance()->Subscribe(this);
      SetTime(*(ActTime::GetInstance()));
    }
    
    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    DataPointTime::~DataPointTime()
    {
      ActTime::GetInstance()->Unsubscribe(this);
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the LEDs which this element and the ones below, wants to be on
    * or off. Normaly this is also the keys to react, but it doesn't have
    * to be this way ...
    *****************************************************************************/
    Leds DataPointTime::GetLedsStatus()
    {
      return NO_LED;
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the keys to which we should react in this element and the
    * elements below. Thereby this is the element to send key messages
    * to for the given keys.
    *****************************************************************************/
    Keys DataPointTime::GetLegalKeys()
    {
      return MPC_NO_KEY;
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Handle this key event. Return true if it is taken care of
    *****************************************************************************/
    bool DataPointTime::HandleKeyEvent(Keys KeyID)
    {
      return false;
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns whether this element is readable/writeable
    *****************************************************************************/
    bool DataPointTime::IsReadOnly()
    {
      return true;
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called by operating system to give time to redraw
    *****************************************************************************/
    void DataPointTime::Run(void)
    {
      mSecCounter += DISPLAY_SAMPLE_TIME;
      if (mSecCounter > (COLON_BLINK_INTERVAL * 2))
      {
        mSecCounter = 0;
      }

      mShowColon = mSecCounter > COLON_BLINK_INTERVAL;
      
      Validate();

      //GetText of TimeText will also set the text and invalidate if changed
      TimeText::GetText();
      
      if (!mValid)
      {
        Redraw();
      }
      else
      {
        RedrawColon();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool DataPointTime::Redraw()
    {
      bool ret = TimeText::Redraw();

      RedrawColon();

      return ret;
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws or clears the colon.
    *****************************************************************************/
    bool DataPointTime::RedrawColon()
    {
      if (mVisible)
      {
        int x = 0;
        int minutes = mTime.GetTime(MINUTES);

        char sz_time[10];
        if (mFormat == DATETIME_FORMAT_MDY_AMPM)// timeformat using am/pm
        {
          bool isAm;
          int hoursOffset;
          GetAs12HourClock(&isAm, &hoursOffset);
          sprintf(sz_time, ":%02d %s", minutes, isAm ? "am" : "pm");
        }
        else
        {
          sprintf(sz_time, ":%02d", minutes);
        }

        SelectWindow();
        GUI_SetColor(mColour);
        GUI_SetBkColor(mBackgroundColour);
        GUI_SetFont(GetFont());

        x = GetWidth() - GUI_GetStringDistX(sz_time);

        if (mShowColon)
        {
          GUI_DispStringAt(":", x, 2);
          mLastColonPosition = x;
        }
        else if (mLastColonPosition == x)
        {
          GUI_ClearRect(x, 0, x + 1, GetHeight() - 1);
        }

        Validate();
      }
      
      return true;
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Update is part of the observer pattern
    *****************************************************************************/
    void DataPointTime::Update(Subject* Object)
    {
      ActTime* pTime = ActTime::GetInstance();

      if (pTime == Object)
      {
        SetTime(*pTime);
      }
      TimeText::Update(Object);
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
  }
}

