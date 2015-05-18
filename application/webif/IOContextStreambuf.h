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
/* FILE NAME        : IOContextStreambuf.h                                  */
/*                                                                          */
/* CREATED DATE     : 22-06-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Stream buffer with automatick callback to the   */
/*                          web servers I/O context                         */
/****************************************************************************/

/*****************************************************************************
   Protect against multiple inclusion through the use of guards
 ****************************************************************************/
#ifndef __IO_CONTEXT_STREAM_BUF_H__
#define __IO_CONTEXT_STREAM_BUF_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <streambuf>

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: IOContextStreambuf
 * DESCRIPTION: 
 * Stream buffer with automatick callback to the web servers I/O context
 *****************************************************************************/
class IOContextStreambuf : public std::streambuf
{
public:
  /********************************************************************
   LIFECYCLE - Constructor
  ********************************************************************/
  IOContextStreambuf(void* pIOContext, char* pBuffer, int maxLength, void (*pCallback)(void* pIOContext, char* pBuffer, int length));
  
  /********************************************************************
   LIFECYCLE - Destructor
  ********************************************************************/
  ~IOContextStreambuf();

  bool IsCommitted();
	bool SetChunked();

  /********************************************************************
   streambuf overrides
  ********************************************************************/
protected:  
  virtual int overflow(int ch);
  virtual int sync();

private:
  /********************************************************************
   PRIVATE FUNCTIONS
  ********************************************************************/
  void Flush();
    
  /********************************************************************
   PRIVATE ATTRIBUTES
  ********************************************************************/
	bool mDestroying;
  bool mCommitted;
	bool mChunked;
	int mChunkNo;
  void* m_pIOContext;
  char* m_pBuffer;
  int m_maxLength;
  void (*m_pCallback)(void* pIOContext, char* pBuffer, int length);
};

#endif
