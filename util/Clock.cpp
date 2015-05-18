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
/* FILE NAME        : Clock.cpp                                             */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

#ifdef __PC__
 #error "This file is only for target"
#endif

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <Clock.h>
#include <MpcTime.h>
#include <ActTime.h>
#include <SwTimer.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define REFRESH_TIME 43200 //Syncronice each 12 hours

// SW Timers
enum
{
  ONE_SECOND_TIMER
};

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  CREATS AN OBJECT.
 ******************************************************************************/
Clock* Clock::mInstance = 0;
const int century[]={20, 19};
/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
Clock* Clock::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new Clock();
  }
  return mInstance;
}


/*****************************************************************************
 * Function GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
bool Clock::UpdateTimeObj(MpcTime* pTimeObj)
{
  if (mOperationInIIC == false)
  {
    mOperationInIIC = true;

    /************************************************************************
     * READ C:YY:MM:WD:DD:HH:MM:SS AND VL BIT CONTENTS
     * *********************************************************************/
    // RTC register/Name  bits 7 ,......0   () indicates number of bits wide
    // 02h/seconds        :    VL(1),0-5(3),0-9(4)
    // 03h/minutes        :    0(1),0-5(3),0-9(4)
    // 04h/hours          :    0(2),0-2(2),0-9(4)
    // 05h/days           :    0(2),0-3(2),0-9(4)
    // 06h/weekdays       :    0(5),0-6(3)
    // 07h/months+century :    Cent(1),0(2),0-1(1),0-9(4)
    // 08h/years          :    0-9(4),0-9(4)
    bool err_flg;
    unsigned int temp1;
    unsigned int temp2;
    unsigned int temp3;
    unsigned char buf[7];
    unsigned int tempvalue;
    int real_year;

    //Reading information from RTC device.....\n\r");
    OS_DI();//sad but true
    err_flg = I2C_RxFROM_RTC(RTC_ADDR_PHIL, 0x02, 7, &buf[0]);
    OS_RestoreI();//sad but true

    if(err_flg != true)
    {
      mOperationInIIC = false;
      return false;
    }
    //RTC device information is loaded, see below:\n\r");
    temp1 = (unsigned int)(century[buf[5]>>7]);
    temp2 = (unsigned int)(buf[6]>>4);
    temp3 = (unsigned int)(buf[6]&0x0F);
    real_year = ((temp1*100)+(temp2*10)+temp3);
    pTimeObj->SetDate(YEAR, real_year);
    buf[5] &= 0x1f;                     // mask dont cares out, and century out
    switch ((((buf[5]&0x0F)+((buf[5]&0x10)>>1)+((buf[5]&0x10)>>3))-1))
    {
      case 0 :
        pTimeObj->SetDate(MONTH, 1);
        break;
      case 1 :
        pTimeObj->SetDate(MONTH, 2);
        break;
      case 2 :
        pTimeObj->SetDate(MONTH, 3);
        break;
      case 3 :
        pTimeObj->SetDate(MONTH, 4);
        break;
      case 4 :
        pTimeObj->SetDate(MONTH, 5);
        break;
      case 5 :
        pTimeObj->SetDate(MONTH, 6);
        break;
      case 6 :
        pTimeObj->SetDate(MONTH, 7);
        break;
      case 7 :
        pTimeObj->SetDate(MONTH, 8);
        break;
      case 8 :
        pTimeObj->SetDate(MONTH, 9);
        break;
      case 9 :
        pTimeObj->SetDate(MONTH, 10);
        break;
      case 10 :
        pTimeObj->SetDate(MONTH, 11);
        break;
      case 11 :
        pTimeObj->SetDate(MONTH, 12);
        break;
    }

    tempvalue = (buf[4] &= 0x07);                     // maske to use the first 3 bit.
            //    buf[4] &= 0x7F;                     // maske to use the first 7 bit.
    switch (tempvalue)
    {
      case 0 :
        pTimeObj->SetDate(DAY_OF_WEEK, 0);
        break;
      case 1 :
        pTimeObj->SetDate(DAY_OF_WEEK, 1);
        break;
      case 2 :
        pTimeObj->SetDate(DAY_OF_WEEK, 2);
        break;
      case 3 :
        pTimeObj->SetDate(DAY_OF_WEEK, 3);
        break;
      case 4 :
        pTimeObj->SetDate(DAY_OF_WEEK, 4);
        break;
      case 5 :
        pTimeObj->SetDate(DAY_OF_WEEK, 5);
        break;
      case 6 :
        pTimeObj->SetDate(DAY_OF_WEEK, 6);
        break;
    }

    // TYPE DAY
    //uart2_PutString("Day     : ");
    buf[3] &= 0x3F;                               // maske to use the first 6 bit.
    tempvalue = (buf[3] >>4);                     // maske to use the first 3 bit.
    tempvalue = (tempvalue * 10);
    tempvalue = tempvalue + ((buf[3] &= 0x0F));                     // maske to use the first 3 bit.
    pTimeObj->SetDate(DAY, tempvalue);

    // TYPE HOURS
    //uart2_PutString("Hours   : ");
    buf[2] &= 0x3F;                               // maske to use the first 6 bit.
    tempvalue = (buf[2] >>4);                     // maske to use the first 3 bit.
    tempvalue = (tempvalue * 10);
    tempvalue = tempvalue + ((buf[2] &= 0x0F));
    pTimeObj->SetTime(HOURS, tempvalue);

    // TYPE MINUTES
    // uart2_PutString("Minutes : ");
    buf[1] &= 0x7F;                               // maske to use the first 6 bit.
    tempvalue = (buf[1] >>4);                     // maske to use the first 3 bit.
    tempvalue = (tempvalue * 10);
    tempvalue = (tempvalue + (buf[1] &= 0x0F));                     // maske to use the first 3 bit.
    pTimeObj->SetTime(MINUTES, tempvalue);

    // TYPE SECONDS
    //  uart2_PutString("Seconds : ");
    buf[0] &= 0x7F;
    tempvalue = (buf[0] >>4);
    tempvalue = (tempvalue * 10);
    tempvalue = tempvalue + (buf[0] &= 0x0F);
    pTimeObj->SetTime(SECONDS, tempvalue);

