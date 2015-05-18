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
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/****************************************************************************/
/* CLASS NAME       : this is not a class                                   */
/*                                                                          */
/* FILE NAME        : DllExportedMethods.cpp                                */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : DLL exported functions for PC Display Viewer.   */
/*                                                                          */
/****************************************************************************/
#ifdef __PC__
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <crtdbg.h>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <WinMain/applicationprotocol.h>
#include <ConfigControl.h>
#include <IobComDrv.h>
#include <SmsCtrl.h>


/*****************************************************************************
LOCAL INCLUDES
*****************************************************************************/
#include <PcDevToolService.h>
#include <PcSimulatorService.h>
#include <PcMessageService.h>
#include <StringWidthCalculator.h>
#include <utf8_util_test.h>


/*****************************************************************************

All these functions sends a message using the SendOSMessage function (see
ApplicationProtocol.cpp). The message will be handled by the display_task.
SendOSMessage function dosn't return until display_task has handled the 
message. SendOSMessageAsync do.

*****************************************************************************/
extern "C" __declspec(dllexport) int __cdecl LoadDisplay(const char* displayName)
{
  return SendOSMessage(OSMSG_LOAD_DISPLAY, (void*)displayName);
}

extern "C" __declspec(dllexport) void __cdecl LoadDisplayId(const int displayId)
{
  SendOSMessage(OSMSG_LOAD_DISPLAY_ID, (void*)&displayId);
}

extern "C" __declspec(dllexport) void __cdecl SetLanguage(const int languageId)
{
  char lan[30];
  SubjectValueParameters* p_params = new SubjectValueParameters();
  
  itoa (languageId, lan, 10);

  p_params->id = 175;  
  strcpy(p_params->value, lan);
  
  SendOSMessage(OSMSG_SET_SUBJECT_VALUE, (void*)(p_params));
}

extern "C" __declspec(dllexport) int __cdecl GetLanguage()
{
  return SendOSMessage(OSMSG_GET_LANGUAGE, (void*)NULL);
}

extern "C" __declspec(dllexport) void __cdecl DumpCurrentDisplay()
{
  SendOSMessage(OSMSG_DUMP_SCREEN, (void*)NULL);
}

extern "C" __declspec(dllexport) void __cdecl DumpDeveloperDisplays()
{
  SendOSMessageAsync(OSMSG_DUMP_SCREENS, (void*)NULL);
}

extern "C" __declspec(dllexport) void __cdecl ExportStringLengths(const char* filename)
{
  SendOSMessage(OSMSG_EXPORT_STRING_LENGTHS, (void*)filename);
}

extern "C" __declspec(dllexport) void __cdecl ExportStringLengthsAdvanced(
                                                const char* filename, 
                                                const char* firstcolumn, 
                                                bool includeHeader,
                                                bool onlyRelationsInCurrentDisplay)
{
  mpc::display::StringWidthParameters* p_params = new mpc::display::StringWidthParameters();
  
  strncpy(p_params->filename, filename, MAX_PATH);
  strncpy(p_params->firstcolumn, firstcolumn, MAX_PATH);
  p_params->includeHeader = includeHeader;
  p_params->onlyRelationsInCurrentDisplay = onlyRelationsInCurrentDisplay;

  SendOSMessage(OSMSG_EXPORT_STRING_LENGTHS_ADV, (void*)(p_params));
}

extern "C" __declspec(dllexport) int __cdecl SetSubjectValue(int subjectId, const char* uft8EncodedValue)
{
  SubjectValueParameters* p_params = new SubjectValueParameters();
  p_params->id = subjectId;
  strcpy(p_params->value, uft8EncodedValue);
  
  return SendOSMessage(OSMSG_SET_SUBJECT_VALUE, (void*)(p_params));
}

extern "C" __declspec(dllexport) void __cdecl  SelectListViewItem(const int index)
{
  SendOSMessage(OSMSG_SELECT_LISTVIEW_ITEM_BY_INDEX, (void*)&index);
}

extern "C" __declspec(dllexport) void __cdecl  KeyPress(const int key)
{
  SendOSMessage(OSMSG_KEY_PRESS, (void*)&key);
}

