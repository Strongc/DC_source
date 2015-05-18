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
/* CLASS NAME       : SmsScheduleGraphic                                    */
/*                                                                          */
/* FILE NAME        : SmsScheduleGraphic.Cpp                                */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* The filename says it all                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Factory.h>
#include <gui_utility/Languages.h>
#include <DisplayController.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <SmsScheduleGraphic.h>
#include <MPCFonts.h>
#include <EventList.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
#define COL_WIDTH   30 // width of columns including margin
#define COL_HEIGHT 216 // height should be dividable by 12 to look nice
#define COL_WIDTH_MARGIN 0 //the margin used on each side of a column

#define GRAPH_X1 20 // this gives space for time-of-day texts to the left
#define GRAPH_Y1 17 // this gives space for legend above graph area
#define GRAPH_X2 (GRAPH_X1 + (COL_WIDTH * NO_OF_WEEKDAY))
#define GRAPH_Y2 (GRAPH_Y1 + COL_HEIGHT)

#define GRAPH_HEIGHT (GRAPH_Y2 - GRAPH_Y1+1)

#define TIME_OF_DAY_HEIGHT (GRAPH_HEIGHT-1) / (NO_OF_TIME_OF_DAY_LABELS-1)

#define ONE_DAY_IN_SEC (24*60*60)

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    /*****************************************************************************
    *
    *
    *              PUBLIC FUNCTIONS
    *
    *
    *****************************************************************************/

    /*****************************************************************************
    * Function - Constructor
    * DESCRIPTION:
    *
    *****************************************************************************/
    SmsScheduleGraphic::SmsScheduleGraphic(Component* pParent): Frame(true,false,pParent)
    {
      U8 i = 0;

      for (i=FIRST_WEEKDAY; i<NO_OF_WEEKDAY; i++)
      {
        mpWeekdayLabels[i] = new Label(this);
        mpWeekdayLabels[i]->SetAlign(GUI_TA_VCENTER+GUI_TA_HCENTER);
        mpWeekdayLabels[i]->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
        mpWeekdayLabels[i]->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
        mpWeekdayLabels[i]->SetLeftMargin(0);
        mpWeekdayLabels[i]->SetRightMargin(0);
        mpWeekdayLabels[i]->SetReadOnly(true);
        mpWeekdayLabels[i]->SetVisible(true);
        mpWeekdayLabels[i]->SetWordWrap(false);
        AddChild(mpWeekdayLabels[i]);
      }

      mpWeekdayLabels[WEEKDAY_MONDAY]->SetStringId(SID_MONDAY_SHORT);
      mpWeekdayLabels[WEEKDAY_TUESDAY]->SetStringId(SID_TUESDAY_SHORT);
      mpWeekdayLabels[WEEKDAY_WEDNESDAY]->SetStringId(SID_WEDNESDAY_SHORT);
      mpWeekdayLabels[WEEKDAY_THURSDAY]->SetStringId(SID_THURSDAY_SHORT);
      mpWeekdayLabels[WEEKDAY_FRIDAY]->SetStringId(SID_FRIDAY_SHORT);
      mpWeekdayLabels[WEEKDAY_SATURDAY]->SetStringId(SID_SATURDAY_SHORT);
      mpWeekdayLabels[WEEKDAY_SUNDAY]->SetStringId(SID_SUNDAY_SHORT);

      for (i=0; i<NO_OF_TIME_OF_DAY_LABELS; i++)
      {
        mpTimeOfDayTexts[i] = new Text(this);
        mpTimeOfDayTexts[i]->SetAlign(GUI_TA_VCENTER+GUI_TA_RIGHT);
        mpTimeOfDayTexts[i]->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
        mpTimeOfDayTexts[i]->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
        mpTimeOfDayTexts[i]->SetLeftMargin(0);
        mpTimeOfDayTexts[i]->SetRightMargin(0);
        mpTimeOfDayTexts[i]->SetReadOnly(true);
        mpTimeOfDayTexts[i]->SetVisible( (i%2==0) );
        mpTimeOfDayTexts[i]->SetWordWrap(false);
        mpTimeOfDayTexts[i]->SetClientArea( 1, GRAPH_Y1+i*TIME_OF_DAY_HEIGHT - 10, 15, GRAPH_Y1+(i+1)*TIME_OF_DAY_HEIGHT - 10);
        AddChild(mpTimeOfDayTexts[i]);
      }
      mpTimeOfDayTexts[0]->SetText("00");
      mpTimeOfDayTexts[1]->SetText("02");
      mpTimeOfDayTexts[2]->SetText("04");
      mpTimeOfDayTexts[3]->SetText("06");
      mpTimeOfDayTexts[4]->SetText("08");
      mpTimeOfDayTexts[5]->SetText("10");
      mpTimeOfDayTexts[6]->SetText("12");
      mpTimeOfDayTexts[7]->SetText("14");
      mpTimeOfDayTexts[8]->SetText("16");
      mpTimeOfDayTexts[9]->SetText("18");
      mpTimeOfDayTexts[10]->SetText("20");
      mpTimeOfDayTexts[11]->SetText("22");
      mpTimeOfDayTexts[12]->SetText("24");

      for (i=0; i<NO_OF_SMS_CATEGORY-1; i++)
      {
        mpLegendFrames[i] = new Frame(true, true, this);
        mpLegendFrames[i]->SetBackgroundColour( BACKCOLOR_BY_TYPE[i] );
        mpLegendFrames[i]->SetClientArea( i*80 + (i>0?0:1), 245, (i+1)*80 - (i==2?1:0), 261 );
        mpLegendFrames[i]->SetVisible(true);

        mpLegendTexts[i] = new Text(mpLegendFrames[i]);
        mpLegendTexts[i]->SetAlign(GUI_TA_VCENTER+GUI_TA_HCENTER);
        mpLegendTexts[i]->SetBackgroundColour( BACKCOLOR_BY_TYPE[i] );
        mpLegendTexts[i]->SetColour( FORECOLOR_BY_TYPE[i]  );
        mpLegendTexts[i]->SetFont(DEFAULT_FONT_11_LANGUAGE_DEP);
        mpLegendTexts[i]->SetLeftMargin(1);
        mpLegendTexts[i]->SetRightMargin(1);
        mpLegendTexts[i]->SetReadOnly(true);
        mpLegendTexts[i]->SetVisible(true);
        mpLegendTexts[i]->SetWordWrap(false);
        mpLegendTexts[i]->SetClientArea( 1, 1, 78 - (i>1?1:0), 15);

        mpLegendFrames[i]->AddChild(mpLegendTexts[i]);

        AddChild(mpLegendFrames[i]);
      }

      mDpWeekStartAtMonday.SetUpdated();
      mDpCurrentTypeOfSchedule.SetUpdated();

      mUpdate = true;
    }

    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    SmsScheduleGraphic::~SmsScheduleGraphic()
    {
      U8 i;
      for (i=FIRST_WEEKDAY; i<NO_OF_WEEKDAY; i++)
      {
        delete mpWeekdayLabels[i];
      }

      for (i=0; i<NO_OF_TIME_OF_DAY_LABELS; i++)
      {
        delete mpTimeOfDayTexts[i];
      }

      for (i=0; i<NO_OF_SMS_CATEGORY-1; i++)
      {
        delete mpLegendFrames[i];
        delete mpLegendTexts[i];
      }
    }


    /*****************************************************************************
    * Function - SubscribtionCancelled
    * DESCRIPTION: Implements the Observer::SubscribtionCancelled
    *
    ****************************************************************************/
    void SmsScheduleGraphic::SubscribtionCancelled(Subject* pSubject)
    {
      // not currently needed, thus not implemented
    }

    /*****************************************************************************
    * Function - Update
    * DESCRIPTION: Implements the Observer::Update
    *
    ****************************************************************************/
    void SmsScheduleGraphic::Update(Subject* pSubject)
    {
      // signal a redraw no matter what changes. This solution is simple (and prone to flicker)
      // but acceptable as changes are made infrequently through use of another display.
      mUpdate = true;

      if (mDpWeekStartAtMonday.Update(pSubject)){}
      else if (mDpCurrentTypeOfSchedule.Update(pSubject)){}
      else if (mDpPhoneNo[PHONE_NO_1].Update(pSubject)){}
      else if (mDpPhoneNo[PHONE_NO_2].Update(pSubject)){}
      else if (mDpPhoneNo[PHONE_NO_3].Update(pSubject)){}

     }

    /*****************************************************************************
    * Function - SetSubjectPointer
    * DESCRIPTION: Implements the Observer::SetSubjectPointer
    *
    ****************************************************************************/
    void SmsScheduleGraphic::SetSubjectPointer(int Id,Subject* pSubject)
    {
      switch (Id)
      {
      case SP_SSG_TYPE_OF_SCHEDULE:
        mDpCurrentTypeOfSchedule.Attach(pSubject);
        break;
      case SP_SSG_WEEK_START_AT_MONDAY:
        mDpWeekStartAtMonday.Attach(pSubject);
        break;
      case SP_SSG_WORK_ENABLED:
        mDpCategoryEnabled[SMS_CATEGORY_WORK].Attach(pSubject);
        break;
      case SP_SSG_OFF_ENABLED:
        mDpCategoryEnabled[SMS_CATEGORY_OFF].Attach(pSubject);
        break;
      case SP_SSG_SLEEP_ENABLED:
        mDpCategoryEnabled[SMS_CATEGORY_SLEEP].Attach(pSubject);
        break;
      case SP_SSG_WORK_STARTTIME:
        mDpCategoryStarttime[SMS_CATEGORY_WORK].Attach(pSubject);
        break;
      case SP_SSG_OFF_STARTTIME:
        mDpCategoryStarttime[SMS_CATEGORY_OFF].Attach(pSubject);
        break;
      case SP_SSG_SLEEP_STARTTIME:
        mDpCategoryStarttime[SMS_CATEGORY_SLEEP].Attach(pSubject);
        break;
      case SP_SSG_1ST_STARTTIME:
        mDpPeriodStarttime[SMS_SCHEDULE_PERIOD_1].Attach(pSubject);
        break;
      case SP_SSG_2ND_STARTTIME:
        mDpPeriodStarttime[SMS_SCHEDULE_PERIOD_2].Attach(pSubject);
        break;
      case SP_SSG_3TH_STARTTIME:
        mDpPeriodStarttime[SMS_SCHEDULE_PERIOD_3].Attach(pSubject);
        break;
      case SP_SSG_1P_NO:
        mDpPrimaryNoId[PHONE_NO_1].Attach(pSubject);
        break;
      case SP_SSG_2P_NO:
        mDpPrimaryNoId[PHONE_NO_2].Attach(pSubject);
        break;
      case SP_SSG_3P_NO:
        mDpPrimaryNoId[PHONE_NO_3].Attach(pSubject);
        break;
      case SP_SSG_1S_NO:
        mDpSecondaryNoId[PHONE_NO_1].Attach(pSubject);
        break;
      case SP_SSG_2S_NO:
        mDpSecondaryNoId[PHONE_NO_2].Attach(pSubject);
        break;
      case SP_SSG_3S_NO:
        mDpSecondaryNoId[PHONE_NO_3].Attach(pSubject);
        break;
      case SP_SSG_PHONE_NO_1:
        mDpPhoneNo[PHONE_NO_1].Attach(pSubject);
        break;
      case SP_SSG_PHONE_NO_2:
        mDpPhoneNo[PHONE_NO_2].Attach(pSubject);
        break;
      case SP_SSG_PHONE_NO_3:
        mDpPhoneNo[PHONE_NO_3].Attach(pSubject);
        break;
      }
    }

    /*****************************************************************************
    * Function - ConnectToSubjects
    * DESCRIPTION: Implements the Observer::ConnectToSubjects
    *
    ****************************************************************************/
    void SmsScheduleGraphic::ConnectToSubjects(void)
    {
      mDpCurrentTypeOfSchedule.Subscribe(this);
      mDpWeekStartAtMonday.Subscribe(this);

      mDpCategoryEnabled[SMS_CATEGORY_WORK].Subscribe(this);
      mDpCategoryEnabled[SMS_CATEGORY_OFF].Subscribe(this);
      mDpCategoryEnabled[SMS_CATEGORY_SLEEP].Subscribe(this);

      mDpCategoryStarttime[SMS_CATEGORY_WORK].Subscribe(this);
      mDpCategoryStarttime[SMS_CATEGORY_OFF].Subscribe(this);
      mDpCategoryStarttime[SMS_CATEGORY_SLEEP].Subscribe(this);

      mDpPeriodStarttime[SMS_SCHEDULE_PERIOD_1].Subscribe(this);
      mDpPeriodStarttime[SMS_SCHEDULE_PERIOD_2].Subscribe(this);
      mDpPeriodStarttime[SMS_SCHEDULE_PERIOD_3].Subscribe(this);

      mDpPrimaryNoId[PHONE_NO_1].Subscribe(this);
      mDpPrimaryNoId[PHONE_NO_2].Subscribe(this);
      mDpPrimaryNoId[PHONE_NO_3].Subscribe(this);

      mDpSecondaryNoId[PHONE_NO_1].Subscribe(this);
      mDpSecondaryNoId[PHONE_NO_2].Subscribe(this);
      mDpSecondaryNoId[PHONE_NO_3].Subscribe(this);

      mDpPhoneNo[PHONE_NO_1].Subscribe(this);
      mDpPhoneNo[PHONE_NO_2].Subscribe(this);
      mDpPhoneNo[PHONE_NO_3].Subscribe(this);

    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    ****************************************************************************/
    void SmsScheduleGraphic::Run(void)
    {
      if (mUpdate)
      {
        mUpdate = false;

        if (mDpWeekStartAtMonday.IsUpdated())
        {
          SetWeekdayClientAreas( mDpWeekStartAtMonday->GetValue() );
        }

        // NB: don't reset updated-flag as flag is used in redraw later on
        if (mDpCurrentTypeOfSchedule.IsUpdated(false))
        {
          SMS_SCHEDULE_TYPE_TYPE schedule_type = mDpCurrentTypeOfSchedule->GetValue();

          switch (schedule_type)
          {
            case SMS_SCHEDULE_TYPE_WORK_OFF_SLEEP:
              SetHelpString(SID_HELP_4_3_4_8);
              break;
            case SMS_SCHEDULE_TYPE_PRIMARY_NO:
              SetHelpString(SID_HELP_4_3_4_9);
              break;
            case SMS_SCHEDULE_TYPE_SECONDARY_NO:
              SetHelpString(SID_HELP_4_3_4_10);
              break;
          }
        }

        Invalidate();
      }
      Frame::Run();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    ****************************************************************************/
    bool SmsScheduleGraphic::Redraw()
    {
      if ( mDpCurrentTypeOfSchedule.IsUpdated()
          || mDpPhoneNo[PHONE_NO_1].IsUpdated()
          || mDpPhoneNo[PHONE_NO_2].IsUpdated()
          || mDpPhoneNo[PHONE_NO_3].IsUpdated())
      {
        DrawLegend();
      }

      Frame::Redraw();

      DrawAxis();
      DrawGridLines();
      DrawGraph();
      
      return true;
    }

    /*****************************************************************************
    *
    *
    *              PRIVATE FUNCTIONS
    *
    *
    ****************************************************************************/

    /*****************************************************************************
    *
    *
    *              PROTECTED FUNCTIONS
    *
    ****************************************************************************/

    /*****************************************************************************
    * Function
    * DESCRIPTION: Draw vertical line for each weekday
    *
    ****************************************************************************/
    void SmsScheduleGraphic::DrawGridLines(void)
    {
      int i;

      for (i=FIRST_WEEKDAY; i<NO_OF_WEEKDAY; i++)
      {
        int x1;
        x1 = GRAPH_X1 + i*COL_WIDTH;

        GUI_SetColor(GetColour());
        GUI_SetLineStyle(GUI_LS_DOT);
        GUI_DrawLine(x1 + COL_WIDTH, GRAPH_Y1, x1 + COL_WIDTH, GRAPH_Y2-1);
      }
      GUI_SetLineStyle(GUI_LS_SOLID);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Set legend texts based on type of schedule
    ****************************************************************************/
    void SmsScheduleGraphic::DrawLegend(void)
    {
      SMS_SCHEDULE_TYPE_TYPE type = mDpCurrentTypeOfSchedule->GetValue();

      if (type == SMS_SCHEDULE_TYPE_WORK_OFF_SLEEP)
      {
        // the work/off/sleep schedule shows its categories in legend
        mpLegendTexts[0]->SetText( Languages::GetInstance()->GetString(SID_WORK_PERIOD) );
        mpLegendTexts[1]->SetText( Languages::GetInstance()->GetString(SID_OFF_PERIOD) );
        mpLegendTexts[2]->SetText( Languages::GetInstance()->GetString(SID_SLEEP_PERIOD) );

        for (int i=SMS_CATEGORY_WORK; i<=SMS_CATEGORY_SLEEP; i++)
        {
          mpLegendTexts[i]->SetAlign(GUI_TA_VCENTER+GUI_TA_HCENTER);

          mpLegendFrames[i]->SetVisible(true);
          mpLegendFrames[i]->Validate();
        }
      }
      else
      {
        // the primary and secondary schedules shows phone numbers in legend
        bool phone_no_is_used[3] = {false, false, false};

        for (int i=FIRST_PHONE_NO; i<=LAST_PHONE_NO; i++ )
        {

          for (U8 j=FIRST_SMS_SCHEDULE_PERIOD; j<=LAST_SMS_SCHEDULE_PERIOD; j++ )
          {
            //check if each phone numbers are used at all
            for (U8 day = FIRST_WEEKDAY; day < NO_OF_WEEKDAY; day++)
            {
              if (type == SMS_SCHEDULE_TYPE_PRIMARY_NO
                && mDpPrimaryNoId[j]->GetValue((int) day) == i)
              {
                phone_no_is_used[i] = true;
                break;
              }
              else if (type == SMS_SCHEDULE_TYPE_SECONDARY_NO
                && mDpSecondaryNoId[j]->GetValue((int) day) == i)
              {
                phone_no_is_used[i] = true;
                break;
              }
            }
          }
          mpLegendFrames[i]->SetVisible(phone_no_is_used[i]);

          mpLegendTexts[i]->SetText(mDpPhoneNo[i]->GetValue());
          mpLegendTexts[i]->SetAlign(GUI_TA_VCENTER+GUI_TA_RIGHT);

        }
      }
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    ****************************************************************************/
    void SmsScheduleGraphic::DrawAxis(void)
    {
      U8 i;

      GUI_SetColor(GetColour());
      GUI_SetLineStyle(GUI_LS_SOLID);

      // draw y axis
      GUI_DrawLine(GRAPH_X1,GRAPH_Y1,GRAPH_X1,GRAPH_Y2);

      // draw x axis
      GUI_DrawLine(GRAPH_X1,GRAPH_Y2,GRAPH_X2,GRAPH_Y2);

      // draw markings of y axis
      for (i = 0; i < NO_OF_TIME_OF_DAY_LABELS; i++)
      {
        int y_pos = GRAPH_Y1+TIME_OF_DAY_HEIGHT*i ;

        //draw long marking for each label
        GUI_DrawLine(GRAPH_X1-2, y_pos, GRAPH_X1+NO_OF_WEEKDAY*COL_WIDTH, y_pos);

        y_pos += (TIME_OF_DAY_HEIGHT/2);

        //draw short marking between labels
        if (i < NO_OF_TIME_OF_DAY_LABELS - 1)
        {
          GUI_DrawLine(GRAPH_X1-1, y_pos, GRAPH_X1, y_pos);
        }
      }

    }


    /*****************************************************************************
    * Function
    * DESCRIPTION: Sets ClientAreas of weekday Labels
    *
    ****************************************************************************/
    void SmsScheduleGraphic::SetWeekdayClientAreas(bool mondayIsFirstDayInWeek)
    {
      if ( FIRST_WEEKDAY != WEEKDAY_MONDAY )
      {
        FatalErrorOccured("Schedule graphic impl. requires monday as day 0");
      }

      if (mondayIsFirstDayInWeek)
      {
        for (U8 i=FIRST_WEEKDAY; i<NO_OF_WEEKDAY; i++)
        {
          mpWeekdayLabels[i]->SetClientArea(GRAPH_X1+1+i*COL_WIDTH, GRAPH_Y1-15, GRAPH_X1+COL_WIDTH-1+i*COL_WIDTH, GRAPH_Y1-2);
        }
      }
      else
      {
        mpWeekdayLabels[WEEKDAY_SUNDAY]->SetClientArea(GRAPH_X1+1, GRAPH_Y1-15, GRAPH_X1+COL_WIDTH-1, GRAPH_Y1-2);

        for (U8 i=1; i<NO_OF_WEEKDAY; i++)
        {
          mpWeekdayLabels[i-1]->SetClientArea(GRAPH_X1+1+i*COL_WIDTH, GRAPH_Y1-15, GRAPH_X1+COL_WIDTH-1+i*COL_WIDTH, GRAPH_Y1-2);
        }
      }
    }


    /*****************************************************************************
    * Function:
    * DESCRIPTION:
    *
    ****************************************************************************/
    U8 SmsScheduleGraphic::SecOfDay2Pixels(I32 seconds)
    {
      // these checks should never evaluate to true, but rather safe than sorry
      if (seconds > ONE_DAY_IN_SEC)
      {
        seconds = ONE_DAY_IN_SEC;
      }
      else if (seconds < 0)
      {
        seconds = 0;
      }

      float pixels = GRAPH_Y1 + (GRAPH_HEIGHT * (((float)seconds) / ONE_DAY_IN_SEC));

      return (U8) pixels;
    }

    /*****************************************************************************
    * Function: DrawGraph
    * DESCRIPTION:
    ****************************************************************************/
    void SmsScheduleGraphic::DrawGraph(void)
    {
      int i, j, day;

      int x1, x2;
      int y1, y2;
      int old_val, new_val;
      EventList e_list;
      int end_time;

      GUI_COLOR period_color = GUI_COLOUR_DEFAULT_BACKGROUND;
      SMS_SCHEDULE_TYPE_TYPE schedule_type = mDpCurrentTypeOfSchedule->GetValue();

      switch ( schedule_type)
      {
        case SMS_SCHEDULE_TYPE_WORK_OFF_SLEEP:
          for (i=FIRST_WEEKDAY; i<NO_OF_WEEKDAY; i++)
          {
            for (j=FIRST_SMS_SCHEDULE_PERIOD; j<=LAST_SMS_SCHEDULE_PERIOD; j++)
            {
              if (mDpCategoryEnabled[j]->GetValue(i))
              {
                e_list.AddEvent( mDpCategoryStarttime[j]->GetValue(i)+ONE_DAY_IN_SEC*i,j);
              }
            }
          }
          break;
        case SMS_SCHEDULE_TYPE_PRIMARY_NO:
          for (i=FIRST_WEEKDAY; i<NO_OF_WEEKDAY; i++)
          {
            for (j=FIRST_SMS_SCHEDULE_PERIOD; j<=LAST_SMS_SCHEDULE_PERIOD; j++)
            {
              e_list.AddEvent( mDpPeriodStarttime[j]->GetValue(i)+ONE_DAY_IN_SEC*i, mDpPrimaryNoId[j]->GetValue(i));
            }
          }
         break;
        case SMS_SCHEDULE_TYPE_SECONDARY_NO:
          for (i=FIRST_WEEKDAY; i<NO_OF_WEEKDAY; i++)
          {
            for (j=FIRST_SMS_SCHEDULE_PERIOD; j<=LAST_SMS_SCHEDULE_PERIOD; j++)
            {
              e_list.AddEvent( mDpPeriodStarttime[j]->GetValue(i)+ONE_DAY_IN_SEC*i, mDpSecondaryNoId[j]->GetValue(i));
            }
          }
          break;
        default:
          break;
      }

      e_list.SortEvents();

      //Draw graph
      if (mDpWeekStartAtMonday->GetValue())
      {
        old_val = e_list.GetValue(7*ONE_DAY_IN_SEC);
      }
      else
      {
        old_val = e_list.GetValue(6*ONE_DAY_IN_SEC);
      }

      for (i=FIRST_WEEKDAY; i<NO_OF_WEEKDAY; i++)
      {
        int e_start;
        int e_count;

        if (mDpWeekStartAtMonday->GetValue())
        {
          day = i;
        }
        else
        {
          day = (i+6)%7;
        }

        x1 = GRAPH_X1 + COL_WIDTH*i;
        x2 = x1+COL_WIDTH;
        y1 = GRAPH_Y1;

        e_list.QueryEvents( day*ONE_DAY_IN_SEC, (day+1)*ONE_DAY_IN_SEC-1, &e_start, &e_count);

        if (e_count>0)
        {
          for (j=0; j<e_count; j++)
          {
            e_list.GetEvent(e_start+j, &end_time, &new_val);

            y2 = SecOfDay2Pixels(end_time%ONE_DAY_IN_SEC);

            period_color = BACKCOLOR_BY_TYPE[old_val];
            // draw a colored rectangle using the calculated coordinates
            GUI_SetColor(GetColour());
            GUI_DrawRect(x1, y1, x2, y2);
            GUI_SetColor(period_color);
            GUI_FillRect(x1+1, y1 + 1 , x2-1, y2 - 1);

            y1 = y2;
            old_val = new_val;
          }
        }
        y2 = SecOfDay2Pixels(ONE_DAY_IN_SEC - 1);
        period_color = BACKCOLOR_BY_TYPE[old_val];
        // draw a colored rectangle using the calculated coordinates
        GUI_SetColor(GetColour());
        GUI_DrawRect(x1, y1, x2, y2);
        GUI_SetColor(period_color);
        GUI_FillRect(x1+1, y1 + 1 , x2-1, y2 - 1);
      }
    }
  } // namespace display
} // namespace mpc




































