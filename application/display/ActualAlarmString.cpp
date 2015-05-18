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
/* CLASS NAME       : ActualAlarmString                                     */
/*                                                                          */
/* FILE NAME        : ActualAlarmString.CPP                                 */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
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
#include <cu351_cpu_types.h>
#include <AlarmText.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "ActualAlarmString.h"
#include <Factory.h>
#include <AlarmLog.h>


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
ActualAlarmString::ActualAlarmString(Component* pParent): ObserverText(pParent)
{
  Languages::GetInstance()->Subscribe(this);
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ActualAlarmString::~ActualAlarmString()
{
  Languages::GetInstance()->Unsubscribe(this);
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - GetUnitString
 * DESCRIPTION:
 *
 ****************************************************************************/
STRING_ID ActualAlarmString::GetUnitString(ERRONEOUS_UNIT_TYPE type, int number)
    {
      for(int i = 0; i < DISPLAY_UNIT_STRINGS_CNT; ++i)
      {
        if( DISPLAY_UNIT_STRINGS[i].UnitType == type
           && DISPLAY_UNIT_STRINGS[i].UnitNumber == number )
        {
            return (*(DISPLAY_UNIT_STRINGS+i)).StringId;
        }
      }
      return SID_UNIT_UNKNOWN;
    }


/*****************************************************************************
 * Function - GetUnitString
 * DESCRIPTION:
 *
 ****************************************************************************/
 void ActualAlarmString::Update(Subject* Object)
 {
   Invalidate();
 }

  bool ActualAlarmString::Redraw()
  {
    if(IsVisible() && mValid == false)
    {
      AlarmLog* alarm_log = dynamic_cast<AlarmLog *>(GetSubject());
      if ( alarm_log == NULL )
      {
        return false;
      }
      AlarmEvent* alarm_event;

      SetText("");
      alarm_log->UseAlarmLog();
      for ( int i = 0; i < ALARM_LOG_SIZE ; i++ )
      {
        alarm_event = alarm_log->GetAlarmLogElement(i);
        if ( !alarm_event->GetAcknowledge() && (alarm_event->GetAlarmId() != ALARM_ID_NO_ALARM) )
        {
          const char* text = AlarmText::GetInstance()->GetString(alarm_event->GetAlarmId(), alarm_event->GetErroneousUnitNumber());
          SetText(text);
          break;
        }
      }
      alarm_log->UnuseAlarmLog();
    }
    return ObserverText::Redraw();
  }

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

  } // namespace display
} // namespace mpc
