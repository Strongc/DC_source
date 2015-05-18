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
/* CLASS NAME       : AnyAvailableCtrl   ´                                  */
/*                                                                          */
/* FILE NAME        : AnyAvailableCtrl.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 17-06-2009  (dd-mm-yyyy)                              */
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
#include <AnyAvailableCtrl.h>

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
AnyAvailableCtrl::AnyAvailableCtrl(AAC_TASK_TYPE_TYPE taskType)
{
  mTaskType = taskType;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
AnyAvailableCtrl::~AnyAvailableCtrl(void)
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnyAvailableCtrl::InitSubTask(void)
{
  if (mTaskType == AAC_TASK_TYPE_EVENT)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnyAvailableCtrl::RunSubTask(void)
{
  std::vector<DataPoint*>::iterator iter;
  bool output;

  mReqTaskTimeFlag = false;

  output = false;
  for (iter = mSourceList.begin(); iter != mSourceList.end(); iter++)
  {
    if ((*iter)->GetQuality() != DP_NEVER_AVAILABLE)
    {
      output = true;
    }
  }

  mpOutput->SetValue( output );
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void AnyAvailableCtrl::Update(Subject* pSubject)
{
  if (mTaskType == AAC_TASK_TYPE_EVENT && mReqTaskTimeFlag == false)
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
void AnyAvailableCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnyAvailableCtrl::ConnectToSubjects(void)
{
  if (mTaskType == AAC_TASK_TYPE_EVENT)
  {
    std::vector<DataPoint*>::iterator iter;
    for (iter = mSourceList.begin(); iter != mSourceList.end(); iter++)
    {
      (*iter)->Subscribe(this);
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnyAvailableCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_AAC_OUTPUT:
      mpOutput.Attach(pSubject);
      break;
    case SP_AAC_SOURCE:
    {
      DataPoint* p = dynamic_cast<DataPoint*>(pSubject);
      if (p)
      {
        pSubject->IncRefCount();
        mSourceList.push_back(p);
      }
      else
      {
        FatalErrorOccured("AnyAvailableCtrl: unsupported DP type");
      }
      break;
    }
    default:
      FatalErrorOccured("AnyAvailableCtrl: unsupported DP type");
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

