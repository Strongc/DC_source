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
/* CLASS NAME       : SoftwareVersion                                       */
/*                                                                          */
/* FILE NAME        : SoftwareVersion.cpp                                   */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <SoftwareVersion.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define STRING_MAX_LEN 81

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  CREATS AN OBJECT.
 ******************************************************************************/
CpuSoftwareVersion* CpuSoftwareVersion::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
CpuSoftwareVersion* CpuSoftwareVersion::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new CpuSoftwareVersion();
    mInstance->SetValue(CPU_SW_VERSION_NO);
  }
  return mInstance;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool CpuSoftwareVersion::SetValue(int versionNo)
{
  char buffer[STRING_MAX_LEN];

  BcdConv(buffer,versionNo);

  StringDataPoint::SetValue(buffer);
  return true;
}


/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
void BcdConv(char* str, int bcdEncodedValue)
{
  char number_buffer[STRING_MAX_LEN];
  char buffer[STRING_MAX_LEN];
  int i;

  buffer[0] = '\0';
  number_buffer[0] = '\0';

  for (i=0; i < 6; i++)
  {
    int v = bcdEncodedValue & 0x00f00000;
    if (v > 0x00900000)
    {
      strcat(buffer,"x");
    }
    else
    {
      sprintf(number_buffer,"%i",v >> 20);
      strcat(buffer,number_buffer);
    }

    if ((((i+1) % 2) == 0) && (i != 5))
    {
      strcat(buffer,".");
    }

    bcdEncodedValue = bcdEncodedValue << 4;
  }

  sprintf(str,"v%s",buffer);
}
/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/
/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
CpuSoftwareVersion::CpuSoftwareVersion(): StringDataPoint("", STRING_MAX_LEN)
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
*****************************************************************************/
CpuSoftwareVersion::~CpuSoftwareVersion()
{

}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - NOT USED -
 *
 ****************************************************************************/

/*****************************************************************************
  CREATS AN OBJECT.
 ******************************************************************************/
IoSoftwareVersion* IoSoftwareVersion::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
IoSoftwareVersion* IoSoftwareVersion::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new IoSoftwareVersion();
    mInstance->SetValue(IO_SW_VERSION_NO);
  }
  return mInstance;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool IoSoftwareVersion::SetValue(int versionNo)
{
  char buffer[STRING_MAX_LEN];

  BcdConv(buffer,versionNo);

  StringDataPoint::SetValue(buffer);
  return true;
}
/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/
/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
IoSoftwareVersion::IoSoftwareVersion(): StringDataPoint("", STRING_MAX_LEN)
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
*****************************************************************************/
IoSoftwareVersion::~IoSoftwareVersion()
{

}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - NOT USED -
 *
 ****************************************************************************/

/*****************************************************************************
  CREATS AN OBJECT.
 ******************************************************************************/
BootLoadSoftwareVersion* BootLoadSoftwareVersion::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
BootLoadSoftwareVersion* BootLoadSoftwareVersion::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new BootLoadSoftwareVersion();
#ifndef __PC__
  // Address is hardcoded in boot loader project! (see standalone_romcopy.ld in cu3x1Platform_SRC\CU3x2_Bootloader\Bootloader\Link)
  #ifdef TFT_16_BIT_LCD
    mInstance->SetValue(*((int *)(0xBFC007E4)));  
  #else
    mInstance->SetValue(*((int *)(0xBFC006E4)));  
  #endif
#else
    mInstance->SetValue(0);
#endif
  }
  return mInstance;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool BootLoadSoftwareVersion::SetValue(int versionNo)
{
  char buffer[STRING_MAX_LEN];

  BcdConv(buffer,versionNo);

  StringDataPoint::SetValue(buffer);
  return true;
}
/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/
/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
BootLoadSoftwareVersion::BootLoadSoftwareVersion(): StringDataPoint("", STRING_MAX_LEN)
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
*****************************************************************************/
BootLoadSoftwareVersion::~BootLoadSoftwareVersion()
{

}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - NOT USED -
 *
 ****************************************************************************/

