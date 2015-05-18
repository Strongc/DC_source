/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MidRagne                                      */
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
/* CLASS NAME       : FloatSwitchCtrlTest                                   */
/*                                                                          */
/* FILE NAME        : FloatSwitchCtrlTest.cpp                               */
/*                                                                          */
/* CREATED DATE     : 07-08-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

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
#include <FloatSwitchCtrlTest.h>
#include <AlarmDef.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define AFSW_CFG(_csw9_,_csw8_,_csw7_,_csw6_,_csw5_,_csw4_,_csw3_,_csw2_,_csw1_) \
FSW_##_csw9_,FSW_##_csw8_,FSW_##_csw7_,FSW_##_csw6_,FSW_##_csw5_,FSW_##_csw4_,FSW_##_csw3_,FSW_##_csw2_,FSW_##_csw1_

#define FSW5_CFG(_csw5_,_csw4_,_csw3_,_csw2_,_csw1_) \
  FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_NOT_DEFINED,\
  FSW_##_csw5_,FSW_##_csw4_,FSW_##_csw3_,FSW_##_csw2_,FSW_##_csw1_

#define FSW4_CFG(_csw4_,_csw3_,_csw2_,_csw1_) \
  FSW_NOT_DEFINED,FSW_NOT_DEFINED, \
  FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_##_csw4_,FSW_##_csw3_,FSW_##_csw2_,FSW_##_csw1_

#define FSW3_CFG(_csw3_,_csw2_,_csw1_) \
  FSW_NOT_DEFINED,FSW_NOT_DEFINED, \
  FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_##_csw3_,FSW_##_csw2_,FSW_##_csw1_

#define FSW2_CFG(_csw2_,_csw1_) \
  FSW_NOT_DEFINED,FSW_NOT_DEFINED, \
  FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_NOT_DEFINED,FSW_##_csw2_,FSW_##_csw1_



#define CFG1_AFSW2P AFSW_CFG(HIGH_WATER_A,ALARM,START2,START1,STOP2,STOP1,DRY_RUN_A,NOT_DEFINED,NOT_DEFINED)
#define CFG2_AFSW2P AFSW_CFG(HIGH_WATER_A,ALARM,START2,START1,STOP2,STOP1,DRY_RUN_A,NOT_DEFINED,HIGH_WATER)
#define CFG3_AFSW2P AFSW_CFG(HIGH_WATER_A,ALARM,START2,START1,STOP2,STOP1,DRY_RUN_A,NOT_DEFINED,DRY_RUN)
#define CFG4_AFSW2P AFSW_CFG(HIGH_WATER_A,ALARM,START2,START1,STOP2,STOP1,DRY_RUN_A,HIGH_WATER,DRY_RUN)


#define CFG1_5FSW2P FSW5_CFG(HIGH_WATER, START2, START1, STOP, DRY_RUN)
#define CFG2_5FSW2P FSW5_CFG(HIGH_WATER, START2, ALARM, START1_STOP, DRY_RUN)
#define CFG3_5FSW2P FSW5_CFG(HIGH_WATER, START2, ALARM, START1, STOP)
#define CFG4_5FSW2P FSW5_CFG(START2, ALARM, START1, STOP, DRY_RUN)
#define CFG5_5FSW2P FSW5_CFG(START2, START1, STOP2, STOP1,  DRY_RUN)
#define CFG6_5FSW2P FSW5_CFG(HIGH_WATER, START2, START1, STOP2, STOP1)
#define CFG7_5FSW2P FSW5_CFG(START2, ALARM, START1, STOP2, STOP1)
#define CFG8_5FSW2P FSW5_CFG(START2, START1, STOP1, STOP2, DRY_RUN)
#define CFG9_5FSW2P FSW5_CFG(START2, STOP2, START1, STOP1, DRY_RUN)
#define CFG10_5FSW2P FSW5_CFG(HIGH_WATER, START2, START1, STOP1, STOP2)
#define CFG11_5FSW2P FSW5_CFG(START2, ALARM, START1, STOP1, STOP2)
#define CFG12_5FSW2P FSW5_CFG(HIGH_WATER, START2, STOP2, START1, STOP1)
#define CFG13_5FSW2P FSW5_CFG(START2, ALARM, STOP2, START1, STOP1)

#define CFG1_4FSW2P FSW4_CFG(HIGH_WATER, START2, START1_STOP, DRY_RUN)
#define CFG2_4FSW2P FSW4_CFG(HIGH_WATER, START2, START1, STOP)
#define CFG3_4FSW2P FSW4_CFG(START2, ALARM, START1, STOP)
#define CFG4_4FSW2P FSW4_CFG(START2, ALARM, START1_STOP, DRY_RUN)
#define CFG5_4FSW2P FSW4_CFG(START2, START1, STOP,  DRY_RUN)
#define CFG6_4FSW2P FSW4_CFG(START2, START1, STOP2, STOP1)
#define CFG7_4FSW2P FSW4_CFG(START2, START1, STOP1, STOP2)
#define CFG8_4FSW2P FSW4_CFG(START2, STOP2, START1_STOP, DRY_RUN)

#define CFG1_4FSW1P FSW4_CFG(HIGH_WATER, START1, STOP, DRY_RUN)

#define CFG1_3FSW2P FSW3_CFG(HIGH_WATER, START2, START1_STOP)
#define CFG2_3FSW2P FSW3_CFG(START2, START1_STOP, DRY_RUN)
#define CFG3_3FSW2P FSW3_CFG(START2, ALARM, START1_STOP)
#define CFG4_3FSW2P FSW3_CFG(START2, START1, STOP)

#define CFG1_3FSW1P FSW3_CFG(HIGH_WATER, START1, STOP)
#define CFG2_3FSW1P FSW3_CFG(HIGH_WATER, START1_STOP, DRY_RUN)
#define CFG3_3FSW1P FSW3_CFG(START1, STOP, DRY_RUN)

#define CFG1_2FSW1P FSW2_CFG(START1, STOP)
#define CFG2_2FSW1P FSW2_CFG(HIGH_WATER, START1_STOP)
#define CFG3_2FSW1P FSW2_CFG(START1_STOP, DRY_RUN)

#define FSWVA 1
#define FSWVN 0
#define FSWN DIGITAL_INPUT_FUNC_STATE_NOT_ACTIVE
#define FSWA DIGITAL_INPUT_FUNC_STATE_ACTIVE

#define A_CFG(_hwl_,_al_,_sta2l_,_sta1l_,sto2l_,sto1l_,_drl_) _hwl_,_al_,_sta2l_,_sta1l_,sto2l_,sto1l_,_drl_
#define FSW_A(_sw2_,_sw1_,_plpct_) FSWN,FSWN,FSWN,FSWN,FSWN,FSWN,FSWN,FSW##_sw2_,FSW##_sw1_,_plpct_,_plpct_

#define FSW_5(_sw5_,_sw4_,_sw3_,_sw2_,_sw1_) FSWN,FSWN,FSWN,FSWN,FSW##_sw5_,FSW##_sw4_,FSW##_sw3_,FSW##_sw2_,FSW##_sw1_
#define FSW5_ADD(_sw5_,_sw4_,_sw3_,_sw2_,_sw1_) FSWV##_sw5_+FSWV##_sw4_+FSWV##_sw3_+FSWV##_sw2_+FSWV##_sw1_
#define FSW_4(_sw4_,_sw3_,_sw2_,_sw1_) FSWN,FSWN,FSWN,FSWN,FSWN,FSW##_sw4_,FSW##_sw3_,FSW##_sw2_,FSW##_sw1_
#define FSW4_ADD(_sw4_,_sw3_,_sw2_,_sw1_) FSWV##_sw4_+FSWV##_sw3_+FSWV##_sw2_+FSWV##_sw1_
#define FSW_3(_sw3_,_sw2_,_sw1_) FSWN,FSWN,FSWN,FSWN,FSWN,FSWN,FSW##_sw3_,FSW##_sw2_,FSW##_sw1_
#define FSW3_ADD(_sw3_,_sw2_,_sw1_) FSWV##_sw3_+FSWV##_sw2_+FSWV##_sw1_
#define FSW_2(_sw2_,_sw1_) FSWN,FSWN,FSWN,FSWN,FSWN,FSWN,FSWN,FSW##_sw2_,FSW##_sw1_
#define FSW2_ADD(_sw2_,_sw1_) FSWV##_sw2_+FSWV##_sw1_

#define FSW5E(_sw5_,_sw4_,_sw3_,_sw2_,_sw1_) FSW5_ADD(_sw5_,_sw4_,_sw3_,_sw2_,_sw1_),            \
  100.0*(FSW5_ADD(_sw5_,_sw4_,_sw3_,_sw2_,_sw1_))/5.0
#define FSW4E(_sw4_,_sw3_,_sw2_,_sw1_) FSW4_ADD(_sw4_,_sw3_,_sw2_,_sw1_),            \
  100.0*(FSW4_ADD(_sw4_,_sw3_,_sw2_,_sw1_))/4.0
#define FSW3E(_sw3_,_sw2_,_sw1_) FSW3_ADD(_sw3_,_sw2_,_sw1_),            \
  100.0*(FSW3_ADD(_sw3_,_sw2_,_sw1_))/3.0
#define FSW2E(_sw2_,_sw1_) FSW2_ADD(_sw2_,_sw1_),            \
  100.0*(FSW2_ADD(_sw2_,_sw1_))/2.0

#define FSW5(_sw5_,_sw4_,_sw3_,_sw2_,_sw1_) FSW_5(_sw5_,_sw4_,_sw3_,_sw2_,_sw1_),FSW5E(_sw5_,_sw4_,_sw3_,_sw2_,_sw1_)
#define FSW4(_sw4_,_sw3_,_sw2_,_sw1_) FSW_4(_sw4_,_sw3_,_sw2_,_sw1_),FSW4E(_sw4_,_sw3_,_sw2_,_sw1_)
#define FSW3(_sw3_,_sw2_,_sw1_) FSW_3(_sw3_,_sw2_,_sw1_),FSW3E(_sw3_,_sw2_,_sw1_)
#define FSW2(_sw2_,_sw1_) FSW_2(_sw2_,_sw1_),FSW2E(_sw2_,_sw1_)

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/


typedef struct
{
  void *vt;
  GF_UINT32 nof_tv;
  GF_UINT8 format;
} TVS;


/*****************************************************************************
  LOCAL CONST VARIABLES
 *****************************************************************************/
