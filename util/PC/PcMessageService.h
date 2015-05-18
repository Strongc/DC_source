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
/* FILE NAME        : PcMessageService.h                                    */
/*                                                                          */
/* CREATED DATE     : 28-07-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_PcMessageService_h
#define mrc_PcMessageService_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <rtos.h>
#include <queue>
#include <windows.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <sms.h>
#include <SubTask.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MAX_MESSAGE_LEN 512

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PcMessageService : public SubTask
{
public:
  static PcMessageService* GetInstance(void);
  
  void SendMsg(const char* pMsg);
  const char* GetMsg(void);

  void SendSmsToCu361(const char* pMessage, char* pFromNumber);
  void SendSms(SmsOut* pSms);
  SmsOut* GetSms(void);

  void InitSubTask(){}
  void RunSubTask();
 
private:
  PcMessageService();
  ~PcMessageService();

  static PcMessageService* mInstance;

  CRITICAL_SECTION  mMsgQueueLock;
  CRITICAL_SECTION  mSmsQueueLock;

  std::queue<char*> mMsgQueue;
  std::queue<SmsOut*> mSmsOutQueue;
  std::queue<SmsIn*> mSmsInQueue;
  
};

#endif
