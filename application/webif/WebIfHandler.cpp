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
/* CLASS NAME       : WebIfHandler                                          */
/*                                                                          */
/* FILE NAME        : WebIfHandler.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 22-06-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See header file                                 */
/****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>
#include <EthernetCtrl.h>
#include <ipconfig/IPConfigByWeb.h>
#include <ipconfig/PasswordSetting.h>
#include <SoftwareVersion.h>
#include <..\\FirmwareUpdate\\FirmwareUpdateCtrl.h>
#include <IO351IOModule.h>

/*****************************************************************************
  LOCAL INCLUDES
 *****************************************************************************/
#include <WebIfHandler.h>

/*****************************************************************************
  INITIALIZE STATIC MEMBERS
 *****************************************************************************/
WebIfHandler* WebIfHandler::mpInstance = NULL;

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 ****************************************************************************/
WebIfHandler::WebIfHandler()
{
  for (int i = 0; i < sizeof(m_pIO351IOModules) / sizeof(m_pIO351IOModules[0]); i++)
  {
    m_pIO351IOModules[i] = NULL;
  }
}
 
/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION: Returns the WebIfHandler instance (singleton)
 ****************************************************************************/
WebIfHandler* WebIfHandler::GetInstance()
{
  if (!mpInstance)
  {
    mpInstance = new WebIfHandler();
  }
  
  return mpInstance;
}

/*****************************************************************************
 * Function - doGetFN
 * DESCRIPTION: Called for <!--#exec cgi="/get.fn szParam"-->
 ****************************************************************************/
void WebIfHandler::doGetFN(const char* szParam, std::ostream& res)
{
  // general
  if (strcmp(szParam, "title") == 0)
  {
    res << PRODUCT_NAME;
  }
  
  // index.html
  else if (strcmp(szParam, "mac_address") == 0)
  {
    EthernetCtrl::GetInstance()->webifGetMacAddress(res);
  }
  
  // pwconfig.html
  else if (strcmp(szParam, "user_name") == 0)
  {
    PasswordSetting::getInstance()->webifGetUserName(res);
  }
  else if (strcmp(szParam, "pwconfig_msg") == 0)
  {
    PasswordSetting::getInstance()->webifGetTextMessage(res);
  }
  
  // ipconfig.html
  else if (strcmp(szParam, "dhcp_enabled") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetDHCPEnabled(res);
  }
  else if (strcmp(szParam, "host_name") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetHostName(res);
  }
  else if (strcmp(szParam, "ip_address") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetIPAddress(false, res);
  }
  else if (strcmp(szParam, "subnet_mask") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetSubnetMask(false, res);
  }
  else if (strcmp(szParam, "default_gateway") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetDefaultGateway(false, res);
  }
  else if (strcmp(szParam, "primary_dns") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetPrimaryDNS(false, res);
  }
  else if (strcmp(szParam, "secondary_dns") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetSecondaryDNS(false, res);
  }
  else if (strcmp(szParam, "act_ip_address") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetIPAddress(true, res);
  }
  else if (strcmp(szParam, "act_subnet_mask") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetSubnetMask(true, res);
  }
  else if (strcmp(szParam, "act_default_gateway") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetDefaultGateway(true, res);
  }
  else if (strcmp(szParam, "act_primary_dns") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetPrimaryDNS(true, res);
  }
  else if (strcmp(szParam, "act_secondary_dns") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetSecondaryDNS(true, res);
  }
  else if (strcmp(szParam, "ipconfig_msg") == 0)
  {
    IPConfigByWeb::getInstance()->webifGetTextMessage(res);
  }
  
  // firmware_update.html
  else if (strcmp(szParam, "firmware_update_settings") == 0)
  {
    FirmwareUpdateCtrl::GetInstance()->webifMakeSettingsHtml(res);
  }
  
  // CRM menu
  else if (strcmp(szParam, "crm_top_menu") == 0)
  {
    res << "<div style=\"border-bottom:2px solid #D3D3D3; padding:0px 0px 5px 0px; margin-bottom:15px\">";
    res << "<a href=\"index.html\">Home</a>";
    res << " &middot; <a href=\"firmware_update.html\">Firmware</a>";
    res << " &middot; <small>";
    res << CpuSoftwareVersion::GetInstance()->GetValue();
    res << ", compiled:&nbsp;";
    res << __DATE__;
    res << "/";
    res << __TIME__;
    res << "<br/>";
    res << "</small>";
    res << "</div>";
  }
  
  // iostatus.html
  else if (strcmp(szParam, "iostatus_no_of_io_modules") == 0)
  {
    if (m_pIO351IOModules[0])
      m_pIO351IOModules[0]->webifGetNoOfIOModules(res);
    else
      res << "?";  
  }
}

