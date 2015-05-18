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
/* CLASS NAME       : StatusGraphics                                        */
/*                                                                          */
/* FILE NAME        : StatusGraphics.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a StatusGraphics.              */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "StatusGraphics.h"
#include "DataPoint.h"
#include <FactoryTypes.h>
#include <MPCFonts.h>
/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
EXTERNS
*****************************************************************************/

#ifdef __PC__
  extern  bool  g_is_calculating_strings;
#endif

namespace mpc
{
  namespace display
  {

  int get_the_pos_from_index[SENSOR_COUNT] =
  {
    1, // pos 0  --
    1, // pos 1  --
    1, // pos 2  --
    1, // pos 3  system flow
    1, // pos 4  --
    2, // pos 5  remote pressure
    1, // pos 6  system pressure
    0, // pos 7  system pre-pressure
    0, // pos 8  system statik pressure
    2, // pos 9  system remote diff pressure 1
    2, // pos 10 system remote diff pressure 2
    2, // pos 11 system remote diff pressure 3
    0, // pos 12 tank pre pressure
    1, // pos 13 system diff pressure
    0, // pos 14 system in diff pressure
    2, // pos 15 system out diff pressure
    2, // pos 16 system level
    0, // pos 17 feed tank level
    2, // pos 18 remote temperature
    0, // pos 19 forward temperature
    1, // pos 20 return temperature
    1, // pos 21 diff temperature
    0,
    0,
    0,
    0,
    0,
    0
  };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    StatusGraphics::StatusGraphics(Component* pParent) : Frame(true,true,pParent)
    {
      // create and add info field as a chield
      mpInfoField = new InfoFieldObserverGroup();
      mpInfoField->SetVisible(true);
      mpInfoField->SetTransparent(true);
      mpInfoField->SetHeight(30);
      mpInfoField->SetWidth(240);
      AddChild(mpInfoField);
      
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    StatusGraphics::~StatusGraphics()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    void StatusGraphics::Run()
    {
      Group::Run();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool StatusGraphics::Redraw()
    {
      if(mValid)
        return true;


      // Reset subject update bit
      mUpdated = false;
      mPumpSystemModeStateUpdated = false;
      mPumpOperationModeStateUpdated = false;

      return true;
    }

    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void StatusGraphics::Update(Subject* pSubject)
    {
      mUpdated = true;
    }

    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void StatusGraphics::SubscribtionCancelled(Subject* pSubject)
    {
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void StatusGraphics::SetSubjectPointer(int id,Subject* pSubject)
    {
      switch(id)
      {

        case SP_SG_INFO_FIELD_REMOTE_BUS_ACTIVE:
        case SP_SG_INFO_FIELD_REMOTE_SERVICE_ACTIVE:
        case SP_SG_INFO_FIELD_REMOTE_VNC_ACTIVE:
          mpInfoField->SetSubjectPointer(id,pSubject);
          break;

      }
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void StatusGraphics::ConnectToSubjects(void)
    {
    }
  } // namespace display
} // namespace mpc


