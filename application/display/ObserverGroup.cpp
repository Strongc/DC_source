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
/* CLASS NAME       : Frame                                                 */
/*                                                                          */
/* FILE NAME        : Frame.cpp                                             */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include "WM.h"
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverGroup.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
  /*****************************************************************************
  *
  *
  *              PUBLIC FUNCTIONS
  *
  *
    *****************************************************************************/

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    ObserverGroup::ObserverGroup(Component* pParent) : Group( pParent )
    {
      mpSubject           = 0;
      mSubjectSubscribed  = false;
    }

    ObserverGroup::~ObserverGroup()
    {

    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Update is part of the observer pattern
    *****************************************************************************/
    void ObserverGroup::Update(Subject* Object)
    {
      Invalidate();
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called if subscription shall be canceled
    *****************************************************************************/
    void ObserverGroup::SubscribtionCancelled(Subject* pSubject)
    {
      mSubjectSubscribed = false;
      mpSubject = 0;
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called to set the subject pointer (used by class
    * factory)
    *****************************************************************************/
    void ObserverGroup::SetSubjectPointer(int Id,Subject* pSubject)
    {
      mpSubject = pSubject;
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called to indicate that subscription kan be made
    *****************************************************************************/
    void ObserverGroup::ConnectToSubjects()
    {
      if ((mpSubject) && (!mSubjectSubscribed))
      {
        mpSubject->Subscribe(this);
        mSubjectSubscribed = true;
      }
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns the pointer to the Subject. 0 if no subject is defined
    *****************************************************************************/
    Subject* ObserverGroup::GetSubject()
    {
      return mpSubject;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets whether this element is readable/writeable
    *****************************************************************************/
    bool ObserverGroup::SetReadOnly(bool readOnly /* = true */)
    {
      Component* itt = GetFirstChild();
      while (itt != 0)
      {
        itt->SetReadOnly(readOnly);
        itt = GetNextChild(itt);
      }
      mReadOnly = readOnly;
      return readOnly;
    }

  } // namespace display
} // namespace mpc
