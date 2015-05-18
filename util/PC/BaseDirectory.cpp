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
/* CLASS NAME       : BaseDirectory                                         */
/*                                                                          */
/* FILE NAME        : BaseDirectory.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 29-05-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <string>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "BaseDirectory.h"


BaseDirectory* BaseDirectory::mInstance = 0;

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
BaseDirectory* BaseDirectory::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new BaseDirectory();
  }
  return mInstance;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
void BaseDirectory::Set(const char* baseDirectory)
{
  strncpy(mBaseDirectory, baseDirectory, BASE_DIRECTORY_LENGTH-1);

  mBaseDirectory[BASE_DIRECTORY_LENGTH-1] = '\0';
}


