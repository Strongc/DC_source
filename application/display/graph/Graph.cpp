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
/* CLASS NAME       : Graph                                                  */
/*                                                                          */
/* FILE NAME        : Graph.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a Graph.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "Graph.h"
#include <MpcFonts.h>
#include "DefaultGraphRendere.h"
#include "Axis.h"
#include "AxisData.h"
/*****************************************************************************
LOCAL INCLUDES
*****************************************************************************/

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
    namespace graph
    {
      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Constructor
      ***********************************************************************/
      Graph::Graph(Component* pParent) : Component(pParent)
      {
        mpGraphRenderArea = new Component(this);
        mpGraphRenderArea->SetTransparent();
        mpGraphRenderArea->SetVisible();

        mpXAxis = new Axis(X_AXIS);
        mpXAxis->SetMinValue(0.0);
        mpXAxis->SetMaxValue(10.0);
        mpXAxis->SetGroups(10);

        mpYAxis = new Axis(Y_AXIS);
        mpYAxis->SetMinValue(0.0);
        mpYAxis->SetMaxValue(100.0);
        mpYAxis->SetGroups(10);

        mpRendere = new DefaultGraphRendere();
        mLegalKeys = MPC_PLUS_KEY | MPC_PLUS_KEY_REP | MPC_MINUS_KEY|MPC_MINUS_KEY_REP;
        mLedsStatus = PLUS_LED|MINUS_LED;
        mCursorPoint = 0;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Dectructor
      ***********************************************************************/
      Graph::~Graph()
      {
        if(mpRendere != NULL)
          delete mpRendere;
        delete mpXAxis;
        delete mpYAxis;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Renders the graph to the mpGraphRenderArea component.
      ***********************************************************************/
      bool Graph::Redraw()
      {
        Component::Redraw();
//        mpGraphRenderArea->Redraw();

        int i = 0;
        int height = GetHeight();
        int width = GetWidth();

        int x_off = 30;
        int top_off = 23;
        int bottom_off = 20;
        int right_off = 20;

        mpGraphRenderArea->SetColour(mColour);
        mpGraphRenderArea->SetBackgroundColour(mBackgroundColour);
        mpGraphRenderArea->SetClientArea(x_off,top_off,width-right_off,height-bottom_off);


        SelectWindow();

        // Draw the Y axis.
        GUI_SetColor(mColour);
        GUI_SetBkColor(mBackgroundColour);
//        GUI_SetFont(DEFAULT_FONT_11);
        GUI_DrawVLine(x_off,top_off - 5, height - bottom_off + 5);
        // Draw arrow
//        GUI_DrawLine(x_off-3,top_off-2,x_off,top_off-5);
//        GUI_DrawLine(x_off+3,top_off-2,x_off,top_off-5);
        int groups = mpYAxis->GetGroups();
        float pixels_pr_group = (float)mpGraphRenderArea->GetHeight() / groups;
        float group_value = (mpYAxis->GetMaxValue() - mpYAxis->GetMinValue()) / groups;
        float value = mpYAxis->GetMaxValue();
        float y = top_off;
        char szTmp[40];
        GUI_RECT rect;
        for(i=0; i < groups; ++i)
        {
          GUI_DrawHLine(y, x_off - 3, x_off + 3);
          sprintf(szTmp, "%0.1f", value);
          GUI_GetTextExtend(&rect,szTmp,strlen(szTmp));
          GUI_DispStringAt(szTmp,x_off - 6 - (rect.x1 - rect.x0), y - ((rect.y1-rect.y0)/2));
          value -= group_value;
          y += pixels_pr_group;
        }

        // draw the x axis
        GUI_DrawHLine(height - bottom_off, x_off - 5, width-right_off+5);
        // Draw arrow
//        GUI_DrawLine(width - right_off + 2, height - bottom_off + 3, width - right_off+5, height - bottom_off);
//        GUI_DrawLine(width - right_off + 2, height - bottom_off - 3, width - right_off+5, height - bottom_off);

        groups = mpXAxis->GetGroups();
        pixels_pr_group = (float)(mpGraphRenderArea->GetWidth()-1) / groups;
        float x = x_off + pixels_pr_group;
        group_value = (mpXAxis->GetMaxValue() - mpXAxis->GetMinValue()) / groups;
        value = mpXAxis->GetMinValue() + group_value;
        y = height - bottom_off + 5;
        for(i=0; i < groups;++i)
        {
          GUI_DrawVLine(x, height-bottom_off-3, height-bottom_off+3);

          sprintf(szTmp, "%0.1f", value);
          GUI_GetTextExtend(&rect,szTmp,strlen(szTmp));
          GUI_DispStringAt(szTmp,x - ((rect.x1 - rect.x0)/2), y);
          value += group_value;

          x += pixels_pr_group;
        }


        Invalidate();
        mpRendere->Draw(mpGraphRenderArea, mpXAxis,mpYAxis);

        SelectWindow();

        const char* p_x_axis_name = mpXAxis->GetName();
        GUI_GetTextExtend(&rect,p_x_axis_name,strlen(p_x_axis_name));
        y = height - bottom_off - (rect.y1-rect.y0)-5;
        x = GetWidth() - (rect.x1-rect.x0) - 1;
        GUI_DispStringAt(p_x_axis_name, x, y);

        const char* p_y_axis_name = mpYAxis->GetName();
        GUI_GetTextExtend(&rect,p_y_axis_name,strlen(p_y_axis_name));
        y = top_off - (rect.y1-rect.y0) - 8;
        x = x_off - ((rect.x1-rect.x0));
        GUI_DispStringAt(p_y_axis_name, x, y);

        if(mCursorPoint > 0)
        {
          unsigned int noOfPoints = mpXAxis->GetNumOfPoints();
          float pixels_pr_point = (float)(mpGraphRenderArea->GetWidth() - 1) / (noOfPoints-1);
          x = pixels_pr_point * mCursorPoint;
          x = x + x_off;
          y = top_off;

          value = mpYAxis->GetDataGroup((unsigned int)0)->GetValue(mCursorPoint);
          float x_value = (float)((mpXAxis->GetMaxValue() - mpXAxis->GetMinValue()) * mCursorPoint) / mpXAxis->GetNumOfPoints();
          sprintf(szTmp,"(%0.2f),(%0.1f)", x_value, value );
          GUI_DispStringAt(szTmp, x + 2, y - 12);

          height = (mpGraphRenderArea->GetHeight() / 20) + 1;
          for(i=0; i < height; ++i)
          {
            GUI_DrawVLine(x,y,y+10);
            y+=20;
          }
        }
        Validate();
        return true;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets the rendere for the graph. The rendere is responsible for drawing
      * the lines between the values in the graph. When the graph is done 
      * using the rendere it is auto deleted. DO NOT PASS renderes allocated
      * on the stack. DO NOT delete the rendere.
      ***********************************************************************/
      void Graph::SetGraphRendere(GraphRendere* pRender)
      {
        if(mpRendere != NULL)
          delete mpRendere;
        mpRendere = pRender;
      }


      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets the data for a graph. Values passed is considered to be y values.
      ***********************************************************************/
/*
      void Graph::SetData(const char* groupName, vector<float>& data)
      {
        int size = data.size();
        if(size > mpXAxis->GetNumOfPoints())
          mpXAxis->SetNumOfPoints(size);
        mpYAxis->RemoveDataGroup(groupName);
        mpYAxis->AddDataGroup(groupName,data);
        Invalidate();
      }

      void Graph::RemoveData(const char* groupName)
      {
        mpXAxis->RemoveDataGroup(groupName);
      }
*/

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets the data for a graph. Values passed is considered to be y values.
      ***********************************************************************/
      Axis* Graph::GetAxis(AxisType type)
      {
        switch(type)
        {
          case X_AXIS:
            return mpXAxis;
          case Y_AXIS:
            return mpYAxis;
        }
        return NULL;
      }
      
      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets the data for a graph. Values passed is considered to be y values.
      ***********************************************************************/
      void Graph::SetAxis(Axis* pAxis)
      {
        // Nooo
        if(pAxis == NULL)
          return;

        switch(pAxis->GetType())
        {
        case X_AXIS:
          delete mpXAxis;
          mpXAxis = pAxis;
          break;
        case Y_AXIS:
          delete mpYAxis;
          mpYAxis = pAxis;
          break;
        }
      }

      void Graph::SetCursorPosition(unsigned int xAxisPoint)
      {
        mCursorPoint = xAxisPoint;
        Invalidate();
      }

      bool Graph::HandleKeyEvent(Keys KeyID)
      {
        switch(KeyID)
        {
        case MPC_PLUS_KEY:
          SetCursorPosition(mCursorPoint+1);
          Invalidate();
          break;
        case MPC_MINUS_KEY:
          SetCursorPosition(mCursorPoint-1);
          Invalidate();
          break;
        case MPC_PLUS_KEY_REP:
          SetCursorPosition( mCursorPoint + (mpXAxis->GetNumOfPoints() *0.05));
          Invalidate();
          break;
        case MPC_MINUS_KEY_REP:
          SetCursorPosition( mCursorPoint - (mpXAxis->GetNumOfPoints() *0.05));
          Invalidate();
          break;
        }
        
        return true;
      }

#ifdef __PC__
      void Graph::CalculateStringWidths()
      {
        Component::CalculateStringWidths();
      }

#endif
    } // namespace graph
  }// namespace display
} // namespace mpc

