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
/* FILE NAME        : GsmCardDataCtrl.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 28-04-2008 dd-mm-yyyy                                 */
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
#include <GsmCardDataCtrl.h>

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
GsmCardDataCtrl::GsmCardDataCtrl()
{
  mGsmCounters[0].mMaxDiff = 5;
  mGsmCounters[1].mMaxDiff = 5;

  mGsmCounters[2].mMaxDiff = 300;
  mGsmCounters[3].mMaxDiff = 300;

  mGsmCounters[4].mMaxDiff = 500;
  mGsmCounters[5].mMaxDiff = 500;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
GsmCardDataCtrl::~GsmCardDataCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GsmCardDataCtrl::InitSubTask()
{
  mRunRequestedFlag = true;
  mpSignalLevel->SetQuality(DP_NOT_AVAILABLE);
  mpSimCardStatus->SetQuality(DP_NOT_AVAILABLE);
  mpSimCardFault->SetValue(false);

  for(int i=0; i<6;i++)
  {
    mGsmCounters[i].mpGeni->SetValue(mGsmCounters[i].mpGeniOld->GetValue());
  }
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GsmCardDataCtrl::RunSubTask()
{
  bool card_update = false;

  mRunRequestedFlag = false;

  /* Handle quality on statusparameters from CIU */
  if(mpCiuCardCommError.IsUpdated())
  {
    if(mpCiuCardCommError->GetValue())
    {
      mpSignalLevel->SetQuality(DP_NOT_AVAILABLE);
      mpSimCardStatus->SetQuality(DP_NOT_AVAILABLE);
      mpGprsCommunicationState->SetQuality(DP_NOT_AVAILABLE);
      mpGprsIpState->SetValue(0);
      mpGprsIpState->SetQuality(DP_NOT_AVAILABLE);
    }
  }

  /* Handle Sim card fault */
  card_update |= mpSimCardStatus.IsUpdated();  // Sim card status evaluation should be done if
  card_update |= mpCiuCardConf.IsUpdated();    // the Sim card status OR the Ciu card conf has changed
  if(card_update == true)
  {
    // Check if a GSM-card is present in the system before evaluating upon the SimCardStatus
    if (mpCiuCardConf->GetValue() == COM_CARD_CIM_250_GSM)
    {
      if( mpSimCardStatus->GetQuality() == DP_AVAILABLE)
      {
        switch ( mpSimCardStatus->GetValue() )
        {
          case SIM_CARD_STATUS_PIN_INVALID:
          case SIM_CARD_STATUS_PUK_INVALID:
          case SIM_CARD_STATUS_SIM_NOT_PRESENT:
          case SIM_CARD_STATUS_DEFECT:
          case SIM_CARD_STATUS_WRONG_TYPE:
            mpSimCardFault->SetValue(true);
            break;
          default:
            mpSimCardFault->SetValue(false);
            break;
        }
      }
    }
    else
    {
      // If the system does not contain a GSM-card any SIM-card error should be removed.
      mpSimCardFault->SetValue(false);
    }
  }

  /* Handle data counters */
  for(int i=0; i<6;i++)
  {
    int act, geni, geni_old, diff, max_diff;
    geni = mGsmCounters[i].mpGeni->GetValue();
    geni_old = mGsmCounters[i].mpGeniOld->GetValue();
    max_diff = mGsmCounters[i].mMaxDiff;
    if(geni>geni_old)
    {
      diff = geni-geni_old;
      if( diff < max_diff)
      {
        act = mGsmCounters[i].mpAct->GetValue( );
        act += diff;
        mGsmCounters[i].mpAct->SetValue( act );
      }
    }
    mGsmCounters[i].mpGeniOld->SetValue( geni );
  }
}


/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void GsmCardDataCtrl::ConnectToSubjects()
{
  for(int i=0;i<6;i++)
  {
    mGsmCounters[i].mpGeni.Subscribe(this);
  }
  mpCiuCardCommError.Subscribe(this);
  mpCiuCardConf.Subscribe(this);
  mpSimCardStatus.Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void GsmCardDataCtrl::Update(Subject* pSubject)
{
  mpCiuCardCommError.Update(pSubject);
  mpSimCardStatus.Update(pSubject);
  mpCiuCardConf.Update(pSubject);

  if(mRunRequestedFlag == false)
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
void GsmCardDataCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void GsmCardDataCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_GCDC_SMS_RECEIVED_GENI:
      mGsmCounters[0].mpGeni.Attach(pSubject);
      break;
    case SP_GCDC_SMS_RECEIVED_GENI_OLD:
      mGsmCounters[0].mpGeniOld.Attach(pSubject);
      break;
    case SP_GCDC_SMS_RECEIVED_ACT:
      mGsmCounters[0].mpAct.Attach(pSubject);
      break;
    case SP_GCDC_SMS_SENT_GENI:
      mGsmCounters[1].mpGeni.Attach(pSubject);
      break;
    case SP_GCDC_SMS_SENT_GENI_OLD:
      mGsmCounters[1].mpGeniOld.Attach(pSubject);
      break;
    case SP_GCDC_SMS_SENT_ACT:
      mGsmCounters[1].mpAct.Attach(pSubject);
      break;
    case SP_GCDC_CSD_INCOMMING_GENI:
      mGsmCounters[2].mpGeni.Attach(pSubject);
      break;
    case SP_GCDC_CSD_INCOMMING_GENI_OLD:
      mGsmCounters[2].mpGeniOld.Attach(pSubject);
      break;
    case SP_GCDC_CSD_INCOMMING_ACT:
      mGsmCounters[2].mpAct.Attach(pSubject);
      break;
    case SP_GCDC_CSD_OUTGOING_GENI:
      mGsmCounters[3].mpGeni.Attach(pSubject);
      break;
    case SP_GCDC_CSD_OUTGOING_GENI_OLD:
      mGsmCounters[3].mpGeniOld.Attach(pSubject);
      break;
    case SP_GCDC_CSD_OUTGOING_ACT:
      mGsmCounters[3].mpAct.Attach(pSubject);
      break;
    case SP_GCDC_GPRS_RECEIVED_GENI:
      mGsmCounters[4].mpGeni.Attach(pSubject);
      break;
    case SP_GCDC_GPRS_RECEIVED_GENI_OLD:
      mGsmCounters[4].mpGeniOld.Attach(pSubject);
      break;
    case SP_GCDC_GPRS_RECEIVED_ACT:
      mGsmCounters[4].mpAct.Attach(pSubject);
      break;
    case SP_GCDC_GPRS_SENT_GENI:
      mGsmCounters[5].mpGeni.Attach(pSubject);
      break;
    case SP_GCDC_GPRS_SENT_GENI_OLD:
      mGsmCounters[5].mpGeniOld.Attach(pSubject);
      break;
    case SP_GCDC_GPRS_SENT_ACT:
      mGsmCounters[5].mpAct.Attach(pSubject);
      break;

    case SP_GCDC_SIGNAL_LEVEL:
      mpSignalLevel.Attach(pSubject);
      break;
    case SP_GCDC_SIM_CARD_STATUS:
      mpSimCardStatus.Attach(pSubject);
      break;
    case SP_GCDC_SIM_CARD_FAULT:
      mpSimCardFault.Attach(pSubject);
      break;

    case SP_GCDC_CIU_CARD_COMM_ERROR:
      mpCiuCardCommError.Attach(pSubject);
      break;
    case SP_GCDC_CIU_CARD_CONF:
      mpCiuCardConf.Attach(pSubject);
      break;

    case SP_GCDC_GPRS_COMMUNICATION_STATE:
      mpGprsCommunicationState.Attach(pSubject);
      break;

    case SP_GCDC_GPRS_IP_ADDRESS:
      mpGprsIpState.Attach(pSubject);
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
