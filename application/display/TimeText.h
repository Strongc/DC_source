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
/* FILE NAME        : TimeText.h                                            */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text assigned to a state.    */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayTimeText_h
#define mpc_displayTimeText_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverText.h"
#include <MpcTime.h>
#include "gui_utility/Languages.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
typedef enum
{
  DATETIME_FORMAT_YMD_ISO = 0,
  DATETIME_FORMAT_DMY = 1, 
  DATETIME_FORMAT_MDY_AMPM = 2
} DATETIME_FORMAT_TYPE;
namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a TimeText string given by a DataPoint
    *
    *****************************************************************************/
    class TimeText : public ObserverText
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      TimeText(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~TimeText();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      /* --------------------------------------------------
      * Recieves notifications about time format changes.
      * 
      * --------------------------------------------------*/
      virtual void Update(Subject* pSubject);

      virtual void SetTime(MpcTime& newTime);

      virtual const char* GetText();

      virtual void ShowSeconds(bool showSeconds);

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void ConvertTimeToText();
      virtual void GetAs12HourClock(bool* isAm, int* hoursOffset);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      MpcTime  mTime;

      // Current time format setting
      DATETIME_FORMAT_TYPE mFormat;
      // mColon may be used by child classes to blink the colon
      char mColon;

      bool mShowSeconds;

    };
  } // namespace display
} // namespace mpc

#endif
