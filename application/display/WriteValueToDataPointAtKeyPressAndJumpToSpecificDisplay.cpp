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
/* CLASS NAME       : WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay      */
/*                                                                          */
/* FILE NAME        : WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay.cpp  */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for at key press to write a given value to a   */
/* DataPoint and then jump to a given Display                               */
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
#include "DisplayController.h"
#include "WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay.h"

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
    WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay::WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay(Component* pParent) : Component(pParent)
    {
      mCheckState = 0;
      mLegalKeys = MPC_OK_KEY;
      mLedsStatus = COMBINE(NO_LED,OK_LED);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay::~WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay()
    {
    }


    /*
    * Sets the state this checkbox is associated with
    */
    void WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay::SetWriteState(int checkState)
    {
      mCheckState = checkState;
    }

    void WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay::SubscribtionCancelled(Subject* pSubject)
    {
      mpDataPoint.Detach(pSubject);
    }

    void WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay::Update(Subject* pSubject)
    {
    }

    /* --------------------------------------------------
    * Connects this observer to the datapoint argument.
    * The checkbox is checked if the datapoint value
    * equals the value of the checkState argument.
    * --------------------------------------------------*/
    void WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay::SetSubjectPointer(int id, Subject* pSubject)
    {
			mpDataPoint.UnsubscribeAndDetach(this);
			mpDataPoint.Attach(pSubject);
    }

    void WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay::ConnectToSubjects(void)
    {
      if(mpDataPoint.IsValid())
      {
        mpDataPoint->Subscribe(this);
        SetWriteState(mpDataPoint->GetAsInt());
      }
    }


    bool WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay::HandleKeyEvent(Keys KeyID)
    {
      switch(KeyID)
      {
        case MPC_OK_KEY:
        {
          int currentValue = mpDataPoint->GetAsInt();

          if (currentValue == mCheckState)
          {
            //make sure observers are notified (NotifyObservers is protected)
            if (currentValue != mpDataPoint->GetMinAsInt())
              mpDataPoint->SetAsInt( mpDataPoint->GetMinAsInt() );
            else
              mpDataPoint->SetAsInt( mpDataPoint->GetMaxAsInt() );
          }

          mpDataPoint->SetAsInt(mCheckState);
                    
          if( mpDisplay )
          {
#ifdef __FADE___
            DisplayController::GetInstance()->Push(mpDisplay, 224, WM_GetWindowOrgY(GetWMHandle()));
#else
            DisplayController::GetInstance()->Push(mpDisplay);
#endif // __FADE___
          }
          return true;
        }

        case MPC_ESC_KEY:
          DisplayController::GetInstance()->Pop();
          return true;
      }
      return false;
    }

  } // namespace display
} // namespace mpc


