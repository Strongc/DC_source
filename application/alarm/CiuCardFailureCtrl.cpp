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
/* CLASS NAME       : CiuCardFailureCtrl                                    */
/*                                                                          */
/* FILE NAME        : CiuCardFailureCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 28-04-2008 dd-mm-yyyy                                 */
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
#include <CiuCardFailureCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
// SW Timers
enum
{
  CIU_COMM_ERROR_TIMER
};

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
CiuCardFailureCtrl::CiuCardFailureCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
CiuCardFailureCtrl::~CiuCardFailureCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CiuCardFailureCtrl::InitSubTask()
{
  mRunRequestedFlag = true;
  mCommErrorTiomeout = false;
  mpTimerObjList[CIU_COMM_ERROR_TIMER] = new SwTimer(10, S, true, true, this);  // Up to 10 sec. according to Geni spec.
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CiuCardFailureCtrl::RunSubTask()
{
  mRunRequestedFlag = false;
  bool err = true;
  bool no_card = false;


  if(mpCiuCardComm.IsUpdated()) // This parameter is used as "I'm alive". The module is sending it approx. each 4 second.
  {
    mCommErrorTiomeout = false;
    mpTimerObjList[CIU_COMM_ERROR_TIMER]->RetriggerSwTimer();
  }

  switch ( mpCiuCardConf->GetValue() )
  {
    case COM_CARD_CIM_150_PROFIBUS:
    case COM_CARD_CIM_200_MODBUS:
    case COM_CARD_CIM_250_GSM:
    case COM_CARD_CIM_270_GRM:
    case COM_CARD_CIM_300_BACNET:
      if(mpCiuCardId->GetQuality()==DP_AVAILABLE)
      {
        if(mpCiuCardId->GetValue() == mpCiuCardConf->GetValue())
        {
          err=false;
        }
      }
      break;

    case COM_CARD_NONE :
    case COM_CARD_OTHER_CARD:
    case COM_CARD_CIM_050_RS_485:
    case COM_CARD_CIM_100_LON:
    case COM_CARD_CIM_110_LON_MPC:
    default :
      no_card = true;
      err = false;
      break;
  }

  if( mCommErrorTiomeout && no_card==false)
  {
    err=true;
  }

  if(err)
  {
    mpCiuCardFault->SetValue(true);
  }
  else
  {
    mpCiuCardFault->SetValue(false);
  }
}


/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void CiuCardFailureCtrl::ConnectToSubjects()
{
  mpCiuCardComm->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void CiuCardFailureCtrl::Update(Subject* pSubject)
{
  mpCiuCardComm.Update(pSubject);

  if (pSubject == mpTimerObjList[CIU_COMM_ERROR_TIMER])
  {
    mCommErrorTiomeout = true;
  }

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
void CiuCardFailureCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void CiuCardFailureCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_CCFC_CIU_CARD_ID:
      mpCiuCardId.Attach(pSubject);
      break;
    case SP_CCFC_CIU_CARD_COMM:
      mpCiuCardComm.Attach(pSubject);
      break;
    case SP_CCFC_CIU_CARD_CONF:
      mpCiuCardConf.Attach(pSubject);
      break;
    case SP_CCFC_CIU_CARD_FAULT:
      mpCiuCardFault.Attach(pSubject);
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
