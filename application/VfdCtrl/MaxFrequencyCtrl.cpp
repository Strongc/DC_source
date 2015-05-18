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
/* CLASS NAME       : MaxFrequencyCtrl   ´                                  */
/*                                                                          */
/* FILE NAME        : MaxFrequencyCtrl.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 28-10-2009  (dd-mm-yyyy)                              */
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
#include <MaxFrequencyCtrl.h>

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
MaxFrequencyCtrl::MaxFrequencyCtrl(void)
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
MaxFrequencyCtrl::~MaxFrequencyCtrl(void)
{
  mpMainsFrequencyIs60Hz->Unsubscribe(this);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void MaxFrequencyCtrl::InitSubTask(void)
{
  mpMainsFrequencyIs60Hz.SetUpdated();
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void MaxFrequencyCtrl::RunSubTask(void)
{
  float max = (mpMainsFrequencyIs60Hz->GetValue() ? 60.0f : 50.0f);

  // Handle floats
  std::vector<FloatDataPoint*>::iterator float_iter;
  for (float_iter = mFloatDestinationList.begin(); float_iter != mFloatDestinationList.end(); float_iter++)
  {
    (*float_iter)->SetMaxValue(max);
  }

}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void MaxFrequencyCtrl::Update(Subject* pSubject)
{
  ReqTaskTime();
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void MaxFrequencyCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void MaxFrequencyCtrl::ConnectToSubjects(void)
{
  mpMainsFrequencyIs60Hz->Subscribe(this);
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void MaxFrequencyCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_MFC_MAINS_FREQUENCY_IS_60_HZ:
      mpMainsFrequencyIs60Hz.Attach(pSubject);
      break;
    case SP_MFC_DESTINATION:
    {
      FloatDataPoint* p_float = dynamic_cast<FloatDataPoint*>(pSubject);
      if (p_float)
      {
        p_float->IncRefCount();
        mFloatDestinationList.push_back(p_float);
      }
      break;
    }
    default:
      FatalErrorOccured("MaxFrequencyCtrl: unsupported DP type");
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

