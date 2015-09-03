/**************************************************************************
 This file is auto-generated by FactoryGenerator, do not modify it manually
**************************************************************************/

/**************************************************************************
 Includes
**************************************************************************/
#include <GUI.h>
#include <TaskCtrl.h>
#include <TaskCtrlEvent.h>
#include <TaskCtrlPeriodic.h>
#include <FactoryTypes.h>
#include <Display.h>
#include "util\Subject.h"

/**************************************************************************
 Function proto types
**************************************************************************/
void RunFactory(void);
TaskCtrlPeriodic* GetControllerPeriodicTask(void);
TaskCtrlPeriodic* GetGeniAppTask(void);
TaskCtrlEvent* GetControllerEventsTask(void);
TaskCtrlPeriodic* GetLowPrioPeriodicTask(void);
TaskCtrlEvent* GetDisplayEventsTask(void);
mpc::display::Display* GetDisplay(int displayId);
Subject* GetSubject(int subjectId);
void FatalErrorOccured(const char* errorDescription /*= "-"*/ );
/**************************************************************************
 Externs
**************************************************************************/
extern const DbUnitStrings DISPLAY_UNIT_STRINGS[];
extern const int DISPLAY_UNIT_STRINGS_CNT;
extern const DbAlarmStrings DISPLAY_ALARM_STRINGS[];
extern const int DISPLAY_ALARM_STRINGS_CNT;
/**************************************************************************
 Display images
**************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmGoOn;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmGoOnSkip;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmGoBack;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmCompleted;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmLargePitBottom;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmLargePitLeft;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmLargePitRight;
#ifdef __PC__
extern std::vector<STRING_ID>  display_help_strings;
extern STRING_ID GetHelpString(int helpbox_index);
#endif

// observer-subject relationId for connecting alarm datapoints to alarm control
#define SP_AC_ALARM_DATA_POINT 4318
#define SP_EASC_ALARM_CONFIG 4319
