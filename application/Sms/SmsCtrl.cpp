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
/* CLASS NAME       : SmsCtrl                                               */
/*                                                                          */
/* FILE NAME        : SmsCtrl.cpp                                           */
/*                                                                          */
/* CREATED DATE     : 30-11-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#ifdef __PC__
#include <PcMessageService.h>
#endif //__PC__

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <SmsCtrl.h>
#include <CurrentSmsNoCtrl.h>
#include <NumericalDataPointInterface.h>
#include <MpcUnits.h>
#include "Languages.h"
#include <string.h>
#include <GeniAppIf.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MAX_SMS_IN_ARG_LENGTH 100

//Define filter to limit the number of transmitted smses. One point is one second
#define SMS_POINT 864                        // 15minutes
#define SMS_ACCOUNT_MAX_POINT SMS_POINT*100  // Initialie account to 100sms

#ifdef SMS_STR_MAX
  #error"SmsCtrl"
#else
  #define SMS_STR_MAX 841  //210*4+1
#endif


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
SmsCtrl* SmsCtrl::mInstance = 0;

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
SmsCtrl::SmsCtrl()
{
  /* initialize sms buffer */
  for(int i = 0; i<SMS_BUF_SIZE; i++)
  {
    mSmsBuf[i] = NULL;
  }
  OS_CREATERSEMA(&mSemSmsBuf);
  mSenderState = SMS_SYNCRONIZING;
  mpStatusSms = NULL;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
