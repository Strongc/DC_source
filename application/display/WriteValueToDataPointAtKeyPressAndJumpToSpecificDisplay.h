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
/* CLASS NAME       : WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay                                          */
/*                                                                          */
/* FILE NAME        : WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay.h                                        */
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
#ifndef mpc_displayWriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay_h
#define mpc_displayWriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Component.h"
#include <Observer.h>
#include <DataPoint.h>

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
    * This Class is responsible for at key press to write a given value to a
    * DataPoint and then jump to a given Display
    *
    *****************************************************************************/
    class WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay : public Component, public Observer
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay(Component* pParent = NULL);
			
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~WriteValueToDataPointAtKeyPressAndJumpToSpecificDisplay();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void SetWriteState(int checkState);

      virtual void SubscribtionCancelled(Subject* pSubject);
      virtual void Update(Subject* pSubject);
      virtual void SetSubjectPointer(int id,Subject* pSubject);
      virtual void ConnectToSubjects(void);
      virtual bool HandleKeyEvent(Keys KeyID);

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

      SubjectPtr<IIntegerDataPoint*> mpDataPoint;
      int mCheckState;
    };
  } // namespace display
} // namespace mpc

#endif
