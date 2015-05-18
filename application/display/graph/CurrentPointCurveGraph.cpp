/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : CurrentPointCurveGraph                                */
/*                                                                          */
/* FILE NAME        : CurrentPointCurveGraph.cpp                            */
/*                                                                          */
/* CREATED DATE     : 29-01-2008   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* See header file                                                          */
/*                                                                          */
/****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <MPCFonts.h>

/*****************************************************************************
LOCAL INCLUDES
*****************************************************************************/
#include <CurrentPointCurveGraph.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
#define Y_AXIS_LABLES_WIDTH   36
#define X_AXIS_LABLES_WIDTH   30
#define Y_AXIS_LABLE_Y_TOP     8
#define X_AXIS_LABLES_HEIGHT  13
#define BLACKBOARD_HEIGHT    116
#define BLACKBOARD_WIDTH     184

namespace mpc
{
  namespace display
  {

    /*****************************************************************************
    * LIFECYCLE - Constructor
    *****************************************************************************/
    CurrentPointCurveGraph::CurrentPointCurveGraph(Component* pParent): ObserverGroup(pParent)
    {
      mRunOnce = true;

      mpCurrentXValue = new FloatDataPoint();
      mpCurrentYValue = new FloatDataPoint();

      mpBlackBoard = new BlackBoard(this);
      mpBlackBoard->SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      mpBlackBoard->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpBlackBoard->SetClientArea(Y_AXIS_LABLES_WIDTH + 3, 15, Y_AXIS_LABLES_WIDTH + BLACKBOARD_WIDTH + 3 - 1, 15 + BLACKBOARD_HEIGHT);
      mpBlackBoard->SetVisible(true);
      AddChild(mpBlackBoard);

      mpCrossChalk = new CrossChalk(mpBlackBoard);
      mpCurrentPointChalk = new CurrentPointChalk(mpBlackBoard);
      mpCurrentPointChalk->SetTextPosition(QUADRANT_POSITION_TOP_CENTRE);

      mpXAxisMax = new Number(this);
      mpXAxisMax->SetAlign(GUI_TA_VCENTER | GUI_TA_RIGHT);
      mpXAxisMax->SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      mpXAxisMax->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpXAxisMax->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);
      mpXAxisMax->SetHeight(X_AXIS_LABLES_HEIGHT);
      mpXAxisMax->SetNumberOfDigits(3);
      mpXAxisMax->SetTransparent(false);
      mpXAxisMax->SetWidth(X_AXIS_LABLES_WIDTH);
      AddChild(mpXAxisMax);
      mpXAxisMax->SetVisible(true);

      mpXAxisMin = new Number(this);
      mpXAxisMin->SetAlign(GUI_TA_VCENTER | GUI_TA_LEFT);
      mpXAxisMin->SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      mpXAxisMin->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpXAxisMin->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);
      mpXAxisMin->SetHeight(X_AXIS_LABLES_HEIGHT);
      mpXAxisMin->SetNumberOfDigits(3);
      mpXAxisMin->SetTransparent(false);
      mpXAxisMin->SetWidth(X_AXIS_LABLES_WIDTH);
      AddChild(mpXAxisMin);
      mpXAxisMin->SetVisible(true);

