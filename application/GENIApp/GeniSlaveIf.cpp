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
/* CLASS NAME       : GeniSlaveIf                                           */
/*                                                                          */
/* FILE NAME        : GeniSlaveIf.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 01-04-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : This file encapsulate GENI slaves               */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
extern "C"
{
  #include "geni_if.h"
}

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <GeniSlaveIf.h>
#include <InfoDef.h>
#include <microp.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define NO_RESP_FNC                  0
#define SLAVE_INFO_RESP_FNC          1
#define IO351_MODULE_CONFIG_RESP_FNC 2
#define DUMMY_RESP_FNC               3

#define CUE_ADDR_OFFSET              0x20 // CUE               (0x20= 32)
#define MP204_ADDR_OFFSET            0x20 // MP204             (0x20= 32)
#define IO111_ADDR_OFFSET            0x28 // IO111             (0x28= 40)
#define IO113_MIXER_ADDR_OFFSET      0x32 // IO113 (MIXER)     (0x32= 50)
#define IO351_PM_ADDR_OFFSET         0x3E // IO351 Pump Module (0x3E = 62, first number on unit is 31)
#define IO351_IOM_ADDR_OFFSET        0x48 // IO351 IO Module   (0x48 = 72, first number on unit is 41)
#define DDA_ADDR_OFFSET              0x52 // DDA Module   (0x52 = 82, first number on unit is 51 (31+51))

#define UNIT_FAMILY_CUE              2
#define UNIT_FAMILY_MP204            7
#define UNIT_FAMILY_IO11x            22
#define UNIT_FAMILY_DDA              30

#define GENI_UCHAR_2_FLOAT           2.54f
#define GENI_I_MO_SCALE              0.1f    //A
#define GENI_P_DC_SCALE              100.0f  //W
#define GENI_OT_SCALE                2.0f    //h
#define GENI_E_DC_SCALE              2.0f    //kWh
#define GENI_ANA_IN_SCALE            100.0f/(254.0f*256.0f) //Pct
#define GENI_ANA_OUT_SCALE           2.54f

/* GSC's used in GSC class */
#define E_PUMP_GSC     26
#define IO351_PUMP_GSC 27

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  INITIALIZE STATIC ATTRIBUTES
 *****************************************************************************/
GeniSlaveIf* GeniSlaveIf::mInstance = 0;

/*****************************************************************************
  CONSTS
 *****************************************************************************/

/* Only for DEVICE_E_PUMP */
const U8 apdu_cue_info_request[] = {
  0x0A,                                 /* PDU_LEN                    */
  0x02,                                 /* class 2                    */
  0xC8,                                 /* INFO / LEN                 */
  27,                                   /* s_v_dc - First ID          */
  30,                                   /* s_i_mo                     */
  32,                                   /* s_f_act                    */
  33,                                   /* s_p_hi                     */
  35,                                   /* s_speed_hi                 */
  45,                                   /* s_torque_hi                */
  86,                                   /* s_t_remote                 */
  151 };                                /* s_energy_hi - Last ID      */

 // APDU TO THE ANALOG INPUTS ON THE IO MODUL
const U8 apdu_set_ana_in_one_0_20[] =
  {
  0x04,//Length of the rest of buffer
  0x04,//Class
  0x82,//OPERATION + Length of APDU
  0x17,//ID
  0x00,//VAL
  };

const U8 apdu_set_ana_in_one_4_20[] =
  {
  0x04,//Length of the rest of buffer
  0x04,//Class
  0x82,//OPERATION + Length of APDU
  0x17,//ID
  0x03,//VAL
  };

const U8 apdu_set_ana_in_one_0_10[] =
  {
  0x04,//Length of the rest of buffer
  0x04,//Class
  0x82,//OPERATION + Length of APDU
  0x17,//ID
  0x02,//VAL
  };

const U8 apdu_set_ana_in_one_not_active[] =
  {
  0x04,//Length of the rest of buffer
  0x04,//Class
  0x82,//OPERATION + Length of APDU
  0x17,//ID
  0x04,//VAL
  };

const U8 apdu_set_ana_in_two_0_20[] =
  {
  0x04,//Length of the rest of buffer
  0x04,//Class
  0x82,//OPERATION + Length of APDU
  0x18,//ID
  0x00,//VAL
  };

const U8 apdu_set_ana_in_two_4_20[] =
  {
  0x04,//Length of the rest of buffer
  0x04,//Class
  0x82,//OPERATION + Length of APDU
  0x18,//ID
  0x03,//VAL
  };

const U8 apdu_set_ana_in_two_0_10[] =
  {
  0x04,//Length of the rest of buffer
  0x04,//Class
  0x82,//OPERATION + Length of APDU
  0x18,//ID
  0x02,//VAL
  };

const U8 apdu_set_ana_in_two_not_active[] =
  {
  0x04,//Length of the rest of buffer
  0x04,//Class
  0x82,//OPERATION + Length of APDU
  0x18,//ID
  0x04,//VAL
  };

const U8 apdu_get_io351_config[] =
  {
  0x06, //Length of the rest of buffer
  0x04, //Class
  0x04, //OPERATION=GET + Length of APDU
  0x14, //NO_OF_PUMPS
  0x15, //NO_OF_VFD
  0x16, //ADDR_OFFSET
  0x19  //SYSTEM_TYPE
  };

const U8 apdu_reset[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  0x01  //RESET
  };

const U8 apdu_alarm_reset[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  0x02  //RESET_ALARM
  };

const U8 apdu_stop[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  0x05  //STOP
  };

const U8 apdu_start[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  0x06  //START
  };

const U8 apdu_remote[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  0x07  //REMOTE
  };

const U8 apdu_local[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  0x08  //LOCAL
  };

const U8 apdu_DDA_UseMode[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  19  //Use Mode
  };

const U8 apdu_DDA_ManualDosing[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  61    //Sets the pump in Control mode ¡°Manual dosing¡±
  };

const U8 apdu_DDA_AnalogueDosing[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  63    //Sets the pump in Control mode "Analogue dosing"
  };

const U8 apdu_DDA_PulseDosing[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  62    //Sets the pump in Control mode "Pulse dosing"
  };

const U8 apdu_DDA_PressStartKey[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  67    //Start the Manual dosing
  };

const U8 apdu_DDA_PressStopKey[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  68    //Stop the Manual dosing
  };


const U8 apdu_const_freq[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  0x16  //CONSTANT_FREQUENCY
  };

const U8 apdu_forward[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  0x0F  //FORWARD
  };

const U8 apdu_reverse[] =
  {
  0x03, //Length of the rest of buffer
  0x03, //Class
  0x81, //OPERATION=SET + Length of APDU
  0x10  //REVERSE
  };

#define APDU_UNIT_ADDR_INDEX 4
U8 apdu_set_unit_addr[] = 
  {
    0x04, //Length of the rest of buffer
    0x04, //Class
    0x82, //OPERATION=SET + Length of APDU
    46, // Geni unit addr id
    0x00, //Placeholder for new id.
  };
 



/*****************************************************************************
  C functions declarations
 *****************************************************************************/
