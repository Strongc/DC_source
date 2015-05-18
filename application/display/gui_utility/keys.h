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
 #ifndef mpcKEYS_h
 #define mpcKEYS_h

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
#include <gui_utility\leds.h>

 /*****************************************************************************
   DEFINES
  *****************************************************************************/

 /*****************************************************************************
   TYPE DEFINES
  *****************************************************************************/

  typedef enum _keyId
  {
    MPC_NO_KEY            = 0x0000,
    MPC_CONTRAST_KEY      = 0x0001,
    MPC_EX_CONTRAST_KEY   = 0x0002,
    MPC_MENU_KEY          = 0x0004,
    MPC_HELP_KEY          = 0x0008,
    MPC_UP_KEY            = 0x0010,
    MPC_DOWN_KEY          = 0x0020,
    MPC_PLUS_KEY          = 0x0040,
    MPC_PLUS_KEY_REP      = 0x0080,
    MPC_MINUS_KEY         = 0x0100,
    MPC_MINUS_KEY_REP     = 0x0200,
    MPC_ESC_KEY           = 0x0400,
    MPC_OK_KEY            = 0x0800,
    MPC_HOME_KEY          = 0x1000
  } KeyId;


  typedef  U32 Keys;

  int KeyId2Int(KeyId& k);

  KeyId Int2KeyId(int i);

  bool IsFlagInSet(U32 set, U16 flag);
  void SetAllowFlags(U32& set, U16 flags);
  void RemoveAllowFlags(U32& set, U16 flags);
  void SetDenyFlags(U32& set, U16 flags);
  void RemoveDenyFlags(U32& set, U16 flags);
/*
  inline bool IsKeyInSet(Keys kSet, KeyId key) {
      return (kSet & key) != 0;
  }
  //bool IsKeyInSet(Keys kSet, int key); 
  extern void SetKey(Keys& kSet, KeyId key);
  extern void RemoveKey(Keys& kSet, KeyId key);
*/
 #endif
