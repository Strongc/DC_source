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
/* CLASS NAME       : SetEventDatapointAction                               */
/*                                                                          */
/* FILE NAME        : SetEventDatapointAction.cpp                           */
/*                                                                          */
/* CREATED DATE     : 2007-10-29                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a SetEventDatapointAction.     */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "SetEventDatapointAction.h"

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
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    SetEventDatapointAction::SetEventDatapointAction(Component* pParent) : Component(pParent)
    {
      SetAllowFlags(mLegalKeys, MPC_OK_KEY);
      SetAllowFlags(mLedsStatus, OK_LED);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    SetEventDatapointAction::~SetEventDatapointAction()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool SetEventDatapointAction::Redraw()
    {
      
      return Component::Redraw();
    }


    bool SetEventDatapointAction::HandleKeyEvent(Keys KeyID)
    {
      switch(KeyID)
      {
        case MPC_OK_KEY:
          if( mEventDP.IsValid() )
            mEventDP->SetEvent();
          break;
      }
      return Component::HandleKeyEvent(KeyID);;
    }


    void SetEventDatapointAction::Update(Subject* Object)
    {
      //ignore
    }

    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void SetEventDatapointAction::SubscribtionCancelled(Subject* pSubject)
    {
      //ignore
    }
    
    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void SetEventDatapointAction::SetSubjectPointer(int Id, Subject* pSubject)
    {
      mEventDP.Attach(pSubject);
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void SetEventDatapointAction::ConnectToSubjects(void)
    {
      mEventDP.Subscribe(this);
    }

  } // namespace display
} // namespace mpc
