/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/*                                                                          */
/* FILE NAME        : utf8_util_test.c                                      */
/*                                                                          */
/* CREATED DATE     : 20-01-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/


#include <iostream>
#include <fstream>


/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "utf8_util.h"

/*****************************************************************************
 * Function - Gsm338FileConvertion
 * DESCRIPTION: used for testing uft8_util
 *
 *****************************************************************************/
bool Gsm338FileConvertion(const char* filename)
{
  
  GF_UINT1 isGsm388 = 0;
  int len = 0;
  const int maxFileSize = 64000;
  char utf8_buf[maxFileSize];
  char sms_buf[maxFileSize];

  char outfile1[256];
  char outfile2[256];
  strncpy(outfile1, filename, 200);
  strncpy(outfile2, filename, 200);
  
  memset(utf8_buf, 0, maxFileSize);
  memset(sms_buf, 0, maxFileSize);

  std::ifstream in(filename, std::ios::in);

	if (in.good())
	{
    //TODO parse line by line
    //while(in.getline(
		in.read((char*)&utf8_buf, maxFileSize);		
    in.close();
	}
  else
  {
    return 0;
  }
  
  // convert to sms format (raw GSM3.38 or ascii-hex-encoded UCS-2)
  len = ConvertUtf8ToSms(utf8_buf, sms_buf, maxFileSize, &isGsm388);

  strcat(outfile1, (isGsm388 ? "_1_gsm338.txt" : "_1_ucs-2.txt"));
  
  std::ofstream of1(outfile1, std::ios::out);
  const char* p1 = (const char*)sms_buf;
  of1.write(p1, len);
  of1.flush();
  of1.close();

  len = ConvertSmsToUtf8(sms_buf, utf8_buf, maxFileSize, !isGsm388);

  strcat(outfile2, "_2_utf8.txt");

  std::ofstream of2(outfile2, std::ios::out);
  const char* p2 = (const char*)utf8_buf;
  of2.write(p2, len);
  of2.flush();
  of2.close();


  return 1;
}





