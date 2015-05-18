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
/* CLASS NAME       : display_task                                          */
/*                                                                          */
/* FILE NAME        : display_task.cpp                                      */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "typedef.h"    /*definements for the Grundfos common types */
#include "RTOS.H"
#include "microp.h"
#include <ledclass.h>
#include "gpio.h"
#include "pwm_drv.h"
#include <PowerDown.h>

#ifdef __PC__
#include <WinMain/ApplicationProtocol.h>
#include <PcDevToolService.h>
#include <PcSimulatorService.h>
#endif

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "stdio.h"
#include "GUI.h"
#include "GUI_VNC.h"
#include <display_task.h>
#include <keyboard.h>
#include <DisplayController.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define INITIAL_BACKLIGHT 700
#define INITIAL_CONTRAST 550

#define POWER_DOWN_EVENT 1

 /*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

  /*****************************************************************************
  EXTERNS
  *****************************************************************************/
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmStartingUp;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/


U16 get_led_status(void)
{
  U16 led = 0;
#ifndef __PC__
  GPio* p_io = GPio::GetInstance();

  led |= p_io->isSet(GP_IO_LED_MENU)      ? 0x0001 : 0;
  led |= p_io->isSet(GP_IO_LED_QUESTION)  ? 0x0002 : 0;
  led |= p_io->isSet(GP_IO_LED_UP)        ? 0x0004 : 0;
  led |= p_io->isSet(GP_IO_LED_DOWN)      ? 0x0008 : 0;
  led |= p_io->isSet(GP_IO_LED_PLUS)      ? 0x0010 : 0;
  led |= p_io->isSet(GP_IO_LED_MINUS)     ? 0x0020 : 0;
  led |= p_io->isSet(GP_IO_LED_ESC)       ? 0x0040 : 0;
  led |= p_io->isSet(GP_IO_LED_OK)        ? 0x0080 : 0;
  led |= p_io->isSet(GP_IO_LED_HOME)      ? 0x0100 : 0;
  led |= p_io->isSet(GP_IO_LED_RED)       ? 0x0200 : 0;
  led |= p_io->isSet(GP_IO_LED_GREEN)     ? 0x0400 : 0;
#endif
  return led;
}

#ifndef __PC__
  void LCDPowerDown(void)
  {
    unsigned int i=0;

#ifndef TFT_16_BIT_LCD
    GPio::GetInstance()->SetAs(GP_IO_CONTRAST, OUTPUT, 0); // GPIO39 contrast
#endif
    REGISTER16(VR4181A_PWRCONREG1) = 2;   // 31;

    SetDutyCycleHW_1(4095);
#ifndef TFT_16_BIT_LCD
    SetDutyCycleHW_2(4095);
#endif

    REGISTER16(VR4181A_PWRCONREG2) &= ~0x0400;  // PowerC -> 0

     OS_Delay(250);

    REGISTER16(VR4181A_PWRCONREG2) |=  0x8000;  // LCUStopReq -> 1

    i=0;

    while (REGISTER16(VR4181A_LCDCTRLREG) & 0x0200)  // wait until IdleCycle == 0
    {
      OS_Delay(1);
      if (++i > 10)
      {
        i++;  // Just to be able to break ...
        break;
      }
    }

    REGISTER16(VR4181A_LCDCTRLREG) &= ~0x0008;  // Clear ContCkE of LCDCTRL to stop Gclk and Hpck
  }
#endif

void DisplaySetup(void)
{
  GUI_Init();
  OS_SendString("GUI init done\n");

#ifdef __WEB_ENABLE
  GUI_VNC_X_StartServer(0, 0);
  OS_DebugSendString("VNC server init done\n");
#endif

}

