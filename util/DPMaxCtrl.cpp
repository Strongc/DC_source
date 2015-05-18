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
/* CLASS NAME       : DPMaxCtrl   ´                                         */
/*                                                                          */
/* FILE NAME        : DPMaxCtrl.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 25-09-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <DPMaxCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  C functions declarations
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
 * DESCRIPTION: This is the constructor for the class, to construct
 * an object of the class type
 *****************************************************************************/
DPMaxCtrl::DPMaxCtrl(void)
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
DPMaxCtrl::~DPMaxCtrl(void)
{
  mpSource->Unsubscribe(this);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DPMaxCtrl::InitSubTask(void)
{
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DPMaxCtrl::RunSubTask(void)
{
  float max = mpSource->GetValue();

  // Handle floats
  std::vector<FloatDataPoint*>::iterator float_iter;
  for (float_iter = mFloatDestinationList.begin(); float_iter != mFloatDestinationList.end(); float_iter++)
  {
    (*float_iter)->SetMaxValue(max);
  }

  // Handle alarm configs
  std::vector<AlarmConfig*>::iterator ac_iter;
  for (ac_iter = mAlarmConfigDestinationList.begin(); ac_iter != mAlarmConfigDestinationList.end(); ac_iter++)
  {
    (*ac_iter)->SetMaxLimit(max);
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void DPMaxCtrl::Update(Subject* pSubject)
{
  ReqTaskTime();
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void DPMaxCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void DPMaxCtrl::ConnectToSubjects(void)
{
  mpSource->Subscribe(this);
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void DPMaxCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_DPMC_SOURCE:
      mpSource.Attach(pSubject);
      break;
    case SP_DPMC_DESTINATION:
    {
      FloatDataPoint* p_float = dynamic_cast<FloatDataPoint*>(pSubject);
      if (p_float)
      {
        p_float->IncRefCount();
        mFloatDestinationList.push_back(p_float);
      }
      else
      {
        AlarmConfig* p_alarm_config = dynamic_cast<AlarmConfig*>(pSubject);
        if (p_alarm_config)
        {
          p_alarm_config->IncRefCount();
          mAlarmConfigDestinationList.push_back(p_alarm_config);
        }
        else
        {
          FatalErrorOccured("DPMaxCtrl: unsupported DP type");
        }
      }
      break;
    }
    default:
      FatalErrorOccured("DPMaxCtrl: unsupported DP type");
      break;
  }
}
/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

