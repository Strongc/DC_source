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
/* MODULE NAME      : SlaveData                                             */
/*                                                                          */
/* FILE NAME        : SlaveData.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 01-04-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Global variables used to hold geni slave data   */
/* Only used by GENI sw                                                     */
/****************************************************************************/
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
#include "SlaveData.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  C functions declarations
 *****************************************************************************/

/*****************************************************************************
  Global variables
 *****************************************************************************/
U8 s_cl2_id020[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id021[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id022[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id023[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id024[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id025[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id026[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id027[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id029[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id030[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id032[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id033[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id034[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id035[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id036[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id043[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id044[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id045[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id046[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id047[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id048[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id049[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id050[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id060[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id064[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id065[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id066[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id067[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id068[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id069[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id070[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id071[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id072[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id081[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id083[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id086[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id094[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id095[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id099[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id141[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id142[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id144[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id145[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id146[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id147[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id148[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id149[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id151[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id152[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id153[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id156[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id158[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id195[MAX_NUMBER_OF_UNITS];
U8 s_cl2_id196[MAX_NUMBER_OF_UNITS];

U8 s_cl4_id030[MAX_NUMBER_OF_UNITS];

U8 s_cl5_id001[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id002[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id003[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id007[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id008[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id009[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id010[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id011[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id012[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id013[MAX_NUMBER_OF_UNITS];
U8 s_cl5_id014[MAX_NUMBER_OF_UNITS];

U8 s_cl2_id027_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE];
U8 s_cl2_id030_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE];
U8 s_cl2_id032_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE];
U8 s_cl2_id033_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE];
U8 s_cl2_id035_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE];
U8 s_cl2_id045_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE];
U8 s_cl2_id086_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE];
U8 s_cl2_id151_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE];


/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/

