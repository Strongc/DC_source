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
/* CLASS NAME       : MenuBar                                               */
/*                                                                          */
/* FILE NAME        : MenuBar.h                                             */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text assigned to a MenuBar.  */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayMenuBar_h
#define mpc_displayMenuBar_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Group.h"
#include "Frame.h"
#include "MenuTab.h"

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
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a MenuBar string given by a DataPoint
    *
    *****************************************************************************/
    class MenuBar : public Group
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      MenuBar(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~MenuBar();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      /* --------------------------------------------------
      * Adds a tab to this window.
      * --------------------------------------------------*/
      virtual int AddMenuTab(MenuTab* tab);

      /* --------------------------------------------------
      * Activates a tab.
      * --------------------------------------------------*/
      virtual bool SetActiveTab(MenuTab* tab);
      virtual bool SetActiveTab(int index);
      virtual int GetActiveTabIndex(void);

      /* --------------------------------------------------
      * Gets the LEDs which this element and the ones below, wants to be on
      * or off. Normaly this is also the keys to react, but it doesn't have
      * to be this way ...
      * --------------------------------------------------*/
      virtual Leds GetLedsStatus();

      /* --------------------------------------------------
      * Gets the keys to which we should react in this element and the
      * elements below. Thereby this is the element to send key messages
      * to for the given keys.
      * --------------------------------------------------*/
      virtual Keys GetLegalKeys();

      /* --------------------------------------------------
      * Handle this key event. Return true if it is taken care of
      * --------------------------------------------------*/
      virtual bool HandleKeyEvent(Keys KeyID);


      void Run();

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    };
  } // namespace display
} // namespace mpc

#endif
