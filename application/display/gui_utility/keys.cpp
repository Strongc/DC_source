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
 /* CLASS NAME       : Keys                                                  */
 /*                                                                          */
 /* FILE NAME        : KEYS.H                                                */
 /*                                                                          */
 /* CREATED DATE     : 2004-08-30                                            */
 /*                                                                          */
 /* SHORT FILE DESCRIPTION :                                                 */
 /****************************************************************************/
 /*****************************************************************************
    Protect against multiple inclusion through the use of guards:
  ****************************************************************************/

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
 #include <keys.h>
 #include "gui.h"
 /*****************************************************************************
   DEFINES
  *****************************************************************************/

 /*****************************************************************************
   TYPE DEFINES
  *****************************************************************************/

  int KeyId2Int(KeyId k)
  {
    switch (k)
    {
      case MPC_CONTRAST_KEY      : return 'c';
      case MPC_EX_CONTRAST_KEY   : return 'C';

      case MPC_MENU_KEY          : return GUI_KEY_RIGHT;
      case MPC_UP_KEY            : return GUI_KEY_UP;
      case MPC_DOWN_KEY          : return GUI_KEY_DOWN;
      case MPC_HOME_KEY          : return GUI_KEY_HOME;
      case MPC_HELP_KEY          : return '?';
      case MPC_PLUS_KEY          : return '+';
      case MPC_PLUS_KEY_REP      : return '*';
      case MPC_MINUS_KEY         : return '-';
      case MPC_MINUS_KEY_REP     : return '/';
      case MPC_ESC_KEY           : return GUI_KEY_ESCAPE;
      case MPC_OK_KEY            : return GUI_KEY_ENTER;
      default                    : return 0;
    }
  }


  KeyId Int2KeyId(int i)
  {
    switch (i)
    {
      case 'c'            : return MPC_CONTRAST_KEY;
      case 'C'            : return MPC_EX_CONTRAST_KEY;
      case GUI_KEY_RIGHT  : return MPC_MENU_KEY;
      case GUI_KEY_UP     : return MPC_UP_KEY;
      case GUI_KEY_DOWN   : return MPC_DOWN_KEY;
      case GUI_KEY_HOME   : return MPC_HOME_KEY;
      case '?'            : return MPC_HELP_KEY;
      case '+'            : return MPC_PLUS_KEY;
      case '*'            : return MPC_PLUS_KEY_REP;
      case '-'            : return MPC_MINUS_KEY;
      case '/'            : return MPC_MINUS_KEY_REP;
      case GUI_KEY_ESCAPE : return MPC_ESC_KEY;
      case GUI_KEY_ENTER  : return MPC_OK_KEY;
      default             : return MPC_NO_KEY;
    }
  }

/*
  bool IsKeyInSet(Keys kSet, KeyId key)
  {
    return (kSet & key) != 0;
  }
*/
/*
  bool IsKeyInSet(Keys kSet, int key)
  {
    KeyId key_id = Int2KeyId(key);

    return (kSet & key_id) != 0;
  }
*/
/*
  bool IsKeyInSet(Keys kSet, int key)
  {
    return (kSet & (KeyId)key) != 0;
  }
*/

  bool IsFlagInSet(U32 set, U16 flag)
  {
    U16 on  = set & 0x0000FFFF;
    U16 off = set >> 0x10;

    return ((on & ~off) & flag) != 0;
  }


  void SetAllowFlags(U32& set, U16 flags)
  {
    set |= flags;
  }

  void RemoveAllowFlags(U32& set, U16 flags)
  {
    set &= (~flags) | 0xFFFF0000;
  }

  void SetDenyFlags(U32& set, U16 flags)
  {
    U32 off = flags << 0x10;
    set = set | off;
  }

  void RemoveDenyFlags(U32& set, U16 flags)
  {
    set &= (~(flags << 0x10)) | 0x0000FFFF;
  }

  
