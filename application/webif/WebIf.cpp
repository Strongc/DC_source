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
/* CLASS NAME       : EthernetCtrl                                          */
/*                                                                          */
/* FILE NAME        : WebIf.cpp                                             */
/*                                                                          */
/* CREATED DATE     : 22-06-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See header file                                 */
/****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <ostream>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
 
/*****************************************************************************
  LOCAL INCLUDES
 *****************************************************************************/
#include <WebIfHandler.h>
#include <IOContextStreambuf.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  LOCAL VARIABLES
*****************************************************************************/
static WebIfHandler* spWebIfHandler = WebIfHandler::GetInstance();

/*****************************************************************************
  PUBLIC FUNCTIONS
 *****************************************************************************/

extern "C" void webifHttpGetFN_C(const char* szParam, void* pIOContext, char* pBuffer, int maxLength, void (*pCallback)(void* pIOContext, char* pBuffer, int length))
{
  IOContextStreambuf buf(pIOContext, pBuffer, maxLength, pCallback);
  std::ostream res(&buf);
  
  if (spWebIfHandler)
  {
    spWebIfHandler->doGetFN(szParam, res);
  }
  else
  {
    res << "&lt;ERROR: spWebIfHandler == NULL&gt;";
  }
}

extern "C" void webifHttpGetCGI_C(const char* szParams, void* pIOContext, char* pBuffer, int maxLength, void (*pCallback)(void* pIOContext, char* pBuffer, int length))
{
  IOContextStreambuf buf(pIOContext, pBuffer, maxLength, pCallback);
  HttpRequest req(szParams);
  HttpResponse res(&buf);
  
  if (spWebIfHandler)
  {
    spWebIfHandler->doGetCGI(req, res);
    
    if (!buf.IsCommitted())
    {
      res.SendError("Unknown get.cgi request!");
    }
  }
  else
  {
    res.SendError("spWebIfHandler == NULL");
  }
}

extern "C" void webifHttpPostCGI_C(const char* szParams, void* pIOContext, char* pBuffer, int maxLength, void (*pCallback)(void* pIOContext, char* pBuffer, int length))
{
  IOContextStreambuf buf(pIOContext, pBuffer, maxLength, pCallback);
  HttpRequest req(szParams);
  HttpResponse res(&buf);
  
  if (req.HasParam("form_id"))
  {
    if (spWebIfHandler)
    {
      spWebIfHandler->doPostCGI(req.GetParam("form_id", "").c_str(), req, res);
      
      if (!buf.IsCommitted())
      {
        res.SendRedirect("/index.html");
      }
    }
    else
    {
      res.SendError("spWebIfHandler == NULL");
    }
  }
  else
  {
    res.SendError("form_id missing in post request!");
  }
}

