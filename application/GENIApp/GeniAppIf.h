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
/* CLASS NAME       : GeniAppIf                                             */
/*                                                                          */
/* FILE NAME        : GeniAppIf.h                                           */
/*                                                                          */
/* CREATED DATE     : 29-02-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                    Main class for the Geni application interface.        */
/*                    Handles the conversion between data point and their   */
/*                    Geni bus reprensentation.                             */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __GENI_APP_IF_H__
#define __GENI_APP_IF_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>
#include <SubTask.h>
#include <fifo.h>
#include <U8DataPoint.h>
#include <U16DataPoint.h>
#include <U32DataPoint.h>
#include <I32DataPoint.h>
#include <FloatDataPoint.h>
#include <EnumDataPoint.h>
#include <EventDataPoint.h>
#include <FactoryTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <BaseGeniAppIf.h>
#include <ActTime.h>


/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: GeniAppIf
 * DESCRIPTION:
 *
 *****************************************************************************/
class GeniAppIf : public BaseGeniAppIf, public SubTask
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static GeniAppIf* GetInstance();

    // Observer
    virtual void SetSubjectPointer(int /*id*/, Subject* pSubject);
    virtual void ConnectToSubjects(void);
    virtual void Update(Subject* pSubject);
    virtual void SubscribtionCancelled(Subject* pSubject);

    // SubTask
    virtual void InitSubTask(void);
    virtual void RunSubTask(void);

    void DataFromBus(); //Event from GENIpro

    bool RunCommand(U8 cmd);
  
  private:
    /********************************************************************
    LIFECYCLE - Default constructor. Private because it is a Singleton
    ********************************************************************/
    GeniAppIf();
    /********************************************************************
    LIFECYCLE - Destructor. Private because it is a Singleton
    ********************************************************************/
    ~GeniAppIf();
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void HandleGeniBuffers(void);

    void HandleCommand(GAI_CMD_TYPE geniCmd);
    void HandleConfiguration8bit(GAI_VAR_TYPE geniVar, U8 newValue);
    void HandleReference8bit(GAI_VAR_TYPE geniVar, U8 newValue);
    void HandleConfiguration16Bit(GAI_VAR_TYPE geniVar, U16 newValue);
    void HandleReference16Bit(GAI_VAR_TYPE geniVar, U16 newValue);
    void HandleConfiguration32Bit(GAI_VAR_TYPE geniVar, U32 newValue);
    void HandleReference32Bit(GAI_VAR_TYPE geniVar, U32 newValue);

    void SubjectToGeni(Subject* pSubject);
    bool HandleSpecialBitPacking(Subject* pSubject);
    void UpdateServiceMode();
    void UpdateIo111DeviceStatus(bool bServiceMode);
    void UpdateCueDeviceStatus(bool bServiceMode);
    void UpdateMp204DeviceStatus(bool bServiceMode);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static GeniAppIf* mInstance;

    OS_RSEMA mSemaSubjectQueue;
    Fifo<Subject*, 1000> mSubjectQueue;

    bool mReqTaskTimeFlag;
    bool mDataFromBusFlag;

    bool mExternalCommandPending;
    U8 mExternalCommand;

	MpcTime* mpActTime;
  

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
