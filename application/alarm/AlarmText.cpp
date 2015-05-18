/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : AlarmText                                             */
/*                                                                          */
/* FILE NAME        : AlarmText.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 25-11-2010 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Factory.h>
#include <Languages.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AlarmText.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

 AlarmText* AlarmText::mInstance = 0;
/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

 AlarmText* AlarmText::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new AlarmText();
  }
  return mInstance;
}

/*****************************************************************************
 * Function - GetStringId
 * DESCRIPTION: get string id of an alarm
 *
 *****************************************************************************/
STRING_ID AlarmText::GetStringId(ALARM_ID_TYPE alarmId)
{
  for (int i = 0; i < DISPLAY_ALARM_STRINGS_CNT; ++i)
  {
    if (DISPLAY_ALARM_STRINGS[i].AlarmId == alarmId)
    {
      return DISPLAY_ALARM_STRINGS[i].StringId;
    }
  }
  return SID_ALARM_UNKNOWN;
}

/*****************************************************************************
 * Function - GetString
 * DESCRIPTION: Get alarm string
 *
 *****************************************************************************/
const char* AlarmText::GetString(ALARM_ID_TYPE alarmId, int erroneousUnitNumber)
{
  STRING_ID stringId = SID_ALARM_UNKNOWN;

  if (alarmId == ALARM_ID_COMBI_ALARM)
  {
    //special handling of combi alarms texts
    if (erroneousUnitNumber > 0 && erroneousUnitNumber <= 4)
    {
      return mDpCombiAlarmName[erroneousUnitNumber - 1]->GetValue();
    }
  }
  else if ((alarmId == ALARM_ID_EXTRA_FAULT_1) ||
           (alarmId == ALARM_ID_EXTRA_FAULT_2) ||
           (alarmId == ALARM_ID_EXTRA_FAULT_3) ||
           (alarmId == ALARM_ID_EXTRA_FAULT_4))
  {
    // special handling of extra fault texts
    if (erroneousUnitNumber > 0 && erroneousUnitNumber <= 4)
    {
      return mDpExtraFaultName[erroneousUnitNumber - 1]->GetValue();
    }
  }
  else
  {
    stringId = GetStringId(alarmId);
  }

  return Languages::GetInstance()->GetString(stringId);
}

/*****************************************************************************
 * Function - GetCombiAlarm
 * DESCRIPTION: Get combi alarm string datapoint
 *
 *****************************************************************************/
StringDataPoint* AlarmText::GetCombiAlarm(int erroneousUnitNumber)
{
  if (erroneousUnitNumber >= 1 && erroneousUnitNumber <= 4)
  {
    return mDpCombiAlarmName[erroneousUnitNumber - 1].GetSubject();
  }

  return NULL;
}

/*****************************************************************************
 * Function - GetExtraFault
 * DESCRIPTION: Get extra fault string datapoint
 *
 *****************************************************************************/
StringDataPoint* AlarmText::GetExtraFault(int erroneousUnitNumber)
{
  if (erroneousUnitNumber >= 1 && erroneousUnitNumber <= 4)
  {
    return mDpExtraFaultName[erroneousUnitNumber - 1].GetSubject();
  }

  return NULL;
}


/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmText::ConnectToSubjects()
{
  //ignore
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmText::Update(Subject* pSubject)
{
  //ignore
}

/*****************************************************************************
* Function - SubscribtionCancelled
* DESCRIPTION:
*
*****************************************************************************/
void AlarmText::SubscribtionCancelled(Subject* pSubject)
{
  //ignore
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmText::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_AT_COMBI_ALARM_NAME_1:
      mDpCombiAlarmName[0].Attach(pSubject);
      break;
    case SP_AT_COMBI_ALARM_NAME_2:
      mDpCombiAlarmName[1].Attach(pSubject);
      break;
    case SP_AT_COMBI_ALARM_NAME_3:
      mDpCombiAlarmName[2].Attach(pSubject);
      break;
    case SP_AT_COMBI_ALARM_NAME_4:
      mDpCombiAlarmName[3].Attach(pSubject);
      break;
    case SP_AT_NAME_EXTRA_FAULT_1:
      mDpExtraFaultName[0].Attach(pSubject);
      break;
    case SP_AT_NAME_EXTRA_FAULT_2:
      mDpExtraFaultName[1].Attach(pSubject);
      break;
    case SP_AT_NAME_EXTRA_FAULT_3:
      mDpExtraFaultName[2].Attach(pSubject);
      break;
    case SP_AT_NAME_EXTRA_FAULT_4:
      mDpExtraFaultName[3].Attach(pSubject);
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
 * Function - Constructor
 * DESCRIPTION: 
 *
 *****************************************************************************/
AlarmText::AlarmText()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: 
 *
 ****************************************************************************/
AlarmText::~AlarmText()
{
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
