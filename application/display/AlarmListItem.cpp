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
/* CLASS NAME       : AlarmListItem                                         */
/*                                                                          */
/* FILE NAME        : AlarmListItem.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AlarmListItem.               */
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
#include <AlarmText.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AlarmListItem.h"
#include "Group.h"
#include "Text.h"
#include "Label.h"
#include "TimeText.h"
#include "Image.h"
#include "DataPointText.h"
#include <Factory.h>
#include <TimeFormatDataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmWarning;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmAlarm;
namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AlarmListItem::AlarmListItem(Component* pParent) : ListViewItem(pParent)
    {
      mLastNeverAvailable = true;
      mAlarmIndex = -1;

      int y = 2;
      mGroup = new Group();

      mIcon = new Image();
      mIcon->SetBitmap(&bmWarning);
      mIcon->SetClientArea(2,y,17,y+16);
      mIcon->SetTransparent(false);
      mGroup->AddChild(mIcon);

      y+=2;
      mLabelErrorUnit = new Label();
      mLabelErrorUnit->SetAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
      mLabelErrorUnit->SetClientArea(19,y,225,y+14);
      mGroup->AddChild(mLabelErrorUnit);
      y += 14;
      
      mLabelErrorString = new Label();
      mLabelErrorString->SetAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
      mLabelErrorString->SetClientArea(5,y,225,y+14);
      mGroup->AddChild(mLabelErrorString);

      mDataPointTextErrorString = new DataPointText();
      mDataPointTextErrorString->SetAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
      mDataPointTextErrorString->SetClientArea(5,y,225,y+14);
      mDataPointTextErrorString->SetVisible(false);
      mGroup->AddChild(mDataPointTextErrorString);
      y += 14;


      // Arrival time
      mLabelArrivalTime = new Label();
      mLabelArrivalTime->SetStringId(SID_ARRIVAL_TIME);
      mLabelArrivalTime->SetAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
      mLabelArrivalTime->SetClientArea(5,y,120,y+14);
      mGroup->AddChild(mLabelArrivalTime);



      mArrivalTime = new TimeText();
      mArrivalTime->SetAlign(GUI_TA_RIGHT|GUI_TA_VCENTER);
      mArrivalTime->SetClientArea(121,y,225,y+14);
      mGroup->AddChild(mArrivalTime);

      TimeFormatDataPoint* pDpTimePreference = TimeFormatDataPoint::GetInstance();
      mArrivalTime->SetSubjectPointer(0,pDpTimePreference);
      mArrivalTime->ConnectToSubjects();

      y += 14;

      // Disappearing time
      mLabelDisappearingTime = new Label();
      mLabelDisappearingTime->SetStringId(SID_DEPARTURE_TIME);
      mLabelDisappearingTime->SetAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
      mLabelDisappearingTime->SetClientArea(5,y,120,y+14);
      mGroup->AddChild(mLabelDisappearingTime);

      mDisappearingTime = new TimeText();
      mDisappearingTime->SetAlign(GUI_TA_RIGHT|GUI_TA_VCENTER);
      mDisappearingTime->SetClientArea(121,y,225,y+14);
      mDisappearingTime->SetSubjectPointer(0,pDpTimePreference);
      mDisappearingTime->ConnectToSubjects();

      mGroup->AddChild(mDisappearingTime);
      y += 14;

      InsertItem(0,mGroup);
      mDisappearingTime->SetVisible();
      mLabelDisappearingTime->SetVisible();
      mArrivalTime->SetVisible();
      mLabelArrivalTime->SetVisible();
      mIcon->SetVisible();
      mLabelErrorString->SetVisible();
      mLabelErrorUnit->SetVisible();
      mGroup->SetVisible();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AlarmListItem::~AlarmListItem()
    {

    }

    void AlarmListItem::SetAlarmIndex(int alarmIndex)
    {
      if(mAlarmIndex != alarmIndex)
      {
        mAlarmIndex = alarmIndex;
        Invalidate();
      }
    }

    bool AlarmListItem::IsNeverAvailable()
    {
      return mLastNeverAvailable;
    }

    void AlarmListItem::Run( )
    {
      /* If the alarm log has changed update. One of the things that may be 
        updated is the availability of this item (mLastNeverAvailable). Since
        ListView::Run first redraws it self then runs ListViewItems calling
        ListViewItem::Run on updates would cause the listview to miss the 
        update, because the ListViewItem::Run would validate the item.
        ListView::Redraw depends on the availability of the ListViewItems, the
        AlarmListItem is only Invalidated on the first call to Run and then 
        redrawn on the second call, to make invalidation work correctly.
      */
      if (mpAlarmLog.IsUpdated())
      {
        // Simply update data and invalidate, then wait for next call to Run
        // No redrawing yet.

        mpAlarmLog->UseAlarmLog();
        UpdateAvailability();


        AlarmEvent*  p_alarm_event = mpAlarmLog->GetAlarmLogElement(mAlarmIndex);
        ALARM_ID_TYPE alarm_id = p_alarm_event->GetAlarmId();
        ERRONEOUS_UNIT_TYPE erroneous_unit = p_alarm_event->GetErroneousUnit();
        int erroneous_unit_number = p_alarm_event->GetErroneousUnitNumber();


        if (alarm_id != ALARM_ID_NO_ALARM)
        {
          STRING_ID string_id = GetUnitString(erroneous_unit, erroneous_unit_number);
          mLabelErrorUnit->SetStringId(string_id);

          if (alarm_id == ALARM_ID_COMBI_ALARM)
          {
            StringDataPoint* p_dp_combi_alarm = AlarmText::GetInstance()->GetCombiAlarm(erroneous_unit_number); 
            mDataPointTextErrorString->SetSubjectPointer(-1, p_dp_combi_alarm);
            mDataPointTextErrorString->SetVisible();
            mDataPointTextErrorString->ConnectToSubjects();
          }
          else if ((alarm_id == ALARM_ID_EXTRA_FAULT_1) ||
                   (alarm_id == ALARM_ID_EXTRA_FAULT_2) ||
                   (alarm_id == ALARM_ID_EXTRA_FAULT_3) ||
                   (alarm_id == ALARM_ID_EXTRA_FAULT_4))
          {
            StringDataPoint* p_dp_extra_fault = AlarmText::GetInstance()->GetExtraFault(erroneous_unit_number); 
            mDataPointTextErrorString->SetSubjectPointer(-1, p_dp_extra_fault);
            mDataPointTextErrorString->SetVisible();
            mDataPointTextErrorString->ConnectToSubjects();
          }
          else
          {
             string_id = AlarmText::GetInstance()->GetStringId(alarm_id);
             mLabelErrorString->SetStringId(string_id);
             mDataPointTextErrorString->SetVisible(false);
          }
          
          mArrivalTime->SetTime(*(p_alarm_event->GetArrivalTime()));
          mDisappearingTime->SetTime(*(p_alarm_event->GetDepartureTime()));

          switch (p_alarm_event->GetAlarmType())
          {
          case ALARM_STATE_WARNING:
            mIcon->SetBitmap(&bmWarning);
            break;
          case ALARM_STATE_ALARM:
            mIcon->SetBitmap(&bmAlarm);
            break;
          default:
            break;
          }
        }
        mpAlarmLog->UnuseAlarmLog();
        Invalidate();
      }
      else
      {
        // Second (or more) call to run, lets redraw
        ListViewItem::Run();
      }
    }

    bool AlarmListItem::Redraw()
    {
      return ListViewItem::Redraw();
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory), its on purpose we don't subscribe to it
    * the alarmList will take care of it?
    * --------------------------------------------------*/
    void AlarmListItem::SetSubjectPointer(int id,Subject* pSubject)
    {
      if (!mpAlarmLog.IsValid())
      {
        if (mpAlarmLog.Attach(pSubject))
        {
          mpAlarmLog.SetUpdated();
          Invalidate();
        }
      }
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void AlarmListItem::ConnectToSubjects(void)
    {
      if (mpAlarmLog.IsValid())
      {
        mpAlarmLog->Subscribe(this);
      }
    }

    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void AlarmListItem::Update(Subject* pSubject)
    {
      if (mpAlarmLog.Update(pSubject))
      {
        Invalidate();
      }
    }

    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void AlarmListItem::SubscribtionCancelled(Subject* pSubject)
    {
      mpAlarmLog.Detach(pSubject);
    }

    STRING_ID AlarmListItem::GetUnitString(ERRONEOUS_UNIT_TYPE type, int number)
    {
      for(int i = 0; i < DISPLAY_UNIT_STRINGS_CNT; ++i)
      {
/* MS VC 6 may have problems
   If you get an access violation on the 
   DISPLAY_UNIT_STRINGS[i].UnitType == type 
   try to un comment the next 3 lines of code.
        DbUnitStrings* p_us = display_unit_strings+i;
        if( p_us->UnitType == type 
           && p_us->UnitNumber == number )
*/
        if( DISPLAY_UNIT_STRINGS[i].UnitType == type 
           && DISPLAY_UNIT_STRINGS[i].UnitNumber == number )
        {
            return (*(DISPLAY_UNIT_STRINGS+i)).StringId;
        }
      }
      return SID_UNIT_UNKNOWN;
    }

    /*************************************************************************
    * Function
    * DESCRIPTION:
    * Make sure you have the lock on the alarmlog before calling this function
    *************************************************************************/
    void AlarmListItem::UpdateAvailability()
    {
      if (mpAlarmLog.IsValid())
      {
        AlarmEvent*  p_alarm_event = mpAlarmLog->GetAlarmLogElement(mAlarmIndex);
        mLastNeverAvailable = p_alarm_event->GetAlarmId() == ALARM_ID_NO_ALARM;
      }
    }


    /*************************************************************************
    * Function
    * DESCRIPTION:
    * return valid for hidden and never-available alarms items to avoid redraw
    *************************************************************************/
    bool AlarmListItem::IsValid()
    {
      if( mValid == false )
        return false;

      if(IsVisible()==false && IsNeverAvailable() )
        return true;

      return ListViewItem::IsValid();
    }


  } // namespace display
} // namespace mpc