void DisplayHwSetup(void)
{
  GPio* p_io = GPio::GetInstance();

  p_io->SetAs(GP_IO_LED_MENU, OUTPUT, 0);           // menu
  p_io->SetAs(GP_IO_LED_QUESTION, OUTPUT, 0);          // ?
  p_io->SetAs(GP_IO_LED_UP, OUTPUT, 0);               // up
  p_io->SetAs(GP_IO_LED_DOWN, OUTPUT, 0);           // down
  p_io->SetAs(GP_IO_LED_PLUS, OUTPUT, 0);             // +
  p_io->SetAs(GP_IO_LED_MINUS, OUTPUT, 0);            // -
  p_io->SetAs(GP_IO_LED_OK, OUTPUT, 0);               // ok
  p_io->SetAs(GP_IO_LED_ESC, OUTPUT, 0);             // esc
  p_io->SetAs(GP_IO_LED_HOME, OUTPUT, 0);           // home
//  p_io->SetAs(GP_IO_LED_RED, OUTPUT, 0);      // error LED
  p_io->SetAs(GP_IO_LED_GREEN, OUTPUT, 0);        // ok LED

  InitializePWMHW_1();                  // LED backlight
  SetDutyCycleHW_1(4*INITIAL_BACKLIGHT);
  EnablePWMHW_1();

#ifndef TFT_16_BIT_LCD
  InitializePWMHW_2();                  // Contrast
  SetDutyCycleHW_2(4*INITIAL_CONTRAST);
  EnablePWMHW_2();

  p_io->SetAs(GP_IO_CONTRAST, OUTPUT, 1);             // GPIO39 contrast
#endif
  p_io->SetAs(GPIO8, OUTPUT, 0);              // Enable all LED's
}

static bool display_running = true;

void StartDisplay(void)
{
  GUI_SetColor(GUI_COLOUR_DEFAULT_FOREGROUND);
  GUI_SetBkColor(GUI_COLOUR_DEFAULT_BACKGROUND);
  GUI_UC_SetEncodeUTF8();
  GUI_DrawBitmap(&bmStartingUp,0,0);
}

OS_STACKPTR int DisplayTaskStack[DISPLAY_STACK_SIZE];
OS_TASK TCBDisplayMainTask;

