/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:  GENIPro                                         */
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
/* MODULE NAME      : vir_drv.h                                             */
/*                                                                          */
/* FILE NAME        : vir_drv.h                                             */
/*                                                                          */
/* FILE DESCRIPTION : interface file for vir_drv.c                          */
/*                                                                          */
/****************************************************************************/
#ifndef _VIR_DRV_H
#define _VIR_DRV_H

/****************************************************************************/
/*                                                                          */
/* G L O B A L   D E F I N E M E N T S                                      */
/*                                                                          */
/****************************************************************************/
#define SMALLEST_TGM_SIZE 6

//  geni driver states                                                                       Idle-sense:
enum { GENI_IDLE_WAIT,                           // 0 waiting on next IDLE                   Active
       GENI_RECEIVE_WAIT,                        // 1 waiting on OK geni start condition     Active
       GENI_RECEIVE_HEAD,                        // 2 waiting for complete header            Active
       GENI_RECEIVING,                           // 3 receiving geni tgm                     Active
       GENI_PROCESSING,                          // 4 processing geni tgm                    Inactive
       GENI_TRANSMITTING                         // 5 transmitting geni tgm                  Inactive
     };
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
extern void ResetGeniDriver(UCHAR ch_indx);
extern UCHAR TransmitGeniTgm(UCHAR ch_indx);
extern void InitGeniDriver(UCHAR ch_indx, UCHAR setup_param);
extern void DisableGeniDriver(UCHAR ch_indx);
extern void ResetGeniDriver(UCHAR ch_indx);

/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/
#endif /* _VIR_DRV_H  */
