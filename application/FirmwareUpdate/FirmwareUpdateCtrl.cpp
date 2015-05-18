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
/* FILE NAME        : FirmwareUpdateCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 13-07-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Subject.h>
#include <DataPoint.h>
#include <FactoryTypes.h>
#include <SubTask.h>

#ifndef __PC__
  #include <PowerDown.h>
#endif //__PC__

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <FirmwareUpdateCtrl.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
CONST
*****************************************************************************/

/*****************************************************************************
CREATES AN OBJECT.
******************************************************************************/
FirmwareUpdateCtrl* FirmwareUpdateCtrl::mInstance = 0;

/*****************************************************************************
*
*
*              PUBLIC FUNCTIONS
*
*
*****************************************************************************/

/*****************************************************************************
* Function - GetInstance
* DESCRIPTION:
*
*****************************************************************************/
FirmwareUpdateCtrl* FirmwareUpdateCtrl::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new FirmwareUpdateCtrl();
  }
  return mInstance;
}

/*****************************************************************************
* Function - InitSubTask
* DESCRIPTION:
*
*****************************************************************************/
void FirmwareUpdateCtrl::InitSubTask()
{
}

/*****************************************************************************
* Function - RunSubTask
* DESCRIPTION:
*
*****************************************************************************/
void FirmwareUpdateCtrl::RunSubTask()
{
  if (mpUpdateState.IsUpdated())
  {
    if (mpUpdateState->GetValue() == FIRMWARE_UPDATE_STATE_START)
    {
#ifndef __PC__
      MPCBasicFirmwareUpdater::getInstance()->setStatusCallback(UpdateStatusCallback);
      MPCBasicFirmwareUpdater::getInstance()->doUpdate();
#endif
    }
    else if (mpUpdateState->GetValue() == FIRMWARE_UPDATE_STATE_STARTBL)
    {
#ifndef __PC__
      MPCBasicFirmwareUpdater::getInstance()->setStatusCallback(UpdateStatusCallback);
      MPCBasicFirmwareUpdater::getInstance()->doUpdateBootloader();
#endif
    }
  }

  if (mpServerIpAddr.IsUpdated())
  {
#ifndef __PC__
    auto IPAddress server_ip_addr(mpServerIpAddr->GetValue());
    MPCBasicFirmwareUpdater::getInstance()->setServerIPAddress(server_ip_addr);
#endif
  }
}

/*****************************************************************************
* Function - ConnectToSubjects
* DESCRIPTION: Subscribe to subjects.
*
*****************************************************************************/
void FirmwareUpdateCtrl::ConnectToSubjects()
{
  mpUpdateState->Subscribe(this);
  mpServerIpAddr->Subscribe(this);
}

/*****************************************************************************
* Function - Update
* DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
* If it is then request task time for sub task.
*
*****************************************************************************/
void FirmwareUpdateCtrl::Update(Subject* pSubject)
{
  if (mpUpdateState.Update(pSubject))
  {
    ReqTaskTime();
  }
  else if (mpServerIpAddr.Update(pSubject))
  {
    ReqTaskTime();
  }
}

/*****************************************************************************
* Function - UpdateStatusCallback
* DESCRIPTION: Callback called by MPCBasicFirmwareUpdate to set status of
* update.
*****************************************************************************/
void FirmwareUpdateCtrl::UpdateStatusCallback(FIRMWARE_UPDATE_STATE_TYPE status)
{
  GetInstance()->mpUpdateState->SetValue(status);
}

/*****************************************************************************
* Function - SubscribtionCancelled
* DESCRIPTION: If pSubject is a pointer of ConfigContol then remove from list
*
*****************************************************************************/
void FirmwareUpdateCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpUpdateState.Detach(pSubject);
  mpServerIpAddr.Detach(pSubject);
}

/*****************************************************************************
* Function - SetSubjectPointer
* DESCRIPTION: If the id is equal to a id for a subject observed.
* Then take a copy of pSubjet to the member pointer for this subject.
*
*****************************************************************************/
void FirmwareUpdateCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
  case SP_FUC_STATE:
    mpUpdateState.Attach(pSubject);
    break;
  case SP_FUC_TFTP_SERVER_IP_ADDRESS:
    mpServerIpAddr.Attach(pSubject);
	break;
	
  default:
    break;
  }
}


#ifndef __PC__
/*****************************************************************************
 * Function - webifMakeStatus
 * DESCRIPTION: Called by the webifHandler
 *****************************************************************************/
