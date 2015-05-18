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
/* CLASS NAME       : AlarmBellsCtrl                                        */
/*                                                                          */
/* FILE NAME        : AlarmBellsCtrl.cpp                                    */
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
#include <AlarmBEllsCtrl.h>

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
AlarmBellsCtrl::AlarmBellsCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AlarmBellsCtrl::~AlarmBellsCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmBellsCtrl::InitSubTask()
{
    mReqTaskTimeFlag = true;
    ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmBellsCtrl::RunSubTask()
{
  AlarmEvent* p_alarm_event;

  ALARM_STATE_TYPE pump_bell[NO_OF_PUMPS];
  ALARM_STATE_TYPE mixer_bell  = ALARM_STATE_READY;
  ALARM_STATE_TYPE system_bell = ALARM_STATE_READY;

  mReqTaskTimeFlag = false;

  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    pump_bell[i] = ALARM_STATE_READY;
  }

  /* Check if an alarm sms has to be sent */
  if (mpAlarmLog.IsUpdated())
  {
    mpAlarmLog->UseAlarmLog();
    for (int i=0; i<ALARM_LOG_SIZE; i++)
    {
      /* Get event */
      p_alarm_event = mpAlarmLog->GetAlarmLogElement(i);

      if (p_alarm_event->GetAlarmId() != ALARM_ID_NO_ALARM && p_alarm_event->GetAcknowledge() == false) //Check if alarme event is active
      {
        switch (p_alarm_event->GetErroneousUnit())
        {
          case ERRONEOUS_UNIT_MP204 :
          case ERRONEOUS_UNIT_IO111 :
          case ERRONEOUS_UNIT_PUMP :
          case ERRONEOUS_UNIT_CUE :
            {
              // Note: Erroneous number is one-indexed and pump bells are zero-index
              int i = p_alarm_event->GetErroneousUnitNumber() - 1;
              if (i >= FIRST_PUMP && i <= LAST_PUMP)
              {
                UpdateState(p_alarm_event, &pump_bell[i]);
              }
              else
              {
                FatalErrorOccured("AlarmBellCtrl: ill no");
              }
            }
            break;
          case ERRONEOUS_UNIT_MIXER :
            UpdateState(p_alarm_event, &mixer_bell);
            break;
          case ERRONEOUS_UNIT_IO351 :
          case ERRONEOUS_UNIT_CU361 :
          case ERRONEOUS_UNIT_LEVEL_SENSOR :
          case ERRONEOUS_UNIT_FLOW_METER :
          case ERRONEOUS_UNIT_POWER_METER :
          case ERRONEOUS_UNIT_BATTERY :
          case ERRONEOUS_UNIT_CIU :
          case ERRONEOUS_UNIT_ANA_IN :
          case ERRONEOUS_UNIT_SYSTEM :
            UpdateState(p_alarm_event, &system_bell);
            break;
          default :
            UpdateState(p_alarm_event, &system_bell);
            break;
        }
      }
    }
    mpAlarmLog->UnuseAlarmLog();

    /* Update Bell datapoints */
    for (int i = 0; i < NO_OF_PUMPS; i++)
    {
      mpPumpBell[i]->SetValue(pump_bell[i]);
    }
    mpMixerBell->SetValue(mixer_bell);
    mpSystemBell->SetValue(system_bell);
  }
}

/*****************************************************************************
 * Function - UpdateState
 * DESCRIPTION: 
 *****************************************************************************/
void AlarmBellsCtrl::UpdateState(AlarmEvent* pAlarmEvent, ALARM_STATE_TYPE* state)
{
#ifdef USE_TFT_COLOURS
  ALARM_STATE_TYPE previous_state = *state;
  if (previous_state != ALARM_STATE_ALARM)
  {
    *state = pAlarmEvent->GetAlarmType();
  }
#else
  *state = ALARM_STATE_ALARM;
#endif
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void AlarmBellsCtrl::ConnectToSubjects()
{
  mpAlarmLog->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void AlarmBellsCtrl::Update(Subject* pSubject)
{
  mpAlarmLog.Update(pSubject);

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
void AlarmBellsCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void AlarmBellsCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_ABC_ALARM_LOG:
      mpAlarmLog.Attach(pSubject);
      break;
    case SP_ABC_PUMP_1_BELL:
      mpPumpBell[PUMP_1].Attach(pSubject);
      break;
    case SP_ABC_PUMP_2_BELL:
      mpPumpBell[PUMP_2].Attach(pSubject);
      break;
    case SP_ABC_PUMP_3_BELL:
      mpPumpBell[PUMP_3].Attach(pSubject);
      break;
    case SP_ABC_PUMP_4_BELL:
      mpPumpBell[PUMP_4].Attach(pSubject);
      break;
    case SP_ABC_PUMP_5_BELL:
      mpPumpBell[PUMP_5].Attach(pSubject);
      break;
    case SP_ABC_PUMP_6_BELL:
      mpPumpBell[PUMP_6].Attach(pSubject);
      break;
    case SP_ABC_MIXER_BELL:
      mpMixerBell.Attach(pSubject);
      break;
    case SP_ABC_SYSTEM_BELL:
      mpSystemBell.Attach(pSubject);
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
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
