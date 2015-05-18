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
/* CLASS NAME       : TaskCtrlEvent                                         */
/*                                                                          */
/* FILE NAME        : TaskCtrlEvent.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 21-09-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : see h-file                                      */
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
#include <SubTask.h>
#include <TaskCtrlEvent.h>
#include <rtos.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  C functions declarations
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
 * DESCRIPTION: This is the constructor for the class, to construct
 * an object of the class type
 *****************************************************************************/
TaskCtrlEvent::TaskCtrlEvent(void)
{
  OS_CREATERSEMA(&mSemaSubTaskQueue);
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
TaskCtrlEvent::~TaskCtrlEvent(void)
{
  /* Taks are never destroyed no clean up */
}

/*****************************************************************************
 * Function - RunSubTasks
 * DESCRIPTION:
 *
*****************************************************************************/
void TaskCtrlEvent::RunSubTasks(void)
{
  SubTask* p_sub_task;

  OS_Use(&mSemaSubTaskQueue);
  p_sub_task = mSubTaskQueue.read();
  while (p_sub_task != NULL)
  {
    OS_Unuse(&mSemaSubTaskQueue);
    p_sub_task->RunSubTask();
    OS_Use(&mSemaSubTaskQueue);
    p_sub_task = mSubTaskQueue.read();
  }
  OS_Unuse(&mSemaSubTaskQueue);
}

/*****************************************************************************
 * Function - ReqRun
 * DESCRIPTION:
 *
*****************************************************************************/
void TaskCtrlEvent::ReqRun(SubTask* pSubTask)
{
  OS_Use(&mSemaSubTaskQueue);
  mSubTaskQueue.write(pSubTask);
  OS_Unuse(&mSemaSubTaskQueue);
}


/*****************************************************************************
 * Function - InitSubTasks
 * DESCRIPTION: Overrides base class function InitSubTasks
 *
*****************************************************************************/
void TaskCtrlEvent::InitSubTasks(void)
{
  TaskCtrl::InitSubTasks();
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 *              EXAMBLE OF A C FUNCTIONS
 *
 *
 ****************************************************************************/
