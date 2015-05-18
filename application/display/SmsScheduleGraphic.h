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
/* FILE NAME        : SmsScheduleGraphic.h                                  */
/*                                                                          */
/* CREATED DATE     : 2007-12-03                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __SmsScheduleGraphic_h
#define __SmsScheduleGraphic_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <Frame.h>
#include <Observer.h>
#include <SubjectPtr.h>
#include <BoolDataPoint.h>
#include <BoolVectorDataPoint.h>
#include <U8DataPoint.h>
#include <U8VectorDataPoint.h>
#include <I32VectorDataPoint.h>
#include <EnumDataPoint.h>
#include <StringDataPoint.h>
#include <Label.h>
#include <Text.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define NUMBER_OF_GRAPH_POINTS (7*24)
#define NO_OF_TIME_OF_DAY_LABELS 13
#define MAX_ELEMENTS 21

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
namespace mpc
{
  namespace display
  {

    const GUI_COLOR BACKCOLOR_BY_TYPE[3] = {GUI_COLOUR_GRAPH_1_BACK, GUI_COLOUR_GRAPH_2_BACK, GUI_COLOUR_GRAPH_3_BACK};
    const GUI_COLOR FORECOLOR_BY_TYPE[3] = {GUI_COLOUR_GRAPH_1_FORE, GUI_COLOUR_GRAPH_2_FORE, GUI_COLOUR_GRAPH_3_FORE};

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class SmsScheduleGraphic: public Frame, public Observer
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      SmsScheduleGraphic(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      ~SmsScheduleGraphic();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void SubscribtionCancelled(Subject* pSubject);
      virtual void Update(Subject* pSubject);
      virtual void SetSubjectPointer(int Id,Subject* pSubject);
      virtual void ConnectToSubjects(void);
      virtual bool Redraw();
      virtual void Run(void);

    private:
      /********************************************************************
      TYPES
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/


      Text*   mpTimeOfDayTexts[NO_OF_TIME_OF_DAY_LABELS];
      Label*  mpWeekdayLabels[NO_OF_WEEKDAY];
      Frame*  mpLegendFrames[NO_OF_SMS_CATEGORY-1];
      Text*  mpLegendTexts[NO_OF_SMS_CATEGORY-1];
      SubjectPtr<EnumDataPoint<SMS_SCHEDULE_TYPE_TYPE>*> mDpCurrentTypeOfSchedule;
      SubjectPtr<BoolDataPoint*>        mDpWeekStartAtMonday;
      SubjectPtr<BoolVectorDataPoint*>  mDpCategoryEnabled[NO_OF_SMS_CATEGORY-1];
      SubjectPtr<I32VectorDataPoint*>   mDpCategoryStarttime[NO_OF_SMS_CATEGORY-1];
      SubjectPtr<I32VectorDataPoint*>   mDpPeriodStarttime[NO_OF_SMS_SCHEDULE_PERIOD];
      SubjectPtr<U8VectorDataPoint*>    mDpPrimaryNoId[NO_OF_SMS_SCHEDULE_PERIOD];
      SubjectPtr<U8VectorDataPoint*>    mDpSecondaryNoId[NO_OF_SMS_SCHEDULE_PERIOD];
      SubjectPtr<StringDataPoint*>      mDpPhoneNo[NO_OF_PHONE_NO];

      bool    mUpdate;

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void DrawAxis(void);
      virtual void DrawLegend(void);
      virtual void DrawGridLines(void);
      virtual void SetWeekdayClientAreas(bool MondayIsFirstDayInWeek);
      virtual void DrawGraph();

      virtual U8 SecOfDay2Pixels(I32 seconds);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/


    };
  } // namespace display
} // namespace mpc


#endif
