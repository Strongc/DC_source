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
/* CLASS NAME       : MergeX                                                */
/*                                                                          */
/* FILE NAME        : MergeX.h                                              */
/*                                                                          */
/* CREATED DATE     : 17-08-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/* Takes a number of DataPoints as input, and writes the value of the first */
/* one with quality == DP_AVAILABLE to an output DataPoint.                 */
/*                                                                          */
/* If no DataPoints are DP_AVAILABLE, checks to see if any are              */
/* DP_NOT_AVAILABLE, in which case the output DataPoint's quality is set to */
/* DP_NOT_AVAILABLE, otherwise it is set to DP_NEVER_AVAILABLE.             */
/*                                                                          */
/* All DataPoints must be of the same type.                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __MERGE_X_H__
#define __MERGE_X_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <rtos.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Subject.h>
#include <SubjectPtr.h>

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
 * CLASS: MergeX
 ****************************************************************************/
template<typename VAL_TYPE, unsigned int INPUT_COUNT>
class MergeX : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor
    DESCRIPTION: RunSubTask() is called every 10 ms, but skips
    nine out of ten.  taskScanOffset selects which one of the ten scans
    (1 - 10) to use to allow distributing the work load.
    ********************************************************************/
    MergeX(unsigned int taskScanOffset)
    {
      mScanSkipCounter = 10 - taskScanOffset;
    }

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~MergeX()
    {
    }


    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/

    void RunSubTask()
    {
      if (++mScanSkipCounter < 10) //Only run every 100 ms
      {
        return;
      }
      mScanSkipCounter = 0;

      for (unsigned int i = 0; i < (sizeof(mInputs) / sizeof(*mInputs)); ++i)
      {
        if (mInputs[i]->GetQuality() == DP_AVAILABLE)
        {
          mpOutput->SetValue(mInputs[i]->GetValue());
          return;
        }
      }

      for (unsigned int i = 0; i < (sizeof(mInputs) / sizeof(*mInputs)); ++i)
      {
        if (mInputs[i]->GetQuality() == DP_NOT_AVAILABLE)
        {
          mpOutput->SetQuality(DP_NOT_AVAILABLE);
          return;
        }
      }

      mpOutput->SetQuality(DP_NEVER_AVAILABLE);
    }

    void InitSubTask()
    {
    }

    void Update(Subject* pSubject)
    {
    }

    void SubscribtionCancelled(Subject* pSubject)
    {
    }

    void ConnectToSubjects()
    {
    }

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    unsigned int mScanSkipCounter;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<VAL_TYPE*> mInputs[INPUT_COUNT];
    SubjectPtr<VAL_TYPE*> mpOutput;
};

#endif