#define A_D ALARM_ID_DRY_RUNNING
#define A_I ALARM_ID_CONFLICTING_LEVELS
#define A_H ALARM_ID_HIGH_LEVEL
#define A_A ALARM_ID_ALARM_LEVEL


#include "_tv.c"
#include "_errortv.c"
#include "_analogtv.c"

#define NOF_TV(_tv_) (sizeof(_tv_)/sizeof(_tv_[0]))

#if 0
const TVS tvs[] =
{
  (void *)tv, NOF_TV(tv), 0,
  (void *)errortv, NOF_TV(errortv), 0,
  (void *)analogtv, NOF_TV(analogtv), 1,
  0,0,0
};
#endif

/*****************************************************************************
  C functions declarations
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
static GF_UINT32 Perr=0;
static GF_UINT32 Ierr=0;
static GF_UINT32 Herr=0;
static GF_UINT32 Derr=0;
static GF_UINT32 Aerr=0;
static GF_UINT32 Uerr=0;
static GF_UINT32 Lerr=0;
static GF_UINT32 LPerr=0;
static GF_UINT32 ALLerr=0;


void FloatSwitchCtrlTest::CheckResults(void *tv, GF_UINT16 i, GF_UINT8 format)
{
  auto FSW_TV *ttv;
  auto ANALOG_FSW_TV *attv;
  auto GF_UINT8 v;

  if (format == 0)
  {
    ttv = (FSW_TV *)tv;
    ttv += i;
  }
  else
  {
    attv = (ANALOG_FSW_TV *)tv;
    attv += i;
    ttv = &attv->fsw_tv;
  }

  v = (GF_UINT8)(mpPumpRef->GetValue());
  if (v != ttv->r)
  {
    Perr++;
  }
  v = (GF_UINT8)mpFloatSwitchInconsistencyFaultObj->GetValue();
  if (v != ttv->i)
  {
    Ierr++;
  }
  v = (GF_UINT8)mpFloatSwitchHighLevelFaultObj->GetValue();
  if (v != ttv->h)
  {
    Herr++;
  }
  v = (GF_UINT8)mpFloatSwitchDryRunFaultObj->GetValue();
  if (v != ttv->d)
  {
    Derr++;
  }
  v = (GF_UINT8)mpFloatSwitchAlarmLevelFaultObj->GetValue();
  if (v != ttv->a)
  {
    Aerr++;
  }
  v = (GF_UINT8)mpUnderLowestStopLevel->GetValue();
  if (v != ttv->s)
  {
    Uerr++;
  }
  if (mpPitLevelCtrlType->GetValue() == 0)
  {
    v = (GF_UINT8)mpFloatSwitchWaterLevel->GetValue();
    if (v != ttv->l)
    {
      Lerr++;
    }
    v = (GF_UINT8)mpFloatSwitchWaterLevelPct->GetValue();
    if (v != ttv->lp)
    {
      LPerr++;
    }
  }
  ALLerr = Perr + Ierr + Herr + Derr + Aerr + Uerr + Lerr + LPerr;
  if (ALLerr) {ALLerr--; ALLerr++;} // To remove warning
}

void FloatSwitchCtrlTest::FSW_assign(void *t, GF_UINT16 i, GF_UINT8 format)
{
  auto FSW_TV *tv;
  auto ANALOG_FSW_TV *attv;

  if (format == 0)
  {
    tv = (FSW_TV *)t;
    mpFloatSwitchCnf[0]->SetValue(tv[i].c.fsw1_cfg);
    mpFloatSwitchCnf[1]->SetValue(tv[i].c.fsw2_cfg);
    mpFloatSwitchCnf[2]->SetValue(tv[i].c.fsw3_cfg);
    mpFloatSwitchCnf[3]->SetValue(tv[i].c.fsw4_cfg);
    mpFloatSwitchCnf[4]->SetValue(tv[i].c.fsw5_cfg);
    mpFloatSwitch[0]->SetValue(tv[i].v.fsw1);
    mpFloatSwitch[1]->SetValue(tv[i].v.fsw2);
    mpFloatSwitch[2]->SetValue(tv[i].v.fsw3);
    mpFloatSwitch[3]->SetValue(tv[i].v.fsw4);
    mpFloatSwitch[4]->SetValue(tv[i].v.fsw5);
    mpPitLevelCtrlType->SetValue(SENSOR_TYPE_FLOAT_SWITCHES);
  }
  else
  {
    //TODO2>6 handle pump 3 to 6

    attv = (ANALOG_FSW_TV *)t;
    mpPitLevelCtrlType->SetValue(SENSOR_TYPE_PRESSURE);
    mpHighLevel->SetValue(attv[i].hw_a);
    mpAlarmLevel->SetValue(attv[i].a_a);
    mpStartLevelPump[PUMP_2]->SetValue(attv[i].sta2_a);
    mpStartLevelPump[PUMP_1]->SetValue(attv[i].sta1_a);
    mpStopLevelPump[PUMP_2]->SetValue(attv[i].sto2_a);
    mpStopLevelPump[PUMP_1]->SetValue(attv[i].sto1_a);
    mpDryRunLevel->SetValue(attv[i].dr_a);
    mpPitLevelMeasured->SetValue(attv[i].fsw_tv.l);
    mpFloatSwitchCnf[0]->SetValue(attv[i].fsw_tv.c.fsw1_cfg);
    mpFloatSwitchCnf[1]->SetValue(attv[i].fsw_tv.c.fsw2_cfg);
    mpFloatSwitch[0]->SetValue(attv[i].fsw_tv.v.fsw1);
    mpFloatSwitch[1]->SetValue(attv[i].fsw_tv.v.fsw2);
  }
}


/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION: This is the constructor for the class, to construct
 * an object of the class type
 *****************************************************************************/
