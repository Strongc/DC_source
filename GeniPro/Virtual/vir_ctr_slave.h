/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project: GeniPro                                          */
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
/* MODULE NAME      : vir_ctr_slave.h                                       */
/*                                                                          */
/* FILE NAME        : vir_ctr_slave.h                                       */
/*                                                                          */
/* FILE DESCRIPTION : Interface to vir_ctr_slave.c                          */
/*                                                                          */
/****************************************************************************/

#ifndef _VIR_CTR_SLAVE_H
  #define _VIR_CTR_SLAVE_H

#include <typedef.h>            /*definements for the Grundfos common types */

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
/****************************************************************************/
/*                                                                          */
/* P R O T O T Y P E S   F O R   E X T E R N A L   F U N C T I O N S        */
/*                                                                          */
/****************************************************************************/
extern void InitSlave(void);
extern void DisableSlave(void);
extern void ResetSlave(void);

extern void SlaveControlHandler(UCHAR event);
extern void ProcesGeniTgm(void);
extern void SendSlaveReply(void);
extern void SlaveConnectTmOut(void);
extern void SlaveDontReply(void);
#endif
/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/



