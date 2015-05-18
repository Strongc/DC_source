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
/* CLASS NAME       : HttpRequest                                           */
/*                                                                          */
/* FILE NAME        : HttpRequest.h                                         */
/*                                                                          */
/* CREATED DATE     : 22-06-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Http request helper                             */
/****************************************************************************/

#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <string>
#include <map>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 *****************************************************************************/

/*****************************************************************************
 FORWARD DECLARATIONS
*****************************************************************************/

/*****************************************************************************
 * CLASS: HttpRequest
 * DESCRIPTION: 
 *****************************************************************************/
class HttpRequest 
{
public:
  HttpRequest(const char* szParams);
  ~HttpRequest();
  
  bool HasParam(const char* szName);
  int GetParam(const char* szName, int minValue, int maxValue, int defaultValue);
  float GetParam(const char* szName, float minValue, float maxValue, float defaultValue);
  std::string GetParam(const char* szName, const char* szDefaultValue);

private:
  void URLDecode(std::string& value);
  
  std::map<std::string, std::string> m_params;
};

#endif
