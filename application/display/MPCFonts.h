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
/* CLASS NAME       : MPCFonts                                                      */
/*                                                                          */
/* FILE NAME        : MPCFonts.h                                                      */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcMPCFonts_h
#define mpcMPCFonts_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <GUI.h>
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

extern "C" GUI_CONST_STORAGE GUI_FONT Helvetica_57_11;
extern "C" GUI_CONST_STORAGE GUI_FONT Helvetica_57_13;
extern "C" GUI_CONST_STORAGE GUI_FONT Helvetica_57_18;
extern "C" GUI_CONST_STORAGE GUI_FONT GB2312_S1112;

extern "C" GUI_CONST_STORAGE GUI_FONT *language_dep_font_11;
extern "C" GUI_CONST_STORAGE GUI_FONT *language_dep_font_13;
extern "C" GUI_CONST_STORAGE GUI_FONT *language_dep_font_18;

extern "C" GUI_CONST_STORAGE GUI_FONT *language_indep_font_11;
extern "C" GUI_CONST_STORAGE GUI_FONT *language_indep_font_13;
extern "C" GUI_CONST_STORAGE GUI_FONT *language_indep_font_18;

#define DEFAULT_FONT_11_LANGUAGE_DEP &language_dep_font_11
#define DEFAULT_FONT_13_LANGUAGE_DEP &language_dep_font_13
#define DEFAULT_FONT_18_LANGUAGE_DEP &language_dep_font_18

#define DEFAULT_FONT_11_LANGUAGE_INDEP &language_indep_font_11
#define DEFAULT_FONT_13_LANGUAGE_INDEP &language_indep_font_13
#define DEFAULT_FONT_18_LANGUAGE_INDEP &language_indep_font_18


#endif

