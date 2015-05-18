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
/* FILE NAME        : HttpRequest.cpp                                       */
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

/*****************************************************************************
  LOCAL INCLUDES
 *****************************************************************************/
#include <HttpRequest.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

 
/*****************************************************************************
  std::string utils
 *****************************************************************************/
inline std::string trim_right(const std::string & s, const std::string & t = " ")
{ 
  std::string d (s); 
  std::string::size_type i(d.find_last_not_of (t));
  if (i == std::string::npos)
    return "";
  else
   return d.erase(d.find_last_not_of (t) + 1); 
}  // end of trim_right

inline std::string trim_left (const std::string & s, const std::string & t = " ") 
{ 
  std::string d (s); 
  return d.erase (0, s.find_first_not_of (t)) ; 
}  // end of trim_left

inline std::string trim (const std::string & s, const std::string & t = " ")
{ 
  std::string d(s); 
  return trim_left(trim_right(d, t), t); 
}  // end of trim

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 ****************************************************************************/
HttpRequest::HttpRequest(const char* szParams)
{
  bool nameActive = true;
  std::string name = "";
  std::string value = "";

  for (int i = 0; i < strlen(szParams); i++)
  {
    if (szParams[i] == '=')
    {
      nameActive = false;
    } 
    else if (szParams[i] == '&')
    {
      nameActive = true;

      URLDecode(value);
      m_params[name] = value;
      
      name = "";
      value = "";
    }
    else
    {
      if (nameActive)
        name += szParams[i];
      else
        value += szParams[i];
    }
  }

  if (!name.empty())
  {
    URLDecode(value);
    m_params[name] = value;
  }
}
  
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
HttpRequest::~HttpRequest()
{
}

/*****************************************************************************
 * Function - HasParam
 * DESCRIPTION: 
 *****************************************************************************/
bool HttpRequest::HasParam(const char* szName)
{
  return m_params.find(szName) != m_params.end() ? true : false;
}

/*****************************************************************************
 * Function - GetParam (int)
 * DESCRIPTION: 
 *****************************************************************************/
int HttpRequest::GetParam(const char* szName, int minValue, int maxValue, int defaultValue)
{
  if (m_params.find(szName) != m_params.end())
  {
    int newValue = atoi(m_params[szName].c_str());
    
    if (newValue < minValue)
      newValue = minValue;
      
    if (newValue > maxValue)
      newValue = maxValue;
      
    return newValue;    
  }
  else
  {
    return defaultValue;
  }
}

/*****************************************************************************
 * Function - GetParam (float)
 * DESCRIPTION: 
 *****************************************************************************/
float HttpRequest::GetParam(const char* szName, float minValue, float maxValue, float defaultValue)
{
  if (m_params.find(szName) != m_params.end())
  {
    float newValue = atof(m_params[szName].c_str());
    
    if (newValue < minValue)
      newValue = minValue;
      
    if (newValue > maxValue)
      newValue = maxValue;
      
    return newValue;    
  }
  else
  {
    return defaultValue;
  }
}

/*****************************************************************************
 * Function - GetParam (string)
 * DESCRIPTION: 
 *****************************************************************************/
std::string HttpRequest::GetParam(const char* szName, const char* szDefaultValue)
{
  if (m_params.find(szName) != m_params.end())
  {
    return trim(m_params[szName]);    
  }
  else
  {
    return szDefaultValue;
  }
}

/*****************************************************************************
 * Function - URLDecode
 * DESCRIPTION: 
 *****************************************************************************/
void HttpRequest::URLDecode(std::string& value)
{
	int i;
	std::string str;
	char cvalue[2];

	// replace '+' with ' ' (space)
	i = value.find("+");
	while (i < std::string::npos)
	{
		value.replace(i, 1, " ");
		i = value.find("+", i + 1);
	}

	// replace "%xx" with character with value xx
	i = value.find("%");
	while (i < std::string::npos)
	{
		char* end;

		str = value.substr(i + 1, 2);
		cvalue[0] = strtol(str.c_str(), &end, 16);
		cvalue[1] = 0;

		value.erase(i, 3); //delete "%xx"
		value.insert(i, std::string(cvalue)); // insert char

		i = value.find("%", i + 1);
	}
}

