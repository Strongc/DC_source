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
/* CLASS NAME       : Clock                                                 */
/*                                                                          */
/* FILE NAME        : Clock.h                                               */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcClock_h
#define mpcClock_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
 *
 *
 * PC - VERSION
 *
 *
 * *****************************************************************************/
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#ifdef __PC__
#include <Observer.h>
#include <SwTimerBassClass.h>
#include <SubTask.h>
 /*****************************************************************************
  DEFINES
 *****************************************************************************/
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  SECONDS,
  MINUTES,
  HOURS,
  SECONDS_SINCE_MIDNIGHT
}TIME_TYPE;

typedef enum
{
  DAY_OF_WEEK,
  DAY,
  MONTH,
  YEAR,
  DAY_OF_YEAR
}DATE_TYPE;

/*****************************************************************************
  FORWARD DECLARATIONS
 *****************************************************************************/
class MpcTime;
class ActTime;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class Clock : public SwTimerBaseClass, public SubTask
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static Clock* GetInstance();
    void InitClock(MpcTime* pTimeObj);
    void RunSubTask(void);
    void InitSubTask();

  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    Clock();
    void UpdateTimeObj(MpcTime* pTimeObj);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~Clock();
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /************************************************************************
    * LOCAL PROTOTYPES
    * *********************************************************************/
    void TimeOutFunc(int TimeOutType);
    virtual void SubscribtionCancelled(Subject* pSubject) {};
    virtual void Update(Subject* pSubject);
    virtual void SetSubjectPointer(int Id,Subject* pSubject) {};
    virtual void ConnectToSubjects() {};

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static Clock* mInstance;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif

/*****************************************************************************
 *
 *
 *  TARGET - VERSION
 *
 *
 ******************************************************************************/
#ifndef __PC__
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <iic.h>
#include <Observer.h>
#include <SwTimerBassClass.h>
#include <SubTask.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  SECONDS,
  MINUTES,
  HOURS,
  SECONDS_SINCE_MIDNIGHT
}TIME_TYPE;

typedef enum
{
  DAY_OF_WEEK,
  DAY,
  MONTH,
  YEAR,
  DAY_OF_YEAR
}DATE_TYPE;

/*****************************************************************************
  FORWARD DECLARATIONS
 *****************************************************************************/
class MpcTime;
class ActTime;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class Clock : public SwTimerBaseClass, public SubTask
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static Clock* GetInstance();
    void InitClock(MpcTime* pTimeObj);
    bool WriteToRtc();
    void RunSubTask(void);
    void InitSubTask();
  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    Clock();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~Clock();
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool UpdateTimeObj(MpcTime* pTimeObj);
    void CalcNewTime();

    /************************************************************************
    * LOCAL PROTOTYPES
    * *********************************************************************/
    bool i2c_wait_trans();
    bool I2C_Tx2RTC(unsigned char slv_addr, unsigned char reg_addr, unsigned char no_data, unsigned char *dptr);
    bool I2C_RxFROM_RTC(unsigned char slv_addr, unsigned char reg_addr, unsigned char no_data, unsigned char *dptr);
    bool i2c_init_comm(unsigned char slv_addr, unsigned char reg_addr);
    void i2c_ctrl_init(void);
    bool i2c_rtc_init(void);

    bool IsActTimeValid();

    virtual void SubscribtionCancelled(Subject* pSubject) {};
    virtual void Update(Subject* pSubject);
    virtual void SetSubjectPointer(int Id,Subject* pSubject) {};
    virtual void ConnectToSubjects() {};
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static Clock* mInstance;
    ActTime* pActTimeObj;
    bool mClockOk;
    bool mRetryInitOfRtc;
    bool mWriteToRtcFlag;
    bool mRefreshSwClockFlag;
    bool mOperationInIIC;
    U16 mOldMilliSec;
    int mRefreshRtcCnt;

    protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif

#endif














