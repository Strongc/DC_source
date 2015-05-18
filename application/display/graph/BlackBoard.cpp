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
/* CLASS NAME       : BlackBoard                                            */
/*                                                                          */
/* FILE NAME        : BlackBoard.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 29-01-2008 (dd-mm-yyyy)                               */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* See header file.                                                         */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <algorithm>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <BlackBoard.h>

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
    /*************************************************************************
    * Function - Constructor
    * DESCRIPTION:
    * Constructor
    **************************************************************************/
    BlackBoard::BlackBoard(Component* pParent /*= NULL*/): Component(pParent)
    {
    }

    /*************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    * Dectructor
    *************************************************************************/
    BlackBoard::~BlackBoard()
    {
    }

    /*************************************************************************
    * Function - Redraw
    * DESCRIPTION:
    * See header file
    *************************************************************************/
    bool BlackBoard::Redraw()
    {
      Component::Redraw();
      
      for (CHALK_VECTOR_ITR itr = mChalks.begin(); itr != mChalks.end(); itr++)
      {
        (*itr)->Redraw();
      }
      
      return true;
    }

    /*************************************************************************
    * Function - AddChalk
    * DESCRIPTION:
    * See header file
    *************************************************************************/
    void BlackBoard::AddChalk(Chalk* pChalk)
    {
      CHALK_VECTOR_ITR itr = std::find(mChalks.begin(), mChalks.end(), pChalk);
      
      if (itr == mChalks.end())
      {
        mChalks.push_back(pChalk);
        Invalidate();
      }
    }
  } // namespace display
}  // namespace mpc

