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
/* FILE NAME        : GUI_Utility.h                                         */
/*                                                                          */
/* CREATED DATE     : 2004-08-03                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Collection of small classes to help the GUI                              */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpcGUI_UTILITY_h
#define mpcGUI_UTILITY_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "GUI.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

namespace mpc
{
/*****************************************************************************
TYPE DEFINES
  *****************************************************************************/
  class Area;
  
  /*****************************************************************************
  * CLASS: Position
  * DESCRIPTION:
  * Defines a position on the display.
  * In the MPC display the upper left corner is (x = 0, y = 0) and the
  * lower right corner is (x = 239, y = 319).
  *
  * The position can also be relativ to another position. The absolute position
  * is then TheAbsolutePosition = [LedDisplay] +
  *                               1stRelativPosition +
  *                               2ndRelativPosition + ...
  *****************************************************************************/
  class Position
  {
  public:
  /********************************************************************
  LIFECYCLE - Default constructor.
    ********************************************************************/
    Position();
    Position(const Position& Pos);
    Position(const U16 X, const U16 Y);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~Position();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    Position& operator =(const Position& Pos);
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool operator ==(const Position& Pos);
    bool operator !=(const Position& Pos);
    Position operator +(const Position& Pos);
    Position operator -(const Position& Pos);
    void operator +=(const Position& Pos);
    void operator -=(const Position& Pos);
    void Set(const U16 X, const U16 Y);
    U16 GetX();
    U16 GetY();
    Position LcdDisplay();
    void BringPositionWithinArea(Area& Ar);
    bool IsPositionWithinArea(Area& Ar);
  private:
  /********************************************************************
  OPERATIONS
    ********************************************************************/
    
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    U16 mX;
    U16 mY;
  };
  
  
  /*****************************************************************************
  * CLASS: Area
  * DESCRIPTION:
  * Defines a area on the display.
  *****************************************************************************/
  class Area
  {
  public:
  /********************************************************************
  LIFECYCLE - Default constructor.
    ********************************************************************/
    Area();
    Area(const GUI_RECT& rect);
    Area(const Area& Ar);
    Area(const Position UpperLeft, const Position LowerRight);
    Area(const U16 UpperleftX, U16 UpperLeftY, U16 LowerRightX, U16 LowerRightY);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~Area();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    Area& operator =(const Area& Ar);
    Area& operator =(const GUI_RECT& Ar);
    
    operator GUI_RECT();
    bool operator ==(Area& Ar);
    bool operator !=(Area& Ar);
    Area operator +(Position& Pos);
    Area operator -(Position& Pos);
    void operator +=(Position& Pos);
    void operator -=(Position& Pos);
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void Set(U16 UpperleftX, U16 UpperLeftY, U16 LowerRightX, U16 LowerRightY);
    void Set(Position UpperLeft, Position LowerRigth);
    U16 GetWidth();
    U16 GetHeight();
    void SetWidth(U16 Width);
    void SetHeight(U16 Height);
    Position GetUpperLeft();
    Position GetLowerRight();
    void BringWithinArea(Area& Ar);
  private:
  /********************************************************************
  OPERATIONS
    ********************************************************************/
    
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    Position mUpperLeft;
    Position mLowerRight;
  };
  
}

#endif
