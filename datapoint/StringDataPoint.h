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
/* FILE NAME        : StringDataPoint.h                                     */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __STRING_DATA_POINT_H__
#define __STRING_DATA_POINT_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <DataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: StringDataPoint
 * DESCRIPTION: String data point
 *****************************************************************************/
class StringDataPoint : public DataPoint
{
public:
  /*****************************************************************************
  * LIFECYCLE - Constructor
  * INPUT: szValue - initial value
  * INPUT: maxNumberOfChars - maximum string length (terminating \0 NOT included)
  *****************************************************************************/
  StringDataPoint(const char* szValue, int maxNumberOfChars);

  /*****************************************************************************
  * LIFECYCLE - Destructor
  *****************************************************************************/
  ~StringDataPoint();

public:
  /*****************************************************************************
  * PUBLIC METHODS
  *****************************************************************************/
  
  /*****************************************************************************
  * FUNCTION - GetValue
  * DESCRIPTION: Returns the data point value
  *****************************************************************************/
  virtual const char* GetValue(void);

  /*****************************************************************************
  * FUNCTION - SetValue
  * DESCRIPTION: Sets the data point value if the new values length is less
  * or equal to the max length specified during construction. Returns true if
  * the new value was set and not equal to the old value.
  *****************************************************************************/
  bool SetValue(const char* newValue);
		
  /*****************************************************************************
  * FUNCTION - GetMaxNoOfChars
  * DESCRIPTION: Returns the maximum string length specified during construction
  *****************************************************************************/
  int GetMaxNoOfChars();

 /*****************************************************************************
  * FUNCTION - GetMaxByteSize
  * DESCRIPTION: Returns the maximum string size specified during construction
  *****************************************************************************/
  int GetMaxByteSize();

  /*********************************************************************************
  * FUNCTION - ValidateUTF8Sring
  * DESCRIPTION: Validates the string against UTF8 and replaces illegal characters.
  * ARGUMENTS: bCrrectStr - true, modify string; false, don't modify string
  * OUTPUT: true, valid string; false, invalid string
  **********************************************************************************/
  bool ValidateStringUTF8(bool bCrrectStr, const char cReplaceWith = ' ');

  /*****************************************************************************
  * Subject overrides
  *****************************************************************************/
	virtual FLASH_ID_TYPE GetFlashId(void);
	virtual void SaveToFlash(IFlashWriter* pWrite, FLASH_SAVE_TYPE save);
	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs);  

protected:
  /*****************************************************************************
  * PROTECTED METHODS
  *****************************************************************************/

  /*****************************************************************************
  * PROTECTED ATTRIBUTES
  *****************************************************************************/
  char* mString;
  const int mMaxByteSize; 
  const int mMaxNumberOfChars;
};

#endif
