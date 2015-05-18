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
/* CLASS NAME       : OnOffAutoCtrl                                         */
/*                                                                          */
/* FILE NAME        : OnOffAutoCtrl.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 26-06-2007 dd-mm-yyyy                                 */
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
#include <OnOffAutoCtrl.h>

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
OnOffAutoCtrl::OnOffAutoCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
OnOffAutoCtrl::~OnOffAutoCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void OnOffAutoCtrl::InitSubTask()
{
  mRunRequestedFlag = true;
  mpUnderLowestStopLevel.SetUpdated();
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void OnOffAutoCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if(mpUnderLowestStopLevel.IsUpdated())
  {
    if(mpUnderLowestStopLevel->GetValue()==true && mpOprModeBus->GetValue()==REQ_OPERATION_MODE_ON)
    {
      mpOprModeBus->SetValue(REQ_OPERATION_MODE_AUTO);
    }
  }

  // Handle bus commands
  if (mpBusCmdStop.IsUpdated())
  {
    mpOprModeBus->SetValue(REQ_OPERATION_MODE_OFF);
  }
  if (mpBusCmdStart.IsUpdated())
  {
    if(mpUnderLowestStopLevel->GetValue()==true)
    {
      mpOprModeBus->SetValue(REQ_OPERATION_MODE_AUTO);
    }
    else
    {
      mpOprModeBus->SetValue(REQ_OPERATION_MODE_ON);
    }
  }
  if (mpBusCmdAuto.IsUpdated())
  {
    mpOprModeBus->SetValue(REQ_OPERATION_MODE_AUTO);
  }


  if (mpOprModeDI->GetValue() != REQ_OPERATION_MODE_AUTO)
  {
    // Operation Mode from digital inputs has higest priority
    mpOprModeReq->SetValue(mpOprModeDI->GetValue());
    mpPumpControlSource->SetValue(CONTROL_SOURCE_DI);
  }
  else if (mpOprModeUser->GetValue() != REQ_OPERATION_MODE_AUTO)
  {
    // Operation Mode from user display has second higest priority
    mpOprModeReq->SetValue(mpOprModeUser->GetValue());
    mpPumpControlSource->SetValue(CONTROL_SOURCE_HMI);
  }
  else if (mpOprModeBus->GetValue() != REQ_OPERATION_MODE_AUTO)
  {
    // Operation Mode from geni bus has third higest priority
    mpOprModeReq->SetValue(mpOprModeBus->GetValue());
    mpPumpControlSource->SetValue(mpPumpBusSource->GetValue());
  }
  else
  {
    // Operation Mode is not decided manually - set request to AUTO
    mpOprModeReq->SetValue(REQ_OPERATION_MODE_AUTO);
    mpPumpControlSource->SetValue(CONTROL_SOURCE_SYSTEM);
  }

  if( mpOprModeReq->GetValue() == REQ_OPERATION_MODE_AUTO)
  {
    mpPumpManual->SetValue(false);
  }
  else
  {
    mpPumpManual->SetValue(true);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void OnOffAutoCtrl::ConnectToSubjects()
{
  mpOprModeDI->Subscribe(this);
  mpOprModeUser->Subscribe(this);
  mpOprModeBus->Subscribe(this);
  mpBusCmdStop->Subscribe(this);
  mpBusCmdStart->Subscribe(this);
  mpBusCmdAuto->Subscribe(this);
  mpPumpBusSource->Subscribe(this);
  mpUnderLowestStopLevel->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void OnOffAutoCtrl::Update(Subject* pSubject)
{
  mpBusCmdStop.Update(pSubject);
  mpBusCmdStart.Update(pSubject);
  mpBusCmdAuto.Update(pSubject);
  mpUnderLowestStopLevel.Update(pSubject);

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
void OnOffAutoCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void OnOffAutoCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_OOAC_OPERATION_MODE_DI:
      mpOprModeDI.Attach(pSubject);
      break;
    case SP_OOAC_OPERATION_MODE_USER:
      mpOprModeUser.Attach(pSubject);
      break;
    case SP_OOAC_OPERATION_MODE_BUS:
      mpOprModeBus.Attach(pSubject);
      break;

    case SP_OOAC_BUS_CMD_STOP:
      mpBusCmdStop.Attach(pSubject);
      break;
    case SP_OOAC_BUS_CMD_START:
      mpBusCmdStart.Attach(pSubject);
      break;
    case SP_OOAC_BUS_CMD_AUTO:
      mpBusCmdAuto.Attach(pSubject);
      break;

    case SP_OOAC_OPERATION_MODE_REQ:
      mpOprModeReq.Attach(pSubject);
      break;

    case SP_OOAC_PUMP_CONTROL_SOURCE:
      mpPumpControlSource.Attach(pSubject);
      break;
    case SP_OOAC_PUMP_BUS_SOURCE:
      mpPumpBusSource.Attach(pSubject);
      break;
    case SP_OOAC_UNDER_LOWEST_STOP_LEVEL:
      mpUnderLowestStopLevel.Attach(pSubject);
      break;

    case SP_OOAC_PUMP_MANUAL:
      mpPumpManual.Attach(pSubject);
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
