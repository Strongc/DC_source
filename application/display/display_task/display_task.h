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
/* CLASS NAME       : display_task                                                      */
/*                                                                          */
/* FILE NAME        : display_task.h                                                      */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcdisplay_task_h
#define mpcdisplay_task_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_SAMPLE_TIME 100
#define DISPLAY_MAIN_TASK_PRIO 100
#define DISPLAY_STACK_SIZE 64000

#ifndef __PC__
#define VNC_ENABLE 1                                      // Enable vnc-server
#define GUI_ENABLE 1                                      // Enable gui support
#else
#define VNC_ENABLE 0                                      // Disable vnc-server
#define GUI_ENABLE 1                                      // Disable gui support
#endif


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  EXPORTED FUNCTIONS
 *****************************************************************************/

extern "C" void DisplayInit(void);

extern "C" void StopDisplay(void);

extern "C" void PCStopDisplay(void);

extern "C" void Initializing(void);
#endif
