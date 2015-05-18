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
/* CLASS NAME       : State                                                  */
/*                                                                          */
/* FILE NAME        : ActiveAlarmList.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a ActiveAlarmList.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ActiveAlarmList.h"
#include <AlarmEvent.h>
#include <AlarmLog.h>
#include <Text.h>
#include <TimeText.h>
#include <DisplayController.h>
#include <ActiveAlarmListItem.h>

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
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    ActiveAlarmList::ActiveAlarmList(Component* pParent) : AlarmLogList(pParent)
    {
      ActiveAlarmListItem*  p_lst_item;
      for(int i=0; i < ALARM_LOG_SIZE;++i)
      {
        p_lst_item = new ActiveAlarmListItem();
        p_lst_item->SetAlarmIndex(i);
        InsertItem(i,p_lst_item);
      }
      SetSelection(0);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ActiveAlarmList::~ActiveAlarmList()
    {

    }
    
    Leds ActiveAlarmList::GetLedsStatus()
    {
      Leds leds = AlarmLogList::GetLedsStatus();
      if (mpEventDp.IsValid())
      {
        leds |= OK_LED;
      }

      return leds;
    }

    Keys ActiveAlarmList::GetLegalKeys()
    {
      Keys keys = AlarmLogList::GetLegalKeys();
      if(mpEventDp.IsValid())
      {
        keys |= MPC_OK_KEY;
      }
      keys |= MPC_ESC_KEY;
      return keys;
    }

    bool ActiveAlarmList::HandleKeyEvent(Keys KeyID)
    {
      switch(KeyID)
      {
      case MPC_OK_KEY:
        if(mpEventDp.IsValid())
        {
          mpEventDp->SetEvent();
          return true;
        }
        break;
      case MPC_ESC_KEY:
        DisplayController::GetInstance()->Pop();
        return true;
      }
      return AlarmLogList::HandleKeyEvent(KeyID);
    }

    void ActiveAlarmList::SubscribtionCancelled(Subject* pSubject)
    {
      mpEventDp.Detach(pSubject);
      AlarmLogList::SubscribtionCancelled(pSubject);
    }
    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void ActiveAlarmList::SetSubjectPointer(int id, Subject* pSubject)
    {
      switch (id)
      {
      case SP_AAL_ALARM_LOG:
        AlarmLogList::SetSubjectPointer(id, pSubject);
        break;
      case SP_AAL_ALARM_RESET_EVENT:
        mpEventDp.Attach(pSubject);
        break;
      }
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void ActiveAlarmList::ConnectToSubjects(void)
    {
      if (mpEventDp.IsValid())
      {
        mpEventDp->Subscribe(this); 
      }
      
      AlarmLogList::ConnectToSubjects();
    }



  } // namespace display
} // namespace mpc


