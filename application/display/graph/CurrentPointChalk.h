/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Plaform                                   */
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
/* CLASS NAME       : CurrentPointChalk                                     */
/*                                                                          */
/* FILE NAME        : CurrentPointChalk.h                                   */
/*                                                                          */
/* CREATED DATE     : 29-01-2008   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Uses two DataPoints to draw current working point                        */
/****************************************************************************/

/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __CURRENT_POINT_GHALK_H__
#define __CURRENT_POINT_GHALK_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/


/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <FloatDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <Chalk.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

typedef enum 
{
  QUADRANT_POSITION_TOP_LEFT,
  QUADRANT_POSITION_TOP_RIGHT,
  QUADRANT_POSITION_TOP_CENTRE,
  QUADRANT_POSITION_BOTTOM_LEFT,
  QUADRANT_POSITION_BOTTOM_RIGHT,
  QUADRANT_POSITION_BOTTOM_CENTRE,
  QUADRANT_POSITION_CENTRE_LEFT,
  QUADRANT_POSITION_CENTRE_RIGHT,
  QUADRANT_POSITION_CENTRE_CENTRE
} QUADRANT_POSITION_TYPE;

namespace mpc
{
  namespace display
  {

    class CurrentPointChalk : public Chalk
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      CurrentPointChalk(BlackBoard* pBlackBoard);
      
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~CurrentPointChalk();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /*****************************************************************************
       * FUNCTION - Redraw
       * DESCRIPTION: Redraws on the blackboard specified during construction
       *****************************************************************************/
      virtual void Redraw();

      virtual void SetDataPoints(FloatDataPoint* pCurrentXDP, FloatDataPoint* pCurrentYDP);

      virtual void SetTextPosition(QUADRANT_POSITION_TYPE textPosition);

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      void CalculateInternals(int width, int height);
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      QUADRANT_POSITION_TYPE mTextPosition;
      FloatDataPoint*        mpCurrentXDP;
      FloatDataPoint*        mpCurrentYDP;
      int                    mpCurrentXNQPosX;
      int                    mpCurrentXNQPosY;
      int                    mpCurrentYNQPosX;
      int                    mpCurrentYNQPosY;
    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    }; // class component

  } // namespace mpc
} // namespace display

#endif
