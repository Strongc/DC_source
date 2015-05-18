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
/* CLASS NAME       : PowerOnWarningCtrl                                    */
/*                                                                          */
/* FILE NAME        : PowerOnWarningCtrl.h                                  */
/*                                                                          */
/* CREATED DATE     : 13-05-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : The module sets an alarm/warning (depending     */
/*                          upon configuration) when the system powers up.  */
/*                          The alarm/warning is cleared by the module at   */
/*                          once leaving an item in the alarm log to        */
/*                          indicate that a power-up - or a restart of the  */
/*                          software - has occured.                         */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcPowerOnWarningCtrl_h
#define mrcPowerOnWarningCtrl_h

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
  FIRST_POWC_FAULT_OBJ,
  POWC_FAULT_OBJ_POWER_ON_WARNING = FIRST_POWC_FAULT_OBJ,

  NO_OF_POWC_FAULT_OBJ,
  LAST_POWC_FAULT_OBJ = NO_OF_POWC_FAULT_OBJ - 1
}POWC_FAULT_OBJ_TYPE;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PowerOnWarningCtrl : public Observer, public SubTask

{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PowerOnWarningCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PowerOnWarningCtrl();
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

    bool mSetPowerOnWarning;
    bool mClearPowerOnWarning;

    AlarmDelay* mpPowerOnWarningDelay[NO_OF_POWC_FAULT_OBJ];
    bool mPowerOnWarningDelayCheckFlag[NO_OF_POWC_FAULT_OBJ];

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
