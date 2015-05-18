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
/* CLASS NAME       : SmsHeartBeatCtrl                                      */
/*                                                                          */
/* FILE NAME        : SmsHeartBeatCtrl.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 28-11-2007 dd-mm-yyyy                                 */
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
#include <SmsHeartBeatCtrl.h>
#include <Languages.h>
#include <SmsCtrl.h>

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

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
SmsHeartBeatCtrl::SmsHeartBeatCtrl()
{
  mpHeartBeatTimeObj = new MpcTime(true);
  mpHeartBeatTimeCompareObs = new MpcTimeCmpCtrl( mpHeartBeatTimeObj, this, CMP_TIME );

  mTimeForHeartBeat = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
SmsHeartBeatCtrl::~SmsHeartBeatCtrl()
{
  delete mpHeartBeatTimeObj;
  delete mpHeartBeatTimeCompareObs;
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsHeartBeatCtrl::InitSubTask()
{
  mpHeartBeatTimeCfg.SetUpdated();

  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsHeartBeatCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  // Handle change in time for "heart beat sms"
  if (mpHeartBeatTimeCfg.IsUpdated())
  {
    int h, m, t;
    t = mpHeartBeatTimeCfg->GetValue();  // seconds since midnight
    h=t/3600;
    m = (t-h*3600)/60;
    mpHeartBeatTimeObj->SetTime(MINUTES, m);
    mpHeartBeatTimeObj->SetTime(HOURS,   h);
  }

  // Time is up, check if day is enabled and send an sms
  if( mTimeForHeartBeat )
  {
    int day;
    mTimeForHeartBeat = false;
    day = ActTime::GetInstance()->GetDate(DAY_OF_WEEK);
    if( day>=0 && day<7 )
    {
      if( mpDaysED[day]->GetValue() )   // Check if current week day is enabled
        SendHeartBeatSms();
    }
    else
      FatalErrorOccured("HeartBeat, illegal day");
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void SmsHeartBeatCtrl::ConnectToSubjects()
{
  mpHeartBeatTimeCfg->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void SmsHeartBeatCtrl::Update(Subject* pSubject)
{
  mpHeartBeatTimeCfg.Update(pSubject);

  if( pSubject == mpHeartBeatTimeCompareObs )
  {
    mTimeForHeartBeat = true;
  }

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
void SmsHeartBeatCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void SmsHeartBeatCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_SHBC_SUN:
      mpDaysED[0].Attach(pSubject);
      break;
    case SP_SHBC_MON:
      mpDaysED[1].Attach(pSubject);
      break;
    case SP_SHBC_THU:
      mpDaysED[2].Attach(pSubject);
      break;
    case SP_SHBC_WED:
      mpDaysED[3].Attach(pSubject);
      break;
    case SP_SHBC_TUE:
      mpDaysED[4].Attach(pSubject);
      break;
    case SP_SHBC_FRI:
      mpDaysED[5].Attach(pSubject);
      break;
    case SP_SHBC_SAT:
      mpDaysED[6].Attach(pSubject);
      break;
    case SP_SHBC_HEART_BEAT_TIME_CFG:
      mpHeartBeatTimeCfg.Attach(pSubject);
      break;
    case SP_SHBC_NO_OF_PUMPS:
      mpNoOfPumps.Attach(pSubject);
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
void SmsHeartBeatCtrl::SendHeartBeatSms(void)
{
  SmsOut* my_sms = new SmsOut();
  SmsOut* status_sms1;
  SmsOut* status_sms2;
  SmsOut* status_sms3;
  char s[200];

  StringDataPoint* pStr = dynamic_cast<StringDataPoint*>(GetSubject( SUBJECT_ID_INSTALLATION_NAME ));

  s[0]=0;
  if( pStr )
  {
    strncat( s, pStr->GetValue(), 50 );
  }
  strcat(s, "\n" );                                                       // CR LF
  strncat(s, Languages::GetInstance()->GetString( SID_I_AM_ALIVE ), 148); // "I'm alive"
  my_sms->AllowInstallationName(false);
  my_sms->SetSmsMessage( s );
  my_sms->SetSendTo( SMS_RECIPIENT_PRI );
  
  status_sms1 = SmsCtrl::GetInstance()->GetStatusSms(1);
  status_sms2 = SmsCtrl::GetInstance()->GetStatusSms(2);
  status_sms3 = SmsCtrl::GetInstance()->GetStatusSms(3);
  SmsCtrl::GetInstance()->SendSms( my_sms );
  SmsCtrl::GetInstance()->SendSms( status_sms1 );
  if(mpNoOfPumps->GetValue() > 2)
  {
    SmsCtrl::GetInstance()->SendSms( status_sms2 );
  }
  if(mpNoOfPumps->GetValue() > 4)
  {
    SmsCtrl::GetInstance()->SendSms( status_sms3 );
  }
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
