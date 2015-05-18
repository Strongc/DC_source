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
/* CLASS NAME       : GetFlashMemory                                        */
/*                                                                          */
/* FILE NAME        : FlashMemory.h                                         */
/*                                                                          */
/* CREATED DATE     : 01-04-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Retreives a pointer to the flash memory         */
/****************************************************************************/

#ifndef FlashMemory_h
#define FlashMemory_h

#include "c1648.h"  // uCPUBusType typedef

#ifdef __cplusplus  
  extern "C" { 
#endif //

  void GetFlashMemory(uCPUBusType** flash, int* sz);

#ifdef __cplusplus    
  }  
#endif // __cplusplus 

#endif // FlashMemory_h