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
/* CLASS NAME       : AvalibleIfSet                                         */
/*                                                                          */
/* FILE NAME        : AvalibleIfSet.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AvalibleIfSet.               */
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
#include "AvalibleIfSet.h"
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
    AvalibleIfSet::AvalibleIfSet(Component* pParent) : CheckBox(pParent)
    {
      mInverted = false;
      mReadOnly = true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AvalibleIfSet::~AvalibleIfSet()
    {
    }

    bool AvalibleIfSet::Redraw()
    {
      ClearArea();
      Validate();
      return true;
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the check state of the check box. Returning the previous check state.
    *****************************************************************************/
    bool AvalibleIfSet::SetCheck(bool check /*= true*/)
    {
      return check;
    }

    /*
    * Sets the state this checkbox is associated with
    */
    void AvalibleIfSet::SetCheckState(int checkState)
    {
      AddCheckState( checkState );
    }


    void AvalibleIfSet::AddCheckState(int checkState)
    {
      mCheckStates.push_back(checkState);
    }

    void AvalibleIfSet::SubscribtionCancelled(Subject* pSubject)
    {
      mpDataPoint.Detach(pSubject);
    }

    void AvalibleIfSet::Update(Subject* pSubject)
    {
      mpDataPoint.Update(pSubject);
      Invalidate();
    }

    void AvalibleIfSet::Invert(bool inverted)
    {
      mInverted = inverted;
    }

    /*
    * return false (= is available) if one of the checkstates matches
    */
    bool AvalibleIfSet::IsNeverAvailable()
    {
      if (mpDataPoint.IsValid())
      {
        bool is_set = false;
        std::vector< int >::iterator iter = mCheckStates.begin();
        std::vector< int >::iterator iterEnd = mCheckStates.end();
        for(; iter != iterEnd; ++iter )
        {
          if(mpDataPoint->GetAsInt() == *iter)
          {
            is_set = true;
            break;
          }
        }

        if (mInverted)
          return is_set;
        else
          return !is_set;
      }
      
      return false;
    }

    bool AvalibleIfSet::IsValid()
    {
      // this component has no graphical representation, thus redraw is never needed.
      return true;
    }

    /* --------------------------------------------------
    * Connects this observer to the datapoint argument.
    * The checkbox is checked if the datapoint value
    * equals the value of the checkState argument.
    * --------------------------------------------------*/
    void AvalibleIfSet::SetSubjectPointer(int id, Subject* pSubject)
    {
      mpDataPoint.UnsubscribeAndDetach(this);
      mpDataPoint.Attach(pSubject);
    }

    void AvalibleIfSet::ConnectToSubjects(void)
    {
      if (mpDataPoint.IsValid())
      {
        mpDataPoint->Subscribe(this);
      }
    }


    bool AvalibleIfSet::HandleKeyEvent(Keys KeyID)
    {
      return false;
    }

      /* --------------------------------------------------
      * Gets the LEDs which this element and the ones below, wants to be on
      * or off. Normaly this is also the keys to react, but it doesn't have
      * to be this way ...
      * --------------------------------------------------*/
    Leds AvalibleIfSet::GetLedsStatus()
    {
      return COMBINE(NO_LED,NO_LED);
    }

  } // namespace display
} // namespace mpc


