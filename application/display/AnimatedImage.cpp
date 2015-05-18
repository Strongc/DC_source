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
/* FILE NAME        : AnimatedImage.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 2011-06-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
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
#include "AnimatedImage.h"
#include "display_task.h" //to get DISPLAY_SAMPLE_TIME


/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
EXTERNS
*****************************************************************************/
namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Constructor
    * DESCRIPTION:
    * Set isHorisontal true when the bitmap has its sub-images in a horisontal sequence
    *****************************************************************************/
    AnimatedImage::AnimatedImage(Component* pParent, bool isHorisontal) : Component(pParent)
    {
      mpBitmap = NULL;
      mCurFrame = 0;
      mRunningDelayInMs = DISPLAY_SAMPLE_TIME * 3;
      mStoppedDelayInMs = DISPLAY_SAMPLE_TIME * 3;

      mSecCounter = 0;
      mIsHorisontal = isHorisontal;
      mNumberOfStoppedImages = 1;
      mAnimationState = ANIMATION_STOPPED;
      
      mpBackgroundBitmap = NULL;
      mBackgroundOffsetX = 0;
      mBackgroundOffsetY = 0;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AnimatedImage::~AnimatedImage()
    {
    }
    
    /*****************************************************************************
    * Function...: SetNumberOfStoppedImages
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::SetNumberOfStoppedImages(int numberOfStoppedImages)
    {
      mNumberOfStoppedImages = numberOfStoppedImages;
    }

    /*****************************************************************************
    * Function...: Start
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::Start(void)
    {
      if (mAnimationState != ANIMATION_USE_SUBJECT)
      {
        mAnimationState = ANIMATION_RUNNING;
      }
    }
    
    
    /*****************************************************************************
    * Function...: Pause
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::Pause(void)
    {
      if (mAnimationState != ANIMATION_USE_SUBJECT)
      {
        mAnimationState = ANIMATION_PAUSED;
      }
    }
    
    /*****************************************************************************
    * Function...: Stop
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::Stop(void)
    {
      int frame_index = -1;
      bool frame_index_changed = false;

      //select first of stopped images
      if (mNumberOfStoppedImages > 0)
      {
        frame_index = GetLastFrameIndex() - mNumberOfStoppedImages + 1;
      }
      else
      {
        frame_index = 0;
      }

      if (mCurFrame != frame_index)
      {
        mCurFrame = frame_index;
        frame_index_changed = true;
      }

      mSecCounter = 0;

      if (mAnimationState != ANIMATION_USE_SUBJECT)
      {
        mAnimationState = ANIMATION_STOPPED;
      }

      if (frame_index_changed)
      {
        Invalidate();
      }
    }

    
    /*****************************************************************************
    * Function...: Reset
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::Reset(void)
    {
      if (IsStarted())
      {
        mCurFrame = 0;
        mSecCounter = 0;
        Invalidate();
      }
      else
      {
        Stop();
      }
    }
    
    /*****************************************************************************
    * Function...: SelectFrame
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::SelectFrame(U16 frameIndex)
    {
      if (frameIndex <= GetLastFrameIndex())
      {
        mCurFrame = frameIndex;
        
        mSecCounter = 0;
      }
    }

    /*****************************************************************************
    * Function...: SetBitmap
    * DESCRIPTION:
    * sets bitmap
    *****************************************************************************/
    void AnimatedImage::SetBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap)
    {
      mpBitmap = pBitmap;
    }
    
    /*****************************************************************************
    * Function...: SetSpeed
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::SetSpeed(int runningDelayInMs, int stoppedDelayInMs) 
    {
      mRunningDelayInMs = runningDelayInMs; 
      mStoppedDelayInMs = stoppedDelayInMs;
    }

    /*****************************************************************************
    * Function...: SetBackgroundBitmap
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::SetBackgroundBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap)
    {
      mpBackgroundBitmap = pBitmap;
    }

    /*****************************************************************************
    * Function...: SetBackgroundOffset
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::SetBackgroundOffset(int x, int y)
    {
      mBackgroundOffsetX = x;
      mBackgroundOffsetY = y;
    }

    /*****************************************************************************
    * Function...: Run
    * DESCRIPTION:
    * Run animation based on started-state
    *****************************************************************************/
    void AnimatedImage::Run()
    {
      mSecCounter += DISPLAY_SAMPLE_TIME;

      bool is_started = IsStarted();
      U16 last_frame_index = GetLastFrameIndex();

      if (is_started)
      {
        if (mSecCounter >= mRunningDelayInMs)
        {
          mSecCounter = 0;
          mCurFrame++;

          if (mNumberOfStoppedImages > 0)
          {
            if (mCurFrame >= last_frame_index - mNumberOfStoppedImages + 1)
            {
              mCurFrame = 0;
            } 
          }
          else
          {
            if (mCurFrame > last_frame_index)
            {
              mCurFrame = 0;
            } 
          }

          Invalidate();
        }
      }
      else // is stopped
      {
        if (mNumberOfStoppedImages > 0)
        {
          if (mSecCounter >= mStoppedDelayInMs)
          {
            mSecCounter = 0;

            int frame_index = mCurFrame + 1;
            
            if (frame_index < last_frame_index - mNumberOfStoppedImages 
              || frame_index > last_frame_index)
            {
              frame_index = last_frame_index - mNumberOfStoppedImages + 1;
            }

            if (mCurFrame != frame_index)
            {
              mCurFrame = frame_index;
              Invalidate();
            }
          }
        }
      }
      Component::Run();
    }

 

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION:
    *****************************************************************************/
    bool AnimatedImage::Redraw()
    {
      int x;
      int y;

      Component::Redraw();

      if (mpBackgroundBitmap != NULL)
      {
        GUI_DrawBitmap(mpBackgroundBitmap, mBackgroundOffsetX, mBackgroundOffsetY);
      }

      if (IsVisible() && mpBitmap != NULL)
      {
        if (mIsHorisontal)
        {
          x = -1 * mCurFrame * GetWidth();
          y = 0;
        }
        else
        {
          x = 0;
          y = -1 * mCurFrame * GetHeight();
        }
        GUI_DrawBitmap(mpBitmap, x, y);          
        
        Validate();
        return true;
      }
      return false;
    }

    
    /*****************************************************************************
    * Function...: IsStarted
    * DESCRIPTION:
    *****************************************************************************/
    bool AnimatedImage::IsStarted()
    {
      bool is_started = false;

      switch (mAnimationState)
      {
      case ANIMATION_USE_SUBJECT:
        if (mpDpStarted.IsValid())
        {
          is_started = (mpDpStarted->IsAvailable() && !mpDpStarted->IsAtMin());
        }
        break;
      case ANIMATION_PAUSED:
      case ANIMATION_STOPPED:
        is_started = false;
        break;
      case ANIMATION_RUNNING:
        is_started = true;
        break;
      }

      return is_started;
    }

    /*****************************************************************************
    * Function...: SetSubjectPointer
    * DESCRIPTION:
    *****************************************************************************/
    void AnimatedImage::SetSubjectPointer(int id, Subject* pSubject)
    { 
      mpDpStarted.Attach(pSubject); 
      mAnimationState = ANIMATION_USE_SUBJECT;
    }

    
    /*****************************************************************************
    * Function...: GetLastFrameIndex
    * DESCRIPTION:
    *****************************************************************************/
    U16 AnimatedImage::GetLastFrameIndex(void)
    {
      U16 last_frame = 0;

      if (mpBitmap != NULL)
      {
        if (mIsHorisontal)
        {
          int w = GetWidth();
          if (w > 0)
          {
            last_frame = (mpBitmap->XSize / w);
          }
        }
        else
        {
          int h = GetHeight();
          if (h > 0)
          {
            last_frame = (mpBitmap->YSize / h);
          }
        }
      }

      if (last_frame != 0)
      {
        last_frame--;
      }
      
      return last_frame;
    
    }

  } // namespace display
} // namespace mpc


