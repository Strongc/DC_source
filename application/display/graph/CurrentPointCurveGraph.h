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
/* FILE NAME        : CurrentPointCurveGraph.h                              */
/*                                                                          */
/* CREATED DATE     : 29-01-2008   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* A graph displaying current point and a curve                             */
/****************************************************************************/

/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __CURRENT_POINT_CURVE_GRAPH_H__
#define __CURRENT_POINT_CURVE_GRAPH_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <ObserverGroup.h>
#include <Number.h>
#include <Quantity.h>
#include <FloatDataPoint.h>
#include <BlackBoard.h>
#include <CurrentPointChalk.h>
#include <CrossChalk.h>
#include <CurrentPointCurveData.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/

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
    * CLASS: CurrentPointCurveGraph
    * DESCRIPTION:
    * Abstract base class for a piece of chalk.
    *****************************************************************************/
    class CurrentPointCurveGraph : public ObserverGroup
    {
    public:
     /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      CurrentPointCurveGraph(Component* pParent = NULL);
      
     /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~CurrentPointCurveGraph();
      
     /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

     /********************************************************************
      OPERATIONS
      ********************************************************************/

      // Observer overrides
      virtual void ConnectToSubjects();
      virtual void SetSubjectPointer(int Id, Subject* pSubject);
      virtual void Update(Subject* pSubject);

      // Component overrides
      virtual void Run();
      virtual bool Redraw();

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      BlackBoard*           mpBlackBoard;
      CurrentPointChalk*    mpCurrentPointChalk;
      CrossChalk*           mpCrossChalk;

      Quantity*             mpYAxis;
      Quantity*             mpXAxis;

      FloatDataPoint*       mpMaxXValue;
      FloatDataPoint*       mpMinXValue;
      FloatDataPoint*       mpMaxYValue;
      FloatDataPoint*       mpMinYValue;
      FloatDataPoint*       mpCurrentXValue;
      FloatDataPoint*       mpCurrentYValue;
      
      SubjectPtr<CurrentPointCurveData*> mpData;

      bool                  mRunOnce;

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      Number*               mpYAxisMax;
      Number*               mpYAxisMin;

      Number*               mpXAxisMax;
      Number*               mpXAxisMin;

    }; 

  } // namespace mpc
} // namespace display

#endif
