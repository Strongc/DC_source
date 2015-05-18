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
/* CLASS NAME       : RelayFuncHandler                                      */
/*                                                                          */
/* FILE NAME        : RelayFuncHandler.h                                    */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Sets relay outputs and connects them to         */
/*                          digital functions according to configuration.   */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __RELAY_FUNC_HANDLER_H__
#define __RELAY_FUNC_HANDLER_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <AppTypeDefs.h>
#include <Observer.h>
#include <SubTask.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <EventDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
const int NO_OF_RELAYS = 23;

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: RelayFuncHandler
 * DESCRIPTION:
 *
 *****************************************************************************/
class RelayFuncHandler : public Observer, public SubTask
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void InitSubTask(void);
    void RunSubTask(void);
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects(void);
    void SetSubjectPointer(int Id, Subject* pSubject);

    bool IsFuncConfiged(RELAY_FUNC_TYPE relayFunc);

    static RelayFuncHandler* GetInstance(); // returns the pointer to the singleton

  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    RelayFuncHandler();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~RelayFuncHandler();

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool SetIOBRelayState(int relayIdx, bool relayState);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static RelayFuncHandler* mpInstance;
    SubjectPtr<BoolDataPoint*> mpRelayStatus[NO_OF_RELAY_FUNC];
    SubjectPtr<U32DataPoint*> mpRelayFuncOutput[NO_OF_RELAY_FUNC];
    SubjectPtr<EventDataPoint*> mpRelayConfigurationChanged;
    SubjectPtr<EnumDataPoint<RELAY_FUNC_TYPE>*> mpConfRelayFunc[NO_OF_RELAYS];
    SubjectPtr<BoolDataPoint*> mpDigOutState[NO_OF_RELAYS];
    SubjectPtr<U8DataPoint*> mpIO351DigOutStatusBits[3];  // IO351 digital output status bits
    SubjectPtr<U32DataPoint*> mpDigOutStatusBits;
    U32 mRelayBuffer;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
