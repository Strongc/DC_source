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
/* CLASS NAME       : EthernetCtrl                                          */
/*                                                                          */
/* FILE NAME        : EthernetCtrl.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 15-02-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <SwTimer.h>
#include <AppTypeDefs.h>
#include <FactoryTypes.h>
#include <StringDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "EthernetCtrl.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
enum  // TYPES OF SW TIMERS
{
  HALF_LEASE_TIME_TIMER,
  LEASE_TIME_ENDED_TIMER
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
EthernetCtrl* EthernetCtrl::mpInstance = 0;
/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 ****************************************************************************/
EthernetCtrl* EthernetCtrl::GetInstance()
{
  if (!mpInstance)
  {
    mpInstance = new EthernetCtrl();
  }
  return mpInstance;
}

/*****************************************************************************
 * Function -  RunSubTask
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::RunSubTask()
{
}
/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::Update(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::SetSubjectPointer(int Id,Subject* pSubject)
{
	// increment reference count manually to make RunFactory work
	pSubject->IncRefCount();
}
/*****************************************************************************
 * Function - TimeOutFunc
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::InitSubTask()
{
}

/*****************************************************************************
 * Function - InitNormal
 * DESCRIPTION:
 *            test
 ****************************************************************************/
void EthernetCtrl::InitLoopBackTest()
{
}

/*****************************************************************************
 * Function - GetEthernetLoopBackTestStatus
 * DESCRIPTION:
 *
 ****************************************************************************/
ETHERNET_LOOP_BACK_TEST_TYPE EthernetCtrl::GetEthernetLoopBackTestStatus()
{
  return ETH_TEST_OK;
}
/*****************************************************************************
 * Function - InitNormal
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::InitNormal()
{

}
/*****************************************************************************
 * Function - TimeOutFunc
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::ConnectToSubjects()
{
}
/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
bool EthernetCtrl::IsEthernetLoopBackTestEnabled()
{
  return true;
}

/*****************************************************************************
 * Function - Constructor
 *****************************************************************************/
EthernetCtrl::EthernetCtrl()
{
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
EthernetCtrl::~EthernetCtrl()
{
}

void EthernetCtrlGetMacAddr(unsigned short *mac0, unsigned short *mac1, unsigned short *mac2)
{
}

extern "C" void EthernetCtrlGetMacAddr_C(unsigned short *mac0, unsigned short *mac1, unsigned short *mac2)
{
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

