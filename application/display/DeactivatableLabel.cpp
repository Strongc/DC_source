/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/* CLASS NAME       : DeactivatableLabel                                    */
/*                                                                          */
/* FILE NAME        : DeactivatableLabel.cpp                                */
/*                                                                          */
/* CREATED DATE     : 2009-05-26                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a DeactivatableLabel.          */
/*                                                                          */
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
#include "DeactivatableLabel.h"
#include <DataPoint.h>

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
    DeactivatableLabel::DeactivatableLabel(Component* pParent) : Label(pParent)
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    DeactivatableLabel::~DeactivatableLabel()
    {
    }
    
    /*****************************************************************************
    * Function - Redraw
    * DESCRIPTION:
    *****************************************************************************/
    bool DeactivatableLabel::Redraw()
    {
      if (mpDpActivated->GetAsInt() == 0)
      {
        Label::SetColour(GUI_COLOUR_TEXT_INACTIVE_FOREGROUND);
      }
      else
      {
        Label::SetColour(GUI_COLOUR_TEXT_DEFAULT_FOREGROUND);
      }
      return Label::Redraw();
    }

    /*****************************************************************************
    * Function - 
    * DESCRIPTION:
    *****************************************************************************/
    Leds DeactivatableLabel::GetLedsStatus()
    {
      if (mpDpActivated->GetAsInt() == 0)
      {
        SetDenyFlags(mLedsStatus, OK_LED);
      }
      else
      {
        RemoveDenyFlags(mLedsStatus, OK_LED);
      }

      return Label::GetLedsStatus();
    }
    
    /*****************************************************************************
    * Function - 
    * DESCRIPTION:
    *****************************************************************************/
    void DeactivatableLabel::SubscribtionCancelled(Subject* pSubject)
    {
      mpDpActivated.Detach(pSubject);
      Label::SubscribtionCancelled(pSubject);
    }

    /*****************************************************************************
    * Function - 
    * DESCRIPTION:
    *****************************************************************************/
    void DeactivatableLabel::Update(Subject* pSubject)
    {
      if (mpDpActivated.Update(pSubject))
      {
        Invalidate();
      }
      
      Label::Update(pSubject);
    }

    /*****************************************************************************
    * Function - 
    * DESCRIPTION:
    *****************************************************************************/
    void DeactivatableLabel::SetSubjectPointer(int id, Subject* pSubject)
    {
      mpDpActivated.Attach(pSubject);
    }

    /*****************************************************************************
    * Function - 
    * DESCRIPTION:
    *****************************************************************************/
    void DeactivatableLabel::ConnectToSubjects(void)
    {
      mpDpActivated->Subscribe(this);
      
      Label::ConnectToSubjects();
    }

  } // namespace display
} // namespace mpc


