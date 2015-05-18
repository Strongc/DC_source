/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: DC                                               */
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
/* CLASS NAME       : Languages                                             */
/*                                                                          */
/* FILE NAME        : Languages.h                                           */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __LANGUAGES_H__
#define __LANGUAGES_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef __PC__
// Stuff used by the GetString function, when running on a PC
#include "OdbcInterface.h"
#include <windows.h>
#include <TCHAR.h>
#include "UTF16-UTF8.h"
#include "BaseDirectory.h"
#endif
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <string_id.h>
#include <subject.h>
#include <observer.h>
/*****************************************************************************
DEFINES
*****************************************************************************/
#ifdef __PC__
  #define MAX_STRLEN  1024
#endif
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
/*****************************************************************************
* CLASS:
* DESCRIPTION:
*
*****************************************************************************/
class Languages : public Subject, Observer
{
public:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  static Languages* GetInstance();
  const char* GetString(STRING_ID Id);

  void SetLanguage(LANGUAGE_TYPE Language);
  LANGUAGE_TYPE GetLanguage();

  const char* GetLanguageName(LANGUAGE_TYPE language);
  STRING_ID GetHelpStingId(U16 index);

#ifdef __PC__
  void LoadCurrentLanguageFromExcel(void);
  void UseStringsFromExcel(bool useExcel);
  bool IsUsingStringsFromExcel(void){return mUseExcel;}
#endif

  virtual void SubscribtionCancelled(Subject* pSubject);
  virtual void Update(Subject* pSubject);
  virtual void SetSubjectPointer(int Id,Subject* pSubject);
  virtual void ConnectToSubjects(void);

private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  void ChangeFont(void);
  void SubstituteCuNumberTags(char* text);
  void SubstituteKeyNameTags(char* text);

  /********************************************************************
                LIFECYCLE - Default constructor.
  ********************************************************************/
  Languages();
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  virtual ~Languages();

  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
  static Languages* mInstance;
  LANGUAGE_TYPE  mLanguage;
  Subject* mpSubject;
  char* mpStrings; // Uncompressed strings.
  char* mpPrevStrings;
  bool mUseExcel;

};
#endif
