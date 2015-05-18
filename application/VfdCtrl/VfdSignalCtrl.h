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
/* CLASS NAME       : VfdSignalCtrl                                         */
/*                                                                          */
/* FILE NAME        : VfdSignalCtrl.h                                       */
/*                                                                          */
/* CREATED DATE     : 04-05-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class to interface to variable frequency drives */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcVfdSignalCtrl_h
#define mrcVfdSignalCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include <Observer.h>
#include <SubTask.h>
#include <BoolDataPoint.h>
#include <EventDataPoint.h>
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
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class VfdSignalCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    VfdSignalCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~VfdSignalCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void InitSubTask();
    void RunSubTask();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<BoolDataPoint*>      mpVfdInstalled;
    SubjectPtr<BoolDataPoint*>      mpCueEnabled;

    // Variable inputs:
    SubjectPtr<BoolDataPoint*>      mpVfdStart;
    SubjectPtr<BoolDataPoint*>      mpVfdReverse;
    SubjectPtr<FloatDataPoint*>     mpVfdFrequency;
    SubjectPtr<BoolDataPoint*>      mpCueCommunicationFlag;

    // Outputs:
    SubjectPtr<FloatDataPoint*>     mpVfdAnalogOutput;
    SubjectPtr<EventDataPoint*>     mpCuePumpStartEvent;
    SubjectPtr<EventDataPoint*>     mpCuePumpStopEvent;
    SubjectPtr<EventDataPoint*>     mpCuePumpForwardEvent;
    SubjectPtr<EventDataPoint*>     mpCuePumpReverseEvent;
    SubjectPtr<FloatDataPoint*>     mpCuePumpRefFrequency;
    SubjectPtr<BoolDataPoint*>      mpCueInstalled;

    // Local variables:
    bool mRunRequestedFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
