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
 /* FILE NAME        : Languages.cpp                                         */
 /*                                                                          */
 /* CREATED DATE     :                                                       */
 /*                                                                          */
 /* SHORT FILE DESCRIPTION :                                                 */
 /*                                                                          */
 /****************************************************************************/
 /*****************************************************************************
   SYSTEM INCLUDES
  *****************************************************************************/

 /*****************************************************************************
   PROJECT INCLUDES
  *****************************************************************************/
#include <GUI.h>
#include <MPCFonts.h>
#include "ZLIB/zlib.h"

 /*****************************************************************************
   LOCAL INCLUDES
  ****************************************************************************/
#include <string_id.h>
#include <Languages.h>
#include "LanguagesStringTable.h"
#include <LanguagesDataPoint.h>

 /*****************************************************************************
   DEFINES
  *****************************************************************************/
extern COMPRESSED_STRINGS LanguagesCompressedStrings[LANG_GEN_LANGUAGE_COUNT];
extern unsigned int LanguagesStringOffsets[LANG_GEN_LANGUAGE_COUNT][LANG_GEN_STRING_COUNT];
extern unsigned int HelpStringIds[LANG_GEN_HELP_STRING_COUNT];

extern "C" GUI_CONST_STORAGE GUI_FONT Helvetica_57_11;
extern "C" GUI_CONST_STORAGE GUI_FONT Helvetica_57_13;
extern "C" GUI_CONST_STORAGE GUI_FONT Helvetica_57_18;
extern "C" GUI_CONST_STORAGE GUI_FONT GB2312_S1112;

 /*****************************************************************************
   TYPE DEFINES
  *****************************************************************************/

 /*****************************************************************************
  INITIALIZATION
 ******************************************************************************/
Languages* Languages::mInstance = 0;

 /*****************************************************************************
  *
  *
  *              PUBLIC FUNCTIONS
  *
  *
  *****************************************************************************/ 
/*****************************************************************************
  * Function - GetInstance
  * DESCRIPTION:
  ****************************************************************************/
Languages* Languages::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new Languages();
  }
  return mInstance;
}

 /*****************************************************************************
  * Function - GetString
  * DESCRIPTION:
  ****************************************************************************/
const char* Languages::GetString(STRING_ID id)
{
  bool valid_id = (id > 0 && id < LANG_GEN_STRING_COUNT);

#ifdef __PC__
  if (mUseExcel)
  {
    return valid_id ? &mpStrings[MAX_STRLEN * id] : "";
  }
  else
#endif
  {
    return valid_id ? &mpStrings[LanguagesStringOffsets[mLanguage][id]] : "";
  }
}

#ifdef __PC__
 /*****************************************************************************
  * Function - UseStringsFromExcel
  * DESCRIPTION:
  ****************************************************************************/
  void Languages::UseStringsFromExcel(bool useExcel)
  {
    mUseExcel = useExcel;

    if (mUseExcel)
    {
      LoadCurrentLanguageFromExcel();
    }
  }
 /*****************************************************************************
  * Function - LoadCurrentLanguageFromExcel
  * DESCRIPTION:
  ****************************************************************************/
  void Languages::LoadCurrentLanguageFromExcel()
  {
    try
    {
      if (mpStrings != NULL)
      {
        delete mpStrings;
      }
      
      mpStrings = new char[(LANG_GEN_STRING_COUNT * MAX_STRLEN)];

      memset(mpStrings,0,(LANG_GEN_STRING_COUNT * MAX_STRLEN));

      wchar_t szConnection[512];
      UTF16 szBaseDir[MAX_PATH * 3 +1];
      UTF82UTF16(szBaseDir, (UTF8*)BaseDirectory::GetInstance()->Get(), MAX_PATH);

      _stprintf(szConnection, _T("Driver=MICROSOFT EXCEL DRIVER (*.XLS);DBQ=%sCu361Texts.xls"), szBaseDir );

      OdbcConnection* p_odbc_conn = new OdbcConnection( szConnection );

      if (mLanguage >= FIRST_LANGUAGE && mLanguage <= LAST_LANGUAGE)
      {
        OdbcStatement stat(p_odbc_conn);

        TCHAR szSQL[512];
        TCHAR sz_string[MAX_STRLEN];
        long string_id = 0;

        _stprintf(szSQL, _T("SELECT L_%d, L_0, Id FROM all_strings"), mLanguage);
        stat.ExecDirect(szSQL);

        while (stat.Fetch())
        {
          stat.GetString(1,sz_string);
          string_id = stat.GetLong(3);

          // use developer-language string if the string is empty
          if (sz_string[0] == '\0')
          {
            stat.GetString(2,sz_string+1);
            sz_string[0] = '{';

            int len = wcslen(sz_string);

            sz_string[len] = '}';
            sz_string[len+1] = '\0';
          }

          if (string_id >= 0 && string_id <= LANG_GEN_STRING_COUNT)
          {
            int noBytes = UTF162UTF8(&mpStrings[MAX_STRLEN * string_id], (char*)sz_string, MAX_STRLEN);
          }
        }
      }
      delete p_odbc_conn;

    }
    catch(TCHAR* pError)
    {
      MessageBox(NULL,pError,NULL,MB_OK);
    }
  }
