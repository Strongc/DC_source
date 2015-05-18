/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : CombiAlarmSlipPoint                                   */
/*                                                                          */
/* FILE NAME        : CombiAlarmSlipPoint.H                                 */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the combi alarm DataPoints into one                      */
/* virtual DataPoint for the display to look at...                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef _COMBI_ALARM_SLIP_POINT_h
#define _COMBI_ALARM_SLIP_POINT_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Subtask.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <EnumDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

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
    namespace ctrl
    {

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class CombiAlarmSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    CombiAlarmSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~CombiAlarmSlipPoint();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void InitSubTask(void);
    virtual void RunSubTask(void);

    virtual void SubscribtionCancelled(Subject* pSubject);
    virtual void Update(Subject* pSubject);
    virtual void SetSubjectPointer(int Id,Subject* pSubject);
    virtual void ConnectToSubjects(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void UpdateVirtualCombiAlarmSource(void);
    virtual void UpdateCurrentCombiAlarmSource(void);
    
    virtual void UpdateUpperStatusLine(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EnumDataPoint<ALARM_TYPE>*> 	mpCombiAlarmDP[NO_OF_COMBI_ALARM_SOURCES];
    SubjectPtr<IIntegerDataPoint*>          mpCurrentCombiAlarmNumber;
    SubjectPtr<EnumDataPoint<ALARM_TYPE>*> 	mpVirtualCombiAlarmSource;
    bool                                    mCurrentlyUpdating;
    
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
    } // namespace ctrl
  } // namespace display
} // namespace mpc
#endif
