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
/* CLASS NAME       : keyboard                                              */
/*                                                                          */
/* FILE NAME        : keyboard.cpp                                          */
/*                                                                          */
/* CREATED DATE     : 2004-10-28                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Keyboard driver                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "microp.h"   
#include "vr4181a.h"
#include "keyboard.conf.h"
#include <cu351_cpu_types.h>
#include "rtos.h"
#include "gui.h"
#include "GUI_VNC.h"
/*****************************************************************************
  DEFINES
 *****************************************************************************/

#define KEY_STABLE_VALUE_IN_COUNTS ((KEY_STABLE_VALUE)/(KEYBORAD_SAMPLE_TIME))
#define KEY_REP_VALUE_IN_COUNTS ((KEY_REP_VALUE)/(KEYBORAD_SAMPLE_TIME))
#define KEY_REP_START_VALUE_IN_COUNTS ((KEY_REP_START_VALUE)/(KEYBORAD_SAMPLE_TIME))
#define KEY_REP_NUMBER_IN_COUNTS (KEY_REP_NUMBER-1)
#define KEY_REP_IDLE_IN_COUNTS ((KEY_REP_IDLE_VALUE)/(KEYBORAD_SAMPLE_TIME))
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

U8 KeyboardStack[KEYBOARD_STACK_SIZE];
OS_TASK KeyboardTCB;

void InitKeyboardHw(void)
{
  REGISTER16(KIUWKS_MPC) = 0x001E + 0x0400;
  REGISTER16(KIUWKI_MPC) = 0x01F4;
  REGISTER16(GPMODE0) &= 0xC000;
  REGISTER16(KIUSCANS_MPC) |= 0x03;
  REGISTER16(KIUSCANREP_MPC) = 0x8036;
}

void HandleKey(bool& key, int& keyCounter, bool& rep, bool state)
{
  if (state)
  {
    ++keyCounter;

    if ((keyCounter == KEY_STABLE_VALUE_IN_COUNTS) && (!rep))
    {
      key = true;
    }
    else if ((keyCounter >= KEY_REP_START_VALUE_IN_COUNTS) && (!rep))
    {
      key = true;
      keyCounter = 0;
      rep = true;
    }
    else if ((keyCounter >= KEY_REP_VALUE_IN_COUNTS) && (rep))
    {
      key = true;
      keyCounter = 0;
      rep = true;
    }
  }
  else
  {
    keyCounter = 0;
    key = false;
    rep = false;
  }
}

static bool contrast_key_down;
static U16 keys;

