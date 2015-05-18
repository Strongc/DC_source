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
/* CLASS NAME       : SetIntDatapointAction                                    */
/*                                                                          */
/* FILE NAME        : SetIntDatapointAction.cpp                                */
/*                                                                          */
/* CREATED DATE     : 2007-0724                                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a SetIntDatapointAction.          */
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
#include "SetIntDatapointAction.h"

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
    SetIntDatapointAction::SetIntDatapointAction(Component* pParent) : Component(pParent)
    {
      mValue = 0;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    SetIntDatapointAction::~SetIntDatapointAction()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool SetIntDatapointAction::Redraw()
    {
      
      return Component::Redraw();
    }


    void SetIntDatapointAction::InitValue(int value)
    {
      mValue = value;

      SetAllowFlags(mLegalKeys, MPC_OK_KEY);
      SetAllowFlags(mLedsStatus, OK_LED);
    }


    bool SetIntDatapointAction::HandleKeyEvent(Keys KeyID)
    {
      switch(KeyID)
      {
        case MPC_OK_KEY:
          if( mIntDP.IsValid() )
            mIntDP->SetAsInt(mValue);
          break;
      }
      return Component::HandleKeyEvent(KeyID);;
    }


    void SetIntDatapointAction::Update(Subject* Object)
    {
      //ignore
    }

    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void SetIntDatapointAction::SubscribtionCancelled(Subject* pSubject)
    {
      //ignore
    }
    
    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void SetIntDatapointAction::SetSubjectPointer(int Id, Subject* pSubject)
    {
      mIntDP.Attach(pSubject);
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void SetIntDatapointAction::ConnectToSubjects(void)
    {
      mIntDP.Subscribe(this);
    }

  } // namespace display
} // namespace mpc
