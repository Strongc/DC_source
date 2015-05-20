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
/* CLASS NAME       : DigitalOutputFunctionState                            */
/*                                                                          */
/* FILE NAME        : DigitalOutputFunctionState.cpp                        */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a DigitalOutputFunctionState.  */
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
***************************************************************************/
#include "DigitalOutputFunctionState.h"

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
    DEFINES
    *****************************************************************************/

    StateStringId mDigitalOutputFunctionStatesStringIds[] =
    {
      { RELAY_FUNC_NO_FUNCTION                  , SID_DO_RELAY_NO_FUNC                      },
      { RELAY_FUNC_PUMP_1                       , SID_DO_PUMP_1_START                       },
      { RELAY_FUNC_PUMP_2                       , SID_DO_PUMP_2_START                       },
      { RELAY_FUNC_PUMP_3                       , SID_PUMP_3_START                          },
      { RELAY_FUNC_PUMP_4                       , SID_PUMP_4_START                          },
      { RELAY_FUNC_PUMP_5                       , SID_PUMP_5_START                          },
      { RELAY_FUNC_PUMP_6                       , SID_PUMP_6_START                          },
      { RELAY_FUNC_MIXER                        , SID_DO_MIXER_START                        },
      { RELAY_FUNC_RELAY_CUSTOM                 , SID_DO_CUSTOM_RELAY                       },
      { RELAY_FUNC_ALARM_RELAY_HIGH_LEVEL       , SID_DO_HIGH_LEVEL_ALARM                   },
      { RELAY_FUNC_ALARM_RELAY_CRITICAL         , SID_DO_URGENT_ALARMS                      },
      { RELAY_FUNC_ALARM_RELAY_ALL_ALARMS       , SID_DO_ALL_ALARMS                         },
      { RELAY_FUNC_ALARM_RELAY_ALL_ALARMS_AND_WARNINGS , SID_DO_ALL_ALARMS_AND_WARNINGS     },
      { RELAY_FUNC_ALARM_RELAY_CUSTOM           , SID_DO_CUSTOM_ALARM_RELAY                 },
      { RELAY_FUNC_USER_IO_1                    , SID_USERDEFINED_FUNCTION_1                },
      { RELAY_FUNC_USER_IO_2                    , SID_USERDEFINED_FUNCTION_2                },
      { RELAY_FUNC_USER_IO_3                    , SID_USERDEFINED_FUNCTION_3                },
      { RELAY_FUNC_USER_IO_4                    , SID_USERDEFINED_FUNCTION_4                },
      { RELAY_FUNC_USER_IO_5                    , SID_USERDEFINED_FUNCTION_5                },
      { RELAY_FUNC_USER_IO_6                    , SID_USERDEFINED_FUNCTION_6                },
      { RELAY_FUNC_USER_IO_7                    , SID_USERDEFINED_FUNCTION_7                },
      { RELAY_FUNC_USER_IO_8                    , SID_USERDEFINED_FUNCTION_8                },
      { RELAY_FUNC_VFD_1_REVERSE                , SID_DO_VFD_1_REVERSE                      },
      { RELAY_FUNC_VFD_2_REVERSE                , SID_DO_VFD_2_REVERSE                      },
      { RELAY_FUNC_VFD_3_REVERSE                , SID_DO_VFD_3_REVERSE                      },
      { RELAY_FUNC_VFD_4_REVERSE                , SID_DO_VFD_4_REVERSE                      },
      { RELAY_FUNC_VFD_5_REVERSE                , SID_DO_VFD_5_REVERSE                      },
      { RELAY_FUNC_VFD_6_REVERSE                , SID_DO_VFD_6_REVERSE                      },
      { RELAY_FUNC_DOSING_PUMP                  , SID_DO_START_DOSING_PUMP                  }
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    DigitalOutputFunctionState::DigitalOutputFunctionState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mDigitalOutputFunctionStatesStringIds;
      mStringIdCount = sizeof( mDigitalOutputFunctionStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    DigitalOutputFunctionState::~DigitalOutputFunctionState()
    {
    }

    /*****************************************************************************
    * Function   : Redraw
    * DESCRIPTION:
    *****************************************************************************/
    bool DigitalOutputFunctionState::Redraw()
    {
      char text[MAX_STATE_TEXT_LENGTH];

      if (mpState.IsValid())
      {
        if (mpState->GetQuality() != DP_AVAILABLE )
        {
          SetText("--");
        }
        else
        {
          int state = mpState->GetAsInt();

          if (state >= RELAY_FUNC_USER_IO_1 && state <= RELAY_FUNC_USER_IO_8)
          {
            if (mpUserIoNames[state - RELAY_FUNC_USER_IO_1].IsValid())
            {
              strcpy(text,Languages::GetInstance()->GetString(SID_DIGITAL_STATE_OPEN_BRACKET));
			        strcat(text,mpUserIoNames[state - RELAY_FUNC_USER_IO_1]->GetValue());
              strcat(text,Languages::GetInstance()->GetString(SID_DIGITAL_STATE_CLOSING_BRACKET));
              SetText(text);
            }

          #ifdef __PC__
            if (g_show_string_ids)
            {
              SetText("(-)");
              ObserverText::Redraw();
              return true;
            }
          #endif

          }
          else
          {
            return StateBracket::Redraw();
          }
        }
      }
      else
      {
        SetText("--");
      }

      ObserverText::Redraw();

      return true;
    }


    /*****************************************************************************
    * Function   : Update
    * DESCRIPTION:
    *****************************************************************************/
    void DigitalOutputFunctionState::Update(Subject* pSubject)
    {
      if (mpState.Update(pSubject))
      {
        // state has changed
        Invalidate();
      }
      else
      {
        int state = mpState->GetAsInt();

        if (state >= RELAY_FUNC_USER_IO_1
          && state <= RELAY_FUNC_USER_IO_8)
        {
          if (mpUserIoNames[state - RELAY_FUNC_USER_IO_1].GetSubject()->GetSubjectId() == pSubject->GetSubjectId())
          {
            // stringdatapoint of current state has been updated
            Invalidate();
          }
        }
      }
    }

    /*****************************************************************************
    * Function   : SubscribtionCancelled
    * DESCRIPTION:
    *****************************************************************************/
    void DigitalOutputFunctionState::SubscribtionCancelled(Subject* pSubject)
    {
      mpState.Unsubscribe(this);
      for (int i = FIRST_USER_IO; i < LAST_USER_IO; i++)
      {
        mpUserIoNames[i - FIRST_USER_IO].Unsubscribe(this);
      }
    }


    /*****************************************************************************
    * Function   : SetSubjectPointer
    * DESCRIPTION:
    *****************************************************************************/
    void DigitalOutputFunctionState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      mpState.Attach(pSubject);
      mpSubject.Attach(pSubject);

      mpUserIoNames[0].Attach(::GetSubject(SUBJECT_ID_RELAY_FUNC_NAME_USER_IO_1));
      mpUserIoNames[1].Attach(::GetSubject(SUBJECT_ID_RELAY_FUNC_NAME_USER_IO_2));
      mpUserIoNames[2].Attach(::GetSubject(SUBJECT_ID_RELAY_FUNC_NAME_USER_IO_3));
      mpUserIoNames[3].Attach(::GetSubject(SUBJECT_ID_RELAY_FUNC_NAME_USER_IO_4));
      mpUserIoNames[4].Attach(::GetSubject(SUBJECT_ID_RELAY_FUNC_NAME_USER_IO_5));
      mpUserIoNames[5].Attach(::GetSubject(SUBJECT_ID_RELAY_FUNC_NAME_USER_IO_6));
      mpUserIoNames[6].Attach(::GetSubject(SUBJECT_ID_RELAY_FUNC_NAME_USER_IO_7));
      mpUserIoNames[7].Attach(::GetSubject(SUBJECT_ID_RELAY_FUNC_NAME_USER_IO_8));

    }

    /*****************************************************************************
    * Function   : ConnectToSubjects
    * DESCRIPTION:
    *****************************************************************************/
    void DigitalOutputFunctionState::ConnectToSubjects(void)
    {
      mpState.Subscribe(this);
      for (int i = FIRST_USER_IO; i < LAST_USER_IO; i++)
      {
        mpUserIoNames[i - FIRST_USER_IO].Subscribe(this);
      }
    }

  } // namespace display
} // namespace mpc




