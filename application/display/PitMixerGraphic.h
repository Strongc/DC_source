/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MidRange                                      */
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
/* CLASS NAME       : PitMixerGraphic                                       */
/*                                                                          */
/* FILE NAME        : PitMixerGraphic.h                                     */
/*                                                                          */
/* CREATED DATE     : 2007-09-14                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef _pit_mixer_graphic_h
#define _pit_mixer_graphic_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "SubjectPtr.h"
#include "FloatDataPoint.h"
#include "AppTypeDefs.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "NumberQuantity.h"
#include "Image.h"
#include "Label.h"
#include "AbstractPitGraphic.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for 
    *
    *****************************************************************************/
    class PitMixerGraphic : public AbstractPitGraphic
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      PitMixerGraphic(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~PitMixerGraphic();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void Run();

      virtual bool IsValid();

      virtual void Invalidate(void);

      virtual bool Redraw();

      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* pSubject);

      /* --------------------------------------------------
      * Called if subscription shall be canceled
      * --------------------------------------------------*/
      virtual void SubscribtionCancelled(Subject* pSubject);

      /* --------------------------------------------------
      * Called to set the subject pointer (used by class
      * factory)
      * --------------------------------------------------*/
      virtual void SetSubjectPointer(int Id,Subject* pSubject);

      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      virtual void ConnectToSubjects(void);

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
      /* --------------------------------------------------
      * Calculates and sets ClientAreas of all children
      * --------------------------------------------------*/
      virtual void CalculateClientAreas(void);
      virtual void InvalidatePitContents(void);
      void UpdateStartLevelMixerPosition(void);
      void UpdateStopLevelMixerPosition(void);

      int GetLevelPosition(float MeasuredValue);
      int AdjustPosition(int x1, int y1, float ValueA, float ValueB, NumberQuantity* pBNumQ, Label* pBLabel);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/

      NumberQuantity* mpNQStartLevel1;
      NumberQuantity* mpNQStartLevelMixer;    
      NumberQuantity* mpNQStopLevelMixer;

      Image* mpImgStartLevel1;
      Image* mpImgStartLevelMixer;
      Image* mpImgStopLevelMixer;

      Image* mpImgStartLevel1Ext;
      Image* mpImgStartLevelMixerExt;
      Image* mpImgStopLevelMixerExt;

      Label* mpLStartLevel1;
      Label* mpLStartLevelMixer;    
      Label* mpLStopLevelMixer;

      FloatDataPoint* mpDpStartLevelMixer;

      SubjectPtr<FloatDataPoint*> mpDpStartLevel1;
      SubjectPtr<FloatDataPoint*> mpDpStartLevelOffset;
      SubjectPtr<FloatDataPoint*> mpDpStopLevelMixer;

    };
  } // namespace display
} // namespace mpc

#endif
