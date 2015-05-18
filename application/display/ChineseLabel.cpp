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
/* CLASS NAME       : ChineseLabel                                          */
/*                                                                          */
/* FILE NAME        : ChineseLabel.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 2004-09-14                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a ChineseLabel.                       */
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
#include "ChineseLabel.h"
#include <GUI.h>
#include <GUIConf.h>
/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_FONT GB2312_S1112;

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
    ChineseLabel::ChineseLabel(Component* pParent /*=NULL*/) : Label(pParent)
    {
      mpChineseFont = &GB2312_S1112;
      Label::SetFont(&mpChineseFont);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ChineseLabel::~ChineseLabel()
    {
    }

    void ChineseLabel::SetFont(const GUI_FONT** Font)
    {

    }

    
  } // namespace display
} // namespace mpc
