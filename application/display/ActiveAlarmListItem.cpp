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
/* CLASS NAME       : ActiveAlarmListItem                                   */
/*                                                                          */
/* FILE NAME        : ActiveAlarmListItem.cpp                               */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a ActiveAlarmListItem.         */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ActiveAlarmListItem.h"
#include "Group.h"
#include "Text.h"
#include "Label.h"
#include "TimeText.h"

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
    ActiveAlarmListItem::ActiveAlarmListItem(Component* pParent) : AlarmListItem(pParent)
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ActiveAlarmListItem::~ActiveAlarmListItem()
    {

    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Make sure you have the lock on the alarmlog before calling this function
    *************************************************************************/
    void ActiveAlarmListItem::UpdateAvailability()
    {
      if (mpAlarmLog.IsValid() && (mAlarmIndex >= 0) && (mAlarmIndex < ALARM_LOG_SIZE))
      {
        AlarmEvent* p_alarm_event = mpAlarmLog->GetAlarmLogElement(mAlarmIndex);
        mLastNeverAvailable = p_alarm_event->GetAlarmId() == ALARM_ID_NO_ALARM || p_alarm_event->GetAcknowledge() == true;
      }
    }
  } // namespace display
} // namespace mpc


