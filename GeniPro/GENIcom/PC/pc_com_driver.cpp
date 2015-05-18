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
/* CLASS NAME       :                                                       */
/*                                                                          */
/* FILE NAME        : pc_com_driver.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 02-03-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/

#include "pc_com_driver.h"

#include "geni_if.h"
#include <rtos.h>
#include <pc_geni_dll_if.h>

extern "C" {
  PutGeni gFromCuToSimulator = 0;
  GetGeni gFromSimulatorToCu = 0;
}

OS_STACKPTR int StackComDriver[128]; /* Task stacks */
OS_TASK         TCBComDriver;        /* Task-control-blocks */
unsigned char mIsComTaskRunning = TRUE;

void ComTask(void)
{
  while (mIsComTaskRunning) 
  {
    GeniCOMInIrq();   // Event driven

    OS_Delay(5);
  }
}


unsigned char StartComTask()
{ 
  if (gFromCuToSimulator && gFromSimulatorToCu)
  {
    OS_CREATETASK(&TCBComDriver, "Com Task", ComTask, 100, StackComDriver); 
    return TRUE;
  }
  return FALSE;
}


void StopComTask() 
{
  mIsComTaskRunning = 0;
}

void SendComData(const unsigned char* data, unsigned int len)
{
  if (gFromCuToSimulator && mIsComTaskRunning)
  {
    (gFromCuToSimulator)((unsigned char*)data, len);
  }
}


unsigned int ReceiveComData(unsigned char* destBuf, unsigned int maxCount)
{
  if (gFromSimulatorToCu && mIsComTaskRunning)
  {
    return (gFromSimulatorToCu)(destBuf, maxCount);
  }
  else 
  {
    return 0;
  }
}







