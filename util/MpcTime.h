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
/* CLASS NAME       : MpcTime                                               */
/*                                                                          */
/* FILE NAME        : MpcTime.h                                             */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef MpcTime_h
#define MpcTime_h

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
#include <clock.h>
#include <Subject.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define INVALID_DATE  -1
#define INVALID_TIME  -1
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class MpcTime : public Subject
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    MpcTime(U32 secondsSince1Jan1970);
    MpcTime(bool currentTime = true);
    MpcTime(const MpcTime& src);

    MpcTime& operator=(const MpcTime& src);
    bool operator==(const MpcTime& src);
    bool operator!=(const MpcTime& src);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~MpcTime();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual bool IsValid(void);
    virtual bool IsDateValid(void);
    virtual bool IsTimeValid(void);

    void SetDateInValid(void);
    void SetTimeInValid(void);

    virtual void SetTime(TIME_TYPE type, int time);
    virtual void SetDate(DATE_TYPE type, int value);
    virtual void Set(MpcTime* tempObj);


    virtual int GetTime(TIME_TYPE type);
    virtual int GetDate(DATE_TYPE type);

    virtual U32  GetSecondsSince1Jan1970(void);
    virtual void SetSecondsSince1Jan1970(U32 secondsSince1Jan1970);

    bool IsTimeLegal();

    protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    int LastDayInMonth(int month, int year);
    void CalcSecondsSince1Jan1970();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    int mHours;
    int mMinutes;
    int mSecunds;

    int mDay;
    int mMonth;
    int mYear;
    int mDayOfWeek;
    int mDayOfYear;
    int mSecSince1970;

    private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    int CalcWeekday();
    bool LeapYear(int year);
    int DoomsdayMonth(int year, int month);
    int DoomsdayCentury(int century);
    void CalcDayOfYear(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