#endif


 /*****************************************************************************
  * Function - SetLanguage
  * DESCRIPTION:
  ****************************************************************************/
void Languages::SetLanguage(LANGUAGE_TYPE language)
{
#ifdef LANG_GEN_DEV_MODE_LANGUAGE
  #ifndef __PC__
    language = (LANGUAGE_TYPE)DEV_LANGUAGE;
  #endif
#endif

  if (mLanguage == language)
  {
    return;
  }

#ifdef __PC__
  if (mUseExcel)
  {
    mLanguage = language;
    LoadCurrentLanguageFromExcel();
  }
  else
#endif
  {
    if (mpPrevStrings != NULL)
    {
      delete mpPrevStrings;
    }
    mpPrevStrings = mpStrings;

    char* target;
    int dest_len = LanguagesCompressedStrings[language].uncompressed_size;
    target = new char[LanguagesCompressedStrings[language].uncompressed_size];
    mpStrings = target;

    int rc = uncompress((Bytef*)target,
      (uLongf*)&dest_len, 
      (Bytef*)LanguagesCompressedStrings[language].p_compressed_data,
      LanguagesCompressedStrings[language].compressed_size);

    if (rc != Z_OK)
    {
      delete target;
      mpStrings = mpPrevStrings;
    }
    else
    {
      for (int i = 0; i < LANG_GEN_STRING_COUNT; i++)
      {
        SubstituteCuNumberTags(&mpStrings[LanguagesStringOffsets[language][i]]);

        SubstituteKeyNameTags(&mpStrings[LanguagesStringOffsets[language][i]]);
      }
      mLanguage = language;
    }
  }

  ChangeFont();
  NotifyObservers();
  

}

 /*****************************************************************************
  * Function - GetLanguage
  * DESCRIPTION:
  ****************************************************************************/
LANGUAGE_TYPE Languages::GetLanguage()
{
  return mLanguage;
}

 /*****************************************************************************
  * Function - GetLanguageName
  * DESCRIPTION:
  ****************************************************************************/
