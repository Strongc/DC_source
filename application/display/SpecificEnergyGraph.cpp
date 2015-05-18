/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : SpecificEnergyGraph                                   */
/*                                                                          */
/* FILE NAME        : SpecificEnergyGraph.cpp                               */
/*                                                                          */
/* CREATED DATE     : 26-10-2009   (dd-mm-yyyy)                             */
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
#include <SpecificEnergyGraph.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * LIFECYCLE - Constructor
    *****************************************************************************/
    SpecificEnergyGraph::SpecificEnergyGraph(Component* pParent) : CurrentPointCurveGraph(pParent)
    {
      mpYAxisMin->SetNumberOfDigits(2);
      mpYAxisMax->SetNumberOfDigits(3);

      mpXAxisMin->SetNumberOfDigits(1);
      mpXAxisMax->SetNumberOfDigits(1);
      
    }

    /*****************************************************************************
    * LIFECYCLE - Destructor
    *****************************************************************************/
    SpecificEnergyGraph::~SpecificEnergyGraph()
    {
    }
  } // namespace mpc
} // namespace display

