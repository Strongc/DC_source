/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: SeMon                                            */
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
/* FILE NAME        : pc_geni_dll_if.h                                      */
/*                                                                          */
/* CREATED DATE     : 18-11-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Defines the Geni interface between controller Dll and PC Simulator       */
/****************************************************************************/
#ifndef _pc_geni_dll_if_h
#define _pc_geni_dll_if_h

// geni callback functions
typedef unsigned int (__stdcall *PutGeni)(unsigned char* buffer, unsigned int len); 
typedef unsigned int (__stdcall *GetGeni)(unsigned char* buffer, unsigned int maxCount); 

#endif