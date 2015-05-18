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
/* MODULE NAME      : vir_tab_master.h                                      */
/*                                                                          */
/* FILE NAME        : vir_tab_master.h                                      */
/*                                                                          */
/* FILE DESCRIPTION : interface file for vir_tab_master.c                   */
/*                                                                          */
/****************************************************************************/
#ifndef _VIR_TAB_MASTER_H
  #define _VIR_TAB_MASTER_H

/****************************************************************************/
/*                                                                          */
/* G L O B A L   D E F I N E M E N T S                                      */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* G L O B A L    C O N S T A N T S                                         */
/*                                                                          */
/****************************************************************************/
// Definition of response func
typedef void ( *DIR_RESP_FNC)(void);

//  Definition of the group table record
typedef struct {
  UCHAR cl;
  UCHAR operation;
  UCHAR priority;
  UCHAR size;
  UCHAR *grp_list_pnt;
  GENI_DEVICE_TYPE device_types;
} GROUP_TAB_TYPE;

extern const ID_PTR *mas_tab[17];
extern const UCHAR nof_bytes_class[17];
extern const GROUP_TAB_TYPE group_table[MAS_MAX_NO_GROUPS];
extern const GROUP_TAB_TYPE static_apdu_tgm[MAS_MAX_NO_GROUPS];
extern const DIR_RESP_FNC dir_resp_table[MAS_LAST_RESP_FNC+1];

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
#endif
/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/

