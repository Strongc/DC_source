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
/* CLASS NAME       : AlarmStatusCtrl                                        */
/*                                                                          */
/* FILE NAME        : AlarmStatusCtrl.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 02-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
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
#include <AlarmStatusCtrl.h>
#include <microp.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

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
AlarmStatusCtrl::AlarmStatusCtrl()
{
  U16 word_idx, num; 

  for (word_idx=0; word_idx<NO_OF_GENI_STATUS_WORDS; word_idx++)
  {
    mSystemAlarmScadaPending[word_idx] = 0;
    mSystemWarningScadaPending[word_idx] = 0;
    for (num=0; num<NO_OF_PUMPS; num++)
    {
      mPumpAlarmScadaPending[word_idx][num] = 0;
      mPumpWarningScadaPending[word_idx][num] = 0;
    }
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AlarmStatusCtrl::~AlarmStatusCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmStatusCtrl::InitSubTask()
{
  mReqTaskTimeFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 * NOTE:      It may be a help to take a look at the Geni profile to get details
 *            about the special bit packing.
 *
 *****************************************************************************/
void AlarmStatusCtrl::RunSubTask()
{
  AlarmEvent* p_alarm_event;
  ALARM_ID_TYPE alarm_id, first_alarm, first_warning;
  U16 bit, word_idx, num;
  bool mPumpAlarmPresent[NO_OF_PUMPS];
  bool update_flag = false;
  mReqTaskTimeFlag = false;

  update_flag =  mpScadaEnabled.IsUpdated();
  update_flag &= mpScadaEnabled->GetValue()==false;
  update_flag |= mpTimeStampLastScadaAck.IsUpdated();
  update_flag |= mpScadaLatchingEnabled.IsUpdated();

  // Clear scada pending bits if scada is acknowledged or disabled
  if (update_flag == true)
  {
    mpAlarmLog.SetUpdated(); // Force update of alarm bits
    for (word_idx=0; word_idx<NO_OF_GENI_STATUS_WORDS; word_idx++)
    {
      mSystemAlarmScadaPending[word_idx] = 0;
      mSystemWarningScadaPending[word_idx] = 0;
      for (num=0; num<NO_OF_PUMPS; num++)
      {
        mPumpAlarmScadaPending[word_idx][num] = 0;
        mPumpWarningScadaPending[word_idx][num] = 0;
      }
    }
  }

  // Check if alarm log has changed
  // update Geni if not in service mode
  if ((mpAlarmLog.IsUpdated()) || (mpServiceModeEnabled.IsUpdated()))
  {
    // Clear temp status variables
    first_alarm = ALARM_ID_NO_ALARM;
    first_warning = ALARM_ID_NO_ALARM;
    mPumpFault = 0;
    mPumpMonitoringFault = 0;

    for (word_idx=0; word_idx<NO_OF_GENI_STATUS_WORDS; word_idx++)
    {
      mSystemAlarmStatus[word_idx] = 0;
      mSystemWarningStatus[word_idx] = 0;
      for (num=0; num<NO_OF_PUMPS; num++)
      {
        mPumpAlarmStatus[word_idx][num] = 0;
        mPumpWarningStatus[word_idx][num] = 0;
      }
    }
    for (int i=0; i<NO_OF_PUMPS; i++)
    {
      mPumpAlarmPresent[i] = false;
    }

    mpAlarmLog->UseAlarmLog();
    for (int i=0; i<ALARM_LOG_SIZE; i++)
    {
      // Get alarm event from log
      p_alarm_event = mpAlarmLog->GetAlarmLogElement(i);
      alarm_id = p_alarm_event->GetAlarmId();

      // Check if the alarm event is active
      if (alarm_id != ALARM_ID_NO_ALARM && p_alarm_event->GetAcknowledge() == false)
      {
        // Find first alarm and warning
        if (first_alarm == ALARM_ID_NO_ALARM && p_alarm_event->GetAlarmType() == ALARM_STATE_ALARM)
        {
          first_alarm = alarm_id;
        }
        else if (first_warning == ALARM_ID_NO_ALARM && p_alarm_event->GetAlarmType() == ALARM_STATE_WARNING)
        {
          first_warning = alarm_id;
        }

        // Find alarm/warning status bits
        switch ( p_alarm_event->GetErroneousUnit() )
        {
          // Pump alarms/warnings
          case ERRONEOUS_UNIT_MP204 :
          case ERRONEOUS_UNIT_IO111 :
          case ERRONEOUS_UNIT_PUMP :
          case ERRONEOUS_UNIT_CUE :
            num = p_alarm_event->GetErroneousUnitNumber()-1;
            if (ConvertPumpAlarmToGeniStatus(alarm_id, word_idx, bit) == true && word_idx < NO_OF_GENI_STATUS_WORDS && num < NO_OF_PUMPS)
            {
              // bit value found for the alarm or warning, set the bit in the related status value
              if (p_alarm_event->GetAlarmType() == ALARM_STATE_ALARM)
              {
                if ( (mpScadaEnabled->GetValue() == true)
                  && (p_alarm_event->GetErrorSource()->GetAlarmConfig()->GetScadaEnabled() == true)
                  && (p_alarm_event->GetArrivalTime()->GetSecondsSince1Jan1970() > mpTimeStampLastScadaAck->GetValue()) )
                {
                  mPumpAlarmScadaPending[word_idx][num] |= (0x1<<bit);
                }
                mPumpAlarmStatus[word_idx][num] |= (0x1<<bit);
                mPumpFault |= (0x1<<num*2);           // Alarm: Set bit 0,2,4...
              }
              else if (p_alarm_event->GetAlarmType() == ALARM_STATE_WARNING)
              {
                if ( (mpScadaEnabled->GetValue() == true)
                  && (p_alarm_event->GetErrorSource()->GetAlarmConfig()->GetScadaEnabled() == true)
                  && (p_alarm_event->GetArrivalTime()->GetSecondsSince1Jan1970() > mpTimeStampLastScadaAck->GetValue()) )
                {
                  mPumpWarningScadaPending[word_idx][num] |= (0x1<<bit);
                }
                mPumpWarningStatus[word_idx][num] |= (0x1<<bit);
                mPumpFault |= (0x2<<num*2);           // Warning: Set bit 1,3,5...
              }
              if (alarm_id == ALARM_ID_GENIBUS_PUMP_MODULE) // Pump / IO111 comm fault
              {
                mPumpMonitoringFault |= (0x1<<num);
              }
            }

            // Set mPumpAlarmPresent for CombiAlarm and PumpCtrl if a pump has any kind of alarm
            if (num < NO_OF_PUMPS)
            {
              // Only search for alarm if none has been registered - no need to use time for registering an alarm more than once
              if (mPumpAlarmPresent[num] == false)
              {
                if (p_alarm_event->GetAlarmType() == ALARM_STATE_ALARM)
                {
                  mPumpAlarmPresent[num] = true;
                }
              }
            }
            break;
          case ERRONEOUS_UNIT_DOSING_PUMP :
            if (ConvertDosingPumpAlarmToGeniStatus(alarm_id, word_idx, bit) == true && word_idx < NO_OF_GENI_STATUS_WORDS)
            {
              // although it is dosing pump unit type, but also belong to pit alarm 4
              if (p_alarm_event->GetAlarmType() == ALARM_STATE_ALARM)
              {
                if ( (mpScadaEnabled->GetValue() == true)
                  && (p_alarm_event->GetErrorSource()->GetAlarmConfig()->GetScadaEnabled() == true)
                  && (p_alarm_event->GetArrivalTime()->GetSecondsSince1Jan1970() > mpTimeStampLastScadaAck->GetValue()) )
                {
                  mSystemAlarmScadaPending[word_idx] |= (0x1<<bit);
                }
                mSystemAlarmStatus[word_idx] |= (0x1<<bit);

                //Handle mixer alarms
                if (p_alarm_event->GetErroneousUnit() == ERRONEOUS_UNIT_MIXER )
                {
                  SET_BIT_HIGH(mPumpFault, 12);
                }
              }
              else if (p_alarm_event->GetAlarmType() == ALARM_STATE_WARNING)
              {
                if ( (mpScadaEnabled->GetValue() == true)
                  && (p_alarm_event->GetErrorSource()->GetAlarmConfig()->GetScadaEnabled() == true)
                  && (p_alarm_event->GetArrivalTime()->GetSecondsSince1Jan1970() > mpTimeStampLastScadaAck->GetValue()) )
                {
                  mSystemWarningScadaPending[word_idx] |= (0x1<<bit);
                }
                mSystemWarningStatus[word_idx] |= (0x1<<bit);
              }
            }
            break;
          default: // Handle as system alarms/warnings
            if (ConvertSystemAlarmToGeniStatus(alarm_id, word_idx, bit) == true && word_idx < NO_OF_GENI_STATUS_WORDS)
            {
              // bit value found for the alarm or warning, set the bit in the related status value
              if (alarm_id == ALARM_ID_COMBI_ALARM) // Special handling for combi alarm 1-4. (err unit = 1-4)
              {
                num = p_alarm_event->GetErroneousUnitNumber() - 1;
                if (num < 4)
                {
                  bit += num;
                }
              }
              else if (alarm_id == ALARM_ID_USER_DEFINED_SENSOR_FAULT) // Special handling for sensor fault 1-3. (err unit = 1-3)
              {
                num = p_alarm_event->GetErroneousUnitNumber() - 1;
                if (num < 3)
                {
                  bit += num;
                }
              }

              if (p_alarm_event->GetAlarmType() == ALARM_STATE_ALARM)
              {
                if ( (mpScadaEnabled->GetValue() == true)
                  && (p_alarm_event->GetErrorSource()->GetAlarmConfig()->GetScadaEnabled() == true)
                  && (p_alarm_event->GetArrivalTime()->GetSecondsSince1Jan1970() > mpTimeStampLastScadaAck->GetValue()) )
                {
                  mSystemAlarmScadaPending[word_idx] |= (0x1<<bit);
                }
                mSystemAlarmStatus[word_idx] |= (0x1<<bit);

                //Handle mixer alarms
                if (p_alarm_event->GetErroneousUnit() == ERRONEOUS_UNIT_MIXER )
                {
                  SET_BIT_HIGH(mPumpFault, 12);
                }
              }
              else if (p_alarm_event->GetAlarmType() == ALARM_STATE_WARNING)
              {
                if ( (mpScadaEnabled->GetValue() == true)
                  && (p_alarm_event->GetErrorSource()->GetAlarmConfig()->GetScadaEnabled() == true)
                  && (p_alarm_event->GetArrivalTime()->GetSecondsSince1Jan1970() > mpTimeStampLastScadaAck->GetValue()) )
                {
                  mSystemWarningScadaPending[word_idx] |= (0x1<<bit);
                }
                mSystemWarningStatus[word_idx] |= (0x1<<bit);
                //Handle mixer warnings
                if (p_alarm_event->GetErroneousUnit() == ERRONEOUS_UNIT_MIXER )
                {
                  SET_BIT_HIGH(mPumpFault, 13);
                }
              }
              if (alarm_id == ALARM_ID_GENIBUS_IO_MODULE) // IO351 comm fault
              {
                mPumpMonitoringFault |= 0x7F; // Pump 1+2+3+4+5+6 + Mixer fault
              }
            }
            break;
        }
      }
    }

    mpAlarmLog->UnuseAlarmLog();

    if (mpServiceModeEnabled->GetValue() == false)
    {
      // Update alarm status datapoints (normal behaviour)
      mpActualAlarm->SetValue(first_alarm);
      mpActualWarning->SetValue(first_warning);
      mpPumpFault->SetValue(mPumpFault);
      mpPumpMonitoringFault->SetValue(mPumpMonitoringFault);

      if (mpScadaLatchingEnabled->GetAsBool() )
      {
        for (word_idx=0; word_idx<NO_OF_GENI_STATUS_WORDS; word_idx++)
        {
          mpSystemAlarmStatus[word_idx]->SetValue(mSystemAlarmStatus[word_idx] | mSystemAlarmScadaPending[word_idx]);
          mpSystemWarningStatus[word_idx]->SetValue(mSystemWarningStatus[word_idx] | mSystemWarningScadaPending[word_idx]);
          for (num=0; num<NO_OF_PUMPS; num++)
          {
            mpPumpAlarmStatus[word_idx][num]->SetValue(mPumpAlarmStatus[word_idx][num] | mPumpAlarmScadaPending[word_idx][num]);
            mpPumpWarningStatus[word_idx][num]->SetValue(mPumpWarningStatus[word_idx][num] | mPumpWarningScadaPending[word_idx][num]);
          }
        }
      }
      else
      {
        for (word_idx=0; word_idx<NO_OF_GENI_STATUS_WORDS; word_idx++)
        {
          mpSystemAlarmStatus[word_idx]->SetValue(mSystemAlarmStatus[word_idx]);
          mpSystemWarningStatus[word_idx]->SetValue(mSystemWarningStatus[word_idx]);
          for (num=0; num<NO_OF_PUMPS; num++)
          {
            mpPumpAlarmStatus[word_idx][num]->SetValue(mPumpAlarmStatus[word_idx][num]);
            mpPumpWarningStatus[word_idx][num]->SetValue(mPumpWarningStatus[word_idx][num]);
          }
        }
      }
    }
    else
    {
      // Suppress all alarms in service mode
      mpActualAlarm->SetValue(ALARM_ID_NO_ALARM);
      mpActualWarning->SetValue(ALARM_ID_NO_ALARM);
      mpPumpFault->SetValue(0);
      mpPumpMonitoringFault->SetValue(0);

      for (word_idx=0; word_idx<NO_OF_GENI_STATUS_WORDS; word_idx++)
      {
        mpSystemAlarmStatus[word_idx]->SetValue(0);
        mpSystemWarningStatus[word_idx]->SetValue(0);
        for (num=0; num<NO_OF_PUMPS; num++)
        {
          mpPumpAlarmStatus[word_idx][num]->SetValue(0);
          mpPumpWarningStatus[word_idx][num]->SetValue(0);
        }
      }
    }


    // Update the Pump Alarm flags for CombiAlarm and PumpCtrl
    for (int i=0; i < NO_OF_PUMPS; i++)
    {
      mpPumpAlarmFlag[i]->SetValue(mPumpAlarmPresent[i]);
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void AlarmStatusCtrl::ConnectToSubjects()
{
  mpAlarmLog->Subscribe(this);
  mpTimeStampLastScadaAck->Subscribe(this);
  mpScadaEnabled->Subscribe(this);
  mpScadaLatchingEnabled->Subscribe(this);
  mpServiceModeEnabled->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void AlarmStatusCtrl::Update(Subject* pSubject)
{
  mpAlarmLog.Update(pSubject);
  mpTimeStampLastScadaAck.Update(pSubject);
  mpScadaEnabled.Update(pSubject);
  mpScadaLatchingEnabled.Update(pSubject);
  mpServiceModeEnabled.Update(pSubject);

  if (mReqTaskTimeFlag == false)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmStatusCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void AlarmStatusCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_ALST_ALARM_LOG:
      mpAlarmLog.Attach(pSubject);
      break;

    case SP_ALST_ACTUAL_ALARM:
      mpActualAlarm.Attach(pSubject);
      break;
    case SP_ALST_ACTUAL_WARNING:
      mpActualWarning.Attach(pSubject);
      break;

    case SP_ALST_PUMP_FAULT_STATUS:
      mpPumpFault.Attach(pSubject);
      break;
    case SP_ALST_PUMP_MONITORING_FAULT:
      mpPumpMonitoringFault.Attach(pSubject);
      break;

    case SP_ALST_SYSTEM_ALARM_STATUS_1:
      mpSystemAlarmStatus[0].Attach(pSubject);
      break;
    case SP_ALST_SYSTEM_ALARM_STATUS_2:
      mpSystemAlarmStatus[1].Attach(pSubject);
      break;
    case SP_ALST_SYSTEM_ALARM_STATUS_3:
      mpSystemAlarmStatus[2].Attach(pSubject);
      break;
    case SP_ALST_SYSTEM_ALARM_STATUS_4:
      mpSystemAlarmStatus[3].Attach(pSubject);
      break;
    case SP_ALST_SYSTEM_WARNING_STATUS_1:
      mpSystemWarningStatus[0].Attach(pSubject);
      break;
    case SP_ALST_SYSTEM_WARNING_STATUS_2:
      mpSystemWarningStatus[1].Attach(pSubject);
      break;
    case SP_ALST_SYSTEM_WARNING_STATUS_3:
      mpSystemWarningStatus[2].Attach(pSubject);
      break;
    case SP_ALST_SYSTEM_WARNING_STATUS_4:
      mpSystemWarningStatus[3].Attach(pSubject);
      break;

    case SP_ALST_PUMP_1_ALARM_STATUS_1:
      mpPumpAlarmStatus[0][0].Attach(pSubject);
      break;
    case SP_ALST_PUMP_1_ALARM_STATUS_2:
      mpPumpAlarmStatus[1][0].Attach(pSubject);
      break;
    case SP_ALST_PUMP_1_ALARM_STATUS_3:
      mpPumpAlarmStatus[2][0].Attach(pSubject);
      break;
    case SP_ALST_PUMP_1_ALARM_STATUS_4:
      mpPumpAlarmStatus[3][0].Attach(pSubject);
      break;
    case SP_ALST_PUMP_1_WARNING_STATUS_1:
      mpPumpWarningStatus[0][0].Attach(pSubject);
      break;
    case SP_ALST_PUMP_1_WARNING_STATUS_2:
      mpPumpWarningStatus[1][0].Attach(pSubject);
      break;
    case SP_ALST_PUMP_1_WARNING_STATUS_3:
      mpPumpWarningStatus[2][0].Attach(pSubject);
      break;
    case SP_ALST_PUMP_1_WARNING_STATUS_4:
      mpPumpWarningStatus[3][0].Attach(pSubject);
      break;
    case SP_ALST_PUMP_2_ALARM_STATUS_1:
      mpPumpAlarmStatus[0][1].Attach(pSubject);
      break;
    case SP_ALST_PUMP_2_ALARM_STATUS_2:
      mpPumpAlarmStatus[1][1].Attach(pSubject);
      break;
    case SP_ALST_PUMP_2_ALARM_STATUS_3:
      mpPumpAlarmStatus[2][1].Attach(pSubject);
      break;
    case SP_ALST_PUMP_2_ALARM_STATUS_4:
      mpPumpAlarmStatus[3][1].Attach(pSubject);
      break;
    case SP_ALST_PUMP_2_WARNING_STATUS_1:
      mpPumpWarningStatus[0][1].Attach(pSubject);
      break;
    case SP_ALST_PUMP_2_WARNING_STATUS_2:
      mpPumpWarningStatus[1][1].Attach(pSubject);
      break;
    case SP_ALST_PUMP_2_WARNING_STATUS_3:
      mpPumpWarningStatus[2][1].Attach(pSubject);
      break;
    case SP_ALST_PUMP_2_WARNING_STATUS_4:
      mpPumpWarningStatus[3][1].Attach(pSubject);
      break;

    case SP_ALST_PUMP_3_ALARM_STATUS_1:
      mpPumpAlarmStatus[0][2].Attach(pSubject);
      break;
    case SP_ALST_PUMP_3_ALARM_STATUS_2:
      mpPumpAlarmStatus[1][2].Attach(pSubject);
      break;
    case SP_ALST_PUMP_3_ALARM_STATUS_3:
      mpPumpAlarmStatus[2][2].Attach(pSubject);
      break;
    case SP_ALST_PUMP_3_ALARM_STATUS_4:
      mpPumpAlarmStatus[3][2].Attach(pSubject);
      break;
    case SP_ALST_PUMP_3_WARNING_STATUS_1:
      mpPumpWarningStatus[0][2].Attach(pSubject);
      break;
    case SP_ALST_PUMP_3_WARNING_STATUS_2:
      mpPumpWarningStatus[1][2].Attach(pSubject);
      break;
    case SP_ALST_PUMP_3_WARNING_STATUS_3:
      mpPumpWarningStatus[2][2].Attach(pSubject);
      break;
    case SP_ALST_PUMP_3_WARNING_STATUS_4:
      mpPumpWarningStatus[3][2].Attach(pSubject);
      break;

    case SP_ALST_PUMP_4_ALARM_STATUS_1:
      mpPumpAlarmStatus[0][3].Attach(pSubject);
      break;
    case SP_ALST_PUMP_4_ALARM_STATUS_2:
      mpPumpAlarmStatus[1][3].Attach(pSubject);
      break;
    case SP_ALST_PUMP_4_ALARM_STATUS_3:
      mpPumpAlarmStatus[2][3].Attach(pSubject);
      break;
    case SP_ALST_PUMP_4_ALARM_STATUS_4:
      mpPumpAlarmStatus[3][3].Attach(pSubject);
      break;
    case SP_ALST_PUMP_4_WARNING_STATUS_1:
      mpPumpWarningStatus[0][3].Attach(pSubject);
      break;
    case SP_ALST_PUMP_4_WARNING_STATUS_2:
      mpPumpWarningStatus[1][3].Attach(pSubject);
      break;
    case SP_ALST_PUMP_4_WARNING_STATUS_3:
      mpPumpWarningStatus[2][3].Attach(pSubject);
      break;
    case SP_ALST_PUMP_4_WARNING_STATUS_4:
      mpPumpWarningStatus[3][3].Attach(pSubject);
      break;

    case SP_ALST_PUMP_5_ALARM_STATUS_1:
      mpPumpAlarmStatus[0][4].Attach(pSubject);
      break;
    case SP_ALST_PUMP_5_ALARM_STATUS_2:
      mpPumpAlarmStatus[1][4].Attach(pSubject);
      break;
    case SP_ALST_PUMP_5_ALARM_STATUS_3:
      mpPumpAlarmStatus[2][4].Attach(pSubject);
      break;
    case SP_ALST_PUMP_5_ALARM_STATUS_4:
      mpPumpAlarmStatus[3][4].Attach(pSubject);
      break;
    case SP_ALST_PUMP_5_WARNING_STATUS_1:
      mpPumpWarningStatus[0][4].Attach(pSubject);
      break;
    case SP_ALST_PUMP_5_WARNING_STATUS_2:
      mpPumpWarningStatus[1][4].Attach(pSubject);
      break;
    case SP_ALST_PUMP_5_WARNING_STATUS_3:
      mpPumpWarningStatus[2][4].Attach(pSubject);
      break;
    case SP_ALST_PUMP_5_WARNING_STATUS_4:
      mpPumpWarningStatus[3][4].Attach(pSubject);
      break;

    case SP_ALST_PUMP_6_ALARM_STATUS_1:
      mpPumpAlarmStatus[0][5].Attach(pSubject);
      break;
    case SP_ALST_PUMP_6_ALARM_STATUS_2:
      mpPumpAlarmStatus[1][5].Attach(pSubject);
      break;
    case SP_ALST_PUMP_6_ALARM_STATUS_3:
      mpPumpAlarmStatus[2][5].Attach(pSubject);
      break;
    case SP_ALST_PUMP_6_ALARM_STATUS_4:
      mpPumpAlarmStatus[3][5].Attach(pSubject);
      break;
    case SP_ALST_PUMP_6_WARNING_STATUS_1:
      mpPumpWarningStatus[0][5].Attach(pSubject);
      break;
    case SP_ALST_PUMP_6_WARNING_STATUS_2:
      mpPumpWarningStatus[1][5].Attach(pSubject);
      break;
    case SP_ALST_PUMP_6_WARNING_STATUS_3:
      mpPumpWarningStatus[2][5].Attach(pSubject);
      break;
    case SP_ALST_PUMP_6_WARNING_STATUS_4:
      mpPumpWarningStatus[3][5].Attach(pSubject);
      break;

    case SP_ALST_PUMP_1_ALARM_FLAG:
      mpPumpAlarmFlag[0].Attach(pSubject);
      break;
    case SP_ALST_PUMP_2_ALARM_FLAG:
      mpPumpAlarmFlag[1].Attach(pSubject);
      break;
    case SP_ALST_PUMP_3_ALARM_FLAG:
      mpPumpAlarmFlag[2].Attach(pSubject);
      break;
    case SP_ALST_PUMP_4_ALARM_FLAG:
      mpPumpAlarmFlag[3].Attach(pSubject);
      break;
    case SP_ALST_PUMP_5_ALARM_FLAG:
      mpPumpAlarmFlag[4].Attach(pSubject);
      break;
    case SP_ALST_PUMP_6_ALARM_FLAG:
      mpPumpAlarmFlag[5].Attach(pSubject);
      break;

    case SP_ALST_SCADA_ENABLED:
      mpScadaEnabled.Attach(pSubject);
      break;
    case SP_ALST_TIMESTAMP_LAST_SCADA_ACK:
      mpTimeStampLastScadaAck.Attach(pSubject);
      break;
    case SP_ALST_SCADA_LATCHING_ENABLED:
      mpScadaLatchingEnabled.Attach(pSubject);
      break;
    case SP_ALST_SERVICE_MODE_ENABLED:
      mpServiceModeEnabled.Attach(pSubject);
      break;

    default:
      break;
  }
}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - ConvertSystemAlarmToGeniStatus
 * DESCRIPTION: Look up the Geni error word+bit for the given system alarm
 *              (See the event status table in the Geni profile)
 *
 *****************************************************************************/
bool AlarmStatusCtrl::ConvertSystemAlarmToGeniStatus(ALARM_ID_TYPE alarm_id, U16 &word_no, U16 &bit_no)
{
  bool found = true;

  switch (alarm_id)
  {
    // Alarm status 1:
    case ALARM_ID_OVERFLOW:                                 word_no = 0;  bit_no = 0;   break;  // 192
    case ALARM_ID_HIGH_LEVEL:                               word_no = 0;  bit_no = 1;   break;  // 191
    case ALARM_ID_ALARM_LEVEL:                              word_no = 0;  bit_no = 2;   break;  // 190
    case ALARM_ID_DRY_RUNNING:                              word_no = 0;  bit_no = 3;   break;  // 57
    case ALARM_ID_MAINS_FAULT:                              word_no = 0;  bit_no = 4;   break;  // 6
    case ALARM_ID_LEVEL_FLOAT_SWITCH_INCONSISTENCY:         word_no = 0;  bit_no = 5;   break;  // 205
    case ALARM_ID_CONFLICTING_LEVELS:                       word_no = 0;  bit_no = 6;   break;  // 204
    case ALARM_ID_LEVEL_SENSOR_SIGNAL_FAULT:                word_no = 0;  bit_no = 7;   break;  // 189
    case ALARM_ID_FLOW_SENSOR_SIGNAL_FAULT:                 word_no = 0;  bit_no = 8;   break;  // 169    
    case ALARM_ID_POWER_METER:                              word_no = 0;  bit_no = 9;   break;  // 186
    case ALARM_ID_MIXER_CONTACTOR:                          word_no = 0;  bit_no = 10;  break;  // 221
    case ALARM_ID_GENIBUS_IO_MODULE:                        word_no = 0;  bit_no = 11;  break;  // 226
    case ALARM_ID_CIU_CARD_FAULT:                           word_no = 0;  bit_no = 12;  break;  // 159
    case ALARM_ID_SIM_CARD_FAULT:                           word_no = 0;  bit_no = 13;  break;  // 160
    case ALARM_ID_MAIN_SYSTEM_COMMUNICATION_FAULT:          word_no = 0;  bit_no = 14;  break;  // 15
    case ALARM_ID_POWER_ON_OCCURED_WARNING:                 word_no = 0;  bit_no = 15;  break;  // 247

    // Alarm status 2:
    case ALARM_ID_UNINTERRUPTABLE_POWER_SUPPLY:             word_no = 1;  bit_no = 0;   break;  // 248
    case ALARM_ID_HARDWARE_FAULT_TYPE_1:                    word_no = 1;  bit_no = 1;   break;  // 72
    case ALARM_ID_ETHERNET_NO_IP_ADDRESS_FROM_DHCP_SERVER:  word_no = 1;  bit_no = 2;   break;  // 231
    case ALARM_ID_ETHERNET_AUTO_DISABLED_DUE_TO_MISUSE:     word_no = 1;  bit_no = 3;   break;  // 232
    case ALARM_ID_MIXER_SERVICE_TIME:                       word_no = 1;  bit_no = 4;   break;  // 222
    case ALARM_ID_MIXER_STARTS_PR_HOUR:                     word_no = 1;  bit_no = 5;   break;  // 223
    case ALARM_ID_CUSTOM_RELAY_ACTIVATED:                   word_no = 1;  bit_no = 6;   break;  // 246
    /*No bit 7 in profile*/
    case ALARM_ID_EXTERNAL_FAULT_SIGNAL:                    word_no = 1;  bit_no = 8;   break;  // 3
    case ALARM_ID_COMBI_ALARM:                              word_no = 1;  bit_no = 9;   break;  // 227, (special, return first bit)
    case ALARM_ID_USER_DEFINED_SENSOR_FAULT:                word_no = 1;  bit_no = 13;  break;  // 188, (special, return first bit)

    // Alarm status 3:
    case ALARM_ID_PRESSURE_SENSOR_SIGNAL_FAULT:             word_no = 2; bit_no = 0; break; // 168
    case ALARM_ID_WATER_ON_FLOOR:                           word_no = 2; bit_no = 1; break; // 229
    case ALARM_ID_GAS_DETECTOR:                             word_no = 2; bit_no = 2; break; // 235
    case ALARM_ID_EXTRA_FAULT_1:                            word_no = 2; bit_no = 3; break; // 249
    case ALARM_ID_EXTRA_FAULT_2:                            word_no = 2; bit_no = 4; break; // 250
    case ALARM_ID_EXTRA_FAULT_3:                            word_no = 2; bit_no = 5; break; // 251
    case ALARM_ID_EXTRA_FAULT_4:                            word_no = 2; bit_no = 6; break; // 252
    case ALARM_ID_H2S_SENSOR_FAULT:                         word_no = 2; bit_no = 7; break; // 118
    case ALARM_ID_SCADA_CALLBACK_TEST:                      word_no = 2; bit_no = 8; break; // 253

    default:
      found = false;
      break;  // No bit to set (should not happen)
  }

  return found;
}

/*****************************************************************************
 * Function - ConvertPumpAlarmToGeniStatus
 * DESCRIPTION: Look up the Geni error bit for the given pump alarm
 *              (See the event status table in the Geni profile)
 *
 *****************************************************************************/
bool AlarmStatusCtrl::ConvertPumpAlarmToGeniStatus(ALARM_ID_TYPE alarm_id, U16 &word_no, U16 &bit_no)
{
  bool found = true;

  switch (alarm_id)
  {
    // Alarm status 1:
    case ALARM_ID_TERMO_RELAY_1_IN_MOTOR:                   word_no = 0;  bit_no = 0;   break;  // 69
    case ALARM_ID_TERMO_RELAY_2_IN_MOTOR:                   word_no = 0;  bit_no = 1;   break;  // 70
    case ALARM_ID_OVERTEMPERATURE:                          word_no = 0;  bit_no = 2;   break;  // 64
    case 71: /*ALARM_ID_MOTOR_TEMPERATURE_2:*/              word_no = 0;  bit_no = 3;   break;  // 71
    case ALARM_ID_UPPER_BEARING_TEMPERATURE:                word_no = 0;  bit_no = 4;   break;  // 145
    case ALARM_ID_LOWER_BEARING_TEMPERATURE:                word_no = 0;  bit_no = 5;   break;  // 146
    case ALARM_ID_INSULATION_RESISTANCE_LOW:                word_no = 0;  bit_no = 6;   break;  // 20
    case 40: /*ALARM_ID_UNDERVOLTAGE:*/                     word_no = 0;  bit_no = 7;   break;  // 40
    case 32: /*ALARM_ID_OVERVOLTAGE:*/                      word_no = 0;  bit_no = 8;   break;  // 32
    case 9:  /*ALARM_ID_PHASE_SEQUENCE_REVERSAL:*/          word_no = 0;  bit_no = 9;   break;  // 9
    case ALARM_ID_OVERLOAD:                                 word_no = 0;  bit_no = 10;  break;  // 48
    case ALARM_ID_UNDERLOAD:                                word_no = 0;  bit_no = 11;  break;  // 56
    case ALARM_ID_MOTOR_PROTECTION_TRIPPED:                 word_no = 0;  bit_no = 12;  break;  // 27
    case 2:  /*ALARM_ID_MISSING_PHASE:*/                    word_no = 0;  bit_no = 13;  break;  // 2
    case 111:/*ALARM_ID_CURRENT_ASYMMETRY:*/                word_no = 0;  bit_no = 14;  break;  // 111
    case 26: /*Load continue:*/                             word_no = 0;  bit_no = 15;  break;  // 26

    // Alarm status 2:
    case 18: /*??? motor protection*/                       word_no = 1;  bit_no = 0;   break;  // 18
    case ALARM_ID_COMMON_PHASE:                             word_no = 1;  bit_no = 1;   break;  // 241
    case ALARM_ID_HUMIDITY_SWITCH_ALARM:                    word_no = 1;  bit_no = 2;   break;  // 22
    case ALARM_ID_VIBRATION:                                word_no = 1;  bit_no = 3;   break;  // 24
    case ALARM_ID_WATER_IN_MOTOR_OIL_FAULT:                 word_no = 1;  bit_no = 4;   break;  // 11
    case ALARM_ID_MAINS_FAULT:                              word_no = 1;  bit_no = 5;   break;  // 6
    case ALARM_ID_CONTACTOR:                                word_no = 1;  bit_no = 6;   break;  // 220
    case ALARM_ID_TOO_MANY_STARTS_PER_H:                    word_no = 1;  bit_no = 7;   break;  // 21
    case ALARM_ID_TIME_FOR_SERVICE:                         word_no = 1;  bit_no = 8;   break;  // 12
    case 4:  /*ALARM_ID_TOO_MANY_RESTARTS*/                 word_no = 1;  bit_no = 9;   break;  // 4
    case ALARM_ID_LOW_FLOW:                                 word_no = 1;  bit_no = 10;  break;  // 58
    case ALARM_ID_LATEST_RUNTIME:                           word_no = 1;  bit_no = 11;  break;  // 245
    case 112: /*Motor cos-phi too high*/                    word_no = 1;  bit_no = 12;  break;  // 112
    case 113: /*Motor cos-phi too low*/                     word_no = 1;  bit_no = 13;  break;  // 113

    // Alarm status 3:
    case 224:/*Pump malfunction:*/                          word_no = 2;  bit_no = 0;   break;  // 224
    case ALARM_ID_GENIBUS_PUMP_MODULE:                      word_no = 2;  bit_no = 1;   break;  // 225
    case ALARM_ID_PUMP_MANUAL:                              word_no = 2;  bit_no = 2;   break;  // 243
    case 175: /*ALARM_ID_TEMPERATURE_2_SENSOR_SIGNAL_FLT:*/ word_no = 2;  bit_no = 3;   break;  // 175
    case ALARM_ID_TERMO_RELAY_1_IN_MOTOR_SIGNAL_FAULT:      word_no = 2;  bit_no = 4;   break;  // 181
    case ALARM_ID_WATER_IN_GLYCOL_SENSOR_SIGNAL_FAULT:      word_no = 2;  bit_no = 5;   break;  // 170
    case ALARM_ID_UPPER_BEARING_TEMPERATURE_SIGNAL_FAULT:   word_no = 2;  bit_no = 6;   break;  // 179
    case ALARM_ID_LOWER_BEARING_TEMPERATURE_SIGNAL_FAULT:   word_no = 2;  bit_no = 7;   break;  // 180
    case ALARM_ID_COMMUNICATION_FAULT:                      word_no = 2;  bit_no = 8;   break;  // 10
    case ALARM_ID_SETUP_CONFLICT:                           word_no = 2;  bit_no = 9;   break;  // 25
    case ALARM_ID_HARDWARE_FAULT_TYPE_1:                    word_no = 2;  bit_no = 10;  break;  // 72
    case ALARM_ID_SENSOR_FAULT:                             word_no = 2;  bit_no = 11;  break;  // 88
    case 155:                                               word_no = 2;  bit_no = 12;  break;  // 155
    case 93:                                                word_no = 2;  bit_no = 13;  break;  // 93
    case 148:                                               word_no = 2;  bit_no = 14;  break;  // 148
    case 149:                                               word_no = 2;  bit_no = 15;  break;  // 149

    // Alarm status 4:
    case 1:                                                 word_no = 3;  bit_no = 0;   break;  //  1
    case 176:                                               word_no = 3;  bit_no = 1;   break;  //  176
    case 89:                                                word_no = 3;  bit_no = 2;   break;  //  89
    case 49:                                                word_no = 3;  bit_no = 3;   break;  //  49
    case 55:                                                word_no = 3;  bit_no = 4;   break;  //  55
    case 30:                                                word_no = 3;  bit_no = 5;   break;  //  30
    case 240:                                               word_no = 3;  bit_no = 6;   break;  //  240
    case 242:                                               word_no = 3;  bit_no = 7;   break;  //  242
    case 77:                                                word_no = 3;  bit_no = 8;   break;  //  77
    case 91:                                                word_no = 3;  bit_no = 9;   break;  //  91
    case 57:                                                word_no = 3;  bit_no = 10;  break;  //  57
    case 213:                                               word_no = 3;  bit_no = 11;  break;  //  213
    case 16:                                                word_no = 3;  bit_no = 12;  break;  //  16
    case ALARM_ID_PUMP_MOTOR_BLOCKED:                       word_no = 3;  bit_no = 13;  break;  //  51
    case ALARM_ID_POWER_METER:                              word_no = 3;  bit_no = 14;  break;  //  186

    default:
      found = false;
      break;  // No bit to set (should not happen)
  }

  return found;
}

/*****************************************************************************
 * Function - ConvertDosingPumpAlarmToGeniStatus
 * DESCRIPTION: Look up the Geni error word+bit for the given system alarm
 *              (See the event status table in the Geni profile)
 *
 *****************************************************************************/
bool AlarmStatusCtrl::ConvertDosingPumpAlarmToGeniStatus(ALARM_ID_TYPE alarm_id, U16 &word_no, U16 &bit_no)
{
  bool found = true;

  switch (alarm_id)
  {
    // Alarm status 4:
    case ALARM_ID_OVER_PRESSURE:                            word_no = 3; bit_no = 0; break; // 210
    case ALARM_ID_MEAN_PRESSURE_TO_LOW:                     word_no = 3; bit_no = 1; break; // 211
    case ALARM_ID_GAS_IN_PUMP_HEAD:                         word_no = 3; bit_no = 2; break; // 35
    case ALARM_ID_CAVITATIONS:                              word_no = 3; bit_no = 3; break; // 208
    case ALARM_ID_PRESSURE_VALVE_LEAKAGE:                   word_no = 3; bit_no = 4; break; // 36
    case ALARM_ID_SUCTION_VALVE_LEAKAGE:                    word_no = 3; bit_no = 5; break; // 37
    case ALARM_ID_VENTING_VALVE_DEFECT:                     word_no = 3; bit_no = 6; break; // 38
    case ALARM_ID_TIME_FOR_SERVICE:                         word_no = 3; bit_no = 7; break; // TODO 12
    case ALARM_ID_SOON_TIME_FOR_SERVICE:                    word_no = 3; bit_no = 8; break; // 33
    case ALARM_ID_CAPACITY_TOO_LOW:                         word_no = 3; bit_no = 9; break; // 17
    case ALARM_ID_DIAPHRAGM_BREAK:                          word_no = 3; bit_no = 10; break; // 19
    case ALARM_ID_PUMP_MOTOR_BLOCKED:                       word_no = 3; bit_no = 11; break; // TODO 51
    case ALARM_ID_PRE_EMPTY_TANK:                           word_no = 3; bit_no = 12; break; // 206
    case ALARM_ID_DRY_RUNNING:                              word_no = 3; bit_no = 13; break; // TODO 57
    case ALARM_ID_FLOW_SENSOR_SIGNAL_FAULT:                 word_no = 3; bit_no = 14; break; // TODO 169
    case ALARM_ID_CABLE_BREAKDOWN_ON_ANALOGUE:              word_no = 3; bit_no = 15; break; // 47

    default:
      found = false;
      break;  // No bit to set (should not happen)
  }

  return found;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
