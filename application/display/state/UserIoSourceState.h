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
/* CLASS NAME       : UserIoSourceState                                     */
/*                                                                          */
/* FILE NAME        : UserIoSourceState.h                                   */
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
#ifndef mpc_displayUserIoSourceState_h
#define mpc_displayUserIoSourceState_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "EnumDataPoint.h"
#include "U8DataPoint.h"
#include "../gui_utility/Languages.h"
#include "IoChannelConfig.h"
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverText.h"
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
    struct SourceIndexStringId
    {
      CHANNEL_SOURCE_TYPE source;
      U8  sourceIndex;
      U16 stringId;
      const char* stringPostfix;
    };
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a UserIoSourceState string given by a DataPoint
    *
    *****************************************************************************/
    class UserIoSourceState : public ObserverText 
    {
    public:
      /********************************************************************
      LIFECYCLE - Constructor.
      ********************************************************************/
      UserIoSourceState(Component* pParent = NULL);
     
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~UserIoSourceState();
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void ShowPrefix(bool ShowPrefix);

      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      virtual U16 GetStateStringId(CHANNEL_SOURCE_TYPE source, U8 index);
      virtual const char* GetStateAsString(CHANNEL_SOURCE_TYPE source, U8 index);
      virtual const char* GetStringPostfix(CHANNEL_SOURCE_TYPE source, U8 index);      

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

#ifdef __PC__
      virtual void CalculateStringWidths(bool forceVisible);
#endif // __PC__

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
      SubjectPtr<IoChannelConfig*>  mpChannelConfig;

      int  mStringIdCount;
      bool mShowPrefix;
    };
  } // namespace display
} // namespace mpc

#endif