extern "C" void set_geni_error(unsigned char i, unsigned char s);
extern "C" void SlaveInfoFromUnit(void);
extern "C" void ConfigFromIoModule(void);

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniSlaveIf::InitSubTask()
{
//  int i;

//XHE  for (i=0; i<NO_OF_DEVICES_TO_CONFIG; i++)
//  {
//    StoreDeviceConfiguration(i);
//  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION: Runs a check of if more info is required. Normally the reception
 *              of infodata will automatically lead to a new info request, but
 *              if the communication stops this subtask will kick it alive
 *              again.
 *
 *****************************************************************************/
void GeniSlaveIf::RunSubTask()
{
  CheckForAutoPollEnableDisable();
  CheckForInfoRequest();
  CheckForPendingUnitConfiguration();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void GeniSlaveIf::ConnectToSubjects()
{
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void GeniSlaveIf::Update(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION: If pSubject is a pointer of ConfigContol then remove from list
 *
 *****************************************************************************/
void GeniSlaveIf::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void GeniSlaveIf::SetSubjectPointer(int id, Subject* pSubject)
{
}

/*****************************************************************************
 * Function - GetUnitAddress
 * DESCRIPTION:
*****************************************************************************/
U8 GeniSlaveIf::GetUnitAddress(IO351_NO_TYPE moduleNo) const
{
  switch (moduleNo)
  {
  case IO351_PM_NO_1:
    return IO351_PM_ADDR_OFFSET + 0;
  case IO351_PM_NO_2:
    return IO351_PM_ADDR_OFFSET + 1;
  case IO351_PM_NO_3:
    return IO351_PM_ADDR_OFFSET + 2;
  case IO351_PM_NO_4:
    return IO351_PM_ADDR_OFFSET + 3;
  case IO351_PM_NO_5:
    return IO351_PM_ADDR_OFFSET + 4;
  case IO351_PM_NO_6:
    return IO351_PM_ADDR_OFFSET + 5;

  case IO351_IOM_NO_1:
    return IO351_IOM_ADDR_OFFSET + 0;
  case IO351_IOM_NO_2:
    return IO351_IOM_ADDR_OFFSET + 1;
  case IO351_IOM_NO_3:
    return IO351_IOM_ADDR_OFFSET + 2;

  case IO111_NO_1:
    return IO111_ADDR_OFFSET + 0;
  case IO111_NO_2:
    return IO111_ADDR_OFFSET + 1;
  case IO111_NO_3:
    return IO111_ADDR_OFFSET + 2;
  case IO111_NO_4:
    return IO111_ADDR_OFFSET + 3;
  case IO111_NO_5:
    return IO111_ADDR_OFFSET + 4;
  case IO111_NO_6:
    return IO111_ADDR_OFFSET + 5;

  case IO113_WITH_MIXER_NO_1:
    return IO113_MIXER_ADDR_OFFSET + 0;


  case CUE_NO_1:
    return CUE_ADDR_OFFSET + 0;
  case CUE_NO_2:
    return CUE_ADDR_OFFSET + 1;
  case CUE_NO_3:
    return CUE_ADDR_OFFSET + 2;
  case CUE_NO_4:
    return CUE_ADDR_OFFSET + 3;
  case CUE_NO_5:
    return CUE_ADDR_OFFSET + 4;
  case CUE_NO_6:
    return CUE_ADDR_OFFSET + 5;

  case MP204_NO_1:
    return MP204_ADDR_OFFSET + 0;
  case MP204_NO_2:
    return MP204_ADDR_OFFSET + 1;
  case MP204_NO_3:
    return MP204_ADDR_OFFSET + 2;
  case MP204_NO_4:
    return MP204_ADDR_OFFSET + 3;
  case MP204_NO_5:
    return MP204_ADDR_OFFSET + 4;
  case MP204_NO_6:
    return MP204_ADDR_OFFSET + 5;
  case DDA_NO_1:
    return DDA_ADDR_OFFSET + 0;
    
  default:
    FatalErrorOccured("GENI address out of range!");
    return 0;       // We have no address to return - thus return a 0
  }
}

/*****************************************************************************
 * Function - ConnectDevice
 * DESCRIPTION:
*****************************************************************************/
U8 GeniSlaveIf::ConnectDevice(IO351_NO_TYPE moduleNo, GENI_DEVICE_TYPE device)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  U8 unit_index;

  OS_Use(&geni_master);
  unit_index = FindUnit(unit_addr);
  OS_Unuse(&geni_master);

  if (unit_index != NO_UNIT)
  {
    // Address is or has been in use
    if (mUnit2Device[unit_index] == NO_DEVICE)
    {
      OS_Use(&geni_master);
      network_list[unit_index].device_type = device;
      OS_Unuse(&geni_master);
      mUnit2Device[unit_index] = device;
    }
    else
    {
      unit_index = NO_UNIT;
    }
  }
  else
  {
    // Insert the device in the geni network list
    OS_Use(&geni_master);
    if (InsertUnit(unit_addr, device) == true)
    {
      unit_index = FindUnit(unit_addr);
      mUnit2Device[unit_index] = device;
    }
    OS_Unuse(&geni_master);
  }

  return unit_index;
}

/*****************************************************************************
 * Function - DisconnectDevice
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DisconnectDevice(IO351_NO_TYPE moduleNo, GENI_DEVICE_TYPE device)
{
  // NOTE: RemoveUnit CANNOT BE USED due to the way of using indexes in this class.
  // Instead, it seems safe to ignore this function. (At next power on the unit will not be inserted anyway)
  // OS_Use(&geni_master);
  // RemoveUnit(GetUnitAddress(moduleNo));
  // OS_Unuse(&geni_master);

  U8 unit_index;
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
  if (unit_index != NO_UNIT && mUnit2Device[unit_index] == device)
  {
    OS_Use(&geni_master);
    network_list[unit_index].device_type = NO_DEVICE;
    OS_Unuse(&geni_master);
    mUnit2Device[unit_index] = NO_DEVICE;
  }
}



/*****************************************************************************
 *
 * MP204 functions
 *
*****************************************************************************/

/*****************************************************************************
 * Function - ConnectMP204
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::ConnectMP204(IO351_NO_TYPE moduleNo)
{
  return (ConnectDevice(moduleNo, DEVICE_MP204) != NO_UNIT);
}

/*****************************************************************************
 * Function - DisconnectMP204
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DisconnectMP204(IO351_NO_TYPE moduleNo)
{
  DisconnectDevice(moduleNo, DEVICE_MP204);
}


/*****************************************************************************
 * Function - MP204Reset
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::MP204Reset(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(NO_RESP_FNC, (U8 *)apdu_reset, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}


/*****************************************************************************
 * Function - MP204AlarmReset
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::MP204AlarmReset(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_alarm_reset, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}


/*****************************************************************************
 * Function - GetMP204Voltage
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204Voltage(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U16 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value =  (U16)s_cl2_id035[unit_index]<<8;  // v_line_hi
      geni_value |= (U16)s_cl2_id036[unit_index];     // v_line_lo
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF00)
      {
        *pValue = 0.1f*geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204Current
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204Current(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U16 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value =  (U16)s_cl2_id043[unit_index]<<8;  // i_line_hi
      geni_value |= (U16)s_cl2_id044[unit_index];     // i_line_lo
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF00)
      {
        *pValue = 0.1f*geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204CurrentAsymmetry
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204CurrentAsymmetry(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id049[unit_index]; // i_asym
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF)
      {
        *pValue = 0.1f*geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204CosPhi
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204CosPhi(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id064[unit_index]; // cos_phi
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF)
      {
        *pValue = 0.01f*geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}

/*****************************************************************************
 * Function - GetMP204Power
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204Power(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U32 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value =  (U32)s_cl2_id065[unit_index]<<24; // power_hi
      geni_value |= (U32)s_cl2_id066[unit_index]<<16; // power_lo1
      geni_value |= (U32)s_cl2_id067[unit_index]<<8;  // power_lo2
      geni_value |= (U32)s_cl2_id068[unit_index];     // power_lo3
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF000000)
      {
        *pValue = geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204Energy
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204Energy(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U32 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value =  (U32)s_cl2_id069[unit_index]<<24; // energy_hi
      geni_value |= (U32)s_cl2_id070[unit_index]<<16; // energy_lo1
      geni_value |= (U32)s_cl2_id071[unit_index]<<8;  // energy_lo2
      geni_value |= (U32)s_cl2_id072[unit_index];     // energy_lo3
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF000000)
      {
        *pValue = 3600000.0f*geni_value; // kWh --> J
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204InsulationResistance
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204InsulationResistance(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id094[unit_index]; // r_insulation
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF)
      {
        *pValue = 10000.0f*geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204TemperaturePtc
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204TemperaturePtc(IO351_NO_TYPE moduleNo, bool* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id099[unit_index]; // dig_in
      OS_Unuse(&geni_class_data);
      *pValue = ((geni_value&0x1) == 0x1);
      ret_val = true;
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204TemperaturePt
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204TemperaturePt(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id048[unit_index]; // t_mo2 (PT sensor)
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF)
      {
        *pValue = 273.15f + (float)geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204TemperatureTempcon
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204TemperatureTempcon(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id047[unit_index]; // t_mo1 (Tempcon)
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF)
      {
        *pValue = 273.15f + (float)geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204AlarmCode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204AlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue)
{
  U8 unit_index, geni_value, unit_family;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id144[unit_index];  //alarm_code
      unit_family = s_cl2_id148[unit_index];
      OS_Unuse(&geni_class_data);
      if (unit_family == UNIT_FAMILY_MP204) // Yes, it's an MP204
      {
        *pValue = (ALARM_ID_TYPE)(geni_value);
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetMP204WarningCode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetMP204WarningCode(IO351_NO_TYPE moduleNo, U32* pValue)
{
  U8 unit_index;
  U32 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_MP204)
    {
      OS_Use(&geni_class_data);
      geni_value  = (U32)s_cl2_id147[unit_index]<<16; // warnings 3
      geni_value |= (U32)s_cl2_id146[unit_index]<<8;  // warnings 2
      geni_value |= (U32)s_cl2_id145[unit_index];     // warnings 1
      OS_Unuse(&geni_class_data);
      *pValue = geni_value;
      ret_val = true;
    }
  }

  return ret_val;
}


/*****************************************************************************
 *
 * CUE functions
 *
*****************************************************************************/

/*****************************************************************************
 * Function - ConnectCUE
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::ConnectCUE(IO351_NO_TYPE moduleNo)
{
  U8 unit_index = ConnectDevice(moduleNo, DEVICE_E_PUMP);
  if (unit_index != NO_UNIT)
  {
    mUnitInfoWanted[unit_index] = true;
    mUnitInfoReceived[unit_index] = false;
    CheckForInfoRequest();
    return  true;
  }
  else
  {
    return false;
  }
}

/*****************************************************************************
 * Function - DisconnectCUE
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DisconnectCUE(IO351_NO_TYPE moduleNo)
{
  DisconnectDevice(moduleNo, DEVICE_E_PUMP);
}


/*****************************************************************************
 * Function - CUEReset
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::CUEReset(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(NO_RESP_FNC, (U8 *)apdu_reset, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - CUEAlarmReset
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::CUEAlarmReset(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_alarm_reset, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}


/*****************************************************************************
 * Function - CUEStop
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::CUEStop(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_stop, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - CUEStart
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::CUEStart(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_start, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - CUERemote
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::CUERemote(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_remote, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - CUELocal
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::CUELocal(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_local, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - CUEConstFreq
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::CUEConstFreq(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_const_freq, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - CUEForward
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::CUEForward(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_forward, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - CUEReverse
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::CUEReverse(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_reverse, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}



/*****************************************************************************
 * Function - GetCUEVoltage
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUEVoltage(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id027[unit_index];  //v_dc
      OS_Unuse(&geni_class_data);

      ret_val = Scale8BitGeniValue(geni_value, &(s_cl2_id027_info[unit_index][GENI_INFO_HEAD]), pValue);
      // Try extended scaling if standard fails
      if ( ret_val == false)
      {
        ret_val = ScaleExtendedGeniValue(geni_value, &(s_cl2_id027_info[unit_index][GENI_INFO_HEAD]), pValue, 1);
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetCUEPower
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUEPower(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U16 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = (U16)s_cl2_id033[unit_index]<<8; //p_hi
      geni_value |= (U16)s_cl2_id034[unit_index]; //p_lo
      OS_Unuse(&geni_class_data);

      ret_val = Scale16BitGeniValue(geni_value, &(s_cl2_id033_info[unit_index][GENI_INFO_HEAD]), pValue);
      // Try extended scaling if standard fails
      if ( ret_val == false)
      {
        ret_val = ScaleExtendedGeniValue(geni_value, &(s_cl2_id033_info[unit_index][GENI_INFO_HEAD]), pValue, 2);
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetCUEEnergy
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUEEnergy(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U32 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = (U32)s_cl2_id151[unit_index]<<16;  //energy_hi
      geni_value |= (U32)s_cl2_id152[unit_index]<<8;  //energy_lo1
      geni_value |= (U32)s_cl2_id153[unit_index]; //energy_lo2
      OS_Unuse(&geni_class_data);

      ret_val = Scale24BitGeniValue(geni_value, &(s_cl2_id151_info[unit_index][GENI_INFO_HEAD]), pValue);
      // Try extended scaling if standard fails
      if ( ret_val == false)
      {
        ret_val = ScaleExtendedGeniValue(geni_value, &(s_cl2_id151_info[unit_index][GENI_INFO_HEAD]), pValue, 3);
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetCUECurrent
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUECurrent(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id030[unit_index];  //i_mo
      OS_Unuse(&geni_class_data);

      ret_val = Scale8BitGeniValue(geni_value, &(s_cl2_id030_info[unit_index][GENI_INFO_HEAD]), pValue);
      // Try extended scaling if standard fails
      if ( ret_val == false)
      {
        ret_val = ScaleExtendedGeniValue(geni_value, &(s_cl2_id030_info[unit_index][GENI_INFO_HEAD]), pValue, 1);
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetCUEFrequency
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUEFrequency(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id032[unit_index];  //f_act
      OS_Unuse(&geni_class_data);

      ret_val = Scale8BitGeniValue(geni_value, &(s_cl2_id032_info[unit_index][GENI_INFO_HEAD]), pValue);
      // Try extended scaling if standard fails
      if ( ret_val == false)
      {
        ret_val = ScaleExtendedGeniValue(geni_value, &(s_cl2_id032_info[unit_index][GENI_INFO_HEAD]), pValue, 1);
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetCUESpeed
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUESpeed(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U16 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = (U16)s_cl2_id035[unit_index]<<8; //speed_hi
      geni_value |= (U16)s_cl2_id036[unit_index]; //speed_lo
      OS_Unuse(&geni_class_data);

      ret_val = Scale16BitGeniValue(geni_value, &(s_cl2_id035_info[unit_index][GENI_INFO_HEAD]), pValue);
      // Try extended scaling if standard fails
      if ( ret_val == false)
      {
        ret_val = ScaleExtendedGeniValue(geni_value, &(s_cl2_id035_info[unit_index][GENI_INFO_HEAD]), pValue, 2);
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetCUETorque
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUETorque(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U16 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = (U16)s_cl2_id045[unit_index]<<8; //torque_hi
      geni_value |= (U16)s_cl2_id046[unit_index]; //torque_lo
      OS_Unuse(&geni_class_data);

      ret_val = Scale16BitGeniValue(geni_value, &(s_cl2_id045_info[unit_index][GENI_INFO_HEAD]), pValue);
      // Try extended scaling if standard fails
      if ( ret_val == false)
      {
        ret_val = ScaleExtendedGeniValue(geni_value, &(s_cl2_id045_info[unit_index][GENI_INFO_HEAD]), pValue, 2);
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetCUERemoteTemperature
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUERemoteTemperature(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id086[unit_index];  //t_remote_2
      OS_Unuse(&geni_class_data);

      ret_val = Scale8BitGeniValue(geni_value, &(s_cl2_id086_info[unit_index][GENI_INFO_HEAD]), pValue);
      // Try extended scaling if standard fails
      if ( ret_val == false)
      {
        ret_val = ScaleExtendedGeniValue(geni_value, &(s_cl2_id086_info[unit_index][GENI_INFO_HEAD]), pValue, 1);
      }
    }
  }

  return ret_val;
}



/*****************************************************************************
 * Function - GetCUEOperationMode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUEOperationMode(IO351_NO_TYPE moduleNo, CUE_OPERATION_MODE_TYPE* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id081[unit_index];  //act_mode1
      OS_Unuse(&geni_class_data);

      switch (geni_value & 0x07) //Mask operation mode in pump
      {
        case 0 :
          *pValue = CUE_OPERATION_MODE_PUMP_ON;
          break;
        case 1 :
          *pValue = CUE_OPERATION_MODE_PUMP_OFF;
          break;
        case 2 :
          *pValue = CUE_OPERATION_MODE_PUMP_MIN;
          break;
        case 3 :
          *pValue = CUE_OPERATION_MODE_PUMP_MAX;
          break;
        default:
          *pValue = CUE_OPERATION_MODE_PUMP_ON;
          break;
      }
      ret_val = true;
    }
  }
  return ret_val;
}



/*****************************************************************************
 * Function - GetCUESystemMode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUESystemMode(IO351_NO_TYPE moduleNo, CUE_SYSTEM_MODE_TYPE* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id083[unit_index];  //act_mode3
      OS_Unuse(&geni_class_data);

      switch (geni_value & 0x07) //Mask system mode in pump
      {
        case 0 :                        // 'Normal'
          *pValue = CUE_SYSTEM_MODE_PUMP_READY;
          break;
        case 1 :                        // 'Power up'
          *pValue = CUE_SYSTEM_MODE_PUMP_READY;
          break;
        case 3 :
          *pValue = CUE_SYSTEM_MODE_PUMP_WARNING;
          break;
        case 4 :                        // 'Event action'
          *pValue = CUE_SYSTEM_MODE_PUMP_ALARM;
          break;
        default:
          *pValue = CUE_SYSTEM_MODE_PUMP_READY;
          break;
      }
      ret_val = true;
    }
  }
  return ret_val;
}



/*****************************************************************************
 * Function - GetCUEControlMode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUEControlMode(IO351_NO_TYPE moduleNo, CUE_LOOP_MODE_TYPE* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id081[unit_index];  //act_mode1
      OS_Unuse(&geni_class_data);

      switch (geni_value & 0x38) //Mask control mode in pump
      {
        case 0x00 : // Constant Pressure
          *pValue = CUE_LOOP_MODE_PUMP_CONST_PRESS;
          break;
        case 0x08 : // Proportional Pressure
          *pValue = CUE_LOOP_MODE_PUMP_PROP_PRESS;
          break;
        case 0x28 : // Auto
          *pValue = CUE_LOOP_MODE_PUMP_AUTO;
          break;
        case 0x10 : // Constant frequency
          *pValue = CUE_LOOP_MODE_PUMP_CONST_FREQ;
          break;
        default:
          *pValue = CUE_LOOP_MODE_PUMP_CONST_PRESS;
          break;
      }
      ret_val = true;
    }
  }
  return ret_val;
}



/*****************************************************************************
 * Function - GetCUESourceMode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUESourceMode(IO351_NO_TYPE moduleNo, CUE_SOURCE_MODE_TYPE* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id083[unit_index];  //act_mode3
      OS_Unuse(&geni_class_data);

      switch (geni_value & 0x10) //Mask source mode in pump
      {
        case 0x00 :
          *pValue = CUE_SOURCE_MODE_REMOTE;
          break;
        case 0x10 :
          *pValue = CUE_SOURCE_MODE_LOCAL;
          break;
        default:
          *pValue = CUE_SOURCE_MODE_LOCAL;
          break;
      }
      ret_val = true;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetCUEReverseStatus
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUEReverseStatus(IO351_NO_TYPE moduleNo, bool* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id095[unit_index];  //drive_modes1
      OS_Unuse(&geni_class_data);

      if (geni_value == 255)
      {
        *pValue = false;
      }
      else switch (geni_value & 0x01) //Mask reverse status
      {
        case 0x00 :
          *pValue = false;
          break;
        case 0x01 :
          *pValue = true;
          break;
        default:
          *pValue = false;
          break;
      }
      ret_val = true;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetCUEAlarmCode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUEAlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue)
{
  U8 unit_index, geni_value, unit_family;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id158[unit_index];  //alarm_code
      unit_family = s_cl2_id148[unit_index];
      OS_Unuse(&geni_class_data);
      if (unit_family == UNIT_FAMILY_CUE) // Yes, it's a CUE (or HM/MGE)
      {
        *pValue = (ALARM_ID_TYPE)(geni_value);
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetCUEWarningCode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetCUEWarningCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id156[unit_index];  //warning_code
      OS_Unuse(&geni_class_data);

      *pValue = (ALARM_ID_TYPE)(geni_value);
      ret_val = true;
    }
  }

  return ret_val;
}



/*****************************************************************************
 * Function - SetCUEReference
 * DESCRIPTION:
 *
*****************************************************************************/
bool GeniSlaveIf::SetCUEReference(IO351_NO_TYPE moduleNo, float outValue)
{
  U8 unit_index;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_E_PUMP)
    {
      OS_Use(&geni_class_data);
      // outValue is in Hertz and must be scale to percent of f_upper
      U8 f_upper = s_cl4_id030[unit_index];
      if (f_upper == 0 || f_upper == 255)
      {
        f_upper = 50;
      }
      outValue = (outValue/f_upper);
      if (outValue > 1.00)
      {
        outValue = 1;
      }
      s_cl5_id001[unit_index] = (U8)((outValue*254)+0.5f);
      OS_Unuse(&geni_class_data);

      ret_val = true;
    }
  }

  return ret_val;
}


/*****************************************************************************
 *
 * End of CUE functions
 *
*****************************************************************************/



/*****************************************************************************
 *
 * IO 111 functions
 *
*****************************************************************************/

/*****************************************************************************
 * Function - ConnectIO111
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::ConnectIO111(IO351_NO_TYPE moduleNo)
{
  return (ConnectDevice(moduleNo, DEVICE_IO111) != NO_UNIT);
}

/*****************************************************************************
 * Function - DisconnectIO111
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DisconnectIO111(IO351_NO_TYPE moduleNo)
{
  DisconnectDevice(moduleNo, DEVICE_IO111);
}

/*****************************************************************************
 * Function - ResetIO111alarm
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::IO111AlarmReset(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(NO_RESP_FNC, (U8 *)apdu_alarm_reset, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - GetIO111TemperatureSupportBearing
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111TemperatureSupportBearing(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id021[unit_index];  //t_support_bear
      OS_Unuse(&geni_class_data);

      if (geni_value != 0xFF)
      {
        *pValue = (float)geni_value + 273.15f;  //from Celcius to Kelvin
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111TemperatureMainBearing
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111TemperatureMainBearing(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id022[unit_index];  //t_main_bear
      OS_Unuse(&geni_class_data);

      if (geni_value != 0xFF)
      {
        *pValue = (float)geni_value + 273.15f;  //from Celcius to Kelvin
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111TemperaturePT100
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111TemperaturePT100(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id027[unit_index];  //t_pt100
      OS_Unuse(&geni_class_data);

      if (geni_value != 0xFF)
      {
        *pValue = (float)geni_value + 273.15f;  //from Celcius to Kelvin
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111TemperaturePT1000
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111TemperaturePT1000(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id029[unit_index];  //t_pt1000
      OS_Unuse(&geni_class_data);

      if (geni_value != 0xFF)
      {
        *pValue = (float)geni_value + 273.15f;  //from Celcius to Kelvin
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111InsulationResistance
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111InsulationResistance(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id030[unit_index];  //r_insulate
      OS_Unuse(&geni_class_data);

      if (geni_value != 0xFF)
      {
        *pValue = ((float)geni_value) * 100000;  // converts from 100kohm to ohm
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111Vibration
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111Vibration(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id050[unit_index];  //vibration
      OS_Unuse(&geni_class_data);

      if (geni_value != 0xFF)
      {
        *pValue = ((float)geni_value) * 0.001;  // converts from mm/s to m/s
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111WaterInOil
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111WaterInOil(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id060[unit_index];  //water_in_oil
      OS_Unuse(&geni_class_data);

      if (geni_value != 0xFF)
      {
        *pValue = ((float)geni_value) * 0.1;  // converts from 0,1% to %
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111MoistureSwitch
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111MoistureSwitch(IO351_NO_TYPE moduleNo, bool* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id144[unit_index];  //digital_sensors
      OS_Unuse(&geni_class_data);

      *pValue = (bool)(geni_value&0x01);  // moisture switch is bit 0 in alarms2
      ret_val = true;
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111ThermalSwitch
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111ThermalSwitch(IO351_NO_TYPE moduleNo, bool* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id144[unit_index];  //digital_sensors
      OS_Unuse(&geni_class_data);

      *pValue = (bool)(geni_value & 0x02);  // Thermal switch is bit 1 in alarms2
      ret_val = true;
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111AlarmCode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111AlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id141[unit_index];  //alarm_code
      OS_Unuse(&geni_class_data);

      *pValue = (ALARM_ID_TYPE)(geni_value);
      ret_val = true;
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO111WarningCode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO111WarningCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id142[unit_index];  //warning_code
      OS_Unuse(&geni_class_data);

      *pValue = (ALARM_ID_TYPE)(geni_value);
      ret_val = true;
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO11xUnitType
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO11xUnitType(IO351_NO_TYPE moduleNo, U8* pValue)
{
  U8 unit_index, geni_value, unit_family;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

// find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      unit_family = s_cl2_id148[unit_index];
      geni_value = s_cl2_id149[unit_index];  // unit_type
      OS_Unuse(&geni_class_data);

      // Verify unit family
      if (unit_family == UNIT_FAMILY_IO11x)
      {
        if( geni_value != 0xFF )
        {
          *pValue = (U8)(geni_value);
          ret_val = true;
        }
      }
    }
  }

  return ret_val;
}

/*****************************************************************************
 * Function - GetIO113Speed
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO113Speed(IO351_NO_TYPE moduleNo, U16* pValue)
{
  U8 unit_index, geni_value_hi, geni_value_lo;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO111)
    {
      OS_Use(&geni_class_data);
      geni_value_hi = s_cl2_id033[unit_index]; // speed_hi
      geni_value_lo = s_cl2_id034[unit_index]; // speed lo
      OS_Unuse(&geni_class_data);

      if (geni_value_hi != 0xFF)
      {
        *pValue = geni_value_hi<<8;
        *pValue += geni_value_lo;
        ret_val = true;
      }
    }
  }


  return ret_val;
}


/*****************************************************************************
 *
 * End of IO 111 functions
 *
*****************************************************************************/



/*****************************************************************************
 *
 * IO 351 functions
 *
*****************************************************************************/


/*****************************************************************************
 * Function - SetIO351Pointer
 * DESCRIPTION: Set pointers to io module control objects
*****************************************************************************/
void GeniSlaveIf::SetIO351Pointer(IO351_NO_TYPE moduleNo, IO351* pIO351)
{
  switch (moduleNo)
  {
  case IO351_IOM_NO_1:
    mpIO351IOModules[0] = pIO351;
    break;
  case IO351_IOM_NO_2:
    mpIO351IOModules[1] = pIO351;
    break;
  case IO351_IOM_NO_3:
    mpIO351IOModules[2] = pIO351;
    break;
  }
}

/*****************************************************************************
 * Function - ConnectIO351
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::ConnectIO351(IO351_NO_TYPE moduleNo)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  U8 unit_index;
  bool unit_inserted;

  OS_Use(&geni_master);
  unit_inserted = InsertUnit(unit_addr, DEVICE_IO351_IO_MODULE);
  OS_Unuse(&geni_master);

  if (unit_inserted)
  {
    OS_Use(&geni_master);
    unit_index = FindUnit(unit_addr);
    OS_Unuse(&geni_master);
    mUnit2Device[unit_index] = DEVICE_IO351_IO_MODULE;
    return  true;
  }
  else
  {
    return false;
  }
}

/*****************************************************************************
 * Function - DisconnectIO351
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DisconnectIO351(IO351_NO_TYPE moduleNo)
{
  // NOTE: RemoveUnit CANNOT BE USED due to the way of using indexes in this class.
  // Instead, it seems safe to ignore this function. (At next power on the unit will not be inserted anyway)
  // OS_Use(&geni_master);
  // RemoveUnit(GetUnitAddress(moduleNo));
  // OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - ResetIO351
 * DESCRIPTION: Reset the IO351
*****************************************************************************/
bool GeniSlaveIf::ResetIO351(IO351_NO_TYPE moduleNo)
{
  bool ret_value;

  OS_Use(&geni_master);
  if (SendDirAPDU(NO_RESP_FNC, (U8 *)apdu_reset, GetUnitAddress(moduleNo)))
  {
    ret_value = true;
  }
  else
  {
    ret_value = false;
  }
  OS_Unuse(&geni_master);

  return ret_value;
}

/*****************************************************************************
 * Function - GetIO351ModuleConfig
 * DESCRIPTION: Request configuration from the IO351
*****************************************************************************/
bool GeniSlaveIf::GetIO351ModuleConfig(IO351_NO_TYPE moduleNo)
{
  bool ret_value;

  OS_Use(&geni_master);
  if (SendDirAPDU(IO351_MODULE_CONFIG_RESP_FNC, (U8*)apdu_get_io351_config, GetUnitAddress(moduleNo)))
  {
    ret_value = true;
  }
  else
  {
    ret_value = false;
  }
  OS_Unuse(&geni_master);

  return ret_value;
}

/*****************************************************************************
 * Function - SetIO351ModuleConfig
 * DESCRIPTION: Configure the IO351
*****************************************************************************/
bool GeniSlaveIf::SetIO351ModuleConfig(IO351_NO_TYPE moduleNo, int noOfPumps, int noOfVfds, int addrOffset, int systemType)
{
  bool ret_value = false;

  U8 apdu_setup_io_modul[11] = {
                                  0x0A, //Length of the rest of buffer
                                  0x04, //Class
                                  0x88, //OPERATION + Length of APDU
                                  0x14, //NO_OF_PUMPS
                                  0x00, //Insert value here
                                  0x15, //NO_OF_VFDS
                                  0x00, //Insert value here
                                  0x16, //ADDR_OFFSET
                                  0x00, //Insert value here
                                  0x19, //SYSTEM TYPE
                                  0x00  //Insert value here
                                };

  apdu_setup_io_modul[4] = noOfPumps;
  apdu_setup_io_modul[6] = noOfVfds;
  apdu_setup_io_modul[8] = addrOffset;
  apdu_setup_io_modul[10] = systemType;

  OS_Use(&geni_master);
  if (SendDirAPDU(DUMMY_RESP_FNC, (U8*)apdu_setup_io_modul, GetUnitAddress(moduleNo)))
  {
    ret_value = true;
  }
  else
  {
    ret_value = false;
  }
  OS_Unuse(&geni_master);

  return ret_value;
}

/*****************************************************************************
 * Function - SetIO351AnalogInputConfig
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::SetIO351AnalogInputConfig(IO351_NO_TYPE moduleNo, IO351_ANA_IN_NO_TYPE anaInNo, SENSOR_ELECTRIC_TYPE type)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  auto bool ret_value = false;

  switch (type)
  {
	case SENSOR_ELECTRIC_TYPE_0_20mA:
	  if (anaInNo == IO351_ANA_IN_NO_1)
	  {
      if (SendDirAPDU(DUMMY_RESP_FNC, (U8*)apdu_set_ana_in_one_0_20, unit_addr))
	    {
	      ret_value = true;
	    }
	  }
	  else if (anaInNo == IO351_ANA_IN_NO_2)
	  {
      if (SendDirAPDU(DUMMY_RESP_FNC, (U8*)apdu_set_ana_in_two_0_20, unit_addr))
	    {
	      ret_value = true;
	    }
	  }
	  break;
	case SENSOR_ELECTRIC_TYPE_4_20mA:
	  if (anaInNo == IO351_ANA_IN_NO_1)
	  {
      if (SendDirAPDU(DUMMY_RESP_FNC, (U8*)apdu_set_ana_in_one_4_20, unit_addr))
	    {
	      ret_value = true;
	    }
	  }
	  else if (anaInNo == IO351_ANA_IN_NO_2)
	  {
      if (SendDirAPDU(DUMMY_RESP_FNC, (U8*)apdu_set_ana_in_two_4_20, unit_addr))
	    {
	      ret_value = true;
	    }
	  }
	  break;
	case SENSOR_ELECTRIC_TYPE_0_10V:
    if (anaInNo == IO351_ANA_IN_NO_1)
	  {
      if (SendDirAPDU(DUMMY_RESP_FNC, (U8*)apdu_set_ana_in_one_0_10, unit_addr))
	    {
	      ret_value = true;
	    }
	  }
	  else if (anaInNo == IO351_ANA_IN_NO_2)
	  {
      if (SendDirAPDU(DUMMY_RESP_FNC, (U8*)apdu_set_ana_in_two_0_10, unit_addr))
	    {
	      ret_value = true;
	    }
	  }
	  break;
	case SENSOR_ELECTRIC_DISABLED:
	  if (anaInNo == IO351_ANA_IN_NO_1)
	  {
      if (SendDirAPDU(DUMMY_RESP_FNC, (U8*)apdu_set_ana_in_one_not_active, unit_addr))
	    {
	      ret_value = true;
	    }
	  }
	  else if (anaInNo == IO351_ANA_IN_NO_2)
	  {
      if (SendDirAPDU(DUMMY_RESP_FNC, (U8*)apdu_set_ana_in_two_not_active, unit_addr))
	    {
	      ret_value = true;
	    }
	  }
	  break;
  }

  return ret_value;
}

/*****************************************************************************
 * Function - GetPowerOnCntIoModule
 * DESCRIPTION: Return power on counter from IO351
 *
*****************************************************************************/
bool GeniSlaveIf::GetIO351PowerOnCnt(IO351_NO_TYPE moduleNo, U16* pValue)
{
  bool ret_val = false;
  U8 unit_index;
  GENI_DEVICE_TYPE device;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if ((device == DEVICE_IO351_IO_MODULE) || (device == DEVICE_IO351_PUMP_MODULE))
    {
      OS_Use(&geni_class_data);
      *pValue = (U16)(s_cl2_id195[unit_index] << 8);
      *pValue |= (U16)s_cl2_id196[unit_index];
      OS_Unuse(&geni_class_data);

      if (*pValue != 0xFFFF)
      {
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO351AlarmCode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetIO351AlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue)
{
  U8 unit_index, geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if ((device == DEVICE_IO351_IO_MODULE) || (device == DEVICE_IO351_PUMP_MODULE))
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id156[unit_index];  //io module error is set in warnings
      OS_Unuse(&geni_class_data);

      if (geni_value != 255)
      {
        if (TEST_BIT_HIGH(geni_value, 0))
        {
          /* EEPROM error */
          *pValue = ALARM_ID_HARDWARE_FAULT_TYPE_2;
        }
        else
        {
          *pValue = ALARM_ID_NO_ALARM;
        }
      }
      else
      {
        *pValue = ALARM_ID_COMMUNICATION_FAULT;
      }

      ret_val = true;
    }
  }
  else
  {
    if (unit_index != NO_UNIT)
    {
      if (mUnitCommError[unit_index] == true)
      {
        *pValue = ALARM_ID_COMMUNICATION_FAULT;
        ret_val = true;
      }
    }
  }

  return ret_val;
}


/*****************************************************************************
 * Function - GetIO351DigitalInputCounter
 * DESCRIPTION: Returns counter value of digital input given by no.
 *              Returns true if state is valid.
 *              Return false if state is invalid due to wrong device, missing
 *              device or comm error to device.
*****************************************************************************/
bool GeniSlaveIf::GetIO351DigitalInputCounter(IO351_NO_TYPE moduleNo, IO351_DIG_IN_NO_TYPE digInNo, U32* pCounter)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  U8 unit_index;
  GENI_DEVICE_TYPE device;
  bool ret_value = false;

  *pCounter = 0;

  // get unit index
  OS_Use(&geni_master);
  unit_index = FindUnit(unit_addr);
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device
    if (device == DEVICE_IO351_IO_MODULE)
    {
      switch (digInNo)
      {
        case IO351_DIG_IN_NO_1:
          OS_Use(&geni_class_data);
          *pCounter = 0x0000001L*s_cl5_id010[unit_index] + 0x0000100L*s_cl5_id009[unit_index] +
                      0x0010000L*s_cl5_id008[unit_index] + 0x1000000L*s_cl5_id007[unit_index];
          OS_Unuse(&geni_class_data);
          ret_value = true;
          break;
        case IO351_DIG_IN_NO_2:
          OS_Use(&geni_class_data);
          *pCounter = 0x0000001L*s_cl5_id014[unit_index] + 0x0000100L*s_cl5_id013[unit_index] +
                      0x0010000L*s_cl5_id012[unit_index] + 0x1000000L*s_cl5_id011[unit_index];
          OS_Unuse(&geni_class_data);
          ret_value = true;
          break;
      }
    }
  }

  return ret_value;
}

