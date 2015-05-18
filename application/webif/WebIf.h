/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : EthernetCtrl                                          */
/*                                                                          */
/* FILE NAME        : WebIf.h                                               */
/*                                                                          */
/* CREATED DATE     : 22-06-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Web interface functions called by rtip/webpage  */
/****************************************************************************/

/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __WEB_IF_H__
#define __WEB_IF_H__

/*****************************************************************************
  LOCAL INCLUDES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
C FUNCTION PROTOTYPES
*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void webifHttpGetFN_C(const char* szParam, void* pIOContext, char* pBuffer, int maxLength, void (*pCallback)(void* pIOContext, char* pBuffer, int length));
void webifHttpGetCGI_C(const char* szParam, void* pIOContext, char* pBuffer, int maxLength, void (*pCallback)(void* pIOContext, char* pBuffer, int length));
void webifHttpPostCGI_C(const char* szParams, void* pIOContext, char* pBuffer, int maxLength, void (*pCallback)(void* pIOContext, char* pBuffer, int length));

#ifdef __cplusplus
}
#endif

#endif
