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
/* CLASS NAME       : FirmwareUpdateCtrl                                    */
/*                                                                          */
/* FILE NAME        : FirmwareUpdateCtrl.h                                  */
/*                                                                          */
/* CREATED DATE     : 13-07-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcFirmwareUpdateCtrl_h
#define mpcFirmwareUpdateCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <vector>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <U32DataPoint.h>
#include <EnumDataPoint.h>

#ifndef __PC__
  #include <HttpResponse.h>
  #include <HttpRequest.h>

  #include "MPCBasicFirmwareUpdater.h"
#endif //__PC__
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
class FirmwareUpdateCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static FirmwareUpdateCtrl* GetInstance();

    void InitSubTask();
    void RunSubTask();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);

    #ifndef __PC__
    void webifMakeStatus(HttpResponse& res);
    void webifMakeSettingsHtml(std::ostream& res);
    void webifDoPost(HttpRequest& req);
    #endif //__PC__
    
    static void UpdateStatusCallback(FIRMWARE_UPDATE_STATE_TYPE);

  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    FirmwareUpdateCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~FirmwareUpdateCtrl();
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static FirmwareUpdateCtrl *mInstance;

    SubjectPtr<EnumDataPoint<FIRMWARE_UPDATE_STATE_TYPE>*> mpUpdateState;
    SubjectPtr<U32DataPoint*> mpServerIpAddr;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
