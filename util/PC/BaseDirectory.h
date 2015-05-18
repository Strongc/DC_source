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
/* FILE NAME        : BaseDirectory.h                                       */
/*                                                                          */
/* CREATED DATE     : 23-05-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION : BaseDirectory is initialized by PCMR.c.         */
/* BaseDirectory is used by Language to locate the excel-file Texts.xls     */
/* BaseDirectory is also used when FlashBlock stores bin files.             */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_BaseDirectory_h
#define mrc_BaseDirectory_h

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define BASE_DIRECTORY_LENGTH 260

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class BaseDirectory
{
public:
  static BaseDirectory* GetInstance(void);

  const char* Get(void){return mBaseDirectory;}
  void Set(const char* baseDirectory);
 
private:
  BaseDirectory(){}
  ~BaseDirectory(){}

  static BaseDirectory* mInstance;
  char mBaseDirectory[BASE_DIRECTORY_LENGTH];
};

#endif