extern "C" void RunKeyboardTask(void)
{
  int  task_timer;
  U16  temp, temp1;
  int  right_arrow_key_counter = 0;
  bool right_arrow_key         = false;
  bool right_arrow_key_rep     = false;
  int  question_key_counter    = 0;
  bool question_key            = false;
  bool question_key_rep        = false;
  int  contrast_key_counter    = 0;
  bool contrast_key            = false;
  bool contrast_key_rep        = false;
  int  plus_key_counter        = 0;
  bool plus_key                = false;
  bool plus_key_rep            = false;
  int  minus_key_counter       = 0;
  bool minus_key               = false;
  bool minus_key_rep           = false;
  int  home_key_counter        = 0;
  bool home_key                = false;
  bool home_key_rep            = false;
  int  up_key_counter          = 0;
  bool up_key                  = false;
  bool up_key_rep              = false;
  int  down_key_counter        = 0;
  bool down_key                = false;
  bool down_key_rep            = false;
  int  ok_key_counter          = 0;
  bool ok_key                  = false;
  bool ok_key_rep              = false;
  int  esc_key_counter         = 0;
  bool esc_key                 = false;
  bool esc_key_rep             = false;
  int  plus_key_rep_number     = 0;
  int  minus_key_rep_number    = 0;
  U16  temp_keys                = 0;
  int  number_of_keys;
  int plus_key_repeated        = 0;
  int minus_key_repeated       = 0;

  InitKeyboardHw();

  while (1)
  {
    task_timer = OS_GetTime();

    temp  = REGISTER16(KIUDAT0_MPC);
    temp1 = REGISTER16(KIUDAT1_MPC);
    temp_keys = 0;
    number_of_keys = 0;

    if (!(temp & 0x0001))      // >
    {
      temp_keys |= 0x0001;
      HandleKey(right_arrow_key,right_arrow_key_counter,right_arrow_key_rep,true);
      number_of_keys++;
    }
    else
    {
      HandleKey(right_arrow_key,right_arrow_key_counter,right_arrow_key_rep,false);
    }

    if (!(temp & 0x0100))     // ?
    {
      temp_keys |= 0x0002;
      HandleKey(question_key,question_key_counter,question_key_rep,true);
      number_of_keys++;
    }
    else
    {
      HandleKey(question_key,question_key_counter,question_key_rep,false);
    }

    if (!(temp & 0x0002))      // +
    {
      temp_keys |= 0x0010;
      HandleKey(plus_key,plus_key_counter,plus_key_rep,true);
      number_of_keys++;

      plus_key_repeated = 0;
    }
    else if (!plus_key_rep || ++plus_key_repeated > KEY_REP_IDLE_IN_COUNTS)
    {
      HandleKey(plus_key,plus_key_counter,plus_key_rep,false);
      plus_key_rep_number = 0;

      plus_key_repeated = 0;
    }

    if (!(temp & 0x0200))      // -
    {
      temp_keys |= 0x0020;
      HandleKey(minus_key,minus_key_counter,minus_key_rep,true);
      number_of_keys++;

      minus_key_repeated = 0;
    }
    else if (!minus_key_rep || ++minus_key_repeated > KEY_REP_IDLE_IN_COUNTS)
    {
      HandleKey(minus_key,minus_key_counter,minus_key_rep,false);
      minus_key_rep_number = 0;

      minus_key_repeated = 0;
    }

    if (!(temp & 0x0004))      // home
    {
      temp_keys |= 0x0100;
      HandleKey(home_key,home_key_counter,home_key_rep,true);
      number_of_keys++;
    }
    else
    {
      HandleKey(home_key,home_key_counter,home_key_rep,false);
    }

    if (!(temp & 0x0400))      // contrast
    {
      temp_keys |= 0x0200;
      HandleKey(contrast_key,contrast_key_counter,contrast_key_rep,true);
      number_of_keys++;
    }
    else
    {
      HandleKey(contrast_key,contrast_key_counter,contrast_key_rep,false);
    }

    if (!(temp1 & 0x0001))      // up
    {
      temp_keys |= 0x0004;
      HandleKey(up_key,up_key_counter,up_key_rep,true);
      number_of_keys++;
    }
    else
    {
      HandleKey(up_key,up_key_counter,up_key_rep,false);
    }

    if (!(temp1 & 0x0100))     // down
    {
      temp_keys |= 0x0008;
      HandleKey(down_key,down_key_counter,down_key_rep,true);
      number_of_keys++;
    }
    else
    {
      HandleKey(down_key,down_key_counter,down_key_rep,false);
    }

    if (!(temp1 & 0x0002))      // esc
    {
      temp_keys |= 0x0040;
      HandleKey(esc_key,esc_key_counter,esc_key_rep,true);
      number_of_keys++;
    }
    else
    {
      HandleKey(esc_key,esc_key_counter,esc_key_rep,false);
    }

    if (!(temp1 & 0x0200))      // ok
    {
      temp_keys |= 0x0080;
      HandleKey(ok_key,ok_key_counter,ok_key_rep,true);
      number_of_keys++;
    }
    else
    {
      HandleKey(ok_key,ok_key_counter,ok_key_rep,false);
    }

    keys = temp_keys;

    if ( number_of_keys == 1 )
    {
      if (contrast_key)
      {
        GUI_StoreKey('c');
        contrast_key = false;
      }
      if (ok_key)
      {
        GUI_StoreKey(GUI_KEY_ENTER);
        ok_key = false;
      }
      if (esc_key)
      {
        GUI_StoreKey(GUI_KEY_ESCAPE);
        esc_key = false;
      }
      if (home_key)
      {
        GUI_StoreKey(GUI_KEY_HOME);
        home_key = false;
      }
      if (right_arrow_key)
      {
        GUI_StoreKey(GUI_KEY_RIGHT);
        right_arrow_key = false;
      }
      if (question_key)
      {
        GUI_StoreKey('?');
        question_key = false;
      }
      if (up_key)
      {
        GUI_StoreKey(GUI_KEY_UP);
        up_key = false;
      }
      if (down_key)
      {
        GUI_StoreKey(GUI_KEY_DOWN);
        down_key = false;
      }
      if (plus_key)
      {
        if ( plus_key_rep )
        {
          if (++plus_key_rep_number > KEY_REP_NUMBER_IN_COUNTS)
          {
            GUI_StoreKey('*');
            plus_key_rep_number =  KEY_REP_NUMBER_IN_COUNTS;
          }
          else
          {
            GUI_StoreKey('+');
          }
        }
        else
        {
          GUI_StoreKey('+');
        }
        plus_key = false;
      }
      if (minus_key)
      {
        if ( minus_key_rep )
        {
          if (++minus_key_rep_number > KEY_REP_NUMBER_IN_COUNTS)
          {
            GUI_StoreKey('/');
            minus_key_rep_number = KEY_REP_NUMBER_IN_COUNTS;
          }
          else
          {
            GUI_StoreKey('-');
          }
        }
        else
        {
          GUI_StoreKey('-');
        }
        minus_key = false;
      }
    }

    if (contrast_key)
    {
      contrast_key_down = true;
    }
    else
    {
      contrast_key_down = false;
    }

    REGISTER16(KIUSCANREP_MPC) = 0x8036;  // new scan start
    OS_DelayUntil (task_timer + KEYBORAD_SAMPLE_TIME);
  }
}

void InitKeyboard(void)
{
  contrast_key_down = false;
  OS_CREATETASK(&KeyboardTCB,"keyboard Driver",RunKeyboardTask,KEYBORAD_PRIORITY,KeyboardStack);
}

U16 GetKeyStates(void)
{
  return keys;
}

bool GetContrastKeyState(void)
{
  return contrast_key_down;
}

