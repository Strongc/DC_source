/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       :                                                       */
/*                                                                          */
/* FILE NAME        : MPCUNITS.CONF.H                                       */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcMPCUNIT_CONF_h
#define mpcMPCUNIT_CONF_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <UnitTypes.h>
#include "mpcunits.h"
#include <string_id.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define UNIT_TEXT_LEN 20
#define NO_OF_UNITS 20

 typedef struct
  {
    float ScaleFactor;
    float Offset;
    STRING_ID UnitTextId;
  } UNIT;

  typedef  struct
  {
    QUANTITY_TYPE Quantity;
    U8 DefaultSiUnit;
    U8 DefaultUsUnit;
    U8 ActualUnit;
    U8 LastUnit;
    UNIT FromStandardToUnit[NO_OF_UNITS];
    UNIT FromUnitToStandard[NO_OF_UNITS];
  } QUANTITY;

extern QUANTITY MpcUnitTabel[Q_LAST_UNIT];

#endif
