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
/* CLASS NAME       : GeniAppRtcCtrl                                        */
/*                                                                          */
/* FILE NAME        : GeniAppRtcCtrl.h                                      */
/*                                                                          */
/* CREATED DATE     : 27-03-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Geni interface to realtime clock handling       */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcGeniAppRtcCtrl_h
#define mrcGeniAppRtcCtrl_h

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
#include <EnumDataPoint.h>
#include <U8DataPoint.h>
#include <U16DataPoint.h>
#include <U32DataPoint.h>
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
class GeniAppRtcCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    GeniAppRtcCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~GeniAppRtcCtrl();
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

    SubjectPtr<U8DataPoint*>    mpRtcSecond;
    SubjectPtr<U8DataPoint*>    mpRtcMinute;
    SubjectPtr<U8DataPoint*>    mpRtcHour;
    SubjectPtr<U8DataPoint*>    mpRtcDayOfMonth;
    SubjectPtr<U8DataPoint*>    mpRtcMonthOfYear;
    SubjectPtr<U8DataPoint*>    mpRtcYear;
    SubjectPtr<U8DataPoint*>    mpRtcDayOfWeek;
    SubjectPtr<U16DataPoint*>   mpRtcDayOfYear;
    SubjectPtr<U32DataPoint*>   mpRtcSecondsSince1970Act;
    SubjectPtr<U32DataPoint*>   mpRtcSecondsSince1970Set;
    SubjectPtr<U8DataPoint*>    mpRtcYearSet;
    SubjectPtr<U8DataPoint*>    mpRtcMonthSet;
    SubjectPtr<U8DataPoint*>    mpRtcDaySet;
    SubjectPtr<U8DataPoint*>    mpRtcHourSet;
    SubjectPtr<U8DataPoint*>    mpRtcMinuteSet;
    SubjectPtr<U8DataPoint*>    mpRtcSecondSet;
    SubjectPtr<EventDataPoint*> mpRtcUseNewTimeEvent;

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
