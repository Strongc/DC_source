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
/* CLASS NAME       : State                                                 */
/*                                                                          */
/* FILE NAME        : AlarmLogLogList.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AlarmLogLogList.             */
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
#include "AlarmLogLogList.h"
#include <AlarmEvent.h>
#include <AlarmLog.h>
#include <AlarmListItem.h>
#include <SetIntDatapointAction.h>

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
    AlarmLogLogList::AlarmLogLogList(Component* pParent) : AlarmLogList(pParent)
    {
      AlarmListItem*  p_lst_item;
      SetIntDatapointAction* p_action;

      InsertColumn(1);

      for (int i = 0; i < ALARM_LOG_SIZE; i++)
      {
        p_lst_item = new AlarmListItem();
        p_lst_item->SetAlarmIndex(i);
        
        InsertItem(i, p_lst_item);
        
        p_action = new SetIntDatapointAction();
        p_action->InitValue(i); 
        SetItem(i, 1, p_action);        
      }

      SetSelection(0);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AlarmLogLogList::~AlarmLogLogList()
    {

    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void AlarmLogLogList::SetSubjectPointer(int id, Subject* pSubject)
    {
      switch(id)
      {
      case SP_ALLL_ALARM_LOG: 
        AlarmLogList::SetSubjectPointer(id, pSubject);
        break;
      case SP_ALLL_SLIPPOINT_CURRENT_NUMBER:
        for (int i = 0; i < ALARM_LOG_SIZE; i++)
        {
          ((SetIntDatapointAction*)GetItem(i,1))->SetSubjectPointer(id, pSubject);
        }
        break;
        
      }
    }
  } // namespace display
} // namespace mpc