//    if(buf[0]&0x80) uart2_PutString("Time and day is not guaranteed to be correct!\n\r");
//    else uart2_PutString("Time and day information is correct!\n\r");
//    return I2C_OK;
    mOperationInIIC = false;
    return true;
  }
  else
  {
    return false;
  }
}

/*****************************************************************************
 * Function - InitClock
 * DESCRIPTION:
 *
 *****************************************************************************/
void Clock::InitClock(MpcTime* pTimeObj)
{
  if( pTimeObj->IsTimeLegal())
  {
    mWriteToRtcFlag = true;
    ActTime::GetInstance()->Set(pTimeObj);
  }
}
/*****************************************************************************
 * Function - InitClock
 * DESCRIPTION:
 *
 *****************************************************************************/
bool Clock::WriteToRtc()
{
  MpcTime* pTimeObj;

  pTimeObj = ActTime::GetInstance();

  bool return_value = true;

  if (mOperationInIIC == false)
  {
    mOperationInIIC = true;
    if (mClockOk == false)
    {
      if (i2c_rtc_init() == true)  // RTC INIT
      {
        mClockOk = true;
      }
    }
    if (mClockOk == true)
    {
      //char err_flg;
      unsigned char buf[7];
      char tmp[10];
      int year_transformer;
                                                                    //Enter time(HH:MM:SS): ");
                                                                    //uart2_wait_for_string(&tmp[0]);
                                                                    //    buf[0]=((tmp[6]&0x0F)<<4)+(tmp[7]&0x0F);
                                                                    //    buf[1]=((tmp[3]&0x0F)<<4)+(tmp[4]&0x0F);
                                                                    //    buf[2]=((tmp[0]&0x0F)<<4)+(tmp[1]&0x0F);
      // SECONDS
      tmp[6] = (pTimeObj->GetTime(SECONDS)/10);
      tmp[7] = (pTimeObj->GetTime(SECONDS)%10);
      buf[0]=((tmp[6]&0x0F)<<4)+(tmp[7]&0x0F);
      // MINUTES
      tmp[3] = (pTimeObj->GetTime(MINUTES)/10);
      tmp[4] = (pTimeObj->GetTime(MINUTES)%10);
      buf[1]=((tmp[3]&0x0F)<<4)+(tmp[4]&0x0F);
      // HOURS
      tmp[0] = (pTimeObj->GetTime(HOURS)/10);
      tmp[1] = (pTimeObj->GetTime(HOURS)%10);
      buf[2]=((tmp[0]&0x0F)<<4)+(tmp[1]&0x0F);
      // DAY OF THE WEEK, (0=Sunday):
                                                                    //buf[4]=tmp[0]&0x0F;
      buf[4] = pTimeObj->GetDate(DAY_OF_WEEK);
                                                                    //    uart2_PutString("\r\nEnter date(YY:MM:DD): ");
                                                                    //    uart2_wait_for_string(&tmp[0]);
                                                                    //    buf[3]=((tmp[6]&0x0F)<<4)+(tmp[7]&0x0F);
                                                                    //    buf[5]=((tmp[3]&0x0F)<<4)+(tmp[4]&0x0F);
                                                                    //    buf[6]=((tmp[0]&0x0F)<<4)+(tmp[1]&0x0F);
                                                                    //esl_test = pTimeObj->GetDate(DAY);
                                                                    //buf[3] = pTimeObj->GetDate(DAY);
      // Date
      tmp[6] = (pTimeObj->GetDate(DAY)/10);
      tmp[7] = (pTimeObj->GetDate(DAY)%10);
      buf[3]=((tmp[6]&0x0F)<<4)+(tmp[7]&0x0F);

      // MONTH
      tmp[3] = (pTimeObj->GetDate(MONTH)/10);
      tmp[4] = (pTimeObj->GetDate(MONTH)%10);
      buf[5]=((tmp[3]&0x0F)<<4)+(tmp[4]&0x0F);

      // YEAR
      year_transformer = (pTimeObj->GetDate(YEAR));
      year_transformer = year_transformer - 2000;           // Always use years after 2000 and before 2100;

      tmp[0] = (year_transformer/10);
      tmp[1] = (year_transformer%10);
      buf[6]=((tmp[0]&0x0F)<<4)+(tmp[1]&0x0F);
                                                                //err_flg = I2C_Tx2RTC(RTC_ADDR_PHIL, 0x02, 7, &buf[0]);
      // SET THE VALUE TO THE RTC
      OS_DI();
      if (I2C_Tx2RTC(RTC_ADDR_PHIL, 0x02, 7, &buf[0]) == false)
      {
        return_value = false;
      }
      OS_RestoreI();
                                                                //    if(err_flg!=I2C_OK) while(1);
                                                                //    uart2_PutString("\n\rRTC is updated!\n\r");
                                                                //    return I2C_OK;
    }
    else
    {
      return_value = false;
    }
  mOperationInIIC = false;
  }
  return return_value;
}
/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 *****************************************************************************/
void Clock::Update(Subject* pSubject)
{
  ReqTaskTime();
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 *****************************************************************************/
void Clock::RunSubTask()
{
  //Update the current time by incrementing the seconds
  CalcNewTime();

  //Time has been set, write actual time to RTC
  if( mWriteToRtcFlag )
  {
    if( WriteToRtc() )
    {
      mWriteToRtcFlag = false;
    }
  }

  //Syncronize clock by reading time in RTC
  if(mRefreshSwClockFlag)
  {
    if (mClockOk == false)
    {
      i2c_ctrl_init(); // WILL ALAYS BE OK
      if (i2c_rtc_init() == true)  // RTC INIT
        mClockOk = true;
    }

    if (mClockOk == true)
    {
      if (UpdateTimeObj(ActTime::GetInstance()) == false)
      {
        mClockOk = false;
      }
      else
      {
        mRefreshSwClockFlag = false;
      }
    }
  }
}

/*****************************************************************************
 *
 *              PRIVATE FUNCTIONS
 *
 ****************************************************************************/
/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
Clock::Clock()
{
  mpTimerObjList[ONE_SECOND_TIMER] = 0;
  mOperationInIIC = false;
  mWriteToRtcFlag = false;
  mRefreshSwClockFlag = true;
  mRefreshRtcCnt = 0;
}

void Clock::InitSubTask()
{
  int retry_cnt;

  mOldMilliSec = OS_GetTime();

  i2c_ctrl_init(); // WILL ALAYS BE OK
  if (i2c_rtc_init() == true)  // RTC INIT
  {
    mClockOk = true;
  }
  else
  {
    mClockOk = false;
  }

  if ( mClockOk == true)
  {
    retry_cnt = 3;
    mClockOk = false; //Start gues
    do
    {
      if( UpdateTimeObj(ActTime::GetInstance()) == true ) // updates act time with the current read time.
      {
        retry_cnt = 0;
        mClockOk = true;
      }
    } while ( retry_cnt-- );
    if ( IsActTimeValid() == false)
    {
      mClockOk = false;
    }
  }

  if ( mClockOk == false)
  {
    MpcTime* time_obj = new MpcTime(false);

    time_obj->SetTime(SECONDS, 00);
    time_obj->SetTime(MINUTES, 0);
    time_obj->SetTime(HOURS, 00);

    time_obj->SetDate(DAY_OF_WEEK, 1);
    time_obj->SetDate(DAY, 1);
    time_obj->SetDate(MONTH, 1);
    time_obj->SetDate(YEAR, 2006);
    ActTime::GetInstance()->Set(time_obj);
    delete time_obj;
  }
  if ( mpTimerObjList[ONE_SECOND_TIMER] == 0)
  {
    mpTimerObjList[ONE_SECOND_TIMER] = new SwTimer(1, S, true, true, this);
  }
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
Clock::~Clock()
{

}

/*****************************************************************************
* Function - CalcNewTime
* DESCRIPTION:
*
****************************************************************************/
void Clock::CalcNewTime()
{
  ActTime::GetInstance()->IncSec();

  if(++mRefreshRtcCnt>REFRESH_TIME && ActTime::GetInstance()->GetTime(SECONDS)==30)
  {
    mRefreshRtcCnt = 0;
    mRefreshSwClockFlag = true;
  }
}

/*****************************************************************************
 * Function - IsCurrentTimeValid
 * DESCRIPTION:
 *
 ****************************************************************************/
bool Clock::IsActTimeValid()
{
  bool return_value = true;
  /******************************************************************
  Check seconds
  ******************************************************************/
  if ( ActTime::GetInstance()->GetTime(SECONDS) < 0)
  {
    return_value = false;
  }
  else if ( ActTime::GetInstance()->GetTime(SECONDS) >= 60)
  {
    return_value = false;
  }
  /******************************************************************
  Check Minutes
  ******************************************************************/
  else if ( ActTime::GetInstance()->GetTime(MINUTES) < 0)
  {
    return_value = false;
  }
  else if ( ActTime::GetInstance()->GetTime(MINUTES) >= 60)
  {
    return_value = false;
  }
  /******************************************************************
  Check Hours
  ******************************************************************/
  else if ( ActTime::GetInstance()->GetTime(HOURS) < 0)
  {
    return_value = false;
  }
  else if ( ActTime::GetInstance()->GetTime(HOURS) >= 24)
  {
    return_value = false;
  }
  /******************************************************************
  Check Year
  ******************************************************************/
  else if ( ActTime::GetInstance()->GetDate(YEAR) < 2000)
  {
    return_value = false;
  }
  else if ( ActTime::GetInstance()->GetDate(YEAR) >= 2100)
  {
    return_value = false;
  }
  /******************************************************************
  Check Month
  ******************************************************************/
  else if ( ActTime::GetInstance()->GetDate(MONTH) < 1)
  {
    return_value = false;
  }
  else if ( ActTime::GetInstance()->GetDate(MONTH) > 12)
  {
    return_value = false;
  }
  /******************************************************************
  Check Date
  ******************************************************************/
  else if ( ActTime::GetInstance()->GetDate(DAY) < 1)
  {
    return_value = false;
  }
  else if ( ActTime::GetInstance()->GetDate(DAY) > 31)
  {
    return_value = false;
  }
  return return_value;
}
/*****************************************************************************
 * Function - i2c_wait_trans
 * DESCRIPTION:
 *
 ****************************************************************************/
bool Clock::i2c_wait_trans()
{
    bool return_value = true;
    int i = 0;

    while((VR4181A_SYSINT1REG & I2C0INTR) != 0)
    {
      if (i > 100)
      {
        return_value = false;
        break;
      }
      else
      {
        i++;
      }
    }
    i = 0;
    while((VR4181A_SYSINT1REG & I2C0INTR) == 0)
    {
      if (i > 800)
      {
        return_value = false;
        break;
      }
      else
      {
        i++;
      }
    }

    return return_value;
}
/*****************************************************************************
 * Function - I2C_Tx2RTC
 * DESCRIPTION:
 *
 ****************************************************************************/
bool Clock::I2C_Tx2RTC(unsigned char slv_addr, unsigned char reg_addr, unsigned char no_data, unsigned char *dptr)
{
    bool return_value = true;
    char j;

    /* start I2C process */
    VR4181A_I2CC0 = I2C_I2CE | I2C_STT | I2C_WTIM | I2C_SPIE;//send start condition
    if(i2c_init_comm(slv_addr,reg_addr) != true)
    {
     return_value = false;
     // return err_flag;
    }
    if ( return_value == true)
    {
    /* start to write data, addresses are incremented automatically */
      for(j=0; j<no_data; j++)
      {
        VR4181A_I2C0 = *dptr++; //  Data for register
        if (i2c_wait_trans() == false)
        {
          return_value = false;
        }
        if((VR4181A_I2CS0 & I2C_ACKD) == 0)
        {
            VR4181A_I2CC0 |= I2C_SPT;
          return_value = false;
        }
      }
    }
    if ( return_value == true)
    {
      VR4181A_I2CC0 |= I2C_SPT;
    }
    return return_value;
}
/*****************************************************************************
 * Function - I2C_RxFROM_RTC
 * DESCRIPTION:
 *
 ****************************************************************************/
bool Clock::I2C_RxFROM_RTC(unsigned char slv_addr, unsigned char reg_addr, unsigned char no_data, unsigned char *dptr)
{
    char j;
    bool err_flag;
    bool return_value = true;
    unsigned char temp[10];
    /* start I2C process */
    VR4181A_I2CC0 = I2C_I2CE | I2C_STT | I2C_WTIM | I2C_SPIE;//send start condition
    err_flag=i2c_init_comm(slv_addr,reg_addr);
    if(err_flag != true)
            {
         return_value = false;
            }
    //    VR4181A_I2CC0 |= I2C_SPT;
    /* restart I2C process */
    if ( return_value == true)
    {
        VR4181A_I2CC0 = I2C_I2CE | I2C_STT | I2C_WTIM;//send new start condition
        VR4181A_I2C0 = slv_addr | 0x01; //  register address
        if (i2c_wait_trans() != true)
        {
          return_value = false;
        }
    }
    if ( return_value == true)
    {
        if((VR4181A_I2CS0 & I2C_ACKD) == 0)
        {
          VR4181A_I2CC0 |= I2C_SPT;
          return_value = false;
        }
    }
    if ( return_value == true)
    {
        for(j = 0; j < 40; j++);  //if no wait, TRC is not changed
        if((VR4181A_I2CS0 & I2C_TRC) != 0)
        {
          VR4181A_I2CC0 |= I2C_SPT;
          return_value = false;
          //return I2C_NOT_RX_MODE_ERR;
        }
    }
    if ( return_value == true)
    {
        /* start to read the data */
        VR4181A_I2CC0 = (VR4181A_I2CC0 | I2C_ACKE) & ~I2C_WTIM;  //8bits stop and acknowledge enable

        for(j=0; j<no_data; j++)
        {
            VR4181A_I2CC0 |= I2C_WREL;
            if (i2c_wait_trans() != true)
            {
              return_value = false;
            }
            temp[j] = VR4181A_I2C0;
            *dptr++= temp[j];
        }
        VR4181A_I2CC0 &= ~I2C_ACKE;
        VR4181A_I2CC0 |= I2C_SPT;
    }
    //return I2C_OK;
    return return_value;
}
/*****************************************************************************
 * Function - I2C_RxFROM_RTC
 * DESCRIPTION:
 *
 ****************************************************************************/
bool Clock::i2c_init_comm(unsigned char slv_addr, unsigned char reg_addr)
{
    int j;
    bool return_value = true;

    int i = 0; // temp

    for(j = 0; j < 10000; j++)
    {
    i++;
    }//software wait
    /* For the first execution, stop condition or start condition is needed
     * to make CH0/CH1 operate, so the below if sentense is inserted.
     */
    if((VR4181A_I2CS0 & I2C_MSTS) == 0)
    {
      VR4181A_I2CC0 |= I2C_SPT;//send stop condition
      i = 0;
      while((VR4181A_SYSINT1REG & I2C0INTR) == 0)
      {
        if ( i == 500000)
        {
          return_value = false;
          break;
        }
        else
        {
          i++;
        }
      }
    }
    if ( return_value == true)
    {
      i = 0;
      while((VR4181A_I2CC0 & I2C_ALD) == I2C_ALD) //check arbitration win/lose
      {
        if ( i == 500000)
        {
          return_value = false;
          break;
        }
        else
        {
          i++;
        }
      }
    }
    /* start device select */
    VR4181A_I2C0 = slv_addr;  //Write slave address for following writes
    if ( return_value == true)
    {
      i = 0;
      while(((VR4181A_SYSINT1REG & I2C0INTR) == 0) || ((VR4181A_I2CS0 & I2C_SPD) != 0))
      {
        if ( i == 500000)
        {
          return_value = false;
          break;
        }
        else
        {
          i++;
        }
      }
    }
    if ( return_value == true)
    {
      if((VR4181A_I2CS0 & I2C_ACKD) == 0)
      {
        VR4181A_I2CC0 |= I2C_SPT;
//      puts("ACK ERROR1!\n");  -> IT IS A MAXIM DEVICE THAT IS MOUNTED!!!!!
     //   return I2C_ACK_ERR;
        return_value = false;
      }
    }
    if ( return_value == true)
    {
        if((VR4181A_I2CS0 & I2C_TRC) == 0)
        {
    //      puts("state of receive!\n");  -> VI SKULLE HELST VÆRE I TRANSMIT MODE!!!!!
          //return I2C_NOT_TX_MODE_ERR;
        return_value = false;
        }
    }
    /* send the init data to the RTC, first setup start address */
    if ( return_value == true)
    {
        VR4181A_I2C0 = reg_addr; //  register address
        if (i2c_wait_trans() == false)
        {
          return_value = false;
        }
    }
    if ( return_value == true)
    {
      if((VR4181A_I2CS0 & I2C_ACKD) == 0)
      {
        VR4181A_I2CC0 |= I2C_SPT;
      //  return I2C_ACK_ERR;
        return_value = false;
      }
    //return I2C_OK;
    }
      return return_value;
}

/*****************************************************************************
 * Function - i2c_ctrl_init
 * DESCRIPTION:
 *
 ****************************************************************************/
void Clock::i2c_ctrl_init(void)
{
 if ( mOperationInIIC == false)
 {
    mOperationInIIC = true;
    VR4181A_PINMODED1 &= 0xfff9;  //Setup GPIO11/12 pins for IIC control
    VR4181A_GPMODE1 &= 0xfebf;    //Setup GPIO11/12 for IIC control
    VR4181A_I2CCL0 = 0x02;
    VR4181A_I2CSVA0 = 0x00;
    mOperationInIIC = false;
  }
}
/*****************************************************************************
 * Function - i2c_rtc_init
 * DESCRIPTION:
 *
 ****************************************************************************/
bool Clock::i2c_rtc_init(void)
{
  if ( mOperationInIIC == false)
  {
    OS_DI();
    mOperationInIIC = true;

    bool return_value = true;
    int i;
    bool err_flg;
    unsigned char buf[5]={0x00, 0x11, 0x03, 0x82, 0x01};
    for ( i = 0; i< 100000; i++)
    {
      i ++;
      i--;
    }
//    uart2_PutString("\n\rInitializing RTC device\n\r");
    if(I2C_Tx2RTC(RTC_ADDR_PHIL, 0x00, 2, &buf[0]) != true)
    {
      return_value = false;
    }
    for ( i = 0; i< 100000; i++)
    {
      i ++;
      i--;
    }
//    if(err_flg!=I2C_OK) while(1);
    if (return_value == true) // NO ERROR YET
    {
      err_flg = I2C_Tx2RTC(RTC_ADDR_PHIL, 0x0D, 3, &buf[2]);
      for ( i = 0; i< 100000; i++)
      {
        i ++;
        i--;
      }
    }
    if(err_flg != true)
    {
      return_value = false;
    }
    for ( i = 0; i< 100000; i++)
    {
      i ++;
      i--;
    }
    mOperationInIIC = false;
    OS_RestoreI();
    return return_value;
//    uart2_PutString("RTC initialized with success\n\r");
//    return I2C_OK;
  }
  else
  {
    return false;
  }
}
