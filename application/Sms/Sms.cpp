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
/* CLASS NAME       : Sms                                                   */
/*                                                                          */
/* FILE NAME        : Sms.cpp                                               */
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
#include <Factory.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <Sms.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              Class Sms functions
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
Sms::Sms()
{
  mMessageLength = 0;                   // Initialize to an empty message
  mMessage[0] = 0;
  mMessage[TEXT_MAX_LENGTH] = 0;        //Insert 0-terminator
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
Sms::~Sms()
{
}

/*****************************************************************************
 * Function - GetSmsMessage
 * DESCRIPTION: Returns the sms mesage as a 0-terminated char array
 *
 *****************************************************************************/
const char* Sms::GetSmsMessage(void)
{
  return mMessage;
}

/*****************************************************************************
 * Function - SetSmsMessage
 * DESCRIPTION: Set the sms messsage
 *
 *****************************************************************************/
void Sms::SetSmsMessage(const char *message)
{
  strncpy(mMessage, message, TEXT_MAX_LENGTH);
  mMessageLength = strlen(mMessage);
}

/*****************************************************************************
 * Function - GetMessageLength
 * DESCRIPTION: Returns the length of the message
 *
 *****************************************************************************/
int Sms::GetMessageLength(void)
{
  return mMessageLength;
}

/*****************************************************************************
 *
 *
 *              Class SmsIn functions
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
SmsIn::SmsIn()
{
  mNumber[0]=0;                        //Initialize to an empty phone number
  mNumber[PHONO_NO_MAX_LENGTH] = 0;    //Insert 0-terminator
}

/*****************************************************************************
 * Function - GetSmsNumber
 * DESCRIPTION: Returns the number of the sender
 *
 *****************************************************************************/
const char* SmsIn::GetSmsNumber(void)
{
  return mNumber;
}

/*****************************************************************************
 * Function - SetSmsNumber
 * DESCRIPTION: Set the sender number
 *
 *****************************************************************************/
void SmsIn::SetSmsNumber(char *nr)
{
  strncpy(mNumber, nr, PHONO_NO_MAX_LENGTH);
}

/*****************************************************************************
 *
 *
 *              Class SmsOut functions
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
SmsOut::SmsOut()
{
  mRetransCnt = 0;
  mInsertInstallationName = true;
  mSmsState = SMS_QUEUE_PRI;
  mSecondaryNumber[0]=0;                      //Initialize to an empty phone number
  mPrimaryNumber[0]=0;                        //Initialize to an empty phone number
  mSendTo = SMS_RECIPIENT_PRI;
  mPrimaryNumber[PHONO_NO_MAX_LENGTH] = 0;    //Insert 0-terminator
  mSecondaryNumber[PHONO_NO_MAX_LENGTH] = 0;  //Insert 0-terminator
}

/*****************************************************************************
 * Function - GetTransmitNumber
 * DESCRIPTION: Returns the number the sms must be sent to
 *
 *****************************************************************************/
const char* SmsOut::GetTransmitNumber(void)
{
  const char* ret_val;

  switch ( mSmsState )
  {
    case SMS_QUEUE_PRI :          //Should not happen
      ret_val = mPrimaryNumber;
      break;
    case SMS_QUEUE_SEC :          //Should not happen
      ret_val = mSecondaryNumber;
      break;
    case SMS_SENDING_PRI :
      ret_val = mPrimaryNumber;
      break;
    case SMS_SENDING_SEC :
      ret_val = mSecondaryNumber;
      break;
    case SMS_WAIT_FOR_ACK_PRI :   //Should not happen
      ret_val = mPrimaryNumber;
      break;
    default :                //Should not happen
      ret_val = mPrimaryNumber;
      break;
  }
  return ret_val;
}

/*****************************************************************************
 * Function - SetPrimaryNumber
 * DESCRIPTION: Set the primary number for the sms
 *
 *****************************************************************************/
void SmsOut::SetPrimaryNumber( const char* aPriNumber )
{
  strncpy(mPrimaryNumber, aPriNumber, PHONO_NO_MAX_LENGTH);
}

/*****************************************************************************
 * Function - SetSecondaryNumber
 * DESCRIPTION: Set the secondary number for the sms
 *
 *****************************************************************************/
void SmsOut::SetSecondaryNumber( const char* aSecNumber )
{
  strncpy(mSecondaryNumber, aSecNumber, PHONO_NO_MAX_LENGTH);
}

/*****************************************************************************
 * Function - GetPrimaryNumber
 * DESCRIPTION: Return the primary phone number for the sms
 *
 *****************************************************************************/
const char* SmsOut::GetPrimaryNumber()
{
  return mPrimaryNumber;
}

/*****************************************************************************
 * Function - GetSecondaryNumber
 * DESCRIPTION: Return the sesondary phone number for the sms
 *
 *****************************************************************************/
const char* SmsOut::GetSecondaryNumber()
{
  return mSecondaryNumber;
}

/*****************************************************************************
 * Function - IsWaitingForAck
 * DESCRIPTION: Returns true if the sms is waiting for ack
 *
 *****************************************************************************/
bool SmsOut::IsWaitingForAck(void)
{
  if( mSmsState==SMS_WAIT_FOR_ACK_PRI )
    return true;
  else
    return false;
}

/*****************************************************************************
 * Function - GetSmsState
 * DESCRIPTION: Returns the transmitting state
 *
 *****************************************************************************/
SMS_STATE_TYPE SmsOut::GetSmsState( void )
{
  return mSmsState;
}

/*****************************************************************************
 * Function - SmsSent
 * DESCRIPTION: Sms transmit session has finished, the sms has been sent,
 * update transmit state and check if it is ok to delete the sms
 *****************************************************************************/
bool SmsOut::SmsSent(void)
{
  bool ret_val = true;                    // Sms sender must now delete sms

  switch ( mSmsState )
  {
    case SMS_SENDING_PRI :
      switch ( mSendTo )
      {
        case SMS_RECIPIENT_PRI:
          break;
        case SMS_RECIPIENT_PRI_SEC:
          mSmsState = SMS_QUEUE_SEC;
          ret_val = false;                  // Sms sender must NOT delete sms
          break;
        case SMS_RECIPIENT_PRI_SEC_IF_NO_ACK:
          mSmsState = SMS_WAIT_FOR_ACK_PRI;
          ret_val = false;                  // Sms sender must NOT delete sms
          break;
      }
      break;
    case SMS_SENDING_SEC :
      /* Do nothing */
      break;

    case SMS_QUEUE_PRI :
    case SMS_QUEUE_SEC :
    case SMS_WAIT_FOR_ACK_PRI :
      FatalErrorOccured("Sms, illegal state");
      break;
  default:
      FatalErrorOccured("Sms, unknown state");
      break;
  }
  return ret_val;
}

/*****************************************************************************
 * Function - Transmit
 * DESCRIPTION: Transmit session has started, update transmit state
 *
 *****************************************************************************/
void SmsOut::Transmit(void)
{
  switch ( mSmsState)
  {
    case SMS_QUEUE_PRI :
      mSmsState = SMS_SENDING_PRI;
      break;
    case SMS_QUEUE_SEC :
      mSmsState = SMS_SENDING_SEC;
      break;
    case SMS_SENDING_PRI :
    case SMS_SENDING_SEC :
    case SMS_WAIT_FOR_ACK_PRI :
      FatalErrorOccured("Sms, illegal state");
      break;
  default :
      FatalErrorOccured("Sms, unknown state");
      break;
  }
}

/*****************************************************************************
 * Function - SetSendTo
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsOut::SetSendTo( SMS_RECIPIENT_TYPE sentTo)
{
  mSendTo = sentTo;
}

/*****************************************************************************
 * Function - SetRetransCounter
 * DESCRIPTION: Initialize retranscounter
 *
 *****************************************************************************/
void SmsOut::SetRetransCounter(int aCnt)
{
  mRetransCnt = aCnt;
}

/*****************************************************************************
 * Function - UpdateTimeToResend
 * DESCRIPTION: Decrement retranscounter. If time is up and sms is waiting for
 *              act, transmit state is set to queue
 *****************************************************************************/
void SmsOut::UpdateTimeToResend(void)
{
  if(mSmsState==SMS_WAIT_FOR_ACK_PRI)
  {
    if( mRetransCnt>0 )
    {
      mRetransCnt--;
      if( mRetransCnt==0 )
      {
        mSmsState=SMS_QUEUE_SEC;
      }
    }
  }
}

/*****************************************************************************
 * Function - InsertInstallationName
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsOut::InsertInstallationName( const char* aName )
{
  auto char mes[TEXT_MAX_LENGTH+1+10]; //install name + ": " requires max 10 chars

  if( mInsertInstallationName==true )
  {
    mes[0] = 0;
    strncat( mes, aName, 8 );
    strcat( mes, ": " );
    strncat( mes, mMessage, TEXT_MAX_LENGTH+1+10 );
    SetSmsMessage(mes);
  }
}
/*****************************************************************************
 * Function - AllowInstallationName
 * DESCRIPTION:
 *
 *****************************************************************************/
void SmsOut::AllowInstallationName( bool state )
{
  mInsertInstallationName = state;
}





