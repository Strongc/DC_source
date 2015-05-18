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
/* CLASS NAME       : IOContextStreambuf                                    */
/*                                                                          */
/* FILE NAME        : IOContextStreambuf.cpp                                */
/*                                                                          */
/* CREATED DATE     : 22-06-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See header file                                 */
/****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 *****************************************************************************/
#include "IOContextStreambuf.h"

using std::streambuf;

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 ****************************************************************************/
IOContextStreambuf::IOContextStreambuf(void* pIOContext, char* pBuffer, int maxLength, void (*pCallback)(void* pIOContext, char* pBuffer, int length)) : streambuf()
{
	m_pIOContext = pIOContext;
	m_pBuffer = pBuffer;
	m_maxLength = maxLength - 10;	// -10 because we need buffer space to add chunked info
	m_pCallback = pCallback;
	mDestroying = false;
  mCommitted = false;
	mChunked = false;
	mChunkNo = 0;

	// init buffer pointers
	setp(m_pBuffer, m_pBuffer + m_maxLength);
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
IOContextStreambuf::~IOContextStreambuf()
{
	mDestroying = true;

  // make sure all data is send
  Flush();
}

/*****************************************************************************
 * Function - overflow
 * DESCRIPTION:
 ****************************************************************************/
int IOContextStreambuf::overflow(int c_)
{
	// flush the buffer
	Flush();
  
	// append to buffer if NOT EOF
	if (c_ != EOF)
	{
		sputc(c_);
	}

	return 0;
}

/*****************************************************************************
 * Function - overflow
 * DESCRIPTION:
 ****************************************************************************/
int IOContextStreambuf::sync()
{
	// flush the buffer
	Flush();
	return 0;
}

/*****************************************************************************
 * Function - IsCommitted
 * DESCRIPTION:
 ****************************************************************************/
bool IOContextStreambuf::IsCommitted()
{
  return mCommitted || ((pptr() - pbase()) != 0);
}

/*****************************************************************************
 * Function - SetChunked
 * DESCRIPTION:
 ****************************************************************************/
bool IOContextStreambuf::SetChunked()
{
  if (mChunked)
  {
    return false;
  }
  else
  {
  	Flush();
  	mChunked = true;
    return true;
  }
}

/*****************************************************************************
 * Function - Flush
 * DESCRIPTION: Sends pending data to the web server I/O context
 ****************************************************************************/
void IOContextStreambuf::Flush()
{
	int count = (pptr() - pbase());

  // data pending?
	if (count) 
  {
    // set committed flag
    mCommitted = true;

		// handle chunked transfer encoding
		if (mChunked)
		{
			char szChunk[10];
			int i, len;

			if (mChunkNo)
				sprintf(szChunk, "\r\n%x\r\n", count);
			else
				sprintf(szChunk, "%x\r\n", count);

			len = strlen(szChunk);

			memmove(&m_pBuffer[len], m_pBuffer, count);

			for (i = 0; i < len; i++)
			{
				m_pBuffer[i] = szChunk[i];
			}

			count += len;

			mChunkNo++;
		}
    
    // send data
		m_pCallback(m_pIOContext, m_pBuffer, count);

    // reset buffer pointers
		setp(m_pBuffer, m_pBuffer + m_maxLength);
	}

  if (mDestroying && mChunked)
  {
    // terminate chunked transfer
    count = 0;
    m_pBuffer[count++] = '\r';
    m_pBuffer[count++] = '\n';
    m_pBuffer[count++] = '0';
    m_pBuffer[count++] = '\r';
    m_pBuffer[count++] = '\n';
    m_pBuffer[count++] = '\r';
    m_pBuffer[count++] = '\n';
		m_pCallback(m_pIOContext, m_pBuffer, count);
  }
}

