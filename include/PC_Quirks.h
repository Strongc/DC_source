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
 /* FILE NAME        : PC_Quirks.h                                           */
 /*                                                                          */
 /* CREATED DATE     : 2009-09-15                                            */
 /*                                                                          */
 /* SHORT FILE DESCRIPTION :                                                 */
 /* Used to work around compiler bugs in the Visual Studio compiler.         */
 /****************************************************************************/
 /*****************************************************************************
    Protect against multiple inclusion through the use of guards:
  ****************************************************************************/
#ifndef __PC_QUIRKS_H__
#define __PC_QUIRKS_H__

#if (__PC__)


// :COMPILER: Work around weird bug in Visual Studio .NET 2003 SP1
//#define SUBJECT_ID_PUMP_3_OPERATION_TIME_YESTERDAY_LOG (SUBJECT_ID_PUMP_3_OPERATION_TIME_TODAY_LOG + 1)
#define SUBJECT_ID_PUMP_3_RUN_TIME (SUBJECT_ID_PUMP_3_PTC_STATUS + 1)


#endif // __PC__

#endif // __PC_QUIRKS_H__
