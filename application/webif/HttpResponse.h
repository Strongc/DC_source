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
/* CLASS NAME       : HttpResponse                                          */
/*                                                                          */
/* FILE NAME        : HttpResponse.h                                        */
/*                                                                          */
/* CREATED DATE     : 22-06-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Http response helper                            */
/****************************************************************************/

#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <map>
#include <string>
#include <ostream>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 *****************************************************************************/

/*****************************************************************************
 FORWARD DECLARATIONS
*****************************************************************************/
class IOContextStreambuf;

/*****************************************************************************
 * CLASS: HttpResponse
 * DESCRIPTION: 
 *****************************************************************************/
class HttpResponse : public std::ostream
{
public:
  HttpResponse(IOContextStreambuf* pBuf);
  ~HttpResponse();

  void AddParam(const char* szName, const char* szValue = "");
  bool SendRedirect(const char* szFile);
  bool SendError(const char* szErrorMessage);
  bool StartHTMLContent();
  bool StartCSVContent(const char* szFileName);
  
private:
  void URLEncode(std::string& value);
  
  IOContextStreambuf* m_pBuf;
  std::map<std::string, std::string> m_params;
};

#endif