extern "C" __declspec(dllexport) void __cdecl SimulateAlarm(int alarmDataPointId, bool alarmIsPresent, bool warningIsPresent)
{
  AlarmParameters* p_params = new AlarmParameters();  
  p_params->alarmDataPointId = alarmDataPointId;
  p_params->alarmActive = alarmIsPresent;
  p_params->warningActive = warningIsPresent;

  SendOSMessageAsync(OSMSG_SET_ERROR_PRESENT, (void*)(p_params));
}

extern "C" __declspec(dllexport) void __cdecl SetDiValue(int index, bool isHigh)
{
  DiParameters* p_params = new DiParameters();  
  p_params->index = index;
  p_params->isHigh = isHigh;

  SendOSMessageAsync(OSMSG_SET_DI_VALUE, (void*)(p_params));
}

extern "C" __declspec(dllexport) void __cdecl SetAiValueInPercent(int index, float valueInPercent)
{
  AiParameters* p_params = new AiParameters();  
  p_params->index = index;
  p_params->value = valueInPercent;

  SendOSMessageAsync(OSMSG_SET_AI_VALUE_PERCENT, (void*)(p_params));
}

extern "C" __declspec(dllexport) void __cdecl SetAiValueInInternalUnit(int index, float valueInSiUnit)
{
  AiParameters* p_params = new AiParameters();  
  p_params->index = index;
  p_params->value = valueInSiUnit;

  SendOSMessageAsync(OSMSG_SET_AI_VALUE_INTERNAL, (void*)(p_params));
}


extern "C" __declspec(dllexport) void __cdecl SendSmsToCu361(const char* pMessage, char* pFromNumber)
{
  PcMessageService::GetInstance()->SendSmsToCu361(pMessage, pFromNumber);
}

extern "C" __declspec(dllexport) float __cdecl GetSubjectValueAsFloat(int subjectId)
{
  SubjectValueParameters p_params;
  p_params.id = subjectId;
  p_params.fvalue = -999;
  p_params.value[299] = '\0';

  PcDevToolService::GetInstance()->GetSubjectValue(&p_params);

  return p_params.fvalue;
}

extern "C" __declspec(dllexport) BSTR __stdcall GetSubjectValueAsString(int subjectId)
{
  SubjectValueParameters p_params;
  p_params.id = subjectId;
  p_params.value[299] = '\0';

  PcDevToolService::GetInstance()->GetSubjectValue(&p_params);

  return SysAllocStringByteLen((const char*) p_params.value, sizeof(p_params.value));
}

extern "C" __declspec(dllexport) BSTR __stdcall GetMessageFromCu361(void)
{
  SmsOut* p_sms = PcMessageService::GetInstance()->GetSms();

  if (p_sms != NULL)
  {
    char str[512];

    sprintf(str, "SMS;%-15s;%s",
      p_sms->GetPrimaryNumber(),
      p_sms->GetSmsMessage());

    delete p_sms;

    return SysAllocStringByteLen(str, strlen(str));
  }
  else
  {
    const char* p_msg = PcMessageService::GetInstance()->GetMsg();
    if (p_msg != NULL)
    {
      return SysAllocStringByteLen(p_msg, strlen(p_msg));
    }
  }

  return NULL;
}


extern "C" __declspec(dllexport) void __cdecl StartSimulation(void)
{
  EventDataPoint* pEnableSimulation = (EventDataPoint*) GetSubject(SUBJECT_ID_IOB_SIM_ENABLE);
  pEnableSimulation->SetEvent();

  IobComDrv::GetInstance()->SetInputSimulationMode(0x073f);
}

extern "C" __declspec(dllexport) void __cdecl StopSimulation(void)
{
  EventDataPoint* pDisableSimulation = (EventDataPoint*) GetSubject(SUBJECT_ID_IOB_SIM_DISABLE);
  pDisableSimulation->SetEvent(); 
}

extern "C" __declspec(dllexport) BSTR __stdcall GetSimuValues()
{
  try
  {
    return PcSimulatorService::GetInstance()->GetSimulatorValues();
  }
  catch(...)
  {
    return NULL;
  }
}

