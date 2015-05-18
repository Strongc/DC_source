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
/* CLASS NAME       : MinuteDataPoint, HourDataPoint, DayDataPoint,         */
/*                    MonthDataPoint, YearDataPoint, DateTimeChangeEvent    */
/*                                                                          */
/* FILE NAME        : TimeDataPoint.h                                       */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __TIME_DATA_POINT_H__
#define __TIME_DATA_POINT_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <DataPoint.h>
#include <Observer.h>
#include <I32DataPoint.h>
#include <EventDataPoint.h>

/*****************************************************************************
* CLASS:       MinuteDataPoint
* DESCRIPTION:
*
*****************************************************************************/
class MinuteDataPoint : public I32DataPoint, Observer
{
public:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  static MinuteDataPoint* GetInstance();
  virtual bool SetValue(I32);

  virtual void SubscribtionCancelled(Subject* pSubject){} //ignore (not needed)
  virtual void Update(Subject* pSubject);
  virtual void SetSubjectPointer(int Id,Subject* pSubject){} //ignore (not needed)
  virtual void ConnectToSubjects(void){} //ignore (not needed)

private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  /********************************************************************
                LIFECYCLE - Default constructor.
  ********************************************************************/
  MinuteDataPoint();
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  virtual ~MinuteDataPoint();
  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
  static MinuteDataPoint* mInstance;
};

/*****************************************************************************
* CLASS:       HourDataPoint
* DESCRIPTION:
*
*****************************************************************************/
class HourDataPoint : public I32DataPoint, Observer
{
public:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  static HourDataPoint* GetInstance();
  virtual bool SetValue(I32);

  virtual void SubscribtionCancelled(Subject* pSubject){} //ignore (not needed)
  virtual void Update(Subject* pSubject);
  virtual void SetSubjectPointer(int Id,Subject* pSubject){} //ignore (not needed)
  virtual void ConnectToSubjects(void){} //ignore (not needed)

private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  /********************************************************************
                LIFECYCLE - Default constructor.
  ********************************************************************/
  HourDataPoint();
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  virtual ~HourDataPoint();
  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
  static HourDataPoint* mInstance;
};

/*****************************************************************************
* CLASS:       DayDataPoint
* DESCRIPTION:
*
*****************************************************************************/
class DayDataPoint : public I32DataPoint, Observer
{
public:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  static DayDataPoint* GetInstance();
  virtual I32 GetValue(void);
  virtual bool SetValue(I32);

  virtual void SubscribtionCancelled(Subject* pSubject){} //ignore (not needed)
  virtual void Update(Subject* pSubject);
  virtual void SetSubjectPointer(int Id,Subject* pSubject){} //ignore (not needed)
  virtual void ConnectToSubjects(void){} //ignore (not needed)

private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  /********************************************************************
                LIFECYCLE - Default constructor.
  ********************************************************************/
  DayDataPoint();
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  virtual ~DayDataPoint();
  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
  static DayDataPoint* mInstance;
};

/*****************************************************************************
* CLASS:       MonthDataPoint
* DESCRIPTION:
*
*****************************************************************************/
class MonthDataPoint : public I32DataPoint, Observer
{
public:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  static MonthDataPoint* GetInstance();
  virtual bool SetValue(I32);

  virtual void SubscribtionCancelled(Subject* pSubject){} //ignore (not needed)
  virtual void Update(Subject* pSubject);
  virtual void SetSubjectPointer(int Id,Subject* pSubject){} //ignore (not needed)
  virtual void ConnectToSubjects(void){} //ignore (not needed)

private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  /********************************************************************
                LIFECYCLE - Default constructor.
  ********************************************************************/
  MonthDataPoint();
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  virtual ~MonthDataPoint();
  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
  static MonthDataPoint* mInstance;
};

/*****************************************************************************
* CLASS:       YearDataPoint
* DESCRIPTION:
*
*****************************************************************************/
class YearDataPoint : public I32DataPoint, Observer
{
public:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  static YearDataPoint* GetInstance();
  virtual bool SetValue(I32);

  virtual void SubscribtionCancelled(Subject* pSubject){} //ignore (not needed)
  virtual void Update(Subject* pSubject);
  virtual void SetSubjectPointer(int Id,Subject* pSubject){} //ignore (not needed)
  virtual void ConnectToSubjects(void){} //ignore (not needed)

private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  /********************************************************************
  LIFECYCLE - Default constructor.
  ********************************************************************/
  YearDataPoint();
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  virtual ~YearDataPoint();
  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
  static YearDataPoint* mInstance;
};


/*****************************************************************************
* CLASS:       DateTimeChangeEvent
* DESCRIPTION: Subscribe to this DateTimeChangeEvent to be notified when the current
* time and date is changed by set methods (not by clock).
*
*****************************************************************************/
class DateTimeChangeEvent : public EventDataPoint
{
public:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  static DateTimeChangeEvent* GetInstance();
private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  /********************************************************************
  LIFECYCLE - Default constructor.
  ********************************************************************/
  DateTimeChangeEvent(){};
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  virtual ~DateTimeChangeEvent(){};
  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
  static DateTimeChangeEvent* mInstance;
};

#endif
