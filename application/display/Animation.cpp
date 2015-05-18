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
/* CLASS NAME       : CheckBox                                                  */
/*                                                                          */
/* FILE NAME        : CheckBox.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a CheckBox.                        */
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
#include "Animation.h"
#include <algorithm>
#include <display_task.h>

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
    Animation::Animation(Component* pParent) : Image(pParent)
    {
      mCurFrame = 0;
      mSecCounter = 0;
      mAnimationState = STARTED;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    Animation::~Animation()
    {
      RemoveAllBitmaps();
    }


    void Animation::AddBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap, int delay)
    {
      AniBmps* p_bmp = new AniBmps();
      p_bmp->pBitmap = pBitmap;
      p_bmp->delay = delay;

      mAnimationBitmaps.push_back(p_bmp);
      Invalidate();
    }

    void Animation::RemoveBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap)
    {
      std::vector<AniBmps*>::iterator iter = mAnimationBitmaps.begin();
      for(;iter != mAnimationBitmaps.end();++iter)
      {
        if((*iter)->pBitmap == pBitmap)
        {
          mAnimationBitmaps.erase(iter);
          delete (*iter);
          iter = mAnimationBitmaps.begin();
          Invalidate();
        }
      }
    }

    void Animation::RemoveAllBitmaps()
    {
      std::vector<AniBmps*>::iterator iter = mAnimationBitmaps.begin();
      for(;iter != mAnimationBitmaps.end();++iter)
      {
        delete (*iter);
      }
      mAnimationBitmaps.clear();
    }

    void Animation::SetDelay(int ms)
    {
      std::vector<AniBmps*>::iterator iter = mAnimationBitmaps.begin();
      for(;iter != mAnimationBitmaps.end();++iter)
      {
        (*iter)->delay = ms;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    void Animation::Run()
    {
      if(mAnimationState == STARTED)
      {
        mSecCounter += DISPLAY_SAMPLE_TIME;
        std::vector<AniBmps*>::iterator iter = (mAnimationBitmaps.begin() + mCurFrame);
        if( iter < mAnimationBitmaps.end() )
        {
          AniBmps* p = *iter;
          // Skip pictures if delay time is smaller than DISPLAY_SAMPLE_TIME
          while(p->delay <= mSecCounter)
          {
            mSecCounter -= p->delay;
            ++mCurFrame;
            Goto(mCurFrame);
            p = *(mAnimationBitmaps.begin() + mCurFrame);
          }
        }
      }
      Image::Run();
    }

    bool Animation::Redraw()
    {
      ClearArea();
      return Image::Redraw();
    }

    bool Animation::Start()
    {
      mAnimationState = STARTED;
      return false;
    }

    bool Animation::Pause()
    {
      mAnimationState = PAUSED;
      return false;
    }

    bool Animation::Stop()
    {
      mCurFrame = 0;
      mSecCounter = 0;
      mAnimationState = STOPPED;
      Invalidate();
      return false;
    }


    bool Animation::Goto(U16 frame)
    {
      mSecCounter = 0;
      if(frame >= mAnimationBitmaps.size())
      {
        mCurFrame = 0;

      }
      else
      {
        mCurFrame = frame;
      }
      SetBitmap((*(mAnimationBitmaps.begin() + mCurFrame))->pBitmap);
      Invalidate();
      return true;
    }
  } // namespace display
} // namespace mpc