/*****************************************************************************
 * Function - GetIO351DigitalInputStatus
 * DESCRIPTION: Returns state of digital input given by no.
 *              Returns true if state is valid.
 *              Return false if state is invalid due to wrong device, missing
 *              device or comm error to device.
*****************************************************************************/
bool GeniSlaveIf::GetIO351DigitalInputStatus(IO351_NO_TYPE moduleNo, U16* pStatus)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  U8 unit_index;
  GENI_DEVICE_TYPE device;
  bool ret_value = false;

  // get unit index
  OS_Use(&geni_master);
  unit_index = FindUnit(unit_addr);
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device
    if (device == DEVICE_IO351_IO_MODULE)
    {
      OS_Use(&geni_class_data);
      *pStatus = (U16)(s_cl2_id024[unit_index] | ((U16)s_cl2_id025[unit_index] << 8));
      OS_Unuse(&geni_class_data);

      if (*pStatus != 0xFFFF)
      {
        *pStatus &= 0x01FF; // 9 bits
        ret_value = true;
      }
    }
  }

  return ret_value;
}

/*****************************************************************************
 * Function - GetIO351PTCStatus
 * DESCRIPTION: Returns state of PTC input given by no.
 *              Returns true if state is valid.
 *              Return false if state is invalid due to wrong device, missing
 *              device or comm error to device.
*****************************************************************************/
bool GeniSlaveIf::GetIO351PTCStatus(IO351_NO_TYPE moduleNo, U8* pStatus)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  U8 unit_index;
  GENI_DEVICE_TYPE device;
  bool ret_value = false;

  // get unit index
  OS_Use(&geni_master);
  unit_index = FindUnit(unit_addr);
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device
    if (device == DEVICE_IO351_IO_MODULE)
    {
      OS_Use(&geni_class_data);
      *pStatus = s_cl2_id025[unit_index];
      OS_Unuse(&geni_class_data);

      *pStatus &= 0x7E; // bit 7 and 0 are not PTC data
      ret_value = true;
    }
  }

  return ret_value;
}

