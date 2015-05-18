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
/* FILE NAME        : UserIoSourceState.cpp                                 */
/*                                                                          */
/* CREATED DATE     : 2008-12-17                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a UserIoSourceState.           */
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
#include <AlarmText.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "DataPoint.h"
#include "UserIoSourceState.h"
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
    SourceIndexStringId mSourceIndexStringIds[] = {
      {CHANNEL_SOURCE_CONSTANT_VALUE, 0,   SID_CONSTANT_LOW, ""},
      {CHANNEL_SOURCE_CONSTANT_VALUE, 1,   SID_CONSTANT_HIGH, ""},
      {CHANNEL_SOURCE_TIMER_FUNC, 0,   SID_TIMER_FUNCTION, ""},
      {CHANNEL_SOURCE_USER_IO, 1,   SID_USERDEFINED_FUNCTION_1, ""},
      {CHANNEL_SOURCE_USER_IO, 2,   SID_USERDEFINED_FUNCTION_2, ""},
      {CHANNEL_SOURCE_USER_IO, 3,   SID_USERDEFINED_FUNCTION_3, ""},
      {CHANNEL_SOURCE_USER_IO, 4,   SID_USERDEFINED_FUNCTION_4, ""},
      {CHANNEL_SOURCE_USER_IO, 5,   SID_USERDEFINED_FUNCTION_5, ""},
      {CHANNEL_SOURCE_USER_IO, 6,   SID_USERDEFINED_FUNCTION_6, ""},
      {CHANNEL_SOURCE_USER_IO, 7,   SID_USERDEFINED_FUNCTION_7, ""},
      {CHANNEL_SOURCE_USER_IO, 8,   SID_USERDEFINED_FUNCTION_8, ""},
      {CHANNEL_SOURCE_COMBI_ALARM, 1,   SID_COMBI_ALARM_1, ""},
      {CHANNEL_SOURCE_COMBI_ALARM, 2,   SID_COMBI_ALARM_2, ""},
      {CHANNEL_SOURCE_COMBI_ALARM, 3,   SID_COMBI_ALARM_3, ""},
      {CHANNEL_SOURCE_COMBI_ALARM, 4,   SID_COMBI_ALARM_4, ""},
      {CHANNEL_SOURCE_AI_INDEX_CU_361, 1,   SID_AI1_CU351, ""},
      {CHANNEL_SOURCE_AI_INDEX_CU_361, 2,   SID_AI2_CU351, ""},
      {CHANNEL_SOURCE_AI_INDEX_CU_361, 3,   SID_AI3_CU351, ""},
      {CHANNEL_SOURCE_AI_INDEX_IO_351_1, 1,   SID_AI1_IO351_41, ""},
      {CHANNEL_SOURCE_AI_INDEX_IO_351_1, 2,   SID_AI2_IO351_41, ""},
      {CHANNEL_SOURCE_AI_INDEX_IO_351_2, 1,   SID_AI1_IO351_42, ""},
      {CHANNEL_SOURCE_AI_INDEX_IO_351_2, 2,   SID_AI2_IO351_42, ""},
      {CHANNEL_SOURCE_AI_INDEX_IO_351_3, 1,   SID_AI1_IO351_43, ""},
      {CHANNEL_SOURCE_AI_INDEX_IO_351_3, 2,   SID_AI2_IO351_43, ""},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_1, 0,   SID_MOTOR_TEMPERATURE, " #1"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_1, 1,   SID_MB_TEMPERATURE, " #1"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_1, 2,   SID_SB_TEMPERATURE, " #1"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_1, 3,   SID_PT100, " #1"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_1, 4,   SID_PT1000, " #1"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_1, 5,   SID_VIBRATION, " #1"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_1, 6,   SID_WATER_IN_OIL, " #1"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_1, 7,   SID_INSOLATION_RESISTANCE, " #1"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_2, 0,   SID_MOTOR_TEMPERATURE, " #2"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_2, 1,   SID_MB_TEMPERATURE, " #2"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_2, 2,   SID_SB_TEMPERATURE, " #2"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_2, 3,   SID_PT100, " #2"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_2, 4,   SID_PT1000, " #2"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_2, 5,   SID_VIBRATION, " #2"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_2, 6,   SID_WATER_IN_OIL, " #2"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_2, 7,   SID_INSOLATION_RESISTANCE, " #2"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_3, 0,   SID_MOTOR_TEMPERATURE, " #3"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_3, 1,   SID_MB_TEMPERATURE, " #3"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_3, 2,   SID_SB_TEMPERATURE, " #3"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_3, 3,   SID_PT100, " #3"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_3, 4,   SID_PT1000, " #3"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_3, 5,   SID_VIBRATION, " #3"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_3, 6,   SID_WATER_IN_OIL, " #3"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_3, 7,   SID_INSOLATION_RESISTANCE, " #3"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_4, 0,   SID_MOTOR_TEMPERATURE, " #4"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_4, 1,   SID_MB_TEMPERATURE, " #4"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_4, 2,   SID_SB_TEMPERATURE, " #4"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_4, 3,   SID_PT100, " #4"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_4, 4,   SID_PT1000, " #4"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_4, 5,   SID_VIBRATION, " #4"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_4, 6,   SID_WATER_IN_OIL, " #4"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_4, 7,   SID_INSOLATION_RESISTANCE, " #4"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_5, 0,   SID_MOTOR_TEMPERATURE, " #5"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_5, 1,   SID_MB_TEMPERATURE, " #5"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_5, 2,   SID_SB_TEMPERATURE, " #5"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_5, 3,   SID_PT100, " #5"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_5, 4,   SID_PT1000, " #5"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_5, 5,   SID_VIBRATION, " #5"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_5, 6,   SID_WATER_IN_OIL, " #5"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_5, 7,   SID_INSOLATION_RESISTANCE, " #5"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_6, 0,   SID_MOTOR_TEMPERATURE, " #6"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_6, 1,   SID_MB_TEMPERATURE, " #6"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_6, 2,   SID_SB_TEMPERATURE, " #6"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_6, 3,   SID_PT100, " #6"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_6, 4,   SID_PT1000, " #6"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_6, 5,   SID_VIBRATION, " #6"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_6, 6,   SID_WATER_IN_OIL, " #6"},
      {CHANNEL_SOURCE_AI_INDEX_IO_111_6, 7,   SID_INSOLATION_RESISTANCE, " #6"},
      {CHANNEL_SOURCE_DI_INDEX_CU_361, 1,   SID_DI1_CU351, ""},
      {CHANNEL_SOURCE_DI_INDEX_CU_361, 2,   SID_DI2_CU351, ""},
      {CHANNEL_SOURCE_DI_INDEX_CU_361, 3,   SID_DI3_CU351, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_1, 1,   SID_DI1_IO351_41, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_1, 2,   SID_DI2_IO351_41, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_1, 3,   SID_DI3_IO351_41, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_1, 4,   SID_DI4_IO351_41, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_1, 5,   SID_DI5_IO351_41, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_1, 6,   SID_DI6_IO351_41, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_1, 7,   SID_DI7_IO351_41, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_1, 8,   SID_DI8_IO351_41, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_1, 9,   SID_DI9_IO351_41, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_2, 1,   SID_DI1_IO351_42, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_2, 2,   SID_DI2_IO351_42, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_2, 3,   SID_DI3_IO351_42, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_2, 4,   SID_DI4_IO351_42, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_2, 5,   SID_DI5_IO351_42, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_2, 6,   SID_DI6_IO351_42, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_2, 7,   SID_DI7_IO351_42, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_2, 8,   SID_DI8_IO351_42, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_2, 9,   SID_DI9_IO351_42, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_3, 1,   SID_DI1_IO351_43, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_3, 2,   SID_DI2_IO351_43, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_3, 3,   SID_DI3_IO351_43, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_3, 4,   SID_DI4_IO351_43, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_3, 5,   SID_DI5_IO351_43, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_3, 6,   SID_DI6_IO351_43, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_3, 7,   SID_DI7_IO351_43, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_3, 8,   SID_DI8_IO351_43, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_351_3, 9,   SID_DI9_IO351_43, ""},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_1, 0,   SID_MOISTURE_IN_MOTOR, " #1"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_1, 1,   SID_THERMAL_SWITCH, " #1"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_2, 0,   SID_MOISTURE_IN_MOTOR, " #2"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_2, 1,   SID_THERMAL_SWITCH, " #2"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_3, 0,   SID_MOISTURE_IN_MOTOR, " #3"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_3, 1,   SID_THERMAL_SWITCH, " #3"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_4, 0,   SID_MOISTURE_IN_MOTOR, " #4"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_4, 1,   SID_THERMAL_SWITCH, " #4"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_5, 0,   SID_MOISTURE_IN_MOTOR, " #5"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_5, 1,   SID_THERMAL_SWITCH, " #5"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_6, 0,   SID_MOISTURE_IN_MOTOR, " #6"},
      {CHANNEL_SOURCE_DI_INDEX_IO_111_6, 1,   SID_THERMAL_SWITCH, " #6"},
      {CHANNEL_SOURCE_SYSTEM_STATES, 0,   SID_PUMP_1_RUNNING, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 1,   SID_PUMP_2_RUNNING, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 2,   SID_PUMP_3_RUNNING, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 3,   SID_PUMP_4_RUNNING, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 4,   SID_PUMP_5_RUNNING, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 5,   SID_PUMP_6_RUNNING, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 6,   SID_ALL_PUMPS_RUNNING, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 7,   SID_ANY_PUMP_RUNNING, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 8,   SID_ALL_PUMP_ALARMS, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 9,   SID_ANY_PUMP_IN_ALARM, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 10,   SID_PUMP_1_IN_ALARM, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 11,   SID_PUMP_2_IN_ALARM, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 12,   SID_PUMP_3_IN_ALARM, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 13,   SID_PUMP_4_IN_ALARM, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 14,   SID_PUMP_5_IN_ALARM, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 15,   SID_PUMP_6_IN_ALARM, ""},
      {CHANNEL_SOURCE_SYSTEM_STATES, 16,   SID_INTERLOCKED, ""}};
  

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    UserIoSourceState::UserIoSourceState(Component* pParent) : ObserverText(pParent)
    {
      mStringIdCount = sizeof(mSourceIndexStringIds) / sizeof(SourceIndexStringId);
      SetAlign(GUI_TA_LEFT|GUI_TA_HCENTER);
      this->SetWordWrap(false);
      Languages::GetInstance()->Subscribe(this);
      mShowPrefix = false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    UserIoSourceState::~UserIoSourceState()
    {
      Languages::GetInstance()->Unsubscribe(this);
    }


    /*****************************************************************************
    * Function...: ShowPrefix
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void UserIoSourceState::ShowPrefix(bool ShowPrefix)
    {
      mShowPrefix = ShowPrefix;
    }

    /*****************************************************************************
    * Function...: Redraw
    * DESCRIPTION:
    * 
    *****************************************************************************/
    bool UserIoSourceState::Redraw()
    {
      char text[MAX_STATE_TEXT_LENGTH];

      if (mpChannelConfig.IsValid())
      {
        const char* the_string_to_show;
        CHANNEL_SOURCE_TYPE source = mpChannelConfig->GetSource();
        U8 source_index = mpChannelConfig->GetSourceIndex();
        const int string_id = GetStateStringId(source, source_index);

#ifdef __PC__
        if (g_show_string_ids)
        {
          char string_id_as_text[10];
          sprintf(string_id_as_text, "%d", string_id);
          SetText(string_id_as_text);
          ObserverText::Redraw();
          return true;
        }
#endif
        const char* prefix = "";
        if (mShowPrefix)
        {
          prefix = mpChannelConfig->GetChannelPrefix();
        }

        const char* postfix = GetStringPostfix(source, source_index);
        
        if (source == CHANNEL_SOURCE_COMBI_ALARM)
        {
          StringDataPoint* p_combi_alarm_name = AlarmText::GetInstance()->GetCombiAlarm(source_index);
          if (p_combi_alarm_name != NULL)
          {
            the_string_to_show = p_combi_alarm_name->GetValue();
          }
        }
        else if (string_id != SID_NONE)
        {
          the_string_to_show = Languages::GetInstance()->GetString(string_id);
        }
        else
        {// else retry with GetStateAsString that might have be overridden by derived classes
          the_string_to_show = GetStateAsString(source, source_index);
        }

        if (strlen(the_string_to_show) > 0)
        {
          strcpy(text, Languages::GetInstance()->GetString(SID_DIGITAL_STATE_OPEN_BRACKET));
          strcat(text, prefix);
			    strcat(text, the_string_to_show);
          strcat(text, postfix);
          strcat(text, Languages::GetInstance()->GetString(SID_DIGITAL_STATE_CLOSING_BRACKET));
          SetText(text);
        }
        else
        {
          SetText("");
        }
      }
      else 
      {
   			strcpy(text, Languages::GetInstance()->GetString(SID_DIGITAL_STATE_OPEN_BRACKET));
		    strcat(text, "--");
        strcat(text, Languages::GetInstance()->GetString(SID_DIGITAL_STATE_CLOSING_BRACKET));
        SetText(text);
      }
      
      ObserverText::Redraw();
      
      return true;
    }


    /*****************************************************************************
    * Function - GetStateStringId
    * DESCRIPTION:
    *****************************************************************************/
    U16 UserIoSourceState::GetStateStringId(CHANNEL_SOURCE_TYPE source, U8 index)
    {
      for (int i = 0; i < mStringIdCount; i++)
      {
        if  (mSourceIndexStringIds[i].source == source 
          && mSourceIndexStringIds[i].sourceIndex == index)
        {
          return mSourceIndexStringIds[i].stringId;
        }
      }
      
      return SID_NONE;
    }

    
    /*****************************************************************************
    * Function - GetStateAsString
    * DESCRIPTION:
    *****************************************************************************/
    const char* UserIoSourceState::GetStateAsString(CHANNEL_SOURCE_TYPE source, U8 index)
    {
      int string_id = GetStateStringId(source, index);
      return Languages::GetInstance()->GetString(string_id);
    }

    /*****************************************************************************
    * Function - GetStringPostfix
    * DESCRIPTION:
    *
    *****************************************************************************/
    const char* UserIoSourceState::GetStringPostfix(CHANNEL_SOURCE_TYPE source, U8 index)
    {
      for (int i = 0; i < mStringIdCount; i++)
      {
        if  (mSourceIndexStringIds[i].source == source 
          && mSourceIndexStringIds[i].sourceIndex == index)
        {
          return mSourceIndexStringIds[i].stringPostfix;
        }
      }
      
      return "";
    }

    /*****************************************************************************
    * Function - ConnectToSubjects
    * DESCRIPTION: Subscribe to subjects.
    *
    *****************************************************************************/
    void UserIoSourceState::ConnectToSubjects()
    {
      mpChannelConfig.Subscribe(this);
    }

    /*****************************************************************************
    * Function - Update
    * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
    * If it is then put the pointer in queue and request task time for sub task.
    *
    *****************************************************************************/
    void UserIoSourceState::Update(Subject* pSubject)
    {
      Invalidate();
    }

    /*****************************************************************************
    * Function - SubscribtionCancelled
    * DESCRIPTION:
    *
    *****************************************************************************/
    void UserIoSourceState::SubscribtionCancelled(Subject* pSubject)
    {
      mpChannelConfig.Unsubscribe(this);
    }

    /*****************************************************************************
    * Function - SetSubjectPointer
    * DESCRIPTION: If the id is equal to a id for a subject observed.
    * Then take a copy of pSubjet to the member pointer for this subject.
    *
    *****************************************************************************/
    void UserIoSourceState::SetSubjectPointer(int id, Subject* pSubject)
    {
      mpChannelConfig.Attach(pSubject);
    }

#ifdef __PC__
    void UserIoSourceState::CalculateStringWidths(bool forceVisible)
    {
      Component::CalculateStringWidths(forceVisible);
      GUI_SetFont(GetFont());
      int component_width = GetWidth() - mLeftMargin - mRightMargin;
      int component_height = GetHeight();
      int string_width_pixels = 0;
      int string_height_pixels = 0;

      for (int i = 0; i < mStringIdCount; i++)
      {
        string_width_pixels = 0;
        string_height_pixels = 0;

        bool fits = false;

        const char* p_text = Languages::GetInstance()->GetString(mSourceIndexStringIds[i].stringId);

        if (strlen(p_text)>0 && p_text[0] != '{')
        {
          GUI_RECT rect;

          if (mWordWrap)
          {
            SetText(p_text);
            int line_count = WrapText(component_width);
            fits = (line_count <= (component_height/GetFont()->YSize));

            GUI_GetTextExtend(&rect,mTextWrapped,strlen(mTextWrapped));
            string_width_pixels = abs(rect.x1 - rect.x0) + 1;
            string_height_pixels = (abs(rect.y1 - rect.y0) + 1) * line_count;
          }
          else
          {
            GUI_GetTextExtend(&rect,p_text,strlen(p_text));
            string_width_pixels = abs(rect.x1 - rect.x0) + 1;
            string_height_pixels = abs(rect.y1 - rect.y0) + 1;

            fits = (string_width_pixels <= component_width);
          }
        }

        bool visible = (mSourceIndexStringIds[i].source == mpChannelConfig->GetSource() 
          && mSourceIndexStringIds[i].sourceIndex == mpChannelConfig->GetSourceIndex());      
  
        CSV_ENTRY entry;
        entry.componentId = mComponentId;
        entry.stringId = mSourceIndexStringIds[i].stringId;
        entry.componentWidth = component_width;
        entry.stringWidth = string_width_pixels;
        entry.componentHeight = component_height;
        entry.stringHeight = string_height_pixels;
        entry.wordwrap = mWordWrap;
        entry.fits = fits;
        entry.visible = visible;
        entry.forcedVisible = forceVisible;

        StringWidthCalculator::GetInstance()->WriteToCSV(entry);
      }
    }
#endif // __PC__


  } // namespace display
} // namespace mpc


