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
/* MODULE NAME      : vir_common.h                                          */
/*                                                                          */
/* FILE NAME        : vir_common.h                                          */
/*                                                                          */
/* FILE DESCRIPTION : interface file for vir_common.c                       */
/*                                                                          */
/****************************************************************************/
#ifndef _VIR_COMMON_H
  #define _VIR_COMMON_H

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
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

#if ( CTO_CLASS_9 == Enable )               // buffer for
  extern UCHAR routing_buf[ROUTER_DF_buf_len];     // reroute of PDU's
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_BUF)
#endif

#if ( CTO_CLASS_9 == Enable )               // buffer for
  extern UCHAR routing_status;
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* P R O T O T Y P E S   F O R   E X T E R N A L   F U N C T I O N S        */
/*                                                                          */
/****************************************************************************/
extern UCHAR GetRoutingPDU(UCHAR buffer_len, UCHAR framing_size);
extern void SetRoutingPDU(void);
extern UCHAR CheckHeader(UCHAR own_addr, UCHAR *reply);
extern void AddHeader(UCHAR dest_addr, UCHAR own_addr, UCHAR sd_type);
extern UCHAR CheckCRC(void);
extern void  AddCRC(void);
extern UCHAR CheckTgm(void);
#endif
/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/



