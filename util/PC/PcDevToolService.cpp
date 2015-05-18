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
/* CLASS NAME       : PcDevToolService                                      */
/*                                                                          */
/* FILE NAME        : PcDevToolService.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 30-07-2008                                            */
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
#include <factory.h>
#include <AlarmConfig.h>
#include <DisplayController.h>
#include <Languages.h>

#include <AlarmDataPoint.h>
#include <StringDataPoint.h>
#include <FloatDataPoint.h>
#include <EventDataPoint.h>
#include <I32VectorDataPoint.h>
#include <U8VectorDataPoint.h>
#include <BoolVectorDataPoint.h>
#include <FloatVectorDataPoint.h>
#include <EnumDataPoint.h>
#include <U8DataPoint.h>
#include <Display.h>
#include <ListView.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "PcDevToolService.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#ifdef USE_TFT_COLOURS
#define NAME_OF_CONTROLLER_WINDOW TEXT("CU362") //should match name used in WinMain.c
#else
#define NAME_OF_CONTROLLER_WINDOW TEXT("CU361") //should match name used in WinMain.c
#endif

using namespace mpc::display;

PcDevToolService* PcDevToolService::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function: GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
PcDevToolService* PcDevToolService::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new PcDevToolService();
  }
  return mInstance;
}

/*****************************************************************************
 * Function - GetControllerWindow
 * DESCRIPTION:
 *
 *****************************************************************************/
HWND PcDevToolService::GetControllerWindow(void)
{
    if(FindWindow(_T("emWinDevice"), _T("CU351"))== NULL)
    {
        if(FindWindow(_T("emWinDevice"), _T("CU352"))== NULL)
        {
            if(FindWindow(_T("emWinDevice"), _T("CU361")) == NULL)
            {
                 if(FindWindow(_T("emWinDevice"), _T("CU362"))== NULL)
                 {
                      if(FindWindow(_T("emWinDevice"), _T("CU371"))== NULL)
                      {
                          if(FindWindow(_T("emWinDevice"), _T("CU372")) == NULL)
                          {
                              //return NULL;
                          }
                          else
                          {
                               return FindWindow(_T("emWinDevice"), _T("CU372"));
                          }
                      }
                      else
                      {
                           return FindWindow(_T("emWinDevice"), _T("CU371"));
                      }

                 }
                 else
                 {
                     return FindWindow(_T("emWinDevice"), _T("CU362"));
                 }

            }
            else
            {
                 return FindWindow(_T("emWinDevice"), _T("CU361"));
            }
        }
        else
        {
            return FindWindow(_T("emWinDevice"), _T("CU352"));
        }

    }
    else
    {
        return FindWindow(_T("emWinDevice"), _T("CU351"));
    }
   // return FindWindow(_T("emWinDevice"),NAME_OF_CONTROLLER_WINDOW);
  
}


/*****************************************************************************
 * Function - LoadDisplay
 * DESCRIPTION:
 *
 *****************************************************************************/
int PcDevToolService::LoadDisplay(const char* displayname)
{
  int i = 1;
  mpc::display::Display*  p_display = GetDisplay(i);
  while (i < MAX_DISPLAY_ID || p_display != NULL)
  {
    if (p_display != NULL && strcmp(p_display->GetDisplayNumber(), displayname)==0)
    {
      return LoadDisplay(i);
    }
    ++i;
    p_display = GetDisplay(i);
  }
  return 1;
}

/*****************************************************************************
* Function - LoadDisplay
* DESCRIPTION:
*
*****************************************************************************/
int PcDevToolService::LoadDisplay(const int displayid)
{
  mpc::display::Display*  p_display = GetDisplay(displayid);
  if (p_display != NULL)
  {
    mpc::display::DisplayController::GetInstance()->ResetTo(p_display);

    StringDataPoint* p_dp = dynamic_cast<StringDataPoint*>(GetSubject(SUBJECT_ID_GPRS_APN)); 

    if (p_dp != NULL && strcmp(p_dp->GetValue(), "ENTER_HELP_MODE") == 0)
    {
      p_dp->SetValue("");
      mpc::display::DisplayController::GetInstance()->KeyEvent(MPC_HELP_KEY);
    }
    else if (p_dp != NULL && strcmp(p_dp->GetValue(), "EXIT_HELP_MODE") == 0)
    {
      p_dp->SetValue("");
      mpc::display::DisplayController::GetInstance()->KeyEvent(MPC_ESC_KEY);
      mpc::display::DisplayController::GetInstance()->KeyEvent(MPC_HOME_KEY);
      mpc::display::DisplayController::GetInstance()->KeyEvent(MPC_ESC_KEY);
      mpc::display::DisplayController::GetInstance()->KeyEvent(MPC_HOME_KEY);
      mpc::display::DisplayController::GetInstance()->KeyEvent(MPC_ESC_KEY);
    }

    return 0;
  }
  else
  {
    printf("Unable to find displayid: %d", displayid);
    return 1;
  }

}

