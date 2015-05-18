/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:                                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos Electronics A/S, 2000               */
/*                                                                          */
/*                            All rights reserved                           */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* MODULE NAME      : vir_ctr_master.h                                      */
/*                                                                          */
/* FILE NAME        : vir_ctr_master.h                                      */
/*                                                                          */
/* FILE DESCRIPTION : interface file for vir_ctr_master.c                   */
/*                                                                          */
/****************************************************************************/
#ifndef _VIR_CTR_MASTER_H
  #define _VIR_CTR_MASTER_H

#include <typedef.h>            /*definements for the Grundfos common types */

/****************************************************************************/
/*                                                                          */
/* G L O B A L   D E F I N E M E N T S                                      */
/*                                                                          */
/****************************************************************************/
// Definition of states
enum { sSLAVE_MODE, // 0
       sREADY,      // 1
       sBUILD_TGM,  // 2
       sDIR_REQ,    // 3
       sAUTO_REQ,   // 4
       sMROUTING,   // 5
       sWAIT_REPLY, // 6
       sDONT_CARE}; // 7

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

  extern UCHAR ma_state;                    // intercom control state
  extern UINT all_master_errors;            // total number of errors

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* P R O T O T Y P E S   F O R   E X T E R N A L   F U N C T I O N S        */
/*                                                                          */
/****************************************************************************/
extern void MasterControlHandler(UCHAR event);
extern void InitMaster(void);
extern void DisableMaster(void);
extern void ResetMaster(void);
extern void CheckWaitingRoutePoll(void);    //  Sub function
/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/
#endif




