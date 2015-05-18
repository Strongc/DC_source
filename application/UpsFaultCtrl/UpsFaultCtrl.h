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
/* CLASS NAME       : UpsFaultCtrl                                          */
/*                                                                          */
/* FILE NAME        : UpsFaultCtrl.h                                        */
/*                                                                          */
/* CREATED DATE     : 30-05-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :  The module evaluates the measured battery      */
/*                           voltage and supplies a battery status, an      */
/*                           error flag and a voltage value for display.    */
/*                           This value can be not avaliable depending upon */
/*                           the status.                                    */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcUpsFaultCtrl_h
#define mrcUpsFaultCtrl_h

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
#include <FloatDataPoint.h>
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

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class UpsFaultCtrl : public Observer, public SubTask

{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    UpsFaultCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~UpsFaultCtrl();
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

    SubjectPtr<BoolDataPoint*> mpBatteryBackupInstalled;
    SubjectPtr<BoolDataPoint*> mpUpsFaultFlag;

    SubjectPtr<FloatDataPoint*> mpBatteryVoltageMeasured;
    SubjectPtr<FloatDataPoint*> mpBatteryVoltageDisplayed;
    SubjectPtr<EnumDataPoint<BATTERY_STATE_TYPE>*> mpBatteryStatus;

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
