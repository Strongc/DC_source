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
/* CLASS NAME       : BoolLogic   ´                                         */
/*                                                                          */
/* FILE NAME        : BoolLogic.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 25-05-2009  (dd-mm-yyyy)                              */
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
#include <BoolLogic.h>

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
BoolLogic::BoolLogic(BOOL_LOGIC_OPERATOR_TYPE boolOperator)
{
  mOperator = boolOperator;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
BoolLogic::~BoolLogic(void)
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void BoolLogic::InitSubTask(void)
{
  mReqTaskTimeFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void BoolLogic::RunSubTask(void)
{
  mReqTaskTimeFlag = false;

  bool output = false;

  if (!mSourceList.empty())
  {
    std::vector<BoolDataPoint*>::const_iterator iter = mSourceList.begin();
    std::vector<BoolDataPoint*>::const_iterator end = mSourceList.end();

    output = (*iter)->GetValue();

    while (++iter != end)
    {
      switch (mOperator)
      {
        case BOOL_LOGIC_OPERATOR_AND:
          output &= (*iter)->GetValue();
          break;

        case BOOL_LOGIC_OPERATOR_OR:
          output |= (*iter)->GetValue();
          break;

        case BOOL_LOGIC_OPERATOR_XOR:
          output ^= (*iter)->GetValue();
          break;

        default:
          FatalErrorOccured("BoolLogic: unknown operator");
          break;
      }
    }
  }

  mpOutput->SetValue(output);
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void BoolLogic::Update(Subject* pSubject)
{
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
void BoolLogic::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void BoolLogic::ConnectToSubjects(void)
{
  std::vector<BoolDataPoint*>::iterator iter;

  for (iter = mSourceList.begin(); iter != mSourceList.end(); ++iter)
  {
      (*iter)->Subscribe(this);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void BoolLogic::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_BL_OUTPUT:
      mpOutput.Attach(pSubject);
      break;

    case SP_BL_SOURCE:
    {
      BoolDataPoint* p = dynamic_cast<BoolDataPoint*>(pSubject);
      if (p)
      {
        p->IncRefCount();
        mSourceList.push_back(p);
      }
      else
      {
        FatalErrorOccured("BoolLogic: unsupported DP type");
      }
      break;
    }

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
*****************************************************************************/

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

