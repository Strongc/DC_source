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
/* FILE NAME        : utf8_util.h                                           */
/*                                                                          */
/* CREATED DATE     : 17-06-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Convert between UTF-8 and SMS format (either GSM 3.38 or UCS-2)          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_utf8_util_h
#define mrc_utf8_util_h

typedef unsigned char        GF_UINT1;
typedef unsigned char        GF_UINT8;
typedef unsigned short int   GF_UINT16;
typedef unsigned long int    GF_UINT32;
#define TRUE 1
#define FALSE 0

//#include <typedef.h>
#include "cu351_cpu_types.h"

#include "ConvertUTF.h"

#ifdef __cplusplus
  extern "C" {
#endif

GF_UINT16 ConvertUtf8ToSms(const char* pSrc, char* pDest, GF_UINT16 maxNoOfBytes, GF_UINT1* pIsDestGsm338);
GF_UINT16 ConvertSmsToUtf8(const char* pSrc, char* pDest, GF_UINT16 maxNoOfBytes, GF_UINT1 sourceIsAsciiHexValues);

void Gsm338test(void);

#ifdef __cplusplus
  }
#endif


#endif
