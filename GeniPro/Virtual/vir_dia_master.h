/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project: GENIPro                                          */
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
/* MODULE NAME      : vir_dia_master.h                                      */
/*                                                                          */
/* FILE NAME        : vir_dia_master.h                                      */
/*                                                                          */
/* FILE DESCRIPTION : interface file for vir_dia_master.c                   */
/*                                                                          */
/****************************************************************************/
#ifndef _VIR_DIA_MASTER_H
  #define _VIR_DIA_MASTER_H

#include <typedef.h>    /*definements for the Grundfos common types */
#include "geni_if.h"    /* Access to Geni interface                 */

/****************************************************************************/
/*                                                                          */
/* G L O B A L   D E F I N E M E N T S                                      */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

  extern UCHAR group_active;                                    // TRUE if request of group is going on else FALSE

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* P R O T O T Y P E S   F O R   E X T E R N A L   F U N C T I O N S        */
/*                                                                          */
/****************************************************************************/
extern void InitMultiPointDia(void);
extern void DisableMultiPointDia(void);
extern void ResetMultiPointDia(void);

extern void SendAutoRequest(void);                              //  Action function
extern void SendDirRequest(void);                               //  Action function
extern void ProcessAutoRequest(void);                           //  Action function
extern void ProcessDirRequest(void);                            //  Action function
extern void NoReplyDirRequest(void);                            //  Action function
extern void NoReplyAutoRequest(void);                           //  Action function
extern void FindNextWaitingGroup(void);                         //  Action function
extern void GotoNextAutoRequest(void);                          //  Action function
extern void FirstRequestInGroup(void);                          //  Action function
extern void CheckPending(void);                                 //  Action function
extern UCHAR SendWaitingDirectTgm(void );                       //  Action function

#endif
/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/



