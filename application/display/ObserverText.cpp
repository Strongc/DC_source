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
/* CLASS NAME       : ObserverText                                          */
/*                                                                          */
/* FILE NAME        : ObserverText.cpp                                      */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Combines a Text object with an Observer         */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <IDatapoint.h>
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverText.h"

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
    * Constructor
    * DESCRIPTION:
    *****************************************************************************/
    ObserverText::ObserverText(Component* pParent) : Text( pParent )
    {
      mSubjectSubscribed  = false;
      mDPWritable         = true;
    }

    /*****************************************************************************
    * Destructor
    * DESCRIPTION:
    *****************************************************************************/
    ObserverText::~ObserverText()
    {
      mpSubject.UnsubscribeAndDetach(this);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void ObserverText::Update(Subject* pSubject)
    {
      mpSubject.Update(pSubject);

      IDataPoint* p  = dynamic_cast<IDataPoint*>(GetSubject());
      
      if (p != NULL)
      {
        mDPWritable = p->GetWritable();
      }

      Component::Invalidate();
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void ObserverText::SubscribtionCancelled(Subject* pSubject)
    {
      mSubjectSubscribed = false;
      mpSubject.Detach(pSubject);
      Invalidate();
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void ObserverText::SetSubjectPointer(int Id, Subject* pSubject)
    {
      if (mpSubject.IsValid())
      {
        if (mSubjectSubscribed)
          mpSubject.UnsubscribeAndDetach(this);
        else
          mpSubject.Detach();
        mSubjectSubscribed = false;
      }
      mpSubject.Attach(pSubject);
      Component::Invalidate();
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    Subject* ObserverText::GetSubject()
    {
      return mpSubject.GetSubject();
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    void ObserverText::ConnectToSubjects(void)
    {
      mSubjectSubscribed = mpSubject.Subscribe(this);
      Invalidate();
    }

    /*****************************************************************************
    * Function - IsNeverAvailable
    * DESCRIPTION: returns is available as default
    *****************************************************************************/
    bool ObserverText::IsNeverAvailable()
    {
      return false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns whether the object is readonly or not
    *****************************************************************************/
    bool ObserverText::IsReadOnly()
    {
      return !(!Component::IsReadOnly() && mDPWritable);
    }


  } // namespace display
} // namespace mpc