void FirmwareUpdateCtrl::webifMakeStatus(HttpResponse& res)
{
  switch (mpUpdateState->GetValue())
  {
  case FIRMWARE_UPDATE_STATE_START:
    res << "Start";
    break;
  case FIRMWARE_UPDATE_STATE_ERASINGPRI:
    res << "Erasing primary flash image, please wait...";
    break;
  case FIRMWARE_UPDATE_STATE_ERASINGSEC:
    res << "Erasing secondary flash image, please wait...";
    break;
  case FIRMWARE_UPDATE_STATE_IDLE:
    res << "Idle";
    break;
  case FIRMWARE_UPDATE_STATE_PROGRAMMINGPRI:
    res << "Programming primary flash image, please wait...";
    break;
  case FIRMWARE_UPDATE_STATE_PROGRAMMINGSEC:
    res << "Programming secondary flash image, please wait...";
    break;
  case FIRMWARE_UPDATE_STATE_SUCCESS:
    res << "SUCCESS - FIRMWARE UPDATED - REBOOT THE CU 361";
    break;
  case FIRMWARE_UPDATE_STATE_RESETPRIMARYCOUNT:
    res << "Reset primary count";
    break;
  case FIRMWARE_UPDATE_STATE_RESETSECONDARYCOUNT:
    res << "Reset secondary count";
    break;
  case FIRMWARE_UPDATE_STATE_FAILUREINTERN:
    res << "<b>ERROR:</b> Internal failure";
    break;
  case FIRMWARE_UPDATE_STATE_FAILUREFLASHWRITE:
    res << "<b>ERROR:</b> Flash write failure";
    break;
  case FIRMWARE_UPDATE_STATE_FAILUREPING:
    res << "<b>ERROR:</b> PING failure - check TFTP server IP address";
    break;
  case FIRMWARE_UPDATE_STATE_FAILURENETWORKDOWN:
    res << "<b>ERROR:</b> Network down";
    break;
  case FIRMWARE_UPDATE_STATE_FAILURENOTFTPSERVER:
    res << "<b>ERROR:</b> TFTP Server NOT found - check TFTP server IP address";
    break;
  case FIRMWARE_UPDATE_STATE_FAILURETFTPERROR:
    res << "<b>ERROR:</b> TFTP Server ERROR - check TFTP server IP address";
    break;

  case FIRMWARE_UPDATE_STATE_STARTBL:
    res << "FIRMWARE_UPDATE_STATE_STARTBL";
    break;
  case FIRMWARE_UPDATE_STATE_RECEIVINGBL:
    res << "FIRMWARE_UPDATE_STATE_RECEIVINGBL";
    break;
  case FIRMWARE_UPDATE_STATE_ERASINGBL:
    res << "FIRMWARE_UPDATE_STATE_ERASINGBL";
    break;
  case FIRMWARE_UPDATE_STATE_PROGRAMMINGBL:
    res << "FIRMWARE_UPDATE_STATE_PROGRAMMINGBL";
    break;
  
  default:
    res << "Unknown FIRMWARE_UPDATE_STATE = " << mpUpdateState->GetValue();
  }
}

/*****************************************************************************
 * Function - webifMakeSettingsHtml
 * DESCRIPTION: Called by the webifHandler
 *****************************************************************************/
void FirmwareUpdateCtrl::webifMakeSettingsHtml(std::ostream& res)
{
  IPAddress ip(mpServerIpAddr->GetValue());
  std::string strIP;
  
  ip.toString(strIP);

  res << "<table cellspacing=\"0px\" cellpadding=\"0px\">";
  
  res << "<tr><td>TFTP Server IP Address:</td><td>&nbsp;&nbsp;&nbsp;</td><td>";
  res << "<input type=\"text\" name=\"tftp_server_ip_address\" value=\"" << strIP << "\" size=\"15\"/>";
  res << "</td></tr>";
  
  res << "</table>";
}

/*****************************************************************************
 * Function - webifDoPost
 * DESCRIPTION: Called by the webifHandler
 *****************************************************************************/
void FirmwareUpdateCtrl::webifDoPost(HttpRequest& req)
{
  IPAddress ip;

  if (strcmp(req.GetParam("reboot_value", "").c_str(), "do_it") == 0)
  {
    SignalEventToPowerDown(POWER_DOWN_EVENT);  
  }
  else 
  {
    if (ip.fromString(req.GetParam("tftp_server_ip_address", "").c_str()))
    {
      if (mpUpdateState->GetValue() != FIRMWARE_UPDATE_STATE_START)
      {
        mpServerIpAddr->SetValue(ip.getIPAddressMpc());
        mpUpdateState->SetValue(FIRMWARE_UPDATE_STATE_START);
      }
    }
  }
}
#endif //__PC__

/*****************************************************************************
*
*
*              PRIVATE FUNCTIONS
*
*
****************************************************************************/

/*****************************************************************************
* Function - Constructor
* DESCRIPTION: Initialise member data
*
*****************************************************************************/
FirmwareUpdateCtrl::FirmwareUpdateCtrl()
{
}

/*****************************************************************************
* Function - Destructor
* DESCRIPTION:
*
****************************************************************************/
FirmwareUpdateCtrl::~FirmwareUpdateCtrl()
{
}

/*****************************************************************************
*
*
*              PROTECTED FUNCTIONS
*                 - RARE USED -
*
****************************************************************************/
