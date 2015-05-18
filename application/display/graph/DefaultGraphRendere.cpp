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
/* CLASS NAME       : DefaultGraphRendere                                                  */
/*                                                                          */
/* FILE NAME        : DefaultGraphRendere.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a DefaultGraphRendere.                        */
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
#include <Component.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "DefaultGraphRendere.h"
#include "Axis.h"

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
  /*****************************************************************************
  * Function
  * DESCRIPTION:
  * Constructor
  *****************************************************************************/
  DefaultGraphRendere::DefaultGraphRendere()
  {
  }

  /*****************************************************************************
  * Function
  * DESCRIPTION:
  * Dectructor
  *****************************************************************************/
  DefaultGraphRendere::~DefaultGraphRendere()
  {
  }

  /*****************************************************************************
  * Function
  * DESCRIPTION:
  * Redraws this element. If it fails (for some reason) the
  * method returns false.
  *****************************************************************************/
  bool DefaultGraphRendere::Draw(Component* hwnd, Axis* xAxis, Axis* yAxis)
  {
    const float y_min = yAxis->GetMinValue();
    const float y_max = yAxis->GetMaxValue();
    const float x_min = xAxis->GetMinValue();
    const float x_max = xAxis->GetMaxValue();

    const int height = hwnd->GetHeight();
    const int width = hwnd->GetWidth();

    unsigned int x_points = xAxis->GetNumOfPoints();
    const float y_pixel_value = (float)height / (y_max - y_min);
    const float x_point_pixels = (float)(width - 1) / (x_points - 1); // point 0 dosn't count

    hwnd->SelectWindow();
    GUI_SetColor(hwnd->GetColour());


    const unsigned int y_data_group_count = yAxis->GetDataGroupCount();
    AxisData* data_group = NULL;
    float value = 0.0;
    float x = 0.0;
    float y = 0.0;

    for(int i=0; i < y_data_group_count; ++i)
    {
      data_group = yAxis->GetDataGroup(i);
      float last_x = 0;
      float last_y = height-1;
      x_points = data_group->GetNumOfPoints();
      for(unsigned int point = 0; point < x_points; ++point)
      {
        value = data_group->GetValue(point);
        y = (height - 1) - (int)((value - y_min) * y_pixel_value);
        x = (int)(x_point_pixels * point);
        if(point > 0)
        {
          GUI_SetColor(hwnd->GetColour());
          GUI_DrawLine(last_x,last_y,x,y);
        }
        last_x = x;
        last_y = y;
      }
    }

//    GUI_DrawVLine(hwnd->GetWidth()-1,0,hwnd->GetHeight()-1);
    hwnd->Validate();
    return true;
  }


    } // namespace graph
  }// namespace display
} // namespace mpc

