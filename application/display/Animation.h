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
/* CLASS NAME       : Animation                                                 */
/*                                                                          */
/* FILE NAME        : Animation.h                                               */
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
#ifndef mpc_displayAnimation_h
#define mpc_displayAnimation_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <vector>
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Image.h>
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
#define  STARTED 0
#define  PAUSED  1
#define  STOPPED 2

namespace mpc
{
  namespace display
  {
    struct AniBmps
    {
      GUI_CONST_STORAGE GUI_BITMAP* pBitmap;
      int delay;
    };

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a State string given by a DataPoint
    *
    *****************************************************************************/
    class Animation : public Image
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      Animation(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~Animation();
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
      virtual bool Start();
      virtual bool Pause();
      virtual bool Stop();
      virtual bool Goto(U16 frame);

      virtual void Run();
      virtual void AddBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap, int delay);
      virtual void RemoveBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap);
      virtual void RemoveAllBitmaps();
      virtual void SetDelay(int ms);
      virtual bool Redraw();
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
      int mCurFrame;
      int mSecCounter;
      std::vector<AniBmps*> mAnimationBitmaps;
      int mAnimationState;
    };
  } // namespace display
} // namespace mpc

#endif