/*****************************************************************************
 * Function - GetIO351DigitalOutputStatus
 * DESCRIPTION: Gets relay status
 *              Returns true if relay status is valid.
 *              Return false if relay status is invalid due to wrong device, missing
 *              device or comm error to device.
*****************************************************************************/
bool GeniSlaveIf::GetIO351DigitalOutputStatus(IO351_NO_TYPE moduleNo, U8* pStatus)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  U8 unit_index;
  GENI_DEVICE_TYPE device;
  bool ret_value = false;

  // get unit index
  OS_Use(&geni_master);
  unit_index = FindUnit(unit_addr);
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device
    if ((device == DEVICE_IO351_IO_MODULE) || (device == DEVICE_IO351_PUMP_MODULE))
    {
      OS_Use(&geni_class_data);
      *pStatus = s_cl2_id026[unit_index];
      OS_Unuse(&geni_class_data);

      if (*pStatus != 0xFF)
      {
        *pStatus &= 0x7F; // 7 bits
        ret_value = true;
      }
    }
  }

  return ret_value;
}

/*****************************************************************************
 * Function - SetIO351DigitalOutputStatus
 * DESCRIPTION: Sets Relay identified by no to state given as input.
 *              Returns true if relay is valid.
 *              Return false if relay is invalid due to wrong device, missing
 *              device or comm error to device.
*****************************************************************************/
bool GeniSlaveIf::SetIO351DigitalOutputStatus(IO351_NO_TYPE moduleNo, IO351_DIG_OUT_NO_TYPE digOutNo, bool status)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  U8 unit_index;
  GENI_DEVICE_TYPE device;
  bool ret_value = false;
  U8 apdu_set_relay[4] = {
                         0x03,//Length of the rest of buffer
                         0x03,//Class
                         0x81,//OPERATION + Length of APDU
                         0x00 //Put ID here
                         };

  OS_Use(&geni_master);
  unit_index = FindUnit(unit_addr);
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO351_IO_MODULE)
    {
      if (status)
      {
        // Set relay using ON command (100-106)
        apdu_set_relay[3] = (U8)(100 + digOutNo);
      }
      else
      {
        // Set relay using OFF command (107-113)
        apdu_set_relay[3] = (U8)(107 + digOutNo);
      }

      if (SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_set_relay, unit_addr))
      {
        ret_value = true;
      }
    }
  }
  return ret_value;
}


/*****************************************************************************
 * Function - GetIO351AnalogInputValue
 * DESCRIPTION: Returns value of analog input in [%] given by no [0;3].
 *              Returns true if value is valid.
 *              Return false if value is invalid due to wrong device, missing
 *              device or comm error to device.
*****************************************************************************/
bool GeniSlaveIf::GetIO351AnalogInputValue(IO351_NO_TYPE moduleNo, IO351_ANA_IN_NO_TYPE anaInNo, float* pValue)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  U8 unit_index;
  GENI_DEVICE_TYPE device;
  bool ret_value = false;

  // init
  *pValue = 0.0f;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(unit_addr);
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO351_IO_MODULE)
    {
      switch (anaInNo)
      {
        case IO351_ANA_IN_NO_1:
          OS_Use(&geni_class_data);
          *pValue = (float)((U16)s_cl2_id021[unit_index] | ((U16)s_cl2_id020[unit_index]<<8))*GENI_ANA_IN_SCALE;
          OS_Unuse(&geni_class_data);
          ret_value = true;
          break;
        case IO351_ANA_IN_NO_2:
          OS_Use(&geni_class_data);
          *pValue = (float)((U16)s_cl2_id023[unit_index] | ((U16)s_cl2_id022[unit_index]<<8))*GENI_ANA_IN_SCALE;
          OS_Unuse(&geni_class_data);
          ret_value = true;
          break;
      }
    }
  }

  if (*pValue > 100.0f)
  {
    ret_value = false;
  }

  return ret_value;
}

