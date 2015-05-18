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
/* CLASS NAME       : Merge3Floats                                          */
/*                                                                          */
/* FILE NAME        : Merge3Floats.h                                        */
/*                                                                          */
/* CREATED DATE     : 20-11-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/* Takes two DataPoints as input, and writes the value of the first one     */
/* with quality == DP_AVAILABLE to an output DataPoint.                     */
/*                                                                          */
/* If no DataPoints are DP_AVAILABLE, checks to see if any are              */
/* DP_NOT_AVAILABLE, in which case the output DataPoint's quality is set to */
/* DP_NOT_AVAILABLE, otherwise it is set to DP_NEVER_AVAILABLE.             */
/*                                                                          */
/* All DataPoints must be of the type FloatDataPoint.                       */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __MERGE_3_FLOATS_H__
#define __MERGE_3_FLOATS_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "MergeX.h"
#include <FloatDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: Merge3Floats
 ****************************************************************************/
class Merge3Floats : public MergeX<FloatDataPoint, 3>
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor
    ********************************************************************/
    Merge3Floats(unsigned int taskScanOffset) :
      MergeX<FloatDataPoint, 3>(taskScanOffset)
    {
    }

    void SetSubjectPointer(int id, Subject* pSubject)
    {
      switch(id)
      {
        case SP_M3F_INPUT_1: mInputs[0].Attach(pSubject); break;
        case SP_M3F_INPUT_2: mInputs[1].Attach(pSubject); break;
        case SP_M3F_INPUT_3: mInputs[2].Attach(pSubject); break;

        case SP_M3F_OUTPUT: mpOutput.Attach(pSubject); break;
      }
    }
};

#endif
