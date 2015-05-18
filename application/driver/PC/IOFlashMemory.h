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
/* CLASS NAME       : ReadFlashMemory, WriteFlashMemory                     */
/*                                                                          */
/* FILE NAME        : IOFlashMemory.h                                       */
/*                                                                          */
/* CREATED DATE     : 01-04-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Reading and writing flashmemory from/to disk.   */
/****************************************************************************/

#ifndef IOFlashMemory_h
#define IOFlashMemory_h

#ifdef __cplusplus  
  extern "C" { 
#endif //

  void ReadFlashMemory(const wchar_t* flashFileName);
  void WriteFlashMemory();
  void RenameFlashMemory();

#ifdef __cplusplus    
  }  
#endif // __cplusplus 
#endif //IOFlashMemory_h