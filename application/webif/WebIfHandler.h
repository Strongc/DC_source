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
/* FILE NAME        : WebIfHandler.h                                        */
/*                                                                          */
/* CREATED DATE     : 22-06-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Handler for dynamic web content                 */
/****************************************************************************/

#ifndef __WEB_IF_HANDLER_H__
#define __WEB_IF_HANDLER_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <ostream>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <io351.h>

/*****************************************************************************
  LOCAL INCLUDES
 *****************************************************************************/
#include <HttpRequest.h>
#include <HttpResponse.h>

/*****************************************************************************
 FORWARD DECLARATIONS
*****************************************************************************/
class IO351IOModule;

/*****************************************************************************
 * CLASS: WebIfHandler
 * DESCRIPTION: Handler for dynamic web content 
 *****************************************************************************/
class WebIfHandler 
{
private:
  WebIfHandler();
  
public:
  static WebIfHandler* GetInstance();

  // methods called by WebIf.cpp  
  void doGetFN(const char* szParam, std::ostream& res);
  void doGetCGI(HttpRequest& req, HttpResponse& res);
  void doPostCGI(const char* szFormID, HttpRequest& req, HttpResponse& res);
  
  /*****************************************************************************
  * Place methods which must run on target between the define below
  *****************************************************************************/
#ifndef __PC__  
  // methods class by IO351 derived classes
  void SetIO351Pointer(IO351_NO_TYPE moduleNo, IO351* pIO351);
#endif  
  
private:
  static WebIfHandler* mpInstance;

  /*****************************************************************************
  * Place attributes which must run on target between the define below
  *****************************************************************************/
#ifndef __PC__  
  IO351IOModule* m_pIO351IOModules[NO_OF_IO351_IOM_NO];
#endif  
};

#endif

