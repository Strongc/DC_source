/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : InterLockMasterCtrl                                   */
/*                                                                          */
/* FILE NAME        : InterLockMasterCtrl.cpp                               */
/*                                                                          */
/* CREATED DATE     : 22-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
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
#include <InterLockMasterCtrl.h>
#include <SmsCtrl.h>
#include <GeniConvert.h>                // To get access to scaling functions
/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  INTERLOCKED_TIMEOUT_TIMER
  };

#define INTERLOCK_SMS_SEND_EARLY_TIME 300      // An interlock SMS shoul be sent 300s (5 min) before the specified
                                               // timeout to allow for some transmission delay of the SMS
#define INTERLOCK_SMS_MINIMUM_RECALL_TIME 60   // An SMS cannot be allowed to be sent until at least 60s has passed
                                               // the last SMS was sent.


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

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
InterLockMasterCtrl::InterLockMasterCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
InterLockMasterCtrl::~InterLockMasterCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void InterLockMasterCtrl::InitSubTask()
{
  mSettingChangedFlag = false;
  mInterLockTimeoutTimerFlag = false;

  CalcInterLockRecallTime();
  mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER] = new SwTimer(mInterLockRecallTime, S, false, false, this);

  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void InterLockMasterCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if (mpInterLockTimeoutPit.IsUpdated() == true)
  {
    CalcInterLockRecallTime();
    mSettingChangedFlag = true;
  }

  if ((mpInterLockPinEnabledPit.IsUpdated() == true) ||
      (mpInterLockPinPit.IsUpdated() == true) ||
      (mpInterLockPhoneNoPit.IsUpdated() == true) ||
      (mpInterLockNamePit.IsUpdated() == true) )
  {
    mSettingChangedFlag = true;
  }


  switch (mInterLockMasterState)
  {
    case ILMS_UNLOCKED:
      // Hvis:  Level_alarm_flag true OG under_lowest_stop_level false skal vi ikke længere være unlocked
      //        I så fald - hvis:  interlock enabled -> LOCKED og afsend SMS
      //                  ellers:  -> WANT_TO_LOCK
      mSettingChangedFlag = false;        // Ændring i settings har ingen betydning for eventuelle actions her.
      mInterLockTimeoutTimerFlag = false; // Timer runout har ingen betydning for eventuelle actions her.
      if ( (mpLevelAlarmFlag->GetValue() == true) &&
           (mpUnderLowestStopLevel->GetValue() == false) )
      {
        if (mpInterLockEnabledPit->GetValue() == true)
        {
          SendInterLockSMS("INTERLOCK");
          mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->SetSwTimerPeriod(mInterLockRecallTime, S, false);
          mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->RetriggerSwTimer();
          mInterLockMasterState = ILMS_LOCKED;
        }
        else
        {
          mInterLockMasterState = ILMS_WANT_TO_LOCK;
        }
      }
      break;
    case ILMS_WANT_TO_LOCK:
      //   Hvis:  Under_lowest_stop_level true ELLER Level_alarm_flag false -> UNLOCKED
      // Ellers:  Hvis:  interlock enabled -> LOCKED og afsend SMS
      mSettingChangedFlag = false;        // Ændring i settings har ingen betydning for eventuelle actions her.
      mInterLockTimeoutTimerFlag = false; // Timer runout har ingen betydning for eventuelle actions her.
      if ( (mpLevelAlarmFlag->GetValue() == false) ||
           (mpUnderLowestStopLevel->GetValue() == true) )
      {
        mInterLockMasterState = ILMS_UNLOCKED;
      }
      else if (mpInterLockEnabledPit->GetValue() == true)
      {
        SendInterLockSMS("INTERLOCK");
        mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->SetSwTimerPeriod(mInterLockRecallTime, S, false);
        mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->RetriggerSwTimer();
        mInterLockMasterState = ILMS_LOCKED;
      }
      break;
    case ILMS_LOCKED:
      //   Hvis:  Under_lowest_stop_level true ELLER interlock_enabled false -> UNLOCKED og afsend afmelding SMS
      // Ellers:  Hvis:  setting changed ændret så gensend SMS
      //          Hvis:  timer runout så gensend SMS
      if ( (mpInterLockEnabledPit->GetValue() == false) ||
           (mpUnderLowestStopLevel->GetValue() == true) )
      {
        mSettingChangedFlag = false;
        mInterLockTimeoutTimerFlag = false;
        SendInterLockSMS("AUTO");
        mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->StopSwTimer();
        mInterLockMasterState = ILMS_UNLOCKED;
      }
      else if (mSettingChangedFlag == true || mInterLockTimeoutTimerFlag == true)
      {
        mSettingChangedFlag = false;
        mInterLockTimeoutTimerFlag = false;
        SendInterLockSMS("INTERLOCK");
        mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->SetSwTimerPeriod(mInterLockRecallTime, S, false);
        mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER]->RetriggerSwTimer();
      }
      break;
    default:
      break;
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void InterLockMasterCtrl::ConnectToSubjects()
{
  mpLevelAlarmFlag->Subscribe(this);
  mpUnderLowestStopLevel->Subscribe(this);

  mpInterLockEnabledPit->Subscribe(this);
  mpInterLockPinEnabledPit->Subscribe(this);
  mpInterLockPinPit->Subscribe(this);
  mpInterLockTimeoutPit->Subscribe(this);
  mpInterLockPhoneNoPit->Subscribe(this);
  mpInterLockNamePit->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void InterLockMasterCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpTimerObjList[INTERLOCKED_TIMEOUT_TIMER])
  {
    mInterLockTimeoutTimerFlag = true;
  }

  mpInterLockPinEnabledPit.Update(pSubject);
  mpInterLockPinPit.Update(pSubject);
  mpInterLockTimeoutPit.Update(pSubject);
  mpInterLockPhoneNoPit.Update(pSubject);
  mpInterLockNamePit.Update(pSubject);

  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void InterLockMasterCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void InterLockMasterCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_ILMC_LEVEL_ALARM_FLAG:
      mpLevelAlarmFlag.Attach(pSubject);
      break;
    case SP_ILMC_UNDER_LOWEST_STOP_LEVEL:
      mpUnderLowestStopLevel.Attach(pSubject);
      break;

    case SP_ILMC_ENABLED_PIT:
      mpInterLockEnabledPit.Attach(pSubject);
      break;
    case SP_ILMC_PIN_ENABLED_PIT:
      mpInterLockPinEnabledPit.Attach(pSubject);
      break;
    case SP_ILMC_PIN_PIT:
      mpInterLockPinPit.Attach(pSubject);
      break;
    case SP_ILMC_TIMEOUT_PIT:
      mpInterLockTimeoutPit.Attach(pSubject);
      break;
    case SP_ILMC_PHONE_NO_PIT:
      mpInterLockPhoneNoPit.Attach(pSubject);
      break;
    case SP_ILMC_NAME_PIT:
      mpInterLockNamePit.Attach(pSubject);
      break;

    default:
      break;

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
 * Function - SendInterLockSMS
 * DESCRIPTION: Build an interlock SMS and turn it over to the transmit function
 *
 *****************************************************************************/
