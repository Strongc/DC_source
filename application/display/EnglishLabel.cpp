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
/* CLASS NAME       : EnglishLabel                                          */
/*                                                                          */
/* FILE NAME        : EnglishLabel.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 2004-09-14                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a EnglishLabel.                       */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "EnglishLabel.h"
#include <MpcFonts.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    EnglishLabel::EnglishLabel(Component* pParent /*=NULL*/) : Label(pParent)
    {
      mpEnglishFont = &Helvetica_57_11;
      Label::SetFont(&mpEnglishFont);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    EnglishLabel::~EnglishLabel()
    {
    }

    void EnglishLabel::SetFont(const GUI_FONT** Font)
    {
      if(*Font == &GB2312_S1112)
      {
        Label::SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      }
      else
      {
        Label::SetFont(Font);
      }
    }

    
  } // namespace display
} // namespace mpc