/*****************************************************************************
 * Function - SetIO351AnalogOutput
 * DESCRIPTION:
 *
*****************************************************************************/
bool GeniSlaveIf::SetIO351AnalogOutput(IO351_NO_TYPE moduleNo, IO351_ANA_OUT_NO_TYPE anaOutNo, float outValue)
{
  const U8 unit_addr = GetUnitAddress(moduleNo);
  U8 unit_index;
  GENI_DEVICE_TYPE device;
  bool ret_value = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(unit_addr);
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_IO351_IO_MODULE)
    {
      switch (anaOutNo)
      {
        case IO351_ANA_OUT_NO_1:
          OS_Use(&geni_class_data);
          s_cl5_id001[unit_index] = (U8)((outValue*GENI_ANA_OUT_SCALE)+0.5f);
          OS_Unuse(&geni_class_data);
          ret_value = true;
          break;
        case IO351_ANA_OUT_NO_2:
          OS_Use(&geni_class_data);
          s_cl5_id002[unit_index] = (U8)((outValue*GENI_ANA_OUT_SCALE)+0.5f);
          OS_Unuse(&geni_class_data);
          ret_value = true;
          break;
        case IO351_ANA_OUT_NO_3:
          OS_Use(&geni_class_data);
          s_cl5_id003[unit_index] = (U8)((outValue*GENI_ANA_OUT_SCALE)+0.5f);
          OS_Unuse(&geni_class_data);
          ret_value = true;
          break;
      }
    }
  }

  return ret_value;
}

/*****************************************************************************
 *
 * End of IO 351 functions
 *
*****************************************************************************/


/*****************************************************************************
 *
 * DDA functions
 *
*****************************************************************************/

/*****************************************************************************
 * Function - ConnectDDA
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::ConnectDDA(IO351_NO_TYPE moduleNo)
{
  return (ConnectDevice(moduleNo, DEVICE_DDA) != NO_UNIT);
}

/*****************************************************************************
 * Function - DisconnectDDA
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DisconnectDDA(IO351_NO_TYPE moduleNo)
{
  DisconnectDevice(moduleNo, DEVICE_DDA);
}

/*****************************************************************************
 * Function - DDAReset
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDAReset(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(NO_RESP_FNC, (U8 *)apdu_reset, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - DDAAlarmReset
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDAAlarmReset(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_alarm_reset, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - GetDDA pressure_max
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDAMaxPressure(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U8 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = (U8)s_cl2_id000[unit_index];     // Maximum possible pressure in dosing
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF)
      {
        *pValue = 0.1f*geni_value;
        ret_val = true;
      }
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Maximum possible dosing capacity
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDAMaxDosingCap(IO351_NO_TYPE moduleNo, U32* pValue)
{
  U8 unit_index;
  U32 geni_value = 0;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value |= (U32)s_cl2_id001[unit_index]<<16;     // Maximum possible pressure in dosing
      geni_value |= (U32)s_cl2_id002[unit_index]<<8;
      geni_value |= (U32)s_cl2_id003[unit_index];
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF0000)
      {
        *pValue = geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA flow monitor dosing capacity
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDADosingCapacity(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U32 geni_value = 0;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value |= (U32)s_cl2_id012[unit_index]<<24;     // Maximum possible pressure in dosing
      geni_value |= (U32)s_cl2_id013[unit_index]<<16;    
      geni_value |= (U32)s_cl2_id014[unit_index]<<8;
      geni_value |= (U32)s_cl2_id015[unit_index];      
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF000000)
      {
        *pValue = 1.0f * geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Actual pressure in dosing head
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDAActPresssure(IO351_NO_TYPE moduleNo, float* pValue)
{
  U8 unit_index;
  U8 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = (U8)s_cl2_id016[unit_index];     // Maximum possible pressure in dosing
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF)
      {
        *pValue = 1.0f*geni_value;
        ret_val = true;
      }
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA total dosing volume
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDATotalVolume(IO351_NO_TYPE moduleNo, U32* pValue)
{
  U8 unit_index;
  U32 geni_value = 0;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value |= (U32)s_cl2_id071[unit_index]<<24;     // Maximum possible pressure in dosing
      geni_value |= (U32)s_cl2_id072[unit_index]<<16;    
      geni_value |= (U32)s_cl2_id073[unit_index]<<8;
      geni_value |= (U32)s_cl2_id074[unit_index];      
      OS_Unuse(&geni_class_data);
      if (geni_value < 0xFF000000)
      {
        *pValue = geni_value;
        ret_val = true;
      }
    }
  }

  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Pump system mode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDASystemMode(IO351_NO_TYPE moduleNo, U8* pStatus)
{
  U8 unit_index;
  U8 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = (U8)s_cl2_id080[unit_index];     
      OS_Unuse(&geni_class_data);
      *pStatus = geni_value;
      ret_val = true;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Pump Actual Operating mode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDAOperatingMode(IO351_NO_TYPE moduleNo, U8* pStatus)
{
  U8 unit_index;
  U8 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = (U8)s_cl2_id081[unit_index];     
      OS_Unuse(&geni_class_data);
      *pStatus = geni_value;
      ret_val = true;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Pump Actual Control mode
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDAControlMode(IO351_NO_TYPE moduleNo, U8* pStatus)
{
  U8 unit_index;
  U8 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = (U8)s_cl2_id082[unit_index];     
      OS_Unuse(&geni_class_data);
      *pStatus = geni_value;
      ret_val = true;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Pump Start/Stop state of the Operating mode control sources
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDAStopCtrState(IO351_NO_TYPE moduleNo, U8* pStatus)
{
  U8 unit_index;
  U8 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = (U8)s_cl2_id085[unit_index];     
      OS_Unuse(&geni_class_data);
      *pStatus = geni_value;
      ret_val = true;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Pump control source currently active.
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDACtrSource(IO351_NO_TYPE moduleNo, U8* pStatus)
{
  U8 unit_index;
  U8 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = (U8)s_cl2_id086[unit_index];     
      OS_Unuse(&geni_class_data);
      *pStatus = geni_value;
      ret_val = true;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Pump running or not
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDAPumpingState(IO351_NO_TYPE moduleNo, bool* pStatus)
{
  U8 unit_index;
  U8 geni_value;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

    // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = (U8)s_cl2_id087[unit_index];     
      OS_Unuse(&geni_class_data);
      
      *pStatus = geni_value & 0x01;
      ret_val = true;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Pump Alarm code
 * DESCRIPTION:
*****************************************************************************/
/*bool GeniSlaveIf::GetDDAAlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue)*/
//{
  //U8 unit_index, geni_value, unit_family;
  //GENI_DEVICE_TYPE device;
  //bool ret_val = false;

  //// find unit
  //OS_Use(&geni_master);
  //unit_index = FindUnit(GetUnitAddress(moduleNo));
  //OS_Unuse(&geni_master);

  //// get device
  //if (GetDevice(unit_index, &device))
  //{
    //// verify device type
    //if (device == DEVICE_DDA)
    //{
      //OS_Use(&geni_class_data);
      //geni_value = s_cl2_id234[unit_index];     
      //unit_family = s_cl2_id148[unit_index];
      //OS_Unuse(&geni_class_data);
      //if (unit_family == UNIT_FAMILY_DDA) // Yes, it's a DDA
      //{
        //*pValue = (ALARM_ID_TYPE)(geni_value);
        //ret_val = true;
      //}
    //}
  //}
  //return ret_val;
/*}*/
bool GeniSlaveIf::GetDDAAlarmCode(IO351_NO_TYPE moduleNo, ALARM_ID_TYPE* pValue)
{
  U8 unit_index, geni_value, unit_family;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id234[unit_index];     
      OS_Unuse(&geni_class_data);
      *pValue = (ALARM_ID_TYPE)(geni_value);
      ret_val = true;
    }
  }
  else
  {
    if (unit_index != NO_UNIT)
    {
      if (mUnitCommError[unit_index] == true)
      {
        *pValue = ALARM_ID_COMMUNICATION_FAULT;
        ret_val = true;
      }
    }
  }

  return ret_val;
}

/*****************************************************************************
 * Function - GetDDA Pump Warning code
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::GetDDAWarningCode(IO351_NO_TYPE moduleNo, U32* pValue)
{
  U8 unit_index, geni_value, unit_family;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      geni_value = s_cl2_id235[unit_index];     
      unit_family = s_cl2_id148[unit_index];
      OS_Unuse(&geni_class_data);
      if (unit_family == UNIT_FAMILY_DDA) // Yes, it's a DDA
      {
        *pValue = (U32)(geni_value);
        ret_val = true;
      }
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - SetDDA Pump Dosing setpoint
 * DESCRIPTION:
*****************************************************************************/
bool GeniSlaveIf::SetDDAReference(IO351_NO_TYPE moduleNo, U32 pValue)
{
  U8 unit_index;
  //U32 geni_value = 0;
  GENI_DEVICE_TYPE device;
  bool ret_val = false;

  // find unit
  OS_Use(&geni_master);
  unit_index = FindUnit(GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);

  // get device
  if (GetDevice(unit_index, &device))
  {
    // verify device type
    if (device == DEVICE_DDA)
    {
      OS_Use(&geni_class_data);
      s_cl5_id001[unit_index] = (U8)(pValue >>24) ;
      s_cl5_id002[unit_index] = (U8)((pValue & 0x00FFFFFF) >> 16);
      s_cl5_id003[unit_index] = (U8)((pValue & 0x0000FFFF) >> 8);
      s_cl5_id004[unit_index] = (U8)(pValue & 0x000000FF);      
      OS_Unuse(&geni_class_data);
    }
  }

  return ret_val;
}

/*****************************************************************************
 * Function - DDA Requested Stop
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDARequestStop(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_stop, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - DDA Requested Start
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDARequestStart(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_start, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - DDA Set to User Mode
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDASetToUserMode(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_DDA_UseMode, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - DDA Set to Manual Dosing Mode
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDASetToManualDosing(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_DDA_ManualDosing, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - DDA Set to Analogue Dosing Mode
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDASetToAnalogueDosing(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_DDA_AnalogueDosing, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - DDA Set to Pulse Dosing Mode
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDASetToPulseDosing(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_DDA_PulseDosing, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - DDA Press Start Key
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDAPressStartKey(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_DDA_PressStartKey, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 * Function - DDA Press Stop Key
 * DESCRIPTION:
*****************************************************************************/
void GeniSlaveIf::DDAPressStopKey(IO351_NO_TYPE moduleNo)
{
  OS_Use(&geni_master);
  SendDirAPDU(DUMMY_RESP_FNC, (U8 *)apdu_DDA_PressStopKey, GetUnitAddress(moduleNo));
  OS_Unuse(&geni_master);
}

/*****************************************************************************
 *
 * End of DDA functions
 *
*****************************************************************************/

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
*****************************************************************************/
GeniSlaveIf * GeniSlaveIf::GetInstance(void)
{
  if (!mInstance)
  {
    mInstance = new GeniSlaveIf();
  }
  return mInstance;
}

/*****************************************************************************
 * Function IncomingSlaveInfo
 * DESCRIPTION: Channels info data received at GENI RS232 to the specified unit.
 *
 *****************************************************************************/