void InterLockMasterCtrl::SendInterLockSMS(char* command)
{
  SmsOut* my_sms = new SmsOut();
  char s[50];
  char temp_string[30];
  U16 interlock_timeout_in_minutes;

  s[0]=0;
  if (mpInterLockPinEnabledPit->GetValue() == true)
  {
    sprintf(s, "%.4d ", mpInterLockPinPit->GetValue());
  }
  strcat(s, command);
  // Extend the message with the interlock time, if the command is "INTERLOCKED"
  if (strcmp(command, "INTERLOCK") == 0)
  {
    interlock_timeout_in_minutes = ToGeni16bitValue(mpInterLockTimeoutPit.GetSubject(), GENI_CONVERT_ID_TIME_1MIN);
    sprintf(temp_string, " %d ", interlock_timeout_in_minutes);
    strcat(s, temp_string);
  }
  my_sms->AllowInstallationName(false);
  my_sms->SetSmsMessage(s);
  my_sms->SetSendTo(SMS_RECIPIENT_PRI);
  my_sms->SetPrimaryNumber(mpInterLockPhoneNoPit->GetValue());
  SmsCtrl::GetInstance()->SendSms(my_sms);
}


/*****************************************************************************
 * Function - CalcInterLockRecallTime
 * DESCRIPTION: Find the time when to resend an interlock SMS. Make sure, that
 *              the time does not go below 1 minute to avoid continous sending
 *              of SMS's and to avoid a recall time that is 0 or negative.
 *
 *****************************************************************************/
void InterLockMasterCtrl::CalcInterLockRecallTime(void)
{
  U32 timeout;

  timeout = mpInterLockTimeoutPit->GetValue();
  if (timeout < INTERLOCK_SMS_SEND_EARLY_TIME + INTERLOCK_SMS_MINIMUM_RECALL_TIME)
  {
    mInterLockRecallTime = INTERLOCK_SMS_MINIMUM_RECALL_TIME;
  }
  else
  {
    mInterLockRecallTime = timeout - INTERLOCK_SMS_SEND_EARLY_TIME;
  }
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
