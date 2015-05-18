/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/* CLASS NAME       : DeactivatableLabel                                    */
/*                                                                          */
/* FILE NAME        : DeactivatableLabel.h                                  */
/*                                                                          */
/* CREATED DATE     : 2009-05-26                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for setting text color to gray while the       */
/* value of a given DataPoint is equal to zero                              */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mrcDeactivatableLabel_h
#define mrcDeactivatableLabel_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <IIntegerDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <Label.h>

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
    *****************************************************************************/
    class DeactivatableLabel : public Label
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      DeactivatableLabel(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~DeactivatableLabel();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual bool Redraw();

      virtual void SubscribtionCancelled(Subject* pSubject);
      virtual void Update(Subject* pSubject);
      virtual void SetSubjectPointer(int id,Subject* pDatapoint);
      virtual void ConnectToSubjects(void);

      // Empty implementation because colour change is not allowed
      virtual void SetColour(U32 Colour, bool forced = false){};
      virtual void SetBackgroundColour(U32 Colour){};

      virtual Leds GetLedsStatus();

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
      SubjectPtr<IIntegerDataPoint*> mpDpActivated;
    };
  } // namespace display
} // namespace mpc

#endif