void GeniSlaveIf::IncomingSlaveInfo(void)
{
  U8  info_head;
  int buf_index;

  /* Check for failed - if so length of telegram is set to 0 */
  OS_Use(&geni_class_data);
  if ( RS232_rx_buf[iLN] != 0 )
  {
    /* A valid answer has been received - place it at the correct unit */
    buf_index = 6; // First info head in rx buffer

    info_head = RS232_rx_buf[buf_index++];
    SetSlaveInfo( &(s_cl2_id027_info[mInfoUnitIndex][GENI_INFO_HEAD]), info_head );
    if ( (info_head & GENI_INFO_SIF_SCALED_VALUE) == GENI_INFO_SIF_SCALED_VALUE )
    {
      SetSlaveInfo( &(s_cl2_id027_info[mInfoUnitIndex][GENI_INFO_UNIT]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id027_info[mInfoUnitIndex][GENI_INFO_ZERO]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id027_info[mInfoUnitIndex][GENI_INFO_RANGE]), RS232_rx_buf[buf_index++] );
    }
    info_head = RS232_rx_buf[buf_index++];
    SetSlaveInfo( &(s_cl2_id030_info[mInfoUnitIndex][GENI_INFO_HEAD]), info_head );
    if ( (info_head & GENI_INFO_SIF_SCALED_VALUE) == GENI_INFO_SIF_SCALED_VALUE )
    {
      SetSlaveInfo( &(s_cl2_id030_info[mInfoUnitIndex][GENI_INFO_UNIT]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id030_info[mInfoUnitIndex][GENI_INFO_ZERO]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id030_info[mInfoUnitIndex][GENI_INFO_RANGE]), RS232_rx_buf[buf_index++] );
    }
    info_head = RS232_rx_buf[buf_index++];
    SetSlaveInfo( &(s_cl2_id032_info[mInfoUnitIndex][GENI_INFO_HEAD]), info_head );
    if ( (info_head & GENI_INFO_SIF_SCALED_VALUE) == GENI_INFO_SIF_SCALED_VALUE )
    {
      SetSlaveInfo( &(s_cl2_id032_info[mInfoUnitIndex][GENI_INFO_UNIT]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id032_info[mInfoUnitIndex][GENI_INFO_ZERO]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id032_info[mInfoUnitIndex][GENI_INFO_RANGE]), RS232_rx_buf[buf_index++] );
    }
    info_head = RS232_rx_buf[buf_index++];
    SetSlaveInfo( &(s_cl2_id033_info[mInfoUnitIndex][GENI_INFO_HEAD]), info_head );
    if ( (info_head & GENI_INFO_SIF_SCALED_VALUE) == GENI_INFO_SIF_SCALED_VALUE )
    {
      SetSlaveInfo( &(s_cl2_id033_info[mInfoUnitIndex][GENI_INFO_UNIT]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id033_info[mInfoUnitIndex][GENI_INFO_ZERO]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id033_info[mInfoUnitIndex][GENI_INFO_RANGE]), RS232_rx_buf[buf_index++] );
    }
    info_head = RS232_rx_buf[buf_index++];
    SetSlaveInfo( &(s_cl2_id035_info[mInfoUnitIndex][GENI_INFO_HEAD]), info_head );
    if ( (info_head & GENI_INFO_SIF_SCALED_VALUE) == GENI_INFO_SIF_SCALED_VALUE )
    {
      SetSlaveInfo( &(s_cl2_id035_info[mInfoUnitIndex][GENI_INFO_UNIT]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id035_info[mInfoUnitIndex][GENI_INFO_ZERO]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id035_info[mInfoUnitIndex][GENI_INFO_RANGE]), RS232_rx_buf[buf_index++] );
    }
    info_head = RS232_rx_buf[buf_index++];
    SetSlaveInfo( &(s_cl2_id045_info[mInfoUnitIndex][GENI_INFO_HEAD]), info_head );
    if ( (info_head & GENI_INFO_SIF_SCALED_VALUE) == GENI_INFO_SIF_SCALED_VALUE )
    {
      SetSlaveInfo( &(s_cl2_id045_info[mInfoUnitIndex][GENI_INFO_UNIT]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id045_info[mInfoUnitIndex][GENI_INFO_ZERO]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id045_info[mInfoUnitIndex][GENI_INFO_RANGE]), RS232_rx_buf[buf_index++] );
    }
    info_head = RS232_rx_buf[buf_index++];
    SetSlaveInfo( &(s_cl2_id086_info[mInfoUnitIndex][GENI_INFO_HEAD]), info_head );
    if ( (info_head & GENI_INFO_SIF_SCALED_VALUE) == GENI_INFO_SIF_SCALED_VALUE )
    {
      SetSlaveInfo( &(s_cl2_id086_info[mInfoUnitIndex][GENI_INFO_UNIT]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id086_info[mInfoUnitIndex][GENI_INFO_ZERO]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id086_info[mInfoUnitIndex][GENI_INFO_RANGE]), RS232_rx_buf[buf_index++] );
    }
    info_head = RS232_rx_buf[buf_index++];
    SetSlaveInfo( &(s_cl2_id151_info[mInfoUnitIndex][GENI_INFO_HEAD]), info_head );
    if ( (info_head & GENI_INFO_SIF_SCALED_VALUE) == GENI_INFO_SIF_SCALED_VALUE )
    {
      SetSlaveInfo( &(s_cl2_id151_info[mInfoUnitIndex][GENI_INFO_UNIT]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id151_info[mInfoUnitIndex][GENI_INFO_ZERO]), RS232_rx_buf[buf_index++] );
      SetSlaveInfo( &(s_cl2_id151_info[mInfoUnitIndex][GENI_INFO_RANGE]), RS232_rx_buf[buf_index++] );
    }
    mUnitInfoReceived[mInfoUnitIndex] = true;
  }
  else
  {
    /* Try again */
    mUnitInfoReceived[mInfoUnitIndex] = false;
  }
  OS_Unuse(&geni_class_data);
  mInfoRequestSent = false;
  CheckForInfoRequest();
}

/*****************************************************************************
 * Function ChangeSlaveGeniAddress
 * DESCRIPTION:
 * Changes the address of the target unit. No validation is performed.
 *
 * Returns [true] if the set addr telegram where queued ok.
 *****************************************************************************/
bool GeniSlaveIf::ChangeSlaveGeniAddress(unsigned char target_address, unsigned char new_address)
{
  bool dir_tlg_sent = false;

  apdu_set_unit_addr[APDU_UNIT_ADDR_INDEX] = new_address;

  OS_Use(&geni_master);
  dir_tlg_sent = SendDirAPDU( DUMMY_RESP_FNC, (U8 *)apdu_set_unit_addr, target_address );
  OS_Unuse(&geni_master);

  return dir_tlg_sent;
}


/*****************************************************************************
 * Function IncomingIO351ModuleConfig
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniSlaveIf::IncomingIO351ModuleConfig(void)
{
  IO351_NO_TYPE module_no = (IO351_NO_TYPE)-1;
  U8 no_of_pumps = 0, no_of_vfds = 0, addr_offset = 0, module_type = 0;
  bool rx_ok_flag;

  OS_Use(&geni_class_data);

  // grab module no via destination address in tx tgm
  switch (RS232_tx_buf[iDA])
  {
  case IO351_IOM_ADDR_OFFSET + 0:
    module_no = IO351_IOM_NO_1;
    break;
  case IO351_IOM_ADDR_OFFSET + 1:
    module_no = IO351_IOM_NO_2;
    break;
  case IO351_IOM_ADDR_OFFSET + 2:
    module_no = IO351_IOM_NO_3;
    break;
  }

  // check for failed - if so length of telegram is set to 0
  if (RS232_rx_buf[iLN] != 0)
  {
    rx_ok_flag         = true;
    no_of_pumps        = RS232_rx_buf[6];
    no_of_vfds         = RS232_rx_buf[7];
    addr_offset        = RS232_rx_buf[8];
    module_type        = RS232_rx_buf[9];
  }
  else
  {
    rx_ok_flag = false;
  }

  OS_Unuse(&geni_class_data);

  if (module_no == IO351_IOM_NO_1)
  {
    if (mpIO351IOModules[0])
    {
      mpIO351IOModules[0]->ConfigReceived(rx_ok_flag, no_of_pumps, no_of_vfds, addr_offset, module_type);
    }
  }
  else if (module_no == IO351_IOM_NO_2)
  {
    if (mpIO351IOModules[1])
    {
      mpIO351IOModules[1]->ConfigReceived(rx_ok_flag, no_of_pumps, no_of_vfds, addr_offset, module_type);
    }
  }
  else if (module_no == IO351_IOM_NO_3)
  {
    if (mpIO351IOModules[2])
    {
      mpIO351IOModules[2]->ConfigReceived(rx_ok_flag, no_of_pumps, no_of_vfds, addr_offset, module_type);
    }
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
 * Function - Constructor
 * DESCRIPTION: Initialisation of private member data
 *
 *****************************************************************************/
GeniSlaveIf::GeniSlaveIf()
{
  int i;

  for (i = 0; i < MAX_NUMBER_OF_UNITS; i++)
  {
    mUnitCommError[i] = false;
    mUnitInfoWanted[i] = false;
    mUnitInfoReceived[i] = false;
    mUnit2Device[i] = NO_DEVICE;
    mUnitConfigPending[i] = false;
  }

  mInfoRequestSent = false;
  mNextInfoToRequest = 0;
  mInfoUnitIndex = 0;

  for (i = 0; i < sizeof(mpIO351IOModules) / sizeof(mpIO351IOModules[0]); i++)
  {
    mpIO351IOModules[i] = NULL;
  }

  InitSlaveData();
  InitSlaveInfo();
//XHE  InitDeviceConfiguration();
}

/*****************************************************************************
 * Function - SetComError
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniSlaveIf::SetGeniError(unsigned char nw_index, unsigned char fault_state)
{
  GENI_DEVICE_TYPE device;

  device = NO_DEVICE;

  if (fault_state) //Make slave data NA
  {
    // Get device before comm error is set true as NO_DEVICE is returned for
    // a device with comm error
    GetDevice(nw_index, &device);
    mUnitCommError[nw_index] = true;
    //Set alarm_code. So far all devices (DEVICE_PUMP and DEVICE_IO351_IO_MODULE)
    //contains alarm_code at id158
    // Clear the info (if any) for E- and S-pumps - new info will be requested
    // when communication error disappears.
    if (device == DEVICE_E_PUMP)
    {
      SetSlaveInfo( &(s_cl2_id027_info[nw_index][GENI_INFO_HEAD]), NO_INFO );
      SetSlaveInfo( &(s_cl2_id030_info[nw_index][GENI_INFO_HEAD]), NO_INFO );
      SetSlaveInfo( &(s_cl2_id032_info[nw_index][GENI_INFO_HEAD]), NO_INFO );
      SetSlaveInfo( &(s_cl2_id033_info[nw_index][GENI_INFO_HEAD]), NO_INFO );
      SetSlaveInfo( &(s_cl2_id035_info[nw_index][GENI_INFO_HEAD]), NO_INFO );
      SetSlaveInfo( &(s_cl2_id045_info[nw_index][GENI_INFO_HEAD]), NO_INFO );
      SetSlaveInfo( &(s_cl2_id086_info[nw_index][GENI_INFO_HEAD]), NO_INFO );
      SetSlaveInfo( &(s_cl2_id151_info[nw_index][GENI_INFO_HEAD]), NO_INFO );

      // Disable auto poll for error pumps to avoid genibus delays for ok units
      network_list[nw_index].device_type = NO_DEVICE;
    }
  }
  else
  {
    mUnitCommError[nw_index] = false;
    // Get device after comm error is set false as NO_DEVICE is returned for
    // a device with comm error
    GetDevice(nw_index, &device);

    // Only request info for E-pump
    if (device == DEVICE_E_PUMP)
    {
      mUnitInfoWanted[nw_index] = true;
    }
    mUnitInfoReceived[nw_index] = false;

    CheckForInfoRequest();
  }


}

/*****************************************************************************
 * Function InitSlaveData
 * DESCRIPTION: Sets all class 2 data N/A
 *
 *****************************************************************************/
void GeniSlaveIf::InitSlaveData()
{
  SetSlaveData(s_cl2_id020, 0); // Initialised to 0 because of ana_in in IO module
  SetSlaveData(s_cl2_id021, 0); // Initialised to 0 because of ana_in in IO module
  SetSlaveData(s_cl2_id022, 0); // Initialised to 0 because of ana_in in IO module
  SetSlaveData(s_cl2_id023, 0); // Initialised to 0 because of ana_in in IO module
  SetSlaveData(s_cl2_id024, 0); // Initialised to 0 because of dig_in in IO module
  SetSlaveData(s_cl2_id025, 0); // Initialised to 0 because of dig_in in IO module
  SetSlaveData(s_cl2_id026, 255);
  SetSlaveData(s_cl2_id027, 255);
  SetSlaveData(s_cl2_id029, 255);
  SetSlaveData(s_cl2_id030, 255);
  SetSlaveData(s_cl2_id032, 255);
  SetSlaveData(s_cl2_id033, 255);
  SetSlaveData(s_cl2_id034, 255);
  SetSlaveData(s_cl2_id035, 255);
  SetSlaveData(s_cl2_id036, 255);
  SetSlaveData(s_cl2_id043, 255);
  SetSlaveData(s_cl2_id044, 255);
  SetSlaveData(s_cl2_id045, 255);
  SetSlaveData(s_cl2_id046, 255);
  SetSlaveData(s_cl2_id047, 255);
  SetSlaveData(s_cl2_id048, 255);
  SetSlaveData(s_cl2_id049, 255);
  SetSlaveData(s_cl2_id050, 255);
  SetSlaveData(s_cl2_id060, 255);
  SetSlaveData(s_cl2_id064, 255);
  SetSlaveData(s_cl2_id065, 255);
  SetSlaveData(s_cl2_id066, 255);
  SetSlaveData(s_cl2_id067, 255);
  SetSlaveData(s_cl2_id068, 255);
  SetSlaveData(s_cl2_id069, 255);
  SetSlaveData(s_cl2_id070, 255);
  SetSlaveData(s_cl2_id071, 255);
  SetSlaveData(s_cl2_id072, 255);
  SetSlaveData(s_cl2_id081, 255);
  SetSlaveData(s_cl2_id083, 255);
  SetSlaveData(s_cl2_id086, 255);
  SetSlaveData(s_cl2_id094, 255);
  SetSlaveData(s_cl2_id095, 255);
  SetSlaveData(s_cl2_id099, 255);
  SetSlaveData(s_cl2_id141, 255);
  SetSlaveData(s_cl2_id142, 255);
  SetSlaveData(s_cl2_id144, 255);
  SetSlaveData(s_cl2_id145, 255);
  SetSlaveData(s_cl2_id146, 255);
  SetSlaveData(s_cl2_id147, 255);
  SetSlaveData(s_cl2_id148, 255);
  SetSlaveData(s_cl2_id151, 255);
  SetSlaveData(s_cl2_id152, 255);
  SetSlaveData(s_cl2_id153, 255);
  SetSlaveData(s_cl2_id156, 255);
  SetSlaveData(s_cl2_id158, 255);
  SetSlaveData(s_cl2_id195, 255);
  SetSlaveData(s_cl2_id196, 255);

  SetSlaveData(s_cl4_id030, 255);
}