/*****************************************************************************
 * Function - doGetCGI
 * DESCRIPTION: HTTP GET with URL /get.cgi
 ****************************************************************************/
void WebIfHandler::doGetCGI(HttpRequest& req, HttpResponse& res)
{
  if (req.HasParam("io_status"))
  {
    res.StartHTMLContent();
    
    if (req.GetParam("io_status", "").compare("iob") == 0)
    {
      res << "<h2>IOB</h2><i>Status not implemented!</i>";
    }
    else if (req.GetParam("io_status", "").compare("io351iomodules") == 0)
    {
      for (int i = 0; i < sizeof(m_pIO351IOModules) / sizeof(m_pIO351IOModules[0]); i++)
      {
        if (i)
        {
          res << "<br/>";
        }
        
        res << "<h2>IO351 IO Module #" << (i + 1) << " Status</h2>";
        if (m_pIO351IOModules[i])
          m_pIO351IOModules[i]->webifMakeStatusHtml(res);
        else
          res << "<i>Pointer to IO351 IO Module not set!</i>";
      }
    }
    else
    {
      res << "<b>Unhandled io_status=" << req.GetParam("io_status", "") << "</b>";
    }
  }

  // firmware_update.html
  if (req.HasParam("firmware_update"))
  {
    res.StartHTMLContent();

    if (req.GetParam("firmware_update", "").compare("status") == 0)
    {
      FirmwareUpdateCtrl::GetInstance()->webifMakeStatus(res);
    }
    else
    {
      res << "<b>Unhandled firmware_update=" << req.GetParam("firmware_update", "") << "</b>";
    }
  }
}

/*****************************************************************************
 * Function - doPostCGI
 * DESCRIPTION: HTTP POST with URL /post.cgi (HTML form)
 ****************************************************************************/
void WebIfHandler::doPostCGI(const char* szFormID, HttpRequest& req, HttpResponse& res)
{
  if (strcmp(szFormID, "ip_config") == 0)
  {
    res.StartHTMLContent();
    
    res << "<html><head><title>Please wait...</title>";
    res << IPConfigByWeb::getInstance()->webifPostNewConfig(
      req.HasParam("dhcp_enabled"),
      req.GetParam("hostname", "").c_str(),
      req.GetParam("ip_address", "").c_str(),
      req.GetParam("subnet_mask", "").c_str(),
      req.GetParam("default_gateway", "").c_str(),
      req.GetParam("primary_dns", "").c_str(),
      req.GetParam("secondary_dns", "").c_str()
    );
    res << "</head><body><i>Please wait...</i></body></html>";
  }
  else if (strcmp(szFormID, "pw_config") == 0)
  {
    res.SendRedirect(PasswordSetting::getInstance()->webifPostNewConfig(
      req.GetParam("existing_password", "").c_str(),
      req.GetParam("new_password", "").c_str(),
      req.GetParam("repeated_password", "").c_str()
    ).c_str());
  }
  else if (strcmp(szFormID, "iostatus") == 0)
  {
    if (m_pIO351IOModules[0])
      m_pIO351IOModules[0]->webifSetNoOfIOModules(req.GetParam("no_of_io_modules", 0, 10, 0));
    res.SendRedirect("/iostatus.html");
  }
  // firmware_update.html
  else if (strcmp(szFormID, "firmware_update") == 0)
  {
    FirmwareUpdateCtrl::GetInstance()->webifDoPost(req);
    res.SendRedirect("/firmware_update.html");
  }
}

/*****************************************************************************
 * Function - SetIO351IOModulePointer
 * DESCRIPTION: 
 ****************************************************************************/
 void WebIfHandler::SetIO351Pointer(IO351_NO_TYPE moduleNo, IO351* pIO351)
{
  switch (moduleNo)
  {
  case IO351_IOM_NO_1:
    m_pIO351IOModules[0] = dynamic_cast<IO351IOModule*>(pIO351);
    break;
  case IO351_IOM_NO_2:
    m_pIO351IOModules[1] = dynamic_cast<IO351IOModule*>(pIO351);
    break;
  }
}
