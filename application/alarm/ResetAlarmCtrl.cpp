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
/* CLASS NAME       : ResetAlarmCtrl                                        */
/*                                                                          */
/* FILE NAME        : ResetAlarmCtrl.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 25-05-2005 dd-mm-yyyy                                 */
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
#include <ResetAlarmCtrl.h>

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
 * DESCRIPTION: Initialises private data and reads configuration.
 *
 *****************************************************************************/
ResetAlarmCtrl::ResetAlarmCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: Unsubscribes the observed AlarmDataPoint's
 *
 ****************************************************************************/
ResetAlarmCtrl::~ResetAlarmCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION: Requests first run to initialize system mode and operation mode
 * alarm after power up.
 *
 *****************************************************************************/
void ResetAlarmCtrl::InitSubTask()
{
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ResetAlarmCtrl::RunSubTask()
{
  if (mpDiFuncStateAlarmReset.IsUpdated())
  {
    mpAlarmResetEvent->SetEvent();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Only subscribe to DiFuncAlarmReset as this is the only subject
 *              wherefrom an update is interesting.
 *
 *****************************************************************************/
void ResetAlarmCtrl::ConnectToSubjects()
{
  mpDiFuncStateAlarmReset->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void ResetAlarmCtrl::Update(Subject* pSubject)
{
  if (mpDiFuncStateAlarmReset.Update(pSubject))
  {
    if (mpDiFuncStateAlarmReset->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
    {
      ReqTaskTime();
    }
    else
    {
      mpDiFuncStateAlarmReset.ResetUpdated();
    }
  }
}

/*****************************************************************************
* Function - SubscribtionCancelled
* DESCRIPTION: If pSubject is a pointer of ResetAlarmContol then set to 0
*
*****************************************************************************/
void ResetAlarmCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpDiFuncStateAlarmReset.Detach(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed by ResetAlarmCtrl.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void ResetAlarmCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
    case SP_RAC_DIG_IN_FUNC_STATE_ALARM_RESET:
      mpDiFuncStateAlarmReset.Attach(pSubject);
      break;
    case SP_RAC_ALARM_RESET_EVENT:
      mpAlarmResetEvent.Attach(pSubject);
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
