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
/* CLASS NAME       : CustomRelayAlarmCtrl                                  */
/*                                                                          */
/* FILE NAME        : CustomRelayAlarmCtrl.h                                */
/*                                                                          */
/* CREATED DATE     : 15-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :  Sets an alarm if the Custom Relay function     */
/*                           is configured and the relay is active.         */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcCustomRelayAlarmCtrl_h
#define mrcCustomRelayAlarmCtrl_h

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
#include <AlarmDelay.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

typedef enum
{
  FIRST_CRAC_FAULT_OBJ,
  CRAC_FAULT_OBJ_CUSTOM_RELAY_ACTIVE = FIRST_CRAC_FAULT_OBJ,

  NO_OF_CRAC_FAULT_OBJ,
  LAST_CRAC_FAULT_OBJ = NO_OF_CRAC_FAULT_OBJ - 1
}CRAC_FAULT_OBJ_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class CustomRelayAlarmCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    CustomRelayAlarmCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~CustomRelayAlarmCtrl();
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
    void SetSubjectPointer(int Id, Subject* pSubject);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<BoolDataPoint*> mpRelayCustomStatus;
    SubjectPtr<EventDataPoint*> mpRelayConfigurationChanged;

    AlarmDelay* mpCustomRelayAlarmAlarmDelay[NO_OF_CRAC_FAULT_OBJ];
    bool mCustomRelayAlarmAlarmDelayCheckFlag[NO_OF_CRAC_FAULT_OBJ];

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
