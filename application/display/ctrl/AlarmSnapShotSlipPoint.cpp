/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : AlarmSnapShotSlipPoint                                */
/*                                                                          */
/* FILE NAME        : AlarmSnapShotSlipPoint.cpp                            */
/*                                                                          */
/* CREATED DATE     : 19-06-2009                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Factory.h>
#include <DataPoint.h>
#include <DisplayController.h>
#include <TimeText.h>
#include <TimeFormatDataPoint.h>
#include <AlarmText.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AlarmSnapShotSlipPoint.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define DISPLAY_ALARM_SNAP_SHOT_ID 150

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    namespace ctrl
    {

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
      AlarmSnapShotSlipPoint::AlarmSnapShotSlipPoint()
      {
        mCurrentlyUpdating = false;

        mpTimeAsText = new TimeText();

      }

      /*****************************************************************************
      * Function - Destructor
      * DESCRIPTION:
      *
      ****************************************************************************/
      AlarmSnapShotSlipPoint::~AlarmSnapShotSlipPoint()
      {
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void AlarmSnapShotSlipPoint::InitSubTask(void)
      {
        if (mDpCurrentNumber.IsValid())
        {
          mDpCurrentNumber->SetAsInt(0);
        }

        UpdateAlarmSnapShot();
        UpdateUpperStatusLine();
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void AlarmSnapShotSlipPoint::RunSubTask(void)
      {
        mCurrentlyUpdating = true;  // Guard the SubTask

        UpdateAlarmSnapShot();
        UpdateUpperStatusLine();

        mCurrentlyUpdating = false; // End of: Guard the SubTask
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void AlarmSnapShotSlipPoint::SubscribtionCancelled(Subject* pSubject)
      {
        // ignore - this never happens
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void AlarmSnapShotSlipPoint::Update(Subject* pSubject)
      {
        if ( !mCurrentlyUpdating )
        {
          ReqTaskTime();
        }
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void AlarmSnapShotSlipPoint::SetSubjectPointer(int id, Subject* pSubject)
      {
        switch (id)
        {
          case SP_ASSSP_CURRENT_NUMBER:
            mDpCurrentNumber.Attach(pSubject);
            break;
          case SP_ASSSP_ALARM_LOG:
            mpAlarmLog.Attach(pSubject);
            break;
            // TODO attach more subjects to read from


            ////////////////////
            // subjects to write to :
            ////////////////////
          case SP_ASSSP_ALARM_STATE:
            mDpVirtualAlarmState.Attach(pSubject);
            break;
          case SP_ASSSP_ERROR_UNIT:
            mDpVirtualErrorUnit.Attach(pSubject);
            break;
          case SP_ASSSP_ALARM_TEXT:
            mDpVirtualAlarmText.Attach(pSubject);
            break;
          case SP_ASSSP_OCCURED_AT:
            mDpVirtualOccurredAt.Attach(pSubject);
            break;
          case SP_ASSSP_DISAPPEARED_AT:
            mDpVirtualDisappearedAt.Attach(pSubject);
            break;
          case SP_ASSSP_SYSTEM_LEVEL:
            mDpVirtualSystemLevel.Attach(pSubject);
            break;
          case SP_ASSSP_SYSTEM_FLOW:
            mDpVirtualSystemFlow.Attach(pSubject);
            break;
          case SP_ASSSP_OPERATION_MODE_PUMP_1:
            mDpVirtualOperationModePump[PUMP_1].Attach(pSubject);
            break;
          case SP_ASSSP_OPERATION_MODE_PUMP_2:
            mDpVirtualOperationModePump[PUMP_2].Attach(pSubject);
            break;
          case SP_ASSSP_OPERATION_MODE_PUMP_3:
            mDpVirtualOperationModePump[PUMP_3].Attach(pSubject);
            break;
          case SP_ASSSP_OPERATION_MODE_PUMP_4:
            mDpVirtualOperationModePump[PUMP_4].Attach(pSubject);
            break;
          case SP_ASSSP_OPERATION_MODE_PUMP_5:
            mDpVirtualOperationModePump[PUMP_5].Attach(pSubject);
            break;
          case SP_ASSSP_OPERATION_MODE_PUMP_6:
            mDpVirtualOperationModePump[PUMP_6].Attach(pSubject);
            break;
          case SP_ASSSP_PUMP_FLOW:
            mDpVirtualPumpFlow.Attach(pSubject);
            break;
          case SP_ASSSP_PUMP_TEMP:
            mDpVirtualPumpTemp.Attach(pSubject);
            break;
          case SP_ASSSP_PUMP_MAINS_VOLTAGE:
            mDpVirtualPumpMainsVoltage.Attach(pSubject);
            break;
          case SP_ASSSP_PUMP_LATEST_CURRENT:
            mDpVirtualPumpLatestCurrent.Attach(pSubject);
            break;
          case SP_ASSSP_PUMP_COS_PHI:
            mDpVirtualPumpCosPhi.Attach(pSubject);
            break;
          case SP_ASSSP_PUMP_POWER:
            mDpVirtualPumpPower.Attach(pSubject);
            break;
          case SP_ASSSP_IS_A_PUMP_ALARM:
            mDpVirtualIsAPumpAlarm.Attach(pSubject);
            break;
        }
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void AlarmSnapShotSlipPoint::ConnectToSubjects(void)
      {
        // use SubscribeE to get update even when alarm index in unchanged
        mDpCurrentNumber->SubscribeE(this);

        mpTimeAsText->SetSubjectPointer(0, TimeFormatDataPoint::GetInstance());
        mpTimeAsText->ConnectToSubjects();
      }



      /*****************************************************************************
      *
      *
      *              PRIVATE FUNCTIONS
      *
      *
      ****************************************************************************/
      namespace
      {
        STRING_ID get_unit_string(ERRONEOUS_UNIT_TYPE type, int number)
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

            if (DISPLAY_UNIT_STRINGS[i].UnitType == type &&
                DISPLAY_UNIT_STRINGS[i].UnitNumber == number)
            {
              return (*(DISPLAY_UNIT_STRINGS+i)).StringId;
            }
          }
          return SID_UNIT_UNKNOWN;
        }
      }


      void AlarmSnapShotSlipPoint::UpdateAlarmSnapShot()
      {
        const int index = mDpCurrentNumber->GetAsInt();
        AlarmEvent* p_alarm_event = mpAlarmLog->GetAlarmLogElement(index);

        mDpVirtualAlarmState->SetValue(p_alarm_event->GetAlarmType());

        mpTimeAsText->SetTime(*(p_alarm_event->GetArrivalTime()));
        mDpVirtualOccurredAt->SetValue(mpTimeAsText->GetText());

        mpTimeAsText->SetTime(*(p_alarm_event->GetDepartureTime()));
        mDpVirtualDisappearedAt->SetValue(mpTimeAsText->GetText());

        const AlarmLogDataHistory::DataHistoryEntry& snap_shot_data = p_alarm_event->GetSnapShotData();

        ERRONEOUS_UNIT_TYPE erroneous_unit_type = p_alarm_event->GetErroneousUnit();
        int pump_error_index = -1;

        if (erroneous_unit_type == ERRONEOUS_UNIT_MP204 ||
            erroneous_unit_type == ERRONEOUS_UNIT_IO111 ||
            erroneous_unit_type == ERRONEOUS_UNIT_PUMP  ||
            erroneous_unit_type == ERRONEOUS_UNIT_CUE   )
        {
          pump_error_index = p_alarm_event->GetErroneousUnitNumber() - 1;
        }

        mDpVirtualErrorUnit->SetValue(Languages::GetInstance()->GetString(get_unit_string(p_alarm_event->GetErroneousUnit(), p_alarm_event->GetErroneousUnitNumber())));
        mDpVirtualAlarmText->SetValue(AlarmText::GetInstance()->GetString(p_alarm_event->GetAlarmId(), p_alarm_event->GetErroneousUnitNumber()));

        mDpVirtualSystemLevel->SetValue(snap_shot_data.mSurfaceLevel);
        if (snap_shot_data.mSurfaceLevelQuality != DP_AVAILABLE || !snap_shot_data.mSurfaceLevelReady)
        {
          mDpVirtualSystemLevel->SetQuality(DP_NOT_AVAILABLE);
        }

        mDpVirtualSystemFlow->SetValue(snap_shot_data.mAverageFlow);
        if (snap_shot_data.mAverageFlowQuality != DP_AVAILABLE)
        {
          mDpVirtualSystemFlow->SetQuality(DP_NOT_AVAILABLE);
        }

        for (unsigned int i = 0; i < NO_OF_PUMPS; ++i)
        {
          mDpVirtualOperationModePump[i]->SetValue(snap_shot_data.mPumpOperatingMode[i]);
          if (snap_shot_data.mPumpOperatingModeQuality[i] != DP_AVAILABLE)
          {
            mDpVirtualOperationModePump[i]->SetQuality(DP_NEVER_AVAILABLE);
          }
        }

        bool is_a_pump_alarm = (pump_error_index >= 0 && pump_error_index < NO_OF_PUMPS);

        mDpVirtualIsAPumpAlarm->SetValue(is_a_pump_alarm);

        if (is_a_pump_alarm)
        {
          mDpVirtualPumpMainsVoltage  ->SetValue(snap_shot_data.mPumpVoltage     [pump_error_index]);
          mDpVirtualPumpLatestCurrent ->SetValue(snap_shot_data.mPumpCurrent     [pump_error_index]);
          mDpVirtualPumpCosPhi        ->SetValue(snap_shot_data.mPumpCosPhi      [pump_error_index]);
          mDpVirtualPumpPower         ->SetValue(snap_shot_data.mPumpPower       [pump_error_index]);
          mDpVirtualPumpFlow          ->SetValue(snap_shot_data.mPumpFlow        [pump_error_index]);
          mDpVirtualPumpTemp          ->SetValue(snap_shot_data.mPumpTemperature [pump_error_index]);

          if (snap_shot_data.mPumpVoltageQuality     [pump_error_index] != DP_AVAILABLE) mDpVirtualPumpMainsVoltage  ->SetQuality(DP_NEVER_AVAILABLE);
          if (snap_shot_data.mPumpCurrentQuality     [pump_error_index] != DP_AVAILABLE) mDpVirtualPumpLatestCurrent ->SetQuality(DP_NEVER_AVAILABLE);
          if (snap_shot_data.mPumpCosPhiQuality      [pump_error_index] != DP_AVAILABLE) mDpVirtualPumpCosPhi        ->SetQuality(DP_NEVER_AVAILABLE);
          if (snap_shot_data.mPumpPowerQuality       [pump_error_index] != DP_AVAILABLE) mDpVirtualPumpPower         ->SetQuality(DP_NEVER_AVAILABLE);
          if (snap_shot_data.mPumpFlowQuality        [pump_error_index] != DP_AVAILABLE) mDpVirtualPumpFlow          ->SetQuality(DP_NEVER_AVAILABLE);
          if (snap_shot_data.mPumpTemperatureQuality [pump_error_index] != DP_AVAILABLE) mDpVirtualPumpTemp          ->SetQuality(DP_NEVER_AVAILABLE);
        }
        else
        {
          mDpVirtualPumpMainsVoltage  ->SetQuality(DP_NEVER_AVAILABLE);
          mDpVirtualPumpLatestCurrent ->SetQuality(DP_NEVER_AVAILABLE);
          mDpVirtualPumpCosPhi        ->SetQuality(DP_NEVER_AVAILABLE);
          mDpVirtualPumpPower         ->SetQuality(DP_NEVER_AVAILABLE);
          mDpVirtualPumpFlow          ->SetQuality(DP_NEVER_AVAILABLE);
          mDpVirtualPumpTemp          ->SetQuality(DP_NEVER_AVAILABLE);
        }
      }


      void AlarmSnapShotSlipPoint::UpdateUpperStatusLine()
      {
        Display* p_display = NULL;
        char display_number[10];

        p_display = GetDisplay(DISPLAY_ALARM_SNAP_SHOT_ID);

        int index = mDpCurrentNumber->GetAsInt() + 1;

        sprintf(display_number, "3.2.%i", index);
        p_display->SetDisplayNumber(display_number);

        DisplayController::GetInstance()->RequestTitleUpdate();
      }
      /*****************************************************************************
      *
      *
      *              PROTECTED FUNCTIONS
      *                 - RARE USED -
      *
      ****************************************************************************/
    } // namespace ctrl
  } // namespace display
} // namespace mpc
