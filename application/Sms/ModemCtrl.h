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
/* CLASS NAME       : ModemCtrl                                             */
/*                                                                          */
/* FILE NAME        : ModemCtrl.h                                           */
/*                                                                          */
/* CREATED DATE     : 22-01-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Controls the geni parameter modem_ctr           */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcModemCtrl_h
#define mrcModemCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <subtask.h>
#include <StringDataPoint.h>
#include <EventDataPoint.h>
#include <EnumDataPoint.h>
#include <LanguagesDataPoint.h>
#include <BoolDataPoint.h>
#include <U8DataPoint.h>
#include <U16DataPoint.h>


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
class ModemCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    ModemCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~ModemCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Subject.
    void SetSubjectPointer(int Id, Subject* pSubject);
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);

    // SubTask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EventDataPoint*>  mpStringsReadEvent;
    SubjectPtr<EventDataPoint*>  mpNewStringsEvent;
    SubjectPtr<EventDataPoint*>  mpNoListReadEvent;
    SubjectPtr<EventDataPoint*>  mpScadaNumberReadEvent;
    SubjectPtr<BoolDataPoint*>   mpScadaCallbackRequest;
    SubjectPtr<EventDataPoint*>  mpGprsSetupReadEvent;

    SubjectPtr<U8DataPoint*>     mpModemCtrl;
    SubjectPtr<StringDataPoint*> mpSmsSecondaryNumber;
    SubjectPtr<StringDataPoint*> mpSmsPrimaryNumber;
    SubjectPtr<StringDataPoint*> mpScadaNumber;
    SubjectPtr<EnumDataPoint<SMS_RECIPIENT_TYPE>*> mpAlarmSmsRecipient;
    SubjectPtr<LanguagesDataPoint*> mpCurrentLanguage;
    SubjectPtr<StringDataPoint*> mpGprsApn;
    SubjectPtr<StringDataPoint*> mpGprsUsername;
    SubjectPtr<StringDataPoint*> mpGprsPassword;

    SubjectPtr<EnumDataPoint<GSM_AUTHENTICATION_TYPE>*> mpAuthentication;
    SubjectPtr<BoolDataPoint*>   mpRoaming;
    SubjectPtr<U16DataPoint*>    mpModbusPort;
    SubjectPtr<U16DataPoint*>    mpGeniPort;

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
