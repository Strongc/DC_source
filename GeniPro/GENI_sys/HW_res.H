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
/* MODULE NAME      :  -                                                    */
/*                                                                          */
/* FILE NAME        :  Hw_res.h                                             */
/*                                                                          */
/* FILE DESCRIPTION :  Redirection of HW_res.h                              */
/*                                                                          */
/****************************************************************************/

#include "platform.h"

#ifdef D78098X
  #include "D78098x\hw_res.h"
#elif defined D70311X
  #include "D70311x\hw_res.h"
#elif defined D7032XX
  #include "D7032xx\hw_res.h"
#elif defined D70332X
  #include "D70332x\hw_res.h"
#elif defined D78010X
  #include "D7801xx\hw_res.h"
#elif defined D78012X
  #include "D7801xx\hw_res.h"
#elif defined D78013X
  #include "D7801xx\hw_res.h"
#elif defined D78014X
  #include "D7801xx\hw_res.h"
#elif defined D7807XX
  #include "D7807xx\hw_res.h"
#elif defined MIPS
  #include "D3018x\hw_res.h"
#else
	#error "Processor not supported"
#endif

/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/
