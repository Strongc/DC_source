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
/* CLASS NAME       : SetIntDatapointOnFocus                                */
/*                                                                          */
/* FILE NAME        : SetIntDatapointOnFocus.cpp                            */
/*                                                                          */
/* CREATED DATE     : 2008-04-16                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for setting predefined value of IntDatapoint   */
/* when the component gains focus.                                          */
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
#include "SetIntDatapointOnFocus.h"

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
    SetIntDatapointOnFocus::SetIntDatapointOnFocus(Component* pParent) : Component(pParent)
    {
      mValue = 0;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    SetIntDatapointOnFocus::~SetIntDatapointOnFocus()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool SetIntDatapointOnFocus::Redraw()
    {
      return Component::Redraw();
    }


    void SetIntDatapointOnFocus::InitValue(int value)
    {
      mValue = value;
    }


    void SetIntDatapointOnFocus::SetFocus(bool focus /*= true*/)
    {
      Component::SetFocus(focus);

      if (focus && mIntDP.IsValid())
        mIntDP->SetAsInt(mValue);

    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void SetIntDatapointOnFocus::SetSubjectPointer(int Id, Subject* pSubject)
    {
      mIntDP.Attach(pSubject);
    }


  } // namespace display
} // namespace mpc
