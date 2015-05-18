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
/* CLASS NAME       : ModeCheckBox                                          */
/*                                                                          */
/* FILE NAME        : ModeCheckBox.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a ModeCheckBox.                */
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
#include "ModeCheckBox.h"

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
    ModeCheckBox::ModeCheckBox(Component* pParent) : CheckBox(pParent)
    {
      mCheckState = -1; // -1 Unlikely ?
			mpDataPoint.SetUpdated();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ModeCheckBox::~ModeCheckBox()
    {
    }


    void ModeCheckBox::Run()
    {
      if (!IsVisible())
      {
        Validate();
        return;
      }

      if (mpDataPoint.IsUpdated())
			{
        CheckBox::SetCheck(mpDataPoint->GetAsInt() == mCheckState);
			}			
				
      CheckBox::Run();
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the check state of the check box. Returning the previous check state.
    *****************************************************************************/
    bool ModeCheckBox::SetCheck(bool check /*= true*/)
    {
      bool rc = CheckBox::IsChecked(); 
			
      // do not react to the call if there is no datapoint or the check isn't set.
      if (!mpDataPoint.IsValid() || (check == false))
      {
        return rc;
      }

			mpDataPoint->SetAsInt(mCheckState);
			
      return rc;
    }

    /*
    * Sets the state this checkbox is associated with
    */
    void ModeCheckBox::SetCheckState(int checkState)
    {
      mCheckState = checkState;

      if (mpDataPoint.IsValid())
			{
				CheckBox::SetCheck(mpDataPoint->GetAsInt() == mCheckState);
			}
    }

    void ModeCheckBox::SubscribtionCancelled(Subject* pSubject)
    {
			mpDataPoint.Detach(pSubject);
    }

    void ModeCheckBox::Update(Subject* pSubject)
    {
			mpDataPoint.Update(pSubject);
    }

    bool ModeCheckBox::IsNeverAvailable()
    {
			if (!mpDataPoint.IsValid() || (mpDataPoint->GetQuality() == DP_NEVER_AVAILABLE))
			{
				return true;
			}
			else
			{
				return false;
			}
    }

    /* --------------------------------------------------
    * Connects this observer to the datapoint argument.
    * The checkbox is checked if the datapoint value
    * equals the value of the checkState argument.
    * --------------------------------------------------*/
    void ModeCheckBox::SetSubjectPointer(int id, Subject* pDataPoint)
    {
			if (mpDataPoint.Attach(pDataPoint))
			{
			    CheckBox::SetCheck(mpDataPoint->GetAsInt() == mCheckState);
			}
			
      Invalidate();
    }

    void ModeCheckBox::ConnectToSubjects(void)
    {
			if (mpDataPoint.IsValid())
			{
				mpDataPoint->Subscribe(this);
			}
			
      Invalidate();
    }


    bool ModeCheckBox::HandleKeyEvent(Keys KeyID)
    {
      if (IsReadOnly() || mChecked)
			{
        return CheckBox::HandleKeyEvent(KeyID);
			}

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
    Leds ModeCheckBox::GetLedsStatus()
    {
      if(IsReadOnly() || mChecked)
      {
        return COMBINE(NO_LED,NO_LED);
      }
      return COMBINE(NO_LED,OK_LED);
    }

  } // namespace display
} // namespace mpc


