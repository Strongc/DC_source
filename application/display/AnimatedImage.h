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
/* CLASS NAME       : AnimatedImage                                         */
/*                                                                          */
/* FILE NAME        : AnimatedImage.h                                       */
/*                                                                          */
/* CREATED DATE     : 2011-06-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef _AnimatedImage_h
#define _AnimatedImage_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "IMinMaxDataPoint.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Component.h"
#include "Observer.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
typedef enum
{
  ANIMATION_RUNNING,
  ANIMATION_PAUSED,
  ANIMATION_STOPPED,
  ANIMATION_USE_SUBJECT
}ANIMATION_STATE_TYPE;

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
    * This Class is responsible for showing flow in a pipeline section
    *
    *****************************************************************************/
    class AnimatedImage : public Component, public Observer
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      AnimatedImage(Component* pParent = NULL, bool isHorisontal = true);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~AnimatedImage();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void SetNumberOfStoppedImages(int numberOfStoppedImages);
      virtual void Start(void);
      virtual void Pause(void);
      virtual void Stop(void);
      virtual void Reset(void);
      virtual void SelectFrame(U16 frameIndex);

      virtual void Run(void);
      virtual bool Redraw(void);
      virtual bool IsStarted(void);

      void SetBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap);
      void SetSpeed(int runningDelayInMs, int stoppedDelayInMs);

      void SetBackgroundBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap);
      void SetBackgroundOffset(int x, int y);

      virtual void SubscribtionCancelled(Subject* pSubject){}
      virtual void Update(Subject* pSubject) {}
      virtual void SetSubjectPointer(int id, Subject* pSubject);
      virtual void ConnectToSubjects(void) {}
      
    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      U16 GetLastFrameIndex(void);
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
      bool mIsHorisontal;
      int mNumberOfStoppedImages;
      int mCurFrame;
      int mSecCounter;
      int mRunningDelayInMs;
      int mStoppedDelayInMs;

      int mBackgroundOffsetX;
      int mBackgroundOffsetY;

      GUI_CONST_STORAGE GUI_BITMAP* mpBitmap;
      GUI_CONST_STORAGE GUI_BITMAP* mpBackgroundBitmap;

      ANIMATION_STATE_TYPE mAnimationState;

      SubjectPtr<IMinMaxDataPoint*> mpDpStarted;

    };

  } // namespace display
} // namespace mpc

#endif
