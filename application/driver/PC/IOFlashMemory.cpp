/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : ReadFlashMemory, WriteFlashMemory                     */
/*                                                                          */
/* FILE NAME        : IOFlashMemory.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 01-04-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Implementing Reading and writing flashmemory    */
/****************************************************************************/

#include "IOFlashMemory.h"
#include "FlashMemory.h"
#include "FlashControl.h"
#include <TCHAR.H>

#include <ConfigControl.h>


static wchar_t flashFileName[MAX_PATH];

void ReadFlashMemory(const wchar_t* _flashFileName) 
{ 
  uCPUBusType* flash;
  int          sz;
  wcscpy(flashFileName,_flashFileName);
  GetFlashMemory(&flash, &sz);  
  // Read from file
  HANDLE file = CreateFile(flashFileName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if (file == INVALID_HANDLE_VALUE) {
    FlashControl::GetInstance()->EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_CONFIG);
    FlashControl::GetInstance()->EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_LOG);
    FlashControl::GetInstance()->EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_GSC);  
    return;
  }

  DWORD nBytesRead;
  ReadFile(file, flash,  sz*(sizeof(uCPUBusType)/sizeof(char)), &nBytesRead, NULL) ; 
  CloseHandle(file);
}

void WriteFlashMemory() 
{
  uCPUBusType* flash;
  int          sz;
  GetFlashMemory(&flash, &sz); 

  // Write to file  
  HANDLE file = CreateFile(flashFileName,GENERIC_READ | GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

  if (file == INVALID_HANDLE_VALUE) return;
  
  DWORD numberOfBytesWritten;  
  WriteFile(file,flash, sz*(sizeof(uCPUBusType)/sizeof(char)),&numberOfBytesWritten,NULL);
  CloseHandle(file);

}

void RenameFlashMemory() 
{
  // renames the file
  DeleteFile(_T("_flash.bin"));  
  MoveFile(flashFileName,_T("_flash.bin"));
}