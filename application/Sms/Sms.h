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
/* FILE NAME        : Sms.h                                                 */
/*                                                                          */
/* CREATED DATE     : 30-11-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :  Defines sms classes                            */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcSms_h
#define mrcSms_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <string>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define PHONO_NO_MAX_LENGTH 16     //Number og caracters, Exclucive 0-terminator
#define TEXT_MAX_LENGTH    840     //Number og caracters(bytes, unicode caracters may use more than one byte), Exclucive 0-terminator.

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  SMS_QUEUE_PRI,
  SMS_QUEUE_SEC,
  SMS_SENDING_PRI,
  SMS_SENDING_SEC,
  SMS_WAIT_FOR_ACK_PRI
}SMS_STATE_TYPE;

 /*****************************************************************************
 * CLASS:
 * DESCRIPTION: Sms is a base class for SmsIn and SmsOut. It controls the message
 *
 *****************************************************************************/
class Sms
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    Sms();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~Sms();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    const char* GetSmsMessage(void);
    void SetSmsMessage(const char *);
    int GetMessageLength(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    int mMessageLength;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    char mMessage[TEXT_MAX_LENGTH+1];
};

 /*****************************************************************************
 * CLASS:  SmsIn
 * DESCRIPTION: SmsIn is a received sms, it knows the message and the phone number
 *              of the sender
 *****************************************************************************/
class SmsIn : public Sms
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SmsIn();

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void SetSmsNumber(char *);
    const char* GetSmsNumber(void);

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

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    char mNumber[PHONO_NO_MAX_LENGTH+1]; //Allocate space for 0-terminator

};

 /*****************************************************************************
 * CLASS:  SmsOut
 * DESCRIPTION: SmsOut is a sms ready to be transmittet. It knows the message,
 *              the primary number and the secondary number. It controls the
 *               transmit sequence: primary -> wait for ack -> secondary
 *****************************************************************************/
class SmsOut : public Sms
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SmsOut();

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    const char* GetTransmitNumber(void);
    void SetPrimaryNumber(const char* aPriNumber);
    void SetSecondaryNumber(const char* aSecNumber);
    const char* GetPrimaryNumber();
    const char* GetSecondaryNumber();
    void UpdateTimeToResend(void);
    SMS_STATE_TYPE GetSmsState(void);
    void Transmit(void);
    bool SmsSent(void);
    void SetRetransCounter(int aCnt);
    void SetSendTo( SMS_RECIPIENT_TYPE );
    bool IsWaitingForAck(void);
    void AllowInstallationName( bool state );
    void InsertInstallationName( const char* aName );

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

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    int mRetransCnt;
    bool mInsertInstallationName;
    SMS_STATE_TYPE mSmsState;
    SMS_RECIPIENT_TYPE mSendTo;
    char mPrimaryNumber[PHONO_NO_MAX_LENGTH+1];    //Allocate space for 0-terminator
    char mSecondaryNumber[PHONO_NO_MAX_LENGTH+1];  //Allocate space for 0-terminator
};


#endif

