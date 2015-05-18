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
/* FILE NAME        : IoChannelLogicState.cpp                               */
/*                                                                          */
/* CREATED DATE     : 2009-02-24                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a IoChannelLogicState.         */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "DataPoint.h"
#include "IoChannelLogicState.h"
#include <AppTypeDefs.h>

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
    * Function - GetStateStringId
    * DESCRIPTION:
    *
    *****************************************************************************/
    U16 IoChannelLogicState::GetStateStringId(int state)
    {
      int index = mIoChannelConfig->GetSourceIndex();

      // add offset for each DI source
      switch (mIoChannelConfig->GetSource())
      {
        case CHANNEL_SOURCE_DI_INDEX_CU_361:
          index += DI_INDEX_DI1_CU361 - 2;
          break;
        case CHANNEL_SOURCE_DI_INDEX_IO_351_1:
          index += DI_INDEX_DI1_IO351_41 - 2;
          break;
        case CHANNEL_SOURCE_DI_INDEX_IO_351_2:
          index += DI_INDEX_DI1_IO351_42 - 2;
          break;
        case CHANNEL_SOURCE_DI_INDEX_IO_351_3:
          index += DI_INDEX_DI1_IO351_43 - 2;
          break;
        default:
          return SID_NONE;
      }

      if (mCurrentDigInConfLogic.GetSubject() != mDigInConfLogic[index].GetSubject())
      {
        mCurrentDigInConfLogic.UnsubscribeAndDetach(this);
        mCurrentDigInConfLogic.Attach(mDigInConfLogic[index].GetSubject());
      }

      int logicState = mCurrentDigInConfLogic->GetAsInt();
     
      return DigitalInputLogicState::GetStateStringId(logicState);

    }

    /*****************************************************************************
    * Function - ConnectToSubjects
    * DESCRIPTION: Subscribe to subjects.
    *
    *****************************************************************************/
    void IoChannelLogicState::ConnectToSubjects()
    {
      mIoChannelConfig.Subscribe(this);

      mCurrentDigInConfLogic.Subscribe(this);
    }

    /*****************************************************************************
    * Function - Update
    * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
    * If it is then put the pointer in queue and request task time for sub task.
    *
    *****************************************************************************/
    void IoChannelLogicState::Update(Subject* pSubject)
    {
      Invalidate();      
    }

    /*****************************************************************************
    * Function - SubscribtionCancelled
    * DESCRIPTION:
    *
    *****************************************************************************/
    void IoChannelLogicState::SubscribtionCancelled(Subject* pSubject)
    {
      mIoChannelConfig.Unsubscribe(this);

      mCurrentDigInConfLogic.Unsubscribe(this);
    }

    /*****************************************************************************
    * Function - SetSubjectPointer
    * DESCRIPTION: 
    *
    *****************************************************************************/
    void IoChannelLogicState::SetSubjectPointer(int id, Subject* pSubject)
    {
      mIoChannelConfig.Attach(pSubject);

      Subject* conf_logic = NULL;

      for (int i = DI_INDEX_DI1_CU361; i <= DI_INDEX_DI9_IO351_42; i++)
      {
        conf_logic = ::GetSubject(SUBJECT_ID_DIG_IN_1_CONF_LOGIC + i - DI_INDEX_DI1_CU361);
        mDigInConfLogic[i - DI_INDEX_DI1_CU361].Attach(conf_logic);
      }
      for (int i = DI_INDEX_DI1_IO351_43; i <= DI_INDEX_DI9_IO351_43; i++)
      {
        conf_logic = ::GetSubject(SUBJECT_ID_DIG_IN_22_CONF_LOGIC + i - DI_INDEX_DI1_IO351_43);
        mDigInConfLogic[i - DI_INDEX_DI1_CU361].Attach(conf_logic);
      }

      mCurrentDigInConfLogic.Attach(mDigInConfLogic[0].GetSubject());

      DigitalInputLogicState::SetSubjectPointer(id, mCurrentDigInConfLogic.GetSubject());
            
    }



  } // namespace display
} // namespace mpc


