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
/* CLASS NAME       : GeniProConfCtrl                                       */
/*                                                                          */
/* FILE NAME        : GeniProConfCtrl.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 27-03-2008 dd-mm-yyyy                                 */
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
#include <GeniProConfCtrl.h>
extern "C"
{
  #include <geni_if.h>         /* Access to GENI vars */
}

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
GeniProConfCtrl::GeniProConfCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
GeniProConfCtrl::~GeniProConfCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniProConfCtrl::InitSubTask()
{
  slave_unit_addr = mpGeniSlaveUnitAddress->GetValue();
  slave_group_addr = mpGeniSlaveGroupAddress->GetValue();

  mpGeniBaudRateConfig.SetUpdated();
  mpGeniMinReplyDelayConfig.SetUpdated();

  mRunRequestedFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniProConfCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  // Update geni slave addresses
  if (mpGeniSlaveUnitAddress.IsUpdated())
  {
    slave_unit_addr = mpGeniSlaveUnitAddress->GetValue();   // GeniPro variable
  }
  if (mpGeniSlaveGroupAddress.IsUpdated())
  {
    slave_group_addr = mpGeniSlaveGroupAddress->GetValue(); // GeniPro variable
  }

  #if (CTO_BUS_TYPE == Slave)
  // Update geni slave bus baud rate
  if (mpGeniBaudRateConfig.IsUpdated())
  {
    baud_rate_default = mpGeniBaudRateConfig->GetValue();   // GeniPro variable
    GeniPresetActBusBaudrate();                             // To activate use of new baud rate
    mpGeniBaudRateSet->SetValue(baud_rate_default);         // Update actual value as well
  }
  if (mpGeniBaudRateSet.IsUpdated() && mpGeniBaudRateSet->GetValue() != set_baud_rate)
  {
    set_baud_rate = mpGeniBaudRateSet->GetValue();          // GeniPro variable
    GeniSetActBusBaudrate();                                // To activate use of new baud rate
  }
  #endif

  // Update geni slave bus min reply delay
  if (mpGeniMinReplyDelayConfig.IsUpdated())
  {
    min_reply_delay_default = mpGeniMinReplyDelayConfig->GetValue();  // GeniPro variable
    mpGeniMinReplyDelaySet->SetValue(min_reply_delay_default);        // Update actual value as well
  }
  if (mpGeniMinReplyDelaySet.IsUpdated())
  {
    set_min_reply_delay = mpGeniMinReplyDelaySet->GetValue();         // GeniPro variable
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void GeniProConfCtrl::ConnectToSubjects()
{
  mpGeniSlaveUnitAddress->Subscribe(this);
  mpGeniSlaveGroupAddress->Subscribe(this);
  mpGeniBaudRateConfig->Subscribe(this);
  mpGeniBaudRateSet->Subscribe(this);
  mpGeniMinReplyDelayConfig->Subscribe(this);
  mpGeniMinReplyDelaySet->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void GeniProConfCtrl::Update(Subject* pSubject)
{
  mpGeniSlaveUnitAddress.Update(pSubject);
  mpGeniSlaveGroupAddress.Update(pSubject);
  mpGeniBaudRateConfig.Update(pSubject);
  mpGeniBaudRateSet.Update(pSubject);
  mpGeniMinReplyDelayConfig.Update(pSubject);
  mpGeniMinReplyDelaySet.Update(pSubject);

  if (mRunRequestedFlag == false)
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
void GeniProConfCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void GeniProConfCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_GPC_GENI_SLAVE_UNIT_ADDRESS:
      mpGeniSlaveUnitAddress.Attach(pSubject);
      break;
    case SP_GPC_GENI_SLAVE_GROUP_ADDRESS:
      mpGeniSlaveGroupAddress.Attach(pSubject);
      break;
    case SP_GPC_GENI_BAUD_RATE_CONFIG:
      mpGeniBaudRateConfig.Attach(pSubject);
      break;
    case SP_GPC_GENI_BAUD_RATE_SET:
      mpGeniBaudRateSet.Attach(pSubject);
      break;
    case SP_GPC_GENI_MIN_REPLY_DELAY_CONFIG:
      mpGeniMinReplyDelayConfig.Attach(pSubject);
      break;
    case SP_GPC_GENI_MIN_REPLY_DELAY_SET:
      mpGeniMinReplyDelaySet.Attach(pSubject);
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
