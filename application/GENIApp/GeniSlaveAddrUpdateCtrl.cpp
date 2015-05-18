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
/* CLASS NAME       : GeniSlaveAddrUpdateCtrl                               */
/*                                                                          */
/* FILE NAME        : GeniSlaveAddrUpdateCtrl.cpp                           */
/*                                                                          */
/* CREATED DATE     : 13-03-2012 dd-mm-yyyy                                 */
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
#include "GeniSlaveAddrUpdateCtrl.h"
#include "GeniSlaveIf.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define UNIT_NUMBER_ADDR_OFFSET 0x1F
#define GENI_BROADCAST_ADDR 0xFF

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
GeniSlaveAddrUpdateCtrl::GeniSlaveAddrUpdateCtrl()
{
  
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
GeniSlaveAddrUpdateCtrl::~GeniSlaveAddrUpdateCtrl()
{
  mpSlaveUnitNumber.Detach();
  mpChangeSlaveUnitNumberEvent.Detach();
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniSlaveAddrUpdateCtrl::InitSubTask()
{
  mChangePendingFlag = false;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniSlaveAddrUpdateCtrl::RunSubTask()
{
  bool success = false;
  unsigned char new_addr = 0;

  if( mChangePendingFlag )
  {
    new_addr = mpSlaveUnitNumber->GetValue() + UNIT_NUMBER_ADDR_OFFSET;

    success = GeniSlaveIf::GetInstance()->ChangeSlaveGeniAddress(GENI_BROADCAST_ADDR, new_addr);

    if( success == false )
    {
      // try again
      ReqTaskTime();
    }
    mChangePendingFlag = !success;
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void GeniSlaveAddrUpdateCtrl::ConnectToSubjects()
{
  mpSlaveUnitNumber.Subscribe(this);
  mpChangeSlaveUnitNumberEvent.Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void GeniSlaveAddrUpdateCtrl::Update(Subject* pSubject)
{
  if( mpChangeSlaveUnitNumberEvent.GetSubject() == pSubject )
  {
    mChangePendingFlag = true;
  }

  ReqTaskTime();
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniSlaveAddrUpdateCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpSlaveUnitNumber.Detach(pSubject);
  mpChangeSlaveUnitNumberEvent.Detach(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void GeniSlaveAddrUpdateCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
  case SP_GSAUC_NEW_GENI_SLAVE_UNIT_NUMBER:
    mpSlaveUnitNumber.Attach(pSubject);
    break;
  case SP_GSAUC_CHANGE_GENI_SLAVE_UNIT_NUMBER_EVENT:
    mpChangeSlaveUnitNumberEvent.Attach(pSubject);
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
