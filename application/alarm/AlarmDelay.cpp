/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MidRange                                      */
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
/* CLASS NAME       : AlarmDelay                                            */
/*                                                                          */
/* FILE NAME        : AlarmDelay.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 06-12-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AlarmDelay.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/


// SW Timers
enum
{
  ALARM_DELAY_FAULT_TIMER,
  ALARM_DELAY_WARNING_TIMER
};


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  LOCAL CONST VARIABLES
 *****************************************************************************/



/*****************************************************************************
  C functions declarations
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION: This is the constructor for the class, to construct
 * an object of the class type
 *****************************************************************************/
AlarmDelay::AlarmDelay(Observer* pCreator)
{
  mpCreator = pCreator;

  // Timers are constucted with a default delaytime as we are not able to retrieve the delay
  // time from the alarm configuration at the time of construction due to relations not being
  // resolved yet.
  mpTimerObjList[ALARM_DELAY_FAULT_TIMER] = new SwTimer(1000, MS, false, false, this);
  mpTimerObjList[ALARM_DELAY_WARNING_TIMER] = new SwTimer(1000, MS, false, false, this);

  Subscribe(pCreator);  // Auto subscribe the alarm delay.
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
AlarmDelay::~AlarmDelay(void)
{
  delete(mpTimerObjList[ALARM_DELAY_FAULT_TIMER]);
  delete(mpTimerObjList[ALARM_DELAY_WARNING_TIMER]);
}


/*****************************************************************************
 * Function - InitAlarmDelay
 * DESCRIPTION: Initialisation of the alarm delay. This function is meant to
 *              be called from the Init of the observer creating the AlarmDelay.
 *              Thus all relations to subjects has been resolved and subject
 *              pointers are valid.
 *
*****************************************************************************/
void AlarmDelay::InitAlarmDelay(void)
{
  mFault = false;
  mWarning = false;
  mFaultTimerFlag = false;
  mWarningTimerFlag = false;
  mCheckAlarmDelayFlag = false;
}


/*****************************************************************************
 * Function - SetFault
 * DESCRIPTION: Sets the mFault true
 *
 *****************************************************************************/
void AlarmDelay::SetFault(void)
{
  mFault = true;
}


/*****************************************************************************
 * Function - SetWarning
 * DESCRIPTION: Sets the mWarning true
 *
 *****************************************************************************/
void AlarmDelay::SetWarning(void)
{
  mWarning = true;
}


/*****************************************************************************
 * Function - ResetFault
 * DESCRIPTION: Sets the mFault false
 *
 *****************************************************************************/
void AlarmDelay::ResetFault(void)
{
  mFault = false;
}


/*****************************************************************************
 * Function - ResetWarning
 * DESCRIPTION: Sets the mWarning false
 *
 *****************************************************************************/
void AlarmDelay::ResetWarning(void)
{
  mWarning = false;
}


/*****************************************************************************
 * Function - GetFault
 * DESCRIPTION: Returns mFault
 *
 *****************************************************************************/
bool AlarmDelay::GetFault(void)
{
  return mFault;
}


/*****************************************************************************
 * Function - GetWarning
 * DESCRIPTION: Returns mWarning
 *
 *****************************************************************************/
bool AlarmDelay::GetWarning(void)
{
  return mWarning;
}



/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pSubject is one of the subjects observed. If it is
 *              then remember the update and notify the observer that created
 *              this instance of AlarmDelay. The observer is then responsible
 *              for calling the appropriate function in AlarmDelay.
 *
 *****************************************************************************/
void AlarmDelay::Update(Subject* pSubject)
{
  mpAlarm.Update(pSubject);

  if (pSubject == mpTimerObjList[ALARM_DELAY_FAULT_TIMER])
  {
    mFaultTimerFlag = true;
  }
  else if (pSubject == mpTimerObjList[ALARM_DELAY_WARNING_TIMER])
  {
    mWarningTimerFlag = true;
  }

  NotifyObservers();
}


/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects
 *
 *****************************************************************************/
void AlarmDelay::ConnectToSubjects(void)
{
  mpAlarm->Subscribe(this);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: AlarmDealy is only supposed to observe an AlarmDataPoint. It
 *              does not know the Id for this datapoint because it is not a
 *              part of the Midrange database. Thus this function must be called
 *              from SetSubjectPointer in the observer that creates this instance
 *              of AlarmDelay. Then it is the observers responsibility to
 *              evaluate the Id of the subject pointer and only redirect the
 *              correct subject pointer to AlarmDelay.
 *
 *****************************************************************************/
void AlarmDelay::SetSubjectPointer(int Id, Subject* pSubject)
{
  mpAlarm.Attach(pSubject);
}


/*****************************************************************************
 * Function - SubscibtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmDelay::SubscribtionCancelled(Subject* pSubject)
{
}


/*****************************************************************************
 * Function - UpdateAlarmDataPoint
 * DESCRIPTION: This function checks the states of mFault and mWarning and
 *              sets the ErrorPresent of the AlarmDataPoint accordingly.
 *              It is called from the observer that instantiated the AlarmDelay
 *
 *              The function consists of two parts.
 *              First the interpretation of the mFault and mWarning is done
 *              depending upon the type of the AlarmDataPoint.
 *              If it is a digital AlarmDataPoint only mFault is used and the
 *              configuration of the AlarmDataPoint decides if we are dealing
 *              with an warning or an alarm.
 *              In all other cases mWarning translates directly into a warning
 *              and mFault into an alarm.
 *
 *              The second part takes care of setting ErrorPresent in the
 *              AlarmDataPoint. In short it sets the ErrorPresent instantly if
 *              the situation improves (an existing alarm changes to a warning
 *              or ready - or an existing warning changes to ready). If the
 *              situation is worse then the proper delaytimer is started (if
 *              it is not running already).
 *
 *****************************************************************************/
void AlarmDelay::UpdateAlarmDataPoint(void)
{
  ALARM_STATE_TYPE temp_error_present;

  temp_error_present = ALARM_STATE_READY;

  // Alarm type must be fetched for every update as some alarms (dry run, high
  // level etc.) can change type during a configuration change.
  mAlarmType = mpAlarm->GetAlarmConfig()->GetAlarmConfigType();
  switch (mAlarmType)
  {
    case AC_DIGITAL:
      if (mFault == true)
      {
        if (mpAlarm->GetAlarmConfig()->GetWarningEnabled() == true)
        {
          temp_error_present = ALARM_STATE_WARNING;
        }
        if (mpAlarm->GetAlarmConfig()->GetAlarmEnabled() == true)
        {
          temp_error_present = ALARM_STATE_ALARM;
        }
      }
      break;
    case AC_INTEGER:
    case AC_FLOAT:
      if (mWarning == true)
      {
        if (mpAlarm->GetAlarmConfig()->GetWarningEnabled() == true)
        {
          temp_error_present = ALARM_STATE_WARNING;
        }
      }
      if (mFault == true)
      {
        if (mpAlarm->GetAlarmConfig()->GetAlarmEnabled() == true)
        {
          temp_error_present = ALARM_STATE_ALARM;
        }
      }
      break;
    default:
      break;
  }

  if (temp_error_present == ALARM_STATE_ALARM)
  {
    if ((mpAlarm->GetErrorPresent() != temp_error_present) &&
        (mpTimerObjList[ALARM_DELAY_FAULT_TIMER]->GetSwTimerStatus() == false))
    {
      mpTimerObjList[ALARM_DELAY_FAULT_TIMER]->SetSwTimerPeriod(mpAlarm->GetAlarmConfig()->GetAlarmDelay()*1000, MS, false);
      mpTimerObjList[ALARM_DELAY_FAULT_TIMER]->RetriggerSwTimer();
    }
  }
  else
  {
    mpTimerObjList[ALARM_DELAY_FAULT_TIMER]->StopSwTimer();
  }

  if (temp_error_present == ALARM_STATE_WARNING)
  {
    if ((mpAlarm->GetErrorPresent() == ALARM_STATE_READY) &&
        (mpTimerObjList[ALARM_DELAY_WARNING_TIMER]->GetSwTimerStatus() == false))
    {
      // Warning has occured after a ready state. It will be reported after the configured delay
      mpTimerObjList[ALARM_DELAY_WARNING_TIMER]->SetSwTimerPeriod(mpAlarm->GetAlarmConfig()->GetAlarmDelay()*1000, MS, false);
      mpTimerObjList[ALARM_DELAY_WARNING_TIMER]->RetriggerSwTimer();
    }
    if (mpAlarm->GetErrorPresent() == ALARM_STATE_ALARM)
    {
      // Warning has occured after an alarm state. It should be reported imideately to remove the alarm.
      mpTimerObjList[ALARM_DELAY_WARNING_TIMER]->SetSwTimerPeriod(0, MS, false);
      mpTimerObjList[ALARM_DELAY_WARNING_TIMER]->RetriggerSwTimer();
    }
  }
  else
  {
    mpTimerObjList[ALARM_DELAY_WARNING_TIMER]->StopSwTimer();
  }

  if (temp_error_present == ALARM_STATE_READY)
  {
    mpAlarm->SetErrorPresent(temp_error_present);
    // The timers for warning and alarm has already been stopped above when checking
    // for warning and alarm state. Thus there is no risk, that any pending alarm or
    // warning will occur when a running timer times out.
  }
}



/*****************************************************************************
 * Function - CheckErrorTimers
 * DESCRIPTION: This function must be called from the observer than instantiated
 *              AlarmDelay whenever a warning- og alarmtimer has timed out.
 *              If warning (or alarm) is enabled at the AlarmDataPoint
 *              ErrorPresent will be set to the appropriate value.
 *
 *****************************************************************************/
void AlarmDelay::CheckErrorTimers(void)
{
  if (mWarningTimerFlag == true)
  {
    mWarningTimerFlag = false;
    if (mpAlarm->GetAlarmConfig()->GetWarningEnabled() == true)
    {
      mpAlarm->SetErrorPresent(ALARM_STATE_WARNING);
    }
    else
    {
      // If warning is not enabled the alarm state should be ready whenever a possible warning is
      // detected. This is to assure, that a situation where an alarm dissapears is resulting in
      // the system going into ready instead of just keeping the alarm on until the warning limit
      // has been reached.
      mpAlarm->SetErrorPresent(ALARM_STATE_READY);
    }
  }
  if (mFaultTimerFlag == true)
  {
    mFaultTimerFlag = false;
    if (mpAlarm->GetAlarmConfig()->GetAlarmEnabled() == true)
    {
        mpAlarm->SetErrorPresent(ALARM_STATE_ALARM);
    }
  }
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/



/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

