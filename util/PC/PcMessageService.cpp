/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/*                                                                          */
/* CLASS NAME       : PcMessageService                                      */
/*                                                                          */
/* FILE NAME        : PcMessageService.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 28-07-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <string>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <SmsCtrl.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "PcMessageService.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MAX_NO_OF_QUEUE_ELEMENTS 10


PcMessageService* PcMessageService::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function: GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
PcMessageService* PcMessageService::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new PcMessageService();

    // should use same task controller as SmsCtrl
    TaskCtrl* p_task = GetLowPrioPeriodicTask();
    p_task->AddSubTask(mInstance);
    mInstance->SetTaskCtrlPointer(p_task);
  }
  return mInstance;
}


/*****************************************************************************
 * Function - SendMsg
 * DESCRIPTION:
 *
 *****************************************************************************/
void PcMessageService::SendMsg(const char* pMsg)
{
  EnterCriticalSection(&mMsgQueueLock);
  if (mMsgQueue.size() < MAX_NO_OF_QUEUE_ELEMENTS)
  {
    char* p_msg = new char[MAX_MESSAGE_LEN + 1];
    strncpy(p_msg, pMsg, MAX_MESSAGE_LEN);
    p_msg[MAX_MESSAGE_LEN] = '\0';

    mMsgQueue.push(p_msg);
  }
  LeaveCriticalSection(&mMsgQueueLock);
}

/*****************************************************************************
 * Function - GetMsg
 * DESCRIPTION:
 *
 *****************************************************************************/
const char* PcMessageService::GetMsg(void)
{
  const char* p_msg = NULL;

  EnterCriticalSection(&mMsgQueueLock);
  if (!mMsgQueue.empty())
  {
    p_msg = mMsgQueue.front();
    mMsgQueue.pop();
  }
  LeaveCriticalSection(&mMsgQueueLock);
  
  return p_msg;
}


/*****************************************************************************
 * Function - SendSms
 * DESCRIPTION:
 *
 *****************************************************************************/
void PcMessageService::SendSms(SmsOut* pSms)
{
  EnterCriticalSection(&mSmsQueueLock);
  if (mSmsOutQueue.size() < MAX_NO_OF_QUEUE_ELEMENTS )
  {
    SmsOut* p_sms = new SmsOut();
    p_sms->SetPrimaryNumber( pSms->GetPrimaryNumber() );
    p_sms->SetSecondaryNumber( pSms->GetSecondaryNumber() );
    p_sms->SetSmsMessage( pSms->GetSmsMessage() );
    mSmsOutQueue.push( p_sms );
  }
  LeaveCriticalSection(&mSmsQueueLock);
}

/*****************************************************************************
 * Function - SendSmsToCu361
 * DESCRIPTION:
 *
 *****************************************************************************/
void PcMessageService::SendSmsToCu361(const char* pMessage, char* pFromNumber)
{
  EnterCriticalSection(&mSmsQueueLock);
  if (mSmsInQueue.size() < MAX_NO_OF_QUEUE_ELEMENTS )
  {
    SmsIn* p_sms = new SmsIn();
    p_sms->SetSmsMessage( pMessage );
    p_sms->SetSmsNumber( pFromNumber );
    mSmsInQueue.push( p_sms );
  }
  LeaveCriticalSection(&mSmsQueueLock);

  ReqTaskTime();
}

/*****************************************************************************
 * Function - GetSms
 * DESCRIPTION:
 *
 *****************************************************************************/
SmsOut* PcMessageService::GetSms(void)
{
  SmsOut* p_sms = NULL;

  EnterCriticalSection(&mSmsQueueLock);
  if (!mSmsOutQueue.empty())
  {
    p_sms = mSmsOutQueue.front();
    mSmsOutQueue.pop();
  }
  LeaveCriticalSection(&mSmsQueueLock);

  return p_sms;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PcMessageService::RunSubTask()
{
  EnterCriticalSection(&mSmsQueueLock);
  if (!mSmsInQueue.empty())
  {
    SmsIn* p_sms = mSmsInQueue.front();
    mSmsInQueue.pop();

    SmsCtrl::GetInstance()->HandleSmsIn(p_sms);
  }
  LeaveCriticalSection(&mSmsQueueLock);
}



/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
PcMessageService::PcMessageService()
{
  InitializeCriticalSection(&mMsgQueueLock);
  InitializeCriticalSection(&mSmsQueueLock);
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 *****************************************************************************/
PcMessageService::~PcMessageService()
{
  DeleteCriticalSection(&mMsgQueueLock);
  DeleteCriticalSection(&mSmsQueueLock);
}
