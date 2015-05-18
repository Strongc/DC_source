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
/* FILE NAME        : FirstUserIoChannelState.cpp                           */
/*                                                                          */
/* CREATED DATE     : 2009-02-24                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a FirstUserIoChannelState.     */
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
#include "FirstUserIoChannelState.h"
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
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    FirstUserIoChannelState::FirstUserIoChannelState(Component* pParent) : UserIoSourceState(pParent)
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    FirstUserIoChannelState::~FirstUserIoChannelState()
    {
    }

    /*****************************************************************************
    * Function - GetStateStringId
    * DESCRIPTION:
    *
    *****************************************************************************/
    U16 FirstUserIoChannelState::GetStateStringId(CHANNEL_SOURCE_TYPE source, U8 index)
    {
      USER_FUNC_SOURCE_TYPE channel = mpUserIoConfig->GetFirstSourceIndex();
      IoChannelConfig* pConfig = mpIoChannelConfigs[channel].GetSubject();

      if (mpChannelConfig.GetSubject() != pConfig)
      {
        mpChannelConfig.UnsubscribeAndDetach(this);
        mpChannelConfig.Attach(pConfig);
      }

      return UserIoSourceState::GetStateStringId(pConfig->GetSource(), pConfig->GetSourceIndex());
    }


    /*****************************************************************************
    * Function - ConnectToSubjects
    * DESCRIPTION: Subscribe to subjects.
    *
    *****************************************************************************/
    void FirstUserIoChannelState::ConnectToSubjects()
    {
      UserIoSourceState::ConnectToSubjects();

      mpUserIoConfig.Subscribe(this);

      for (int i = FIRST_USER_FUNC_SOURCE; i <= LAST_USER_FUNC_SOURCE; i++)
      {
        mpIoChannelConfigs[i].Subscribe(this);
      }
    }

    /*****************************************************************************
    * Function - Update
    * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
    * If it is then put the pointer in queue and request task time for sub task.
    *
    *****************************************************************************/
    void FirstUserIoChannelState::Update(Subject* pSubject)
    {
      UserIoSourceState::Update(pSubject);
    }

    /*****************************************************************************
    * Function - SubscribtionCancelled
    * DESCRIPTION:
    *
    *****************************************************************************/
    void FirstUserIoChannelState::SubscribtionCancelled(Subject* pSubject)
    {
      UserIoSourceState::SubscribtionCancelled(pSubject);

      mpUserIoConfig.Unsubscribe(this);

      for (int i = FIRST_USER_FUNC_SOURCE; i <= LAST_USER_FUNC_SOURCE; i++)
      {
        mpIoChannelConfigs[i].Unsubscribe(this);
      }
    }

    /*****************************************************************************
    * Function - SetSubjectPointer
    * DESCRIPTION: If the id is equal to a id for a subject observed.
    * Then take a copy of pSubjet to the member pointer for this subject.
    *
    *****************************************************************************/
    void FirstUserIoChannelState::SetSubjectPointer(int id, Subject* pSubject)
    {
      mpUserIoConfig.Attach(pSubject);

      for (int i = USER_FUNC_SOURCE_1_1; i <= USER_FUNC_SOURCE_8_1; i++)
      {
        mpIoChannelConfigs[i].Attach(::GetSubject(SUBJECT_ID_USER_FUNC_1_SOURCE_1_CONFIG + i - USER_FUNC_SOURCE_1_1));
      }

      for (int i = USER_FUNC_SOURCE_1_2; i <= USER_FUNC_SOURCE_8_2; i++)
      {
        mpIoChannelConfigs[i].Attach(::GetSubject(SUBJECT_ID_USER_FUNC_1_SOURCE_2_CONFIG + i - USER_FUNC_SOURCE_1_2));
      }

      USER_FUNC_SOURCE_TYPE channel = mpUserIoConfig->GetFirstSourceIndex();
      IoChannelConfig* pConfig = mpIoChannelConfigs[channel].GetSubject();

      UserIoSourceState::SetSubjectPointer(id, pConfig);
    }



  } // namespace display
} // namespace mpc


