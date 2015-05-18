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
/* CLASS NAME       : ModemCtrl.cpp                                         */
/*                                                                          */
/* FILE NAME        : ModemCtrl.cpp.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 30-11-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <ModemCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
ModemCtrl::ModemCtrl()
{

}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ModemCtrl::~ModemCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ModemCtrl::InitSubTask()
{
  mRunRequestedFlag = false;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ModemCtrl::RunSubTask()
{
  bool update;
  mRunRequestedFlag = false;

  /* If phonenumbers have changed, set flag to geni to indicate that it has to read the number list */
  update = false;
  update |= mpSmsSecondaryNumber.IsUpdated();
  update |= mpSmsPrimaryNumber.IsUpdated();
  update |= mpAlarmSmsRecipient.IsUpdated();
  if ( update )
  {
    U8 modem_ctrl = mpModemCtrl->GetValue();
    modem_ctrl |= 0x01;     //Set bit 0: "Number list sub-object setting status in product"
    mpModemCtrl->SetValue( modem_ctrl );
  }
  /* Check if modem has read the number list and clear flag */
  if (mpNoListReadEvent.IsUpdated())
  {
    U8 modem_ctrl = mpModemCtrl->GetValue();
    modem_ctrl &= ~0x01;    //Clear bit 0: "Number list sub-object setting status in product"
    mpModemCtrl->SetValue( modem_ctrl );
  }

  /* If language has changed, set flag to geni to indicate that it has to read the alarm strings */
  if (mpCurrentLanguage.IsUpdated())
  {
    U8 modem_ctrl = mpModemCtrl->GetValue();
    modem_ctrl |= 0x02;     //Set bit 1: "Module notice string sub-objects setting status in product"
    mpNewStringsEvent->SetEvent();
    mpModemCtrl->SetValue( modem_ctrl );
  }
  /* Check if modem has read the alarm strings and clear flag */
  if (mpStringsReadEvent.IsUpdated())
  {
    U8 modem_ctrl = mpModemCtrl->GetValue();
    modem_ctrl &= ~0x02;    //Clear bit 1: "Module notice string sub-objects setting status in product"
    mpModemCtrl->SetValue( modem_ctrl );
  }

  /* If scada callback request has changed, set geni flag to indicate it */
  if (mpScadaCallbackRequest.IsUpdated())
  {
    U8 modem_ctrl = mpModemCtrl->GetValue();
    if (mpScadaCallbackRequest->GetValue() == true)
    {
      modem_ctrl |= 0x04;     //Set bit 2: "Scada callback request"
    }
    else
    {
      modem_ctrl &= ~0x04;    //Clear bit 2: "No scada callback request"
    }
    mpModemCtrl->SetValue( modem_ctrl );
  }

  /* If SCADA number has changed, set flag to geni to indicate that it has to read the SCADA number */
  if (mpScadaNumber.IsUpdated())
  {
    U8 modem_ctrl = mpModemCtrl->GetValue();
    modem_ctrl |= 0x10;     //Set bit 4: "Call back phone no update status"
    mpModemCtrl->SetValue( modem_ctrl );
  }
  /* Check if modem has read the SCADA number and clear flag */
  if (mpScadaNumberReadEvent.IsUpdated())
  {
      U8 modem_ctrl = mpModemCtrl->GetValue();
      modem_ctrl &= ~0x10;  //Clear bit 4: "Call back phone no update status"
      mpModemCtrl->SetValue( modem_ctrl );
  }

  /* If GPRS setup has changed , set flag to geni to indicate that it has to read the setup */
  update = false;
  update |= mpAuthentication.IsUpdated();
  update |= mpRoaming.IsUpdated();
  update |= mpModbusPort.IsUpdated();
  update |= mpGeniPort.IsUpdated();
  update |= mpGprsApn.IsUpdated();
  update |= mpGprsUsername.IsUpdated();
  update |= mpGprsPassword.IsUpdated();
  if ( update )
  {
    U8 modem_ctrl = mpModemCtrl->GetValue();
    modem_ctrl |= 0x20;     //Set bit 5: "XXX"
    mpModemCtrl->SetValue( modem_ctrl );
  }
  /* Check if modem has read the gprs setup and clear flag */
  if (mpGprsSetupReadEvent.IsUpdated())
  {
    U8 modem_ctrl = mpModemCtrl->GetValue();
    modem_ctrl &= ~0x20;    //Clear bit 5: "XXX"
    mpModemCtrl->SetValue( modem_ctrl );
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void ModemCtrl::ConnectToSubjects()
{
  mpStringsReadEvent->Subscribe(this);
  mpNoListReadEvent->Subscribe(this);
  mpScadaNumberReadEvent->Subscribe(this);
  mpSmsSecondaryNumber->Subscribe(this);
  mpSmsPrimaryNumber->Subscribe(this);
  mpAlarmSmsRecipient->Subscribe(this);
  mpCurrentLanguage->Subscribe(this);
  mpScadaNumber->Subscribe(this);
  mpScadaCallbackRequest->Subscribe(this);
  mpGprsApn->Subscribe(this);
  mpGprsUsername->Subscribe(this);
  mpGprsPassword->Subscribe(this);
  mpGprsSetupReadEvent->Subscribe(this);
  mpAuthentication->Subscribe(this);
  mpRoaming->Subscribe(this);
  mpModbusPort->Subscribe(this);
  mpGeniPort->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then request task time for sub task.
 *
 *****************************************************************************/
void ModemCtrl::Update(Subject* pSubject)
{
  mpScadaCallbackRequest.Update(pSubject);
  mpStringsReadEvent.Update(pSubject);
  mpNoListReadEvent.Update(pSubject);
  mpSmsSecondaryNumber.Update(pSubject);
  mpSmsPrimaryNumber.Update(pSubject);
  mpAlarmSmsRecipient.Update(pSubject);
  mpCurrentLanguage.Update(pSubject);
  mpScadaNumberReadEvent.Update(pSubject);
  mpScadaNumber.Update(pSubject);
  mpGprsApn.Update(pSubject);
  mpGprsUsername.Update(pSubject);
  mpGprsPassword.Update(pSubject);
  mpGprsSetupReadEvent.Update(pSubject);
  mpAuthentication.Update(pSubject);
  mpRoaming.Update(pSubject);
  mpModbusPort.Update(pSubject);
  mpGeniPort.Update(pSubject);
  if (mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void ModemCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpStringsReadEvent.Detach(pSubject);
  mpNoListReadEvent.Detach(pSubject);
  mpSmsSecondaryNumber.Detach(pSubject);
  mpSmsPrimaryNumber.Detach(pSubject);
  mpAlarmSmsRecipient.Detach(pSubject);
  mpCurrentLanguage.Detach(pSubject);
  mpScadaNumberReadEvent.Detach(pSubject);
  mpScadaNumber.Detach(pSubject);
  mpScadaCallbackRequest.Detach(pSubject);
  mpGprsApn.Detach(pSubject);
  mpGprsUsername.Detach(pSubject);
  mpGprsPassword.Detach(pSubject);
  mpAuthentication.Detach(pSubject);
  mpRoaming.Detach(pSubject);
  mpModbusPort.Detach(pSubject);
  mpGeniPort.Detach(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void ModemCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_MCC_CURRENT_LANGUAGE:
      mpCurrentLanguage.Attach(pSubject);
      break;
    case SP_MCC_SMS_MODULE_ALARM_STRINGS_READ_EVENT:
      mpStringsReadEvent.Attach(pSubject);
      break;
    case SP_MCC_NO_LIST_READ_EVENT:
      mpNoListReadEvent.Attach(pSubject);
      break;
    case SP_MCC_SCADA_NUMBER_READ_EVENT:
      mpScadaNumberReadEvent.Attach(pSubject);
      break;
    case SP_MCC_SCADA_CALLBACK_REQUEST:
      mpScadaCallbackRequest.Attach(pSubject);
      break;
    case SP_MCC_MODEM_CTRL:
      mpModemCtrl.Attach(pSubject);
      break;
    case SP_MCC_SECONDARY_NUMBER:
      mpSmsSecondaryNumber.Attach(pSubject);
      break;
    case SP_MCC_PRIMARY_NUMBER:
      mpSmsPrimaryNumber.Attach(pSubject);
      break;
    case SP_MCC_ALARM_SMS_RECIPIENT:
      mpAlarmSmsRecipient.Attach(pSubject);
      break;
    case SP_MCC_SMS_MODULE_NEW_ALARM_STRINGS_EVENT:
      mpNewStringsEvent.Attach(pSubject);
      break;
    case SP_MCC_SCADA_NUMBER:
      mpScadaNumber.Attach(pSubject);
      break;
    case SP_MCC_GPRS_APN:
      mpGprsApn.Attach(pSubject);
      break;
    case SP_MCC_GPRS_USERNAME:
      mpGprsUsername.Attach(pSubject);
      break;
    case SP_MCC_GPRS_PASSWORD:
      mpGprsPassword.Attach(pSubject);
      break;
    case SP_MCC_AUTHENTICATION:
      mpAuthentication.Attach(pSubject);
      break;
    case SP_MCC_ROAMING:
      mpRoaming.Attach(pSubject);
      break;
    case SP_MCC_MODBUS_PORT:
      mpModbusPort.Attach(pSubject);
      break;
    case SP_MCC_GENI_PORT:
      mpGeniPort.Attach(pSubject);
      break;
    case SP_MCC_GPRS_SETUP_READ_EVENT:
      mpGprsSetupReadEvent.Attach(pSubject);
      break;

    default:
      break;
  }
}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
