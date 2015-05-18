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
/* CLASS NAME       : ObserverText                                          */
/*                                                                          */
/* FILE NAME        : ObserverText.h                                        */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayObserverText_h
#define mpc_displayObserverText_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Text.h"
#include "Observer.h"
#include "Subject.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define MAX_STATE_TEXT_LENGTH 81
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
    *****************************************************************************/
    class ObserverText : public Text, public Observer
    {
    public:
      ObserverText(Component* pParent = NULL);
      virtual ~ObserverText();
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* Object);
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
      * Returns the pointer to the Subject. 0 if no subject
      * is defined
      * --------------------------------------------------*/
      virtual Subject* GetSubject();
      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      virtual void ConnectToSubjects(void);

      /* --------------------------------------------------
      * If the subject is a DataPoint and the quality is
      * DP_NEVER_AVAILABLE this function shall return true
      * I'm a little lazzy, so I implemented a default ver
      * of the function in ObserverText that return false.
      * The correct design would be to make it pure virtual.
      * --------------------------------------------------*/
      virtual bool IsNeverAvailable();

      /* --------------------------------------------------
      * Returns whether this element is readable/writeable
      * --------------------------------------------------*/
      virtual bool IsReadOnly();

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
      SubjectPtr<Subject*> mpSubject;  // Holds the pointer to the DataPoint (if any)
      bool mSubjectSubscribed;         // Holds if a subscription has been made
      bool mDPWritable;
    };
  } // namespace display
} // namespace mpc

#endif
