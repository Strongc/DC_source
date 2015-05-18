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
/* CLASS NAME       : ContrastMessageBox                                    */
/*                                                                          */
/* FILE NAME        : ContrastMessageBox.h                                  */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayContrastMessageBox_h
#define mpc_displayContrastMessageBox_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <Observer.h>
#include <U32DataPoint.h>
#include <PopupBox.h>
#include <Image.h>
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
    *****************************************************************************/
    class ContrastMessageBox : public PopupBox, public Observer
    {
    public:
      ContrastMessageBox(Component* pParent = NULL);
      virtual ~ContrastMessageBox();

      /********************************************************************
      OPERATIONS
      ********************************************************************/

      virtual bool HandleKeyEvent(Keys key);

      virtual void Run(void);

      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* Object);
      /* --------------------------------------------------
      * Called if subscription shall be canceled
      * --------------------------------------------------*/
      virtual void SubscribtionCancelled(Subject* pSubject);
      /* --------------------------------------------------
      * Called to set the subject pointer
      * --------------------------------------------------*/
      virtual void SetSubjectPointer(int Id, Subject* pSubject);
      /* --------------------------------------------------
      * Returns the pointer to the Subject. 0 if no subject
      * is defined
      * --------------------------------------------------*/
      Subject* GetSubject();
      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      virtual void ConnectToSubjects(void);

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      SubjectPtr<U32DataPoint*> mpContrast;         // Holds the pointer to the
                                          // DataPoint (if any)

      bool mSubjectSubscribed;            // Holds if a subscription has
                                          // been made

      bool mControlBrightness;
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
      Image*  mpIconImage;
      Image*  mpSliderImage;
      Image*  mpCursorImage;
    };
  } // namespace display
} // namespace mpc

#endif