const char* Languages::GetLanguageName(LANGUAGE_TYPE language)
{
  int string_id = 0;
  switch (language)
  {
  case UK_LANGUAGE:
    string_id = SID_BRITISH_ENGLISH_; 
    break;
  case DK_LANGUAGE:
    string_id = SID_DANISH_;
    break;
  case DE_LANGUAGE:
    string_id = SID_GERMAN_UK_;
    break;
  case FR_LANGUAGE:
    string_id = SID_FRENCH_;
    break;
  case IT_LANGUAGE:
    string_id = SID_ITALIAN_;
    break;
  case ES_LANGUAGE:
    string_id = SID_SPANISH_;
    break;
  case PT_LANGUAGE:
    string_id = SID_PORTUGUESE_;
    break;
  case GR_LANGUAGE:
    string_id = SID_GREEK_;
    break;
  case NL_LANGUAGE:
    string_id = SID_DUTCH_;
    break;
  case SE_LANGUAGE:
    string_id = SID_SWEDISH_;
    break;
  case FI_LANGUAGE:
    string_id = SID_FINNISH_;
    break;
  case PL_LANGUAGE:
    string_id = SID_POLISH_;
    break;
  case RU_LANGUAGE:
    string_id = SID_RUSSIAN_;
    break;
  case CH_LANGUAGE:
    string_id = SID_CHINESE_;
    break;
  case KR_LANGUAGE:
    string_id = SID_KOREAN_;
    break;
  case JP_LANGUAGE:
    string_id = SID_JAPANESE_;
    break;
  case TR_LANGUAGE:
    string_id = SID_TURKISH_;
    break;
  case CZ_LANGUAGE:
    string_id = SID_CZECH_;
    break;   
  case HU_LANGUAGE:
    string_id = SID_HUNGARIAN_;
    break; 
  case BG_LANGUAGE:
    string_id = SID_BULGARIAN_;
    break;
  case SL_LANGUAGE:
    string_id = SID_SLOVENIAN_;
    break;
  case HR_LANGUAGE:
    string_id = SID_CROATIAN_;
    break;
  case DEV_LANGUAGE:
    return "Developer";
  default:
    return "Unknown Language";
  }
  
  return GetString(string_id);
}

 /*****************************************************************************
  * Function - GetHelpStingId
  * DESCRIPTION:
  ****************************************************************************/
STRING_ID Languages::GetHelpStingId(U16 index)
{
  if (index > LANG_GEN_HELP_STRING_COUNT)
  {
    return (STRING_ID) 0;
  }
  else
  {
    return (STRING_ID) HelpStringIds[index];
  }
}

 /*****************************************************************************
  * Function - SubscribtionCancelled
  * DESCRIPTION:
  ****************************************************************************/
void Languages::SubscribtionCancelled(Subject* pSubject)
{
  mpSubject = NULL;
}

 /*****************************************************************************
  * Function - Update
  * DESCRIPTION:
  ****************************************************************************/
void Languages::Update(Subject* pSubject)
{
  if (mpSubject)
  {
    LanguagesDataPoint* p_language_dp = LanguagesDataPoint::GetInstance();

    #ifdef LANG_GEN_DEV_MODE_LANGUAGE
      p_language_dp->SetValue(DEV_LANGUAGE);
    #else
      if (p_language_dp->GetValue() == DEV_LANGUAGE)
      {
        p_language_dp->SetValue(DEFAULT_LANGUAGE);
      }
    #endif //  LANG_GEN_DEV_MODE_LANGUAGE

    SetLanguage(p_language_dp->GetValue());
  }
}

 /*****************************************************************************
  * Function - SetSubjectPointer
  * DESCRIPTION:
  ****************************************************************************/
void Languages::SetSubjectPointer(int id, Subject* pSubject)
{
  mpSubject = pSubject;
}

 /*****************************************************************************
  * Function - ConnectToSubjects
  * DESCRIPTION:
  ****************************************************************************/
void Languages::ConnectToSubjects(void)
{
  if (mpSubject)
  {
    mpSubject->Subscribe(this);

#ifdef LANG_GEN_DEV_MODE_LANGUAGE
    LanguagesDataPoint::GetInstance()->SetValue(DEV_LANGUAGE);
#else
    if (LanguagesDataPoint::GetInstance()->GetValue() == DEV_LANGUAGE)
    {
      LanguagesDataPoint::GetInstance()->SetValue(UK_LANGUAGE);
    }
#endif

    SetLanguage(LanguagesDataPoint::GetInstance()->GetValue());
  }
}


 /*****************************************************************************
  * Function - Constructor
  * DESCRIPTION:
  *
  *****************************************************************************/
 Languages::Languages()
 {
   mpStrings = NULL;
   mpPrevStrings = NULL;
   mLanguage = LAST_LANGUAGE;
   mpSubject = NULL;
   mUseExcel = false;

   SetSubjectPointer(0, LanguagesDataPoint::GetInstance());
   ConnectToSubjects();
 }

 /*****************************************************************************
  * Function - Destructor
  * DESCRIPTION:
  *
  ****************************************************************************/
 Languages::~Languages()
 {
   if(mpPrevStrings != NULL)
     delete mpPrevStrings;
   if(mpStrings != NULL)
     delete mpStrings;
 }

 /*****************************************************************************
  *
  *
  *              PRIVATE FUNCTIONS
  *
  *
  ****************************************************************************/