extern "C" __declspec(dllexport) BSTR __stdcall GetAIValues()
{
  try
  {
    return PcSimulatorService::GetInstance()->GetAIValues();
  }
  catch(...)
  {
    return NULL;
  }
}

extern "C" __declspec(dllexport) BSTR __stdcall GetDIValues()
{
  try
  {
    return PcSimulatorService::GetInstance()->GetDIValues();
  }
  catch(...)
  {
    return NULL;
  }
}

extern "C" __declspec(dllexport) BSTR __stdcall GetDOValues()
{
  try
  {
    return PcSimulatorService::GetInstance()->GetDOValues();
  }
  catch(...)
  {
    return NULL;
  }
}

extern "C" __declspec(dllexport) bool __cdecl IsControllerCreated()
{
  HWND wnd_handle = PcDevToolService::GetInstance()->GetControllerWindow();
  return (wnd_handle != NULL);
}

extern "C" __declspec(dllexport) bool __cdecl IsControllerVisible()
{
  HWND wnd_handle = PcDevToolService::GetInstance()->GetControllerWindow();
  return IsWindowVisible(wnd_handle);
}

extern "C" __declspec(dllexport) bool __cdecl IsControllerInitialized()
{
  bool is_initialized = false;

  if (IsControllerCreated())
  {
    is_initialized = mpc::display::DisplayController::GetInstance()->GetCurrentDisplay() != NULL;
  }

  return is_initialized;
}

extern "C" __declspec(dllexport) void __cdecl HideController()
{
  HWND wnd_handle = PcDevToolService::GetInstance()->GetControllerWindow();
  ShowWindow(wnd_handle, SW_HIDE);
}

extern "C" __declspec(dllexport) void __cdecl ShowController()
{
  HWND wnd_handle = PcDevToolService::GetInstance()->GetControllerWindow();
  ShowWindow(wnd_handle, SW_SHOWNORMAL);
}

extern "C" __declspec(dllexport) int __cdecl SetSubjectQuality(int subjectId, int quality)
{
  QualityParameters* p_params = new QualityParameters();
  p_params->id = subjectId;
  p_params->quality = quality;
  int ret = SendOSMessage(OSMSG_SET_SUBJECT_QUALITY, (void*)(p_params));

  return ret;
}

extern "C" __declspec(dllexport) void __cdecl LoadFlashFilesFromFolder(const char* folder)
{
  ConfigControl::GetInstance()->AsyncCopyFlashFiles(folder, false);
}

extern "C" __declspec(dllexport) void __cdecl SaveFlashFilesToFolder(const char* folder)
{
  ConfigControl::GetInstance()->AsyncCopyFlashFiles(folder, true);
}



extern "C" __declspec(dllexport) int __cdecl SetIO111Values(
                                              int moduleNo,
                                              float temperature,
                                              float temperaturePt100,
                                              float temperaturePt1000,
                                              float temperatureMainBearings,
                                              float temperatureSupportBearings,
                                              float waterInOil,
                                              float resistance,
                                              float vibration,
                                              int   moisture,
                                              int   thermalSwitch)
{
  Io111Parameters* p_params = new Io111Parameters();
  p_params->moduleNo = moduleNo;
  p_params->temperature = temperature;
  p_params->temperaturePt100 = temperaturePt100;
  p_params->temperaturePt1000 = temperaturePt1000;
  p_params->temperatureMainBearing = temperatureMainBearings;
  p_params->temperatureSupportBearing = temperatureSupportBearings;
  p_params->waterInOil = waterInOil;
  p_params->resistance = resistance;
  p_params->vibration = vibration;
  p_params->moisture = moisture;
  p_params->thermalSwitch = thermalSwitch;

  int ret = SendOSMessage(OSMSG_SET_IO111_VALUES, (void*)(p_params));

  return ret;
}


extern "C" __declspec(dllexport) BSTR __stdcall GetIO111Values(int moduleNo)
{
  try
  {
    return PcSimulatorService::GetInstance()->GetIO111Values(moduleNo);
  }
  catch(...)
  {
    return NULL;
  }
}


extern "C" __declspec(dllexport) int __cdecl TestGSM388Conversion(const char* filename)
{
  return (int) Gsm338FileConvertion(filename);
}

#endif // __PC__