/*****************************************************************************
* Function - SetSubjectValue
* DESCRIPTION:
*
*****************************************************************************/
int PcDevToolService::SetSubjectValue(SubjectValueParameters* idValuePair)
{
  int id =  idValuePair->id;
  char* value = idValuePair->value;
    
  Subject* sub = GetSubject(id);

  FloatDataPoint* fdp = dynamic_cast<FloatDataPoint*>(sub); 
  if (fdp != NULL)
  {
    float f_value = (float) atof(value);
    fdp->SetValue( f_value );
    return 0;
  }
  else
  {
    StringDataPoint* sdp = dynamic_cast<StringDataPoint*>(sub); 
    if (sdp != NULL)
    {
      sdp->SetValue( value );
      return 0;
    }
    else
    {
      int i_value = atoi(value);

      IIntegerDataPoint* iint = dynamic_cast<IIntegerDataPoint*>(sub); 
      if (iint != NULL)
      {
        iint->SetAsInt(i_value);
        return 0;
      }
      else
      {
        AlarmDataPoint* adp = dynamic_cast<AlarmDataPoint*>(sub); 
        INumberDataPoint* inum = dynamic_cast<INumberDataPoint*>(sub); 
        EventDataPoint* edp = dynamic_cast<EventDataPoint*>(sub);
        if (edp != NULL)
        {
          edp->SetEvent();
          return 0;
        }
        else if (inum != NULL)
        {
          inum->SetAsInt(i_value); 
          return 0;
        }
        else if (adp != NULL)
        {
          if (i_value >= FIRST_ALARM_STATE && i_value <= LAST_ALARM_STATE )
            adp->SetErrorPresent((ALARM_STATE_TYPE) i_value);
          return 0;
        }
        else
        {
          I32VectorDataPoint* i32vec = dynamic_cast<I32VectorDataPoint*>(sub); 
          U8VectorDataPoint* u8vec = dynamic_cast<U8VectorDataPoint*>(sub); 
          BoolVectorDataPoint* boolvec = dynamic_cast<BoolVectorDataPoint*>(sub); 
          FloatVectorDataPoint* fvec = dynamic_cast<FloatVectorDataPoint*>(sub); 
          if (i32vec || u8vec || boolvec || fvec)
          {
            char* startpos = strstr(idValuePair->value, ";");

            int number_of_elements = 0;

            if (i32vec != NULL)
            {
              number_of_elements = i32vec->GetInitialSize();
            }
            else if (u8vec != NULL)
            {
              number_of_elements = u8vec->GetInitialSize();
            }
            else if (boolvec != NULL)
            {
              number_of_elements = boolvec->GetInitialSize();
            }
            else if (fvec != NULL)
            {
              number_of_elements = fvec->GetInitialSize();
            }

            if (startpos != NULL)
            {
              startpos++;
              int i = 0;
              while (i < number_of_elements)
              {
                char* endpos = strstr(startpos, ";");
                if (endpos == 0)
                {
                  break;
                }

                *endpos = 0;
                endpos++;

                if (fvec != NULL)
                {
                  float fval = atof( startpos );
                  fvec->SetValue(i, fval);
                }
                else
                {
                  int val = atoi( startpos );
                  
                  if (i32vec != NULL)
                  {
                    i32vec->SetValue(i, val);
                  }
                  else if (u8vec != NULL)
                  {
                    u8vec->SetValue(i, val);
                  }
                  else if (boolvec != NULL)
                  {
                    boolvec->SetValue(i, val ? true : false);
                  }
                }

                startpos = endpos;

                i++;
              }
            
            }
            
            return 0;
          }

        }
      }
    }
  }
  return 1;
}