/*****************************************************************************
 * Function SetSlaveData
 * DESCRIPTION: Fill slave id array with value.
 *
 *****************************************************************************/
void GeniSlaveIf::SetSlaveData(U8* id, U8 value)
{
  int i;

  for (i=0; i<MAX_NUMBER_OF_UNITS; i++)
  {
    id[i] = value;
  }
}

/*****************************************************************************
 * Function InitSlaveInfo
 * DESCRIPTION: Sets all class 2 info to NO_INFO
 *
 *****************************************************************************/
void GeniSlaveIf::InitSlaveInfo()
{
  int i;

  for (i=0; i<MAX_NUMBER_OF_UNITS; i++)
  {
    SetSlaveInfo( &(s_cl2_id027_info[i][GENI_INFO_HEAD]), NO_INFO );
    SetSlaveInfo( &(s_cl2_id030_info[i][GENI_INFO_HEAD]), NO_INFO );
    SetSlaveInfo( &(s_cl2_id032_info[i][GENI_INFO_HEAD]), NO_INFO );
    SetSlaveInfo( &(s_cl2_id033_info[i][GENI_INFO_HEAD]), NO_INFO );
    SetSlaveInfo( &(s_cl2_id035_info[i][GENI_INFO_HEAD]), NO_INFO );
    SetSlaveInfo( &(s_cl2_id045_info[i][GENI_INFO_HEAD]), NO_INFO );
    SetSlaveInfo( &(s_cl2_id086_info[i][GENI_INFO_HEAD]), NO_INFO );
    SetSlaveInfo( &(s_cl2_id151_info[i][GENI_INFO_HEAD]), NO_INFO );
  }
}

/*****************************************************************************
 * Function SetSlaveInfo
 * DESCRIPTION: Sets the specified slave info to value.
 *
 *****************************************************************************/
void GeniSlaveIf::SetSlaveInfo(U8* id, U8 value)
{
  *id = value;
}

/*****************************************************************************
 * Function Scale8BitGeniValue
 * DESCRIPTION: Uses the geni_values info to scale the value to SI.
 *
 *****************************************************************************/
bool GeniSlaveIf::Scale8BitGeniValue(U8 geni_value, U8* info, float* val)
{
  U8 info_head, info_unit, info_range, full_scale;
  float scale_factor;
  float temp;
  int info_zero;
  bool ret_val = false;

  info_head = *info;
  info++;
  info_unit = *info;
  info++;
  info_zero = (int)(*info);
  info++;
  info_range = *info;
  info++;

  if ( (info_head & GENI_INFO_VI_MASK) == GENI_INFO_VI_0_255 )
  {
    full_scale = 255;
  }
  else
  {
    full_scale = 254;
    if ( geni_value == 255 )
    {
      /* The geni_value is illegal - set info_unit to 255 to prevent
       * scaling */
      info_unit = 255;
    }
  }

  if ( (info_unit & GENI_INFO_SZ_MASK) == GENI_INFO_SZ_NEGATIVE )
  {
    info_zero = -info_zero;
  }

  if ( ( (info_head & GENI_INFO_SIF_MASK) == GENI_INFO_SIF_SCALED_VALUE ) )
  {
    if ( !GetScaleFactor(info_unit, &scale_factor) )
    {
      /* Info unit is not usable - set info_unit to invalid */
      info_unit = 255;
    }
    if ( info_unit != 255 )
    {
      /* Info is ready - scale the value */
      temp = ( ( geni_value * (float)(info_range) / full_scale ) + info_zero) * scale_factor;
      /* If geni_value is a temperature it might still be in Celsius or Fahrenheit -
       * it must be konverted to Kelvin */
      switch ( (info_unit & (~GENI_INFO_SZ_MASK)) )
      {
        case GENI_UNIT_DOT1C :
        case GENI_UNIT_1C :
          temp += 273.15f;
          break;
        case GENI_UNIT_1F :
          temp = (temp + 459.67f) / 1.8f;
          break;
        default:
          break;
      }
      *val = temp;
      ret_val = true;
    }
  }
  return ret_val;
}




/*****************************************************************************
 * Function Scale16BitGeniValue
 * DESCRIPTION: Uses the geni_values info to scale the value to SI.
 *
 *****************************************************************************/
bool GeniSlaveIf::Scale16BitGeniValue(U16 geni_value, U8* info, float* val)
{
  U8 info_head, info_unit, info_range;
  U16 full_scale;
  float scale_factor;
  int info_zero;
  bool ret_val = false;

  info_head = *info;
  info++;
  info_unit = *info;
  info++;
  info_zero = (int)(*info);
  info++;
  info_range = *info;
  info++;

  if ( (info_head & GENI_INFO_VI_MASK) == GENI_INFO_VI_0_255 )
  {
    full_scale = ((255*256) + 255);
  }
  else
  {
    full_scale = (254*256);
    if ( geni_value > (254*256) )
    {
      /* The geni_value is illegal - set info_unit to 255 to prevent
       * scaling */
      info_unit = 255;
    }
  }

  if ( (info_unit & GENI_INFO_SZ_MASK) == GENI_INFO_SZ_NEGATIVE )
  {
    info_zero = -info_zero;
  }

  if ( ( (info_head & GENI_INFO_SIF_MASK) == GENI_INFO_SIF_SCALED_VALUE ) )
  {
    if ( !GetScaleFactor(info_unit, &scale_factor) )
    {
      /* Info unit is not usable - set info_unit to invalid */
      info_unit = 255;
    }
    if ( info_unit != 255 )
    {
      /* Info is ready - scale the value */
      *val = ( ( geni_value * (float)(info_range) / full_scale ) + info_zero) * scale_factor;
      ret_val = true;
    }
  }
  return ret_val;
}



/*****************************************************************************
 * Function Scale24BitGeniValue
 * DESCRIPTION: Uses the geni_values info to scale the value to SI.
 *
 *****************************************************************************/
bool GeniSlaveIf::Scale24BitGeniValue(U32 geni_value, U8* info, float* val)
{
  U8 info_head, info_unit, info_range;
  U32 full_scale;
  float scale_factor;
  int info_zero;
  bool ret_val = false;

  info_head = *info;
  info++;
  info_unit = *info;
  info++;
  info_zero = (int)(*info);
  info++;
  info_range = *info;
  info++;

  if ( (info_head & GENI_INFO_VI_MASK) == GENI_INFO_VI_0_255 )
  {
    full_scale = ((255*256*256) + (255*256) + 255);
  }
  else
  {
    full_scale = (254*256*256);
    if ( geni_value > (254*256*256) )
    {
      /* The geni_value is illegal - set info_unit to 255 to prevent
       * scaling */
      info_unit = 255;
    }
  }

  if ( (info_unit & GENI_INFO_SZ_MASK) == GENI_INFO_SZ_NEGATIVE )
  {
    info_zero = -info_zero;
  }

  if ( ( (info_head & GENI_INFO_SIF_MASK) == GENI_INFO_SIF_SCALED_VALUE ) )
  {
    if ( !GetScaleFactor(info_unit, &scale_factor) )
    {
      /* Info unit is not usable - set info_unit to invalid */
      info_unit = 255;
    }
    if ( info_unit != 255 )
    {
      /* Info is ready - scale the value */
      *val = ( ( geni_value * (float)(info_range) / full_scale ) + info_zero) * scale_factor;
      ret_val = true;
    }
  }
  return ret_val;
}



/*****************************************************************************
 * Function ScaleExtendedGeniValue
 * DESCRIPTION: Uses the geni_values info to scale the value to SI.
 *
 *****************************************************************************/
bool GeniSlaveIf::ScaleExtendedGeniValue(U32 geni_value, U8* info, float* val, U8 no_of_bytes)
{
  U8 info_head, info_unit;
  float scale_factor;
  int info_zero;
  bool ret_val = false;

  info_zero = 0;                        // Initialize
  info_head = *info;
  info++;
  info_unit = *info;
  info++;
  if ( ( (info_head & GENI_INFO_SIF_MASK) == GENI_INFO_EXTENDED_PRECISION ) )
  {
    if ( (info_head & GENI_INFO_VI_MASK) != GENI_INFO_VI_0_255 )
    {
      if ( ((geni_value>>(8*(no_of_bytes-1))) & 0xFF) == 0xFF )
      {
        /* 0xFF not valid - set info_unit to invalid */
        info_unit = 255;
      }
    }
    info_zero = (int)((*info)<<8);
    info++;
    info_zero |= (int)(*info);
  }
  else
  {
    /* Not extended info - set info_unit to invalid */
    info_unit = 255;
  }
  info++;

  if ( (info_unit & GENI_INFO_SZ_MASK) == GENI_INFO_SZ_NEGATIVE )
  {
    info_zero = -info_zero;
  }

  if ( ( (info_head & GENI_INFO_SIF_MASK) == GENI_INFO_EXTENDED_PRECISION ) )
  {
    if ( !GetScaleFactor(info_unit, &scale_factor) )
    {
      /* Info unit is not usable - set info_unit to invalid */
      info_unit = 255;
    }
    if ( info_unit != 255 )
    {
      /* Info is ready - scale the value */
      *val = ( geni_value + info_zero ) * scale_factor;
      ret_val = true;
    }
  }
  return ret_val;
}


/*****************************************************************************
 * Function - GetScaleFactor
 * DESCRIPTION:
 *
 *****************************************************************************/
