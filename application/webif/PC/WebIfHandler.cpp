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
}

/*****************************************************************************
 * Function - doGetCGI
 * DESCRIPTION: HTTP GET with URL /get.cgi
 ****************************************************************************/
void WebIfHandler::doGetCGI(HttpRequest& req, HttpResponse& res)
{
}

/*****************************************************************************
 * Function - doPostCGI
 * DESCRIPTION: HTTP POST with URL /post.cgi (HTML form)
 ****************************************************************************/
void WebIfHandler::doPostCGI(const char* szFormID, HttpRequest& req, HttpResponse& res)
{
}

