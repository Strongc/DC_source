/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : ErrorHandler                                          */
/*                                                                          */
/* FILE NAME        : ErrorHandler.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 03-01-2008  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Clears display and shows a flashing multiline error message.             */
/*                                                                          */
/****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <GUItype.h>
#include <GUI.h>
#include <WM.h>
#include <stdio.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <ErrorHandler.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

extern "C" GUI_CONST_STORAGE GUI_FONT Helvetica_57_13;

/*****************************************************************************
 * FUNCTION - ShowErrorMessage_C
 * DESCRIPTION: Displays an error message. 
 * Further actions (Rebooting or ignoring errors when not debugging) depend 
 * on the caller (or the caller of the caller...).
 *
 * Called by embOS\D3018X\OS_Error and Factory::FatalErrorOccured()
 *  
 ****************************************************************************/
extern "C" void ShowErrorMessage_C(const char* errorDescription = "-")
{
  int i;
  char sz_msg[250];
  WM_HWIN win;
  WM_HWIN win_next;
  GUI_RECT  rect;

  WM_SelectWindow(WM_GetDesktopWindow());
  win = WM_GetFirstChild(WM_GetDesktopWindow());

  while(win != (WM_HWIN) NULL)
  {
    win_next = WM_GetNextSibling(win);
    WM_DeleteWindow(win);
    win = win_next;
  }

  WM_SetStayOnTop(WM_GetDesktopWindow(),1);
  WM_BringToTop(WM_GetDesktopWindow());
  GUI_SetBkColor(GUI_WHITE);
  GUI_SetColor(GUI_BLACK);
  GUI_SetFont(&Helvetica_57_13);
  GUI_Clear();

  sprintf(sz_msg, "Fatal error.\n\n %s", errorDescription);

  rect.x0 = 0;
  rect.y0 = 10;
  rect.x1 = 239;
  rect.y1 = 319;

  for (i=0; i<5; i++)
  {
    GUI_Clear();
    GUI_Delay(100);
    GUI_DispStringInRect(sz_msg,&rect,GUI_TA_VCENTER|GUI_TA_HCENTER);
    GUI_Delay(500);
  }
}

/*****************************************************************************
 * FUNCTION - ShowErrorMessage
 * DESCRIPTION: the C++ entry to ShowErrorMessage_C()
 *  
 ****************************************************************************/
void ShowErrorMessage(const char* errorDescription /*= "-"*/)
{
  ShowErrorMessage_C(errorDescription);
}