      mpXAxis    = new Quantity(this);
      mpXAxis->SetAlign(GUI_TA_VCENTER | GUI_TA_RIGHT);
      mpXAxis->SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      mpXAxis->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpXAxis->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);
      mpXAxis->SetHeight(X_AXIS_LABLES_HEIGHT);
      mpXAxis->SetTransparent(false);
      mpXAxis->SetWidth(X_AXIS_LABLES_WIDTH);
      AddChild(mpXAxis);
      mpXAxis->SetVisible(true);

      mpYAxisMax = new Number(this);
      mpYAxisMax->SetAlign(GUI_TA_VCENTER | GUI_TA_RIGHT);
      mpYAxisMax->SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      mpYAxisMax->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpYAxisMax->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);
      mpYAxisMax->SetHeight(X_AXIS_LABLES_HEIGHT);
      mpYAxisMax->SetNumberOfDigits(2);
      mpYAxisMax->SetTransparent(false);
      mpYAxisMax->SetWidth(Y_AXIS_LABLES_WIDTH);
      AddChild(mpYAxisMax);
      mpYAxisMax->SetVisible(true);

      mpYAxisMin = new Number(this);
      mpYAxisMin->SetAlign(GUI_TA_VCENTER | GUI_TA_RIGHT);
      mpYAxisMin->SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      mpYAxisMin->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpYAxisMin->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);
      mpYAxisMin->SetHeight(X_AXIS_LABLES_HEIGHT);
      mpYAxisMin->SetNumberOfDigits(2);
      mpYAxisMin->SetTransparent(false);
      mpYAxisMin->SetWidth(Y_AXIS_LABLES_WIDTH);
      AddChild(mpYAxisMin);
      mpYAxisMin->SetVisible(true);

      mpYAxis    = new Quantity(this);
      mpYAxis->SetAlign(GUI_TA_VCENTER | GUI_TA_RIGHT);
      mpYAxis->SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      mpYAxis->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpYAxis->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);
      mpYAxis->SetHeight(X_AXIS_LABLES_HEIGHT);
      mpYAxis->SetTransparent(false);
      mpYAxis->SetWidth(Y_AXIS_LABLES_WIDTH);
      AddChild(mpYAxis);
      mpYAxis->SetVisible(true);

      mpMaxXValue = new FloatDataPoint;
      mpMinXValue = new FloatDataPoint;
      mpMaxYValue = new FloatDataPoint;
      mpMinYValue = new FloatDataPoint;
    }

    /*****************************************************************************
    * LIFECYCLE - Destructor
    *****************************************************************************/
    CurrentPointCurveGraph::~CurrentPointCurveGraph()
    {
      delete mpCrossChalk;
      delete mpCurrentPointChalk;
      delete mpBlackBoard;
      delete mpCurrentXValue;
      delete mpCurrentYValue;
      delete mpMaxXValue;
      delete mpMinXValue;
      delete mpMaxYValue;
      delete mpMinYValue;
    }

    /*****************************************************************************
    * FUNCTION - SetSubjectPointer
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CurrentPointCurveGraph::SetSubjectPointer(int Id, Subject* pSubject)
    {
      mpData.Attach(pSubject);
    }

    /*****************************************************************************
    * FUNCTION - ConnectToSubjects
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CurrentPointCurveGraph::ConnectToSubjects(void)
    {
      mpData->Subscribe(this);
    }

    void CurrentPointCurveGraph::Update(Subject* pSubject)
    {
      mpData.Update(pSubject);
      ObserverGroup::Update(pSubject);
    }

    /*****************************************************************************
    * FUNCTION - Run
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CurrentPointCurveGraph::Run()
    {
      if (mRunOnce)
      {
        mpXAxisMax->SetSubjectPointer(0, mpMaxXValue);
        mpXAxisMin->SetSubjectPointer(0, mpMinXValue);
        mpXAxis->SetSubjectPointer(0, mpCurrentXValue);

        mpYAxisMax->SetSubjectPointer(0, mpMaxYValue);
        mpYAxisMin->SetSubjectPointer(0, mpMinYValue);
        mpYAxis->SetSubjectPointer(0, mpCurrentYValue);

        mpXAxisMax->ConnectToSubjects();
        mpXAxisMin->ConnectToSubjects();
        mpXAxis->ConnectToSubjects();

        mpYAxisMax->ConnectToSubjects();
        mpYAxisMin->ConnectToSubjects();
        mpYAxis->ConnectToSubjects();

        mpCurrentPointChalk->SetDataPoints(mpCurrentXValue, mpCurrentYValue);
        mpCrossChalk->SetData(mpData.GetSubject());

        mpYAxisMax->SetChildPos(1, mpBlackBoard->GetChildPosY() - 7);
        mpYAxisMin->SetChildPos(1, (mpBlackBoard->GetHeight() + mpBlackBoard->GetChildPosY() - 1) - 7);
        mpYAxis->SetChildPos(1, X_AXIS_LABLES_HEIGHT + mpBlackBoard->GetChildPosY() - 7);

        mpXAxisMin->SetChildPos(mpBlackBoard->GetChildPosX(),  mpBlackBoard->GetHeight() + mpBlackBoard->GetChildPosY() + 2);
        mpXAxisMax->SetChildPos(mpBlackBoard->GetChildPosX() + mpBlackBoard->GetWidth() - Y_AXIS_LABLES_WIDTH + 7, mpBlackBoard->GetHeight() + mpBlackBoard->GetChildPosY() + 2);
        mpXAxis->SetChildPos(mpBlackBoard->GetChildPosX() + mpBlackBoard->GetWidth() - Y_AXIS_LABLES_WIDTH + 7, mpBlackBoard->GetHeight() + mpBlackBoard->GetChildPosY() + X_AXIS_LABLES_HEIGHT + 2);

        mRunOnce = false;
      }

      if (mpData.IsUpdated())
      {
        mpMaxXValue->SetQuantity(mpData->GetXQuantity());
        mpMaxXValue->SetMaxValue(mpData->GetMaxXValue());
        mpMaxXValue->SetMinValue(mpData->GetMaxXValue());
        mpMaxXValue->SetValue(mpData->GetMaxXValue());

        mpMinXValue->SetQuantity(mpData->GetXQuantity());
        mpMinXValue->SetMaxValue(mpData->GetMinXValue());
        mpMinXValue->SetMinValue(mpData->GetMinXValue());
        mpMinXValue->SetValue(mpData->GetMinXValue());

        mpMaxYValue->SetQuantity(mpData->GetYQuantity());
        mpMaxYValue->SetMaxValue(mpData->GetMaxYValue());
        mpMaxYValue->SetMinValue(mpData->GetMaxYValue());
        mpMaxYValue->SetValue(mpData->GetMaxYValue());

        mpMinYValue->SetQuantity(mpData->GetYQuantity());
        mpMinYValue->SetMaxValue(mpData->GetMinYValue());
        mpMinYValue->SetMinValue(mpData->GetMinYValue());
        mpMinYValue->SetValue(mpData->GetMinYValue());

        mpCurrentXValue->SetQuantity(mpData->GetXQuantity());
        mpCurrentYValue->SetQuantity(mpData->GetYQuantity());

        mpCurrentXValue->SetMaxValue(mpData->GetMaxXValue());
        mpCurrentXValue->SetMinValue(mpData->GetMinXValue());
        mpCurrentYValue->SetMaxValue(mpData->GetMaxYValue());
        mpCurrentYValue->SetMinValue(mpData->GetMinYValue());

        if (mpData->GetCurrentXQuality() == DP_AVAILABLE)
        {
          mpCurrentXValue->SetValue(mpData->GetCurrentXValue());
        }
        else
        {
          mpCurrentXValue->SetQuality(mpData->GetCurrentXQuality());
        }

        if (mpData->GetCurrentYQuality() == DP_AVAILABLE)
        {
          mpCurrentYValue->SetValue(mpData->GetCurrentYValue());
        }
        else
        {
          mpCurrentYValue->SetQuality(mpData->GetCurrentYQuality());
        }
      }

      ObserverGroup::Run();
    }

    /*****************************************************************************
    * FUNCTION - Redraw
    * DESCRIPTION: See header file
    *****************************************************************************/
    bool CurrentPointCurveGraph::Redraw()
    {
      if (!IsValid())
      {
        int width = GetWidth();
        int x_axis_arrow_pos_x = width-3-3;
        int y_axis_pos_x = Y_AXIS_LABLES_WIDTH+2;
        int height = GetHeight();
        int graph_bottom = mpBlackBoard->GetHeight() + mpBlackBoard->GetChildPosY() - 1;

        ObserverGroup::Redraw();

        GUI_SetLineStyle(GUI_LS_SOLID);
        GUI_SetColor(GetColour());
        GUI_DrawRect(0,0,width-1,height-1);

        // Y-axis
        GUI_DrawLine(y_axis_pos_x, graph_bottom + 1, y_axis_pos_x, 3);

        // X-axis
        GUI_DrawLine(y_axis_pos_x, graph_bottom + 1, width - 3, graph_bottom + 1);

        // X-axis arrow
        GUI_DrawLine(x_axis_arrow_pos_x, graph_bottom + 1 - 1, x_axis_arrow_pos_x + 2, graph_bottom + 1 - 1);
        GUI_DrawLine(x_axis_arrow_pos_x, graph_bottom + 1 + 1, x_axis_arrow_pos_x + 2, graph_bottom + 1 + 1);
        GUI_DrawLine(x_axis_arrow_pos_x, graph_bottom + 1 - 2, x_axis_arrow_pos_x + 1, graph_bottom + 1 - 2);
        GUI_DrawLine(x_axis_arrow_pos_x, graph_bottom + 1 + 2, x_axis_arrow_pos_x + 1, graph_bottom + 1 + 2);

        // Y-axis arrow
        GUI_DrawLine(y_axis_pos_x-1, 6, y_axis_pos_x-1, 4);
        GUI_DrawLine(y_axis_pos_x+1, 6, y_axis_pos_x+1, 4);
        GUI_DrawLine(y_axis_pos_x-2, 6, y_axis_pos_x-2, 5);
        GUI_DrawLine(y_axis_pos_x+2, 6, y_axis_pos_x+2, 5);

/*        GUI_DrawLine(mpBlackBoard->GetChildPosX()-2,mpBlackBoard->GetChildPosY(),mpBlackBoard->GetChildPosX(),mpBlackBoard->GetChildPosY());
        GUI_DrawLine(mpBlackBoard->GetChildPosX()-2,(mpBlackBoard->GetHeight() + mpBlackBoard->GetChildPosY() - 1),
                     mpBlackBoard->GetChildPosX(),(mpBlackBoard->GetHeight() + mpBlackBoard->GetChildPosY() - 1));*/
      }
      return true;
    }


  } // namespace mpc
} // namespace display

