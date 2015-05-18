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
/* CLASS NAME       : AvalibleIfSet                                          */
/*                                                                          */
/* FILE NAME        : AvalibleIfSet.h                                        */
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
#ifndef mpc_displayAvalibleIfSet_h
#define mpc_displayAvalibleIfSet_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Observer.h>
#include <IIntegerDataPoint.h>
#include <vector>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <CheckBox.h>

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
    * This Class is responsible for how to show a State string given by a DataPoint
    *
    *****************************************************************************/
    class AvalibleIfSet : public CheckBox, public Observer
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      AvalibleIfSet(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~AvalibleIfSet();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual bool Redraw();
      virtual bool SetCheck(bool check = true);
      
      /*
      * Sets the state this checkbox is associated with
      */
      virtual void SetCheckState(int checkState);
      virtual void AddCheckState(int checkState);

      virtual void SubscribtionCancelled(Subject* pSubject);
      virtual void Update(Subject* pSubject);
      virtual void SetSubjectPointer(int id,Subject* pDatapoint);
      virtual void ConnectToSubjects(void);
      virtual bool IsNeverAvailable();
      virtual bool IsValid();
      virtual bool HandleKeyEvent(Keys KeyID);

      virtual void Invert(bool = true);

      /* --------------------------------------------------
      * Gets the LEDs which this element and the ones below, wants to be on
      * or off. Normaly this is also the keys to react, but it doesn't have
      * to be this way ...
      * --------------------------------------------------*/
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

      SubjectPtr<IIntegerDataPoint*> mpDataPoint;
      
      bool mInverted;
      std::vector<int> mCheckStates; 

    };
  } // namespace display
} // namespace mpc

#endif