SmsCtrl::~SmsCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsCtrl::InitSubTask()
{
  mpSmsOutSeqNumber->SetValue(0);  //Initialize geniparameter to 0
  mpSmsSentId->SetQuality(DP_NOT_AVAILABLE);//To ensure that an update is received when geni master set value to 0
  mSmsMaxCntAccount = SMS_ACCOUNT_MAX_POINT;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsCtrl::RunSubTask()
{
  /* Add points to SmsAccount  */
  if( mSmsMaxCntAccount<SMS_ACCOUNT_MAX_POINT )
  {
    mSmsMaxCntAccount++;  //one second is one point
  }

  /* Handle sms sent event */
  if( mpSmsSentId.IsUpdated() &&  mpSmsSentId->GetQuality()==DP_AVAILABLE)
  {
    if( mpSmsOutSeqNumber->GetValue() == 0 )
    {
      if( mpSmsSentId->GetValue()==0 )
      {
        mpSmsOutSeqNumber->SetValue(1);
        mSenderState = SMS_IDLE;
      }
    }
    else if( mpSmsOutSeqNumber->GetValue() == 1 )
    {
      /* Do nothing */
    }
    else if(mpSmsSentId->GetValue() == mpSmsOutSeqNumber->GetValue())
    {
      mpPendingSms->SetValue(false);
      mSenderState = SMS_IDLE;

      OS_Use(&mSemSmsBuf);
      if( mSmsBuf[mActiveSms] != NULL )
      {
        if( mSmsBuf[mActiveSms]->SmsSent() )
        {
          delete mSmsBuf[mActiveSms];
          mSmsBuf[mActiveSms] = NULL;
        }
        else
        {
          mSmsBuf[mActiveSms]->SetRetransCounter(mpRetransTime->GetValue());
        }
      }
      OS_Unuse(&mSemSmsBuf);
    }
  }

  /* Update timeout timers in sms objects */
  OS_Use(&mSemSmsBuf);
  for(int i = 0; i<SMS_BUF_SIZE; i++)
  {
    if(mSmsBuf[i]!=NULL)
    {
      mSmsBuf[i]->UpdateTimeToResend();
    }
  }
  OS_Unuse(&mSemSmsBuf);

  /* Check if a sms is reaedy to be sent */
  if(mSenderState == SMS_IDLE)
  {
    OS_Use(&mSemSmsBuf);
    for(int i = 0; i<SMS_BUF_SIZE; i++)
    {
      if(mSmsBuf[i]!=NULL)
      {
        if( mSmsBuf[i]->GetSmsState() == SMS_QUEUE_PRI || mSmsBuf[i]->GetSmsState() == SMS_QUEUE_SEC)
        {
          if( mSmsMaxCntAccount >= SMS_POINT )
          {
            mSmsMaxCntAccount -= SMS_POINT;
            U32 nr;
            mActiveSms = i;
            nr = mpSmsOutSeqNumber->GetValue();
            if( nr<255 ) //out_sequence_no is 8bit on genibus. 0 and 1 is used for syncronizing
              nr++;
            else
              nr = 2;
            mpSmsOutSeqNumber->SetValue( nr );
            mpPendingSms->SetValue(true);
            mSmsBuf[i]->Transmit();
            mSenderState = SMS_TRANSMITTING;
          }
          else
          {
            delete mSmsBuf[i];// Delete a sms
            mSmsBuf[i]=NULL;
          }
          break;
        }
      }
    }
    OS_Unuse(&mSemSmsBuf);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void SmsCtrl::ConnectToSubjects()
{
  mpSmsSentId->Subscribe(this);
  mpInstallationName->Subscribe(this); 
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void SmsCtrl::Update(Subject* pSubject)
{
  mpSmsSentId.Update(pSubject);  
  mpInstallationName.Update(pSubject);
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void SmsCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_SC_SMS_SENT_ID:
      mpSmsSentId.Attach(pSubject);
      break;
    case SP_SC_SMS_OUT_SEQ_NUMBER:
      mpSmsOutSeqNumber.Attach(pSubject);
      break;
    case SP_SC_SMS_PENDING:
      mpPendingSms.Attach(pSubject);
      break;
    case SP_SC_PRIMARY_NUMBER:
      mpPrimaryNumber.Attach(pSubject);
      break;
    case SP_SC_SECONDARY_NUMBER:
      mpSecondaryNumber.Attach(pSubject);
      break;
    case SP_SC_SCADA_WATCH_PHONE_NUMBER:
      mpWatchPhoneNumber.Attach(pSubject);
      break;
    case SP_SC_ALARM_RESET_EVENT:
      mpAlarmResetEvent.Attach(pSubject);
      break;
    case SP_SC_RETRANS_TIME:
      mpRetransTime.Attach(pSubject);
      break;
    case SP_SC_SMS_AUTHORISE_METHOD:
      mpSmsAuthoriseMethod.Attach(pSubject);
      break;
    case SP_SC_PIN_CODE:
      mpPinCode.Attach(pSubject);
      break;
    case SP_SC_GET_ALARMS_CMD:
      mpGetAlarmsCmdEvent.Attach(pSubject);
      break;
    case SP_SC_CUSTOM_RELAY :
      mpCustomRelay.Attach(pSubject);
    case SP_SC_SIGNAL_LEVEL:
      mpSignalLevel.Attach(pSubject);
      break;
    case SP_SC_PIT_LEVEL_CTRL_TYPE:
      mpPitLevelCtrlType.Attach(pSubject);
      break;
    case SP_SC_AUTO_EVENT:
      mpAutoEvent.Attach(pSubject);
      break;
    case SP_SC_INTERLOCK_EVENT:
      mpInterlockEvent.Attach(pSubject);
      break;
    case SP_SC_INTERLOCK_TIMEOUT:
      mpInterlockTimeout.Attach(pSubject);
      break;
    case SP_SC_NO_OF_PUMPS:
      mpNoOfPumps.Attach(pSubject);
      break;
    case SP_SC_INSTALLATION_NAME:
      mpInstallationName.Attach(pSubject);
      break;
    case SP_SC_SMS_INSTALLATION_NAME:
      mpSmsInstallationName.Attach(pSubject);
      break;
	case SP_SC_BLOCK_REPLY_TEXT_SMS:
	  mpblockreplySms.Attach(pSubject);
	  break;

    /* Measure values */
    case SP_SC_SURFACE_LEVEL:
      mpSurfaceLevel.Attach(pSubject);
      break;
    case SP_SC_PUMP_1_OPERATINIG_TIME:
      mpPumpOperatingTime[PUMP_1].Attach(pSubject);
      break;
    case SP_SC_PUMP_2_OPERATINIG_TIME:
      mpPumpOperatingTime[PUMP_2].Attach(pSubject);
      break;
    case SP_SC_PUMP_3_OPERATINIG_TIME:
      mpPumpOperatingTime[PUMP_3].Attach(pSubject);
      break;
    case SP_SC_PUMP_4_OPERATINIG_TIME:
      mpPumpOperatingTime[PUMP_4].Attach(pSubject);
      break;
    case SP_SC_PUMP_5_OPERATINIG_TIME:
      mpPumpOperatingTime[PUMP_5].Attach(pSubject);
      break;
    case SP_SC_PUMP_6_OPERATINIG_TIME:
      mpPumpOperatingTime[PUMP_6].Attach(pSubject);
      break;
    case SP_SC_PUMP_1_NO_OF_STARTS:
      mpPumpNoOfStarts[PUMP_1].Attach(pSubject);
      break;
    case SP_SC_PUMP_2_NO_OF_STARTS:
      mpPumpNoOfStarts[PUMP_2].Attach(pSubject);
      break;
    case SP_SC_PUMP_3_NO_OF_STARTS:
      mpPumpNoOfStarts[PUMP_3].Attach(pSubject);
      break;
    case SP_SC_PUMP_4_NO_OF_STARTS:
      mpPumpNoOfStarts[PUMP_4].Attach(pSubject);
      break;
    case SP_SC_PUMP_5_NO_OF_STARTS:
      mpPumpNoOfStarts[PUMP_5].Attach(pSubject);
      break;
    case SP_SC_PUMP_6_NO_OF_STARTS:
      mpPumpNoOfStarts[PUMP_6].Attach(pSubject);
      break;
    case SP_SC_PUMP_1_FLOW:
      mpPumpFlow[PUMP_1].Attach(pSubject);
      break;
    case SP_SC_PUMP_2_FLOW:
      mpPumpFlow[PUMP_2].Attach(pSubject);
      break;
    case SP_SC_PUMP_3_FLOW:
      mpPumpFlow[PUMP_3].Attach(pSubject);
      break;
    case SP_SC_PUMP_4_FLOW:
      mpPumpFlow[PUMP_4].Attach(pSubject);
      break;
    case SP_SC_PUMP_5_FLOW:
      mpPumpFlow[PUMP_5].Attach(pSubject);
      break;
    case SP_SC_PUMP_6_FLOW:
      mpPumpFlow[PUMP_6].Attach(pSubject);
      break;
    case SP_SC_SYS_FLOW:
      mpSysFlow.Attach(pSubject);
      break;
    case SP_SC_SYS_VOLUME:
      mpSysVolume.Attach(pSubject);
      break;
    case SP_SC_FLOAT_SWITCH_WATER_LEVEL:
      mpFloatSwitchWaterLevel.Attach(pSubject);
      break;
    case SP_SC_ANA_OUT_USER_1:
      mpAnaOutUser1.Attach(pSubject);
      break;
    case SP_SC_ANA_OUT_USER_2:
      mpAnaOutUser2.Attach(pSubject);
      break;
    case SP_SC_ANA_OUT_USER_3:
      mpAnaOutUser3.Attach(pSubject);
      break;

    default:
      break;

  }
}

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION: Returns pointer to singleton object of SmsSender.
 *
 *****************************************************************************/
SmsCtrl* SmsCtrl::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new SmsCtrl();
  }
  return mInstance;
}

/*****************************************************************************
 * Function - GetActiveSms
 * DESCRIPTION: Returns a pointer to the sms rteady to be sent. If threre are
 *              no sms ready a NULL pointer is returned
 *****************************************************************************/
SmsOut* SmsCtrl::GetActiveSms(void)
{
  return mSmsBuf[mActiveSms];
}

/*****************************************************************************
 * Function - AckSms
 * DESCRIPTION: Send Ack to all active sms
 *
 *****************************************************************************/
void SmsCtrl::AckSms(void)
{
  OS_Use(&mSemSmsBuf);
  for(int i = 0; i<SMS_BUF_SIZE; i++)
  {
    if(mSmsBuf[i]!=NULL)
    {
      if( mSmsBuf[i]->IsWaitingForAck() )
      {
        delete mSmsBuf[i];
        mSmsBuf[i]=NULL;
      }
    }
  }
  OS_Unuse(&mSemSmsBuf);
}

/*****************************************************************************
 * Function - HandleSmsIn
 * DESCRIPTION: Interprete and handle received sms
 *
 *****************************************************************************/
void SmsCtrl::HandleSmsIn(SmsIn* pSms)
{
  char str[3][MAX_SMS_IN_ARG_LENGTH];       //3 strings to contain received parameters
  char pin_code_str[10];
  int c;
  bool sender_accepted = false;
  const char * delimiter_pos;
  int str_index = 0;
  unsigned int length;
  int cmd_index = 0;
  int arg_index = 1;
  bool send_return_sms = true;
  bool wrong_pin_code = false;
  bool wrong_phone_number = false;
  bool validate_phone_number=false;
  bool validate_pin_code=false;

  /* Check for ": " if found, the sms is sent from a CU361 and return sms must not be send */
  if(strstr(pSms->GetSmsMessage(),": "))
  {
    send_return_sms = false;
  }

  //Split sms into arguments/subStrings
  c=' ';
  for(int i=0;i<3;i++)
  {
    str[i][0] = 0; //Initialize to empty string
  }
  for(int i=0;i<3;i++)
  {
    delimiter_pos = strchr(pSms->GetSmsMessage()+str_index, c);
    if( delimiter_pos!=NULL )
    {
      length = delimiter_pos - (pSms->GetSmsMessage()+str_index);
      if( length<MAX_SMS_IN_ARG_LENGTH )
      {
        strncpy( str[i], pSms->GetSmsMessage()+str_index, length );
        str[i][length] = 0;
        str_index += (length+1);
      }
      else
      {
        str[i][0] = 0;  //Too long argument, set to empty string
      }
    }
    else
    {
      strncpy( str[i], pSms->GetSmsMessage()+str_index, MAX_SMS_IN_ARG_LENGTH );
      break;
    }
  }

  /* verify sender */
  sprintf( pin_code_str, "%.4d", mpPinCode->GetValue() );
  switch ( mpSmsAuthoriseMethod->GetValue() )
  {
    case SMS_AUTHORISE_METHOD_NONE :
      cmd_index = 0;
      arg_index = 1;
      sender_accepted = true;
      break;
    case SMS_AUTHORISE_METHOD_PINCODE :
      cmd_index = 1;
      arg_index = 2;
      if( (strncmp(str[0], pin_code_str, 4 ) == 0) && (strlen(str[0]) == 4) )
      {
        sender_accepted = true;
      }
      else
      {
        wrong_pin_code = true;
      }
      break;
    case SMS_AUTHORISE_METHOD_PNONE_NUMBER :
      cmd_index = 0;
      arg_index = 1;
      sender_accepted = CurrentSmsNoCtrl::GetInstance()->NumberInPhoneBook(pSms->GetSmsNumber());
      wrong_phone_number = !sender_accepted;
      break;
    case SMS_AUTHORISE_METHOD_BOTH :
      sender_accepted = CurrentSmsNoCtrl::GetInstance()->NumberInPhoneBook(pSms->GetSmsNumber());
      wrong_phone_number = !sender_accepted;
      if( (strncmp(str[0], pin_code_str, 4 ) == 0) && (strlen(str[0]) == 4) )
      {
		if(!wrong_phone_number)
         sender_accepted = true;

		 cmd_index = 1;
         arg_index = 2;
      }
      else
      {
        cmd_index = 0;
        arg_index = 1;		
		sender_accepted = false;
        wrong_pin_code = true;
      }
      break;
  }

  if(mpblockreplySms->GetValue())
  {
	validate_phone_number = wrong_phone_number;
	validate_pin_code=wrong_pin_code;
  }


  if (sender_accepted==false)
  {
  // if (send_return_sms) 
	 if (send_return_sms && !validate_pin_code && !validate_phone_number)
    {
      /* Sender not acepted, return a sms */
      char s1[SMS_STR_MAX+1]; //The 1 is the 0-terminator
      unsigned int max_len;
      SmsOut *my_sms = new SmsOut();

      s1[0]=0;  //Initialize string to empty string

      if (wrong_pin_code)
      {
        max_len = SMS_STR_MAX - strlen(s1);
        if( max_len>0 ) strncat(s1, Languages::GetInstance()->GetString( SID_USER_ACCESS_DENIED_PIN_CODE_NOT_ACCEPTED ), max_len); //"User access denied. PIN-code not accepted."
        max_len = SMS_STR_MAX - strlen(s1);
        if( max_len>0 ) strncat(s1, ": ", max_len);               // ": "
        max_len = SMS_STR_MAX - strlen(s1);
        if( max_len>0 ) strncat(s1, str[0], max_len);            // "1234"
      }
      if (wrong_phone_number)
      {
        if (wrong_pin_code)
        {
          max_len = SMS_STR_MAX - strlen(s1);
          if( max_len>0 ) strncat(s1, "\n", max_len);            //"CR+LF
        }
        max_len = SMS_STR_MAX - strlen(s1);
        if( max_len>0 ) strncat(s1, Languages::GetInstance()->GetString( SID_USER_ACCESS_DENIED_PHONE_NO_NOT_ACCEPTED ), max_len); //"User access denied. Phone no. not accepted."
        max_len = SMS_STR_MAX - strlen(s1);
        if( max_len>0 ) strncat(s1, ": ", max_len);                  // ": "
        max_len = SMS_STR_MAX - strlen(s1);
        if( max_len>0 ) strncat(s1, pSms->GetSmsNumber(), max_len); // "+4512345678"
      }
      my_sms->SetSmsMessage( s1 );
      my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
      my_sms->SetSendTo(SMS_RECIPIENT_PRI);
      SendSms( my_sms );
    }
  }
  else
  {
    strnupr( str[cmd_index], MAX_SMS_IN_ARG_LENGTH);
    if( strcmp(str[cmd_index], "ACK") == 0)
    {
      AckSms();

      /* Return a sms */
      SmsOut *my_sms = new SmsOut();
      my_sms->SetSmsMessage( (char*)Languages::GetInstance()->GetString( SID_ALARM_SMS_HAS_BEEN_ACKNOWLEDGED ) );  //"Alarm SMS has been acknowledged"
      my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
      my_sms->SetSendTo(SMS_RECIPIENT_PRI);
      SendSms( my_sms );
    }
    else if( strcmp(str[cmd_index], "ALARMRESET") == 0 || strcmp(str[cmd_index], "RESETALARM") == 0)
    {
      mpAlarmResetEvent->SetEvent();

      /* Return a sms */
      if(send_return_sms)
      {
        SmsOut *my_sms = new SmsOut();
        my_sms->SetSmsMessage( (char*)Languages::GetInstance()->GetString( SID_ALARMS_HAS_BEEN_RESET ) );  //"Alarms has been reset"
        my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
        my_sms->SetSendTo(SMS_RECIPIENT_PRI);
        SendSms( my_sms );
      }
    }
    else if( strcmp(str[cmd_index], "GETALARMS") == 0)
    {
      strncpy(mAlarmSmsPhoneNumber, pSms->GetSmsNumber(),17);
      mpGetAlarmsCmdEvent->SetEvent();
    }
    else if( strcmp(str[cmd_index], "STATUS") == 0 || strcmp(str[cmd_index], "STATUS1") == 0)
    {
      SmsOut* status_sms1 = GetStatusSms(1);
      status_sms1->SetPrimaryNumber(pSms->GetSmsNumber());
      SendSms(status_sms1);
	  if(mpNoOfPumps->GetValue()>2)
	  {
		  SmsOut* status_sms2 = GetStatusSms(2);
		  status_sms2->SetPrimaryNumber(pSms->GetSmsNumber());
		  SendSms(status_sms2);
		  if(mpNoOfPumps->GetValue()>4)
		  {
			  SmsOut* status_sms3 = GetStatusSms(3);
			  status_sms3->SetPrimaryNumber(pSms->GetSmsNumber());
			  SendSms(status_sms3);
		  }
	  }
	}
    else if( strcmp(str[cmd_index], "AUTO") == 0)
    {
      mpAutoEvent->SetEvent();

      if(send_return_sms)
      {
        /* Return a sms */
        SmsOut *my_sms = new SmsOut();
        my_sms->SetSmsMessage( (char*)Languages::GetInstance()->GetString( SID_AUTO ) );  //"Auto"
        my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
        my_sms->SetSendTo(SMS_RECIPIENT_PRI);
        SendSms( my_sms );
      }
    }
    else if( strcmp(str[cmd_index], "INTERLOCK") == 0)
    {
      int minutes;
      minutes = atoi(str[arg_index]);
      if(minutes>0)  //0 if atoi converts a non number
      {
        mpInterlockTimeout->SetValue(minutes*60);//Set time in seconds
      }
      mpInterlockEvent->SetEvent();

      if(send_return_sms)
      {
        /* Return a sms */
        SmsOut *my_sms = new SmsOut();
        my_sms->SetSmsMessage( (char*)Languages::GetInstance()->GetString( SID_INTERLOCK_COMMAND_RECEIVED ) );  //"Interlock command received"
        my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
        my_sms->SetSendTo(SMS_RECIPIENT_PRI);
        SendSms( my_sms );
      }
    }
    else if( strcmp(str[cmd_index], "FORCERELAY") == 0)
    {
      mpCustomRelay->SetValue( true );

      if(send_return_sms)
      {
        /* Return a sms */
        SmsOut *my_sms = new SmsOut();
        my_sms->SetSmsMessage( (char*)Languages::GetInstance()->GetString( SID_CUSTOM_RELAY_HAS_BEEN_FORCED ) );  //"Custom relay has been forced"
        my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
        my_sms->SetSendTo(SMS_RECIPIENT_PRI);
        SendSms( my_sms );
      }
    }
    else if( strcmp(str[cmd_index], "RELEASERELAY") == 0)
    {
      mpCustomRelay->SetValue( false );

      if(send_return_sms)
      {
        /* Return a sms */
        SmsOut *my_sms = new SmsOut();
        my_sms->SetSmsMessage( (char*)Languages::GetInstance()->GetString( SID_CUSTOM_RELAY_HAS_BEEN_RELEASED ) );  //"Custom relay has been released"
        my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
        my_sms->SetSendTo(SMS_RECIPIENT_PRI);
        SendSms( my_sms );
      }
    }
    else if( strcmp(str[cmd_index], "HELP") == 0 || strcmp(str[cmd_index], "?") == 0)
    {
      SmsOut *my_sms = new SmsOut();
      my_sms->SetSmsMessage( (char*)Languages::GetInstance()->GetString( SID_SMS_HELP_MSG ) );  // "GETALARMS..."
      my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
      my_sms->SetSendTo(SMS_RECIPIENT_PRI);
      SendSms( my_sms );
    }
    else if( strcmp(str[cmd_index], "SIGNALLEVEL") == 0)
    {
      char s1[200];
      int s1_left = 200;
      s1[0]= 0 ;
      strncat(s1, Languages::GetInstance()->GetString( SID_SIGNAL_LEVEL ), 200);    // "Signal level"
      s1_left = 200 - strlen(s1);
      strncat(s1, ": ", s1_left);                                                   // ": "
      s1_left = 200 - strlen(s1);
      switch ( mpSignalLevel->GetValue() )
      {
        case GSM_SIGNAL_LEVEL_UNKNOWN :
          strncat(s1, Languages::GetInstance()->GetString( SID_DASH ), s1_left);    // "--"
          break;
        case GSM_SIGNAL_LEVEL_NO_SIGNAL :
          strncat(s1, Languages::GetInstance()->GetString( SID_NO_SIGNAL ), s1_left); // "No signal"
          break;
        case GSM_SIGNAL_LEVEL_1 :
          strncat(s1, "0%" , s1_left);                                              // "0%"
          break;
        case GSM_SIGNAL_LEVEL_2 :
          strncat(s1, "25%", s1_left);                                              // "25%"
          break;
        case GSM_SIGNAL_LEVEL_3 :
          strncat(s1, "50%", s1_left);                                              // "50%"
          break;
        case GSM_SIGNAL_LEVEL_4 :
          strncat(s1, "75%", s1_left);                                              // "75%"
          break;
        case GSM_SIGNAL_LEVEL_5 :
          strncat(s1, "100%", s1_left);                                             // "100%"
          break;
        default:
          strncat(s1, Languages::GetInstance()->GetString( SID_DASH ), s1_left);    // "--"
          break;
      }
      SmsOut *my_sms = new SmsOut();
      my_sms->SetSmsMessage( s1 );
      my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
      my_sms->SetSendTo(SMS_RECIPIENT_PRI);
      SendSms( my_sms );
    }
    else if (strcmp(str[cmd_index], "AO1") == 0 || strcmp(str[cmd_index], "AO2") == 0 || strcmp(str[cmd_index], "AO3") == 0 || strcmp(str[cmd_index], "A01") == 0 || strcmp(str[cmd_index], "A02") == 0 || strcmp(str[cmd_index], "A03") == 0) //Check for both 0(zero) and O(uppercase o)
    {
      bool percent_missing = false;
      bool percent_not_number = false;
      bool percent_out_of_bounds = false;
      int percent = 0;

      if (strlen(str[arg_index]) == 0)
      {
        percent_missing = true;
      }
      else
      {
        size_t index = 0;
        while (str[arg_index][index])
        {
          if (str[arg_index][index] < '0' || str[arg_index][index] > '9')
          {
            percent_not_number = true;
          }
          index++;
        }

        if (!percent_not_number)
        {
          percent = atoi(str[arg_index]);
        }

        if (percent < 0 || percent > 100)
        {
          percent_out_of_bounds = true;
        }
      }

      if (!percent_missing && !percent_not_number && !percent_out_of_bounds)
      {
        switch (str[cmd_index][2])
        {
          case '1': mpAnaOutUser1->SetValue(percent); break;
          case '2': mpAnaOutUser2->SetValue(percent); break;
          case '3': mpAnaOutUser3->SetValue(percent); break;
        }
      }

      /* Return a sms */
      if(send_return_sms)
      {
        SmsOut *my_sms = new SmsOut();

        if (percent_missing)
        {
          my_sms->SetSmsMessage((char*)Languages::GetInstance()->GetString(SID_ARGUMENT_MISSING));  //"Missing argument"
        }
        else if (percent_not_number)
        {
          my_sms->SetSmsMessage((char*)Languages::GetInstance()->GetString(SID_ARGUMENT_NOT_A_NUMBER));  //"Argument not a number"
        }
        else if (percent_out_of_bounds)
        {
          my_sms->SetSmsMessage((char*)Languages::GetInstance()->GetString(SID_ARGUMENT_OUT_OF_BOUNDS));  //"Argument out of bounds"
        }
        else
        {
          my_sms->SetSmsMessage((char*)Languages::GetInstance()->GetString(SID_ANA_OUT_USER_SET));  //"Analogue output set"
        }
	//	if(!percent_missing && !percent_not_number && !percent_out_of_bounds)
		{
			my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
			my_sms->SetSendTo(SMS_RECIPIENT_PRI);
			SendSms( my_sms );
		}
      }
    }
    else if (strcmp(str[cmd_index], "GENICMD") == 0)
    {
      bool command_missing = false;
      bool command_not_number = false;
      bool command_out_of_bounds = false;
      int command = 0;

      if (strlen(str[arg_index]) == 0)
      {
        command_missing = true;
      }
      else
      {
        size_t index = 0;
        while (str[arg_index][index])
        {
          if (str[arg_index][index] < '0' || str[arg_index][index] > '9')
          {
            command_not_number = true;
          }
          index++;
        }

        if (!command_not_number)
        {
          command = atoi(str[arg_index]);
        }

        if (command < 0 || command > 255)
        {
          command_out_of_bounds = true;
        }
      }

      bool command_busy = false;
      if (!command_missing && !command_not_number && !command_out_of_bounds)
      {
        command_busy = !GeniAppIf::GetInstance()->RunCommand((U8)command);
      }

      /* Return a sms */
      if(send_return_sms)
      {
        SmsOut *my_sms = new SmsOut();

        if (command_missing)
        {
          my_sms->SetSmsMessage((char*)Languages::GetInstance()->GetString(SID_ARGUMENT_MISSING));  //"Missing argument"
        }
        else if (command_not_number)
        {
          my_sms->SetSmsMessage((char*)Languages::GetInstance()->GetString(SID_ARGUMENT_NOT_A_NUMBER));  //"Argument not a number"
        }
        else if (command_out_of_bounds)
        {
          my_sms->SetSmsMessage((char*)Languages::GetInstance()->GetString(SID_ARGUMENT_OUT_OF_BOUNDS_0_255));  //"Argument out of bounds [0-255]"
        }
        else if (command_busy)
        {
          my_sms->SetSmsMessage((char*)Languages::GetInstance()->GetString(SID_COMMAND_BUSY));  //"Command interpreter busy"
        }
        else
        {
          my_sms->SetSmsMessage((char*)Languages::GetInstance()->GetString(SID_COMMAND_ACCEPTED));  //"Command accepted"
        }
	//	if(!command_missing && !command_not_number && !command_out_of_bounds && !command_busy)
		{
			my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
			my_sms->SetSendTo(SMS_RECIPIENT_PRI);
			SendSms( my_sms );
		}
      }
    }
    else
    {
    if(!mpblockreplySms->GetValue())
	{
      /* Received command not accepted, return a sms */
      char s1[400];
      int s1_left = 400;
      s1[0]= 0 ;

      strncat(s1, Languages::GetInstance()->GetString( SID_UNKNOWN_COMMAND_RECEIVED ), 400);    // "Unknown SMS command received"
      s1_left = 400 - strlen(s1);
      strncat(s1, ": ", s1_left);                                                   // ": "
      s1_left = 400 - strlen(s1);
      strncat(s1, str[cmd_index], s1_left);    // "Command"

      if(send_return_sms)
      {
        SmsOut *my_sms = new SmsOut();
        my_sms->SetSmsMessage(s1);
        my_sms->SetPrimaryNumber(pSms->GetSmsNumber());
        my_sms->SetSendTo(SMS_RECIPIENT_PRI);
        SendSms( my_sms );
      }
	}
    }
  }
}

/*****************************************************************************
 * Function - SendDirectAlarmSms
 * DESCRIPTION: Insert phonenumber in received sms and send sms
 *
 *****************************************************************************/
void SmsCtrl::SendDirectAlarmSms(SmsOut* aSms)
{
  aSms->SetPrimaryNumber( mAlarmSmsPhoneNumber );
  SendSms(aSms);
}

/*****************************************************************************
 * Function - SendWatchAlarmSms
 * DESCRIPTION: Insert atch phonenumber and send sms
 *
 *****************************************************************************/
void SmsCtrl::SendWatchAlarmSms(SmsOut* aSms)
{
  aSms->SetPrimaryNumber( mpWatchPhoneNumber->GetValue() );
  SendSms(aSms);
}

/*****************************************************************************
 * Function - SendSms
 * DESCRIPTION: Insert phone numbers if empty, and send sms
 *
 *****************************************************************************/
void SmsCtrl::SendSms(SmsOut* aSms)
{
  bool sms_inserted = false;
  
  if( *aSms->GetPrimaryNumber()==0 )
  {
    aSms->SetPrimaryNumber( mpPrimaryNumber->GetValue() );
  }
  if( *aSms->GetSecondaryNumber()==0 )
  {
    aSms->SetSecondaryNumber( mpSecondaryNumber->GetValue() );
  }

  aSms->InsertInstallationName(GetInstallationName());  

  OS_Use(&mSemSmsBuf);
  for( int i = 0; i<SMS_BUF_SIZE; i++ )
  {
    if( mSmsBuf[i]==NULL )
    {
      mSmsBuf[i] = aSms;
      sms_inserted = true;
      break;
    }
  }
  OS_Unuse(&mSemSmsBuf);

#ifdef __PC__
  PcMessageService::GetInstance()->SendSms(aSms);
#endif //__PC__

  /* Sms buffer is full, delete the sms */
  if( sms_inserted==false )
  {
    delete aSms;
  }
}

/*****************************************************************************
 * Function - GetStatusSms
 * DESCRIPTION: Built a status sms and return a pointer to the sms
 *
 *****************************************************************************/
SmsOut* SmsCtrl::GetStatusSms(int sequence)
{
  const int str_id_pump[MAX_NO_OF_PUMPS] = { SID_SMS_PUMP_1_, SID_SMS_PUMP_2_, SID_SMS_PUMP_3, SID_SMS_PUMP_4, SID_SMS_PUMP_5, SID_SMS_PUMP_6 };      //Pump X
  char s1[SMS_STR_MAX]; //En sms må ikke være mere end 210karakterer, udfør evt tjek i nedenstående der sikrer mod overindeksering
  char s2[25];
  unsigned int max_len;
  SmsOut *my_sms = new SmsOut();

  mpc::display::NumericalDataPointInterface ndi_surface_level(mpSurfaceLevel.GetSubject());
  mpc::display::NumericalDataPointInterface ndi_sys_flow(mpSysFlow.GetSubject());
  mpc::display::NumericalDataPointInterface ndi_sys_volume(mpSysVolume.GetSubject());

  mpc::display::NumericalDataPointInterface* ndi_operating_hour[MAX_NO_OF_PUMPS];
  mpc::display::NumericalDataPointInterface* ndi_starts[MAX_NO_OF_PUMPS];
  mpc::display::NumericalDataPointInterface* ndi_flow[MAX_NO_OF_PUMPS];

  ndi_operating_hour[PUMP_1] = new mpc::display::NumericalDataPointInterface(mpPumpOperatingTime[PUMP_1].GetSubject());
  ndi_operating_hour[PUMP_2] = new mpc::display::NumericalDataPointInterface(mpPumpOperatingTime[PUMP_2].GetSubject());
  ndi_operating_hour[PUMP_3] = new mpc::display::NumericalDataPointInterface(mpPumpOperatingTime[PUMP_3].GetSubject());
  ndi_operating_hour[PUMP_4] = new mpc::display::NumericalDataPointInterface(mpPumpOperatingTime[PUMP_4].GetSubject());
  ndi_operating_hour[PUMP_5] = new mpc::display::NumericalDataPointInterface(mpPumpOperatingTime[PUMP_5].GetSubject());
  ndi_operating_hour[PUMP_6] = new mpc::display::NumericalDataPointInterface(mpPumpOperatingTime[PUMP_6].GetSubject());

  ndi_starts[PUMP_1] = new mpc::display::NumericalDataPointInterface(mpPumpNoOfStarts[PUMP_1].GetSubject());
  ndi_starts[PUMP_2] = new mpc::display::NumericalDataPointInterface(mpPumpNoOfStarts[PUMP_2].GetSubject());
  ndi_starts[PUMP_3] = new mpc::display::NumericalDataPointInterface(mpPumpNoOfStarts[PUMP_3].GetSubject());
  ndi_starts[PUMP_4] = new mpc::display::NumericalDataPointInterface(mpPumpNoOfStarts[PUMP_4].GetSubject());
  ndi_starts[PUMP_5] = new mpc::display::NumericalDataPointInterface(mpPumpNoOfStarts[PUMP_5].GetSubject());
  ndi_starts[PUMP_6] = new mpc::display::NumericalDataPointInterface(mpPumpNoOfStarts[PUMP_6].GetSubject());

  ndi_flow[PUMP_1] = new mpc::display::NumericalDataPointInterface(mpPumpFlow[PUMP_1].GetSubject());
  ndi_flow[PUMP_2] = new mpc::display::NumericalDataPointInterface(mpPumpFlow[PUMP_2].GetSubject());
  ndi_flow[PUMP_3] = new mpc::display::NumericalDataPointInterface(mpPumpFlow[PUMP_3].GetSubject());
  ndi_flow[PUMP_4] = new mpc::display::NumericalDataPointInterface(mpPumpFlow[PUMP_4].GetSubject());
  ndi_flow[PUMP_5] = new mpc::display::NumericalDataPointInterface(mpPumpFlow[PUMP_5].GetSubject());
  ndi_flow[PUMP_6] = new mpc::display::NumericalDataPointInterface(mpPumpFlow[PUMP_6].GetSubject());

  s1[0]=0;
  strncat(s1, Languages::GetInstance()->GetString( SID_SMS_STATUS_ ), SMS_STR_MAX);        // "Status:"
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, "\n", max_len);                                                        // "\n"

  /* Water Level */
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, Languages::GetInstance()->GetString( SID_SMS_LEVEL_ ), max_len);     // "Level:"
  if( max_len>0 )
    strncat(s1, " ", max_len);                                                       // " "
  if( mpPitLevelCtrlType->GetValue()==SENSOR_TYPE_FLOAT_SWITCHES  )
  {
    sprintf( s2, "%d/%d", mpFloatSwitchWaterLevel->GetValue(), mpFloatSwitchWaterLevel->GetMaxValue() );  //"3/5"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat( s1, s2, max_len);
  }
  else
  {
    ndi_surface_level.GetDataPointAsString(s2, 2);
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, s2, max_len);                                                           // "1.9"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, Languages::GetInstance()->GetString(MpcUnits::GetInstance()->GetActualUnitString(mpSurfaceLevel->GetQuantity())), max_len); // "m"
  }

  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, "\n", max_len);                                                         // CR LF

  /* Flow */
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, Languages::GetInstance()->GetString( SID_SMS_FLOW_ ), max_len);         // "Flow rate:"
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, " ", max_len);                                                       // " "
  ndi_sys_flow.GetDataPointAsString(s2, 3);
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, s2, max_len);                                                         // "10000"
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, Languages::GetInstance()->GetString(MpcUnits::GetInstance()->GetActualUnitString(mpSysFlow->GetQuantity())), max_len); // "m3/h"
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, "\n", max_len);                                                         // CR LF

  /* Volume */
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, Languages::GetInstance()->GetString( SID_SMS_VOLUME_ ), max_len);         // "Volume:"
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, " ", max_len);                                                       // " "
  ndi_sys_volume.GetDataPointAsString(s2, 6);
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, s2, max_len);                                                         // "10000"
  max_len = SMS_STR_MAX - strlen(s1);
  if( max_len>0 )
    strncat(s1, Languages::GetInstance()->GetString(MpcUnits::GetInstance()->GetActualUnitString(mpSysVolume->GetQuantity())), max_len); // "m3"
  max_len = SMS_STR_MAX - strlen(s1);

  
  {
	  //what is the sms sequence number (1,2,3 large SMSs)
	  int i=0;
  switch(sequence)
  {
  case 0:
  case 1: i =0;
	  break;
  case 2:
	  i=2;
	  break;
  case 3:
	  i=4;
	  break;
  default: i=0;
  }
  /* Each Pumpe */
  for(int j=i; (i<mpNoOfPumps->GetValue()) && (i<(j+2)); i++)
  {
    if( max_len>0 )
      strncat(s1, "\n", max_len);                                                         // CR LF
    /* Pump X */
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, Languages::GetInstance()->GetString( str_id_pump[i] ), max_len);      // "Pump 1:"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, "\n", max_len);                                                      // ""CR LF

    /* Operating hours */
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, Languages::GetInstance()->GetString( SID_SMS_OPERATING_HOURS_ ), max_len); // "Operating hours:"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, " ", max_len);                                                       // " "
    ndi_operating_hour[i]->GetDataPointAsString(s2, 6);
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, s2, max_len);                                                         // "10000"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, Languages::GetInstance()->GetString(MpcUnits::GetInstance()->GetActualUnitString(mpPumpOperatingTime[i]->GetQuantity())), max_len); // "h"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, "\n", max_len);                                                       // CR LF

    /* Starts */
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, Languages::GetInstance()->GetString( SID_SMS_NUMBER_OF_STARTS_ ), max_len);// "Number of starts:"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, " ", max_len);                                                       // " "
    ndi_starts[i]->GetDataPointAsString(s2,6);
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, s2, max_len);                                                         // "10000"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, "\n", max_len);                                                       // CR LF

    /* Flow */
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, Languages::GetInstance()->GetString( SID_SMS_FLOW_ ), max_len);         // "Flow rate:"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, " ", max_len);                                                       // " "
    ndi_flow[i]->GetDataPointAsString(s2, 3);
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, s2, max_len);                                                         // "10000"
    max_len = SMS_STR_MAX - strlen(s1);
    if( max_len>0 )
      strncat(s1, Languages::GetInstance()->GetString(MpcUnits::GetInstance()->GetActualUnitString(mpPumpFlow[i]->GetQuantity())), max_len); // "m3/h"
    max_len = SMS_STR_MAX - strlen(s1);
    //if( max_len>0 )
    //  strncat(s1, "\n", max_len);                                                      // CR LF

  }
  }
  delete ndi_operating_hour[0];
  delete ndi_operating_hour[1];
  delete ndi_starts[0];
  delete ndi_starts[1];
  delete ndi_flow[0];
  delete ndi_flow[1];

  my_sms->SetSmsMessage(s1);
  return my_sms;
}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - strupr
 * DESCRIPTION: Converts a string to uppercase
 *
 *****************************************************************************/
