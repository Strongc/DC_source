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
/* FILE NAME        : C_BaseDirectory.h                                     */
/*                                                                          */
/* CREATED DATE     : 29-05-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Used by PMCR.c to initialize the BaseDirectory (singleton)               */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_C_BaseDirectory_h
#define mrc_C_BaseDirectory_h

#ifdef __cplusplus
  extern "C" {
#endif

void C_SetBaseDirectory(const char* BaseDirectory);

#ifdef __cplusplus
  }
#endif


#endif
