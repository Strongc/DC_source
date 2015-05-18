/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : FirstUserIoChannelState                               */
/*                                                                          */
/* FILE NAME        : FirstUserIoChannelState.h                             */
/*                                                                          */
/* CREATED DATE     : 2008-12-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayFirstUserIoChannelState_h
#define mpc_displayFirstUserIoChannelState_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "EnumDataPoint.h"
#include "UserIoConfig.h"
#include "IoChannelConfig.h"
#include "../gui_utility/Languages.h"
#include "DisplayTypes.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverText.h"
#include "UserIoSourceState.h"
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
    * This Class is responsible for how to show a FirstUserIoChannelState string given by a DataPoint
    *
    *****************************************************************************/
    class FirstUserIoChannelState : public UserIoSourceState 
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      FirstUserIoChannelState(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~FirstUserIoChannelState();
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual U16 GetStateStringId(CHANNEL_SOURCE_TYPE source, U8 index);

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
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      SubjectPtr<UserIoConfig*>     mpUserIoConfig;
      SubjectPtr<IoChannelConfig*>  mpIoChannelConfigs[NO_OF_USER_FUNC_SOURCE + 1];
      
    };
  } // namespace display
} // namespace mpc

#endif
