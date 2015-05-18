/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/*                                                                          */
/* FILE NAME        : C_UseStringsFromExcel.h                               */
/*                                                                          */
/* CREATED DATE     : 02-06-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_C_UseStringsFromExcel_h
#define mrc_C_UseStringsFromExcel_h

#ifdef __cplusplus
  extern "C" {
#endif

void C_UseStringsFromExcel(char useExcel);

#ifdef __cplusplus
  }
#endif


#endif
