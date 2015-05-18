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
/* CLASS NAME       : StringDataPoint                                       */
/*                                                                          */
/* FILE NAME        : StringDataPoint.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "StringDataPoint.h"
#include <ConvertUTF.h> 

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * FUNCTION - Constructor
 * DESCRIPTION: See header file
 * The mMaxByteSize is initialized to 4 x maxNumberOfChars 
 * to accommendate even the largest UTF-8 encoded string.
 *****************************************************************************/
StringDataPoint::StringDataPoint(const char* szValue, int maxNumberOfChars) : mMaxNumberOfChars(maxNumberOfChars), mMaxByteSize(maxNumberOfChars*4)
{
  mString = OS_NEW(char[mMaxByteSize + 1]);
  if (strlen(szValue) <= (mMaxByteSize))
    strcpy(mString, szValue);
  else  
    strcpy(mString, "");
  mQuality = DP_AVAILABLE;
}

/*****************************************************************************
 * FUNCTION - Destructor
 * DESCRIPTION: See header file
 ****************************************************************************/
StringDataPoint::~StringDataPoint()
{
  delete mString;
}

/*****************************************************************************
 * FUNCTION - GetValue
 * DESCRIPTION: See header file
 *****************************************************************************/
const char* StringDataPoint::GetValue()
{
  return mString;
}

/*****************************************************************************
 * FUNCTION - SetValue
 * DESCRIPTION: See header file
 *****************************************************************************/
bool StringDataPoint::SetValue(const char* newValue)
{
  bool notify = false;

  OS_EnterRegion();
  if ((strlen(newValue) <= mMaxByteSize) && (strcmp(mString, newValue) != 0))
  {
      strcpy(mString, newValue);
      notify = true;
  }
  OS_LeaveRegion();
  
  notify |= SetQuality(DP_AVAILABLE, false);
  
  if (notify)  
    NotifyObservers();

  NotifyObserversE();
  
  return notify;
}

/*****************************************************************************
* FUNCTION - GetMaxNoOfChars
* DESCRIPTION: See header file
*****************************************************************************/
int StringDataPoint::GetMaxNoOfChars()
{
  return mMaxNumberOfChars;
}

/*****************************************************************************
* FUNCTION - GetMaxByteLength
* DESCRIPTION: See header file
*****************************************************************************/
int StringDataPoint::GetMaxByteSize()
{
  return mMaxByteSize;
}

/*****************************************************************************
* FUNCTION - GetFlashId
* DESCRIPTION: 
*****************************************************************************/
FLASH_ID_TYPE StringDataPoint::GetFlashId(void)
{
	return FLASH_ID_STRING_DATA_POINT;
}

/*****************************************************************************
* FUNCTION - SaveToFlash
* DESCRIPTION: 
*****************************************************************************/
void StringDataPoint::SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
{
	pWriter->WriteString(mString, mMaxByteSize);
}

/*****************************************************************************
* FUNCTION - LoadFromFlash
* DESCRIPTION: 
*****************************************************************************/
void StringDataPoint::LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
{
	pReader->ReadString(mString, mMaxByteSize, mString);
}

/*********************************************************************************
* FUNCTION - ValidateStringUTF8
* DESCRIPTION: Validates the string against UTF8 and replaces illegal characters.
* ARGUMENTS: bCrrectStr - true, modify string; false, don't modify string
* OUTPUT: true, valid string; false, invalid string
**********************************************************************************/ 
bool StringDataPoint::ValidateStringUTF8(bool bCrrectStr, const char pReplaceWith)
{
  int i = 0;
  int char_length = 1; 
  unsigned char* valid_string = (unsigned char*) mString;
  int str_length = strlen(mString);
  bool is_str_valid = true;
  
  while (i < str_length)
  {        
    char_length = 1;                                                   //1 byte     
    if      ((valid_string[i] & 0xe0) == 0xc0) char_length = 2;        //2 bytes          
    else if ((valid_string[i] & 0xf0) == 0xe0) char_length = 3;        //3 bytes                 
    else if ((valid_string[i] & 0xf8) == 0xf0) char_length = 4;        //4 bytes            

    if(!(isLegalUTF8Sequence(&valid_string[i], &valid_string[i + char_length])))
    {
      is_str_valid = false;
      if(bCrrectStr)
      {
        for(int j = 0; j < char_length; j++)
        {
          valid_string[i + j] = pReplaceWith;
        }
      }
    }
    i+= char_length;
  }

  mString = (char*) valid_string;
  return is_str_valid;  
}