/*****************************************************************************
* Function - GetSubjectValue
* DESCRIPTION:
*
*****************************************************************************/
int PcDevToolService::GetSubjectValue(SubjectValueParameters* idValuePair)
{
  int id =  idValuePair->id;

  Subject* sub = GetSubject(id);

  FloatDataPoint* fdp = dynamic_cast<FloatDataPoint*>(sub); 
  if (fdp != NULL)
  {
    idValuePair->fvalue = fdp->GetAsFloat();
    return 0;
  }
  else
  {
    StringDataPoint* sdp = dynamic_cast<StringDataPoint*>(sub); 
    if (sdp != NULL)
    {
      idValuePair->fvalue = -1;
      strcpy(idValuePair->value, sdp->GetValue());     
      return 0;
    }
    else
    {
      IIntegerDataPoint* iint = dynamic_cast<IIntegerDataPoint*>(sub); 
      if (iint != NULL)
      {
        idValuePair->fvalue = (float)iint->GetAsInt();        
        sprintf(idValuePair->value, "%i", iint->GetAsInt());
        return 0;
      }
      else
      {
        AlarmDataPoint* adp = dynamic_cast<AlarmDataPoint*>(sub); 
        INumberDataPoint* inum = dynamic_cast<INumberDataPoint*>(sub); 
        if (inum != NULL)
        {
          idValuePair->fvalue = inum->GetAsFloat();
          sprintf(idValuePair->value, "%f", inum->GetAsFloat());
          return 0;
        }
        else if (adp != NULL)
        {
          idValuePair->fvalue = (float) adp->GetErrorPresent();
          sprintf(idValuePair->value, "%i", adp->GetErrorPresent());
          return 0;
        }
        else
        {
          I32VectorDataPoint* i32vec = dynamic_cast<I32VectorDataPoint*>(sub); 
          U8VectorDataPoint* u8vec = dynamic_cast<U8VectorDataPoint*>(sub); 
          BoolVectorDataPoint* boolvec = dynamic_cast<BoolVectorDataPoint*>(sub); 
          FloatVectorDataPoint* fvec = dynamic_cast<FloatVectorDataPoint*>(sub); 
          if (i32vec != NULL)
          {
            strset(idValuePair->value, 0);
            int i = 0;
            while (i < i32vec->GetInitialSize())
            {
              sprintf(idValuePair->value, "%s;%i", idValuePair->value, i32vec->GetValue(i));
              i++;
            }
            int len = strlen(idValuePair->value);
            idValuePair->value[len] = ';';
            idValuePair->value[len + 1] = '\0';
            idValuePair->fvalue = -1;
            return 0;
          }
          else if (u8vec != NULL)
          {
            strset(idValuePair->value, 0);
            int i = 0;
            while (i < u8vec->GetInitialSize())
            {
              sprintf(idValuePair->value, "%s;%i", idValuePair->value, u8vec->GetValue(i));
              i++;
            }
            int len = strlen(idValuePair->value);
            idValuePair->value[len] = ';';
            idValuePair->value[len + 1] = '\0';
            idValuePair->fvalue = -1;
            return 0;
          }
          else if (boolvec != NULL)
          {
            strset(idValuePair->value, 0);
            int i = 0;
            while (i < boolvec->GetInitialSize())
            {
              sprintf(idValuePair->value, "%s;%i", idValuePair->value, (boolvec->GetValue(i)?1:0));
              i++;
            }
            int len = strlen(idValuePair->value);
            idValuePair->value[len] = ';';
            idValuePair->value[len + 1] = '\0';
            idValuePair->fvalue = -1;
            return 0;
          }
          else if (fvec != NULL)
          {
            strset(idValuePair->value, 0);
            int i = 0;
            while (i < fvec->GetInitialSize())
            {
              sprintf(idValuePair->value, "%s;%.0f", idValuePair->value, fvec->GetValue(i));
              i++;
            }
            int len = strlen(idValuePair->value);
            idValuePair->value[len] = ';';
            idValuePair->value[len + 1] = '\0';
            idValuePair->fvalue = -1;
            return 0;
          }

        }
      }
    }
  }
  return 1;
}

/*****************************************************************************
* Function - SetSubjectQuality
* DESCRIPTION:
*
*****************************************************************************/
int PcDevToolService::SetSubjectQuality(QualityParameters* idQualityPair)
{
  int id =  idQualityPair->id;
  int quality = idQualityPair->quality;
    
  Subject* sub = GetSubject(id);

  IDataPoint* dp = dynamic_cast<IDataPoint*>(sub); 
  if (dp != NULL && quality > -1 && quality < 3 )
  {
    dp->SetQuality( (DP_QUALITY_TYPE) quality);
    return 0;
  } 
  return 1;
}

/*****************************************************************************
* Function - SelectListViewItem
* DESCRIPTION:
*
*****************************************************************************/
int PcDevToolService::SelectListViewItem(int index)
{
  Display* p_display = DisplayController::GetInstance()->GetCurrentDisplay();

  if (p_display != NULL)
  {
    ListView* p_listview = dynamic_cast<ListView*>(p_display->GetRoot()->GetCurrentChild());

    if (p_listview != NULL)
    {
      if (index == 0)
      {
        p_listview->SelectFirstVisibleItem();
      }
      else
      {
        p_listview->SetSelection(index);
      }
      return 0;
    }
  }
  
  return 1;
}


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
PcDevToolService::PcDevToolService()
{

}