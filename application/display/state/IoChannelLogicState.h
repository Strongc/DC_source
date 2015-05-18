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
/* CLASS NAME       : IoChannelLogicState                                   */
/*                                                                          */
/* FILE NAME        : IoChannelLogicState.h                                 */
/*                                                                          */
/* CREATED DATE     : 2009-02-26                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayIoChannelLogicState_h
#define mpc_displayIoChannelLogicState_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "BoolDataPoint.h"
#include "IoChannelConfig.h"
#include "../gui_utility/Languages.h"
#include "DisplayTypes.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverText.h"
#include "DigitalInputLogicState.h"
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
    * This Class is responsible for showing an IoChannelLogicState string
    * given by a IoChannelConfig DataPoint
    *
    *****************************************************************************/
    class IoChannelLogicState : public DigitalInputLogicState 
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      IoChannelLogicState(Component* pParent = NULL){}
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~IoChannelLogicState(){}
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual U16 GetStateStringId(int state);

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
      SubjectPtr<IoChannelConfig*>  mIoChannelConfig;
      SubjectPtr<BoolDataPoint*>    mDigInConfLogic[DI_INDEX_DI9_IO351_43];
      SubjectPtr<BoolDataPoint*>    mCurrentDigInConfLogic;

      
    };
  } // namespace display
} // namespace mpc

#endif
