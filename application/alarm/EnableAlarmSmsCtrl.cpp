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
/* CLASS NAME       : EnableAlarmSmsCtrl                                    */
/*                                                                          */
/* FILE NAME        : EnableAlarmSmsCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 25-02-2008 dd-mm-yyyy                                 */
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
#include <EnableAlarmSmsCtrl.h>

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
EnableAlarmSmsCtrl::EnableAlarmSmsCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: Unsubscirbes the observed AlarmDataPoint's
 *
 ****************************************************************************/
EnableAlarmSmsCtrl::~EnableAlarmSmsCtrl()
{
  while (mAlarmConfigs.empty() == false)
  {
    mAlarmConfigs.back()->Unsubscribe(this);
    mAlarmConfigs.pop_back();
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION: Requests first run to initialize system mode and operation mode
 * alarm after power up.
 *
 *****************************************************************************/
void EnableAlarmSmsCtrl::InitSubTask()
{
  mDpEnableScadaForAllAlarmConfigsEvent.ResetUpdated();
  mDpEnableSmsForAllAlarmConfigsEvent.ResetUpdated();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION: 
 *
 *****************************************************************************/
void EnableAlarmSmsCtrl::RunSubTask()
{
  if (mDpEnableScadaForAllAlarmConfigsEvent.IsUpdated())
    SetScadaAlarms( true );

  if (mDpEnableSmsForAllAlarmConfigsEvent.IsUpdated())
    SetSmsAlarms( true );
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void EnableAlarmSmsCtrl::ConnectToSubjects()
{
  mDpEnableScadaForAllAlarmConfigsEvent->Subscribe(this);
  mDpEnableSmsForAllAlarmConfigsEvent->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void EnableAlarmSmsCtrl::Update(Subject* pSubject)
{
  mDpEnableScadaForAllAlarmConfigsEvent.Update(pSubject);
  mDpEnableSmsForAllAlarmConfigsEvent.Update(pSubject);

  ReqTaskTime();
}

/*****************************************************************************
* Function - SubscribtionCancelled
* DESCRIPTION: 
*
*****************************************************************************/
void EnableAlarmSmsCtrl::SubscribtionCancelled(Subject* pSubject)
{
  int vector_size;

  vector_size = mAlarmConfigs.size();
  for ( int i=0; i < vector_size; i++)
  {
    if (mAlarmConfigs.at(i).Detach(pSubject))
    {
      return;
    }
  }

}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed by EnableAlarmSmsCtrl.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void EnableAlarmSmsCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  
  SubjectPtr<AlarmConfig*>* p_element;
  switch(id)
  {
    case SP_EASC_ALARM_CONFIG:
      p_element = new SubjectPtr<AlarmConfig*>;
      p_element->Attach(pSubject);
      mAlarmConfigs.push_back(*p_element);
      delete p_element;
      break;

    case SP_EASC_ENABLE_SCADA_ALARM:
      mDpEnableScadaForAllAlarmConfigsEvent.Attach(pSubject);
      break;
    case SP_EASC_ENABLE_SMS_ALARM:
      mDpEnableSmsForAllAlarmConfigsEvent.Attach(pSubject);
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
 * Function - SetScadaAlarms
 * DESCRIPTION: 
 *
 *****************************************************************************/
void EnableAlarmSmsCtrl::SetScadaAlarms(bool enable)
{
  std::vector< SubjectPtr<AlarmConfig*> >::iterator iter;

  for( iter = mAlarmConfigs.begin(); iter != mAlarmConfigs.end(); iter++ )
  {
    (*iter)->SetScadaEnabled(enable);
  }
}

/*****************************************************************************
 * Function - SetSmsAlarms
 * DESCRIPTION: 
 *
 *****************************************************************************/
void EnableAlarmSmsCtrl::SetSmsAlarms(bool enable)
{
  std::vector< SubjectPtr<AlarmConfig*> >::iterator iter;

  for( iter = mAlarmConfigs.begin(); iter != mAlarmConfigs.end(); iter++ )
  {
    (*iter)->SetSms1Enabled(enable);
    (*iter)->SetSms2Enabled(enable);
    (*iter)->SetSms3Enabled(enable);
  }
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