void SmsCtrl::strupr(char* pStr)
{
  char* c = pStr;

  while ( *c != '\0' )
  {
    if (islower (*c))
      *c = toupper (*c);
    c++;
  }
}

/*****************************************************************************
 * Function - strupr
 * DESCRIPTION: Converts up to len chars of a string to uppercase
 *
 *****************************************************************************/
void SmsCtrl::strnupr(char* pStr, unsigned int len)
{
  char* c = pStr;
  int cnt = 0;

  while ( *c != '\0' )
  {
    cnt++;
    if( cnt>len )
      break;
    if (islower (*c))
      *c = toupper (*c);
    c++;
  }
}

/*****************************************************************************
 * Function - GetInstallationName
 * DESCRIPTION: Gets the valid (UTF8) installation name
 *
 *****************************************************************************/
const char* SmsCtrl::GetInstallationName()
{ 
 // if(mpInstallationName.IsUpdated())
 // {
    char* p_string = new char[8 + 1];
    strncpy(p_string, mpInstallationName->GetValue(), 8);  
    p_string[8] = '\0';
    mpSmsInstallationName->SetValue(p_string);
    mpSmsInstallationName->ValidateStringUTF8(true);
    delete p_string;
 // } 

 return mpSmsInstallationName->GetValue();
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
