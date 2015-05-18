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
/* FILE NAME        : AlarmLogList.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AlarmLogList.                */
/*                                                                          */
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
#include "AlarmLogList.h"
#include <AlarmLog.h>
#include <Text.h>
#include <label.h>
#include <TimeText.h>
#include <AlarmListItem.h>

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
    AlarmLogList::AlarmLogList(Component* pParent) : ListView(pParent)
    {
      // The error messages depends on the language
      Languages::GetInstance()->Subscribe(this);

      SetRowHeight(65); // Each row is a listview of 3 rows.

      InsertColumn(0);
      SetColumnWidth(0, 228);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AlarmLogList::~AlarmLogList()
    {
      Languages::GetInstance()->Unsubscribe(this);
    }

    /* --------------------------------------------------
    * Returns the pointer to the Subject. 0 if no subject
    * is defined
    * --------------------------------------------------*/
    Subject* AlarmLogList::GetSubject()
    {
      return mpAlarmLog.GetSubject();
    }

    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void AlarmLogList::SubscribtionCancelled(Subject* pSubject)
    {
      mpAlarmLog.Detach(pSubject);
      
      Invalidate();
    }

   /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void AlarmLogList::SetSubjectPointer(int id, Subject* pSubject)
    {
      if (mpAlarmLog.Attach(pSubject))
      {
        int i = 0;
        const U16 row_count = mRows.size();
        AlarmListItem*  alarm_list_item;
        for(i=0; i < row_count; ++i)
        {
          alarm_list_item = dynamic_cast<AlarmListItem*>(GetListViewItem(i));
          alarm_list_item->SetSubjectPointer(id, GetSubject());
        }
        Invalidate();
      }
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void AlarmLogList::ConnectToSubjects(void)
    {
      if (mpAlarmLog.IsValid())
      {
        mpAlarmLog->Subscribe(this);
        int i = 0;
        const U16 row_count = mRows.size();
        AlarmListItem*  alarm_list_item;
        for(i=0; i < row_count; ++i)
        {
          alarm_list_item = dynamic_cast<AlarmListItem*>(GetListViewItem(i));
          alarm_list_item->ConnectToSubjects();
        }
        Invalidate();
      }
    }

    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void AlarmLogList::Update(Subject* pSubject)
    {
      Invalidate();
    }
  } // namespace display
} // namespace mpc


