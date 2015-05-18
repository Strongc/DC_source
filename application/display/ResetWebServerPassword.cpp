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
/* CLASS NAME       : ReseteWebServerPassword                                                  */
/*                                                                          */
/* FILE NAME        : ReseteWebServerPassword.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for at key press to write a given value to a   */
/* DataPoint and then jump to a given Display                               */
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
#include "DisplayController.h"
#include "ResetWebServerPassword.h"
#ifdef __PC__
#include <PC/MPCWebServer.h>
#else
#include <MPCWebServer.h>
#endif
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
    ResetWebServerPassword::ResetWebServerPassword(Component* pParent) : Component(pParent)
    {
      mLegalKeys = MPC_OK_KEY;
      mLedsStatus = COMBINE(NO_LED,OK_LED);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ResetWebServerPassword::~ResetWebServerPassword()
    {
    }

    bool ResetWebServerPassword::HandleKeyEvent(Keys KeyID)
    {
      switch(KeyID)
      {
        case MPC_OK_KEY:
          MPCWebServer::getInstance()->resetPassword();
          return true;
        case MPC_ESC_KEY:
          DisplayController::GetInstance()->Pop();
          return true;
      }
      return false;
    }

  } // namespace display
} // namespace mpc