bool GeniSlaveIf::GetScaleFactor(U8 info_unit, float* p_scale_factor)
{
  bool ret_val = true;

  switch ( (info_unit & (~GENI_INFO_SZ_MASK)) )
  {
    /* Current scaling */
    case GENI_UNIT_1UA :
      *p_scale_factor = 1E-6f;
      break;
    case GENI_UNIT_100MA :
      *p_scale_factor = 0.1f;
      break;
    case GENI_UNIT_200MA :
      *p_scale_factor = 0.2f;
      break;
    case GENI_UNIT_500MA :
      *p_scale_factor = 0.5f;
      break;
    case GENI_UNIT_5A :
      *p_scale_factor = 5;
      break;

    /* Voltage scaling */
    case GENI_UNIT_100MV :
      *p_scale_factor = 0.1f;
      break;
    case GENI_UNIT_1V :
      *p_scale_factor = 1;
      break;
    case GENI_UNIT_2V :
      *p_scale_factor = 2;
      break;
    case GENI_UNIT_5V :
      *p_scale_factor = 5;
      break;

    /* Power scaling */
    case GENI_UNIT_1W :
      *p_scale_factor = 1;
      break;
    case GENI_UNIT_10W :
      *p_scale_factor = 10;
      break;
    case GENI_UNIT_100W :
      *p_scale_factor = 100;
      break;
    case GENI_UNIT_1KW :
      *p_scale_factor = 1000;
      break;
    case GENI_UNIT_10KW :
      *p_scale_factor = 10000;
      break;

    /* Temperature scaling */
    case GENI_UNIT_DOT1C :
      *p_scale_factor = 0.1f;
      break;
    case GENI_UNIT_1C :
      *p_scale_factor = 1;
      break;
    case GENI_UNIT_1F :
      *p_scale_factor = 1;
      break;
    case GENI_UNIT_DOT01K :
      *p_scale_factor = 0.01f;
      break;

    /* Frequency scaling */
    case GENI_UNIT_05HZ :
      *p_scale_factor = 0.5f;
      break;
    case GENI_UNIT_1HZ :
      *p_scale_factor = 1;
      break;
    case GENI_UNIT_2HZ :
      *p_scale_factor = 2;
      break;
    case GENI_UNIT_2DOT5HZ :
      *p_scale_factor = 2.5f;
      break;

    /* Energy scaling */
    case GENI_UNIT_1WS :
      *p_scale_factor = 1;
      break;
    case GENI_UNIT_1WH :
      *p_scale_factor = 3600;
      break;
    case GENI_UNIT_DOT1KWH :
      *p_scale_factor = (3600*100);
      break;
    case GENI_UNIT_1KWH :
      *p_scale_factor = (3600*1000);
      break;
    case GENI_UNIT_2KWH :
      *p_scale_factor = (2*3600*1000);
      break;
    case GENI_UNIT_10KWH :
      *p_scale_factor = (10*3600*1000);
      break;
    case GENI_UNIT_100KWH :
      *p_scale_factor = (100*3600*1000);
      break;
    case GENI_UNIT_512KWH :
      *p_scale_factor = (512*3600*1000);
      break;
    case GENI_UNIT_1MWH :
      *p_scale_factor = ((float)1000*3600*1000);
      break;
    case GENI_UNIT_10MWH :
      *p_scale_factor = ((float)10000*3600*1000);
      break;
    case GENI_UNIT_100MWH :
      *p_scale_factor = ((float)100000*3600*1000);
      break;

    /* Time scaling */
    case GENI_UNIT_1024H :
      *p_scale_factor = (1024*60*60);
      break;
    case GENI_UNIT_100H :
      *p_scale_factor = (100*60*60);
      break;
    case GENI_UNIT_1024MIN :
      *p_scale_factor = (1024*60);
      break;
    case GENI_UNIT_2H :
      *p_scale_factor = (2*60*60);
      break;
    case GENI_UNIT_1H :
      *p_scale_factor = (60*60);
      break;
    case GENI_UNIT_2MIN :
      *p_scale_factor = (2*60);
      break;
    case GENI_UNIT_1MIN :
      *p_scale_factor = 60;
      break;
    case GENI_UNIT_30S :
      *p_scale_factor = 30;
      break;
    case GENI_UNIT_10S :
      *p_scale_factor = 10;
      break;
    case GENI_UNIT_1S :
      *p_scale_factor = 1;
      break;
    case GENI_UNIT_DOT1S :
      *p_scale_factor = 0.1f;
      break;

    /* Rot. velocity scaling */
    case GENI_UNIT_1RPM :
      *p_scale_factor = 1;
      break;
    case GENI_UNIT_12RPM :
      *p_scale_factor = 12;
      break;
    case GENI_UNIT_100RPM :
      *p_scale_factor = 100;
      break;

    /* Torque scaling */
    case GENI_UNIT_1NM :
      *p_scale_factor = 1;
      break;
    case GENI_UNIT_DOT1NM :
      *p_scale_factor = 0.1f;
      break;

    default:
      ret_val = false;
      break;
  }

  return ret_val;
}




/*****************************************************************************
 * Function - GetDevice
 * DESCRIPTION: Get geni slave device if it exists in network list and no
 *              comm error.
*****************************************************************************/
bool GeniSlaveIf::GetDevice(U8 unit_index, GENI_DEVICE_TYPE* device)
{
  bool ret_val = true;

  if (unit_index == NO_UNIT)
  {
    ret_val = false;
  }
  else if (mUnitCommError[unit_index] == true)
  {
    ret_val = false;
  }
  else
  {
    *device = mUnit2Device[unit_index];
  }
  return ret_val;
}



/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniSlaveIf::CheckForInfoRequest()
{
  int unit_index;
  bool all_units_checked;
  bool dir_tlg_sent;
  GENI_DEVICE_TYPE device;

  if ( mInfoRequestSent == false )
  {
    /* There is no info request active - a new can be sent */
    all_units_checked = false;
    unit_index = mNextInfoToRequest;
    while ( (mInfoRequestSent == false) && (all_units_checked == false) )
    {
      if (GetDevice(unit_index, &device) == true)
      {
        /* Info is only requested from existing units without communication error */
        if ( mUnitInfoWanted[unit_index] == true && mUnitInfoReceived[unit_index] == false )
        {
          /* The unit needs info data - send a request */
          OS_Use(&geni_master);
          dir_tlg_sent = SendDirAPDU( SLAVE_INFO_RESP_FNC, (U8 *)apdu_cue_info_request, network_list[unit_index].unit_addr );
          OS_Unuse(&geni_master);
          if ( dir_tlg_sent == true )
          {
            mInfoUnitIndex = unit_index;
            mInfoRequestSent = true;
          }
        }
      }
      unit_index++;
      if ( unit_index >= MAX_NUMBER_OF_UNITS )
      {
        unit_index = 0;
      }
      if ( unit_index == mNextInfoToRequest )
      {
        /* All units have been checked - no info needed */
        all_units_checked = true;
      }
    }
    mNextInfoToRequest = unit_index;
  }
}

/*****************************************************************************
 * Function InitDeviceConfiguration
 * DESCRIPTION: Fill out mConfiguration array
 *
 *****************************************************************************/
void GeniSlaveIf::InitDeviceConfiguration()
{
//  int i;

//  /* E-Pump */
//  mConfiguration[0].GscNo = E_PUMP_GSC;
//  mConfiguration[0].GeniDevice = DEVICE_E_PUMP;
//  /* IO351-Pump */
//  mConfiguration[1].GscNo = IO351_PUMP_GSC;
//  mConfiguration[1].GeniDevice = DEVICE_IO351_PUMP;
//  /* Clear apdu's */
//  for (i=0; i<GSC_APDU_LEN; i++)
//  {
//    mConfiguration[0].Apdu[i] = 0;
//    mConfiguration[1].Apdu[i] = 0;
//  }
}

/*****************************************************************************
 * Function StoreDeviceConfiguration
 * DESCRIPTION: If a GSC configuration exists it is retrieved from the GSC class
 * and stored in mConfiguration[device_index].Apdu
 *****************************************************************************/
void GeniSlaveIf::StoreDeviceConfiguration(int device_index)
{
//krv  GSCConfiguration* p_gsc_config;
//krv  GSC_CLASS4_ITEM_TYPE* p_class_4;
//  int len, apdu_index, i;

//  GSC::GetInstance()->UseGsc();
//  if (GSC::GetInstance()->GetConfigurationPointer(mConfiguration[device_index].GscNo, &p_gsc_config))
//  {
//    if (p_gsc_config->IsConfigurationValid())
//    {
//      // class 4
//      len = p_gsc_config->GetNumberOfClass4Items();
//      if (len > 0)
//      {
//        mConfiguration[device_index].Apdu[0] = len*2 + 2;      //LE
//        mConfiguration[device_index].Apdu[1] = 4;              //CL
//        mConfiguration[device_index].Apdu[2] = 0x80 | (len*2); //OP | LE
//        apdu_index = 3;
//        for (i=0; i<len; i++)
//        {
//          p_class_4 = p_gsc_config->GetClass4Item(i);
//          mConfiguration[device_index].Apdu[apdu_index] = p_class_4->id;
//          apdu_index++;
//          mConfiguration[device_index].Apdu[apdu_index] = p_class_4->value;
//          apdu_index++;
//        }
//      }
//      // class 12: Not supported!
//      // class 15: Not supported!
//    }
//  }
//  GSC::GetInstance()->UnuseGsc();
}

/*****************************************************************************
 * Function - SendDeviceConfiguration
 * DESCRIPTION: If device configuration exists for the device argument it is
 * send to unit by addr argument.
 *
 * true is returned if configuration is send or if configuration does not exist.
 * false is returned if configuration could not be send.
 *****************************************************************************/
bool GeniSlaveIf::SendDeviceConfiguration(U8 addr, GENI_DEVICE_TYPE device)
{
  bool return_value = true;
//  U8* p_apdu = 0;

//  if (GetDeviceConfiguration(device, &p_apdu))
//  {
//    OS_Use(&geni_master);
//    if (SendDirAPDU(DUMMY_RESP_FNC, p_apdu, addr) == FALSE)
//    {
//      return_value = false;
//    }
//    OS_Unuse(&geni_master);
//  }

  return return_value;
}

/*****************************************************************************
 * Function GetDeviceConfiguration
 * DESCRIPTION: Chekcs if the requested device configuration exists.
 * If not false is returned.
 * If true is returned p_apdu now points at the requested configuration.
 *****************************************************************************/
bool GeniSlaveIf::GetDeviceConfiguration(GENI_DEVICE_TYPE device, U8** p_apdu)
{
  bool return_value = false;

//  bool config_found = false;
//  int config_index = 0;

//  while (config_index < NO_OF_DEVICES_TO_CONFIG && !config_found)
//  {
//    if (mConfiguration[config_index].GeniDevice == device)
//    {
//      config_found = true;
//      if (mConfiguration[config_index].Apdu[0] > 0)
//      {
//        *p_apdu = mConfiguration[config_index].Apdu;
//        return_value = true;
//      }
//    }
//    config_index++;
//  }
  return return_value;
}

/*****************************************************************************
 * Function CheckForPendingUnitConfiguration
 * DESCRIPTION: Check if units needs configuration. If so configuration is
 * tried once again.
 *****************************************************************************/
void GeniSlaveIf::CheckForPendingUnitConfiguration()
{
//  int unit_index;
//  GENI_DEVICE_TYPE device;
//  U8 addr;

//  for (unit_index=0; unit_index<MAX_NUMBER_OF_UNITS; unit_index++)
//  {
//    if (mUnitConfigPending[unit_index])
//    {
//      if (GetDevice(unit_index, &device))
//      {
//        /* Configuration is only send to existing units without communication error */
//        OS_Use(&geni_master);
//        addr = network_list[unit_index].unit_addr;
//        OS_Unuse(&geni_master);
//        if (SendDeviceConfiguration(addr, device))
//          mUnitConfigPending[unit_index] = false;
//      }
//    }
//  }
}

/*****************************************************************************
 * Function CheckForAutoPollEnableDisable
 * DESCRIPTION: Once in a while, insert an error pump in the auto poll list to
 * check if it has become ok again.
 * Note: Pumps are removed from the auto poll list in SetComError to avoid
 * Genibus delays for ok units due to long reply timeout for error units.
 *****************************************************************************/
void GeniSlaveIf::CheckForAutoPollEnableDisable()
{
  static int unit_index = 0, testing_unit = MAX_NUMBER_OF_UNITS;

  if ( testing_unit < MAX_NUMBER_OF_UNITS )
  {
    if ( mUnitCommError[testing_unit] == true )
    {
      // The pump is still not ready. Disable auto poll again
      network_list[testing_unit].device_type = NO_DEVICE;
    }
    testing_unit = MAX_NUMBER_OF_UNITS;
  }
  else
  {
    int loop_count = 0;
    while ( loop_count < (MAX_NUMBER_OF_UNITS)/2 )  // Just test half the units in each run
    {
      if ( mUnit2Device[unit_index] == DEVICE_E_PUMP || mUnit2Device[unit_index] == DEVICE_IO111 || mUnit2Device[unit_index] == DEVICE_MP204 || mUnit2Device[unit_index] == DEVICE_DDA)
      {
        if ( mUnitCommError[unit_index] == true )
        {
          // Pump is not ok. Enable auto poll for pump for 1 second
          network_list[unit_index].device_type = mUnit2Device[unit_index];
          testing_unit = unit_index;
          loop_count = MAX_NUMBER_OF_UNITS;
        }
        else if ( network_list[unit_index].device_type == NO_DEVICE )
        {
          // Pump is ok. Ensure to enable auto poll again (async with SetComError called from Geni task)
          network_list[unit_index].device_type = mUnit2Device[unit_index];
        }
      }
      loop_count++;
      unit_index = ++unit_index % (MAX_NUMBER_OF_UNITS);
    }
  }
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 *               C FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - set_geni_error
 * DESCRIPTION:
 *
 *****************************************************************************/
extern "C" void set_geni_error(unsigned char nw_index, unsigned char fault_state)
{
  GeniSlaveIf::GetInstance()->SetGeniError(nw_index, fault_state);
}

/*****************************************************************************
 * Function - SlaveInfoFromUnit0-7
 * DESCRIPTION:
 * Responsefunctions for direct telegram
 *****************************************************************************/
extern "C" void SlaveInfoFromUnit(void)
{
  GeniSlaveIf::GetInstance()->IncomingSlaveInfo();
}

/*****************************************************************************
 * Function - ConfigFromIO351
 * DESCRIPTION:
 * Responsefunctions for direct telegram
 *****************************************************************************/
extern "C" void ConfigFromIO351(void)
{
  GeniSlaveIf::GetInstance()->IncomingIO351ModuleConfig();
}

/*****************************************************************************
 * Function - DummyReply
 * DESCRIPTION:
 * Responsefunction for direct telegram. Used when no action is
 * needed on request reply
 *****************************************************************************/
extern "C" void DummyReply(void)
{
}
