/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:    GENIpro                                       */
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
/* MODULE NAME      :   rs232_drv_l.h                                         */
/*                                                                          */
/* FILE NAME        :   rs232_drv_l.h                                         */
/*                                                                          */
/* FILE DESCRIPTION :   Redirection of h-file                               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "platform.h"

#ifdef D78098X
  #include "K0\rs232_drv_l.h"
#elif defined D78010X
  #include "K0\rs232_drv_l.h"
#elif defined D78011X
  #include "K0\rs232_drv_l.h"
#elif defined D78012X
  #include "K0\rs232_drv_l.h"
#elif defined D78013X
  #include "K0\rs232_drv_l.h"
#elif defined D78014X
  #include "K0\rs232_drv_l.h"
#elif defined D7807XX
  #include "K0\rs232_drv_l.h"
#elif defined D70311X
  #include "V850E\rs232_drv_l_D70311x.h"
#elif defined D7032XX
  #include "V850E\rs232_drv_l_D70_32xx_332x.h"
#elif defined D70332X
  #include "V850E\rs232_drv_l_D70_32xx_332x.h"
#elif defined MIPS
  #include "R4000\rs232_drv_l.h"
#else
  #error "Processor not supported"
#endif

