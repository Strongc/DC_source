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
/* CLASS NAME       : VfdSignalCtrl                                         */
/*                                                                          */
/* FILE NAME        : VfdSignalCtrl.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 04-05-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
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
#include <VfdSignalCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/


/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
VfdSignalCtrl::VfdSignalCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
VfdSignalCtrl::~VfdSignalCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdSignalCtrl::InitSubTask()
{
  mpVfdInstalled.SetUpdated();

  mRunRequestedFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdSignalCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if (mpVfdInstalled->GetValue() == true)
  {
    // Force update of CUE requests when communication has just been established
    if (mpCueCommunicationFlag.IsUpdated() && mpCueCommunicationFlag->GetValue() == true)
    {
      mpVfdStart.SetUpdated();
      mpVfdReverse.SetUpdated();
    }

    // Handle start/stop
    if (mpVfdStart.IsUpdated())
    {
      if (mpCueEnabled->GetValue() == true)
      {
        if (mpVfdStart->GetValue() == true)
        {
          mpCuePumpStartEvent->SetEvent();
        }
        else
        {
          mpCuePumpStopEvent->SetEvent();
        }
      }
    }

    // Handle forward/reverse
    if (mpVfdReverse.IsUpdated())
    {
      if (mpCueEnabled->GetValue() == true)
      {
        if (mpVfdReverse->GetValue() == true)
        {
          mpCuePumpReverseEvent->SetEvent();
        }
        else
        {
          mpCuePumpForwardEvent->SetEvent();
        }
      }
    }

    // Handle new frequency
    if (mpVfdFrequency.IsUpdated())
    {
      float vfd_frequency = mpVfdFrequency->GetValue();
      mpVfdAnalogOutput->SetValueAsPercent(100.0f*vfd_frequency/mpVfdFrequency->GetMaxValue());
      if (mpCueEnabled->GetValue() == true)
      {
        mpCuePumpRefFrequency->SetValue(vfd_frequency);
      }
    }
  }

  // Set a CUE installed flag. Depending on VFD installed and CUE enabled
  if (mpVfdInstalled.IsUpdated() || mpCueEnabled.IsUpdated())
  {
    mpCueInstalled->SetValue(mpVfdInstalled->GetValue() == true && mpCueEnabled->GetValue() == true);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void VfdSignalCtrl::ConnectToSubjects()
{
  mpVfdInstalled->Subscribe(this);
  mpCueEnabled->Subscribe(this);
  mpVfdFrequency->Subscribe(this);
  mpVfdStart->Subscribe(this);
  mpVfdReverse->Subscribe(this);
  mpCueCommunicationFlag->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class.
 *
 *****************************************************************************/
void VfdSignalCtrl::Update(Subject* pSubject)
{
  mpVfdInstalled.Update(pSubject);
  mpCueEnabled.Update(pSubject);
  mpVfdFrequency.Update(pSubject);
  mpVfdStart.Update(pSubject);
  mpVfdReverse.Update(pSubject);
  mpCueCommunicationFlag.Update(pSubject);

  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdSignalCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void VfdSignalCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_VFS_VFD_INSTALLED:
      mpVfdInstalled.Attach(pSubject);
      break;
    case SP_VFS_VFD_CUE_ENABLED:
      mpCueEnabled.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_VFS_VFD_FREQUENCY:
      mpVfdFrequency.Attach(pSubject);
      break;
    case SP_VFS_VFD_START:
      mpVfdStart.Attach(pSubject);
      break;
    case SP_VFS_VFD_REVERSE:
      mpVfdReverse.Attach(pSubject);
      break;
    case SP_VFS_CUE_COMMUNICATION_FLAG:
      mpCueCommunicationFlag.Attach(pSubject);
      break;

    // Outputs:
    case SP_VFS_VFD_ANALOG_OUTPUT:
      mpVfdAnalogOutput.Attach(pSubject);
      break;
    case SP_VFS_CUE_PUMP_START_EVENT:
      mpCuePumpStartEvent.Attach(pSubject);
      break;
    case SP_VFS_CUE_PUMP_STOP_EVENT:
      mpCuePumpStopEvent.Attach(pSubject);
      break;
    case SP_VFS_CUE_PUMP_FORWARD_EVENT:
      mpCuePumpForwardEvent.Attach(pSubject);
      break;
    case SP_VFS_CUE_PUMP_REVERSE_EVENT:
      mpCuePumpReverseEvent.Attach(pSubject);
      break;
    case SP_VFS_CUE_PUMP_REF_FREQUENCY:
      mpCuePumpRefFrequency.Attach(pSubject);
      break;
    case SP_VFS_CUE_INSTALLED:
      mpCueInstalled.Attach(pSubject);
      break;

    default:
      break;
  }
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
