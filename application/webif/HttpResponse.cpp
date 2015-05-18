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
/* FILE NAME        : HttpResponse.cpp                                      */
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
#include <HttpResponse.h>
#include <IOContextStreambuf.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 ****************************************************************************/
HttpResponse::HttpResponse(IOContextStreambuf* pBuf) : std::ostream(pBuf)
{
  m_pBuf = pBuf;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
HttpResponse::~HttpResponse()
{
}

/*****************************************************************************
 * Function - AddParam
 * DESCRIPTION:
 * Adds a parameter which is appended to the URL when SendRedirect is used.
 ****************************************************************************/
void HttpResponse::AddParam(const char* szName, const char* szValue)
{
  if (szName && szValue)
  {
    m_params[szName] = szValue;
  }
}
  
/*****************************************************************************
 * Function - SendRedirect
 * DESCRIPTION:
 * Sends a HTTP redirect (303) to the specified file with parameters appended
 * to the URL added with the AddParam function.
 * Returns true on success, false if the response was committed.
 ****************************************************************************/
bool HttpResponse::SendRedirect(const char* szFile)
{
  if (m_pBuf->IsCommitted())
  {
    return false;
  }
  else  
  {
    *this << "HTTP/1.1 303 See Other\r\n";
    *this << "Server: Micro-Web\r\n";
    *this << "Connection: close\r\n";
    *this << "Location: " << szFile;
    if (m_params.size())
    {
      std::string value;
      
      *this << "?";
      
      for (std::map<std::string, std::string>::iterator itr = m_params.begin(); itr != m_params.end(); itr++)
      {
        if (itr == m_params.begin())
          *this << itr->first;
        else  
          *this << "&" << itr->first;
          
        value = itr->second;
        if (value.size())
        {
          URLEncode(value);
          *this << "=" << value;
        }        
      }
    }
    *this << "\r\n"; // terminate location header
   	*this << "\r\n"; // terminate HTTP headers
    return true;
  }
}
  
/*****************************************************************************
 * Function - SendError
 * DESCRIPTION: 
 * Sends a HTTP 500 error with the specified message
 * Returns true on success, false if the response was committed
 ****************************************************************************/
bool HttpResponse::SendError(const char* szErrorMessage)
{
  if (m_pBuf->IsCommitted())
  {
    return false;
  }
  else
  {
    *this << "HTTP/1.1 500 Error\r\n";
    *this << "Server: Micro-Web\r\n";
    *this << "Connection: close\r\n";
    *this << "Content-Type: text/html\r\n";
    *this << "Transfer-Encoding: chunked\r\n";
    *this << "Expires: -1\r\n";
    *this << "Pragma: no-cache\r\n";
    *this << "Cache-Control: no-store, no-cache\r\n";
   	*this << "\r\n"; // terminate HTTP headers
    
    m_pBuf->SetChunked();
    
    *this << "<html><head><title>" << PRODUCT_NAME << " error</title></head><body><b>" << PRODUCT_NAME << " error:</b> " << szErrorMessage << "</body></html>";
    
    return true;
  }
}
  
/*****************************************************************************
 * Function - StartHTMLContent
 * DESCRIPTION: 
 * Adds HTTP headers and sets IO context stream to chunked.
 * Returns true on success, false if the response was committed
 ****************************************************************************/
bool HttpResponse::StartHTMLContent()
{
  if (m_pBuf->IsCommitted())
  {
    return false;
  }
  else
  {
    *this << "HTTP/1.1 200 OK\r\n";
    *this << "Server: Micro-Web\r\n";
    *this << "Connection: close\r\n";
    *this << "Content-Type: text/html\r\n";
    *this << "Transfer-Encoding: chunked\r\n";
    *this << "Expires: -1\r\n";
    *this << "Pragma: no-cache\r\n";
    *this << "Cache-Control: no-store, no-cache\r\n";
   	*this << "\r\n"; // terminate HTTP headers

    // chunked transfer encoding from now on....  
    return m_pBuf->SetChunked();
  }
}

/*****************************************************************************
 * Function - StartCSVContent
 * DESCRIPTION: 
 * Adds HTTP headers and sets IO context stream to chunked
 * Returns true on success, false if the response was committed
 ****************************************************************************/
bool HttpResponse::StartCSVContent(const char* szFileName)
{
  if (m_pBuf->IsCommitted())
  {
    return false;
  }
  else
  {
    *this << "HTTP/1.1 200 OK\r\n";
    *this << "Server: Micro-Web\r\n";
    *this << "Connection: close\r\n";
    *this << "Content-Type: application/csv\r\n";
    *this << "Content-Disposition: attachment; filename=\"" << szFileName << "\"\r\n";
    *this << "Transfer-Encoding: chunked\r\n";
    *this << "Expires: -1\r\n";
    *this << "Pragma: no-cache\r\n";
    *this << "Cache-Control: no-store, no-cache\r\n";
   	*this << "\r\n"; // terminate HTTP headers

    // chunked transfer encoding from now on....  
    return m_pBuf->SetChunked();
  }
}
  
/*****************************************************************************
 * Function - URLEncode
 * DESCRIPTION:
 ****************************************************************************/
void HttpResponse::URLEncode(std::string& value)
{
	std::string::size_type i = 0;
	char cvalue[4];

	while (i < value.size())
	{
		if ((value[i] >= '0') && (value[i] <= '9')
			  || (value[i] >= 'A') && (value[i] <= 'Z')
			  || (value[i] >= 'a') && (value[i] <= 'z'))
		{
			i++;
		}
		else
		{
			sprintf(cvalue, "%%%02X", value[i] & 0xFF);
			value.erase(i, (std::string::size_type)1);
			value.insert(i, cvalue);
			i += 3;
		}
	}
}
  
