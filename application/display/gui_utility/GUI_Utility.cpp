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
/* FILE NAME        : GUI_Utility.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 2004-08-04                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Collection of small classes to help the GUI                              */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "GUI.h"
#include "GUI_Utility.h"
/*****************************************************************************
DEFINES
*****************************************************************************/
#define UPPERLEFT_X 0
#define UPPERLEFT_Y 0
#define LOWER_RIGHT_X 239
#define LOWER_RIGHT_Y 319

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
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
  * Default contstructor for class Position
  *****************************************************************************/
  Position::Position()
  {
    mX = 0;
    mY = 0;
  }
  
  /*****************************************************************************
  * Function - Constructor
  * DESCRIPTION:
  * Copy constructor for class Position
  *****************************************************************************/
  Position::Position(const Position& Pos)
  {
    mX = Pos.mX;
    mY = Pos.mY;
  }
  
  /*****************************************************************************
  * Function - Constructor
  * DESCRIPTION:
  * Constructor using a x- and y coordinat
  *****************************************************************************/
  Position::Position(const U16 X, const U16 Y)
  {
    mX = X;
    mY = Y;
  }
  
  /*****************************************************************************
  * Function - Destructor
  * DESCRIPTION:
  * Default destructor
  *****************************************************************************/
  Position::~Position()
  {
    
  }
  
  /*****************************************************************************
  * Function - operator =
  * DESCRIPTION:
  * The assigment operator
  *****************************************************************************/
  Position& Position::operator =(const Position& Pos)
  {
    mX = Pos.mX;
    mY = Pos.mY;
    return *this;
  }
  
  /*****************************************************************************
  * Function - operator ==
  * DESCRIPTION:
  *
  *****************************************************************************/
  bool Position::operator ==(const Position& Pos)
  {
    return (mX == Pos.mX) && (mY == Pos.mY);
  }
  
  /*****************************************************************************
  * Function - operator !=
  * DESCRIPTION:
  *
  *****************************************************************************/
  bool Position::operator !=(const Position& Pos)
  {
    return !((mX == Pos.mX) && (mY == Pos.mY));
  }
  
  /*****************************************************************************
  * Function - operator +
  * DESCRIPTION:
  *
  *****************************************************************************/
  Position Position::operator +(const Position& Pos)
  {
    return Position(mX + Pos.mX, mY + Pos.mY);
  }
  
  /*****************************************************************************
  * Function - operator -
  * DESCRIPTION:
  *
  *****************************************************************************/
  Position Position::operator -(const Position& Pos)
  {
    return Position(mX - Pos.mX, mY - Pos.mY);
  }
  
  /*****************************************************************************
  * Function - operator +=
  * DESCRIPTION:
  *
  *****************************************************************************/
  void Position::operator +=(const Position& Pos)
  {
    mX += Pos.mX;
    mY += Pos.mY;
  }
  
  /*****************************************************************************
  * Function - operator -=
  * DESCRIPTION:
  *
  *****************************************************************************/
  void Position::operator -=(const Position& Pos)
  {
    mX -= Pos.mX;
    mY -= Pos.mY;
  }
  
  /*****************************************************************************
  * Function - Set
  * DESCRIPTION:
  * Sets the position to the coordinates given by X and Y
  *****************************************************************************/
  void Position::Set(const U16 X, const U16 Y)
  {
    mX = X;
    mY = Y;
  }
  
  /*****************************************************************************
  * Function - GetX
  * DESCRIPTION:
  * Returns the x-coordinate of the Position
  *****************************************************************************/
  U16 Position::GetX()
  {
    return mX;
  }
  
  /*****************************************************************************
  * Function - GetY
  * DESCRIPTION:
  * Returns the y-coordinate of the Position
  *****************************************************************************/
  U16 Position::GetY()
  {
    return mY;
  }
  
  /*****************************************************************************
  * Function - LcdDisplay
  * DESCRIPTION:
  * Returns the initial position of the display
  *****************************************************************************/
  Position  Position::LcdDisplay()
  {
    return Position(UPPERLEFT_X,UPPERLEFT_Y);
  }
  
  /*****************************************************************************
  * Function - BringPositionWithinArea
  * DESCRIPTION:
  * Brings the Position within the area
  *****************************************************************************/
  void Position::BringPositionWithinArea(Area& Ar)
  {
    Position UpperLeftPos  = Ar.GetUpperLeft();
    Position LowerRightPos = Ar.GetLowerRight();
    
    if (mX < UpperLeftPos.GetX())
    {
      mX = UpperLeftPos.GetX();
    }
    else if (mX > LowerRightPos.GetX())
    {
      mX = LowerRightPos.GetX();
    }
    
    if (mY < UpperLeftPos.GetY())
    {
      mY = UpperLeftPos.GetY();
    }
    else if (mY > LowerRightPos.GetY())
    {
      mY = LowerRightPos.GetY();
    }
  }
  
  /*****************************************************************************
  * Function - IsPositionWithinArea
  * DESCRIPTION:
  * Return whether the Position is within the area
  *****************************************************************************/
  
  bool Position::IsPositionWithinArea(Area& Ar)
  {
    return (mX <= Ar.GetLowerRight().GetX()) &&
      (mX >= Ar.GetUpperLeft().GetX())  &&
      
      (mY <= Ar.GetLowerRight().GetY()) &&
      (mY >= Ar.GetUpperLeft().GetY());
  }
  
  
  
  /*****************************************************************************
  * Function - Constructor
  * DESCRIPTION:
  *****************************************************************************/
  Area::Area()
  {
    mUpperLeft.Set(0,0);
    mLowerRight.Set(0,0);
  }
  
  /*****************************************************************************
  * Function - Copy constructor
  * DESCRIPTION:
  *****************************************************************************/
  Area::Area(const Area& Ar)
  {
    mUpperLeft = Ar.mUpperLeft;
    mLowerRight = Ar.mLowerRight;
  }

  Area::Area(const GUI_RECT& rect)
  {
    (*this) = (GUI_RECT)rect;
  }
  
  /*****************************************************************************
  * Function - Constructor
  * DESCRIPTION:
  *****************************************************************************/
  Area::Area(Position UpperLeft, Position LowerRight)
  {
    mUpperLeft = UpperLeft;
    mLowerRight = LowerRight;
  }
  
  /*****************************************************************************
  * Function - Constructor
  * DESCRIPTION:
  *****************************************************************************/
  Area::Area(U16 UpperLeftX, U16 UpperLeftY, U16 LowerRightX, U16 LowerRigthY)
  {
    mUpperLeft.Set(UpperLeftX,UpperLeftY);
    mLowerRight.Set(LowerRightX,LowerRigthY);
  }
  
  /*****************************************************************************
  * Function - Destructor.
  * DESCRIPTION:
  *****************************************************************************/
  Area::~Area()
  {
    
  }
  
  /*****************************************************************************
  * Function - TypeCast
  * DESCRIPTION:
  *****************************************************************************/
  Area::operator GUI_RECT()
  {
    GUI_RECT rect;
    
    rect.x0 = mUpperLeft.GetX();
    rect.y0 = mUpperLeft.GetY();
    rect.x1 = mLowerRight.GetX();
    rect.y1 = mLowerRight.GetY();
    
    return rect;
  }
  
  /*****************************************************************************
  * Function - Assigen operator =
  * DESCRIPTION:
  *****************************************************************************/
  Area& Area::operator =(const Area& Ar)
  {
    mUpperLeft = Ar.mUpperLeft;
    mLowerRight = Ar.mLowerRight;
    return *this;
  }
  Area& Area::operator =(const GUI_RECT& rect)
  {
    mUpperLeft.Set(rect.x0,rect.y0);
    mLowerRight.Set(rect.x1,rect.y1);
    return *this;
  }
  /*****************************************************************************
  * Function - Operator ==
  * DESCRIPTION:
  *****************************************************************************/
  bool Area::operator ==(Area& Ar)
  {
    return ((mLowerRight == Ar.mLowerRight) && (mUpperLeft == Ar.mUpperLeft));
  }
  
  /*****************************************************************************
  * Function - Operator !=
  * DESCRIPTION:
  *****************************************************************************/
  bool Area::operator !=(Area& Ar)
  {
    return !((mLowerRight == Ar.mLowerRight) && (mUpperLeft == Ar.mUpperLeft));
  }
  
  /*****************************************************************************
  * Function - operator +
  * DESCRIPTION:
  *****************************************************************************/
  Area Area::operator +(Position& Pos)
  {
    Area new_area;
    
    new_area.Set(mUpperLeft.GetX() + Pos.GetX(),
      mUpperLeft.GetY() + Pos.GetY(),
      mLowerRight.GetX() + Pos.GetX(),
      mLowerRight.GetY() + Pos.GetY() );
    return new_area;
  }
  
  /*****************************************************************************
  * Function - Operator -
  * DESCRIPTION:
  *****************************************************************************/
  Area Area::operator -(Position& Pos)
  {
    Area new_area;
    
    new_area.Set(mUpperLeft.GetX() - Pos.GetX(),
      mUpperLeft.GetY() - Pos.GetY(),
      mLowerRight.GetX() - Pos.GetX(),
      mLowerRight.GetY() - Pos.GetY() );
    return new_area;
  }
  
  /*****************************************************************************
  * Function - Operator +=
  * DESCRIPTION:
  *****************************************************************************/
  void Area::operator +=(Position& Pos)
  {
    mUpperLeft.Set(mUpperLeft.GetX() + Pos.GetX(),mUpperLeft.GetY() + Pos.GetY());
    mLowerRight.Set(mLowerRight.GetX() + Pos.GetX(),mLowerRight.GetY() + Pos.GetY());
  }
  
  /*****************************************************************************
  * Function - Operator -=
  * DESCRIPTION:
  *****************************************************************************/
  void Area::operator -=(Position& Pos)
  {
    mUpperLeft.Set(mUpperLeft.GetX() - Pos.GetX(),mUpperLeft.GetY() - Pos.GetY());
    mLowerRight.Set(mLowerRight.GetX() - Pos.GetX(),mLowerRight.GetY() - Pos.GetY());
  }
  
  /*****************************************************************************
  * Function - Set
  * DESCRIPTION:
  *****************************************************************************/
  void Area::Set(U16 UpperleftX, U16 UpperLeftY, U16 LowerRightX, U16 LowerRightY)
  {
    mUpperLeft.Set(UpperleftX,UpperLeftY);
    mLowerRight.Set(LowerRightX,LowerRightY);
  }
  
  /*****************************************************************************
  * Function - Set
  * DESCRIPTION:
  *****************************************************************************/
  void Area::Set(Position UpperLeft, Position LowerRight)
  {
    mUpperLeft = UpperLeft;
    mLowerRight = LowerRight;
  }
  
  /*****************************************************************************
  * Function - GetWidth
  * DESCRIPTION:
  *****************************************************************************/
  U16 Area::GetWidth()
  {
    return mLowerRight.GetX() - mUpperLeft.GetX() + 1;
  }
  
  /*****************************************************************************
  * Function - GetHeight
  * DESCRIPTION:
  *****************************************************************************/
  U16 Area::GetHeight()
  {
    return mLowerRight.GetY() - mUpperLeft.GetY() + 1;
  }
  
  /*****************************************************************************
  * Function - SetWidth
  * DESCRIPTION:
  *****************************************************************************/
  void Area::SetWidth(U16 Width)
  {
    mLowerRight.Set(mUpperLeft.GetX() + Width,mLowerRight.GetY());
  }
  
  /*****************************************************************************
  * Function - SetHeight
  * DESCRIPTION:
  *****************************************************************************/
  void Area::SetHeight(U16 Height)
  {
    mLowerRight.Set(mLowerRight.GetX(),mUpperLeft.GetY() +  Height);
  }
  
  /*****************************************************************************
  * Function - GetUpperLeft
  * DESCRIPTION:
  *****************************************************************************/
  Position Area::GetUpperLeft()
  {
    return mUpperLeft;
  }
  
  /*****************************************************************************
  * Function - GetLowerRight
  * DESCRIPTION:
  *****************************************************************************/
  Position Area::GetLowerRight()
  {
    return mLowerRight;
  }
  
  /*****************************************************************************
  * Function - Bring AreaWithinArea
  * DESCRIPTION:
  *****************************************************************************/
  void Area::BringWithinArea(Area& Ar)
  {
    mUpperLeft.BringPositionWithinArea(Ar);
    mLowerRight.BringPositionWithinArea(Ar);
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
  *                 - RARE USED -
  *
  ****************************************************************************/
}

