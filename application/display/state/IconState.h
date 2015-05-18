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
/* CLASS NAME       : State                                                 */
/*                                                                          */
/* FILE NAME        : State.h                                               */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text assigned to a state.    */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayICONSTATE_h
#define mpc_displayICONSTATE_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverText.h"
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
    struct StateIconId
    {
      int state;
      GUI_CONST_STORAGE GUI_BITMAP* bmIcon;
    };
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a State string given by a DataPoint
    *
    *****************************************************************************/
    class IconState : public ObserverText
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      IconState(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~IconState();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      void SetBackgroundBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap);
      void SetBackgroundOffset(int x, int y);

      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      /* --------------------------------------------------
      * If the subject is a DataPoint and the quality is
      * DP_NEVER_AVAILABLE this function shall return true
      * --------------------------------------------------*/
      virtual bool IsNeverAvailable();

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
      GUI_CONST_STORAGE GUI_BITMAP* GetIconStateId(int state);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      // Manipulate the mpStates pointer to point to the correct state strings
      // in the constructor of classes speciallizing this class.
      U16 mIconIdCount;
      const StateIconId* mpStateIconIds;
      
      GUI_CONST_STORAGE GUI_BITMAP* mpBackgroundBitmap;
      int mBackgroundOffsetX;
      int mBackgroundOffsetY;
    };
  } // namespace display
} // namespace mpc

#endif