extern "C" void DisplayMainTask(void)
{
  int  start_task_time, end_task_time, task_time_diff;
#ifndef __PC__
  int i;

  for (i = 4*INITIAL_BACKLIGHT; i > 0; i -= 4)
  {
    SetDutyCycleHW_1(i);
    GUI_Delay(1);
  }
#endif

  #ifdef __PC__ // Kill the rest of the system running on PC
  HWND main_wnd_handle = PcDevToolService::GetInstance()->GetControllerWindow();
  if(main_wnd_handle)
  {
    ShowWindow(main_wnd_handle, SW_SHOWNORMAL);
  }
  #endif // __PC__

  while (display_running)
  {
    start_task_time = OS_GetTime();
    mpc::display::DisplayController::GetInstance()->Run();
    GUI_Delay(5);

    #ifdef __PC__
    if( HasOSMessage() )
    {
      OS_EnterRegion();
      OS_IncDI();
      OSMessage* msg;

      // Get message from queue without removing it.
      msg = PeekOSMessage();
      switch(msg->message)
      {
      case 0:
        break;
      case OSMSG_LOAD_DISPLAY:
        msg->rc = PcDevToolService::GetInstance()->LoadDisplay((const char*)(msg->data));
        break;
      case OSMSG_LOAD_DISPLAY_ID:
        msg->rc = PcDevToolService::GetInstance()->LoadDisplay(*(int*)(msg->data));
        break;
      case OSMSG_SET_LANGUAGE:
        Languages::GetInstance()->SetLanguage(*((LANGUAGE_TYPE*)msg->data));
        msg->rc = 0;
        break;
      case OSMSG_GET_LANGUAGE:
        msg->rc = Languages::GetInstance()->GetLanguage();        
        break;
      case OSMSG_DUMP_SCREEN:
        mpc::display::DisplayDumper::GetInstance()->DumpScreen(false);
        msg->rc = 0;
        break;
      case OSMSG_DUMP_SCREENS:
        mpc::display::DisplayDumper::GetInstance()->DumpScreens(true, false);
        msg->rc = 0;
        break;
      case OSMSG_SET_SUBJECT_VALUE:
        msg->rc = PcDevToolService::GetInstance()->SetSubjectValue( ((SubjectValueParameters*)msg->data) );
        break;
      case OSMSG_GET_SUBJECT_VALUE_AS_FLOAT:
        msg->rc = PcDevToolService::GetInstance()->GetSubjectValue( ((SubjectValueParameters*)msg->data) );
        break;
      case OSMSG_SET_SUBJECT_QUALITY:
        msg->rc = PcDevToolService::GetInstance()->SetSubjectQuality( ((QualityParameters*)msg->data) );
        break;
      case OSMSG_SET_DI_VALUE:
        msg->rc = PcSimulatorService::GetInstance()->SetDiValue( ((DiParameters*)msg->data) );
        break;
      case OSMSG_SET_AI_VALUE_PERCENT:
        msg->rc = PcSimulatorService::GetInstance()->SetAiValueInPercent( ((AiParameters*)msg->data) );
        break;
      case OSMSG_SET_AI_VALUE_INTERNAL:
        msg->rc = PcSimulatorService::GetInstance()->SetAiValueInInternalUnit( ((AiParameters*)msg->data) );
        break;
      case OSMSG_SELECT_LISTVIEW_ITEM_BY_INDEX:
        msg->rc = PcDevToolService::GetInstance()->SelectListViewItem(*(int*)(msg->data));
        break;
      case OSMSG_KEY_PRESS:
        GUI_StoreKey(*(int*)(msg->data));
        msg->rc = 0;
        break;
      case OSMSG_SET_ERROR_PRESENT:
        msg->rc = PcSimulatorService::GetInstance()->SimulateAlarm( ((AlarmParameters*)msg->data) );
        break;
      case OSMSG_EXPORT_STRING_LENGTHS:
        mpc::display::StringWidthCalculator::GetInstance()->ExportStringWidths((const char*)(msg->data));
        msg->rc = 0;
        break;
      case OSMSG_EXPORT_STRING_LENGTHS_ADV:
        mpc::display::StringWidthCalculator::GetInstance()->ExportStringWidthsAdv(((mpc::display::StringWidthParameters*)(msg->data)));
        msg->rc = 0;
        break;
      case OSMSG_SET_IO111_VALUES:
        msg->rc = PcSimulatorService::GetInstance()->SetIO111Values( ((Io111Parameters*)msg->data) );
        break;
      default:
        msg->rc = -1;
        break;
      }
      // Remove message from queue
      DisposeOSMessage();

      OS_DecRI();
      OS_LeaveRegion();
      OS_Delay(1);
    }
    #endif //__PC__

    // calculate time to wait
    end_task_time = OS_GetTime();
    task_time_diff = end_task_time - start_task_time;

    if( task_time_diff <= 1 )
      OS_WaitSingleEventTimed( POWER_DOWN_EVENT, DISPLAY_SAMPLE_TIME );         // We will skip ...
    else if ( task_time_diff > (DISPLAY_SAMPLE_TIME-1) )
      OS_WaitSingleEventTimed( POWER_DOWN_EVENT, 1 );                          // Run again, now
    else
      OS_WaitSingleEventTimed( POWER_DOWN_EVENT, DISPLAY_SAMPLE_TIME - task_time_diff );
  }
#ifndef __PC__
  LCDPowerDown();                     // TODO: Check the for-loops !! It takes a lot of time
#endif
  SignalEventToPowerDown(DISPLAY_POWERED_DOWN_EVENT);
#ifndef __PC__
  while (1)                                                                    // We will die anyway
  {
    GUI_Delay(1000);
  }
#endif
}



void PCStopDisplay(void)
{
  display_running = false;
}

void StopDisplay(void)
{
  display_running = false;
  OS_SetPriority(&TCBDisplayMainTask, POWER_DOWN_PRIO);
  OS_SignalEvent(POWER_DOWN_EVENT, &TCBDisplayMainTask);
}

extern "C" void DisplayInit(void)
{
  DisplaySetup();
  StartDisplay();
#ifndef __PC__
  DisplayHwSetup();
#endif

  OS_CREATETASK(&TCBDisplayMainTask, "Display Main Task", DisplayMainTask, DISPLAY_MAIN_TASK_PRIO, DisplayTaskStack);
#ifndef __PC__
  InitKeyboard();
#endif
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
