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
/* CLASS NAME       : OnOffAutoCtrl                                         */
/*                                                                          */
/* FILE NAME        : OnOffAutoCtrl.h                                       */
/*                                                                          */
/* CREATED DATE     : 26-06-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcOnOffAutoCtrl_h
#define mrcOnOffAutoCtrl_h

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
#include <EnumDataPoint.h>
#include <EventDataPoint.h>

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
class OnOffAutoCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    OnOffAutoCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~OnOffAutoCtrl();
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
    SubjectPtr<EnumDataPoint<CONTROL_SOURCE_TYPE>*> mpPumpControlSource;
    SubjectPtr<EnumDataPoint<CONTROL_SOURCE_TYPE>*> mpPumpBusSource;

    SubjectPtr<EnumDataPoint<REQ_OPERATION_MODE_TYPE>*> mpOprModeDI;
    SubjectPtr<EnumDataPoint<REQ_OPERATION_MODE_TYPE>*> mpOprModeUser;
    SubjectPtr<EnumDataPoint<REQ_OPERATION_MODE_TYPE>*> mpOprModeBus;
    SubjectPtr<EventDataPoint*> mpBusCmdStop;
    SubjectPtr<EventDataPoint*> mpBusCmdStart;
    SubjectPtr<EventDataPoint*> mpBusCmdAuto;
    SubjectPtr<BoolDataPoint*> mpUnderLowestStopLevel;
    SubjectPtr<BoolDataPoint*> mpPumpManual;

    SubjectPtr<EnumDataPoint<REQ_OPERATION_MODE_TYPE>*> mpOprModeReq;

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
