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
/* CLASS NAME       : InputFunctionState                                    */
/*                                                                          */
/* FILE NAME        : InputFunctionState.h                                  */
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
#ifndef mpc_displayInputFunctionState_h
#define mpc_displayInputFunctionState_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "EnumDataPoint.h"
#include "IoChannelConfig.h"
#include "../gui_utility/Languages.h"
#include "DisplayTypes.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverText.h"
#include "UserIoSourceState.h"
#include "AnalogInputFunctionState.h"
#include "DigitalInputFunctionState.h"
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
    * This Class is responsible for how to show a InputFunctionState string given by a DataPoint
    *
    *****************************************************************************/
    class InputFunctionState : public UserIoSourceState 
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      InputFunctionState(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~InputFunctionState();
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual U16 GetStateStringId(CHANNEL_SOURCE_TYPE source, U8 index);
      virtual const char* GetStateAsString(CHANNEL_SOURCE_TYPE source, U8 index);

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
      bool GetStateId(CHANNEL_SOURCE_TYPE source, U8 index, int* aiState, int* diState);
      
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
      AnalogInputFunctionState* mpAiFuncState;
      DigitalInputFunctionState* mpDiFuncState;

      SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*>     mpAnaInConfFunc[NO_OF_AI_INDEX];
      SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*> mpDigInConfFunc[NO_OF_DI_INDEX];
      
    };
  } // namespace display
} // namespace mpc

#endif
