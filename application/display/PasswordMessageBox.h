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
/* CLASS NAME       : PasswordMessageBox                                    */
/*                                                                          */
/* FILE NAME        : PasswordMessageBox.h                                  */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayPasswordMessageBox_h
#define mpc_displayPasswordMessageBox_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <PopupBox.h>
#include <Observer.h>
#include <U32DataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
#define PW_NUMBER_OF_DIGIT 4
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    // FORWARD DECLARATIONS
    class Image;
    class Label;
    
    class MultiNumber;
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class PasswordMessageBox : public PopupBox, Observer
    {
    public:
      PasswordMessageBox(Component* pParent = NULL);
      virtual ~PasswordMessageBox();

      /********************************************************************
      OPERATIONS
      ********************************************************************/

      virtual bool HandleKeyEvent(Keys KeyID);

      virtual void Run(void);
      virtual void SetPw(int Pw);
      virtual void SetPwType(int type);

      virtual void SubscribtionCancelled(Subject* pSubject);
      virtual void Update(Subject* pSubject);
      virtual void SetSubjectPointer(int Id,Subject* pSubject);
      virtual void ConnectToSubjects(void);

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      void DeleteMessageBox();
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void ValidatePassword(void);
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      int              mPw;
      int              mPwType;
      bool             mTestPw;

      Image*           mpIconImage;
      Label*           mpTitle;
      Label*           mpMessage;
      MultiNumber*     mpMultiNumber;
      U32DataPoint*    mpDpMultiNumber;
    };
  } // namespace display
} // namespace mpc

#endif
