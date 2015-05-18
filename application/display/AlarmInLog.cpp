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
/* CLASS NAME       : AlarmInLog                                            */
/*                                                                          */
/* FILE NAME        : AlarmInLog.cpp                                        */
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
#include <AlarmInLog.h>
#include <alarmdef.h>
#include <alarmevent.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

 /*****************************************************************************
 CREATS AN OBJECT.
 ******************************************************************************/

 AlarmInLog* AlarmInLog::mInstance = 0;



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
AlarmInLog::AlarmInLog() : Observer()
{
  mValue = ALARM_STATE_READY;
  mQuality = DP_NEVER_AVAILABLE;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AlarmInLog::~AlarmInLog()
{

}

AlarmInLog* AlarmInLog::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new AlarmInLog();
  }
  return mInstance;
}

void AlarmInLog::SubscribtionCancelled(Subject* pSubject)
{
  mpAlarmLog.Detach(pSubject);
}

void AlarmInLog::Update(Subject* pSubject)
{
  ALARM_STATE_TYPE value = ALARM_STATE_READY;

  if (mpAlarmLog.IsValid())
  {
	  for ( int i = 0; i < ALARM_LOG_SIZE; i++)
	  {
      AlarmEvent* p_alarm_event = mpAlarmLog->GetAlarmLogElement(i);
	    if ((p_alarm_event->GetAlarmId() != ALARM_ID_NO_ALARM) &&
	        (!p_alarm_event->GetAcknowledge()))
	    {
        if (value != ALARM_STATE_ALARM)
        {
          value = p_alarm_event->GetAlarmType();
        }
        else
        {
          break;
        }
	    }
	  }
		
	  mValue = value;

	  mQuality = (mValue != ALARM_STATE_READY ? DP_AVAILABLE : DP_NEVER_AVAILABLE);

	  NotifyObservers();
  }
}

void AlarmInLog::SetSubjectPointer(int Id,Subject* pSubject)
{
  mpAlarmLog.Attach(pSubject); 
}

void AlarmInLog::ConnectToSubjects(void)
{
  if ( mpAlarmLog.IsValid() )
  {
    mpAlarmLog->Subscribe(this);
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
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
