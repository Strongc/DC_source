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
/* CLASS NAME       : AlarmListItem                                   */
/*                                                                          */
/* FILE NAME        : AlarmListItem.h                                 */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/* This is a listviewitem in the ActiveAlarmList. This implementation       */
/* overloads the IsNeverAvailable function in the ListViewItem class        */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayAlarmListItem_h
#define mpc_displayAlarmListItem_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ListViewItem.h"
#include <FactoryTypes.h>
#include <AlarmEvent.h>
#include <Observer.h>
#include <AlarmLog.h>
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
// Forward declarations.
    class Group;
    class Label;
    class TimeText;
    class Image;
    class DataPointText;
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * 
    *
    *****************************************************************************/
    class AlarmListItem : public ListViewItem
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      AlarmListItem(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~AlarmListItem();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void SetAlarmIndex(int alarmIndex);

      virtual void Run( );
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      /* --------------------------------------------------
      * If the subject is a DataPoint and the quality is
      * DP_NEVER_AVAILABLE this function shall return true
      * --------------------------------------------------*/
      virtual bool IsNeverAvailable();

      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* Object);
      /* --------------------------------------------------
      * Called if subscription shall be canceled
      * --------------------------------------------------*/
      virtual void SubscribtionCancelled(Subject* pSubject);
      /* --------------------------------------------------
      * Called to set the subject pointer (used by class
      * factory)
      * --------------------------------------------------*/
      virtual void SetSubjectPointer(int Id,Subject* pSubject);
      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      virtual void ConnectToSubjects(void);

      virtual void UpdateAvailability();

      virtual bool IsValid();

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
      STRING_ID GetUnitString(ERRONEOUS_UNIT_TYPE type, int number);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      bool  mLastNeverAvailable;

      SubjectPtr<AlarmLog*> mpAlarmLog;
      int       mAlarmIndex;  // The index of the AlarmEvent this ListViewItem redraws

      Group*    mGroup;
      Label*    mLabelErrorUnit;
      Label*    mLabelErrorString;
      DataPointText* mDataPointTextErrorString;
      Image*    mIcon;

      Label*    mLabelArrivalTime;
      TimeText* mArrivalTime;

      Label*    mLabelDisappearingTime;
      TimeText* mDisappearingTime;
      
    };
  } // namespace display
} // namespace mpc

#endif