GUI_CONST_STORAGE GUI_FONT *language_dep_font_11 = &Helvetica_57_11;
GUI_CONST_STORAGE GUI_FONT *language_dep_font_13 = &Helvetica_57_13;
GUI_CONST_STORAGE GUI_FONT *language_dep_font_18 = &Helvetica_57_18;

GUI_CONST_STORAGE GUI_FONT *language_indep_font_11 = &Helvetica_57_11;
GUI_CONST_STORAGE GUI_FONT *language_indep_font_13 = &Helvetica_57_13;
GUI_CONST_STORAGE GUI_FONT *language_indep_font_18 = &Helvetica_57_18;

 /*****************************************************************************
  * Function - ChangeFont
  * DESCRIPTION:
  ****************************************************************************/
void Languages::ChangeFont(void)
{
  switch (mLanguage)
  {
    case CH_LANGUAGE :
      language_dep_font_11 = &GB2312_S1112;
      language_dep_font_13 = &GB2312_S1112;
      language_dep_font_18 = &GB2312_S1112;
      break;

    default :
      language_dep_font_11 = &Helvetica_57_11;
      language_dep_font_13 = &Helvetica_57_13;
      language_dep_font_18 = &Helvetica_57_18;
  }

  language_indep_font_11 = &Helvetica_57_11;
  language_indep_font_13 = &Helvetica_57_13;
  language_indep_font_18 = &Helvetica_57_18;
}

 /*****************************************************************************
  * Function - SubstituteCuNumberTags
  * DESCRIPTION: Substitutes all CU 361/362 with CU number (361 or 362)
  ****************************************************************************/
void Languages::SubstituteCuNumberTags(char* pText)
{
  // search for the CU number tag. 
  // NOTE: The tag length must be equal to length of the replacement string!
  char* pos = pText;
  do
  {
    pos = strstr(pos, "CU");
    if (pos != NULL)
    {
      pos += 2;
      if (*pos == ' ')
      {
        pos++;
      }
      
      char* tag361_pos = strstr(pos, "361");
      char* tag362_pos = strstr(pos, "362");
      if ((pos == tag361_pos && tag361_pos != NULL)
        || (pos == tag362_pos && tag362_pos != NULL))
      {
        pos = strncpy(pos, CU_NUMBER, 3);
      }
    }
  } while (pos != NULL);
}


 /*****************************************************************************
  * Function - SubstituteKeyNameTags
  * DESCRIPTION: Substitutes all [esc] and (esc) with the curved arrow symbol
  ****************************************************************************/
void Languages::SubstituteKeyNameTags(char* pText)
{
#ifdef TFT_16_BIT_LCD
  // search for the [esc] tag
  // NOTE: The tag length must be equal to length of the replacement string!
  char* pos = pText;
  do
  {
    char* sub_pos = strstr(pos, "[esc]");
    if (sub_pos != NULL)
    {
      pos = sub_pos;
    }

    if (sub_pos == NULL)
    {
      sub_pos = strstr(pos, "(esc)");
      pos = sub_pos;
    }

    if (pos != NULL && sub_pos != NULL)
    {
      // unicode 253C (= UTF8 E2.94.BC) is used for the curved arrow symbol
      pos[1] = 0xE2;
      pos[2] = 0x94;
      pos[3] = 0xBC;

      pos += 5;
    }
  } while (pos != NULL);
#endif
}
/*****************************************************************************
  *
  *
  *              PROTECTED FUNCTIONS
  *                 - RARE USED -
  *
  ****************************************************************************/

