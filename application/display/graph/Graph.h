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
/* CLASS NAME       : Graph                                                 */
/*                                                                          */
/* FILE NAME        : Graph.h                                               */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a Graph with an X and Y axis   */
/****************************************************************************/

/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayGraph_h
#define mpc_displayGraph_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Component.h>
#include <Keys.h>
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/

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
      class GraphRendere;
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a Graph with an x and y axis and
    * a set of data.
    *
    *****************************************************************************/
    class Graph : public Component
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      Graph(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~Graph();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      virtual Axis* GetAxis(AxisType type);
      virtual void SetAxis(Axis* pAxis);
/*
      virtual void SetData(const char* groupName, vector<float>& data);
      virtual void RemoveData(const char* groupName);
*/
      virtual void SetGraphRendere(GraphRendere*  pRender);
      virtual void SetCursorPosition(unsigned int xAxisPoint);

      virtual bool HandleKeyEvent(Keys KeyID);

#ifdef __PC__
      virtual void CalculateStringWidths();
#endif

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      Axis*  mpXAxis;
      Axis*  mpYAxis;

      Component*  mpGraphRenderArea; // Pointer to area for drawing the graph
      GraphRendere* mpRendere;
      unsigned int  mCursorPoint;
    };
    } // namespace graph
  }// namespace display
} // namespace mpc

#endif
