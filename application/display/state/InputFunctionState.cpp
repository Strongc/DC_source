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
/* FILE NAME        : InputFunctionState.cpp                                */
/*                                                                          */
/* CREATED DATE     : 2008-12-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a InputFunctionState.          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef __PC__
#include "StringWidthCalculator.h"
extern bool g_show_string_ids;
#endif
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "DataPoint.h"
#include "InputFunctionState.h"
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
    InputFunctionState::InputFunctionState(Component* pParent) : UserIoSourceState(pParent)
    {
      mpAiFuncState = new AnalogInputFunctionState();
      mpDiFuncState = new DigitalInputFunctionState();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    InputFunctionState::~InputFunctionState()
    {
    }

    /*****************************************************************************
    * Function - GetStateStringId
    * DESCRIPTION:
    *****************************************************************************/
    U16 InputFunctionState::GetStateStringId(CHANNEL_SOURCE_TYPE source, U8 index)
    {
      int ai_state = -1;
      int di_state = -1;

      GetStateId(source, index, &ai_state, &di_state);

      if (ai_state != -1)
      {
        return mpAiFuncState->GetStateStringId(ai_state);
      }
      else if (di_state != -1)
      {
        return mpDiFuncState->GetStateStringId(di_state);
      }
      
      return SID_NONE;
    }



    /*****************************************************************************
    * Function - GetStateAsString
    * DESCRIPTION:
    *****************************************************************************/
    const char* InputFunctionState::GetStateAsString(CHANNEL_SOURCE_TYPE source, U8 index)
    {
      int ai_state = -1;
      int di_state = -1;

      GetStateId(source, index, &ai_state, &di_state);

      if (ai_state != -1)
      {
        return mpAiFuncState->GetStateAsString(ai_state);
      }
      else if (di_state != -1)
      {
        return mpDiFuncState->GetStateAsString(di_state);
      }

      return "";
    }


    /*****************************************************************************
    * Function - ConnectToSubjects
    * DESCRIPTION: Subscribe to subjects.
    *
    *****************************************************************************/
    void InputFunctionState::ConnectToSubjects()
    {
      UserIoSourceState::ConnectToSubjects();
      
      for (int i = AI_INDEX_AI1_CU361; i < LAST_AI_INDEX; i++)
      {
        mpAnaInConfFunc[i].Subscribe(this);
      }

      for (int i = DI_INDEX_DI1_CU361; i < LAST_DI_INDEX; i++)
      {
        mpDigInConfFunc[i].Subscribe(this);
      }

    }

    /*****************************************************************************
    * Function - Update
    * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
    * If it is then put the pointer in queue and request task time for sub task.
    *
    *****************************************************************************/
    void InputFunctionState::Update(Subject* pSubject)
    {
      Invalidate();
    }

    /*****************************************************************************
    * Function - SubscribtionCancelled
    * DESCRIPTION:
    *
    *****************************************************************************/
    void InputFunctionState::SubscribtionCancelled(Subject* pSubject)
    {
      UserIoSourceState::SubscribtionCancelled(pSubject);

      for (int i = AI_INDEX_AI1_CU361; i < LAST_AI_INDEX; i++)
      {
        mpAnaInConfFunc[i].Unsubscribe(this);
      }

      for (int i = DI_INDEX_DI1_CU361; i < LAST_DI_INDEX; i++)
      {
        mpDigInConfFunc[i].Unsubscribe(this);
      }
    }

    /*****************************************************************************
    * Function - SetSubjectPointer
    * DESCRIPTION: If the id is equal to a id for a subject observed.
    * Then take a copy of pSubjet to the member pointer for this subject.
    *
    *****************************************************************************/
    void InputFunctionState::SetSubjectPointer(int id, Subject* pSubject)
    {
      UserIoSourceState::SetSubjectPointer(id, pSubject);

      for (int i = AI_INDEX_AI1_CU361; i <= AI_INDEX_AI2_IO351_42; i++)
      {
        mpAnaInConfFunc[i].Attach(::GetSubject(SUBJECT_ID_ANA_IN_1_CONF_MEASURED_VALUE + i - 1));
      }
      for (int i = AI_INDEX_AI1_IO351_43; i <= AI_INDEX_AI2_IO351_43; i++)
      {
        mpAnaInConfFunc[i].Attach(::GetSubject(SUBJECT_ID_ANA_IN_8_CONF_MEASURED_VALUE + i - 8));
      }
      for (int i = DI_INDEX_DI1_CU361; i <= DI_INDEX_DI9_IO351_42; i++)
      {
        mpDigInConfFunc[i].Attach(::GetSubject(SUBJECT_ID_DIG_IN_1_CONF_DIGITAL_INPUT_FUNC + i - 1));
      }
      for (int i = DI_INDEX_DI1_IO351_43; i <= DI_INDEX_DI9_IO351_43; i++)
      {
        mpDigInConfFunc[i].Attach(::GetSubject(SUBJECT_ID_DIG_IN_22_CONF_DIGITAL_INPUT_FUNC + i - 22));
      }

    }


    
    /*****************************************************************************
    * Function - GetStateId
    * DESCRIPTION:
    *****************************************************************************/
    bool InputFunctionState::GetStateId(CHANNEL_SOURCE_TYPE source, U8 index, int* aiState, int* diState)
    {
      // all AI and DI functions are 1-indexed
      if (index == 0)
      {
        return false;
      }

      index--;
      
      switch (source)
      {
         case CHANNEL_SOURCE_AI_INDEX_CU_361:
          *aiState = mpAnaInConfFunc[index + AI_INDEX_AI1_CU361]->GetAsInt();
          break;
        case CHANNEL_SOURCE_AI_INDEX_IO_351_1:
          *aiState = mpAnaInConfFunc[index + AI_INDEX_AI1_IO351_41]->GetAsInt();
          break;
        case CHANNEL_SOURCE_AI_INDEX_IO_351_2:
          *aiState = mpAnaInConfFunc[index + AI_INDEX_AI1_IO351_42]->GetAsInt();
          break;
        case CHANNEL_SOURCE_AI_INDEX_IO_351_3:
          *aiState = mpAnaInConfFunc[index + AI_INDEX_AI1_IO351_43]->GetAsInt();
          break;
        case CHANNEL_SOURCE_DI_INDEX_CU_361:
          *diState = mpDigInConfFunc[index + DI_INDEX_DI1_CU361]->GetAsInt();
          break;
        case CHANNEL_SOURCE_DI_INDEX_IO_351_1:
          *diState = mpDigInConfFunc[index + DI_INDEX_DI1_IO351_41]->GetAsInt();
          break;
        case CHANNEL_SOURCE_DI_INDEX_IO_351_2:
          *diState = mpDigInConfFunc[index + DI_INDEX_DI1_IO351_42]->GetAsInt();
          break;
        case CHANNEL_SOURCE_DI_INDEX_IO_351_3:
          *diState = mpDigInConfFunc[index + DI_INDEX_DI1_IO351_43]->GetAsInt();
          break;
        default:
          return false;
      }

      return true;
    }


  } // namespace display
} // namespace mpc


