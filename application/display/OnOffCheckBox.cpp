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
/* CLASS NAME       : OnOffCheckBox                                         */
/*                                                                          */
/* FILE NAME        : OnOffCheckBox.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a OnOffCheckBox.               */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "OnOffCheckBox.h"

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
    OnOffCheckBox::OnOffCheckBox(Component* pParent) : ModeCheckBox(pParent)
    {
      mNotCheckState = -1; // -1 Unlikely ?
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    OnOffCheckBox::~OnOffCheckBox()
    {
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the check state of the check box. Returning the previous check state.
    *****************************************************************************/
    bool OnOffCheckBox::SetCheck(bool check /*= true*/)
    {
      // do not react to the call if there is no datapoint
      if (!mpDataPoint.IsValid())
      {
        return false;
      }

      if (check)
      {
        mpDataPoint->SetAsInt(mCheckState);
      }
      else
      {
        mpDataPoint->SetAsInt(mNotCheckState);
      }
			
      return CheckBox::SetCheck(check);
    }



    /*
    * Sets the state this checkbox is associated with when not checked
    */
    void OnOffCheckBox::SetNotCheckState(int notCheckState)
    {
      mNotCheckState = notCheckState;

			SetCheckState(mCheckState);
    }


    bool OnOffCheckBox::HandleKeyEvent(Keys KeyID)
    {
      if(IsReadOnly())
        return CheckBox::HandleKeyEvent(KeyID);

      switch(KeyID)
      {
        case MPC_OK_KEY:
          SetCheck(!IsChecked());
          return true;
      }
      return CheckBox::HandleKeyEvent(KeyID);
    }

      /* --------------------------------------------------
      * Gets the LEDs which this element and the ones below, wants to be on
      * or off. Normaly this is also the keys to react, but it doesn't have
      * to be this way ...
      * --------------------------------------------------*/
    Leds OnOffCheckBox::GetLedsStatus()
    {
      if(IsReadOnly())
      {
        return COMBINE(NO_LED,NO_LED);
      }
      return COMBINE(NO_LED,OK_LED);
    }

  } // namespace display
} // namespace mpc


