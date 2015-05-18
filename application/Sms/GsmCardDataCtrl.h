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
/* CLASS NAME       : GsmCardDataCtrl                                       */
/*                                                                          */
/* FILE NAME        : GsmCardDataCtrl.h                                     */
/*                                                                          */
/* CREATED DATE     : 29-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcGsmCardDataCtrl_h
#define mrcGsmCardDataCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Observer.h>
#include <SubTask.h>
#include <EnumDataPoint.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <BoolDataPoint.h>
#include <U32DataPoint.h>


/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
typedef struct
{
  SubjectPtr<U32DataPoint*> mpGeni;
  SubjectPtr<U32DataPoint*> mpGeniOld;
  SubjectPtr<U32DataPoint*> mpAct;
  int mMaxDiff;
}GSM_COUNTER_TYPE;

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/



/*****************************************************************************
 * CLASS:
 * DESCR  IPTION:
 *
 *****************************************************************************/
class GsmCardDataCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    GsmCardDataCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~GsmCardDataCtrl();
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
    SubjectPtr<EnumDataPoint<GSM_SIGNAL_LEVEL_TYPE>*> mpSignalLevel;
    SubjectPtr<EnumDataPoint<SIM_CARD_STATUS_TYPE>*> mpSimCardStatus;
    SubjectPtr<EnumDataPoint<COM_CARD_TYPE>*> mpCiuCardConf;
    SubjectPtr<BoolDataPoint*> mpCiuCardCommError;
    SubjectPtr<BoolDataPoint*> mpSimCardFault;
    SubjectPtr<EnumDataPoint<COM_STATE_TYPE>*> mpGprsCommunicationState;
    SubjectPtr<U32DataPoint*> mpGprsIpState;

    GSM_COUNTER_TYPE mGsmCounters[6];
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
