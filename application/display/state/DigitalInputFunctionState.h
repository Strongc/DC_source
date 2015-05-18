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
/* CLASS NAME       : DigitalInputFunctionState.h                           */
/*                                                                          */
/* FILE NAME        : DigitalInputFunctionState.h                           */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayDigitalInputFunctionState_h
#define mpc_displayDigitalInputFunctionState_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <StringDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "StateBracket.h"
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
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a DigitalInputFunctionState string given by a DataPoint
    *
    *****************************************************************************/
    class DigitalInputFunctionState : public StateBracket
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      DigitalInputFunctionState(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~DigitalInputFunctionState();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      //virtual bool Redraw();
      virtual const char* GetStateAsString(int state);
      virtual void ConnectToSubjects(void);

      /********************************************************************
      OPERATIONS
      ********************************************************************/
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
      SubjectPtr<StringDataPoint*> mDpNameOfExtraFault[NO_OF_EXTRA_FAULTS];
      SubjectPtr<StringDataPoint*> mDpNameOfUsdCounter[NO_OF_USD_COUNTERS];
    };
  } // namespace display
} // namespace mpc

#endif