FloatSwitchCtrlTest::FloatSwitchCtrlTest(void)
{
  /* Initialize member variables */
  mReqTaskTimeFlag = true;
  mTestState = 0;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
FloatSwitchCtrlTest::~FloatSwitchCtrlTest(void)
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrlTest::InitSubTask(void)
{
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrlTest::RunSubTask(void)
{
  #if 0 //TEST JSM
  static GF_UINT8 microtest = 0;
  static GF_UINT32 teststep = 0;

  static const TVS *tvs_ptr = tvs;//TEST JSM
  //TEST JSM static TVS *tvs_ptr = tvs;

  auto GF_UINT32 i;
  auto GF_UINT8 v;

  switch (mTestState)
  {
    case 0:
      /* Update all inputs */
      FSW_assign(tvs_ptr->vt, teststep, tvs_ptr->format);
      microtest = 0;
      mTestState = 1;
      break;
    case 1:
      microtest++;
      if (microtest > 10)
      {
        mTestState = 2;
      }
      break;
    case 2:
      CheckResults(tvs_ptr->vt, teststep, tvs_ptr->format);
      teststep++;
      if (teststep < tvs_ptr->nof_tv)
      {
        mTestState = 0;
      }
      else
      {
        tvs_ptr++;
        teststep = 0;
        mTestState = 0;
        if (tvs_ptr->vt == 0)
        {
          mTestState = 3; /* WE ARE DONE */
        }
      }
      break;
    default:
      teststep++;
      if (teststep > tvs_ptr->nof_tv)
      {
        teststep = tvs_ptr->nof_tv;
      }
      break;
  }
  #endif
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void FloatSwitchCtrlTest::Update(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrlTest::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrlTest::ConnectToSubjects(void)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCtrlTest::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_FSCT_FLOAT_SWITCH_1 :
      mpFloatSwitch[0].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_2 :
      mpFloatSwitch[1].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_3 :
      mpFloatSwitch[2].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_4 :
      mpFloatSwitch[3].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_5 :
      mpFloatSwitch[4].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_CNF_1 :
      mpFloatSwitchCnf[0].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_CNF_2 :
      mpFloatSwitchCnf[1].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_CNF_3 :
      mpFloatSwitchCnf[2].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_CNF_4 :
      mpFloatSwitchCnf[3].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_CNF_5 :
      mpFloatSwitchCnf[4].Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_INCONSISTENCY_FAULT_OBJ :
      mpFloatSwitchInconsistencyFaultObj.Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_HIGH_LEVEL_FAULT_OBJ :
      mpFloatSwitchHighLevelFaultObj.Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_DRY_RUN_FAULT_OBJ :
      mpFloatSwitchDryRunFaultObj.Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_ALARM_LEVEL_OBJ :
      mpFloatSwitchAlarmLevelFaultObj.Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_WATER_LEVEL_PCT :
      mpFloatSwitchWaterLevelPct.Attach(pSubject);
      break;
    case SP_FSCT_FLOAT_SWITCH_WATER_LEVEL :
      mpFloatSwitchWaterLevel.Attach(pSubject);
      break;
    case SP_FSCT_UNDER_LOWEST_STOP_LEVEL :
      mpUnderLowestStopLevel.Attach(pSubject);
      break;
    case SP_FSCT_PUMP_REF :
      mpPumpRef.Attach(pSubject);
      break;
    case SP_FSCT_START_LEVEL_PUMP_1 :
      mpStartLevelPump[PUMP_1].Attach(pSubject);
      break;
    case SP_FSCT_START_LEVEL_PUMP_2 :
      mpStartLevelPump[PUMP_2].Attach(pSubject);
      break;
    case SP_FSCT_START_LEVEL_PUMP_3 :
      mpStartLevelPump[PUMP_3].Attach(pSubject);
      break;
    case SP_FSCT_START_LEVEL_PUMP_4 :
      mpStartLevelPump[PUMP_4].Attach(pSubject);
      break;
    case SP_FSCT_START_LEVEL_PUMP_5 :
      mpStartLevelPump[PUMP_5].Attach(pSubject);
      break;
    case SP_FSCT_START_LEVEL_PUMP_6 :
      mpStartLevelPump[PUMP_6].Attach(pSubject);
      break;
    case SP_FSCT_STOP_LEVEL_PUMP_1 :
      mpStopLevelPump[PUMP_1].Attach(pSubject);
      break;
    case SP_FSCT_STOP_LEVEL_PUMP_2 :
      mpStopLevelPump[PUMP_2].Attach(pSubject);
      break;
    case SP_FSCT_STOP_LEVEL_PUMP_3 :
      mpStopLevelPump[PUMP_3].Attach(pSubject);
      break;
    case SP_FSCT_STOP_LEVEL_PUMP_4 :
      mpStopLevelPump[PUMP_4].Attach(pSubject);
      break;
    case SP_FSCT_STOP_LEVEL_PUMP_5 :
      mpStopLevelPump[PUMP_5].Attach(pSubject);
      break;
    case SP_FSCT_STOP_LEVEL_PUMP_6 :
      mpStopLevelPump[PUMP_6].Attach(pSubject);
      break;
    case SP_FSCT_ALARM_LEVEL :
      mpAlarmLevel.Attach(pSubject);
      break;
    case SP_FSCT_DRY_RUN_LEVEL:
      mpDryRunLevel.Attach(pSubject);
      break;
    case SP_FSCT_HIGH_LEVEL:
      mpHighLevel.Attach(pSubject);
      break;
    case SP_FSCT_PIT_LEVEL_MEASURED:
      mpPitLevelMeasured.Attach(pSubject);
      break;
    case SP_FSCT_PIT_LEVEL_CTRL_TYPE:
      mpPitLevelCtrlType.Attach(pSubject);
      break;
  }
}
/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

